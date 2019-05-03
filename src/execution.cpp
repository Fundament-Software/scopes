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
#if SCOPES_USE_ORCJIT
#include <llvm-c/OrcBindings.h>
#include <llvm-c/BitWriter.h>
#else
#include <llvm-c/ExecutionEngine.h>
#include "llvm/IR/Module.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#endif

#include <limits.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <vector>

#define SCOPES_CACHE_KEY_BITCODE 1

namespace scopes {

static void *global_c_namespace = nullptr;

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

#if SCOPES_USE_ORCJIT

LLVMOrcJITStackRef orc = nullptr;
LLVMTargetMachineRef target_machine = nullptr;

//static std::vector<void *> loaded_libs;
static std::unordered_map<Symbol, void *, Symbol::Hash> cached_dlsyms;

LLVMTargetMachineRef get_target_machine() {
    return target_machine;
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

    const char *CPU = nullptr;
    const char *Features = nullptr;
    target_machine = LLVMCreateTargetMachine(target, triple, CPU, Features,
        optlevel, reloc, codemodel);
    assert(target_machine);
    orc = LLVMOrcCreateInstance(target_machine);
    assert(orc);
#if 0
    LLVMOrcTargetAddress retaddr = 0;
    LLVMOrcErrorCode err = LLVMOrcCreateLazyCompileCallback(orc, &retaddr, lazy_compile_callback, nullptr);
    assert(err == LLVMOrcErrSuccess);
#endif
    return {};
}

static std::vector<const PointerMap *> pointer_maps;

static uint64_t orc_symbol_resolver(const char *name, void *ctx) {
    const PointerMap *map = (const PointerMap *)ctx;
    if (map) {
        auto it = map->find(name);
        if (it != map->end()) {
            return (uint64_t)it->second;
        }
    }
    void *ptr = retrieve_symbol(name);
    if (!ptr) {
        printf("ORC failed to resolve symbol: %s\n", name);
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

    if (cache && filepath) {
        char *errormsg;
        if (LLVMCreateMemoryBufferWithContentsOfFile(filepath, &membuf, &errormsg)) {
            SCOPES_ERROR(CGenBackendFailed, errormsg);
        }
    } else {
        auto target_machine = get_target_machine();
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
    }

    if (irbuf) {
        LLVMDisposeMemoryBuffer(irbuf);
    }

    LLVMOrcModuleHandle newhandle = 0;
    auto ptrmap = new PointerMap(map);
    pointer_maps.push_back(ptrmap);
    auto err = LLVMOrcAddObjectFile(orc, &newhandle, membuf,
        orc_symbol_resolver, ptrmap);
    if (!err) {
        module_handles.push_back(newhandle);
    }

    //assert(false);

    if (err) {
        SCOPES_ERROR(ExecutionEngineFailed, LLVMGetErrorMessage(err));
    }

    return {};
}

SCOPES_RESULT(uint64_t) get_address(const char *name) {
    SCOPES_RESULT_TYPE(uint64_t);
    LLVMOrcTargetAddress addr = 0;
    auto err = LLVMOrcGetSymbolAddress(orc, &addr, name);
    if (err) {
        SCOPES_ERROR(ExecutionEngineFailed, LLVMGetErrorMessage(err));
    }
    if (!addr) {
        StyledStream ss;
        ss << "get_address(): symbol missing '" << name << "'" << std::endl;
    }
    assert(addr);
    return addr;
}

SCOPES_RESULT(void *) get_pointer_to_global(LLVMValueRef g) {
    SCOPES_RESULT_TYPE(void *);
    size_t length = 0;
    const char *sym_name = LLVMGetValueName2(g, &length);
    assert(length);
#if 0
    char *mangled_sym_name = nullptr;
    LLVMOrcGetMangledSymbol(orc, &mangled_sym_name, sym_name);
    printf("mangled sym: %s\n", mangled_sym_name);
    LLVMOrcDisposeMangledSymbol(mangled_sym_name);
#endif
    return (void *)SCOPES_GET_RESULT(get_address(sym_name));
}

#else // !SCOPES_USE_ORCJIT

static LLVMExecutionEngineRef ee = nullptr;

#if 0
LLVMExecutionEngineRef get_execution_engine() {
    return ee;
}
#endif

void add_jit_event_listener(LLVMJITEventListenerRef listener) {
    llvm::ExecutionEngine *pEE = reinterpret_cast<llvm::ExecutionEngine*>(ee);
    pEE->RegisterJITEventListener(
        reinterpret_cast<llvm::JITEventListener *>(listener));
}

SCOPES_RESULT(void) init_execution() {
    return {};
}

SCOPES_RESULT(void) add_module(LLVMModuleRef module) {
    SCOPES_RESULT_TYPE(void);
    if (!ee) {
        char *errormsg = nullptr;

        LLVMMCJITCompilerOptions opts;
        LLVMInitializeMCJITCompilerOptions(&opts, sizeof(opts));
        opts.OptLevel = 0;
        opts.NoFramePointerElim = true;

        if (LLVMCreateMCJITCompilerForModule(&ee, module, &opts,
            sizeof(opts), &errormsg)) {
            SCOPES_ERROR(ExecutionEngineFailed, errormsg);
        }
    }
    LLVMAddModule(ee, module);
    llvm::ExecutionEngine *pEE = reinterpret_cast<llvm::ExecutionEngine*>(ee);
    pEE->runStaticConstructorsDestructors(
        *reinterpret_cast<llvm::Module *>(module), false);
    return {};
}

SCOPES_RESULT(uint64_t) get_address(const char *name) {
    SCOPES_RESULT_TYPE(uint64_t);
    auto ptr = LLVMGetGlobalValueAddress(ee, name);
    if (ptr) return ptr;
    llvm::ExecutionEngine *pEE = reinterpret_cast<llvm::ExecutionEngine*>(ee);
    return pEE->getFunctionAddress(name);
}

SCOPES_RESULT(void *) get_pointer_to_global(LLVMValueRef g) {
    return LLVMGetPointerToGlobal(ee, g);
}

LLVMTargetMachineRef get_target_machine() {
    assert(ee);
    return LLVMGetExecutionEngineTargetMachine(ee);
}

#endif // !SCOPES_USE_ORCJIT

void init_llvm() {
    global_c_namespace = dlopen(NULL, RTLD_LAZY);

    LLVMEnablePrettyStackTrace();
    #if !SCOPES_USE_ORCJIT
        LLVMLinkInMCJIT();
        //LLVMLinkInInterpreter();
    #endif
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmParser();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeDisassembler();
}

} // namespace scopes
