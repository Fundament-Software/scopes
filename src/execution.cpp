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
#include "timer.hpp"

#ifdef SCOPES_WIN32
#include "dlfcn.h"
#include "stdlib_ex.h"
#include <math.h>
#else
#include <dlfcn.h>
#endif

#include <llvm-c/Core.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Support.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/LLJIT.h>
#include <llvm-c/OrcEE.h>
#include <llvm-c/Disassembler.h>

#include <llvm-c/Transforms/PassManagerBuilder.h>
#include <llvm-c/Transforms/InstCombine.h>
#include <llvm-c/Transforms/IPO.h>

#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/Object/SymbolSize.h"

#include "llvm/Support/TargetSelect.h"

#include <limits.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <vector>

#include <zlib.h>
#include "absl/container/flat_hash_map.h"

#define SCOPES_CACHE_KEY_BITCODE 1
#define SCOPES_LLVM_SUPPORT_DISASSEMBLY 1

#if SCOPES_MACOS
#define SCOPES_JIT_SYMBOL_PREFIX '_'
#else
#define SCOPES_JIT_SYMBOL_PREFIX 0
#endif

namespace scopes {

////////////////////////////////////////////////////////////////////////////////

#if SCOPES_LLVM_SUPPORT_DISASSEMBLY
static void pprint(int pos, unsigned char *buf, int len, const char *disasm) {
  int i;
  printf("%04x:  ", pos);
  for (i = 0; i < 8; i++) {
    if (i < len) {
      printf("%02x ", buf[i]);
    } else {
      printf("   ");
    }
  }

  printf("   %s\n", disasm);
}

static void do_disassemble(LLVMTargetMachineRef tm, void *fptr, int siz) {

    unsigned char *buf = (unsigned char *)fptr;

  LLVMDisasmContextRef D = LLVMCreateDisasmCPUFeatures(
    LLVMGetTargetMachineTriple(tm),
    LLVMGetTargetMachineCPU(tm),
    LLVMGetTargetMachineFeatureString(tm),
    NULL, 0, NULL, NULL);
    LLVMSetDisasmOptions(D,
        LLVMDisassembler_Option_PrintImmHex);
  char outline[1024];
  int pos;

  if (!D) {
    printf("ERROR: Couldn't create disassembler\n");
    return;
  }

  pos = 0;
  while (pos < siz) {
    size_t l = LLVMDisasmInstruction(D, buf + pos, siz - pos, 0, outline,
                                     sizeof(outline));
    if (!l) {
      pprint(pos, buf + pos, 1, "\t???");
      pos++;
        break;
    } else {
      pprint(pos, buf + pos, l, outline);
      pos += l;
    }
  }

  LLVMDisasmDispose(D);
}

class DisassemblyListener : public llvm::JITEventListener {
public:
    bool enabled = false;

    DisassemblyListener() {}

    absl::flat_hash_map<std::string, size_t> sizes;

    void InitializeDebugData(
        llvm::StringRef name,
        llvm::object::SymbolRef::Type type, uint64_t sz) {
        if(type == llvm::object::SymbolRef::ST_Function) {
            sizes[name.data()] = sz;
        }
    }

    virtual void notifyObjectLoaded(
        ObjectKey K,
        const llvm::object::ObjectFile &Obj,
        const llvm::RuntimeDyld::LoadedObjectInfo &L) {
        if (!enabled)
            return;
        #if 0
        StyledStream ss;
        ss << "object emitted!" << std::endl;
        #endif
        auto size_map = llvm::object::computeSymbolSizes(Obj);
        for(auto & S : size_map) {
            llvm::object::SymbolRef sym = S.first;
            auto name = sym.getName();
            auto type = sym.getType();
            if(name && type)
                InitializeDebugData(name.get(),type.get(),S.second);
        }
    }
};
#endif

#if SCOPES_LLVM_SUPPORT_DISASSEMBLY
static DisassemblyListener *disassembly_listener = nullptr;
#endif

void enable_disassembly(bool enable) {
#if SCOPES_LLVM_SUPPORT_DISASSEMBLY
    assert(disassembly_listener);
    disassembly_listener->enabled = enable;
#endif
}

void print_disassembly(std::string symbol, void *pfunc) {
#if SCOPES_LLVM_SUPPORT_DISASSEMBLY
    assert(disassembly_listener);
    //auto td = LLVMGetExecutionEngineTargetData(ee);
    auto it = disassembly_listener->sizes.find(symbol);
    if (it != disassembly_listener->sizes.end()) {
        std::cout << "disassembly:\n";
        auto target_machine = get_jit_target_machine();
        do_disassemble(target_machine, pfunc, it->second);
        return;
    }
    std::cout << "no disassembly available\n";
#else
    std::cout << "disassembly is unsupported\n";
#endif
}

////////////////////////////////////////////////////////////////////////////////

void build_and_run_opt_passes(LLVMModuleRef module, int opt_level) {
    LLVMPassManagerBuilderRef passBuilder;

    passBuilder = LLVMPassManagerBuilderCreate();
    LLVMPassManagerBuilderSetOptLevel(passBuilder, opt_level);
    LLVMPassManagerBuilderSetSizeLevel(passBuilder, 2);
    if (opt_level == 0) {
        LLVMPassManagerBuilderSetDisableUnrollLoops(passBuilder, true);
    }
    //LLVMAddInstructionCombiningPass(passBuilder);
    #if 1
    if (opt_level >= 2) {
        LLVMPassManagerBuilderUseInlinerWithThreshold(passBuilder, 225);
    }
    #endif

    LLVMPassManagerRef functionPasses =
      LLVMCreateFunctionPassManagerForModule(module);
    LLVMPassManagerRef modulePasses =
      LLVMCreatePassManager();
    //LLVMAddAnalysisPasses(LLVMGetExecutionEngineTargetMachine(ee), functionPasses);

    LLVMPassManagerBuilderPopulateFunctionPassManager(passBuilder,
                                                      functionPasses);
    LLVMPassManagerBuilderPopulateModulePassManager(passBuilder, modulePasses);

    LLVMPassManagerBuilderDispose(passBuilder);
    if (opt_level == 0) {
        LLVMAddDeadArgEliminationPass(modulePasses);
        LLVMAddInstructionCombiningPass(modulePasses);
    }

    LLVMInitializeFunctionPassManager(functionPasses);
    for (LLVMValueRef value = LLVMGetFirstFunction(module);
         value; value = LLVMGetNextFunction(value))
      LLVMRunFunctionPassManager(functionPasses, value);
    LLVMFinalizeFunctionPassManager(functionPasses);

    LLVMRunPassManager(modulePasses, module);

    LLVMDisposePassManager(functionPasses);
    LLVMDisposePassManager(modulePasses);
}

////////////////////////////////////////////////////////////////////////////////

static void *global_c_namespace = nullptr;
//static LLVMOrcJITStackRef orc = nullptr;
static LLVMOrcLLJITRef orc = nullptr;
static LLVMOrcObjectLayerRef object_layer = nullptr;
static LLVMOrcJITDylibRef jit_dylib = nullptr;
static LLVMTargetMachineRef jit_target_machine = nullptr;
static LLVMTargetMachineRef object_target_machine = nullptr;
//static std::vector<void *> loaded_libs;
static absl::flat_hash_map<Symbol, void *, Symbol::Hash> cached_dlsyms;

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
    LLVMOrcJITTargetAddress addr = 0;
    auto err = LLVMOrcLLJITLookup(orc, &addr, name);
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

#if 0
uint64_t lazy_compile_callback(LLVMOrcJITStackRef orc, void *ctx) {
    printf("lazy_compile_callback ???\n");
    return 0;
}
#endif

static LLVMErrorRef definition_generator(
    LLVMOrcDefinitionGeneratorRef GeneratorObj, void *Ctx,
    LLVMOrcLookupStateRef *LookupState, LLVMOrcLookupKind Kind,
    LLVMOrcJITDylibRef JD, LLVMOrcJITDylibLookupFlags JDLookupFlags,
    LLVMOrcCLookupSet LookupSet, size_t LookupSetSize) {
    std::vector<LLVMJITCSymbolMapPair> symbolpairs;

    for (int i = 0; i < LookupSetSize; ++i) {
        auto name = LookupSet[i].Name;
        auto str = LLVMOrcSymbolStringPoolEntryStr(name);
        auto ptr = retrieve_symbol(str);
        //printf("request[%i] = \"%s\" : %p\n", i, str, ptr);
        if (ptr) {
            LLVMJITCSymbolMapPair pair;
            memset(&pair, 0, sizeof(pair));
            pair.Name = name;
            pair.Sym.Address = (uint64_t)ptr;
            symbolpairs.push_back(pair);
        }
    }

    /*auto custom = LLVMOrcCreateCustomMaterializationUnit("Custom Materialization", nullptr, orcpairs.data(), orcpairs.size(), symbolpairs[0].Name,
      [](void* Ctx, LLVMOrcMaterializationResponsibilityRef MR) {
        int MainResult = 0;

        size_t NumSymbols;
        LLVMOrcSymbolStringPoolEntryRef* Symbols =
          LLVMOrcMaterializationResponsibilityGetRequestedSymbols(MR, &NumSymbols);

        for (int i = 0; i < NumSymbols; ++i)
          printf("%s\n", LLVMOrcSymbolStringPoolEntryStr(Symbols[i]));

        LLVMOrcDisposeSymbols(Symbols);
      },
      [](void* Ctx, LLVMOrcJITDylibRef JD, LLVMOrcSymbolStringPoolEntryRef Symbol) { printf("%s\n", LLVMOrcSymbolStringPoolEntryStr(Symbol)); },
        [](void* Ctx) {});
    err = LLVMOrcJITDylibDefine(jit_dylib, custom);*/

    auto mu = LLVMOrcAbsoluteSymbols(symbolpairs.data(), symbolpairs.size());
    return LLVMOrcJITDylibDefine(jit_dylib, mu);
}

static LLVMOrcObjectLayerRef llvm_create_object_layer(
    void *Ctx, LLVMOrcExecutionSessionRef ES, const char *Triple) {

    object_layer = LLVMOrcCreateRTDyldObjectLinkingLayerWithSectionMemoryManager(ES);
    LLVMOrcRTDyldObjectLinkingLayerRegisterJITEventListener(object_layer, LLVMCreateGDBRegistrationListener());

#if SCOPES_LLVM_SUPPORT_DISASSEMBLY
    if (!disassembly_listener) {
        disassembly_listener = new DisassemblyListener();
        llvm::JITEventListener *le = disassembly_listener;
        LLVMOrcRTDyldObjectLinkingLayerRegisterJITEventListener(object_layer,
            llvm::wrap(le));
    }
#endif

	return object_layer;
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
    // temporary, will be consumed by orc creation
    auto jtm = LLVMCreateTargetMachine(target, triple, CPU, Features,
        optlevel, reloc, codemodel);
    assert(jtm);

    auto builder = LLVMOrcCreateLLJITBuilder();
    auto tmb = LLVMOrcJITTargetMachineBuilderCreateFromTargetMachine(jtm);
    LLVMOrcLLJITBuilderSetJITTargetMachineBuilder(builder, tmb);
    LLVMOrcLLJITBuilderSetObjectLinkingLayerCreator(builder, llvm_create_object_layer, nullptr);

    auto err = LLVMOrcCreateLLJIT(&orc, builder);
    if (err) {
        SCOPES_ERROR(ExecutionEngineFailed, LLVMGetErrorMessage(err));
    }
    assert(orc);

    jit_dylib = LLVMOrcLLJITGetMainJITDylib(orc);

    LLVMOrcDefinitionGeneratorRef defgen = 0;
    // should we use LLVMOrcLLJITGetGlobalPrefix() here?
    err = LLVMOrcCreateDynamicLibrarySearchGeneratorForProcess(&defgen, SCOPES_JIT_SYMBOL_PREFIX,
        nullptr, nullptr);
    if (err) {
        SCOPES_ERROR(ExecutionEngineFailed, LLVMGetErrorMessage(err));
    }
    LLVMOrcJITDylibAddGenerator(jit_dylib, defgen);

    LLVMOrcJITDylibAddGenerator(jit_dylib,
        LLVMOrcCreateCustomCAPIDefinitionGenerator(&definition_generator, nullptr));

#if 0
    LLVMOrcTargetAddress retaddr = 0;
    LLVMOrcErrorCode err = LLVMOrcCreateLazyCompileCallback(orc, &retaddr, lazy_compile_callback, nullptr);
    assert(err == LLVMOrcErrSuccess);
#endif
    return {};
}

static std::vector<const PointerMap *> pointer_maps;

static LLVMMemoryBufferRef module_to_membuffer(LLVMModuleRef module) {
    LLVMMemoryBufferRef irbuf = nullptr;
#if SCOPES_CACHE_KEY_BITCODE
        irbuf = LLVMWriteBitcodeToMemoryBuffer(module);
#else
        char *s = LLVMPrintModuleToString(module);
        irbuf = LLVMCreateMemoryBufferWithMemoryRangeCopy(s, strlen(s),
            "irbuf");
        LLVMDisposeMessage(s);
#endif
    return irbuf;
}

SCOPES_RESULT(void) add_module(LLVMModuleRef module, const PointerMap &map,
    uint64_t compiler_flags) {
    SCOPES_RESULT_TYPE(void);
#if SCOPES_ALLOW_CACHE
    bool cache = ((compiler_flags & CF_Cache) == CF_Cache);
#else
    const bool cache = false;
#endif

    LLVMMemoryBufferRef irbuf = nullptr;
    LLVMMemoryBufferRef membuf = nullptr;
    if (cache) {
        irbuf = module_to_membuffer(module);
    }

    const String *key = nullptr;
    const char *filepath = nullptr;
    if (cache) {
        assert(irbuf);
        key = get_cache_key(compiler_flags & SCOPES_CACHE_COMPILER_FLAGS,
            LLVMGetBufferStart(irbuf), LLVMGetBufferSize(irbuf));
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
    //LLVMOrcModuleHandle newhandle = 0;
    auto ptrmap = new PointerMap(map);
    pointer_maps.push_back(ptrmap);
    auto ES = LLVMOrcLLJITGetExecutionSession(orc);
    std::vector<LLVMJITCSymbolMapPair> symbolpairs;
    for (auto it = ptrmap->begin(); it != ptrmap->end(); ++it) {
        const char *name = it->first.c_str();
        void *ptr = const_cast< void *>(it->second);

        LLVMJITCSymbolMapPair pair;
        memset(&pair, 0, sizeof(pair));
        pair.Name = LLVMOrcExecutionSessionIntern(ES, name);
        pair.Sym.Address = (uint64_t)ptr;
        symbolpairs.push_back(pair);
    }
    auto mu = LLVMOrcAbsoluteSymbols(symbolpairs.data(), symbolpairs.size());
    err = LLVMOrcJITDylibDefine(jit_dylib, mu);
    if (err) {
        SCOPES_ERROR(ExecutionEngineFailed, LLVMGetErrorMessage(err));
    }

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
            data.data(), data.size(), "");

        #endif

        err = LLVMOrcLLJITAddObjectFile(orc, jit_dylib, membuf);
        //err = LLVMOrcAddObjectFile(orc, &newhandle, membuf, orc_symbol_resolver, ptrmap);
        goto done;
    } else {
        goto skip_cache;
    }
skip_cache:
    {
        if (compiler_flags & CF_O3) {
            Timer optimize_timer(TIMER_Optimize);
            int level = 0;
            if ((compiler_flags & CF_O3) == CF_O1)
                level = 1;
            else if ((compiler_flags & CF_O3) == CF_O2)
                level = 2;
            else if ((compiler_flags & CF_O3) == CF_O3)
                level = 3;
            build_and_run_opt_passes(module, level);
        }

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
        err = LLVMOrcLLJITAddObjectFile(orc, jit_dylib, membuf);
        //err = LLVMOrcAddObjectFile(orc, &newhandle, membuf, orc_symbol_resolver, ptrmap);
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

    /*if (!err) {
        module_handles.push_back(newhandle);
    }*/

    if (err) {
        SCOPES_ERROR(ExecutionEngineFailed, LLVMGetErrorMessage(err));
    }

    return {};
}

SCOPES_RESULT(void) add_object(const char *path) {
    SCOPES_RESULT_TYPE(void);
    LLVMErrorRef err = nullptr;
    //LLVMOrcModuleHandle newhandle = 0;
    LLVMMemoryBufferRef membuf = nullptr;
    char *errormsg;
    if (LLVMCreateMemoryBufferWithContentsOfFile(path, &membuf, &errormsg)) {
        SCOPES_ERROR(CGenBackendFailed, errormsg);
    }
    err = LLVMOrcLLJITAddObjectFile(orc, jit_dylib, membuf);
    //err = LLVMOrcAddObjectFile(orc, &newhandle, membuf, orc_symbol_resolver, nullptr);
    if (!err) {
        //module_handles.push_back(newhandle);
    } else {
        SCOPES_ERROR(ExecutionEngineFailed, LLVMGetErrorMessage(err));
    }

    return {};
}

void init_llvm() {
    global_c_namespace = dlopen(NULL, RTLD_LAZY);

    // remove crash message
    //LLVMEnablePrettyStackTrace();

    // new known targets must also be handled elsewhere; grep SCOPES_KNOWN_TARGETS to find them
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();
    LLVMInitializeNativeDisassembler();
#ifdef SCOPES_TARGET_WEBASSEMBLY
    LLVMInitializeWebAssemblyTargetInfo();
    LLVMInitializeWebAssemblyTarget();
    LLVMInitializeWebAssemblyTargetMC();
    LLVMInitializeWebAssemblyAsmPrinter();
    LLVMInitializeWebAssemblyAsmParser();
    LLVMInitializeWebAssemblyDisassembler();
#endif
#ifdef SCOPES_TARGET_AARCH64
    LLVMInitializeAArch64Target();
    LLVMInitializeAArch64TargetMC();
    LLVMInitializeAArch64TargetInfo();
    LLVMInitializeAArch64AsmPrinter();
    LLVMInitializeAArch64AsmParser();
    LLVMInitializeAArch64Disassembler();
#endif
#ifdef SCOPES_TARGET_RISCV
    LLVMInitializeRISCVTarget();
    LLVMInitializeRISCVTargetMC();
    LLVMInitializeRISCVTargetInfo();
    LLVMInitializeRISCVAsmPrinter();
    LLVMInitializeRISCVAsmParser();
    LLVMInitializeRISCVDisassembler();
#endif



#ifdef SCOPES_WIN32
    // from mingwex.a
    LLVMAddSymbol("ldexpl", (void *)&ldexpl);
    LLVMAddSymbol("ldexpf", (void *)&ldexpf);
    // required by LLVM
    LLVMAddSymbol("sincos", (void *)&sincos);
    LLVMAddSymbol("sincosf", (void *)&sincosf);

#ifdef _MSC_VER
    LLVMAddSymbol("vfprintf", (void *)&vfprintf);
#else
    LLVMAddSymbol("__mingw_vfprintf", (void *)&__mingw_vfprintf);
#endif
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
