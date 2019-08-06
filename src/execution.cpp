/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "execution.hpp"
#include "string.hpp"
#include "error.hpp"
#include "symbol.hpp"
#include "cache.hpp"
#include "compiler_flags.hpp"

#ifdef SCOPES_WIN32
#include "dlfcn.h"
#else
#include <dlfcn.h>
#endif

#include <llvm-c/Core.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Support.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/OrcBindings.h>

#include <limits.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <vector>

#include <zlib.h>

#define SCOPES_CACHE_KEY_BITCODE 1

namespace scopes {

static void *global_c_namespace = nullptr;
static LLVMOrcJITStackRef orc = nullptr;
static LLVMTargetMachineRef jit_target_machine = nullptr;
static LLVMTargetMachineRef object_target_machine = nullptr;
//static std::vector<void *> loaded_libs;
static std::unordered_map<Symbol, void *, Symbol::Hash> cached_dlsyms;

const String *get_default_target_triple() {
    auto str = LLVMGetDefaultTargetTriple();
    auto result = String::from_cstr(str);
    LLVMDisposeMessage(str);
    return result;
}

/** Normalize a target triple. The result needs to be disposed with
  LLVMDisposeMessage. */
//char* LLVMNormalizeTargetTriple(const char* triple);

SCOPES_RESULT(uint64_t) get_address(const char *name) {
    SCOPES_RESULT_TYPE(uint64_t);
    LLVMOrcTargetAddress addr = 0;
    auto err = LLVMOrcGetSymbolAddress(orc, &addr, name);
    if (err) {
        SCOPES_ERROR(ExecutionEngineFailed, LLVMGetErrorMessage(err));
    }
    if (!addr) {
        SCOPES_ERROR(RTGetAddressFailed, Symbol(String::from_cstr(name)));
    }
    return addr;
}

static void *retrieve_symbol(const char *name) {
#if 1
    //auto it = cached_dlsyms.find(name);
    //if (it == cached_dlsyms.end()) {
        void *ptr = LLVMSearchForAddressOfSymbol(name);
        if (!ptr) {
            ptr = dlsym(global_c_namespace, name);
        }
        #if 0
        if (ptr) {
            cached_dlsyms.insert({name, ptr});
        }
        #endif
        return ptr;
    //} else {
    //    return it->second;
    //}
    //LLVMOrcTargetAddress addr = 0;
    //auto err = LLVMOrcGetSymbolAddress(orc, &addr, name);
    //if (addr) return (void *)addr;
#else
    size_t i = loaded_libs.size();
    while (i--) {
        void *ptr = dlsym(loaded_libs[i], name);
        if (ptr) {
            LLVMAddSymbol(name, ptr);
            return ptr;
        }
    }
    return dlsym(global_c_namespace, name);
#endif
}

void *local_aware_dlsym(Symbol name) {
    return retrieve_symbol(name.name()->data);
}

LLVMTargetMachineRef get_jit_target_machine() {
    return jit_target_machine;
}

LLVMTargetMachineRef get_object_target_machine() {
    return object_target_machine;
}

void add_jit_event_listener(LLVMJITEventListenerRef listener) {
    LLVMOrcRegisterJITEventListener(orc, listener);
}

uint64_t lazy_compile_callback(LLVMOrcJITStackRef orc, void *ctx) {
    printf("lazy_compile_callback ???\n");
    return 0;
}

SCOPES_RESULT(void) init_execution() {
    SCOPES_RESULT_TYPE(void);
    if (orc) return {};
    char *triple = LLVMGetDefaultTargetTriple();
    //printf("triple: %s\n", triple);
    char *error_message = nullptr;
    LLVMTargetRef target = nullptr;
    if (LLVMGetTargetFromTriple(triple, &target, &error_message)) {
        SCOPES_ERROR(ExecutionEngineFailed, error_message);
    }
    assert(target);
    assert(LLVMTargetHasJIT(target));
    assert(LLVMTargetHasTargetMachine(target));

    auto optlevel =
        LLVMCodeGenLevelNone;
        //LLVMCodeGenLevelLess;
        //LLVMCodeGenLevelDefault;
        //LLVMCodeGenLevelAggressive;

    auto reloc =
        LLVMRelocDefault;
        //LLVMRelocStatic;
        //LLVMRelocPIC;
        //LLVMRelocDynamicNoPic;

    auto codemodel =
        //LLVMCodeModelDefault;
        LLVMCodeModelJITDefault;
        //LLVMCodeModelSmall;
        //LLVMCodeModelKernel;
        //LLVMCodeModelMedium;
        //LLVMCodeModelLarge;

    object_target_machine = LLVMCreateTargetMachine(target, triple,
        nullptr, nullptr,
        LLVMCodeGenLevelDefault, LLVMRelocStatic, LLVMCodeModelDefault);
    assert(object_target_machine);

    const char *CPU = nullptr;
    const char *Features = nullptr;
    jit_target_machine = LLVMCreateTargetMachine(target, triple, CPU, Features,
        optlevel, reloc, codemodel);
    assert(jit_target_machine);
    orc = LLVMOrcCreateInstance(jit_target_machine);
    assert(orc);

    LLVMOrcRegisterJITEventListener(orc, LLVMCreateGDBRegistrationListener());
#if 0
    LLVMOrcTargetAddress retaddr = 0;
    LLVMOrcErrorCode err = LLVMOrcCreateLazyCompileCallback(orc, &retaddr, lazy_compile_callback, nullptr);
    assert(err == LLVMOrcErrSuccess);
#endif
    return {};
}

static std::vector<const PointerMap *> pointer_maps;

static uint64_t orc_symbol_resolver(const char *name, void *ctx) {
#if SCOPES_MACOS
	assert(*name == '_');
	name++;
#endif
    const PointerMap *map = (const PointerMap *)ctx;
    if (map) {
        auto it = map->find(name);
        if (it != map->end()) {
            return (uint64_t)it->second;
        }
    }
    void *ptr = retrieve_symbol(name);
    if (!ptr) {
        //printf("ORC failed to resolve symbol: %s\n", name);
    }
    return reinterpret_cast<uint64_t>(ptr);
}

static std::vector<LLVMOrcModuleHandle> module_handles;
SCOPES_RESULT(void) add_module(LLVMModuleRef module, const PointerMap &map,
    uint64_t compiler_flags) {
    SCOPES_RESULT_TYPE(void);

    bool cache = ((compiler_flags & CF_Cache) == CF_Cache);

    LLVMMemoryBufferRef irbuf = nullptr;
    LLVMMemoryBufferRef membuf = nullptr;
    if (cache) {
#if SCOPES_CACHE_KEY_BITCODE
        irbuf = LLVMWriteBitcodeToMemoryBuffer(module);
#else
        char *s = LLVMPrintModuleToString(module);
        irbuf = LLVMCreateMemoryBufferWithMemoryRangeCopy(s, strlen(s),
            "irbuf");
        LLVMDisposeMessage(s);
#endif
    }

    const String *key = nullptr;
    const char *filepath = nullptr;
    if (cache) {
        assert(irbuf);
        key = get_cache_key(LLVMGetBufferStart(irbuf), LLVMGetBufferSize(irbuf));
        filepath = get_cache_file(key);

        const char *keyfilepath = get_cache_key_file(key);
        if (keyfilepath) {
            char *errormsg;
            LLVMMemoryBufferRef cmpirbuf = nullptr;
            if (LLVMCreateMemoryBufferWithContentsOfFile(keyfilepath, &cmpirbuf, &errormsg)) {
                SCOPES_ERROR(CGenBackendFailed, errormsg);
            }
            auto sz = LLVMGetBufferSize(irbuf);
            if ((sz != LLVMGetBufferSize(cmpirbuf))
                || (memcmp(LLVMGetBufferStart(irbuf), LLVMGetBufferStart(cmpirbuf),sz) != 0)) {
                auto a = LLVMGetBufferStart(irbuf);
                auto b = LLVMGetBufferStart(cmpirbuf);
                for (int i = 0; i < sz; ++i) {
                    if (*a != *b) {
                        printf("error at character #%i: %c != %c\n", i, *a, *b);
                        break;
                    }
                    a++; b++;
                }
                char newkeyfilepath[PATH_MAX];
                strcpy(newkeyfilepath, keyfilepath);
                strcat(newkeyfilepath, ".conflict");

                FILE *f = fopen(newkeyfilepath, "wb");
                assert(f);
                fwrite(LLVMGetBufferStart(irbuf), 1, LLVMGetBufferSize(irbuf), f);
                fclose(f);

                assert(false);
            }
        }
    }

    LLVMErrorRef err = nullptr;
    LLVMOrcModuleHandle newhandle = 0;
    auto ptrmap = new PointerMap(map);
    pointer_maps.push_back(ptrmap);
    if (cache && filepath) {
        #if 0
        char *errormsg;
        if (LLVMCreateMemoryBufferWithContentsOfFile(filepath, &membuf, &errormsg)) {
            SCOPES_ERROR(CGenBackendFailed, errormsg);
        }
        #else

        auto f = gzopen(filepath, "r");
        if (!f) {
            printf("failed to open cache file for reading (%s)\n", strerror(errno));
            goto skip_cache;
        }

        std::vector<char> data;
    repeat:
        char buf[8192];
        int r = gzread(f, buf, sizeof(buf));
        if (r < 0) {
            printf("failed to read from cache file (%s)\n", strerror(errno));
            goto skip_cache;
        }
        if (r > 0) {
            auto offset = data.size();
            data.resize(offset + r);
            memcpy(&data[offset], buf, r);
        }
        if (r == sizeof(buf))
            goto repeat;

        gzclose(f);

        membuf = LLVMCreateMemoryBufferWithMemoryRangeCopy(
            &data[0], data.size(), "");

        #endif

        err = LLVMOrcAddObjectFile(orc, &newhandle, membuf,
            orc_symbol_resolver, ptrmap);
        goto done;
    } else {
        goto skip_cache;
    }
skip_cache:
    {
        auto target_machine = get_jit_target_machine();
        assert(target_machine);

        char *errormsg;
        if (LLVMTargetMachineEmitToMemoryBuffer(target_machine, module,
            LLVMObjectFile, &errormsg, &membuf)) {
            SCOPES_ERROR(CGenBackendFailed, errormsg);
        }

        if (cache) {
            assert(key && irbuf && membuf);
            set_cache(key, LLVMGetBufferStart(irbuf), LLVMGetBufferSize(irbuf),
                LLVMGetBufferStart(membuf), LLVMGetBufferSize(membuf));
        }

        #if 1
        err = LLVMOrcAddObjectFile(orc, &newhandle, membuf,
            orc_symbol_resolver, ptrmap);
        #else
        LLVMDisposeMemoryBuffer(membuf);
        err = LLVMOrcAddEagerlyCompiledIR(orc, &newhandle, module,
            orc_symbol_resolver, ptrmap);
        #endif
    }

done:
    if (irbuf) {
        LLVMDisposeMemoryBuffer(irbuf);
    }

    if (!err) {
        module_handles.push_back(newhandle);
    }

    if (err) {
        SCOPES_ERROR(ExecutionEngineFailed, LLVMGetErrorMessage(err));
    }

    return {};
}

SCOPES_RESULT(void) add_object(const char *path) {
    SCOPES_RESULT_TYPE(void);
    LLVMErrorRef err = nullptr;
    LLVMOrcModuleHandle newhandle = 0;
    LLVMMemoryBufferRef membuf = nullptr;
    char *errormsg;
    if (LLVMCreateMemoryBufferWithContentsOfFile(path, &membuf, &errormsg)) {
        SCOPES_ERROR(CGenBackendFailed, errormsg);
    }
    err = LLVMOrcAddObjectFile(orc, &newhandle, membuf,
        orc_symbol_resolver, nullptr);
    if (!err) {
        module_handles.push_back(newhandle);
    } else {
        SCOPES_ERROR(ExecutionEngineFailed, LLVMGetErrorMessage(err));
    }

    return {};
}

void init_llvm() {
    global_c_namespace = dlopen(NULL, RTLD_LAZY);

    LLVMEnablePrettyStackTrace();
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();
    LLVMInitializeNativeDisassembler();
    LLVMInitializeWebAssemblyTargetInfo();
    LLVMInitializeWebAssemblyTarget();
    LLVMInitializeWebAssemblyTargetMC();
    LLVMInitializeWebAssemblyAsmPrinter();
    LLVMInitializeWebAssemblyAsmParser();
    LLVMInitializeWebAssemblyDisassembler();

#ifdef SCOPES_WIN32
    // from mingwex.a
    LLVMAddSymbol("ldexpl", (void *)&ldexpl);
    LLVMAddSymbol("ldexpf", (void *)&ldexpf);
    // required by LLVM
    LLVMAddSymbol("sincos", (void *)&sincos);
    LLVMAddSymbol("sincosf", (void *)&sincosf);

#endif

#if 0
    auto targ = LLVMGetFirstTarget();
    while (targ) {
        /** Returns the name of a target. See llvm::Target::getName */
        printf("%s\n", LLVMGetTargetName(targ));

        targ = LLVMGetNextTarget(targ);
    }
#endif

}

} // namespace scopes
