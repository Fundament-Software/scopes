/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "execution.hpp"
#include "string.hpp"
#include "error.hpp"
#include "symbol.hpp"

#ifdef SCOPES_WIN32
#include "dlfcn.h"
#else
#include <dlfcn.h>
#endif

#define SCOPES_USE_ORCJIT 0

#include <llvm-c/Core.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Support.h>
#if !SCOPES_USE_ORCJIT
#include <llvm-c/ExecutionEngine.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <vector>

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

uint64_t lazy_compile_callback(LLVMOrcJITStackRef orc, void *ctx) {
    printf("lazy_compile_callback ???\n");
    return 0;
}

void init_execution() {
    if (orc) return;
    char *triple = LLVMGetDefaultTargetTriple();
    //printf("triple: %s\n", triple);
    char *error_message = nullptr;
    LLVMTargetRef target = nullptr;
    if (LLVMGetTargetFromTriple(triple, &target, &error_message)) {
        location_error(String::from_cstr(error_message));
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
}

static uint64_t orc_symbol_resolver(const char *name, void *ctx) {
    void *ptr = retrieve_symbol(name);
    if (!ptr) {
        printf("ORC failed to resolve symbol: %s\n", name);
    }
    return reinterpret_cast<uint64_t>(ptr);
}

static std::vector<LLVMOrcModuleHandle *> module_handles;
void add_module(LLVMModuleRef module) {
    //LLVMDumpModule(module);
    LLVMSharedModuleRef smod = LLVMOrcMakeSharedModule(module);
    assert(smod);
    LLVMOrcModuleHandle *handle = new LLVMOrcModuleHandle();
    module_handles.push_back(handle);
    LLVMOrcErrorCode err = LLVMOrcAddLazilyCompiledIR(orc, handle, smod, orc_symbol_resolver, nullptr);
    assert(err == LLVMOrcErrSuccess);
}

uint64_t get_address(const char *name) {
    assert(false);
    return 0;
}

void *get_pointer_to_global(LLVMValueRef g) {
    const char *sym_name = LLVMGetValueName(g);
#if 0
    char *mangled_sym_name = nullptr;
    LLVMOrcGetMangledSymbol(orc, &mangled_sym_name, sym_name);
    printf("mangled sym: %s\n", mangled_sym_name);
    LLVMOrcDisposeMangledSymbol(mangled_sym_name);
#endif
    LLVMOrcTargetAddress addr = 0;
    LLVMOrcErrorCode err = LLVMOrcGetSymbolAddress(orc, &addr, sym_name);
    assert(err == LLVMOrcErrSuccess);
    assert(addr);
    return reinterpret_cast<void *>(addr);
}

#else // !SCOPES_USE_ORCJIT

LLVMExecutionEngineRef ee = nullptr;

void init_execution() {
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
            SCOPES_LOCATION_ERROR(String::from_cstr(errormsg));
        }
    }
    LLVMAddModule(ee, module);
    return true;
}

uint64_t get_address(const char *name) {
    return LLVMGetGlobalValueAddress(ee, name);
}

void *get_pointer_to_global(LLVMValueRef g) {
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
