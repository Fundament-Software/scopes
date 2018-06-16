/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "gen_llvm.hpp"
#include "source_file.hpp"
#include "types.hpp"
#include "label.hpp"
#include "platform_abi.hpp"
#include "anchor.hpp"
#include "error.hpp"
#include "execution.hpp"
#include "gc.hpp"
#include "stream_label.hpp"
#include "scope.hpp"
#include "timer.hpp"
#include "compiler_flags.hpp"
#include "hash.hpp"

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

#include "llvm/IR/Module.h"
//#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DIBuilder.h"
//#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/Object/SymbolSize.h"
//#include "llvm/Support/Timer.h"
//#include "llvm/Support/raw_os_ostream.h"

#include "dyn_cast.inc"

#pragma GCC diagnostic ignored "-Wvla-extension"

namespace scopes {

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

typedef llvm::DIBuilder *LLVMDIBuilderRef;

static LLVMDIBuilderRef LLVMCreateDIBuilder(LLVMModuleRef M) {
  return new llvm::DIBuilder(*llvm::unwrap(M));
}

static void LLVMDisposeDIBuilder(LLVMDIBuilderRef Builder) {
  Builder->finalize();
  delete Builder;
}

static llvm::MDNode *value_to_mdnode(LLVMValueRef value) {
    return value ? cast<llvm::MDNode>(
        llvm::unwrap<llvm::MetadataAsValue>(value)->getMetadata()) : nullptr;
}

template<typename T>
static T *value_to_DI(LLVMValueRef value) {
    return value ? cast<T>(
        llvm::unwrap<llvm::MetadataAsValue>(value)->getMetadata()) : nullptr;
}

static LLVMValueRef mdnode_to_value(llvm::MDNode *node) {
  return llvm::wrap(
    llvm::MetadataAsValue::get(*llvm::unwrap(LLVMGetGlobalContext()), node));
}

typedef llvm::DINode::DIFlags LLVMDIFlags;

static LLVMValueRef LLVMDIBuilderCreateSubroutineType(
    LLVMDIBuilderRef Builder, LLVMValueRef ParameterTypes) {
    return mdnode_to_value(
        Builder->createSubroutineType(value_to_DI<llvm::MDTuple>(ParameterTypes)));
}

static LLVMValueRef LLVMDIBuilderCreateCompileUnit(LLVMDIBuilderRef Builder,
    unsigned Lang,
    const char *File, const char *Dir, const char *Producer, bool isOptimized,
    const char *Flags, unsigned RV, const char *SplitName,
    //DICompileUnit::DebugEmissionKind Kind,
    uint64_t DWOId) {
    auto ctx = (llvm::LLVMContext *)LLVMGetGlobalContext();
    auto file = llvm::DIFile::get(*ctx, File, Dir);
    return mdnode_to_value(
        Builder->createCompileUnit(Lang, file,
                      Producer, isOptimized, Flags,
                      RV, SplitName,
                      llvm::DICompileUnit::DebugEmissionKind::FullDebug,
                      //llvm::DICompileUnit::DebugEmissionKind::LineTablesOnly,
                      DWOId));
}

static LLVMValueRef LLVMDIBuilderCreateFunction(
    LLVMDIBuilderRef Builder, LLVMValueRef Scope, const char *Name,
    const char *LinkageName, LLVMValueRef File, unsigned LineNo,
    LLVMValueRef Ty, bool IsLocalToUnit, bool IsDefinition,
    unsigned ScopeLine) {
  return mdnode_to_value(Builder->createFunction(
        cast<llvm::DIScope>(value_to_mdnode(Scope)), Name, LinkageName,
        cast<llvm::DIFile>(value_to_mdnode(File)),
        LineNo, cast<llvm::DISubroutineType>(value_to_mdnode(Ty)),
        IsLocalToUnit, IsDefinition, ScopeLine));
}

static LLVMValueRef LLVMGetFunctionSubprogram(LLVMValueRef func) {
    return mdnode_to_value(
        llvm::cast<llvm::Function>(llvm::unwrap(func))->getSubprogram());
}

static void LLVMSetFunctionSubprogram(LLVMValueRef func, LLVMValueRef subprogram) {
    llvm::cast<llvm::Function>(llvm::unwrap(func))->setSubprogram(
        value_to_DI<llvm::DISubprogram>(subprogram));
}

#if 0
static LLVMValueRef LLVMDIBuilderCreateLexicalBlock(LLVMDIBuilderRef Builder,
    LLVMValueRef Scope, LLVMValueRef File, unsigned Line, unsigned Col) {
    return mdnode_to_value(Builder->createLexicalBlock(
        value_to_DI<llvm::DIScope>(Scope),
        value_to_DI<llvm::DIFile>(File), Line, Col));
}
#endif

static LLVMValueRef LLVMCreateDebugLocation(unsigned Line,
                                     unsigned Col, const LLVMValueRef Scope,
                                     const LLVMValueRef InlinedAt) {
  llvm::MDNode *SNode = value_to_mdnode(Scope);
  llvm::MDNode *INode = value_to_mdnode(InlinedAt);
  return mdnode_to_value(llvm::DebugLoc::get(Line, Col, SNode, INode).get());
}

static LLVMValueRef LLVMDIBuilderCreateFile(
    LLVMDIBuilderRef Builder, const char *Filename,
                            const char *Directory) {
  return mdnode_to_value(Builder->createFile(Filename, Directory));
}

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
        NumIntrinsics,
    };

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

    std::unordered_map<Label *, LLVMValueRef> label2md;
    std::unordered_map<SourceFile *, LLVMValueRef> file2value;
    std::unordered_map< ParamKey, LLVMValueRef, HashFuncParamPair> param2value;
    static std::unordered_map<const Type *, LLVMTypeRef> type_cache;
    static ArgTypes type_todo;

    std::unordered_map<Any, LLVMValueRef, Any::Hash> extern2global;
    std::unordered_map<void *, LLVMValueRef> ptr2global;

    Label::UserMap user_map;

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
    static LLVMAttributeRef attr_byval;
    static LLVMAttributeRef attr_sret;
    static LLVMAttributeRef attr_nonnull;
    LLVMValueRef intrinsics[NumIntrinsics];

    Label *active_function;
    LLVMValueRef active_function_value;

    bool use_debug_info;
    bool inline_pointers;

    template<unsigned N>
    static LLVMAttributeRef get_attribute(const char (&s)[N]) {
        unsigned kind = LLVMGetEnumAttributeKindForName(s, N - 1);
        assert(kind);
        return LLVMCreateEnumAttribute(LLVMGetGlobalContext(), kind, 0);
    }

    LLVMIRGenerator() :
        active_function(nullptr),
        active_function_value(nullptr),
        use_debug_info(true),
        inline_pointers(true) {
        static_init();
        for (int i = 0; i < NumIntrinsics; ++i) {
            intrinsics[i] = nullptr;
        }
    }

    LLVMValueRef source_file_to_scope(SourceFile *sf) {
        assert(use_debug_info);

        auto it = file2value.find(sf);
        if (it != file2value.end())
            return it->second;

        char *dn = strdup(sf->path.name()->data);
        char *bn = strdup(dn);

        LLVMValueRef result = LLVMDIBuilderCreateFile(di_builder,
            basename(bn), dirname(dn));
        free(dn);
        free(bn);

        file2value.insert({ sf, result });

        return result;
    }

    LLVMValueRef label_to_subprogram(Label *l) {
        assert(use_debug_info);

        auto it = label2md.find(l);
        if (it != label2md.end())
            return it->second;

        const Anchor *anchor = l->anchor;

        LLVMValueRef difile = source_file_to_scope(anchor->file);

        LLVMValueRef subroutinevalues[] = {
            nullptr
        };
        LLVMValueRef disrt = LLVMDIBuilderCreateSubroutineType(di_builder,
            LLVMMDNode(subroutinevalues, 1));

        LLVMValueRef difunc = LLVMDIBuilderCreateFunction(
            di_builder, difile, l->name.name()->data, l->name.name()->data,
            difile, anchor->lineno, disrt, false, true,
            anchor->lineno);

        label2md.insert({ l, difunc });
        return difunc;
    }

    LLVMValueRef anchor_to_location(const Anchor *anchor) {
        assert(use_debug_info);

        //auto old_bb = LLVMGetInsertBlock(builder);
        //LLVMValueRef func = LLVMGetBasicBlockParent(old_bb);
        LLVMValueRef disp = LLVMGetFunctionSubprogram(active_function_value);

        LLVMValueRef result = LLVMCreateDebugLocation(
            anchor->lineno, anchor->column, disp, nullptr);

        return result;
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
        LLVMPositionBuilderAtEnd(builder, bb);
#define LLVM_INTRINSIC_IMPL_END() \
        LLVMPositionBuilderAtEnd(builder, oldbb); \
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
        attr_byval = get_attribute("byval");
        attr_sret = get_attribute("sret");
        attr_nonnull = get_attribute("nonnull");

        LLVMContextSetDiagnosticHandler(LLVMGetGlobalContext(),
            diag_handler,
            nullptr);

    }

#undef DEFINE_BUILTIN

    static bool all_parameters_lowered(Label *label) {
        for (auto &&param : label->params) {
            if (param->kind != PK_Regular)
                return false;
            //if ((param->type == TYPE_Type) || (param->type == TYPE_Label))
            //    return false;
            if (isa<ReturnLabelType>(param->type) && (param->index != 0))
                return false;
        }
        return true;
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

    SCOPES_RESULT(LLVMValueRef) abi_import_argument(Parameter *param, LLVMValueRef func, size_t &k) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        ABIClass classes[MAX_ABI_CLASSES];
        size_t sz = abi_classify(param->type, classes);
        if (!sz) {
            LLVMValueRef val = LLVMGetParam(func, k++);
            return LLVMBuildLoad(builder, val, "");
        }
        LLVMTypeRef T = SCOPES_GET_RESULT(type_to_llvm_type(param->type));
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
        return val;
    }

    SCOPES_RESULT(void) abi_export_argument(LLVMValueRef val, const Type *AT,
        std::vector<LLVMValueRef> &values, std::vector<size_t> &memptrs) {
        SCOPES_RESULT_TYPE(void);
        ABIClass classes[MAX_ABI_CLASSES];
        size_t sz = abi_classify(AT, classes);
        if (!sz) {
            LLVMValueRef ptrval = safe_alloca(SCOPES_GET_RESULT(type_to_llvm_type(AT)));
            LLVMBuildStore(builder, val, ptrval);
            val = ptrval;
            memptrs.push_back(values.size());
            values.push_back(val);
            return true;
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
                return true;
            }
        }
        values.push_back(val);
        return true;
    }

    static SCOPES_RESULT(void) abi_transform_parameter(const Type *AT,
        std::vector<LLVMTypeRef> &params) {
        SCOPES_RESULT_TYPE(void);
        ABIClass classes[MAX_ABI_CLASSES];
        size_t sz = abi_classify(AT, classes);
        auto T = SCOPES_GET_RESULT(type_to_llvm_type(AT));
        if (!sz) {
            params.push_back(LLVMPointerType(T, 0));
            return true;
        }
        auto tk = LLVMGetTypeKind(T);
        if (tk == LLVMStructTypeKind) {
            auto ST = abi_struct_type(classes, sz);
            if (ST) {
                for (size_t i = 0; i < sz; ++i) {
                    params.push_back(LLVMStructGetTypeAtIndex(ST, i));
                }
                return true;
            }
        }
        params.push_back(T);
        return true;
    }

    static SCOPES_RESULT(LLVMTypeRef) create_llvm_type(const Type *type) {
        SCOPES_RESULT_TYPE(LLVMTypeRef);
        switch(type->kind()) {
        case TK_Integer:
            return LLVMIntType(cast<IntegerType>(type)->width);
        case TK_Real:
            switch(cast<RealType>(type)->width) {
            case 32: return f32T;
            case 64: return f64T;
            default: break;
            }
            break;
        case TK_Extern: {
            return LLVMPointerType(
                SCOPES_GET_RESULT(_type_to_llvm_type(cast<ExternType>(type)->type)), 0);
        } break;
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
        case TK_Tuple: {
            auto ti = cast<TupleType>(type);
            size_t count = ti->types.size();
            LLVMTypeRef elements[count];
            for (size_t i = 0; i < count; ++i) {
                elements[i] = SCOPES_GET_RESULT(_type_to_llvm_type(ti->types[i]));
            }
            return LLVMStructType(elements, count, ti->packed);
        } break;
        case TK_Union: {
            auto ui = cast<UnionType>(type);
            return _type_to_llvm_type(ui->tuple_type);
        } break;
        case TK_Typename: {
            if (type == TYPE_Void)
                return LLVMVoidType();
            else if (type == TYPE_Sampler) {
                SCOPES_LOCATION_ERROR(String::from(
                    "sampler type can not be used for native target"));
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
        case TK_ReturnLabel: {
            auto rlt = cast<ReturnLabelType>(type);
            return _type_to_llvm_type(rlt->return_type);
        } break;
        case TK_Function: {
            auto fi = cast<FunctionType>(type);
            size_t count = fi->argument_types.size();
            bool use_sret = is_memory_class(fi->return_type);

            std::vector<LLVMTypeRef> elements;
            elements.reserve(count);
            LLVMTypeRef rettype;
            if (use_sret) {
                elements.push_back(
                    LLVMPointerType(SCOPES_GET_RESULT(_type_to_llvm_type(fi->return_type)), 0));
                rettype = voidT;
            } else {
                rettype = SCOPES_GET_RESULT(_type_to_llvm_type(fi->return_type));
            }
            for (size_t i = 0; i < count; ++i) {
                auto AT = fi->argument_types[i];
                SCOPES_CHECK_RESULT(abi_transform_parameter(AT, elements));
            }
            return LLVMFunctionType(rettype,
                &elements[0], elements.size(), fi->vararg());
        } break;
        case TK_SampledImage: {
            SCOPES_LOCATION_ERROR(String::from(
                "sampled image type can not be used for native target"));
        } break;
        case TK_Image: {
            SCOPES_LOCATION_ERROR(String::from(
                "image type can not be used for native target"));
        } break;
        };

        StyledString ss;
        ss.out << "IL->IR: cannot convert type " << type;
        SCOPES_LOCATION_ERROR(ss.str());
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
                size_t count = ti->types.size();
                LLVMTypeRef elements[count];
                for (size_t i = 0; i < count; ++i) {
                    elements[i] = SCOPES_GET_RESULT(_type_to_llvm_type(ti->types[i]));
                }
                LLVMStructSetBody(LLT, elements, count, false);
            } break;
            case TK_Union: {
                auto ui = cast<UnionType>(ST);
                size_t count = ui->types.size();
                size_t sz = ui->size;
                size_t al = ui->align;
                // find member with the same alignment
                for (size_t i = 0; i < count; ++i) {
                    const Type *ET = ui->types[i];
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

    SCOPES_RESULT(LLVMValueRef) label_to_value(Label *label) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        if (label->is_basic_block_like()) {
            auto bb = SCOPES_GET_RESULT(label_to_basic_block(label));
            if (!bb) return nullptr;
            else
                return LLVMBasicBlockAsValue(bb);
        } else {
            return label_to_function(label);
        }
    }

    static bool ok;
    static void fatal_error_handler(const char *Reason) {
        set_last_location_error(String::from_cstr(Reason));
        ok = false;
    }

    void bind_parameter(Parameter *param, LLVMValueRef value) {
        assert(value);
        param2value[{active_function_value, param}] = value;
    }

    bool parameter_is_bound(Parameter *param) {
        return param2value.find({active_function_value, param}) != param2value.end();
    }

    SCOPES_RESULT(LLVMValueRef) resolve_parameter(Parameter *param) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        auto it = param2value.find({active_function_value, param});
        if (it == param2value.end()) {
            assert(active_function_value);
            if (param->label) {
                location_message(param->label->anchor, String::from("declared here"));
            }
            StyledString ss;
            ss.out << "IL->IR: can't access free variable " << param;
            SCOPES_LOCATION_ERROR(ss.str());
        }
        assert(it->second);
        return it->second;
    }

    SCOPES_RESULT(LLVMValueRef) argument_to_value(Any value) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        if (value.type == TYPE_Parameter) {
            return resolve_parameter(value.parameter);
        }
        switch(value.type->kind()) {
        case TK_Integer: {
            auto it = cast<IntegerType>(value.type);
            if (it->issigned) {
                switch(it->width) {
                case 8: return LLVMConstInt(i8T, value.i8, true);
                case 16: return LLVMConstInt(i16T, value.i16, true);
                case 32: return LLVMConstInt(i32T, value.i32, true);
                case 64: return LLVMConstInt(i64T, value.i64, true);
                default: break;
                }
            } else {
                switch(it->width) {
                case 1: return LLVMConstInt(i1T, value.i1, false);
                case 8: return LLVMConstInt(i8T, value.u8, false);
                case 16: return LLVMConstInt(i16T, value.u16, false);
                case 32: return LLVMConstInt(i32T, value.u32, false);
                case 64: return LLVMConstInt(i64T, value.u64, false);
                default: break;
                }
            }
        } break;
        case TK_Real: {
            auto rt = cast<RealType>(value.type);
            switch(rt->width) {
            case 32: return LLVMConstReal(f32T, value.f32);
            case 64: return LLVMConstReal(f64T, value.f64);
            default: break;
            }
        } break;
        case TK_Extern: {
            auto it = extern2global.find(value);
            if (it == extern2global.end()) {
                const String *namestr = value.symbol.name();
                const char *name = namestr->data;
                assert(name);
                auto et = cast<ExternType>(value.type);
                LLVMTypeRef LLT = SCOPES_GET_RESULT(type_to_llvm_type(et->type));
                LLVMValueRef result = nullptr;
                if ((namestr->count > 5) && !strncmp(name, "llvm.", 5)) {
                    result = LLVMAddFunction(module, name, LLT);
                } else {
                    void *pptr = local_aware_dlsym(value.symbol);
                    uint64_t ptr = *(uint64_t*)&pptr;
                    if (!ptr) {
                        ok = true;
                        LLVMInstallFatalErrorHandler(fatal_error_handler);
                        ptr = get_address(name);
                        LLVMResetFatalErrorHandler();
                        SCOPES_CHECK_OK(ok);
                    }
                    if (!ptr) {
                        StyledString ss;
                        ss.out << "could not resolve " << value;
                        SCOPES_LOCATION_ERROR(ss.str());
                    }
                    result = LLVMAddGlobal(module, LLT, name);
                }
                extern2global.insert({ value, result });
                return result;
            } else {
                return it->second;
            }
        } break;
        case TK_Pointer: {
            LLVMTypeRef LLT = SCOPES_GET_RESULT(type_to_llvm_type(value.type));
            if (!value.pointer) {
                return LLVMConstPointerNull(LLT);
            } else if (inline_pointers) {
                return LLVMConstIntToPtr(
                    LLVMConstInt(i64T, *(uint64_t*)&value.pointer, false),
                    LLT);
            } else {
                // to serialize a pointer, we serialize the allocation range
                // of the pointer as a global binary blob
                void *baseptr;
                size_t alloc_size;
                if (!find_allocation(value.pointer, baseptr, alloc_size)) {
                    StyledString ss;
                    ss.out << "IL->IR: constant pointer of type " << value.type
                        << " points to unserializable memory";
                    SCOPES_LOCATION_ERROR(ss.str());
                }
                LLVMValueRef basevalue = nullptr;
                auto it = ptr2global.find(baseptr);

                auto pi = cast<PointerType>(value.type);
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
                size_t offset = (uint8_t*)value.pointer - (uint8_t*)baseptr;
                LLVMValueRef indices[2];
                indices[0] = LLVMConstInt(i64T, 0, false);
                indices[1] = LLVMConstInt(i64T, offset, false);
                return LLVMConstPointerCast(
                    LLVMConstGEP(basevalue, indices, 2), LLT);
            }
        } break;
        case TK_Typename: {
            LLVMTypeRef LLT = SCOPES_GET_RESULT(type_to_llvm_type(value.type));
            auto tn = cast<TypenameType>(value.type);
            switch(tn->storage_type->kind()) {
            case TK_Tuple: {
                auto ti = cast<TupleType>(tn->storage_type);
                size_t count = ti->types.size();
                LLVMValueRef values[count];
                for (size_t i = 0; i < count; ++i) {
                    values[i] = SCOPES_GET_RESULT(argument_to_value(SCOPES_GET_RESULT(ti->unpack(value.pointer, i))));
                }
                return LLVMConstNamedStruct(LLT, values, count);
            } break;
            default: {
                Any storage_value = value;
                storage_value.type = tn->storage_type;
                LLVMValueRef val = SCOPES_GET_RESULT(argument_to_value(storage_value));
                return LLVMConstBitCast(val, LLT);
            } break;
            }
        } break;
        case TK_Array: {
            auto ai = cast<ArrayType>(value.type);
            size_t count = ai->count;
            LLVMValueRef values[count];
            for (size_t i = 0; i < count; ++i) {
                values[i] = SCOPES_GET_RESULT(argument_to_value(SCOPES_GET_RESULT(ai->unpack(value.pointer, i))));
            }
            return LLVMConstArray(SCOPES_GET_RESULT(type_to_llvm_type(ai->element_type)),
                values, count);
        } break;
        case TK_Vector: {
            auto vi = cast<VectorType>(value.type);
            size_t count = vi->count;
            LLVMValueRef values[count];
            for (size_t i = 0; i < count; ++i) {
                values[i] = SCOPES_GET_RESULT(argument_to_value(SCOPES_GET_RESULT(vi->unpack(value.pointer, i))));
            }
            return LLVMConstVector(values, count);
        } break;
        case TK_Tuple: {
            auto ti = cast<TupleType>(value.type);
            size_t count = ti->types.size();
            LLVMValueRef values[count];
            for (size_t i = 0; i < count; ++i) {
                values[i] = SCOPES_GET_RESULT(argument_to_value(SCOPES_GET_RESULT(ti->unpack(value.pointer, i))));
            }
            return LLVMConstStruct(values, count, false);
        } break;
        case TK_Union: {
            auto ui = cast<UnionType>(value.type);
            value.type = ui->tuple_type;
            return SCOPES_GET_RESULT(argument_to_value(value));
        } break;
        default: break;
        };

        StyledString ss;
        ss.out << "IL->IR: cannot convert argument of type " << value.type;
        SCOPES_LOCATION_ERROR(ss.str());
    }

    struct ReturnTraits {
        bool multiple_return_values;
        bool terminated;
        const ReturnLabelType *rtype;

        ReturnTraits() :
            multiple_return_values(false),
            terminated(false),
            rtype(nullptr) {}
    };

    SCOPES_RESULT(LLVMValueRef) build_call(const Type *functype, LLVMValueRef func, Args &args,
        ReturnTraits &traits) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        size_t argcount = args.size() - 1;

        auto fi = cast<FunctionType>(functype);

        bool use_sret = is_memory_class(fi->return_type);

        std::vector<LLVMValueRef> values;
        values.reserve(argcount + 1);

        if (use_sret) {
            values.push_back(safe_alloca(SCOPES_GET_RESULT(_type_to_llvm_type(fi->return_type))));
        }
        std::vector<size_t> memptrs;
        for (size_t i = 0; i < argcount; ++i) {
            auto &&arg = args[i + 1];
            LLVMValueRef val = SCOPES_GET_RESULT(argument_to_value(arg.value));
            auto AT = arg.value.indirect_type();
            SCOPES_GET_RESULT(abi_export_argument(val, AT, values, memptrs));
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

        auto ret = LLVMBuildCall(builder, func, &values[0], values.size(), "");
        for (auto idx : memptrs) {
            auto i = idx + 1;
            LLVMAddCallSiteAttribute(ret, i, attr_nonnull);
        }
        auto rlt = cast<ReturnLabelType>(fi->return_type);
        traits.rtype = rlt;
        traits.multiple_return_values = rlt->has_multiple_return_values();
        if (use_sret) {
            LLVMAddCallSiteAttribute(ret, 1, attr_sret);
            return LLVMBuildLoad(builder, values[0], "");
        } else if (!rlt->is_returning()) {
            LLVMBuildUnreachable(builder);
            traits.terminated = true;
            return nullptr;
        } else if (rlt->return_type == TYPE_Void) {
            return nullptr;
        } else {
            return ret;
        }
    }

    LLVMValueRef set_debug_location(Label *label) {
        assert(use_debug_info);
        LLVMValueRef diloc = anchor_to_location(label->body.anchor);
        LLVMSetCurrentDebugLocation(builder, diloc);
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
#if 0
            // add allocas at the tail
            auto oldbb = LLVMGetInsertBlock(builder);
            auto entry = LLVMGetEntryBasicBlock(active_function_value);
            auto term = LLVMGetBasicBlockTerminator(entry);
            if (term) {
                LLVMPositionBuilderBefore(builder, term);
            } else {
                LLVMPositionBuilderAtEnd(builder, entry);
            }
            LLVMValueRef result;
            if (val) {
                result = LLVMBuildArrayAlloca(builder, ty, val, "");
            } else {
                result = LLVMBuildAlloca(builder, ty, "");
            }
            LLVMPositionBuilderAtEnd(builder, oldbb);
            return result;
#elif 1
            // add allocas to the front
            auto oldbb = LLVMGetInsertBlock(builder);
            auto entry = LLVMGetEntryBasicBlock(active_function_value);
            auto instr = LLVMGetFirstInstruction(entry);
            if (instr) {
                LLVMPositionBuilderBefore(builder, instr);
            } else {
                LLVMPositionBuilderAtEnd(builder, entry);
            }
            LLVMValueRef result;
            if (val) {
                result = LLVMBuildArrayAlloca(builder, ty, val, "");
            } else {
                result = LLVMBuildAlloca(builder, ty, "");
                //LLVMSetAlignment(result, 16);
            }
            LLVMPositionBuilderAtEnd(builder, oldbb);
            return result;
#else
            // add allocas locally
            LLVMValueRef result;
            if (val) {
                result = LLVMBuildArrayAlloca(builder, ty, val, "");
            } else {
                result = LLVMBuildAlloca(builder, ty, "");
            }
            return result;
#endif
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

    SCOPES_RESULT(void) write_label_body(Label *label) {
    repeat:
        SCOPES_RESULT_TYPE(void);
        if (!label->body.is_complete()) {
            set_active_anchor(label->body.anchor);
            SCOPES_LOCATION_ERROR(String::from("IL->IR: incomplete label body encountered"));
        }
#if SCOPES_DEBUG_CODEGEN
        {
            StyledStream ss(std::cout);
            std::cout << "generating LLVM for label:" << std::endl;
            stream_label(ss, label, StreamLabelFormat::debug_single());
            std::cout << std::endl;
        }
#endif
        auto &&body = label->body;
        auto &&enter = body.enter;
        auto &&args = body.args;

        set_active_anchor(label->body.anchor);

        LLVMValueRef diloc = nullptr;
        if (use_debug_info) {
            diloc = set_debug_location(label);
        }

        assert(!args.empty());
        size_t argcount = args.size() - 1;
        size_t argn = 1;
#define READ_ANY(NAME) \
        assert(argn <= argcount); \
        Any &NAME = args[argn++].value;
#define READ_VALUE(NAME) \
        assert(argn <= argcount); \
        LLVMValueRef NAME = SCOPES_GET_RESULT(argument_to_value(args[argn++].value));
#define READ_LABEL_VALUE(NAME) \
        assert(argn <= argcount); \
        LLVMValueRef NAME = SCOPES_GET_RESULT(label_to_value(args[argn++].value)); \
        assert(NAME);
#define READ_TYPE(NAME) \
        assert(argn <= argcount); \
        assert(args[argn].value.type == TYPE_Type); \
        LLVMTypeRef NAME = SCOPES_GET_RESULT(type_to_llvm_type(args[argn++].value.typeref));

        LLVMValueRef retvalue = nullptr;
        ReturnTraits rtraits;
        if (enter.type == TYPE_Builtin) {
            switch(enter.builtin.value()) {
            case FN_Branch: {
                READ_VALUE(cond);
                READ_LABEL_VALUE(then_block);
                READ_LABEL_VALUE(else_block);
                assert(LLVMValueIsBasicBlock(then_block));
                assert(LLVMValueIsBasicBlock(else_block));
                LLVMBuildCondBr(builder, cond,
                    LLVMValueAsBasicBlock(then_block),
                    LLVMValueAsBasicBlock(else_block));
                rtraits.terminated = true;
            } break;
            case OP_Tertiary: {
                READ_VALUE(cond);
                READ_VALUE(then_value);
                READ_VALUE(else_value);
                retvalue = LLVMBuildSelect(
                    builder, cond, then_value, else_value, "");
            } break;
            case FN_Unconst: {
                READ_ANY(val);
                if (val.type == TYPE_Label) {
                    retvalue = SCOPES_GET_RESULT(label_to_function(val));
                } else {
                    retvalue = SCOPES_GET_RESULT(argument_to_value(val));
                }
            } break;
            case FN_ExtractValue: {
                READ_VALUE(val);
                READ_ANY(index);
                retvalue = LLVMBuildExtractValue(
                    builder, val, SCOPES_GET_RESULT(cast_number<int32_t>(index)), "");
            } break;
            case FN_InsertValue: {
                READ_VALUE(val);
                READ_VALUE(eltval);
                READ_ANY(index);
                retvalue = LLVMBuildInsertValue(
                    builder, val, eltval, SCOPES_GET_RESULT(cast_number<int32_t>(index)), "");
            } break;
            case FN_ExtractElement: {
                READ_VALUE(val);
                READ_VALUE(index);
                retvalue = LLVMBuildExtractElement(builder, val, index, "");
            } break;
            case FN_InsertElement: {
                READ_VALUE(val);
                READ_VALUE(eltval);
                READ_VALUE(index);
                retvalue = LLVMBuildInsertElement(builder, val, eltval, index, "");
            } break;
            case FN_ShuffleVector: {
                READ_VALUE(v1);
                READ_VALUE(v2);
                READ_VALUE(mask);
                retvalue = LLVMBuildShuffleVector(builder, v1, v2, mask, "");
            } break;
            case FN_Undef: { READ_TYPE(ty);
                retvalue = LLVMGetUndef(ty); } break;
            case FN_Alloca: { READ_TYPE(ty);
                retvalue = safe_alloca(ty);
            } break;
            case FN_AllocaArray: { READ_TYPE(ty); READ_VALUE(val);
                retvalue = safe_alloca(ty, val); } break;
            case FN_AllocaOf: {
                READ_VALUE(val);
                retvalue = safe_alloca(LLVMTypeOf(val));
                LLVMBuildStore(builder, val, retvalue);
            } break;
            case FN_Malloc: { READ_TYPE(ty);
                retvalue = LLVMBuildMalloc(builder, ty, ""); } break;
            case FN_MallocArray: { READ_TYPE(ty); READ_VALUE(val);
                retvalue = LLVMBuildArrayMalloc(builder, ty, val, ""); } break;
            case FN_Free: { READ_VALUE(val);
                LLVMBuildFree(builder, val);
                retvalue = nullptr; } break;
            case FN_GetElementPtr: {
                READ_VALUE(pointer);
                assert(argcount > 1);
                size_t count = argcount - 1;
                LLVMValueRef indices[count];
                for (size_t i = 0; i < count; ++i) {
                    indices[i] = SCOPES_GET_RESULT(argument_to_value(args[argn + i].value));
                }
                retvalue = LLVMBuildGEP(builder, pointer, indices, count, "");
            } break;
            case FN_Bitcast: { READ_VALUE(val); READ_TYPE(ty);
                auto T = LLVMTypeOf(val);
                if (T == ty) {
                    retvalue = val;
                } else if (LLVMGetTypeKind(ty) == LLVMStructTypeKind) {
                    // completely braindead, but what can you do
                    LLVMValueRef ptr = safe_alloca(T);
                    LLVMBuildStore(builder, val, ptr);
                    ptr = LLVMBuildBitCast(builder, ptr, LLVMPointerType(ty,0), "");
                    retvalue = LLVMBuildLoad(builder, ptr, "");
                } else {
                    retvalue = LLVMBuildBitCast(builder, val, ty, "");
                }
            } break;
            case FN_IntToPtr: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildIntToPtr(builder, val, ty, ""); } break;
            case FN_PtrToInt: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildPtrToInt(builder, val, ty, ""); } break;
            case FN_ITrunc: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildTrunc(builder, val, ty, ""); } break;
            case FN_SExt: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildSExt(builder, val, ty, ""); } break;
            case FN_ZExt: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildZExt(builder, val, ty, ""); } break;
            case FN_FPTrunc: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildFPTrunc(builder, val, ty, ""); } break;
            case FN_FPExt: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildFPExt(builder, val, ty, ""); } break;
            case FN_FPToUI: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildFPToUI(builder, val, ty, ""); } break;
            case FN_FPToSI: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildFPToSI(builder, val, ty, ""); } break;
            case FN_UIToFP: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildUIToFP(builder, val, ty, ""); } break;
            case FN_SIToFP: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildSIToFP(builder, val, ty, ""); } break;
            case FN_VolatileLoad:
            case FN_Load: { READ_VALUE(ptr);
                retvalue = LLVMBuildLoad(builder, ptr, "");
                if (enter.builtin.value() == FN_VolatileLoad) { LLVMSetVolatile(retvalue, true); }
            } break;
            case FN_VolatileStore:
            case FN_Store: { READ_VALUE(val); READ_VALUE(ptr);
                retvalue = LLVMBuildStore(builder, val, ptr);
                if (enter.builtin.value() == FN_VolatileStore) { LLVMSetVolatile(retvalue, true); }
                retvalue = nullptr;
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
                switch(enter.builtin.value()) {
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
                retvalue = LLVMBuildICmp(builder, pred, a, b, "");
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
                switch(enter.builtin.value()) {
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
                retvalue = LLVMBuildFCmp(builder, pred, a, b, "");
            } break;
            case OP_Add: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildAdd(builder, a, b, ""); } break;
            case OP_AddNUW: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildNUWAdd(builder, a, b, ""); } break;
            case OP_AddNSW: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildNSWAdd(builder, a, b, ""); } break;
            case OP_Sub: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildSub(builder, a, b, ""); } break;
            case OP_SubNUW: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildNUWSub(builder, a, b, ""); } break;
            case OP_SubNSW: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildNSWSub(builder, a, b, ""); } break;
            case OP_Mul: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildMul(builder, a, b, ""); } break;
            case OP_MulNUW: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildNUWMul(builder, a, b, ""); } break;
            case OP_MulNSW: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildNSWMul(builder, a, b, ""); } break;
            case OP_SDiv: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildSDiv(builder, a, b, ""); } break;
            case OP_UDiv: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildUDiv(builder, a, b, ""); } break;
            case OP_SRem: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildSRem(builder, a, b, ""); } break;
            case OP_URem: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildURem(builder, a, b, ""); } break;
            case OP_Shl: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildShl(builder, a, b, ""); } break;
            case OP_LShr: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildLShr(builder, a, b, ""); } break;
            case OP_AShr: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildAShr(builder, a, b, ""); } break;
            case OP_BAnd: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildAnd(builder, a, b, ""); } break;
            case OP_BOr: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildOr(builder, a, b, ""); } break;
            case OP_BXor: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildXor(builder, a, b, ""); } break;
            case OP_FAdd: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildFAdd(builder, a, b, ""); } break;
            case OP_FSub: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildFSub(builder, a, b, ""); } break;
            case OP_FMul: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildFMul(builder, a, b, ""); } break;
            case OP_FDiv: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildFDiv(builder, a, b, ""); } break;
            case OP_FRem: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildFRem(builder, a, b, ""); } break;
            case OP_FMix: {
                READ_VALUE(a);
                READ_VALUE(b);
                READ_VALUE(x);
                LLVMValueRef one = build_matching_constant_real_vector(a, 1.0);
                auto invx = LLVMBuildFSub(builder, one, x, "");
                retvalue = LLVMBuildFAdd(builder,
                    LLVMBuildFMul(builder, a, invx, ""),
                    LLVMBuildFMul(builder, b, x, ""),
                    "");
            } break;
            case FN_Length: {
                READ_VALUE(x);
                auto T = LLVMTypeOf(x);
                if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                    retvalue = build_length_op(x);
                } else {
                    LLVMValueRef func_fabs = get_intrinsic((T == f64T)?llvm_fabs_f64:llvm_fabs_f32);
                    assert(func_fabs);
                    LLVMValueRef values[] = { x };
                    retvalue = LLVMBuildCall(builder, func_fabs, values, 1, "");
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
                    retvalue = LLVMBuildFDiv(builder, x, l, "");
                } else {
                    retvalue = LLVMConstReal(T, 1.0);
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
                retvalue = LLVMBuildFSub(builder,
                    LLVMBuildFMul(builder, a, b120, ""),
                    LLVMBuildFMul(builder, b, a120, ""), "");
                retvalue = LLVMBuildShuffleVector(builder, retvalue, retvalue, v120, "");
            } break;
            case OP_Step: {
                // select (lhs > rhs) (T 0) (T 1)
                READ_VALUE(a);
                READ_VALUE(b);
                LLVMValueRef one = build_matching_constant_real_vector(a, 1.0);
                LLVMValueRef zero = build_matching_constant_real_vector(b, 0.0);
                retvalue = LLVMBuildSelect(
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
                switch(enter.builtin.value()) {
                case OP_Pow: { op = (ET == f64T)?llvm_pow_f64:llvm_pow_f32; } break;
                default: break;
                }
                func = get_intrinsic(op);
                assert(func);
                if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                    auto count = LLVMGetVectorSize(T);
                    retvalue = LLVMGetUndef(T);
                    for (unsigned i = 0; i < count; ++i) {
                        LLVMValueRef idx = LLVMConstInt(i32T, i, false);
                        LLVMValueRef values[] = {
                            LLVMBuildExtractElement(builder, a, idx, ""),
                            LLVMBuildExtractElement(builder, b, idx, "")
                        };
                        LLVMValueRef eltval = LLVMBuildCall(builder, func, values, 2, "");
                        retvalue = LLVMBuildInsertElement(builder, retvalue, eltval, idx, "");
                    }
                } else {
                    LLVMValueRef values[] = { a, b };
                    retvalue = LLVMBuildCall(builder, func, values, 2, "");
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
            case OP_Floor: { READ_VALUE(x);
                auto T = LLVMTypeOf(x);
                auto ET = T;
                if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                    ET = LLVMGetElementType(T);
                }
                LLVMValueRef func = nullptr;
                Intrinsic op = NumIntrinsics;
                switch(enter.builtin.value()) {
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
                default: break;
                }
                func = get_intrinsic(op);
                assert(func);
                if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                    auto count = LLVMGetVectorSize(T);
                    retvalue = LLVMGetUndef(T);
                    for (unsigned i = 0; i < count; ++i) {
                        LLVMValueRef idx = LLVMConstInt(i32T, i, false);
                        LLVMValueRef values[] = { LLVMBuildExtractElement(builder, x, idx, "") };
                        LLVMValueRef eltval = LLVMBuildCall(builder, func, values, 1, "");
                        retvalue = LLVMBuildInsertElement(builder, retvalue, eltval, idx, "");
                    }
                } else {
                    LLVMValueRef values[] = { x };
                    retvalue = LLVMBuildCall(builder, func, values, 1, "");
                }
            } break;
            case SFXFN_Unreachable:
                retvalue = LLVMBuildUnreachable(builder);
                rtraits.terminated = true;
                break;
            default: {
                StyledString ss;
                ss.out << "IL->IR: unsupported builtin " << enter.builtin << " encountered";
                SCOPES_LOCATION_ERROR(ss.str());
            } break;
            }
        } else if (enter.type == TYPE_Label) {
            LLVMValueRef value = SCOPES_GET_RESULT(label_to_value(enter));
            if (!value) {
                // no basic block was generated - just generate assignments
                auto &&params = enter.label->params;
                for (size_t i = 1; i <= argcount; ++i) {
                    if (i < params.size()) {
                        bind_parameter(params[i], SCOPES_GET_RESULT(argument_to_value(args[i].value)));
                    }
                }
                label = enter.label;
                goto repeat;
            } else if (LLVMValueIsBasicBlock(value)) {
                LLVMValueRef values[argcount];
                for (size_t i = 0; i < argcount; ++i) {
                    values[i] = SCOPES_GET_RESULT(argument_to_value(args[i + 1].value));
                }
                auto bbfrom = LLVMGetInsertBlock(builder);
                // assign phi nodes
                auto &&params = enter.label->params;
                LLVMBasicBlockRef incobbs[] = { bbfrom };
                for (size_t i = 1; i < params.size(); ++i) {
                    Parameter *param = params[i];
                    LLVMValueRef phinode = SCOPES_GET_RESULT(argument_to_value(param));
                    LLVMValueRef incovals[] = { values[i - 1] };
                    LLVMAddIncoming(phinode, incovals, incobbs, 1);
                }
                LLVMBuildBr(builder, LLVMValueAsBasicBlock(value));
                rtraits.terminated = true;
            } else {
                if (use_debug_info) {
                    LLVMSetCurrentDebugLocation(builder, diloc);
                }
                retvalue = SCOPES_GET_RESULT(build_call(
                    enter.label->get_function_type(),
                    value, args, rtraits));
            }
        } else if (enter.type == TYPE_Closure) {
            StyledString ss;
            ss.out << "IL->IR: invalid call of compile time closure at runtime";
            SCOPES_LOCATION_ERROR(ss.str());
        } else if (is_function_pointer(enter.indirect_type())) {
            retvalue = SCOPES_GET_RESULT(build_call(extract_function_type(enter.indirect_type()),
                SCOPES_GET_RESULT(argument_to_value(enter)), args, rtraits));
        } else if (enter.type == TYPE_Parameter) {
            assert (enter.parameter->type != TYPE_Nothing);
            assert(enter.parameter->type != TYPE_Unknown);
            LLVMValueRef values[argcount];
            for (size_t i = 0; i < argcount; ++i) {
                values[i] = SCOPES_GET_RESULT(argument_to_value(args[i + 1].value));
            }
            // must be a return
            assert(enter.parameter->index == 0);
            // must be returning from this function
            assert(enter.parameter->label == active_function);

            Label *label = enter.parameter->label;
            bool use_sret = is_memory_class(label->get_return_type());
            if (use_sret) {
                auto pval = SCOPES_GET_RESULT(resolve_parameter(enter.parameter));
                if (argcount > 1) {
                    LLVMTypeRef types[argcount];
                    for (size_t i = 0; i < argcount; ++i) {
                        types[i] = LLVMTypeOf(values[i]);
                    }

                    LLVMValueRef val = LLVMGetUndef(LLVMStructType(types, argcount, false));
                    for (size_t i = 0; i < argcount; ++i) {
                        val = LLVMBuildInsertValue(builder, val, values[i], i, "");
                    }
                    LLVMBuildStore(builder, val, pval);
                } else if (argcount == 1) {
                    LLVMBuildStore(builder, values[0], pval);
                }
                LLVMBuildRetVoid(builder);
            } else {
                if (argcount > 1) {
                    LLVMBuildAggregateRet(builder, values, argcount);
                } else if (argcount == 1) {
                    LLVMBuildRet(builder, values[0]);
                } else {
                    LLVMBuildRetVoid(builder);
                }
            }
            rtraits.terminated = true;
        } else {
            StyledString ss;
            ss.out << "IL->IR: cannot translate call to " << enter;
            SCOPES_LOCATION_ERROR(ss.str());
        }

        Any contarg = args[0].value;
        if (rtraits.terminated) {
            // write nothing
        } else if ((contarg.type == TYPE_Parameter)
            && (contarg.parameter->type != TYPE_Nothing)) {
            assert(contarg.parameter->type != TYPE_Unknown);
            assert(contarg.parameter->index == 0);
            assert(contarg.parameter->label == active_function);
            Label *label = contarg.parameter->label;
            bool use_sret = is_memory_class(label->get_return_type());
            if (use_sret) {
                auto pval = SCOPES_GET_RESULT(resolve_parameter(contarg.parameter));
                if (retvalue) {
                    LLVMBuildStore(builder, retvalue, pval);
                }
                LLVMBuildRetVoid(builder);
            } else {
                if (retvalue) {
                    LLVMBuildRet(builder, retvalue);
                } else {
                    LLVMBuildRetVoid(builder);
                }
            }
        } else if (contarg.type == TYPE_Label) {
            auto bb = SCOPES_GET_RESULT(label_to_basic_block(contarg.label));
            if (bb) {
                if (retvalue) {
                    auto bbfrom = LLVMGetInsertBlock(builder);
                    LLVMBasicBlockRef incobbs[] = { bbfrom };

#define UNPACK_RET_ARGS() \
    if (rtraits.multiple_return_values) { \
        assert(rtraits.rtype); \
        auto &&values = rtraits.rtype->values; \
        auto &&params = contarg.label->params; \
        size_t pi = 1; \
        for (size_t i = 0; i < values.size(); ++i) { \
            if (pi >= params.size()) \
                break; \
            Parameter *param = params[pi]; \
            auto &&arg = values[i]; \
            if (is_unknown(arg.value)) { \
                LLVMValueRef incoval = LLVMBuildExtractValue(builder, retvalue, i, ""); \
                T(param, incoval); \
                pi++; \
            } \
        } \
    } else { \
        auto &&params = contarg.label->params; \
        if (params.size() > 1) { \
            assert(params.size() == 2); \
            Parameter *param = params[1]; \
            T(param, retvalue); \
        } \
    }
                    #define T(PARAM, VALUE) \
                        LLVMAddIncoming(SCOPES_GET_RESULT(argument_to_value(PARAM)), &VALUE, incobbs, 1);
                    UNPACK_RET_ARGS()
                    #undef T
                }

                LLVMBuildBr(builder, bb);
            } else {
                if (retvalue) {
                    #define T(PARAM, VALUE) \
                        bind_parameter(PARAM, VALUE);
                    UNPACK_RET_ARGS()
                    #undef T
                }
                label = contarg.label;
                goto repeat;
            }
#undef UNPACK_RET_ARGS
        } else if (contarg.type == TYPE_Nothing) {
            StyledStream ss(SCOPES_CERR);
            stream_label(ss, label, StreamLabelFormat::debug_single());
            SCOPES_LOCATION_ERROR(String::from("IL->IR: unexpected end of function"));
        } else {
            StyledStream ss(SCOPES_CERR);
            stream_label(ss, label, StreamLabelFormat::debug_single());
            SCOPES_LOCATION_ERROR(String::from("IL->IR: continuation is of invalid type"));
        }

        LLVMSetCurrentDebugLocation(builder, nullptr);
        return true;
    }
#undef READ_ANY
#undef READ_VALUE
#undef READ_TYPE
#undef READ_LABEL_VALUE

    void set_active_function(Label *l) {
        if (active_function == l) return;
        active_function = l;
        if (l) {
            auto it = label2func.find(l);
            assert(it != label2func.end());
            active_function_value = it->second;
        } else {
            active_function_value = nullptr;
        }
    }

    SCOPES_RESULT(void) process_labels() {
        SCOPES_RESULT_TYPE(void);
        while (!bb_label_todo.empty()) {
            auto it = bb_label_todo.back();
            set_active_function(it.first);
            Label *label = it.second;
            bb_label_todo.pop_back();

            auto it2 = label2bb.find({active_function_value, label});
            assert(it2 != label2bb.end());
            LLVMBasicBlockRef bb = it2->second;
            LLVMPositionBuilderAtEnd(builder, bb);

            SCOPES_CHECK_RESULT(write_label_body(label));
        }
        return true;
    }

    bool has_single_caller(Label *l) {
        auto it = user_map.label_map.find(l);
        assert(it != user_map.label_map.end());
        auto &&users = it->second;
        if (users.size() != 1)
            return false;
        Label *userl = *users.begin();
        if (userl->body.enter == Any(l))
            return true;
        if (userl->body.args[0] == Any(l))
            return true;
        return false;
    }

    SCOPES_RESULT(LLVMBasicBlockRef) label_to_basic_block(Label *label) {
        SCOPES_RESULT_TYPE(LLVMBasicBlockRef);
        auto old_bb = LLVMGetInsertBlock(builder);
        LLVMValueRef func = LLVMGetBasicBlockParent(old_bb);
        auto it = label2bb.find({func, label});
        if (it == label2bb.end()) {
            if (has_single_caller(label)) {
                // not generating basic blocks for single user labels
                label2bb.insert({{func, label}, nullptr});
                return nullptr;
            }
            const char *name = label->name.name()->data;
            auto bb = LLVMAppendBasicBlock(func, name);
            label2bb.insert({{func, label}, bb});
            bb_label_todo.push_back({active_function, label});
            LLVMPositionBuilderAtEnd(builder, bb);

            auto &&params = label->params;
            if (!params.empty()) {
                size_t paramcount = label->params.size() - 1;
                for (size_t i = 0; i < paramcount; ++i) {
                    Parameter *param = params[i + 1];
                    auto pvalue = LLVMBuildPhi(builder,
                        SCOPES_GET_RESULT(type_to_llvm_type(param->type)),
                        param->name.name()->data);
                    bind_parameter(param, pvalue);
                }
            }

            LLVMPositionBuilderAtEnd(builder, old_bb);
            return bb;
        } else {
            return it->second;
        }
    }

    SCOPES_RESULT(LLVMValueRef) label_to_function(Label *label,
        bool root_function = false,
        Symbol funcname = SYM_Unnamed) {
        SCOPES_RESULT_TYPE(LLVMValueRef);
        auto it = label2func.find(label);
        if (it == label2func.end()) {

            const Anchor *old_anchor = get_active_anchor();
            set_active_anchor(label->anchor);
            Label *last_function = active_function;

            auto old_bb = LLVMGetInsertBlock(builder);

            if (funcname == SYM_Unnamed) {
                funcname = label->name;
            }

            const char *name;
            if (root_function) {
                if (funcname == SYM_Unnamed) {
                    name = "unnamed";
                } else {
                    name = funcname.name()->data;
                }
                size_t mangled_name_size = strlen(name) + 32;
                char *mangled_name = new char[mangled_name_size];
                snprintf(mangled_name, mangled_name_size, "_scopes_jit_%s_%p", name, (void *)label);
                name = mangled_name;
            } else {
                name = funcname.name()->data;
            }

            SCOPES_CHECK_RESULT(label->verify_compilable());
            auto ilfunctype = label->get_function_type();
            auto fi = cast<FunctionType>(ilfunctype);
            bool use_sret = is_memory_class(fi->return_type);

            auto functype = SCOPES_GET_RESULT(type_to_llvm_type(ilfunctype));

            auto func = LLVMAddFunction(module, name, functype);
            if (use_debug_info) {
                LLVMSetFunctionSubprogram(func, label_to_subprogram(label));
            }
            LLVMSetLinkage(func, LLVMPrivateLinkage);
            label2func[label] = func;
            set_active_function(label);

            auto bb = LLVMAppendBasicBlock(func, "");
            LLVMPositionBuilderAtEnd(builder, bb);

            auto &&params = label->params;
            size_t offset = 0;
            if (use_sret) {
                offset++;
                Parameter *param = params[0];
                bind_parameter(param, LLVMGetParam(func, 0));
            }

            size_t paramcount = params.size() - 1;

            if (use_debug_info)
                set_debug_location(label);
            size_t k = offset;
            for (size_t i = 0; i < paramcount; ++i) {
                Parameter *param = params[i + 1];
                LLVMValueRef val = SCOPES_GET_RESULT(abi_import_argument(param, func, k));
                bind_parameter(param, val);
            }

            SCOPES_CHECK_RESULT(write_label_body(label));

            LLVMPositionBuilderAtEnd(builder, old_bb);

            set_active_function(last_function);
            set_active_anchor(old_anchor);
            return func;
        } else {
            return it->second;
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

            LLVMDIBuilderCreateCompileUnit(di_builder,
                llvm::dwarf::DW_LANG_C99, "file", "directory", "scopes",
                false, "", 0, "", 0);
            //LLVMAddNamedMetadataOperand(module, "llvm.dbg.cu", dicu);
        }
    }

    SCOPES_RESULT(void) teardown_generate(Label *entry = nullptr) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_CHECK_RESULT(process_labels());

        size_t k = SCOPES_GET_RESULT(finalize_types());
        assert(!k);

        LLVMDisposeBuilder(builder);
        LLVMDisposeDIBuilder(di_builder);

#if SCOPES_DEBUG_CODEGEN
        LLVMDumpModule(module);
#endif
        char *errmsg = NULL;
        if (LLVMVerifyModule(module, LLVMReturnStatusAction, &errmsg)) {
            StyledStream ss(SCOPES_CERR);
            if (entry) {
                stream_label(ss, entry, StreamLabelFormat());
            }
            LLVMDumpModule(module);
            SCOPES_LOCATION_ERROR(
                String::join(
                    String::from("LLVM: "),
                    String::from_cstr(errmsg)));
        }
        LLVMDisposeMessage(errmsg);
        return true;
    }

    // for generating object files
    SCOPES_RESULT(LLVMModuleRef) generate(const String *name, Scope *table) {
        SCOPES_RESULT_TYPE(LLVMModuleRef);
        {
            std::unordered_set<Label *> visited;
            Labels labels;
            Scope *t = table;
            while (t) {
                for (auto it = t->map->begin(); it != t->map->end(); ++it) {
                    Label *fn = it->second.value;

                    SCOPES_CHECK_RESULT(fn->verify_compilable());
                    fn->build_reachable(visited, &labels);
                }
                t = t->parent;
            }
            for (auto it = labels.begin(); it != labels.end(); ++it) {
                (*it)->insert_into_usermap(user_map);
            }
        }

        setup_generate(name->data);

        Scope *t = table;
        while (t) {
            for (auto it = t->map->begin(); it != t->map->end(); ++it) {

                Symbol name = it->first;
                Label *fn = it->second.value;

                auto func = SCOPES_GET_RESULT(label_to_function(fn, true, name));
                LLVMSetLinkage(func, LLVMExternalLinkage);

            }
            t = t->parent;
        }

        SCOPES_CHECK_RESULT(teardown_generate());
        return module;
    }

    typedef std::pair<LLVMModuleRef, LLVMValueRef> ModuleValuePair;

    SCOPES_RESULT(ModuleValuePair) generate(Label *entry) {
        SCOPES_RESULT_TYPE(ModuleValuePair);
        assert(all_parameters_lowered(entry));
        assert(!entry->is_basic_block_like());

        {
            std::unordered_set<Label *> visited;
            entry->build_reachable(visited, nullptr);
            for (auto it = visited.begin(); it != visited.end(); ++it) {
                (*it)->insert_into_usermap(user_map);
            }
        }

        const char *name = entry->name.name()->data;
        setup_generate(name);

        auto func = SCOPES_GET_RESULT(label_to_function(entry, true));
        LLVMSetLinkage(func, LLVMExternalLinkage);

        SCOPES_CHECK_RESULT(teardown_generate(entry));

        return ModuleValuePair(module, func);
    }

};

bool LLVMIRGenerator::ok = true;
std::unordered_map<const Type *, LLVMTypeRef> LLVMIRGenerator::type_cache;
ArgTypes LLVMIRGenerator::type_todo;
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
LLVMAttributeRef LLVMIRGenerator::attr_byval = nullptr;
LLVMAttributeRef LLVMIRGenerator::attr_sret = nullptr;
LLVMAttributeRef LLVMIRGenerator::attr_nonnull = nullptr;

//------------------------------------------------------------------------------
// IL COMPILER
//------------------------------------------------------------------------------

#if 0
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
    ctx.inline_pointers = false;
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
        SCOPES_LOCATION_ERROR(String::from_cstr(errormsg));
    }
    free(path_cstr);
    return true;
}

#if 0
static DisassemblyListener *disassembly_listener = nullptr;
#endif

SCOPES_RESULT(Any) compile(Label *fn, uint64_t flags) {
    SCOPES_RESULT_TYPE(Any);
    Timer sum_compile_time(TIMER_Compile);
#if SCOPES_COMPILE_WITH_DEBUG_INFO
#else
    flags |= CF_NoDebugInfo;
#endif
#if SCOPES_OPTIMIZE_ASSEMBLY
    flags |= CF_O3;
#endif

    SCOPES_CHECK_RESULT(fn->verify_compilable());
    const Type *functype = Pointer(
        fn->get_function_type(), PTF_NonWritable, SYM_Unnamed);

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

    auto module = result.first;
    auto func = result.second;
    assert(func);

    init_execution();
    SCOPES_CHECK_RESULT(add_module(module));

#if 0
    if (!disassembly_listener && (flags & CF_DumpDisassembly)) {
        llvm::ExecutionEngine *pEE = reinterpret_cast<llvm::ExecutionEngine*>(ee);
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

    void *pfunc = get_pointer_to_global(func);
#if 0
    if (flags & CF_DumpDisassembly) {
        assert(disassembly_listener);
        //auto td = LLVMGetExecutionEngineTargetData(ee);
        auto it = disassembly_listener->sizes.find(pfunc);
        if (it != disassembly_listener->sizes.end()) {
            std::cout << "disassembly:\n";
            do_disassemble(target_machine, pfunc, it->second);
        } else {
            std::cout << "no disassembly available\n";
        }
    }
#endif

    return Any::from_pointer(functype, pfunc);
}

} // namespace scopes
