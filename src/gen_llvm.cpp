/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "gen_llvm.hpp"
#include "source_file.hpp"
#include "types.hpp"
#include "platform_abi.hpp"
#include "anchor.hpp"
#include "error.hpp"
#include "execution.hpp"
#include "gc.hpp"
#include "scope.hpp"
#include "timer.hpp"
#include "value.hpp"
#include "stream_expr.hpp"
#include "compiler_flags.hpp"
#include "prover.hpp"
#include "hash.hpp"
#include "qualifiers.hpp"
#include "qualifier.inc"
#include "verify_tools.inc"

#ifdef SCOPES_WIN32
#include "stdlib_ex.h"
#else
#endif
#include <libgen.h>

#include <llvm-c/Core.h>
//#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Transforms/PassManagerBuilder.h>
#include <llvm-c/Transforms/InstCombine.h>
#include <llvm-c/Disassembler.h>
#include <llvm-c/Support.h>
#include <llvm-c/DebugInfo.h>
#include <llvm-c/BitWriter.h>

#include "llvm/IR/Module.h"
//#include "llvm/IR/DebugInfoMetadata.h"
//#include "llvm/IR/DIBuilder.h"
//#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/Object/SymbolSize.h"
//#include "llvm/Support/Timer.h"
//#include "llvm/Support/raw_os_ostream.h"

#include "dyn_cast.inc"

#pragma GCC diagnostic ignored "-Wvla-extension"

namespace scopes {

#define SCOPES_GEN_TARGET "IR"

#define SCOPES_LLVM_SUPPORT_DISASSEMBLY 1

//------------------------------------------------------------------------------
// IL->LLVM IR GENERATOR
//------------------------------------------------------------------------------

static void build_and_run_opt_passes(LLVMModuleRef module, int opt_level) {
    LLVMPassManagerBuilderRef passBuilder;

    passBuilder = LLVMPassManagerBuilderCreate();
    LLVMPassManagerBuilderSetOptLevel(passBuilder, opt_level);
    //LLVMPassManagerBuilderSetOptLevel(passBuilder, 1);
    LLVMPassManagerBuilderSetSizeLevel(passBuilder, 2);
    //LLVMPassManagerBuilderSetDisableUnrollLoops(passBuilder, true);
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

    LLVMInitializeFunctionPassManager(functionPasses);
    for (LLVMValueRef value = LLVMGetFirstFunction(module);
         value; value = LLVMGetNextFunction(value))
      LLVMRunFunctionPassManager(functionPasses, value);
    LLVMFinalizeFunctionPassManager(functionPasses);

    LLVMRunPassManager(modulePasses, module);

    LLVMDisposePassManager(functionPasses);
    LLVMDisposePassManager(modulePasses);
}

const double deg2rad = 0.017453292519943295;
const double rad2deg = 57.29577951308232;

struct PointerNamespace {

    size_t name = 0;
    size_t next_pointer_id = 1;
    char kind = '?';
    std::unordered_map<const void *, std::string> ptr2id;

    std::string get_pointer_id(const void *ptr) {
        auto it = ptr2id.find(ptr);
        if (it != ptr2id.end()) {
            return it->second;
        }
        auto id = next_pointer_id++;
        StyledString ss = StyledString::plain();
        ss.out << "$" << kind << std::hex << name << "_" << id << std::dec;
        auto result = ss.cppstr();
        ptr2id.insert({ptr, result});
        return result;
    }
};

struct PointerNamespaces {
    PointerNamespace local;
    PointerNamespace global;
    PointerNamespace func;
};

struct LLVMIRGenerator {
    enum PMIntrinsic {
        llvm_bitreverse,
        llvm_ctpop,
        llvm_ctlz,
        llvm_cttz,
    };

    enum Intrinsic {
        llvm_sin_f32,
        llvm_sin_f64,
        llvm_cos_f32,
        llvm_cos_f64,
        llvm_sqrt_f32,
        llvm_sqrt_f64,
        llvm_fabs_f32,
        llvm_fabs_f64,
        llvm_trunc_f32,
        llvm_trunc_f64,
        llvm_floor_f32,
        llvm_floor_f64,
        llvm_pow_f32,
        llvm_pow_f64,
        llvm_exp_f32,
        llvm_exp_f64,
        llvm_log_f32,
        llvm_log_f64,
        llvm_exp2_f32,
        llvm_exp2_f64,
        llvm_log2_f32,
        llvm_log2_f64,

        custom_fsign_f32,
        custom_fsign_f64,
        custom_radians_f32,
        custom_radians_f64,
        custom_degrees_f32,
        custom_degrees_f64,

        libc_tan_f32,
        libc_tan_f64,
        libc_asin_f32,
        libc_asin_f64,
        libc_acos_f32,
        libc_acos_f64,
        libc_atan_f32,
        libc_atan_f64,
        libc_atan2_f32,
        libc_atan2_f64,
        libc_sinh_f32,
        libc_sinh_f64,
        libc_cosh_f32,
        libc_cosh_f64,
        libc_tanh_f32,
        libc_tanh_f64,
        libc_asinh_f32,
        libc_asinh_f64,
        libc_acosh_f32,
        libc_acosh_f64,
        libc_atanh_f32,
        libc_atanh_f64,

        NumIntrinsics,
    };

#if 0
    struct HashFuncLabelPair {
        size_t operator ()(const std::pair<LLVMValueRef, Label *> &value) const {
            return
                hash2(std::hash<LLVMValueRef>()(value.first),
                    std::hash<Label *>()(value.second));
        }
    };


    typedef std::pair<LLVMValueRef, Parameter *> ParamKey;
    struct HashFuncParamPair {
        size_t operator ()(const ParamKey &value) const {
            return
                hash2(std::hash<LLVMValueRef>()(value.first),
                    std::hash<Parameter *>()(value.second));
        }
    };

    std::unordered_map<Label *, LLVMValueRef> label2func;
    std::unordered_map< std::pair<LLVMValueRef, Label *>,
        LLVMBasicBlockRef, HashFuncLabelPair> label2bb;
    std::vector< std::pair<Label *, Label *> > bb_label_todo;

    std::unordered_map< ParamKey, LLVMValueRef, HashFuncParamPair> param2value;


    Label::UserMap user_map;
#endif

    typedef std::vector<LLVMValueRef> LLVMValueRefs;
    typedef std::vector<LLVMTypeRef> LLVMTypeRefs;

    std::unordered_map<Symbol, LLVMMetadataRef, Symbol::Hash> file2value;
    std::unordered_map<void *, LLVMValueRef> ptr2global;
    std::unordered_map<ValueIndex, LLVMValueRef, ValueIndex::Hash> ref2value;
    std::unordered_map<Function *, LLVMMetadataRef> func2md;
    std::unordered_map<Function *, Symbol> func_export_table;
    std::unordered_map<Global *, LLVMValueRef> global2global;
    std::vector<LLVMValueRef> constructors;
    LLVMValueRef constructor_function = nullptr;
    std::deque<FunctionRef> function_todo;
    static Types type_todo;
    static std::unordered_map<const Type *, LLVMTypeRef> type_cache;
    static std::unordered_map<Function *, std::string> func_cache;
    static std::unordered_map<Global *, std::string> global_cache;

    static std::unordered_map<size_t, PointerNamespaces *> pointer_namespaces;

    PointerNamespaces *_ns;

    void set_pointer_namespace(size_t name) {
        _ns = get_pointer_namespaces(name);
    }

    static PointerNamespaces *get_pointer_namespaces(size_t name) {
    repeat:
        auto it = pointer_namespaces.find(name);
        if (it != pointer_namespaces.end()) {
            if (!name) return it->second;
            //printf("namespace taken, retrying...\n");
            name = hash2(name, name);
            goto repeat;
        }
        PointerNamespaces *ns = new PointerNamespaces();
        ns->local.name = name;
        ns->local.kind = 'l';
        ns->global.name = name;
        ns->global.kind = 'g';
        ns->func.name = name;
        ns->func.kind = 'f';
        pointer_namespaces.insert({name, ns});
        return ns;
    }

    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMDIBuilderRef di_builder;

    static LLVMTypeRef voidT;
    static LLVMTypeRef i1T;
    static LLVMTypeRef i8T;
    static LLVMTypeRef i16T;
    static LLVMTypeRef i32T;
    static LLVMTypeRef i64T;
    static LLVMTypeRef f32T;
    static LLVMTypeRef f32x2T;
    static LLVMTypeRef f64T;
    static LLVMTypeRef f80T;
    static LLVMTypeRef f128T;
    static LLVMTypeRef rawstringT;
    static LLVMTypeRef noneT;
    static LLVMValueRef noneV;
    static LLVMValueRef falseV;
    static LLVMValueRef trueV;
    static LLVMAttributeRef attr_byval;
    static LLVMAttributeRef attr_sret;
    static LLVMAttributeRef attr_nonnull;
    LLVMValueRef intrinsics[NumIntrinsics];

    // polymorphic intrinsics
    typedef std::pair<PMIntrinsic, LLVMTypeRef> PMIntrinsicKey;
    struct HashPMIntrinsicKey {
        size_t operator ()(const PMIntrinsicKey &value) const {
            return
                hash2(std::hash<PMIntrinsic>()(value.first),
                    std::hash<LLVMTypeRef>()(value.second));
        }
    };

    std::unordered_map< PMIntrinsicKey, LLVMValueRef, HashPMIntrinsicKey > pm_intrinsics;

    bool use_debug_info = true;
    bool generate_object = false;
    bool serialize_pointers = false;
    FunctionRef active_function;
    FunctionRef entry_function;
    std::vector<LLVMValueRef> generated_symbols;

    PointerMap pointer_map;

    std::string get_local_pointer_id(const void *ptr) {
        assert(_ns);
        return _ns->local.get_pointer_id(ptr);
    }

    std::string get_global_pointer_id(const void *ptr) {
        assert(_ns);
        return _ns->global.get_pointer_id(ptr);
    }
    std::string get_func_pointer_id(const void *ptr) {
        assert(_ns);
        return _ns->func.get_pointer_id(ptr);
    }

    static const Type *arguments_to_tuple(const Type *T) {
        if (isa<ArgumentsType>(T)) {
            return cast<ArgumentsType>(T)->to_tuple_type();
        }
        return T;
    }

    static const Type *abi_return_type(const FunctionType *ft) {
        if (ft->has_exception()) {
            if (is_returning_value(ft->except_type)) {
                Types types = { TYPE_Bool, ft->except_type };
                if (is_returning_value(ft->return_type)) {
                    types.push_back(arguments_to_tuple(ft->return_type));
                }
                return tuple_type(types).assert_ok();
            } else if (is_returning_value(ft->return_type)) {
                Types types = { TYPE_Bool, ft->return_type };
                return tuple_type(types).assert_ok();
            } else {
                return TYPE_Bool;
            }
        } else if (is_returning_value(ft->return_type)) {
            return arguments_to_tuple(ft->return_type);
        } else {
            return empty_arguments_type();
        }
    }

    struct TryInfo {
        LLVMBasicBlockRef bb_except;
        LLVMValueRefs except_values;

        TryInfo() :
            bb_except(nullptr)
        {}

        void clear() {
            bb_except = nullptr;
            except_values.clear();
        }
    };

    TryInfo try_info;

    struct LoopInfo {
        LoopLabelRef loop;
        LLVMBasicBlockRef bb_loop;
        LLVMValueRefs repeat_values;

        LoopInfo() :
            bb_loop(nullptr)
        {}
    };

    LoopInfo loop_info;

    struct LabelInfo {
        LabelRef label;
        LLVMBasicBlockRef bb_merge;
        LLVMValueRefs merge_values;

        LabelInfo() :
            bb_merge(nullptr)
        {}
    };

    std::vector<LabelInfo> label_info_stack;

    template<unsigned N>
    static LLVMAttributeRef get_attribute(const char (&s)[N]) {
        unsigned kind = LLVMGetEnumAttributeKindForName(s, N - 1);
        assert(kind);
        return LLVMCreateEnumAttribute(LLVMGetGlobalContext(), kind, 0);
    }

    LLVMIRGenerator() {
        static_init();
        for (int i = 0; i < NumIntrinsics; ++i) {
            intrinsics[i] = nullptr;
        }
    }

    LLVMMetadataRef source_file_to_scope(Symbol sf) {
        assert(use_debug_info);

        auto it = file2value.find(sf);
        if (it != file2value.end())
            return it->second;

        static char _fname[PATH_MAX];
        static char _dname[PATH_MAX];
        auto str = sf.name();
        strncpy(_fname, str->data, PATH_MAX);
        strncpy(_dname, str->data, PATH_MAX);

        char *fname = basename(_fname);
        char *dname = dirname(_dname);

        LLVMMetadataRef result = LLVMDIBuilderCreateFile(di_builder,
            fname, strlen(fname), dname, strlen(dname));

        file2value.insert({ sf, result });

        return result;
    }

    LLVMMetadataRef function_to_subprogram(const FunctionRef &l) {
        assert(use_debug_info);

        auto it = func2md.find(l.unref());
        if (it != func2md.end())
            return it->second;

        const Anchor *anchor = l.anchor();

        LLVMMetadataRef difile = source_file_to_scope(anchor->path);

        /*
        LLVMMetadataRef subroutinevalues[] = {
            nullptr
        };
        */
        LLVMMetadataRef disrt = LLVMDIBuilderCreateSubroutineType(di_builder,
            difile, nullptr, 0, LLVMDIFlagZero);

        auto name = l->name.name();
        LLVMMetadataRef difunc = LLVMDIBuilderCreateFunction(
            di_builder, difile, name->data, name->count,
            // todo: insert actual linkage name here
            name->data, name->count,
            difile, anchor->lineno, disrt, false, true,
            anchor->lineno, LLVMDIFlagZero, false);

        func2md.insert({ l.unref(), difunc });
        return difunc;
    }

    static void diag_handler(LLVMDiagnosticInfoRef info, void *) {
        const char *severity = "Message";
        switch(LLVMGetDiagInfoSeverity(info)) {
        case LLVMDSError: severity = "Error"; break;
        case LLVMDSWarning: severity = "Warning"; break;
        case LLVMDSRemark: return;// severity = "Remark"; break;
        case LLVMDSNote: return;//severity = "Note"; break;
        default: break;
        }

        char *str = LLVMGetDiagInfoDescription(info);
        fprintf(stderr, "LLVM %s: %s\n", severity, str);
        LLVMDisposeMessage(str);
        //LLVMDiagnosticSeverity LLVMGetDiagInfoSeverity(LLVMDiagnosticInfoRef DI);
    }

    LLVMValueRef get_intrinsic(PMIntrinsic op, LLVMTypeRef T) {
        auto key = PMIntrinsicKey(op, T);
        auto it = pm_intrinsics.find(key);
        if (it != pm_intrinsics.end())
            return it->second;
        const char *prefix = nullptr;
        int argcount = 0;
        switch(op) {
#define LLVM_PM_INTRINSIC(NAME, ARGC) \
    case llvm_ ## NAME: prefix = #NAME; argcount = ARGC; break;
        LLVM_PM_INTRINSIC(bitreverse, 1)
        LLVM_PM_INTRINSIC(ctpop, 1)
        LLVM_PM_INTRINSIC(ctlz, 2)
        LLVM_PM_INTRINSIC(cttz, 2)
#undef LLVM_PM_INTRINSIC
        default: assert(false); break;
        }
        char strname[256];
        if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
            int count = LLVMGetVectorSize(T);
            auto ET = LLVMGetElementType(T);
            assert(LLVMGetTypeKind(ET) == LLVMIntegerTypeKind);
            int width = LLVMGetIntTypeWidth(ET);
            snprintf(strname, 255, "llvm.%s.v%ii%i", prefix, count, width);
        } else {
            assert(LLVMGetTypeKind(T) == LLVMIntegerTypeKind);
            int width = LLVMGetIntTypeWidth(T);
            snprintf(strname, 255, "llvm.%s.i%i", prefix, width);
        }
        LLVMTypeRef argtypes[] = { T, i1T };
        LLVMValueRef result = LLVMAddFunction(module, strname,
            LLVMFunctionType(T, argtypes, argcount, false));
        pm_intrinsics.insert({key, result});
        return result;
    }

    LLVMValueRef get_intrinsic(Intrinsic op) {
        if (!intrinsics[op]) {
            LLVMValueRef result = nullptr;
            switch(op) {
#define LLVM_INTRINSIC_IMPL(ENUMVAL, RETTYPE, STRNAME, ...) \
    case ENUMVAL: { \
        LLVMTypeRef argtypes[] = {__VA_ARGS__}; \
        result = LLVMAddFunction(module, STRNAME, LLVMFunctionType(RETTYPE, argtypes, sizeof(argtypes) / sizeof(LLVMTypeRef), false)); \
    } break;

#define LLVM_INTRINSIC_IMPL_BEGIN(ENUMVAL, RETTYPE, STRNAME, ...) \
    case ENUMVAL: { \
        LLVMTypeRef argtypes[] = { __VA_ARGS__ }; \
        result = LLVMAddFunction(module, STRNAME, \
            LLVMFunctionType(f32T, argtypes, sizeof(argtypes) / sizeof(LLVMTypeRef), false)); \
        LLVMSetLinkage(result, LLVMPrivateLinkage); \
        auto bb = LLVMAppendBasicBlock(result, ""); \
        auto oldbb = LLVMGetInsertBlock(builder); \
        position_builder_at_end(bb);

#define LLVM_INTRINSIC_IMPL_END() \
        position_builder_at_end(oldbb); \
    } break;

            LLVM_INTRINSIC_IMPL(llvm_sin_f32, f32T, "llvm.sin.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_sin_f64, f64T, "llvm.sin.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_cos_f32, f32T, "llvm.cos.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_cos_f64, f64T, "llvm.cos.f64", f64T)

            LLVM_INTRINSIC_IMPL(llvm_sqrt_f32, f32T, "llvm.sqrt.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_sqrt_f64, f64T, "llvm.sqrt.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_fabs_f32, f32T, "llvm.fabs.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_fabs_f64, f64T, "llvm.fabs.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_trunc_f32, f32T, "llvm.trunc.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_trunc_f64, f64T, "llvm.trunc.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_floor_f32, f32T, "llvm.floor.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_floor_f64, f64T, "llvm.floor.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_pow_f32, f32T, "llvm.pow.f32", f32T, f32T)
            LLVM_INTRINSIC_IMPL(llvm_pow_f64, f64T, "llvm.pow.f64", f64T, f64T)
            LLVM_INTRINSIC_IMPL(llvm_exp_f32, f32T, "llvm.exp.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_exp_f64, f64T, "llvm.exp.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_log_f32, f32T, "llvm.log.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_log_f64, f64T, "llvm.log.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_exp2_f32, f32T, "llvm.exp2.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_exp2_f64, f64T, "llvm.exp2.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_log2_f32, f32T, "llvm.log2.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_log2_f64, f64T, "llvm.log2.f64", f64T)

            LLVM_INTRINSIC_IMPL(libc_tan_f32, f32T, "tanf", f32T)
            LLVM_INTRINSIC_IMPL(libc_tan_f64, f64T, "tan", f64T)
            LLVM_INTRINSIC_IMPL(libc_asin_f32, f32T, "asinf", f32T)
            LLVM_INTRINSIC_IMPL(libc_asin_f64, f64T, "asin", f64T)
            LLVM_INTRINSIC_IMPL(libc_acos_f32, f32T, "acosf", f32T)
            LLVM_INTRINSIC_IMPL(libc_acos_f64, f64T, "acos", f64T)
            LLVM_INTRINSIC_IMPL(libc_atan_f32, f32T, "atanf", f32T)
            LLVM_INTRINSIC_IMPL(libc_atan_f64, f64T, "atan", f64T)
            LLVM_INTRINSIC_IMPL(libc_atan2_f32, f32T, "atan2f", f32T, f32T)
            LLVM_INTRINSIC_IMPL(libc_atan2_f64, f64T, "atan2", f64T, f64T)
            LLVM_INTRINSIC_IMPL(libc_sinh_f32, f32T, "sinhf", f32T)
            LLVM_INTRINSIC_IMPL(libc_sinh_f64, f64T, "sinh", f64T)
            LLVM_INTRINSIC_IMPL(libc_cosh_f32, f32T, "coshf", f32T)
            LLVM_INTRINSIC_IMPL(libc_cosh_f64, f64T, "cosh", f64T)
            LLVM_INTRINSIC_IMPL(libc_tanh_f32, f32T, "tanhf", f32T)
            LLVM_INTRINSIC_IMPL(libc_tanh_f64, f64T, "tanh", f64T)
            LLVM_INTRINSIC_IMPL(libc_asinh_f32, f32T, "asinhf", f32T)
            LLVM_INTRINSIC_IMPL(libc_asinh_f64, f64T, "asinh", f64T)
            LLVM_INTRINSIC_IMPL(libc_acosh_f32, f32T, "acoshf", f32T)
            LLVM_INTRINSIC_IMPL(libc_acosh_f64, f64T, "acosh", f64T)
            LLVM_INTRINSIC_IMPL(libc_atanh_f32, f32T, "atanhf", f32T)
            LLVM_INTRINSIC_IMPL(libc_atanh_f64, f64T, "atanh", f64T)

            LLVM_INTRINSIC_IMPL_BEGIN(custom_fsign_f32, f32T, "custom.fsign.f32", f32T)
                // (0 < val) - (val < 0)
                LLVMValueRef val = LLVMGetParam(result, 0);
                LLVMValueRef zero = LLVMConstReal(f32T, 0.0);
                LLVMValueRef a = LLVMBuildZExt(builder, LLVMBuildFCmp(builder, LLVMRealOLT, zero, val, ""), i8T, "");
                LLVMValueRef b = LLVMBuildZExt(builder, LLVMBuildFCmp(builder, LLVMRealOLT, val, zero, ""), i8T, "");
                val = LLVMBuildSub(builder, a, b, "");
                val = LLVMBuildSIToFP(builder, val, f32T, "");
                LLVMBuildRet(builder, val);
            LLVM_INTRINSIC_IMPL_END()
            LLVM_INTRINSIC_IMPL_BEGIN(custom_fsign_f64, f64T, "custom.fsign.f64", f64T)
                // (0 < val) - (val < 0)
                LLVMValueRef val = LLVMGetParam(result, 0);
                LLVMValueRef zero = LLVMConstReal(f64T, 0.0);
                LLVMValueRef a = LLVMBuildZExt(builder, LLVMBuildFCmp(builder, LLVMRealOLT, zero, val, ""), i8T, "");
                LLVMValueRef b = LLVMBuildZExt(builder, LLVMBuildFCmp(builder, LLVMRealOLT, val, zero, ""), i8T, "");
                val = LLVMBuildSub(builder, a, b, "");
                val = LLVMBuildSIToFP(builder, val, f64T, "");
                LLVMBuildRet(builder, val);
            LLVM_INTRINSIC_IMPL_END()
            LLVM_INTRINSIC_IMPL_BEGIN(custom_radians_f32, f32T, "custom.radians.f32", f32T)
                LLVMBuildRet(builder, LLVMBuildFMul(builder,
                    LLVMGetParam(result, 0), LLVMConstReal(f32T, deg2rad), ""));
            LLVM_INTRINSIC_IMPL_END()
            LLVM_INTRINSIC_IMPL_BEGIN(custom_radians_f64, f64T, "custom.radians.f64", f64T)
                LLVMBuildRet(builder, LLVMBuildFMul(builder,
                    LLVMGetParam(result, 0), LLVMConstReal(f64T, deg2rad), ""));
            LLVM_INTRINSIC_IMPL_END()
            LLVM_INTRINSIC_IMPL_BEGIN(custom_degrees_f32, f32T, "custom.degrees.f32", f32T)
                LLVMBuildRet(builder, LLVMBuildFMul(builder,
                    LLVMGetParam(result, 0), LLVMConstReal(f32T, rad2deg), ""));
            LLVM_INTRINSIC_IMPL_END()
            LLVM_INTRINSIC_IMPL_BEGIN(custom_degrees_f64, f64T, "custom.degrees.f64", f64T)
                LLVMBuildRet(builder, LLVMBuildFMul(builder,
                    LLVMGetParam(result, 0), LLVMConstReal(f64T, rad2deg), ""));
            LLVM_INTRINSIC_IMPL_END()
#undef LLVM_INTRINSIC_IMPL
#undef LLVM_INTRINSIC_IMPL_BEGIN
#undef LLVM_INTRINSIC_IMPL_END
            default: assert(false); break;
            }
            intrinsics[op] = result;
        }
        return intrinsics[op];
    }

    static void static_init() {
        if (voidT) return;
        voidT = LLVMVoidType();
        i1T = LLVMInt1Type();
        i8T = LLVMInt8Type();
        i16T = LLVMInt16Type();
        i32T = LLVMInt32Type();
        i64T = LLVMInt64Type();
        f32T = LLVMFloatType();
        f32x2T = LLVMVectorType(f32T, 2);
        f64T = LLVMDoubleType();
        f80T = LLVMX86FP80Type();
        f128T = LLVMFP128Type();
        noneV = LLVMConstStruct(nullptr, 0, false);
        noneT = LLVMTypeOf(noneV);
        rawstringT = LLVMPointerType(LLVMInt8Type(), 0);
        falseV = LLVMConstInt(i1T, 0, false);
        trueV = LLVMConstInt(i1T, 1, false);
        attr_byval = get_attribute("byval");
        attr_sret = get_attribute("sret");
        attr_nonnull = get_attribute("nonnull");

        LLVMContextSetDiagnosticHandler(LLVMGetGlobalContext(),
            diag_handler,
            nullptr);

    }

    static LLVMTypeRef abi_struct_type(const ABIClass *classes, size_t sz) {
        LLVMTypeRef types[sz];
        size_t k = 0;
        for (size_t i = 0; i < sz; ++i) {
            ABIClass cls = classes[i];
            switch(cls) {
            case ABI_CLASS_SSE: {
                types[i] = f32x2T; k++;
            } break;
            case ABI_CLASS_SSESF: {
                types[i] = f32T; k++;
            } break;
            case ABI_CLASS_SSEDF: {
                types[i] = f64T; k++;
            } break;
            case ABI_CLASS_INTEGER: {
                types[i] = i64T; k++;
            } break;
            case ABI_CLASS_INTEGERSI: {
                types[i] = i32T; k++;
            } break;
            case ABI_CLASS_INTEGERSI16: {
                types[i] = i16T; k++;
            } break;
            case ABI_CLASS_INTEGERSI8: {
                types[i] = i8T; k++;
            } break;
            default: {
                // do nothing
#if 0
                StyledStream ss;
                ss << "unhandled ABI class: " <<
                    abi_class_to_string(cls) << std::endl;
#endif
            } break;
            }
        }
        if (k != sz) return nullptr;
        return LLVMStructType(types, sz, false);
    }

    SCOPES_RESULT(LLVMValueRef) abi_import_argument(const Type *param_type, LLVMValueRef func, size_t &k) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        ABIClass classes[MAX_ABI_CLASSES];
        size_t sz = abi_classify(param_type, classes);
        if (!sz) {
            LLVMAddAttributeAtIndex(func, (k + 1), attr_byval);
            LLVMValueRef val = LLVMGetParam(func, k++);
            return LLVMBuildLoad(builder, val, "");
        }
        LLVMTypeRef T = SCOPES_GET_RESULT(type_to_llvm_type(param_type));
        auto tk = LLVMGetTypeKind(T);
        if (tk == LLVMStructTypeKind) {
            auto ST = abi_struct_type(classes, sz);
            if (ST) {
                // reassemble from argument-sized bits
                auto ptr = safe_alloca(ST);
                auto zero = LLVMConstInt(i32T,0,false);
                for (size_t i = 0; i < sz; ++i) {
                    LLVMValueRef indices[] = {
                        zero, LLVMConstInt(i32T,i,false),
                    };
                    auto dest = LLVMBuildGEP(builder, ptr, indices, 2, "");
                    auto param = LLVMGetParam(func, k++);
                    //LLVMBuildStore(builder, param, fix_named_struct_store(param, dest));
                    LLVMBuildStore(builder, param, dest);
                }
                ptr = LLVMBuildBitCast(builder, ptr, LLVMPointerType(T, 0), "");
                return LLVMBuildLoad(builder, ptr, "");
            }
        }
        LLVMValueRef val = LLVMGetParam(func, k++);
        assert(val);
        return val;
    }

    SCOPES_RESULT(void) abi_export_argument(LLVMValueRef val, const Type *AT,
        LLVMValueRefs &values, std::vector<size_t> &memptrs) {
        SCOPES_RESULT_TYPE(void);
        ABIClass classes[MAX_ABI_CLASSES];
        size_t sz = abi_classify(AT, classes);
        if (!sz) {
            LLVMValueRef ptrval = safe_alloca(SCOPES_GET_RESULT(type_to_llvm_type(AT)));
            LLVMBuildStore(builder, val, fix_named_struct_store(val, ptrval));
            //LLVMBuildStore(builder, val, ptrval);
            val = ptrval;
            memptrs.push_back(values.size());
            values.push_back(val);
            return {};
        }
        auto tk = LLVMGetTypeKind(LLVMTypeOf(val));
        if (tk == LLVMStructTypeKind) {
            auto ST = abi_struct_type(classes, sz);
            if (ST) {
                // break into argument-sized bits
                auto ptr = safe_alloca(LLVMTypeOf(val));
                auto zero = LLVMConstInt(i32T,0,false);
                LLVMBuildStore(builder, val, ptr);
                ptr = LLVMBuildBitCast(builder, ptr, LLVMPointerType(ST, 0), "");
                for (size_t i = 0; i < sz; ++i) {
                    LLVMValueRef indices[] = {
                        zero, LLVMConstInt(i32T,i,false),
                    };
                    auto val = LLVMBuildGEP(builder, ptr, indices, 2, "");
                    val = LLVMBuildLoad(builder, val, "");
                    values.push_back(val);
                }
                return {};
            }
        }
        values.push_back(val);
        return {};
    }

    static SCOPES_RESULT(void) abi_transform_parameter(const Type *AT,
        LLVMTypeRefs &params) {
        SCOPES_RESULT_TYPE(void);
        ABIClass classes[MAX_ABI_CLASSES];
        size_t sz = abi_classify(AT, classes);
        auto T = SCOPES_GET_RESULT(type_to_llvm_type(AT));
        if (!sz) {
            params.push_back(LLVMPointerType(T, 0));
            return {};
        }
        auto tk = LLVMGetTypeKind(T);
        if (tk == LLVMStructTypeKind) {
            auto ST = abi_struct_type(classes, sz);
            if (ST) {
                for (size_t i = 0; i < sz; ++i) {
                    params.push_back(LLVMStructGetTypeAtIndex(ST, i));
                }
                return {};
            }
        }
        params.push_back(T);
        return {};
    }

    static SCOPES_RESULT(LLVMTypeRef) create_llvm_type(const Type *type) {
        SCOPES_RESULT_TYPE(LLVMTypeRef);
        switch(type->kind()) {
        case TK_Qualify: {
            auto qt = cast<QualifyType>(type);
            auto lltype = SCOPES_GET_RESULT(_type_to_llvm_type(qt->type));
            auto rq = try_qualifier<ReferQualifier>(type);
            if (rq) {
                lltype = LLVMPointerType(lltype, 0);
            }
            return lltype;
        } break;
        case TK_Integer:
            return LLVMIntType(cast<IntegerType>(type)->width);
        case TK_Real:
            switch(cast<RealType>(type)->width) {
            case 32: return f32T;
            case 64: return f64T;
            case 80: return f80T;
            case 128: return f128T;
            default: break;
            }
            break;
        case TK_Pointer:
            return LLVMPointerType(
                SCOPES_GET_RESULT(_type_to_llvm_type(cast<PointerType>(type)->element_type)), 0);
        case TK_Array:
        case TK_Matrix: {
            auto ai = cast<ArrayLikeType>(type);
            return LLVMArrayType(SCOPES_GET_RESULT(_type_to_llvm_type(ai->element_type)), ai->count());
        } break;
        case TK_Vector: {
            auto vi = cast<ArrayLikeType>(type);
            return LLVMVectorType(SCOPES_GET_RESULT(_type_to_llvm_type(vi->element_type)), vi->count());
        } break;
        case TK_Arguments: {
            if (type == empty_arguments_type())
                return LLVMVoidType();
            return create_llvm_type(cast<ArgumentsType>(type)->to_tuple_type());
        } break;
        case TK_Tuple: {
            auto ti = cast<TupleType>(type);
            size_t count = ti->values.size();
            LLVMTypeRef elements[count];
            for (size_t i = 0; i < count; ++i) {
                elements[i] = SCOPES_GET_RESULT(_type_to_llvm_type(ti->values[i]));
            }
            return LLVMStructType(elements, count, ti->packed);
        } break;
        case TK_Typename: {
            if (type == TYPE_Sampler) {
                SCOPES_ERROR(CGenTypeUnsupportedInTarget, TYPE_Sampler);
            }
            auto tn = cast<TypenameType>(type);
            if (!tn->is_opaque()) {
                switch(tn->storage()->kind()) {
                case TK_Tuple: {
                    type_todo.push_back(type);
                } break;
                default: {
                    return create_llvm_type(tn->storage());
                } break;
                }
            }
            return LLVMStructCreateNamed(
                LLVMGetGlobalContext(), tn->name()->data);
        } break;
        case TK_Function: {
            auto fi = cast<FunctionType>(type);
            size_t count = fi->argument_types.size();
            auto rtype = abi_return_type(fi);
            bool use_sret = is_memory_class(rtype);

            LLVMTypeRefs elements;
            elements.reserve(count);
            LLVMTypeRef rettype;
            if (use_sret) {
                elements.push_back(
                    LLVMPointerType(SCOPES_GET_RESULT(_type_to_llvm_type(rtype)), 0));
                rettype = voidT;
            } else {
                ABIClass classes[MAX_ABI_CLASSES];
                size_t sz = abi_classify(rtype, classes);
                LLVMTypeRef T = SCOPES_GET_RESULT(_type_to_llvm_type(rtype));
                rettype = T;
                if (sz) {
                    auto tk = LLVMGetTypeKind(T);
                    if (tk == LLVMStructTypeKind) {
                        auto ST = abi_struct_type(classes, sz);
                        if (ST) { rettype = ST; }
                    }
                }
            }
            for (size_t i = 0; i < count; ++i) {
                auto AT = fi->argument_types[i];
                SCOPES_CHECK_RESULT(abi_transform_parameter(AT, elements));
            }
            return LLVMFunctionType(rettype,
                &elements[0], elements.size(), fi->vararg());
        } break;
        case TK_SampledImage: {
            SCOPES_ERROR(CGenTypeUnsupportedInTarget, TYPE_SampledImage);
        } break;
        case TK_Image: {
            SCOPES_ERROR(CGenTypeUnsupportedInTarget, TYPE_Image);
        } break;
        case TK_Sampler: {
            SCOPES_ERROR(CGenTypeUnsupportedInTarget, TYPE_Sampler);
        } break;
        default: break;
        };

        SCOPES_ERROR(CGenFailedToTranslateType, type);
    }

    static SCOPES_RESULT(size_t) finalize_types() {
        SCOPES_RESULT_TYPE(size_t);
        size_t result = type_todo.size();
        while (!type_todo.empty()) {
            const Type *T = type_todo.back();
            type_todo.pop_back();
            auto tn = cast<TypenameType>(T);
            if (tn->is_opaque())
                continue;
            LLVMTypeRef LLT = SCOPES_GET_RESULT(_type_to_llvm_type(T));
            const Type *ST = tn->storage();
            switch(ST->kind()) {
            case TK_Tuple: {
                auto ti = cast<TupleType>(ST);
                size_t count = ti->values.size();
                LLVMTypeRef elements[count];
                for (size_t i = 0; i < count; ++i) {
                    elements[i] = SCOPES_GET_RESULT(_type_to_llvm_type(ti->values[i]));
                }
                LLVMStructSetBody(LLT, elements, count, false);
            } break;
            default: assert(false); break;
            }
        }
        return result;
    }

    static SCOPES_RESULT(LLVMTypeRef) _type_to_llvm_type(const Type *type) {
        SCOPES_RESULT_TYPE(LLVMTypeRef);
        auto it = type_cache.find(type);
        if (it == type_cache.end()) {
            LLVMTypeRef result = SCOPES_GET_RESULT(create_llvm_type(type));
            type_cache.insert({type, result});
            return result;
        } else {
            return it->second;
        }
    }

    static SCOPES_RESULT(LLVMTypeRef) type_to_llvm_type(const Type *type) {
        SCOPES_RESULT_TYPE(LLVMTypeRef);
        auto typeref = SCOPES_GET_RESULT(_type_to_llvm_type(type));
        SCOPES_CHECK_RESULT(finalize_types());
        return typeref;
    }

    static Error *last_llvm_error;
    static void fatal_error_handler(const char *Reason) {
        last_llvm_error = ErrorCGenBackendFailed::from(strdup(Reason));
    }

    void bind(const ValueIndex &node, LLVMValueRef value) {
        assert(value);
        ref2value.insert({node, value});
    }

    SCOPES_RESULT(LLVMValueRef) write_return(const LLVMValueRefs &values, bool is_except = false) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        assert(active_function);
        auto fi = extract_function_type(active_function->get_type());
        auto rtype = abi_return_type(fi);
        auto abiretT = SCOPES_GET_RESULT(type_to_llvm_type(rtype));
        LLVMValueRef value = nullptr;
        if (fi->has_exception()) {
            bool has_except_value = is_returning_value(fi->except_type);
            bool has_return_value = is_returning_value(fi->return_type);

            if (has_except_value || has_return_value) {
                auto abivalue = LLVMGetUndef(abiretT);
                if (is_except) {
                    abivalue = LLVMBuildInsertValue(builder, abivalue, falseV, 0, "");
                    if (is_returning_value(fi->except_type)) {
                        auto exceptT = SCOPES_GET_RESULT(type_to_llvm_type(fi->except_type));
                        abivalue = LLVMBuildInsertValue(builder, abivalue,
                            values_to_struct(exceptT, values), 1, "");
                    }
                } else {
                    abivalue = LLVMBuildInsertValue(builder, abivalue, trueV, 0, "");
                    if (is_returning_value(fi->return_type)) {
                        int retvalueindex = (has_except_value?2:1);
                        auto returnT = SCOPES_GET_RESULT(type_to_llvm_type(fi->return_type));
                        abivalue = LLVMBuildInsertValue(builder, abivalue,
                            values_to_struct(returnT, values), retvalueindex, "");
                    }
                }
                value = abivalue;
            } else {
                if (is_except) {
                    value = falseV;
                } else {
                    value = trueV;
                }
            }
        } else {
            if (is_returning_value(fi->return_type)) {
                auto returnT = SCOPES_GET_RESULT(type_to_llvm_type(fi->return_type));
                value = values_to_struct(returnT, values);
            }
        }
        LLVMValueRef parentfunc = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
        bool use_sret = is_memory_class(rtype);
        if (use_sret) {
            auto ptr = LLVMGetParam(parentfunc, 0);
            ptr = fix_named_struct_store(value, ptr);
            LLVMBuildStore(builder, value, ptr);
            return LLVMBuildRetVoid(builder);
        } else if (rtype == empty_arguments_type()) {
            return LLVMBuildRetVoid(builder);
        } else {
            assert(value);
            // check if ABI needs something else and do a bitcast
            auto retT = LLVMGetReturnType(LLVMGetElementType(LLVMTypeOf(parentfunc)));
            auto srcT = abiretT;
            if (retT != srcT) {
                LLVMValueRef dest = safe_alloca(srcT);
                LLVMBuildStore(builder, value, dest);
                value = LLVMBuildBitCast(builder, dest, LLVMPointerType(retT, 0), "");
                value = LLVMBuildLoad(builder, value, "");
            }
            return LLVMBuildRet(builder, value);
        }
    }

    SCOPES_RESULT(LLVMValueRef) write_return(const TypedValues &values, bool is_except = false) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        LLVMValueRefs refs;
        for (auto val : values) {
            refs.push_back(SCOPES_GET_RESULT(ref_to_value(val)));
        }
        return write_return(refs, is_except);
    }

    SCOPES_RESULT(void) translate_Return(const ReturnRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_CHECK_RESULT(write_return(node->values));
        return {};
    }

    SCOPES_RESULT(void) translate_Raise(const RaiseRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_CHECK_RESULT(build_merge_phi(try_info.except_values, node->values));
        LLVMBuildBr(builder, try_info.bb_except);
        return {};
    }

    SCOPES_RESULT(void) Function_finalize(const FunctionRef &node) {
        SCOPES_RESULT_TYPE(void);

        active_function = node;
        auto it = ref2value.find(ValueIndex(node));
        assert(it != ref2value.end());
        LLVMValueRef func = it->second;
        assert(func);
        auto ilfunctype = node->get_type();
        auto fi = extract_function_type(ilfunctype);
        auto rtype = abi_return_type(fi);
        bool use_sret = is_memory_class(rtype);
        auto bb = LLVMAppendBasicBlock(func, "");

        try_info.clear();

        if (!node->raises.empty()) {
            try_info.bb_except = LLVMAppendBasicBlock(func, "except");
            position_builder_at_end(try_info.bb_except);
            if (use_debug_info)
                set_debug_location(node.anchor());
            SCOPES_CHECK_RESULT(build_phi(try_info.except_values, fi->except_type));
            SCOPES_CHECK_RESULT(write_return(try_info.except_values, true));
        }

        position_builder_at_end(bb);
        if ((node == entry_function) && constructor_function) {
            LLVMBuildCall(builder, constructor_function, nullptr, 0, "");
        }

        auto &&params = node->params;
        size_t offset = 0;
        if (use_sret) {
            offset++;
            //Parameter *param = params[0];
            //bind(param, LLVMGetParam(func, 0));
        }

        size_t paramcount = params.size();

        if (use_debug_info)
            set_debug_location(node.anchor());
        size_t k = offset;
        for (size_t i = 0; i < paramcount; ++i) {
            ParameterRef param = params[i];
            LLVMValueRef val = SCOPES_GET_RESULT(abi_import_argument(param->get_type(), func, k));
            assert(val);
            bind(ValueIndex(param), val);
        }
        SCOPES_CHECK_RESULT(translate_block(node->body));
        return {};
    }

    SCOPES_RESULT(LLVMValueRef) Function_to_value(const FunctionRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);

        auto ilfunctype = extract_function_type(node->get_type());

        bool is_export = false;
        std::string name;
        bool is_external = false;

        if (generate_object) {
            auto it = func_export_table.find(node.unref());
            if (it != func_export_table.end()) {
                auto str = it->second.name();
                name = std::string(str->data, str->count);
                is_export = true;
            }
        } else {
            auto it = func_cache.find(node.unref());
            if (it == func_cache.end()) {
                auto funcname = node->name;
                StyledString ss = StyledString::plain();
                if (funcname == SYM_Unnamed) {
                    ss.out << "unnamed";
                } else {
                    ss.out << funcname.name()->data;
                }

                ss.out << "<";
                int index = 0;
                for (auto T : ilfunctype->argument_types) {
                    if (index > 0)
                        ss.out << ",";
                    stream_type_name(ss.out, T);
                    index++;
                }
                ss.out << ">";
                ss.out << get_func_pointer_id(node.unref());
                name = ss.cppstr();

                func_cache.insert({node.unref(), name});
            } else {
                name = it->second;
                is_external = true;
            }
        }

        auto functype = SCOPES_GET_RESULT(type_to_llvm_type(ilfunctype));

        auto func = LLVMAddFunction(module, name.c_str(), functype);

        if (is_external)
            return func;

        generated_symbols.push_back(func);

        if (use_debug_info) {
            LLVMSetSubprogram(func, function_to_subprogram(node));
        }
        if (is_export) {
            LLVMSetLinkage(func, LLVMExternalLinkage);
            //LLVMSetDLLStorageClass(func, LLVMDLLExportStorageClass);
            LLVMSetVisibility(func, LLVMDefaultVisibility);
        } else if (generate_object) {
            LLVMSetLinkage(func, LLVMPrivateLinkage);
            LLVMSetVisibility(func, LLVMHiddenVisibility);
        }
        function_todo.push_back(node);
        return func;
    }

    SCOPES_RESULT(LLVMValueRef) Parameter_to_value(const ParameterRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        SCOPES_ERROR(CGenUnboundValue, node);
    }

    SCOPES_RESULT(LLVMValueRef) LoopLabelArguments_to_value(const LoopLabelArgumentsRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        SCOPES_ERROR(CGenUnboundValue, node);
    }

    SCOPES_RESULT(LLVMValueRef) Exception_to_value(const ExceptionRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        SCOPES_ERROR(CGenUnboundValue, node);
    }

    SCOPES_RESULT(void) translate_block(const Block &node) {
        SCOPES_RESULT_TYPE(void);
        for (auto entry : node.body) {
            SCOPES_CHECK_RESULT(translate_instruction(entry));
        }
        if (node.terminator) {
            SCOPES_CHECK_RESULT(translate_instruction(node.terminator));
        }
        return {};
    }


    LabelInfo &find_label_info(const LabelRef &label) {
        int i = label_info_stack.size();
        while (i-- > 0) {
            auto &&info = label_info_stack[i];
            if (info.label == label)
                return info;
        }
        assert(false);
        return label_info_stack.back();
    }

    SCOPES_RESULT(void) build_merge_phi(const LLVMValueRefs &phis, const TypedValues &values) {
        SCOPES_RESULT_TYPE(void);
        if (phis.size() != values.size()) {
            StyledStream ss;
            for (auto phi : phis) {
                LLVMDumpValue(phi);
                ss << std::endl;
            }
            for (auto value : values) {
                stream_value(ss, value, StreamValueFormat::singleline());
            }
        }
        assert(phis.size() == values.size());
        LLVMBasicBlockRef bb = LLVMGetInsertBlock(builder);
        assert(bb);
        LLVMBasicBlockRef incobbs[] = { bb };
        int count = phis.size();
        for (int i = 0; i < count; ++i) {
            auto phi = phis[i];
            auto ty = LLVMTypeOf(phi);
            auto llvmval = SCOPES_GET_RESULT(ref_to_value(values[i]));
            llvmval = build_struct_cast(llvmval, ty);
            LLVMValueRef incovals[] = { llvmval };
            LLVMAddIncoming(phi, incovals, incobbs, 1);
        }
        return {};
    }

    SCOPES_RESULT(LLVMValueRef) build_merge(const LabelRef &label, const TypedValues &values) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        auto &&label_info = find_label_info(label);
        SCOPES_CHECK_RESULT(build_merge_phi(label_info.merge_values, values));
        assert(label_info.bb_merge);
        return LLVMBuildBr(builder, label_info.bb_merge);
    }

    SCOPES_RESULT(void) translate_Merge(const MergeRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto label = node->label.cast<Label>();
        SCOPES_CHECK_RESULT(build_merge(label, node->values));
        return {};
    }

    SCOPES_RESULT(void) translate_Repeat(const RepeatRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_CHECK_RESULT(build_merge_phi(loop_info.repeat_values, node->values));
        LLVMBuildBr(builder, loop_info.bb_loop);
        return {};
    }

    SCOPES_RESULT(void) translate_Label(const LabelRef &node) {
        SCOPES_RESULT_TYPE(void);
        LabelInfo label_info;
        label_info.label = node;
        LLVMBasicBlockRef bb = LLVMGetInsertBlock(builder);
        LLVMValueRef func = LLVMGetBasicBlockParent(bb);
        label_info.bb_merge = nullptr;
        auto rtype = node->get_type();
        if (is_returning(rtype)) {
            label_info.bb_merge = LLVMAppendBasicBlock(func, node->name.name()->data);
            position_builder_at_end(label_info.bb_merge);
            SCOPES_CHECK_RESULT(build_phi(label_info.merge_values, node));
        }
        label_info_stack.push_back(label_info);
        {
            position_builder_at_end(bb);
            SCOPES_CHECK_RESULT(translate_block(node->body));
        }
        if (label_info.bb_merge)
            position_builder_at_end(label_info.bb_merge);
        label_info_stack.pop_back();
        return {};
    }

    SCOPES_RESULT(void) build_phi(LLVMValueRefs &refs, const Type *T) {
        SCOPES_RESULT_TYPE(void);
        assert(refs.empty());
        int count = get_argument_count(T);
        for (int i = 0; i < count; ++i) {
            auto argT = SCOPES_GET_RESULT(type_to_llvm_type(get_argument(T, i)));
            auto val = LLVMBuildPhi(builder, argT, "");
            refs.push_back(val);
        }
        return {};
    }

    void map_phi(const LLVMValueRefs &refs, const TypedValueRef &node) {
        for (int i = 0; i < refs.size(); ++i) {
            bind(ValueIndex(node, i), refs[i]);
        }
    }

    SCOPES_RESULT(void) build_phi(LLVMValueRefs &refs, const TypedValueRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_CHECK_RESULT(build_phi(refs, node->get_type()));
        map_phi(refs, node);
        return {};
    }

    SCOPES_RESULT(void) translate_LoopLabel(const LoopLabelRef &node) {
        SCOPES_RESULT_TYPE(void);
        //auto rtype = node->get_type();
        auto old_loop_info = loop_info;
        loop_info.loop = node;
        LLVMBasicBlockRef bb = LLVMGetInsertBlock(builder);
        LLVMValueRef func = LLVMGetBasicBlockParent(bb);
        loop_info.bb_loop = LLVMAppendBasicBlock(func, "loop");
        loop_info.repeat_values.clear();
        position_builder_at_end(loop_info.bb_loop);
        SCOPES_CHECK_RESULT(build_phi(loop_info.repeat_values, node->args));
        position_builder_at_end(bb);
        SCOPES_CHECK_RESULT(build_merge_phi(loop_info.repeat_values, node->init));
        LLVMBuildBr(builder, loop_info.bb_loop);
        position_builder_at_end(loop_info.bb_loop);
        SCOPES_CHECK_RESULT(translate_block(node->body));
        loop_info = old_loop_info;
        return {};
    }

    LLVMValueRef values_to_struct(LLVMTypeRef T, const LLVMValueRefs &values) {
        int count = (int)values.size();
        if (count == 1) {
            return build_struct_cast(values[0], T);
        } else {
            LLVMValueRef value = LLVMGetUndef(T);
            for (int i = 0; i < count; ++i) {
                value = LLVMBuildInsertValue(builder, value,
                    build_struct_cast(values[i], LLVMStructGetTypeAtIndex(T, i)),
                    i, "");
            }
            return value;
        }
    }

    void struct_to_values(LLVMValueRefs &values, const Type *T, LLVMValueRef value) {
        assert(value);
        int count = get_argument_count(T);
        if (count == 1) {
            values.push_back(value);
        } else {
            for (int i = 0; i < count; ++i) {
                values.push_back(LLVMBuildExtractValue(builder, value, i, ""));
            }
        }
    }

    SCOPES_RESULT(LLVMTypeRef) node_to_llvm_type(const ValueRef &node) {
        SCOPES_RESULT_TYPE(LLVMTypeRef);
        return type_to_llvm_type(SCOPES_GET_RESULT(extract_type_constant(node)));
    }

    LLVMValueRef fix_named_struct_store(LLVMValueRef val, LLVMValueRef ptr) {
        // fix struct vs named struct
        auto ET = LLVMGetElementType(LLVMTypeOf(ptr));
        auto ST = LLVMTypeOf(val);
        if (ET != ST) {
            assert(LLVMGetTypeKind(ET) == LLVMStructTypeKind);
            ptr = LLVMBuildBitCast(builder, ptr, LLVMPointerType(ST, 0), "");
        }
        return ptr;
    }

    LLVMValueRef build_struct_cast(LLVMValueRef val, LLVMTypeRef ty) {
        if (LLVMTypeOf(val) != ty) {
            if (LLVMGetTypeKind(ty) == LLVMStructTypeKind) {
                // completely braindead, but what can you do
                LLVMValueRef ptr = safe_alloca(LLVMTypeOf(val));
                LLVMBuildStore(builder, val, ptr);
                ptr = LLVMBuildBitCast(builder, ptr, LLVMPointerType(ty,0), "");
                return LLVMBuildLoad(builder, ptr, "");
            }
        }
        return val;
    }

    SCOPES_RESULT(void) translate_Unreachable(const UnreachableRef &node) {
        LLVMBuildUnreachable(builder);
        return {};
    }

    SCOPES_RESULT(void) translate_Discard(const DiscardRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_ERROR(CGenUnsupportedBuiltin, SFXFN_Discard);
        return {};
    }

    SCOPES_RESULT(void) translate_Call(const CallRef &call) {
        SCOPES_RESULT_TYPE(void);
        auto callee = call->callee;
        auto &&args = call->args;

        const Type *rtype = strip_lifetime(callee->get_type());
        if (!is_function_pointer(rtype)) {
            SCOPES_ERROR(CGenInvalidCallee, callee->get_type());
        }
        SCOPES_CHECK_RESULT(build_call(call,
            extract_function_type(rtype),
            SCOPES_GET_RESULT(ref_to_value(callee)), args));
        return {};
    }

    SCOPES_RESULT(void) translate_Sample(const SampleRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_ERROR(CGenUnsupportedImageOp);
    }

    SCOPES_RESULT(void) translate_ImageQuerySize(const ImageQuerySizeRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_ERROR(CGenUnsupportedImageOp);
    }

    SCOPES_RESULT(void) translate_ImageQueryLod(const ImageQueryLodRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_ERROR(CGenUnsupportedImageOp);
    }

    SCOPES_RESULT(void) translate_ImageQueryLevels(const ImageQueryLevelsRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_ERROR(CGenUnsupportedImageOp);
    }

    SCOPES_RESULT(void) translate_ImageQuerySamples(const ImageQuerySamplesRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_ERROR(CGenUnsupportedImageOp);
    }

    SCOPES_RESULT(void) translate_ImageRead(const ImageReadRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_ERROR(CGenUnsupportedImageOp);
    }

    SCOPES_RESULT(void) translate_ImageWrite(const ImageWriteRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_ERROR(CGenUnsupportedImageOp);
    }

    SCOPES_RESULT(void) translate_ExecutionMode(const ExecutionModeRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_ERROR(CGenUnsupportedImageOp);
    }

    SCOPES_RESULT(void) translate_GetElementPtr(const GetElementPtrRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto pointer = SCOPES_GET_RESULT(ref_to_value(node->value));
        assert(LLVMGetTypeKind(LLVMTypeOf(pointer)) == LLVMPointerTypeKind);
        auto &&src_indices = node->indices;
        size_t count = src_indices.size();
        assert(count);
        LLVMValueRef indices[count];
        for (size_t i = 0; i < count; ++i) {
            indices[i] = SCOPES_GET_RESULT(ref_to_value(src_indices[i]));
            assert(LLVMGetTypeKind(LLVMTypeOf(indices[i])) == LLVMIntegerTypeKind);
        }
        auto val = LLVMBuildGEP(builder, pointer, indices, count, "");
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_ExtractValue(const ExtractValueRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto val = LLVMBuildExtractValue(builder,
            SCOPES_GET_RESULT(ref_to_value(node->value)),
            node->index, "");
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_InsertValue(const InsertValueRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto srcval = SCOPES_GET_RESULT(ref_to_value(node->value));
        auto elm = SCOPES_GET_RESULT(ref_to_value(node->element));
        auto elmtype = LLVMTypeOf(srcval);
        if (LLVMGetTypeKind(elmtype) == LLVMStructTypeKind) {
            elmtype = LLVMStructGetTypeAtIndex(elmtype, node->index);
        } else {
            elmtype = LLVMGetElementType(elmtype);
        }
        elm = build_struct_cast(elm, elmtype);
        auto val = LLVMBuildInsertValue(builder, srcval, elm,
            node->index, "");
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_ExtractElement(const ExtractElementRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto val = LLVMBuildExtractElement(builder,
            SCOPES_GET_RESULT(ref_to_value(node->value)),
            SCOPES_GET_RESULT(ref_to_value(node->index)),
            "");
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_InsertElement(const InsertElementRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto val = LLVMBuildInsertElement(builder,
            SCOPES_GET_RESULT(ref_to_value(node->value)),
            SCOPES_GET_RESULT(ref_to_value(node->element)),
            SCOPES_GET_RESULT(ref_to_value(node->index)),
            "");
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_ShuffleVector(const ShuffleVectorRef &node) {
        SCOPES_RESULT_TYPE(void);

        auto &&mask = node->mask;

        int count = mask.size();
        LLVMValueRef indices[count];
        for (int i = 0; i < count; ++i) {
            indices[i] = LLVMConstInt(i32T, mask[i], false);
        }
        auto maskv = LLVMConstVector(indices, count);
        auto val = LLVMBuildShuffleVector(builder,
            SCOPES_GET_RESULT(ref_to_value(node->v1)),
            SCOPES_GET_RESULT(ref_to_value(node->v2)),
            maskv,
            "");
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_Alloca(const AllocaRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto ty = SCOPES_GET_RESULT(type_to_llvm_type(node->type));
        LLVMValueRef val;
        if (node->is_array()) {
            auto count = SCOPES_GET_RESULT(ref_to_value(node->count));
            val = safe_alloca(ty, count);
        } else {
            val = safe_alloca(ty);
        }
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_Malloc(const MallocRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto ty = SCOPES_GET_RESULT(type_to_llvm_type(node->type));
        LLVMValueRef val;
        if (node->is_array()) {
            auto count = SCOPES_GET_RESULT(ref_to_value(node->count));
            val = LLVMBuildArrayMalloc(builder, ty, count, "");
        } else {
            val = LLVMBuildMalloc(builder, ty, "");
        }
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_Free(const FreeRef &node) {
        SCOPES_RESULT_TYPE(void);
        LLVMBuildFree(builder,
            SCOPES_GET_RESULT(ref_to_value(node->value)));
        return {};
    }

    SCOPES_RESULT(void) translate_Load(const LoadRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto val = LLVMBuildLoad(builder, SCOPES_GET_RESULT(ref_to_value(node->value)), "");
        if (node->is_volatile) {
            LLVMSetVolatile(val, true);
        }
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_Store(const StoreRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto value = SCOPES_GET_RESULT(ref_to_value(node->value));
        auto ptr = SCOPES_GET_RESULT(ref_to_value(node->target));
        ptr = fix_named_struct_store(value, ptr);
        auto val = LLVMBuildStore(builder, value, ptr);
        if (node->is_volatile) {
            LLVMSetVolatile(val, true);
        }
        return {};
    }

    SCOPES_RESULT(void) translate_AtomicRMW(const AtomicRMWRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto ptr = SCOPES_GET_RESULT(ref_to_value(node->target));
        auto value = SCOPES_GET_RESULT(ref_to_value(node->value));
        LLVMAtomicRMWBinOp op;
        switch(node->op) {
        case AtomicRMWOpXchg: op = LLVMAtomicRMWBinOpXchg; break;
        case AtomicRMWOpAdd: op = LLVMAtomicRMWBinOpAdd; break;
        case AtomicRMWOpSub: op = LLVMAtomicRMWBinOpSub; break;
        case AtomicRMWOpAnd: op = LLVMAtomicRMWBinOpAnd; break;
        case AtomicRMWOpNAnd: op = LLVMAtomicRMWBinOpNand; break;
        case AtomicRMWOpOr: op = LLVMAtomicRMWBinOpOr; break;
        case AtomicRMWOpXor: op = LLVMAtomicRMWBinOpXor; break;
        case AtomicRMWOpSMin: op = LLVMAtomicRMWBinOpMin; break;
        case AtomicRMWOpSMax: op = LLVMAtomicRMWBinOpMax; break;
        case AtomicRMWOpUMin: op = LLVMAtomicRMWBinOpUMin; break;
        case AtomicRMWOpUMax: op = LLVMAtomicRMWBinOpUMax; break;
        default: {
            SCOPES_ERROR(CGenUnsupportedAtomicOp);
        } break;
        }
        auto val = LLVMBuildAtomicRMW(builder, op,
            ptr, value,
            LLVMAtomicOrderingSequentiallyConsistent,
            false);
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_CmpXchg(const CmpXchgRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto ptr = SCOPES_GET_RESULT(ref_to_value(node->target));
        auto cmp = SCOPES_GET_RESULT(ref_to_value(node->cmp));
        auto value = SCOPES_GET_RESULT(ref_to_value(node->value));
        auto val = LLVMBuildAtomicCmpXchg(builder,
            ptr, cmp, value,
            LLVMAtomicOrderingSequentiallyConsistent,
            LLVMAtomicOrderingSequentiallyConsistent,
            false);
        auto val0 = LLVMBuildExtractValue(builder, val, 0, "");
        auto val1 = LLVMBuildExtractValue(builder, val, 1, "");
        map_phi({ val0, val1 }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_Barrier(const BarrierRef &node) {
        //SCOPES_RESULT_TYPE(void);
        // just ignore
        return {};
    }

    SCOPES_RESULT(void) translate_Annotate(const AnnotateRef &node) {
        return {};
    }

    SCOPES_RESULT(void) translate_Select(const SelectRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto val =
            LLVMBuildSelect(
                builder,
                SCOPES_GET_RESULT(ref_to_value(node->cond)),
                SCOPES_GET_RESULT(ref_to_value(node->value1)),
                SCOPES_GET_RESULT(ref_to_value(node->value2)),
                "");
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_ICmp(const ICmpRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto a = SCOPES_GET_RESULT(ref_to_value(node->value1));
        auto b = SCOPES_GET_RESULT(ref_to_value(node->value2));
        LLVMIntPredicate pred = LLVMIntEQ;
        switch(node->cmp_kind) {
        case ICmpEQ: pred = LLVMIntEQ; break;
        case ICmpNE: pred = LLVMIntNE; break;
        case ICmpUGT: pred = LLVMIntUGT; break;
        case ICmpUGE: pred = LLVMIntUGE; break;
        case ICmpULT: pred = LLVMIntULT; break;
        case ICmpULE: pred = LLVMIntULE; break;
        case ICmpSGT: pred = LLVMIntSGT; break;
        case ICmpSGE: pred = LLVMIntSGE; break;
        case ICmpSLT: pred = LLVMIntSLT; break;
        case ICmpSLE: pred = LLVMIntSLE; break;
        default: assert(false); break;
        }
        auto val = LLVMBuildICmp(builder, pred, a, b, "");
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_FCmp(const FCmpRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto a = SCOPES_GET_RESULT(ref_to_value(node->value1));
        auto b = SCOPES_GET_RESULT(ref_to_value(node->value2));
        LLVMRealPredicate pred = LLVMRealOEQ;
        switch(node->cmp_kind) {
        case FCmpOEQ: pred = LLVMRealOEQ; break;
        case FCmpONE: pred = LLVMRealONE; break;
        case FCmpORD: pred = LLVMRealORD; break;
        case FCmpOGT: pred = LLVMRealOGT; break;
        case FCmpOGE: pred = LLVMRealOGE; break;
        case FCmpOLT: pred = LLVMRealOLT; break;
        case FCmpOLE: pred = LLVMRealOLE; break;
        case FCmpUEQ: pred = LLVMRealUEQ; break;
        case FCmpUNE: pred = LLVMRealUNE; break;
        case FCmpUNO: pred = LLVMRealUNO; break;
        case FCmpUGT: pred = LLVMRealUGT; break;
        case FCmpUGE: pred = LLVMRealUGE; break;
        case FCmpULT: pred = LLVMRealULT; break;
        case FCmpULE: pred = LLVMRealULE; break;
        default: assert(false); break;
        }
        auto val = LLVMBuildFCmp(builder, pred, a, b, "");
        map_phi({ val }, node);
        return {};
    }

    LLVMValueRef translate_pm_unop(const UnOpRef &node, LLVMValueRef x, PMIntrinsic op) {
        auto T = LLVMTypeOf(x);
        LLVMValueRef func = get_intrinsic(op, T);
        assert(func);
        LLVMValueRef values[] = { x, trueV };
        int valcount = 1;
        switch (op) {
        case llvm_ctlz:
        case llvm_cttz: {
            valcount++;
        } break;
        default: break;
        }
        LLVMValueRef result = LLVMBuildCall(builder, func, values, valcount, "");
        if (op == llvm_ctlz) {
            // emulate FindUMsb
            LLVMValueRef constant;
            if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                auto count = LLVMGetVectorSize(T);
                auto ET = LLVMGetElementType(T);
                assert(LLVMGetTypeKind(ET) == LLVMIntegerTypeKind);
                int width = LLVMGetIntTypeWidth(ET);
                constant = LLVMConstInt(ET, width - 1, false);
                std::vector<LLVMValueRef> comps;
                comps.resize(count, constant);
                constant = LLVMConstVector(&comps[0], count);
            } else {
                assert(LLVMGetTypeKind(T) == LLVMIntegerTypeKind);
                int width = LLVMGetIntTypeWidth(T);
                constant = LLVMConstInt(T, width - 1, false);
            }
            result = LLVMBuildSub(builder, constant, result, "");
        }
        return result;
    }

    SCOPES_RESULT(void) translate_UnOp(const UnOpRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto x = SCOPES_GET_RESULT(ref_to_value(node->value));
        LLVMValueRef val = nullptr;
        switch(node->op) {
        case UnOpLength: {
            auto T = LLVMTypeOf(x);
            if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                val = build_length_op(x);
            } else {
                LLVMValueRef func_fabs = get_intrinsic((T == f64T)?llvm_fabs_f64:llvm_fabs_f32);
                assert(func_fabs);
                LLVMValueRef values[] = { x };
                val = LLVMBuildCall(builder, func_fabs, values, 1, "");
            }
        } break;
        case UnOpNormalize: {
            auto T = LLVMTypeOf(x);
            if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                auto count = LLVMGetVectorSize(T);
                auto ET = LLVMGetElementType(T);
                LLVMValueRef l = build_length_op(x);
                l = LLVMBuildInsertElement(builder,
                    LLVMGetUndef(LLVMVectorType(ET, 1)), l,
                    LLVMConstInt(i32T, 0, false),
                    "");
                LLVMValueRef mask[count];
                for (int i = 0; i < count; ++i) {
                    mask[i] = 0;
                }
                l = LLVMBuildShuffleVector(builder, l, l,
                    LLVMConstNull(LLVMVectorType(i32T, count)), "");
                val = LLVMBuildFDiv(builder, x, l, "");
            } else {
                val = LLVMConstReal(T, 1.0);
            }
        } break;
#define UNOP(SRC, FUNC) \
        case SRC: { val = translate_pm_unop(node, x, FUNC); } break;
        UNOP(UnOpBitReverse, llvm_bitreverse)
        UNOP(UnOpBitCount, llvm_ctpop)
        UNOP(UnOpFindMSB, llvm_ctlz)
        UNOP(UnOpFindLSB, llvm_cttz)
#undef UNOP
        default: {
            auto T = LLVMTypeOf(x);
            auto ET = T;
            if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                ET = LLVMGetElementType(T);
            }
            LLVMValueRef func = nullptr;
            Intrinsic op = NumIntrinsics;
            switch(node->op) {
#define UNOP(SRC, FUNC) \
    case SRC: { op = (ET == f64T)?FUNC ## _f64:FUNC ## _f32; } break;
            UNOP(UnOpSin, llvm_sin)
            UNOP(UnOpCos, llvm_cos)
            UNOP(UnOpTan, libc_tan)
            UNOP(UnOpAsin, libc_asin)
            UNOP(UnOpAcos, libc_acos)
            UNOP(UnOpAtan, libc_atan)
            UNOP(UnOpSinh, libc_sinh)
            UNOP(UnOpCosh, libc_cosh)
            UNOP(UnOpTanh, libc_tanh)
            UNOP(UnOpASinh, libc_asinh)
            UNOP(UnOpACosh, libc_acosh)
            UNOP(UnOpATanh, libc_atanh)
            UNOP(UnOpSqrt, llvm_sqrt)
            UNOP(UnOpFAbs, llvm_fabs)
            UNOP(UnOpTrunc, llvm_trunc)
            UNOP(UnOpFloor, llvm_floor)
            UNOP(UnOpExp, llvm_exp)
            UNOP(UnOpLog, llvm_log)
            UNOP(UnOpExp2, llvm_exp2)
            UNOP(UnOpLog2, llvm_log2)
            UNOP(UnOpFSign, custom_fsign)
            UNOP(UnOpRadians, custom_radians)
            UNOP(UnOpDegrees, custom_degrees)
#undef UNOP
            default:
                SCOPES_ERROR(CGenUnsupportedUnOp);
            }
            func = get_intrinsic(op);
            assert(func);
            if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                auto count = LLVMGetVectorSize(T);
                LLVMValueRef retvalue = LLVMGetUndef(T);
                for (unsigned i = 0; i < count; ++i) {
                    LLVMValueRef idx = LLVMConstInt(i32T, i, false);
                    LLVMValueRef values[] = { LLVMBuildExtractElement(builder, x, idx, "") };
                    LLVMValueRef eltval = LLVMBuildCall(builder, func, values, 1, "");
                    retvalue = LLVMBuildInsertElement(builder, retvalue, eltval, idx, "");
                }
                val = retvalue;
            } else {
                LLVMValueRef values[] = { x };
                val = LLVMBuildCall(builder, func, values, 1, "");
            }
        } break;
        }
        map_phi({ val }, node);
        return {};
    }

    LLVMValueRef build_intrinsic_binop(Intrinsic op, LLVMValueRef a, LLVMValueRef b) {
        auto T = LLVMTypeOf(a);
        LLVMValueRef func = nullptr;
        func = get_intrinsic(op);
        assert(func);
        if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
            auto count = LLVMGetVectorSize(T);
            LLVMValueRef retvalue = LLVMGetUndef(T);
            for (unsigned i = 0; i < count; ++i) {
                LLVMValueRef idx = LLVMConstInt(i32T, i, false);
                LLVMValueRef values[] = {
                    LLVMBuildExtractElement(builder, a, idx, ""),
                    LLVMBuildExtractElement(builder, b, idx, "")
                };
                LLVMValueRef eltval = LLVMBuildCall(builder, func, values, 2, "");
                retvalue = LLVMBuildInsertElement(builder, retvalue, eltval, idx, "");
            }
            return retvalue;
        } else {
            LLVMValueRef values[] = { a, b };
            return LLVMBuildCall(builder, func, values, 2, "");
        }
    }

    SCOPES_RESULT(void) translate_BinOp(const BinOpRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto a = SCOPES_GET_RESULT(ref_to_value(node->value1));
        auto b = SCOPES_GET_RESULT(ref_to_value(node->value2));
        auto T = LLVMTypeOf(a);
        auto ET = T;
        if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
            ET = LLVMGetElementType(T);
        }
        LLVMValueRef val = nullptr;
        switch(node->op) {
#define BINOP(SRC, FUNC) case SRC: val = FUNC(builder, a, b, ""); break;
#define INTRINSIC_BINOP(SRC, FUNC) \
    case SRC: { val = build_intrinsic_binop((ET == f64T)?FUNC ## _f64:FUNC ## _f32, a, b); } break;
        BINOP(BinOpAdd, LLVMBuildAdd)
        BINOP(BinOpAddNUW, LLVMBuildNUWAdd)
        BINOP(BinOpAddNSW, LLVMBuildNSWAdd)
        BINOP(BinOpSub, LLVMBuildSub)
        BINOP(BinOpSubNUW, LLVMBuildNUWSub)
        BINOP(BinOpSubNSW, LLVMBuildNSWSub)
        BINOP(BinOpMul, LLVMBuildMul)
        BINOP(BinOpMulNUW, LLVMBuildNUWMul)
        BINOP(BinOpMulNSW, LLVMBuildNSWMul)
        BINOP(BinOpUDiv, LLVMBuildUDiv)
        BINOP(BinOpSDiv, LLVMBuildSDiv)
        BINOP(BinOpURem, LLVMBuildURem)
        BINOP(BinOpSRem, LLVMBuildSRem)
        BINOP(BinOpShl, LLVMBuildShl)
        BINOP(BinOpLShr, LLVMBuildLShr)
        BINOP(BinOpAShr, LLVMBuildAShr)
        BINOP(BinOpBAnd, LLVMBuildAnd)
        BINOP(BinOpBOr, LLVMBuildOr)
        BINOP(BinOpBXor, LLVMBuildXor)
        BINOP(BinOpFAdd, LLVMBuildFAdd)
        BINOP(BinOpFSub, LLVMBuildFSub)
        BINOP(BinOpFMul, LLVMBuildFMul)
        BINOP(BinOpFDiv, LLVMBuildFDiv)
        BINOP(BinOpFRem, LLVMBuildFRem)
        INTRINSIC_BINOP(BinOpAtan2, libc_atan2)
        INTRINSIC_BINOP(BinOpPow, llvm_pow)
        case BinOpCross: {
            auto T = LLVMTypeOf(a);
            assert (LLVMGetTypeKind(T) == LLVMVectorTypeKind);
            LLVMValueRef i0 = LLVMConstInt(i32T, 0, false);
            LLVMValueRef i1 = LLVMConstInt(i32T, 1, false);
            LLVMValueRef i2 = LLVMConstInt(i32T, 2, false);
            LLVMValueRef i120[] = { i1, i2, i0 };
            LLVMValueRef v120 = LLVMConstVector(i120, 3);
            LLVMValueRef a120 = LLVMBuildShuffleVector(builder, a, a, v120, "");
            LLVMValueRef b120 = LLVMBuildShuffleVector(builder, b, b, v120, "");
            LLVMValueRef retvalue = LLVMBuildFSub(builder,
                LLVMBuildFMul(builder, a, b120, ""),
                LLVMBuildFMul(builder, b, a120, ""), "");
            val = LLVMBuildShuffleVector(builder, retvalue, retvalue, v120, "");
        } break;
        case BinOpStep: {
            LLVMValueRef one = build_matching_constant_real_vector(a, 1.0);
            LLVMValueRef zero = build_matching_constant_real_vector(b, 0.0);
            val = LLVMBuildSelect(builder,
                LLVMBuildFCmp(builder, LLVMRealOGT, a, b, ""),
                zero, one, "");
        } break;
#undef BINOP
        default: {
            SCOPES_ERROR(CGenUnsupportedBinOp);
        } break;
        }

        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_TriOp(const TriOpRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto a = SCOPES_GET_RESULT(ref_to_value(node->value1));
        auto b = SCOPES_GET_RESULT(ref_to_value(node->value2));
        auto c = SCOPES_GET_RESULT(ref_to_value(node->value3));
        LLVMValueRef val = nullptr;
        switch(node->op) {
        case TriOpFMix: {
            LLVMValueRef one = build_matching_constant_real_vector(a, 1.0);
            auto invx = LLVMBuildFSub(builder, one, c, "");
            val = LLVMBuildFAdd(builder,
                LLVMBuildFMul(builder, a, invx, ""),
                LLVMBuildFMul(builder, b, c, ""),
                "");
        } break;
        default: {
            SCOPES_ERROR(CGenUnsupportedTriOp);
        } break;
        }
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_Cast(const CastRef &node) {
        SCOPES_RESULT_TYPE(void);
        LLVMValueRef val = SCOPES_GET_RESULT(ref_to_value(node->value));
        auto ty = SCOPES_GET_RESULT(type_to_llvm_type(node->get_type()));
        auto T = LLVMTypeOf(val);
        if (T != ty) {
            switch(node->op) {
            case CastBitcast: {
                auto newval = build_struct_cast(val, ty);
                if (newval == val) {
                    val = LLVMBuildBitCast(builder, val, ty, "");
                } else {
                    val = newval;
                }
            } break;
            case CastPtrToRef: break;
            case CastRefToPtr: break;
#define CAST_OP(SRC, OP) case SRC: val = OP(builder, val, ty, ""); break;
            CAST_OP(CastIntToPtr, LLVMBuildIntToPtr);
            CAST_OP(CastPtrToInt, LLVMBuildPtrToInt);
            CAST_OP(CastSExt, LLVMBuildSExt);
            CAST_OP(CastITrunc, LLVMBuildTrunc);
            CAST_OP(CastZExt, LLVMBuildZExt);
            CAST_OP(CastFPTrunc, LLVMBuildFPTrunc);
            CAST_OP(CastFPExt, LLVMBuildFPExt);
            CAST_OP(CastFPToUI, LLVMBuildFPToUI);
            CAST_OP(CastFPToSI, LLVMBuildFPToSI);
            CAST_OP(CastUIToFP, LLVMBuildUIToFP);
            CAST_OP(CastSIToFP, LLVMBuildSIToFP);
#undef CAST_OP
            default: {
                SCOPES_ERROR(CGenUnsupportedCastOp);
            } break;
            }
        }
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_Switch(const SwitchRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto expr = SCOPES_GET_RESULT(ref_to_value(node->expr));
        LLVMBasicBlockRef bb = LLVMGetInsertBlock(builder);
        LLVMValueRef func = LLVMGetBasicBlockParent(bb);
        LLVMBasicBlockRef bbdefault = LLVMAppendBasicBlock(func, "default");
        position_builder_at_end(bb);
        int count = (int)node->cases.size();
        assert(count);
        auto _sw = LLVMBuildSwitch(builder, expr, bbdefault, count - 1);
        int i = count;
        LLVMBasicBlockRef lastbb = nullptr;
        while (i-- > 0) {
            auto &_case = *node->cases[i];
            LLVMBasicBlockRef bbcase = nullptr;
            if (_case.kind == CK_Default) {
                position_builder_at_end(bbdefault);
                bbcase = bbdefault;
            } else if (_case.body.empty()) {
                auto lit = SCOPES_GET_RESULT(ref_to_value(ValueIndex(_case.literal)));
                LLVMAddCase(_sw, lit, lastbb);
                continue;
            } else {
                auto lit = SCOPES_GET_RESULT(ref_to_value(ValueIndex(_case.literal)));
                bbcase = LLVMAppendBasicBlock(func, "case");
                position_builder_at_end(bbcase);
                LLVMAddCase(_sw, lit, bbcase);
            }
            SCOPES_CHECK_RESULT(translate_block(_case.body));
            if (!_case.body.terminator) {
                LLVMBuildBr(builder, lastbb);
            }
            lastbb = bbcase;
        }
        return {};
    }

    void position_builder_at_end(LLVMBasicBlockRef bb) {
        //LLVMDumpValue(LLVMBasicBlockAsValue(bb));
        LLVMPositionBuilderAtEnd(builder, bb);
    }

    SCOPES_RESULT(void) translate_CondBr(const CondBrRef &node) {
        SCOPES_RESULT_TYPE(void);
        LLVMBasicBlockRef bb = LLVMGetInsertBlock(builder);
        LLVMValueRef func = LLVMGetBasicBlockParent(bb);
        auto cond = SCOPES_GET_RESULT(ref_to_value(node->cond));
        assert(cond);
        LLVMBasicBlockRef bbthen = LLVMAppendBasicBlock(func, "then");
        LLVMBasicBlockRef bbelse = LLVMAppendBasicBlock(func, "else");
        LLVMBuildCondBr(builder, cond, bbthen, bbelse);

        // write then-block
        {
            position_builder_at_end(bbthen);
            SCOPES_CHECK_RESULT(translate_block(node->then_body));
        }
        // write else-block
        {
            position_builder_at_end(bbelse);
            SCOPES_CHECK_RESULT(translate_block(node->else_body));
        }

        return {};
    }

    SCOPES_RESULT(void) translate_instruction(const InstructionRef &node) {
        switch(node->kind()) {
        #define T(NAME, BNAME, CLASS) \
            case NAME: return translate_ ## CLASS(node.cast<CLASS>());
        SCOPES_INSTRUCTION_VALUE_KIND()
        #undef T
            default: assert(false); break;
        }
        return {};
    }

    SCOPES_RESULT(LLVMValueRef) ref_to_value(const ValueIndex &ref) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        auto it = ref2value.find(ref);
        LLVMValueRef value = nullptr;
        if (it != ref2value.end()) {
            value = it->second;
        } else {
            SCOPES_TRACE_CODEGEN(ref.value);
            switch(ref.value->kind()) {
            #define T(NAME, BNAME, CLASS) \
                case NAME: value = SCOPES_GET_RESULT( \
                    CLASS ## _to_value(ref.value.cast<CLASS>())); break;
            SCOPES_PURE_VALUE_KIND()
            #undef T
                default: {
                    SCOPES_ERROR(CGenFailedToTranslateValue, ref.value->kind());
                } break;
            }
            ref2value.insert({ref,value});
        }
        return value;
    }

    SCOPES_RESULT(LLVMValueRef) PureCast_to_value(const PureCastRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        LLVMTypeRef LLT = SCOPES_GET_RESULT(type_to_llvm_type(node->get_type()));
        auto val = SCOPES_GET_RESULT(ref_to_value(ValueIndex(node->value)));
        return LLVMConstBitCast(val, LLT);
    }

    SCOPES_RESULT(LLVMValueRef) Undef_to_value(const UndefRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        LLVMTypeRef LLT = SCOPES_GET_RESULT(type_to_llvm_type(node->get_type()));
        return LLVMGetUndef(LLT);
    }

    SCOPES_RESULT(LLVMValueRef) GlobalString_to_value(const GlobalStringRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        LLVMTypeRef LLT = SCOPES_GET_RESULT(type_to_llvm_type(node->get_type()));
        //auto ET = LLVMGetElementType(LLT);
        auto data = LLVMConstString(node->value.data(), node->value.size(), true);
        LLVMValueRef result = LLVMAddGlobal(module, LLVMTypeOf(data), "");
        LLVMSetInitializer(result, data);
        LLVMSetGlobalConstant(result, true);
        result = LLVMConstBitCast(result, LLT);
        return result;
    }

    SCOPES_RESULT(LLVMValueRef) Global_to_value(const GlobalRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        auto it = global2global.find(node.unref());
        if (it == global2global.end()) {
            auto pi = cast<PointerType>(node->get_type());
            LLVMTypeRef LLT = SCOPES_GET_RESULT(type_to_llvm_type(pi->element_type));
            LLVMValueRef result = nullptr;
            if (node->storage_class == SYM_SPIRV_StorageClassPrivate) {

                std::string name;
                bool is_external = false;

                if (generate_object) {
                    auto globname = node->name;
                    StyledString ss = StyledString::plain();
                    if (globname == SYM_Unnamed) {
                        ss.out << "unnamed";
                    } else {
                        ss.out << globname.name()->data;
                    }
                    ss.out << "<";
                    stream_type_name(ss.out, pi->element_type);
                    ss.out << ">";
                    stream_address(ss.out, node.unref());
                    name = ss.cppstr();
                } else {
                    auto it = global_cache.find(node.unref());
                    if (it == global_cache.end()) {
                        auto globname = node->name;
                        StyledString ss = StyledString::plain();
                        if (globname == SYM_Unnamed) {
                            ss.out << "unnamed";
                        } else {
                            ss.out << globname.name()->data;
                        }
                        ss.out << "<";
                        stream_type_name(ss.out, pi->element_type);
                        ss.out << ">";
                        ss.out << get_global_pointer_id(node.unref());
                        name = ss.cppstr();
                        global_cache.insert({node.unref(), name});
                    } else {
                        name = it->second;
                        is_external = true;
                    }
                }

                result = LLVMAddGlobal(module, LLT, name.c_str());
                global2global.insert({ node.unref(), result });
                if (!is_external) {

                    LLVMValueRef init = nullptr;

                    if (node->initializer) {
                        init = SCOPES_GET_RESULT(
                            ref_to_value(TypedValueRef(node->initializer)));
                    } else {
                        init = LLVMConstNull(LLT);
                    }
                    LLVMSetInitializer(result, init);

                    if (node->constructor) {
                        constructors.push_back(
                            SCOPES_GET_RESULT(
                                ref_to_value(
                                    TypedValueRef(node->constructor))));
                    }
                }
                return result;
            } else {
                const String *namestr = node->name.name();
                const char *name = namestr->data;
                assert(name);
                if ((namestr->count > 5) && !strncmp(name, "llvm.", 5)) {
                    result = LLVMGetNamedFunction(module, name);
                    if (result) {
                        LLT = LLVMPointerType(LLT, 0);
                        if (LLVMTypeOf(result) != LLT) {
                            result = LLVMConstBitCast(result, LLT);
                            //LLVMDumpValue(result);
                            //SCOPES_ERROR(CGenInvalidRedeclaration, name);
                        }
                    } else {
                        result = LLVMAddFunction(module, name, LLT);
                    }
                } else {
                    result = LLVMGetNamedGlobal(module, name);
                    if (result) {
                        LLT = LLVMPointerType(LLT, 0);
                        if (LLVMTypeOf(result) != LLT) {
                            //result = LLVMConstBitCast(result, LLT);
                            //LLVMDumpValue(result);
                            SCOPES_ERROR(CGenInvalidRedeclaration, name);
                        }
                    } else {
                        void *pptr = local_aware_dlsym(node->name);
                        uint64_t ptr = *(uint64_t*)&pptr;
                        if (!ptr) {
                            last_llvm_error = nullptr;
                            LLVMInstallFatalErrorHandler(fatal_error_handler);
                            ptr = SCOPES_GET_RESULT(get_address(name));
                            LLVMResetFatalErrorHandler();
                            if (last_llvm_error) {
                                SCOPES_RETURN_ERROR(last_llvm_error);
                            }
                        }
                        if (!ptr) {
                            SCOPES_ERROR(CGenFailedToResolveExtern, node);
                        }
                        result = LLVMAddGlobal(module, LLT, name);
                    }
                }
                global2global.insert({ node.unref(), result });
                return result;
            }
        } else {
            return it->second;
        }
    }

    SCOPES_RESULT(LLVMValueRef) ConstInt_to_value(const ConstIntRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        auto T = SCOPES_GET_RESULT(type_to_llvm_type(node->get_type()));
        return LLVMConstIntOfArbitraryPrecision(T,
                                              node->words.size(),
                                              &node->words[0]);
    }

    SCOPES_RESULT(LLVMValueRef) ConstReal_to_value(const ConstRealRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        auto T = SCOPES_GET_RESULT(type_to_llvm_type(node->get_type()));
        return LLVMConstReal(T, node->value);
    }

    LLVMValueRef make_string_constant(const char *str) {
        auto data = LLVMConstString(str, strlen(str)+1, true);
        auto basevalue = LLVMAddGlobal(module, LLVMTypeOf(data), "");
        LLVMSetInitializer(basevalue, data);
        LLVMSetGlobalConstant(basevalue, true);
        return basevalue;
    }

    SCOPES_RESULT(LLVMValueRef) ConstPointer_to_value(const ConstPointerRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        auto LLT = SCOPES_GET_RESULT(type_to_llvm_type(node->get_type()));
        if (!node->value) {
            return LLVMConstPointerNull(LLT);
        } else if (!generate_object) {
            if (serialize_pointers) {
                auto name = get_local_pointer_id(node->value);
                auto ET = LLVMGetElementType(LLT);
                auto glob = LLVMAddGlobal(module, ET, name.c_str());
                pointer_map.insert({name, node->value});
                return glob;
            } else {
                return LLVMConstIntToPtr(
                    LLVMConstInt(i64T, *(uint64_t*)&(node->value), false),
                    LLT);
            }
        } else {
            // to serialize a pointer, we serialize the allocation range
            // of the pointer as a global binary blob
            void *baseptr;
            size_t alloc_size;
            if (!find_allocation((void *)node->value, baseptr, alloc_size)) {
                SCOPES_ERROR(CGenCannotSerializeMemory, node->get_type());
            }
            LLVMValueRef basevalue = nullptr;
            auto it = ptr2global.find(baseptr);

            auto pi = cast<PointerType>(node->get_type());
            bool writable = pi->is_writable();

            if (it == ptr2global.end()) {
                auto data = LLVMConstString((const char *)baseptr, alloc_size, true);
                basevalue = LLVMAddGlobal(module, LLVMTypeOf(data), "");
                ptr2global.insert({ baseptr, basevalue });
                LLVMSetInitializer(basevalue, data);
                if (!writable) {
                    LLVMSetGlobalConstant(basevalue, true);
                }
            } else {
                basevalue = it->second;
            }
            size_t offset = (uint8_t*)node->value - (uint8_t*)baseptr;
            LLVMValueRef indices[2];
            indices[0] = LLVMConstInt(i64T, 0, false);
            indices[1] = LLVMConstInt(i64T, offset, false);
            return LLVMConstPointerCast(
                LLVMConstGEP(basevalue, indices, 2), LLT);
        }
    }

    SCOPES_RESULT(LLVMValueRef) ConstAggregate_to_value(const ConstAggregateRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        LLVMTypeRef LLT = SCOPES_GET_RESULT(type_to_llvm_type(node->get_type()));
        size_t count = node->values.size();
        LLVMValueRef values[count];
        for (size_t i = 0; i < count; ++i) {
            values[i] = SCOPES_GET_RESULT(ref_to_value(ValueIndex(get_field(node, i))));
        }
        switch(LLVMGetTypeKind(LLT)) {
        case LLVMStructTypeKind: {
            if (node->get_type()->kind() == TK_Typename) {
                return LLVMConstNamedStruct(LLT, values, count);
            } else {
                return LLVMConstStruct(values, count, false);
            }
        } break;
        case LLVMArrayTypeKind: {
            auto ai = cast<ArrayLikeType>(SCOPES_GET_RESULT(storage_type(node->get_type())));
            return LLVMConstArray(SCOPES_GET_RESULT(type_to_llvm_type(ai->element_type)),
                values, count);
        } break;
        case LLVMVectorTypeKind: {
            return LLVMConstVector(values, count);
        } break;
        default:
            assert(false);
            return nullptr;
        }
    }

    SCOPES_RESULT(void) build_call(const CallRef &call,
        const Type *functype, LLVMValueRef func, const TypedValues &args) {
        SCOPES_RESULT_TYPE(void);
        size_t argcount = args.size();

        LLVMMetadataRef diloc = nullptr;
        if (use_debug_info) {
            diloc = set_debug_location(call.anchor());
            assert(diloc);
        }

        auto fi = cast<FunctionType>(functype);

        auto rtype = abi_return_type(fi);
        bool use_sret = is_memory_class(rtype);

        LLVMValueRefs values;
        values.reserve(argcount + 1);

        auto retT = SCOPES_GET_RESULT(type_to_llvm_type(rtype));
        if (use_sret) {
            values.push_back(safe_alloca(retT));
        }
        std::vector<size_t> memptrs;
        for (size_t i = 0; i < argcount; ++i) {
            auto arg = args[i];
            LLVMValueRef val = SCOPES_GET_RESULT(ref_to_value(arg));
            auto AT = arg->get_type();
            SCOPES_CHECK_RESULT(abi_export_argument(val, AT, values, memptrs));
        }

        size_t fargcount = fi->argument_types.size();
        assert(argcount >= fargcount);
        // make variadic calls C compatible
        if (fi->flags & FF_Variadic) {
            for (size_t i = fargcount; i < argcount; ++i) {
                auto value = values[i];
                // floats need to be widened to doubles
                if (LLVMTypeOf(value) == f32T) {
                    values[i] = LLVMBuildFPExt(builder, value, f64T, "");
                }
            }
        }

        if (use_debug_info) {
            set_debug_location(diloc);
        }

        auto ret = LLVMBuildCall(builder, func, &values[0], values.size(), "");
        for (auto idx : memptrs) {
            auto i = idx + 1;
            LLVMAddCallSiteAttribute(ret, i, attr_nonnull);
            LLVMAddCallSiteAttribute(ret, i, attr_byval);
        }
        if (use_sret) {
            LLVMAddCallSiteAttribute(ret, 1, attr_sret);
            ret = LLVMBuildLoad(builder, values[0], "");
        } else if (is_returning_value(rtype)) {
            // check if ABI needs something else and do a bitcast
            auto srcT = LLVMTypeOf(ret);
            if (retT != srcT) {
                LLVMValueRef dest = safe_alloca(srcT);
                LLVMBuildStore(builder, ret, dest);
                ret = LLVMBuildBitCast(builder, dest, LLVMPointerType(retT, 0), "");
                ret = LLVMBuildLoad(builder, ret, "");
            }
        }

        if (fi->has_exception()) {
            bool has_except_value = is_returning_value(fi->except_type);
            bool has_return_value = is_returning_value(fi->return_type);
            if (has_except_value) {
                assert(call->except);
                auto except = LLVMBuildExtractValue(builder, ret, 1, "");
                LLVMValueRefs values;
                struct_to_values(values, fi->except_type, except);
                map_phi(values, call->except);
            }
            auto old_bb = LLVMGetInsertBlock(builder);
            auto bb_except = LLVMAppendBasicBlock(LLVMGetBasicBlockParent(old_bb), "call_failed");
            position_builder_at_end(bb_except);
            SCOPES_CHECK_RESULT(translate_block(call->except_body));
            position_builder_at_end(old_bb);
            if (!is_returning(fi->return_type)) {
                // always raises
                LLVMBuildBr(builder, bb_except);
                return {};
            } else {
                auto bb = LLVMAppendBasicBlock(LLVMGetBasicBlockParent(old_bb), "call_ok");
                LLVMValueRef ok = ret;
                if (has_except_value || has_return_value) {
                    ok = LLVMBuildExtractValue(builder, ret, 0, "");
                }
                LLVMBuildCondBr(builder, ok, bb, bb_except);
                position_builder_at_end(bb);
                if (has_return_value) {
                    int retvalue_index = (has_except_value?2:1);
                    ret = LLVMBuildExtractValue(builder, ret, retvalue_index, "");
                    LLVMValueRefs values;
                    struct_to_values(values, fi->return_type, ret);
                    map_phi(values, call);
                }
            }
        } else if (!is_returning(fi->return_type)) {
            LLVMBuildUnreachable(builder);
        } else {
            LLVMValueRefs values;
            struct_to_values(values, fi->return_type, ret);
            map_phi(values, call);
        }
        return {};
    }

    LLVMMetadataRef anchor_to_location(const Anchor *anchor) {
        assert(use_debug_info);

        auto old_bb = LLVMGetInsertBlock(builder);
        assert(old_bb);
        LLVMValueRef func = LLVMGetBasicBlockParent(old_bb);
        LLVMMetadataRef disp = LLVMGetSubprogram(func);

        LLVMMetadataRef scope = disp;
        if (active_function.anchor()->path != anchor->path) {
            LLVMMetadataRef difile = source_file_to_scope(anchor->path);
            scope = LLVMDIBuilderCreateLexicalBlock(di_builder, disp, difile, 1, 1);
        }

        LLVMMetadataRef result = LLVMDIBuilderCreateDebugLocation(
            LLVMGetGlobalContext(),
            anchor->lineno, anchor->column, scope, nullptr);

        return result;
    }

    void set_debug_location(LLVMMetadataRef diloc) {
        assert(use_debug_info);
        LLVMSetCurrentDebugLocation(builder,
            LLVMMetadataAsValue(LLVMGetGlobalContext(), diloc));
    }

    LLVMMetadataRef set_debug_location(const Anchor *anchor) {
        assert(use_debug_info);
        LLVMMetadataRef diloc = anchor_to_location(anchor);
        set_debug_location(diloc);
        return diloc;
    }

    LLVMValueRef build_length_op(LLVMValueRef x) {
        auto T = LLVMTypeOf(x);
        auto ET = LLVMGetElementType(T);
        LLVMValueRef func_sqrt = get_intrinsic((ET == f64T)?llvm_sqrt_f64:llvm_sqrt_f32);
        assert(func_sqrt);
        auto count = LLVMGetVectorSize(T);
        LLVMValueRef src = LLVMBuildFMul(builder, x, x, "");
        LLVMValueRef retvalue = nullptr;
        for (unsigned i = 0; i < count; ++i) {
            LLVMValueRef idx = LLVMConstInt(i32T, i, false);
            LLVMValueRef val = LLVMBuildExtractElement(builder, src, idx, "");
            if (i == 0) {
                retvalue = val;
            } else {
                retvalue = LLVMBuildFAdd(builder, retvalue, val, "");
            }
        }
        LLVMValueRef values[] = { retvalue };
        return LLVMBuildCall(builder, func_sqrt, values, 1, "");
    }

    LLVMValueRef safe_alloca(LLVMTypeRef ty, LLVMValueRef val = nullptr) {
        if (val && !LLVMIsConstant(val)) {
            // for stack arrays with dynamic size, build the array locally
            return LLVMBuildArrayAlloca(builder, ty, val, "");
        } else {
            // add allocas to the front
            auto oldbb = LLVMGetInsertBlock(builder);
            auto entry = LLVMGetEntryBasicBlock(LLVMGetBasicBlockParent(oldbb));
            auto instr = LLVMGetFirstInstruction(entry);
            if (instr) {
                LLVMPositionBuilderBefore(builder, instr);
            } else {
                position_builder_at_end(entry);
            }
            LLVMValueRef result;
            if (val) {
                result = LLVMBuildArrayAlloca(builder, ty, val, "");
            } else {
                result = LLVMBuildAlloca(builder, ty, "");
                //LLVMSetAlignment(result, 16);
            }
            position_builder_at_end(oldbb);
            return result;
        }
    }

    LLVMValueRef build_matching_constant_real_vector(LLVMValueRef value, double c) {
        auto T = LLVMTypeOf(value);
        if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
            unsigned count = LLVMGetVectorSize(T);
            auto ET = LLVMGetElementType(T);
            LLVMValueRef one = LLVMConstReal(ET, c);
            LLVMValueRef values[count];
            for (unsigned i = 0; i < count; ++i) {
                values[i] = one;
            }
            return LLVMConstVector(values, count);
        } else {
            return LLVMConstReal(T, c);
        }
    }

    void setup_generate(const char *module_name) {
        module = LLVMModuleCreateWithName(module_name);
        builder = LLVMCreateBuilder();
        di_builder = LLVMCreateDIBuilder(module);

        if (use_debug_info) {
            const char *DebugStr = "Debug Info Version";
            LLVMValueRef DbgVer[3];
            DbgVer[0] = LLVMConstInt(i32T, 1, 0);
            DbgVer[1] = LLVMMDString(DebugStr, strlen(DebugStr));
            DbgVer[2] = LLVMConstInt(i32T, 3, 0);
            LLVMAddNamedMetadataOperand(module, "llvm.module.flags",
                LLVMMDNode(DbgVer, 3));

            const char *fname = "file"; // module_name
            const char *dname = "directory";
            LLVMMetadataRef fileref = LLVMDIBuilderCreateFile(di_builder,
                fname, strlen(fname),
                dname, strlen(dname));

            const char *producer = "scopes";
            LLVMDIBuilderCreateCompileUnit(
                di_builder,
                /*Lang*/ LLVMDWARFSourceLanguageC99,
                /*FileRef*/ fileref,
                /*Producer*/ producer, strlen(producer),
                /*isOptimized*/ false,
                /*Flags*/ "", 0,
                /*RuntimeVer*/ 0,
                /*SplitName*/ "", 0,
                /*Kind*/ LLVMDWARFEmissionFull,
                /*DWOId*/ 0,
                /*SplitDebugInlining*/ true,
                /*DebugInfoForProfiling*/ false);

            //LLVMAddNamedMetadataOperand(module, "llvm.dbg.cu", dicu);
        }
    }

    SCOPES_RESULT(void) process_functions() {
        SCOPES_RESULT_TYPE(void);
        while (!function_todo.empty()) {
            FunctionRef func = function_todo.front();
            function_todo.pop_front();
            SCOPES_CHECK_RESULT(Function_finalize(func));
        }
        return {};
    }

    SCOPES_RESULT(void) teardown_generate(const FunctionRef &entry = FunctionRef()) {
        SCOPES_RESULT_TYPE(void);
        size_t k = SCOPES_GET_RESULT(finalize_types());
        assert(!k);

        LLVMDisposeBuilder(builder);
        LLVMDIBuilderFinalize(di_builder);
        LLVMDisposeDIBuilder(di_builder);

#if SCOPES_DEBUG_CODEGEN
        LLVMDumpModule(module);
#endif
        char *errmsg = NULL;
        if (LLVMVerifyModule(module, LLVMReturnStatusAction, &errmsg)) {
            StyledStream ss(SCOPES_CERR);
            //if (entry) {
            //    stream_value(ss, entry);
            //}
            LLVMDumpModule(module);
            SCOPES_ERROR(CGenBackendFailed, errmsg);
        }
        LLVMDisposeMessage(errmsg);
        return {};
    }

    typedef std::pair<LLVMModuleRef, LLVMValueRef> ModuleValuePair;

    void build_constructor_function() {
        assert(constructor_function);
        auto func = constructor_function;
        //LLVMSetLinkage(func, LLVMPrivateLinkage);
        auto bb = LLVMAppendBasicBlock(func, "");
        position_builder_at_end(bb);

        for (auto val : constructors) {
            LLVMBuildCall(builder, val, nullptr, 0, "");
        }

        LLVMBuildRetVoid(builder);
    }

    void build_constructors() {
        if (constructors.empty()) {
            return;
        }
        auto count = constructors.size();
        LLVMValueRef structs[count];
        int i = 0;
        auto nullbyteptr = LLVMConstNull(rawstringT);
        for (auto val : constructors) {
            LLVMValueRef constvals[] = { LLVMConstInt(i32T, i, false), val, nullbyteptr };
            structs[i] = LLVMConstStruct(constvals, 3, false);
            i++;
        }
        auto arr = LLVMConstArray(LLVMTypeOf(structs[0]), structs, count);
        auto glob = LLVMAddGlobal(module, LLVMTypeOf(arr), "llvm.global_ctors");
        LLVMSetInitializer(glob, arr);
        LLVMSetLinkage(glob, LLVMAppendingLinkage);
    }

    // for generating object files
    SCOPES_RESULT(LLVMModuleRef) generate(const String *name, const Scope *table) {
        SCOPES_RESULT_TYPE(LLVMModuleRef);

        setup_generate(name->data);

        std::vector<LLVMValueRef> exported_globals;

        const Scope *t = table;
        while (t) {
            auto it = sc_scope_next(t, -1);
            while (it._2 != -1) {
                auto key = it._0.cast<Const>();
                auto val = it._1;
                if (key->get_type() == TYPE_Symbol) {
                    Symbol name = Symbol::wrap(key.cast<ConstInt>()->value());
                    FunctionRef fn = SCOPES_GET_RESULT(extract_function_constant(val));
                    func_export_table.insert({fn.unref(), name});
                    LLVMValueRef func = SCOPES_GET_RESULT(ref_to_value(ValueIndex(fn)));
                    exported_globals.push_back(func);
                }
                it = sc_scope_next(t, it._2);
            }
            t = t->parent();
        }

        SCOPES_CHECK_RESULT(process_functions());

        build_constructors();

        {
            // build llvm.used
            auto count = exported_globals.size();
            LLVMValueRef constvals[count];
            for (int i = 0; i < count; ++i) {
                LLVMValueRef func = exported_globals[i];
                constvals[i] = LLVMConstBitCast(func, rawstringT);
            };
            auto arr = LLVMConstArray(rawstringT, constvals, count);
            auto glob = LLVMAddGlobal(module, LLVMTypeOf(arr), "llvm.used");
            LLVMSetInitializer(glob, arr);
            LLVMSetLinkage(glob, LLVMAppendingLinkage);
        }

        SCOPES_CHECK_RESULT(teardown_generate());
        return module;
    }

    SCOPES_RESULT(ModuleValuePair) generate(const FunctionRef &entry) {
        SCOPES_RESULT_TYPE(ModuleValuePair);

        const char *name = entry->name.name()->data;
        setup_generate(name);

        {
            constructor_function = LLVMAddFunction(module, "",
                LLVMFunctionType(voidT, nullptr, 0, false));
        }

        entry_function = entry;

        auto func = SCOPES_GET_RESULT(ref_to_value(ValueIndex(entry)));
        LLVMSetLinkage(func, LLVMExternalLinkage);

        SCOPES_CHECK_RESULT(process_functions());
        build_constructor_function();
        SCOPES_CHECK_RESULT(teardown_generate(entry));

        return ModuleValuePair(module, func);
    }

};

Error *LLVMIRGenerator::last_llvm_error = nullptr;
std::unordered_map<const Type *, LLVMTypeRef> LLVMIRGenerator::type_cache;
std::unordered_map<Function *, std::string> LLVMIRGenerator::func_cache;
std::unordered_map<Global *, std::string> LLVMIRGenerator::global_cache;
std::unordered_map<size_t, PointerNamespaces *> LLVMIRGenerator::pointer_namespaces;
Types LLVMIRGenerator::type_todo;
LLVMTypeRef LLVMIRGenerator::voidT = nullptr;
LLVMTypeRef LLVMIRGenerator::i1T = nullptr;
LLVMTypeRef LLVMIRGenerator::i8T = nullptr;
LLVMTypeRef LLVMIRGenerator::i16T = nullptr;
LLVMTypeRef LLVMIRGenerator::i32T = nullptr;
LLVMTypeRef LLVMIRGenerator::i64T = nullptr;
LLVMTypeRef LLVMIRGenerator::f32T = nullptr;
LLVMTypeRef LLVMIRGenerator::f32x2T = nullptr;
LLVMTypeRef LLVMIRGenerator::f64T = nullptr;
LLVMTypeRef LLVMIRGenerator::f80T = nullptr;
LLVMTypeRef LLVMIRGenerator::f128T = nullptr;
LLVMTypeRef LLVMIRGenerator::rawstringT = nullptr;
LLVMTypeRef LLVMIRGenerator::noneT = nullptr;
LLVMValueRef LLVMIRGenerator::noneV = nullptr;
LLVMValueRef LLVMIRGenerator::falseV = nullptr;
LLVMValueRef LLVMIRGenerator::trueV = nullptr;
LLVMAttributeRef LLVMIRGenerator::attr_byval = nullptr;
LLVMAttributeRef LLVMIRGenerator::attr_sret = nullptr;
LLVMAttributeRef LLVMIRGenerator::attr_nonnull = nullptr;

//------------------------------------------------------------------------------
// IL COMPILER
//------------------------------------------------------------------------------

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
    DisassemblyListener() {}

    std::unordered_map<void *, size_t> sizes;

    void InitializeDebugData(
        llvm::StringRef name,
        llvm::object::SymbolRef::Type type, uint64_t sz) {
        if(type == llvm::object::SymbolRef::ST_Function) {
            #if !defined(__arm__) && !defined(__linux__)
            name = name.substr(1);
            #endif
            void * addr = (void *)get_address(name.data()).assert_ok();
            if(addr) {
                assert(addr);
                sizes[addr] = sz;
            }
        }
    }

    virtual void NotifyObjectEmitted(
        const llvm::object::ObjectFile &Obj,
        const llvm::RuntimeDyld::LoadedObjectInfo &L) {
        StyledStream ss;
        ss << "object emitted!" << std::endl;
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

SCOPES_RESULT(void) compile_object(const String *triple,
    CompilerFileKind kind, const String *path, const Scope *scope, uint64_t flags) {
    SCOPES_RESULT_TYPE(void);
    Timer sum_compile_time(TIMER_Compile);
#if SCOPES_COMPILE_WITH_DEBUG_INFO
#else
    flags |= CF_NoDebugInfo;
#endif
#if SCOPES_OPTIMIZE_ASSEMBLY
    flags |= CF_O3;
#endif

    LLVMIRGenerator ctx;
    ctx.generate_object = true;
    if (flags & CF_NoDebugInfo) {
        ctx.use_debug_info = false;
    }

    LLVMModuleRef module;
    {
        Timer generate_timer(TIMER_Generate);
        module = SCOPES_GET_RESULT(ctx.generate(path, scope));
    }

    if (flags & CF_O3) {
        Timer optimize_timer(TIMER_Optimize);
        int level = 0;
        if ((flags & CF_O3) == CF_O1)
            level = 1;
        else if ((flags & CF_O3) == CF_O2)
            level = 2;
        else if ((flags & CF_O3) == CF_O3)
            level = 3;
        build_and_run_opt_passes(module, level);
    }
    if (flags & CF_DumpModule) {
        LLVMDumpModule(module);
    }

    auto tt = LLVMNormalizeTargetTriple(triple->data);
    static char triplestr[1024];
    strncpy(triplestr, tt, 1024);
    LLVMDisposeMessage(tt);

    char *error_message = nullptr;
    LLVMTargetRef target = nullptr;
    if (LLVMGetTargetFromTriple(triplestr, &target, &error_message)) {
        SCOPES_ERROR(CGenBackendFailed, error_message);
    }
    // code model must be JIT default for reasons beyond my comprehension
    auto tm = LLVMCreateTargetMachine(target, triplestr, nullptr, nullptr,
        LLVMCodeGenLevelDefault, LLVMRelocPIC, LLVMCodeModelJITDefault);
    assert(tm);

    char *path_cstr = strdup(path->data);
    LLVMBool failed = false;
    switch(kind) {
    case CFK_Object: {
        failed = LLVMTargetMachineEmitToFile(tm, module, path_cstr,
            LLVMObjectFile, &error_message);
    } break;
    case CFK_ASM: {
        failed = LLVMTargetMachineEmitToFile(tm, module, path_cstr,
            LLVMAssemblyFile, &error_message);
    } break;
    case CFK_BC: {
        failed = LLVMWriteBitcodeToFile(module, path_cstr);
    } break;
    case CFK_LLVM: {
        failed = LLVMPrintModuleToFile(module, path_cstr, &error_message);
    } break;
    default: {
        free(path_cstr);
        SCOPES_ERROR(CGenBackendFailed, "unknown file kind");
    } break;
    }
    free(path_cstr);
    if (failed) {
        SCOPES_ERROR(CGenBackendFailed, error_message);
    }
    return {};
}

#if SCOPES_LLVM_SUPPORT_DISASSEMBLY
static DisassemblyListener *disassembly_listener = nullptr;
#endif

SCOPES_RESULT(ConstPointerRef) compile(const FunctionRef &fn, uint64_t flags) {
    SCOPES_RESULT_TYPE(ConstPointerRef);
    Timer sum_compile_time(TIMER_Compile);
#if SCOPES_COMPILE_WITH_DEBUG_INFO
#else
    flags |= CF_NoDebugInfo;
#endif
#if SCOPES_OPTIMIZE_ASSEMBLY
    flags |= CF_O3;
#endif

    /*
    const Type *functype = pointer_type(
        fn->get_type(), PTF_NonWritable, SYM_Unnamed);
    */
   const Type *functype = fn->get_type();

    LLVMIRGenerator ctx;
    if (flags & CF_Cache) {
        ctx.serialize_pointers = true;
        ctx.set_pointer_namespace(fn->name.hash());
    } else {
        ctx.set_pointer_namespace(0);
    }
    if (flags & CF_NoDebugInfo) {
        ctx.use_debug_info = false;
    }

    LLVMIRGenerator::ModuleValuePair result;
    {
        /*
        A note on debugging "LLVM ERROR:" messages that seem to give no plausible
        point of origin: you can either set a breakpoint at llvm::report_fatal_error
        or at exit if the llvm symbols are missing, and then look at the stack trace.
        */
        Timer generate_timer(TIMER_Generate);
        result = SCOPES_GET_RESULT(ctx.generate(fn));
    }

#if 0
    {
        StyledStream ss;
        ss << ctx.functions_generated << " function(s) generated" << std::endl;
    }
#endif

    auto module = result.first;
    auto func = result.second;
    assert(func);

    SCOPES_CHECK_RESULT(init_execution());

#if SCOPES_LLVM_SUPPORT_DISASSEMBLY
    if (!disassembly_listener && (flags & CF_DumpDisassembly)) {
        disassembly_listener = new DisassemblyListener();
        llvm::JITEventListener *le = disassembly_listener;
        add_jit_event_listener(reinterpret_cast<LLVMJITEventListenerRef>(le));
    }
#endif

    if (flags & CF_O3) {
        Timer optimize_timer(TIMER_Optimize);
        int level = 0;
        if ((flags & CF_O3) == CF_O1)
            level = 1;
        else if ((flags & CF_O3) == CF_O2)
            level = 2;
        else if ((flags & CF_O3) == CF_O3)
            level = 3;
        build_and_run_opt_passes(module, level);
    }
    if (flags & CF_DumpModule) {
        LLVMDumpModule(module);
    } else if (flags & CF_DumpFunction) {
        LLVMDumpValue(func);
    }

    std::string funcname;
    {
        size_t length = 0;
        const char *name = LLVMGetValueName2(func, &length);
        funcname = std::string(name, length);
    }

#if 1
    std::vector< std::string > bindsyms;
    for (auto sym : ctx.generated_symbols) {
        size_t length = 0;
        const char *name = LLVMGetValueName2(sym, &length);
        bindsyms.push_back(std::string(name, length));
    }
#endif

    SCOPES_CHECK_RESULT(add_module(module, ctx.pointer_map, flags));

#if 1
    for (auto sym : bindsyms) {
        void *ptr = (void *)SCOPES_GET_RESULT(get_address(sym.c_str()));
        set_address_name(ptr, String::from(sym.c_str(), sym.size()));
    }
#endif

    //LLVMDumpModule(module);
    void *pfunc = (void *)SCOPES_GET_RESULT(get_address(funcname.c_str()));
#if SCOPES_LLVM_SUPPORT_DISASSEMBLY
    if (flags & CF_DumpDisassembly) {
        assert(disassembly_listener);
        //auto td = LLVMGetExecutionEngineTargetData(ee);
        auto it = disassembly_listener->sizes.find(pfunc);
        if (it != disassembly_listener->sizes.end()) {
            std::cout << "disassembly:\n";
            auto target_machine = get_jit_target_machine();
            do_disassemble(target_machine, pfunc, it->second);
        } else {
            std::cout << "no disassembly available\n";
        }
    }
#endif

    return ref(fn.anchor(), ConstPointer::from(functype, pfunc));
}

} // namespace scopes
