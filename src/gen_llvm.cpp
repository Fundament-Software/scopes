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
#include <llvm-c/Disassembler.h>
#include <llvm-c/Support.h>
#include <llvm-c/DebugInfo.h>

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
#define SCOPES_LLVM_CACHE_FUNCTIONS 1

//------------------------------------------------------------------------------
// IL->LLVM IR GENERATOR
//------------------------------------------------------------------------------

static void build_and_run_opt_passes(LLVMModuleRef module, int opt_level) {
    LLVMPassManagerBuilderRef passBuilder;

    passBuilder = LLVMPassManagerBuilderCreate();
    LLVMPassManagerBuilderSetOptLevel(passBuilder, opt_level);
    LLVMPassManagerBuilderSetSizeLevel(passBuilder, 0);
    if (opt_level >= 2) {
        LLVMPassManagerBuilderUseInlinerWithThreshold(passBuilder, 225);
    }

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

struct LLVMIRGenerator {
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

    std::unordered_map<SourceFile *, LLVMMetadataRef> file2value;
    std::unordered_map<void *, LLVMValueRef> ptr2global;
    std::unordered_map<ValueIndex, LLVMValueRef, ValueIndex::Hash> ref2value;
    std::unordered_map<Function *, LLVMMetadataRef> func2md;
    std::unordered_map<Function *, Symbol> func_export_table;
    std::unordered_map<Global *, LLVMValueRef> global2global;
    std::deque<FunctionRef> function_todo;
    static Types type_todo;
    static std::unordered_map<const Type *, LLVMTypeRef> type_cache;
    static std::unordered_map<Function *, LLVMModuleRef> func_cache;
    static std::unordered_map<Global *, LLVMModuleRef> global_cache;

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
    static LLVMTypeRef rawstringT;
    static LLVMTypeRef noneT;
    static LLVMValueRef noneV;
    static LLVMValueRef falseV;
    static LLVMValueRef trueV;
    static LLVMAttributeRef attr_byval;
    static LLVMAttributeRef attr_sret;
    static LLVMAttributeRef attr_nonnull;
    LLVMValueRef intrinsics[NumIntrinsics];

    bool use_debug_info;
    bool generate_object;
    FunctionRef active_function;
    std::vector<LLVMValueRef> generated_symbols;

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

    LLVMIRGenerator() :
        //active_function(nullptr),
        //active_function_value(nullptr),
        use_debug_info(true),
        generate_object(false) {
        static_init();
        for (int i = 0; i < NumIntrinsics; ++i) {
            intrinsics[i] = nullptr;
        }
    }

    LLVMMetadataRef source_file_to_scope(SourceFile *sf) {
        assert(use_debug_info);

        auto it = file2value.find(sf);
        if (it != file2value.end())
            return it->second;

        char *dn = strdup(sf->path.name()->data);
        char *bn = strdup(dn);

        char *fname = basename(bn);
        char *dname = dirname(dn);

        LLVMMetadataRef result = LLVMDIBuilderCreateFile(di_builder,
            fname, strlen(fname), dname, strlen(dname));

        free(dn);
        free(bn);

        file2value.insert({ sf, result });

        return result;
    }

    LLVMMetadataRef function_to_subprogram(const FunctionRef &l) {
        assert(use_debug_info);

        auto it = func2md.find(l.unref());
        if (it != func2md.end())
            return it->second;

        const Anchor *anchor = l.anchor();

        LLVMMetadataRef difile = source_file_to_scope(anchor->file);

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
                    LLVMBuildStore(builder, LLVMGetParam(func, k++), dest);
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
            LLVMBuildStore(builder, val, ptrval);
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
            default: break;
            }
            break;
        case TK_Pointer:
            return LLVMPointerType(
                SCOPES_GET_RESULT(_type_to_llvm_type(cast<PointerType>(type)->element_type)), 0);
        case TK_Array: {
            auto ai = cast<ArrayType>(type);
            return LLVMArrayType(SCOPES_GET_RESULT(_type_to_llvm_type(ai->element_type)), ai->count);
        } break;
        case TK_Vector: {
            auto vi = cast<VectorType>(type);
            return LLVMVectorType(SCOPES_GET_RESULT(_type_to_llvm_type(vi->element_type)), vi->count);
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
        case TK_Union: {
            auto ui = cast<UnionType>(type);
            return _type_to_llvm_type(ui->tuple_type);
        } break;
        case TK_Typename: {
            if (type == TYPE_Sampler) {
                SCOPES_ERROR(CGenTypeUnsupportedInTarget, TYPE_Sampler);
            }
            auto tn = cast<TypenameType>(type);
            if (tn->finalized()) {
                switch(tn->storage_type->kind()) {
                case TK_Tuple:
                case TK_Union: {
                    type_todo.push_back(type);
                } break;
                default: {
                    return create_llvm_type(tn->storage_type);
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
            if (!tn->finalized())
                continue;
            LLVMTypeRef LLT = SCOPES_GET_RESULT(_type_to_llvm_type(T));
            const Type *ST = tn->storage_type;
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
            case TK_Union: {
                auto ui = cast<UnionType>(ST);
                size_t count = ui->values.size();
                size_t sz = ui->size;
                size_t al = ui->align;
                // find member with the same alignment
                for (size_t i = 0; i < count; ++i) {
                    const Type *ET = ui->values[i];
                    size_t etal = SCOPES_GET_RESULT(align_of(ET));
                    if (etal == al) {
                        size_t remsz = sz - SCOPES_GET_RESULT(size_of(ET));
                        LLVMTypeRef values[2];
                        values[0] = SCOPES_GET_RESULT(_type_to_llvm_type(ET));
                        if (remsz) {
                            // too small, add padding
                            values[1] = LLVMArrayType(i8T, remsz);
                            LLVMStructSetBody(LLT, values, 2, false);
                        } else {
                            LLVMStructSetBody(LLT, values, 1, false);
                        }
                        break;
                    }
                }
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
            LLVMBuildStore(builder, value, LLVMGetParam(parentfunc, 0));
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

        if (fi->has_exception()) {
            try_info.bb_except = LLVMAppendBasicBlock(func, "except");
            position_builder_at_end(try_info.bb_except);
            if (use_debug_info)
                set_debug_location(node.anchor());
            SCOPES_CHECK_RESULT(build_phi(try_info.except_values, fi->except_type));
            SCOPES_CHECK_RESULT(write_return(try_info.except_values, true));
        }

        position_builder_at_end(bb);

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
        const String *name = nullptr;
        {
            auto it = func_export_table.find(node.unref());
            if (it != func_export_table.end()) {
                name = it->second.name();
                is_export = true;
            }
        }
        if (!name) {
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
            stream_address(ss.out, node.unref());
            name = ss.str();
        }

        auto functype = SCOPES_GET_RESULT(type_to_llvm_type(ilfunctype));

        auto func = LLVMAddFunction(module, name->data, functype);

#if SCOPES_LLVM_CACHE_FUNCTIONS
        if (!generate_object) {
            auto it = func_cache.find(node.unref());
            if (it != func_cache.end()) {
                assert(it->second != module);
                return func;
            }

            func_cache.insert({node.unref(), module});
        }
#endif
        generated_symbols.push_back(func);

        if (use_debug_info) {
            LLVMSetSubprogram(func, function_to_subprogram(node));
        }
        if (is_export) {
            LLVMSetLinkage(func, LLVMExternalLinkage);
        } else if (generate_object) {
            LLVMSetLinkage(func, LLVMPrivateLinkage);
        } else {
#if !SCOPES_LLVM_CACHE_FUNCTIONS
            LLVMSetLinkage(func, LLVMPrivateLinkage);
#endif
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
            auto llvmval = SCOPES_GET_RESULT(ref_to_value(values[i]));
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
            return values[0];
        } else {
            LLVMValueRef value = LLVMGetUndef(T);
            for (int i = 0; i < count; ++i) {
                value = LLVMBuildInsertValue(builder, value, values[i], i, "");
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

    SCOPES_RESULT(LLVMValueRef) translate_builtin(Builtin builtin, const TypedValues &args) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        size_t argcount = args.size();
        size_t argn = 0;

#define READ_VALUE(NAME) \
        assert(argn < argcount); \
        TypedValueRef _ ## NAME = args[argn++]; \
        LLVMValueRef NAME = SCOPES_GET_RESULT(ref_to_value(_ ## NAME));

#define READ_TYPE(NAME) \
        assert(argn < argcount); \
        LLVMTypeRef NAME = SCOPES_GET_RESULT(node_to_llvm_type(args[argn++]));

        switch(builtin.value()) {
        case FN_Annotate: {
            return nullptr;
        } break;
        case OP_Tertiary: {
            READ_VALUE(cond);
            READ_VALUE(then_value);
            READ_VALUE(else_value);
            return LLVMBuildSelect(
                builder, cond, then_value, else_value, "");
        } break;
        case FN_ExtractValue: {
            READ_VALUE(val);
            READ_VALUE(index);
            assert(LLVMIsConstant(index));
            assert(LLVMGetTypeKind(LLVMTypeOf(index)) == LLVMIntegerTypeKind);
            auto index_value = LLVMConstIntGetZExtValue(index);
            return LLVMBuildExtractValue(builder, val, index_value, "");
        } break;
        case FN_InsertValue: {
            READ_VALUE(val);
            READ_VALUE(eltval);
            READ_VALUE(index);
            assert(LLVMIsConstant(index));
            assert(LLVMGetTypeKind(LLVMTypeOf(index)) == LLVMIntegerTypeKind);
            auto index_value = LLVMConstIntGetZExtValue(index);
            return LLVMBuildInsertValue(builder, val, eltval, index_value, "");
        } break;
        case FN_ExtractElement: {
            READ_VALUE(val);
            READ_VALUE(index);
            return LLVMBuildExtractElement(builder, val, index, "");
        } break;
        case FN_InsertElement: {
            READ_VALUE(val);
            READ_VALUE(eltval);
            READ_VALUE(index);
            return LLVMBuildInsertElement(builder, val, eltval, index, "");
        } break;
        case FN_ShuffleVector: {
            READ_VALUE(v1);
            READ_VALUE(v2);
            READ_VALUE(mask);
            return LLVMBuildShuffleVector(builder, v1, v2, mask, "");
        } break;
        case FN_View:
        case FN_Lose:
        case FN_Track:
        case FN_Dupe:
        case FN_Move: {
            READ_VALUE(val);
            return val;
        } break;
        case FN_NullOf: { READ_TYPE(ty);
            return LLVMConstNull(ty); } break;
        case FN_Undef: { READ_TYPE(ty);
            return LLVMGetUndef(ty); } break;
        case FN_Alloca: { READ_TYPE(ty);
            return safe_alloca(ty);
        } break;
        case FN_AllocaArray: { READ_TYPE(ty); READ_VALUE(val);
            return safe_alloca(ty, val); } break;
        case FN_Malloc: { READ_TYPE(ty);
            return LLVMBuildMalloc(builder, ty, ""); } break;
        case FN_MallocArray: { READ_TYPE(ty); READ_VALUE(val);
            return LLVMBuildArrayMalloc(builder, ty, val, ""); } break;
        case FN_Free: { READ_VALUE(val);
            return LLVMBuildFree(builder, val); } break;
        case FN_GetElementRef: {
            READ_VALUE(pointer);
            assert(LLVMGetTypeKind(LLVMTypeOf(pointer)) == LLVMPointerTypeKind);
            assert(argcount > 1);
            size_t count = argcount;
            LLVMValueRef indices[count];
            LLVMTypeRef IT = nullptr;
            for (size_t i = 1; i < count; ++i) {
                indices[i] = SCOPES_GET_RESULT(ref_to_value(args[i]));
                //assert(LLVMGetValueKind(indices[i]) == LLVMConstantIntValueKind);
                assert(LLVMGetTypeKind(LLVMTypeOf(indices[i])) == LLVMIntegerTypeKind);
                IT = LLVMTypeOf(indices[i]);
            }
            assert(IT);
            indices[0] = LLVMConstInt(IT, 0, false);
            return LLVMBuildGEP(builder, pointer, indices, count, "");
        } break;
        case FN_GetElementPtr: {
            READ_VALUE(pointer);
            assert(LLVMGetTypeKind(LLVMTypeOf(pointer)) == LLVMPointerTypeKind);
            assert(argcount > 1);
            size_t count = argcount - 1;
            LLVMValueRef indices[count];
            for (size_t i = 0; i < count; ++i) {
                indices[i] = SCOPES_GET_RESULT(ref_to_value(args[argn + i]));
                assert(LLVMGetTypeKind(LLVMTypeOf(indices[i])) == LLVMIntegerTypeKind);
            }
            return LLVMBuildGEP(builder, pointer, indices, count, "");
        } break;
        case FN_Bitcast: { READ_VALUE(val); READ_TYPE(ty);
            auto T = LLVMTypeOf(val);
            if (T == ty) {
                return val;
            } else if (LLVMGetTypeKind(ty) == LLVMStructTypeKind) {
                // completely braindead, but what can you do
                LLVMValueRef ptr = safe_alloca(T);
                LLVMBuildStore(builder, val, ptr);
                ptr = LLVMBuildBitCast(builder, ptr, LLVMPointerType(ty,0), "");
                return LLVMBuildLoad(builder, ptr, "");
            } else {
                return LLVMBuildBitCast(builder, val, ty, "");
            }
        } break;
        case FN_IntToPtr: { READ_VALUE(val); READ_TYPE(ty);
            return LLVMBuildIntToPtr(builder, val, ty, ""); } break;
        case FN_PtrToInt: { READ_VALUE(val); READ_TYPE(ty);
            return LLVMBuildPtrToInt(builder, val, ty, ""); } break;
        case FN_ITrunc: { READ_VALUE(val); READ_TYPE(ty);
            return LLVMBuildTrunc(builder, val, ty, ""); } break;
        case FN_SExt: { READ_VALUE(val); READ_TYPE(ty);
            return LLVMBuildSExt(builder, val, ty, ""); } break;
        case FN_ZExt: { READ_VALUE(val); READ_TYPE(ty);
            return LLVMBuildZExt(builder, val, ty, ""); } break;
        case FN_FPTrunc: { READ_VALUE(val); READ_TYPE(ty);
            return LLVMBuildFPTrunc(builder, val, ty, ""); } break;
        case FN_FPExt: { READ_VALUE(val); READ_TYPE(ty);
            return LLVMBuildFPExt(builder, val, ty, ""); } break;
        case FN_FPToUI: { READ_VALUE(val); READ_TYPE(ty);
            return LLVMBuildFPToUI(builder, val, ty, ""); } break;
        case FN_FPToSI: { READ_VALUE(val); READ_TYPE(ty);
            return LLVMBuildFPToSI(builder, val, ty, ""); } break;
        case FN_UIToFP: { READ_VALUE(val); READ_TYPE(ty);
            return LLVMBuildUIToFP(builder, val, ty, ""); } break;
        case FN_SIToFP: { READ_VALUE(val); READ_TYPE(ty);
            return LLVMBuildSIToFP(builder, val, ty, ""); } break;
        case FN_Deref: {
            READ_VALUE(ptr);
            assert(LLVMGetTypeKind(LLVMTypeOf(ptr)) == LLVMPointerTypeKind);
            LLVMValueRef retvalue = LLVMBuildLoad(builder, ptr, "");
            return retvalue;
        } break;
        case FN_Assign: {
            READ_VALUE(lhs);
            READ_VALUE(rhs);

            rhs = fix_named_struct_store(lhs, rhs);

            LLVMValueRef retvalue = LLVMBuildStore(builder, lhs, rhs);
            return retvalue;
        } break;
        case FN_PtrToRef: {
            READ_VALUE(ptr);
            return ptr;
        } break;
        case FN_RefToPtr: {
            READ_VALUE(ptr);
            return ptr;
        } break;
        case FN_VolatileLoad:
        case FN_Load: { READ_VALUE(ptr);
            LLVMValueRef retvalue = LLVMBuildLoad(builder, ptr, "");
            if (builtin.value() == FN_VolatileLoad) { LLVMSetVolatile(retvalue, true); }
            return retvalue;
        } break;
        case FN_VolatileStore:
        case FN_Store: { READ_VALUE(val); READ_VALUE(ptr);

            ptr = fix_named_struct_store(val, ptr);

            LLVMValueRef retvalue = LLVMBuildStore(builder, val, ptr);
            if (builtin.value() == FN_VolatileStore) { LLVMSetVolatile(retvalue, true); }
            return retvalue;
        } break;
        case OP_ICmpEQ:
        case OP_ICmpNE:
        case OP_ICmpUGT:
        case OP_ICmpUGE:
        case OP_ICmpULT:
        case OP_ICmpULE:
        case OP_ICmpSGT:
        case OP_ICmpSGE:
        case OP_ICmpSLT:
        case OP_ICmpSLE: {
            READ_VALUE(a); READ_VALUE(b);
            LLVMIntPredicate pred = LLVMIntEQ;
            switch(builtin.value()) {
                case OP_ICmpEQ: pred = LLVMIntEQ; break;
                case OP_ICmpNE: pred = LLVMIntNE; break;
                case OP_ICmpUGT: pred = LLVMIntUGT; break;
                case OP_ICmpUGE: pred = LLVMIntUGE; break;
                case OP_ICmpULT: pred = LLVMIntULT; break;
                case OP_ICmpULE: pred = LLVMIntULE; break;
                case OP_ICmpSGT: pred = LLVMIntSGT; break;
                case OP_ICmpSGE: pred = LLVMIntSGE; break;
                case OP_ICmpSLT: pred = LLVMIntSLT; break;
                case OP_ICmpSLE: pred = LLVMIntSLE; break;
                default: assert(false); break;
            }
            return LLVMBuildICmp(builder, pred, a, b, "");
        } break;
        case OP_FCmpOEQ:
        case OP_FCmpONE:
        case OP_FCmpORD:
        case OP_FCmpOGT:
        case OP_FCmpOGE:
        case OP_FCmpOLT:
        case OP_FCmpOLE:
        case OP_FCmpUEQ:
        case OP_FCmpUNE:
        case OP_FCmpUNO:
        case OP_FCmpUGT:
        case OP_FCmpUGE:
        case OP_FCmpULT:
        case OP_FCmpULE: {
            READ_VALUE(a); READ_VALUE(b);
            LLVMRealPredicate pred = LLVMRealOEQ;
            switch(builtin.value()) {
                case OP_FCmpOEQ: pred = LLVMRealOEQ; break;
                case OP_FCmpONE: pred = LLVMRealONE; break;
                case OP_FCmpORD: pred = LLVMRealORD; break;
                case OP_FCmpOGT: pred = LLVMRealOGT; break;
                case OP_FCmpOGE: pred = LLVMRealOGE; break;
                case OP_FCmpOLT: pred = LLVMRealOLT; break;
                case OP_FCmpOLE: pred = LLVMRealOLE; break;
                case OP_FCmpUEQ: pred = LLVMRealUEQ; break;
                case OP_FCmpUNE: pred = LLVMRealUNE; break;
                case OP_FCmpUNO: pred = LLVMRealUNO; break;
                case OP_FCmpUGT: pred = LLVMRealUGT; break;
                case OP_FCmpUGE: pred = LLVMRealUGE; break;
                case OP_FCmpULT: pred = LLVMRealULT; break;
                case OP_FCmpULE: pred = LLVMRealULE; break;
                default: assert(false); break;
            }
            return LLVMBuildFCmp(builder, pred, a, b, "");
        } break;
        case OP_Add: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildAdd(builder, a, b, ""); } break;
        case OP_AddNUW: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildNUWAdd(builder, a, b, ""); } break;
        case OP_AddNSW: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildNSWAdd(builder, a, b, ""); } break;
        case OP_Sub: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildSub(builder, a, b, ""); } break;
        case OP_SubNUW: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildNUWSub(builder, a, b, ""); } break;
        case OP_SubNSW: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildNSWSub(builder, a, b, ""); } break;
        case OP_Mul: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildMul(builder, a, b, ""); } break;
        case OP_MulNUW: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildNUWMul(builder, a, b, ""); } break;
        case OP_MulNSW: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildNSWMul(builder, a, b, ""); } break;
        case OP_SDiv: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildSDiv(builder, a, b, ""); } break;
        case OP_UDiv: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildUDiv(builder, a, b, ""); } break;
        case OP_SRem: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildSRem(builder, a, b, ""); } break;
        case OP_URem: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildURem(builder, a, b, ""); } break;
        case OP_Shl: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildShl(builder, a, b, ""); } break;
        case OP_LShr: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildLShr(builder, a, b, ""); } break;
        case OP_AShr: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildAShr(builder, a, b, ""); } break;
        case OP_BAnd: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildAnd(builder, a, b, ""); } break;
        case OP_BOr: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildOr(builder, a, b, ""); } break;
        case OP_BXor: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildXor(builder, a, b, ""); } break;
        case OP_FAdd: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildFAdd(builder, a, b, ""); } break;
        case OP_FSub: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildFSub(builder, a, b, ""); } break;
        case OP_FMul: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildFMul(builder, a, b, ""); } break;
        case OP_FDiv: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildFDiv(builder, a, b, ""); } break;
        case OP_FRem: { READ_VALUE(a); READ_VALUE(b);
            return LLVMBuildFRem(builder, a, b, ""); } break;
        case OP_FMix: {
            READ_VALUE(a);
            READ_VALUE(b);
            READ_VALUE(x);
            LLVMValueRef one = build_matching_constant_real_vector(a, 1.0);
            auto invx = LLVMBuildFSub(builder, one, x, "");
            return LLVMBuildFAdd(builder,
                LLVMBuildFMul(builder, a, invx, ""),
                LLVMBuildFMul(builder, b, x, ""),
                "");
        } break;
        case FN_Length: {
            READ_VALUE(x);
            auto T = LLVMTypeOf(x);
            if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                return build_length_op(x);
            } else {
                LLVMValueRef func_fabs = get_intrinsic((T == f64T)?llvm_fabs_f64:llvm_fabs_f32);
                assert(func_fabs);
                LLVMValueRef values[] = { x };
                return LLVMBuildCall(builder, func_fabs, values, 1, "");
            }
        } break;
        case FN_Normalize: {
            READ_VALUE(x);
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
                return LLVMBuildFDiv(builder, x, l, "");
            } else {
                return LLVMConstReal(T, 1.0);
            }
        } break;
        case FN_Cross: {
            READ_VALUE(a);
            READ_VALUE(b);
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
            return LLVMBuildShuffleVector(builder, retvalue, retvalue, v120, "");
        } break;
        case OP_Step: {
            // select (lhs > rhs) (T 0) (T 1)
            READ_VALUE(a);
            READ_VALUE(b);
            LLVMValueRef one = build_matching_constant_real_vector(a, 1.0);
            LLVMValueRef zero = build_matching_constant_real_vector(b, 0.0);
            return LLVMBuildSelect(
                builder,
                LLVMBuildFCmp(builder, LLVMRealOGT, a, b, ""),
                zero, one, "");
        } break;
        // binops
        case OP_Pow: {
            READ_VALUE(a);
            READ_VALUE(b);
            auto T = LLVMTypeOf(a);
            auto ET = T;
            if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                ET = LLVMGetElementType(T);
            }
            LLVMValueRef func = nullptr;
            Intrinsic op = NumIntrinsics;
            switch(builtin.value()) {
            case OP_Pow: { op = (ET == f64T)?llvm_pow_f64:llvm_pow_f32; } break;
            default: break;
            }
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
        } break;
        // unops
        case OP_Sin:
        case OP_Cos:
        case OP_Sqrt:
        case OP_FAbs:
        case OP_FSign:
        case OP_Trunc:
        case OP_Exp:
        case OP_Log:
        case OP_Exp2:
        case OP_Log2:
        case OP_Floor:
        case OP_Radians:
        case OP_Degrees:
        { READ_VALUE(x);
            auto T = LLVMTypeOf(x);
            auto ET = T;
            if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                ET = LLVMGetElementType(T);
            }
            LLVMValueRef func = nullptr;
            Intrinsic op = NumIntrinsics;
            switch(builtin.value()) {
            case OP_Sin: { op = (ET == f64T)?llvm_sin_f64:llvm_sin_f32; } break;
            case OP_Cos: { op = (ET == f64T)?llvm_cos_f64:llvm_cos_f32; } break;
            case OP_Sqrt: { op = (ET == f64T)?llvm_sqrt_f64:llvm_sqrt_f32; } break;
            case OP_FAbs: { op = (ET == f64T)?llvm_fabs_f64:llvm_fabs_f32; } break;
            case OP_Trunc: { op = (ET == f64T)?llvm_trunc_f64:llvm_trunc_f32; } break;
            case OP_Floor: { op = (ET == f64T)?llvm_floor_f64:llvm_floor_f32; } break;
            case OP_Exp: { op = (ET == f64T)?llvm_exp_f64:llvm_exp_f32; } break;
            case OP_Log: { op = (ET == f64T)?llvm_log_f64:llvm_log_f32; } break;
            case OP_Exp2: { op = (ET == f64T)?llvm_exp2_f64:llvm_exp2_f32; } break;
            case OP_Log2: { op = (ET == f64T)?llvm_log2_f64:llvm_log2_f32; } break;
            case OP_FSign: { op = (ET == f64T)?custom_fsign_f64:custom_fsign_f32; } break;
            case OP_Radians: { op = (ET == f64T)?custom_radians_f64:custom_radians_f32; } break;
            case OP_Degrees: { op = (ET == f64T)?custom_degrees_f64:custom_degrees_f32; } break;
            default: break;
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
                return retvalue;
            } else {
                LLVMValueRef values[] = { x };
                return LLVMBuildCall(builder, func, values, 1, "");
            }
        } break;
        case SFXFN_Unreachable:
            return LLVMBuildUnreachable(builder);
        default: {
            SCOPES_ERROR(CGenUnsupportedBuiltin, builtin);
        } break;
        }
#undef READ_TYPE
#undef READ_VALUE
        return nullptr;
    }

    SCOPES_RESULT(void) translate_Call(const CallRef &call) {
        SCOPES_RESULT_TYPE(void);
        auto callee = call->callee;
        auto &&args = call->args;

        auto T = try_get_const_type(callee);
        const Type *rtype = callee->get_type();
        if (is_function_pointer(rtype)) {
            SCOPES_CHECK_RESULT(build_call(call,
                extract_function_type(rtype),
                SCOPES_GET_RESULT(ref_to_value(callee)), args));
            return {};
        } else if (T == TYPE_Builtin) {
            auto builtin = SCOPES_GET_RESULT(extract_builtin_constant(callee));
            auto result = SCOPES_GET_RESULT(translate_builtin(builtin, args));
            if (result) {
                map_phi({ result }, call);
            }
            return {};
        }
        SCOPES_ERROR(CGenInvalidCallee, callee->get_type());
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
        if (it != ref2value.end())
            return it->second;
        LLVMValueRef value = nullptr;
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
        return value;
    }

    SCOPES_RESULT(LLVMValueRef) PureCast_to_value(const PureCastRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        LLVMTypeRef LLT = SCOPES_GET_RESULT(type_to_llvm_type(node->get_type()));
        auto val = SCOPES_GET_RESULT(ref_to_value(ValueIndex(node->value)));
        return LLVMConstBitCast(val, LLT);
    }

    SCOPES_RESULT(LLVMValueRef) Global_to_value(const GlobalRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        auto it = global2global.find(node.unref());
        if (it == global2global.end()) {
            auto pi = cast<PointerType>(node->get_type());
            LLVMTypeRef LLT = SCOPES_GET_RESULT(type_to_llvm_type(pi->element_type));
            LLVMValueRef result = nullptr;
            if (node->storage_class == SYM_SPIRV_StorageClassPrivate) {
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
                const String *name = ss.str();
                result = LLVMAddGlobal(module, LLT, name->data);
                global2global.insert({ node.unref(), result });
                if (!generate_object) {
                    auto it = global_cache.find(node.unref());
                    if (it != global_cache.end()) {
                        assert(it->second != module);
                        return result;
                    } else {
                        global_cache.insert({node.unref(), module});
                    }
                }
                LLVMSetInitializer(result, LLVMConstNull(LLT));
                return result;
            } else {
                const String *namestr = node->name.name();
                const char *name = namestr->data;
                assert(name);
                if ((namestr->count > 5) && !strncmp(name, "llvm.", 5)) {
                    result = LLVMAddFunction(module, name, LLT);
                } else {
                    void *pptr = local_aware_dlsym(node->name);
                    uint64_t ptr = *(uint64_t*)&pptr;
                    if (!ptr) {
                        last_llvm_error = nullptr;
                        LLVMInstallFatalErrorHandler(fatal_error_handler);
                        ptr = get_address(name);
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
        return LLVMConstInt(T, node->value, false);
    }

    SCOPES_RESULT(LLVMValueRef) ConstReal_to_value(const ConstRealRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        auto T = SCOPES_GET_RESULT(type_to_llvm_type(node->get_type()));
        return LLVMConstReal(T, node->value);
    }

    SCOPES_RESULT(LLVMValueRef) ConstPointer_to_value(const ConstPointerRef &node) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        auto LLT = SCOPES_GET_RESULT(type_to_llvm_type(node->get_type()));
        if (!node->value) {
            return LLVMConstPointerNull(LLT);
        } else if (!generate_object) {
            return LLVMConstIntToPtr(
                LLVMConstInt(i64T, *(uint64_t*)&(node->value), false),
                LLT);
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
            auto ai = cast<ArrayType>(SCOPES_GET_RESULT(storage_type(node->get_type())));
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

        LLVMMetadataRef result = LLVMDIBuilderCreateDebugLocation(
            LLVMGetGlobalContext(),
            anchor->lineno, anchor->column, disp, nullptr);

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
        SCOPES_CHECK_RESULT(process_functions());

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
            if (entry) {
                stream_value(ss, entry);
            }
            LLVMDumpModule(module);
            SCOPES_ERROR(CGenBackendFailed, errmsg);
        }
        LLVMDisposeMessage(errmsg);
        return {};
    }

    typedef std::pair<LLVMModuleRef, LLVMValueRef> ModuleValuePair;

    // for generating object files
    SCOPES_RESULT(LLVMModuleRef) generate(const String *name, Scope *table) {
        SCOPES_RESULT_TYPE(LLVMModuleRef);

        setup_generate(name->data);

        Scope *t = table;
        while (t) {
            for (auto it = t->map->begin(); it != t->map->end(); ++it) {
                ValueRef val = it->second.expr;
                if (!val) continue;
                Symbol name = it->first;
                FunctionRef fn = SCOPES_GET_RESULT(extract_function_constant(val));
                func_export_table.insert({fn.unref(), name});

                SCOPES_CHECK_RESULT(ref_to_value(ValueIndex(fn)));
            }
            t = t->parent;
        }

        SCOPES_CHECK_RESULT(teardown_generate());
        return module;
    }

    SCOPES_RESULT(ModuleValuePair) generate(const FunctionRef &entry) {
        SCOPES_RESULT_TYPE(ModuleValuePair);

        const char *name = entry->name.name()->data;
        setup_generate(name);

        auto func = SCOPES_GET_RESULT(ref_to_value(ValueIndex(entry)));
        LLVMSetLinkage(func, LLVMExternalLinkage);

        SCOPES_CHECK_RESULT(teardown_generate(entry));

        return ModuleValuePair(module, func);
    }

};

Error *LLVMIRGenerator::last_llvm_error = nullptr;
std::unordered_map<const Type *, LLVMTypeRef> LLVMIRGenerator::type_cache;
std::unordered_map<Function *, LLVMModuleRef> LLVMIRGenerator::func_cache;
std::unordered_map<Global *, LLVMModuleRef> LLVMIRGenerator::global_cache;
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

#if 1
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
    llvm::ExecutionEngine *ee;
    DisassemblyListener(llvm::ExecutionEngine *_ee) : ee(_ee) {}

    std::unordered_map<void *, size_t> sizes;

    void InitializeDebugData(
        llvm::StringRef name,
        llvm::object::SymbolRef::Type type, uint64_t sz) {
        if(type == llvm::object::SymbolRef::ST_Function) {
            #if !defined(__arm__) && !defined(__linux__)
            name = name.substr(1);
            #endif
            void * addr = (void*)ee->getFunctionAddress(name);
            if(addr) {
                assert(addr);
                sizes[addr] = sz;
            }
        }
    }

    virtual void NotifyObjectEmitted(
        const llvm::object::ObjectFile &Obj,
        const llvm::RuntimeDyld::LoadedObjectInfo &L) {
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

SCOPES_RESULT(void) compile_object(const String *path, Scope *scope, uint64_t flags) {
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

    auto target_machine = get_target_machine();
    assert(target_machine);

    char *errormsg = nullptr;
    char *path_cstr = strdup(path->data);
    if (LLVMTargetMachineEmitToFile(target_machine, module, path_cstr,
        LLVMObjectFile, &errormsg)) {
        SCOPES_ERROR(CGenBackendFailed, errormsg);
    }
    free(path_cstr);
    return {};
}

#if 1
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

    init_execution();
    SCOPES_CHECK_RESULT(add_module(module));

#if 1
    if (!disassembly_listener && (flags & CF_DumpDisassembly)) {
        llvm::ExecutionEngine *pEE = reinterpret_cast<llvm::ExecutionEngine*>(get_execution_engine());
        disassembly_listener = new DisassemblyListener(pEE);
        pEE->RegisterJITEventListener(disassembly_listener);
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

#if 1
    for (auto sym : ctx.generated_symbols) {
        void *ptr = get_pointer_to_global(sym);
        size_t length = 0;
        const char *name = LLVMGetValueName2(sym, &length);
        set_address_name(ptr, String::from(name, length));
    }
#endif

    //LLVMDumpModule(module);
    void *pfunc = get_pointer_to_global(func);
#if 1
    if (flags & CF_DumpDisassembly) {
        assert(disassembly_listener);
        //auto td = LLVMGetExecutionEngineTargetData(ee);
        auto it = disassembly_listener->sizes.find(pfunc);
        if (it != disassembly_listener->sizes.end()) {
            std::cout << "disassembly:\n";
            auto target_machine = get_target_machine();
            do_disassemble(target_machine, pfunc, it->second);
        } else {
            std::cout << "no disassembly available\n";
        }
    }
#endif

    return ref(fn.anchor(), ConstPointer::from(functype, pfunc));
}

} // namespace scopes