/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "gen_spirv.hpp"
#include "source_file.hpp"
#include "types.hpp"
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
#include "symbol_enum.inc"

#include "glslang/SpvBuilder.h"
#include "glslang/disassemble.h"
#include "glslang/GLSL.std.450.h"
#include "SPIRV-Cross/spirv_glsl.hpp"
#include "spirv-tools/libspirv.hpp"
#include "spirv-tools/optimizer.hpp"

#include "dyn_cast.inc"

#pragma GCC diagnostic ignored "-Wvla-extension"

namespace scopes {

#define SCOPES_GEN_TARGET "SPIR-V"

// prefix: GLSLstd450
#define SCOPES_GLSL_STD_450_FUNCS() \
    T(Round) \
    T(RoundEven) \
    T(Trunc) \
    T(FAbs) \
    T(SAbs) \
    T(FSign) \
    T(SSign) \
    T(Floor) \
    T(Ceil) \
    T(Fract) \
    T(Radians) \
    T(Degrees) \
    T(Sin) \
    T(Cos) \
    T(Tan) \
    T(Asin) \
    T(Acos) \
    T(Atan) \
    T(Sinh) \
    T(Cosh) \
    T(Tanh) \
    T(Asinh) \
    T(Acosh) \
    T(Atanh) \
    T(Atan2) \
    T(Pow) \
    T(Exp) \
    T(Log) \
    T(Exp2) \
    T(Log2) \
    T(Sqrt) \
    T(InverseSqrt) \
    T(Determinant) \
    T(MatrixInverse) \
    T(Modf) \
    T(ModfStruct) \
    T(FMin) \
    T(UMin) \
    T(SMin) \
    T(FMax) \
    T(UMax) \
    T(SMax) \
    T(FClamp) \
    T(UClamp) \
    T(SClamp) \
    T(FMix) \
    T(IMix) \
    T(Step) \
    T(SmoothStep) \
    T(Fma) \
    T(Frexp) \
    T(FrexpStruct) \
    T(Ldexp) \
    T(PackSnorm4x8) \
    T(PackUnorm4x8) \
    T(PackSnorm2x16) \
    T(PackUnorm2x16) \
    T(PackHalf2x16) \
    T(PackDouble2x32) \
    T(UnpackSnorm2x16) \
    T(UnpackUnorm2x16) \
    T(UnpackHalf2x16) \
    T(UnpackSnorm4x8) \
    T(UnpackUnorm4x8) \
    T(UnpackDouble2x32) \
    T(Length) \
    T(Distance) \
    T(Cross) \
    T(Normalize) \
    T(FaceForward) \
    T(Reflect) \
    T(Refract) \
    T(FindILsb) \
    T(FindSMsb) \
    T(FindUMsb) \
    T(InterpolateAtCentroid) \
    T(InterpolateAtSample) \
    T(InterpolateAtOffset) \
    T(NMin) \
    T(NMax) \
    T(NClamp) \

//------------------------------------------------------------------------------
// IL->SPIR-V GENERATOR
//------------------------------------------------------------------------------

static void disassemble_spirv(std::vector<unsigned int> &contents, bool debug = false) {
    spv_context context = spvContextCreate(SPV_ENV_UNIVERSAL_1_2);
    uint32_t options = SPV_BINARY_TO_TEXT_OPTION_PRINT;
    options |= SPV_BINARY_TO_TEXT_OPTION_INDENT;
    if (!debug)
        options |= SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES;
    //options |= SPV_BINARY_TO_TEXT_OPTION_SHOW_BYTE_OFFSET;
    if (stream_default_style == stream_ansi_style) {
        options |= SPV_BINARY_TO_TEXT_OPTION_COLOR;
    }
    spv_diagnostic diagnostic = nullptr;
    spv_result_t error =
        spvBinaryToText(context, contents.data(), contents.size(), options,
                        nullptr, &diagnostic);
    spvContextDestroy(context);
    if (error) {
        spvDiagnosticPrint(diagnostic);
        spvDiagnosticDestroy(diagnostic);
        StyledStream ss(SCOPES_CERR);
        ss << "error while pretty-printing disassembly, falling back to"
            " failsafe disassembly" << std::endl;
        #if SCOPES_USE_WCHAR
        std::stringstream stdss;
        spv::Disassemble(stdss, contents);
        ss << String::from_stdstring(stdss.str());
        #else
        spv::Disassemble(std::cerr, contents);
        #endif
    }
}

static void format_spv_location(StyledStream &ss, const char* source,
    const spv_position_t& position) {
    ss << Style_Location;
    StyledStream ps = StyledStream::plain(ss);
    if (source) {
        ps << source << ":";
    }
    ps << position.line << ":" << position.column;
    ss << ":" << Style_None << " ";
}

static SCOPES_RESULT(void) verify_spirv(std::vector<unsigned int> &contents) {
    SCOPES_RESULT_TYPE(void);
    spv_target_env target_env = SPV_ENV_UNIVERSAL_1_2;
    //spvtools::ValidatorOptions options;

    StyledString ss;
    spvtools::SpirvTools tools(target_env);
    tools.SetMessageConsumer([&ss](spv_message_level_t level, const char* source,
                                const spv_position_t& position,
                                const char* message) {
        switch (level) {
        case SPV_MSG_FATAL:
        case SPV_MSG_INTERNAL_ERROR:
        case SPV_MSG_ERROR:
            format_spv_location(ss.out, source, position);
            ss.out << Style_Error << "error: " << Style_None
                << message << std::endl;
            break;
        case SPV_MSG_WARNING:
            format_spv_location(ss.out, source, position);
            ss.out << Style_Warning << "warning: " << Style_None
                << message << std::endl;
            break;
        case SPV_MSG_INFO:
            format_spv_location(ss.out, source, position);
            ss.out << Style_Comment << "info: " << Style_None
                << message << std::endl;
            break;
        default:
            break;
        }
    });

    bool succeed = tools.Validate(contents);
    if (!succeed) {
        disassemble_spirv(contents, true);
        SCOPES_CERR << ss._ss.str();
        SCOPES_ERROR(CGenBackendValidationFailed);
    }
    return {};
}

struct SPIRVGenerator {
    typedef std::pair<const Type *, uint64_t> TypeFlagPair;
    struct HashTypeFlagsPair {
        size_t operator ()(const TypeFlagPair &value) const {
            return
                hash2(std::hash<const Type *>{}(value.first),
                    std::hash<uint64_t>{}(value.second));
        }
    };

    typedef std::vector<spv::Id> Ids;

    std::unordered_map<ValueIndex, spv::Id, ValueIndex::Hash> ref2value;
    std::unordered_map<Function *, spv::Function *> func2func;
    std::deque<FunctionRef> function_todo;
    std::unordered_map<TypeFlagPair, spv::Id, HashTypeFlagsPair> type_cache;

    std::unordered_map<int, ExecutionMode *> execution_modes;

    std::unordered_map<Symbol, spv::Id, Symbol::Hash> intrinsics;

    spv::SpvBuildLogger logger;
    spv::Builder builder;

    spv::Instruction *entry_point;
    spv::Id glsl_ext_inst;
    bool use_debug_info;
    FunctionRef active_function;
    int functions_generated;

    spv::Id get_intrinsic(Symbol name) {
        auto it = intrinsics.find(name);
        if (it != intrinsics.end())
            return it->second;
        auto str = name.name();
        spv::Id result = 0;
        // prefix: GLSLstd450
        #define T(NAME) \
            if (!strncmp(str->data, "GLSL.std.450." #NAME, sizeof("GLSL.std.450." #NAME))) { \
                result = GLSLstd450 ## NAME; \
            } else
        SCOPES_GLSL_STD_450_FUNCS()
        #undef T
        /* else */ {}
        if (result) {
            intrinsics.insert({name, result});
        }
        return result;

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
        spv::Block *bb_except;
        Ids except_values;

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
        spv::Block *bb_loop;
        Ids repeat_values;

        LoopInfo() :
            bb_loop(nullptr)
        {}
    };

    LoopInfo loop_info;

    struct LabelInfo {
        LabelRef label;
        spv::Block *bb_merge;
        Ids merge_values;

        LabelInfo() :
            bb_merge(nullptr)
        {}
    };

    std::vector<LabelInfo> label_info_stack;

#if 0
    template<unsigned N>
    static LLVMAttributeRef get_attribute(const char (&s)[N]) {
        unsigned kind = LLVMGetEnumAttributeKindForName(s, N - 1);
        assert(kind);
        return LLVMCreateEnumAttribute(LLVMGetGlobalContext(), kind, 0);
    }
#endif

    SPIRVGenerator() :
        builder('S' << 24 | 'C' << 16 | 'O' << 8 | 'P', &logger),
        entry_point(nullptr),
        glsl_ext_inst(0),
        use_debug_info(true),
        functions_generated(0) {
        static_init();
        #if 0
        for (int i = 0; i < NumIntrinsics; ++i) {
            intrinsics[i] = nullptr;
        }
        #endif
    }

    static SCOPES_RESULT(spv::Dim) dim_from_symbol(Symbol sym) {
        SCOPES_RESULT_TYPE(spv::Dim);
        switch(sym.value()) {
        #define T(NAME) \
            case SYM_SPIRV_Dim ## NAME: return spv::Dim ## NAME;
            B_SPIRV_DIM()
        #undef T
            default:
                SCOPES_ERROR(CGenUnsupportedDimensionality, sym);
                break;
        }
        return spv::DimMax;
    }

    static SCOPES_RESULT(spv::ImageFormat) image_format_from_symbol(Symbol sym) {
        SCOPES_RESULT_TYPE(spv::ImageFormat);
        switch(sym.value()) {
        #define T(NAME) \
            case SYM_SPIRV_ImageFormat ## NAME: return spv::ImageFormat ## NAME;
            B_SPIRV_IMAGE_FORMAT()
        #undef T
            default:
                SCOPES_ERROR(CGenUnsupportedImageFormat, sym);
                break;
        }
        return spv::ImageFormatMax;
    }

    static SCOPES_RESULT(spv::ExecutionMode) execution_mode_from_symbol(Symbol sym) {
        SCOPES_RESULT_TYPE(spv::ExecutionMode);
        switch(sym.value()) {
        #define T(NAME) \
            case SYM_SPIRV_ExecutionMode ## NAME: return spv::ExecutionMode ## NAME;
            B_SPIRV_EXECUTION_MODE()
        #undef T
            default:
                SCOPES_ERROR(CGenUnsupportedExecutionMode, sym);
                break;
        }
        return spv::ExecutionModeMax;
    }

    static SCOPES_RESULT(spv::StorageClass) storage_class_from_extern_class(Symbol sym) {
        SCOPES_RESULT_TYPE(spv::StorageClass);
        switch(sym.value()) {
        #define T(NAME) \
            case SYM_SPIRV_StorageClass ## NAME: return spv::StorageClass ## NAME;
            B_SPIRV_STORAGE_CLASS()
        #undef T
            default:
                SCOPES_ERROR(CGenUnsupportedPointerStorageClass, sym);
                break;
        }
        return spv::StorageClassMax;
    }

    static void static_init() {
    }

    SCOPES_RESULT(spv::Id) create_struct_type(const Type *type, uint64_t flags,
        const TypenameType *tname = nullptr) {
        SCOPES_RESULT_TYPE(spv::Id);
        // todo: packed tuples
        auto ti = cast<TupleType>(type);
        size_t count = ti->values.size();
        std::vector<spv::Id> members;
        for (size_t i = 0; i < count; ++i) {
            members.push_back(SCOPES_GET_RESULT(
                type_to_spirv_type(ti->values[i])));
        }
        const char *name = "tuple";
        if (tname) {
            name = tname->name()->data;
        }
        auto id = builder.makeStructType(members, name);
        if (flags & GF_BufferBlock) {
            builder.addDecoration(id, spv::DecorationBufferBlock);
        } else if (flags & GF_Block) {
            builder.addDecoration(id, spv::DecorationBlock);
        }
        for (size_t i = 0; i < count; ++i) {
            Symbol key = SYM_Unnamed;
            auto kq = try_qualifier<KeyQualifier>(ti->values[i]);
            if (kq) {
                key = kq->key;
            }
            builder.addMemberName(id, i, key.name()->data);
            if (flags & GF_Volatile) {
                builder.addMemberDecoration(id, i, spv::DecorationVolatile);
            }
            if (flags & GF_Coherent) {
                builder.addMemberDecoration(id, i, spv::DecorationCoherent);
            }
            if (flags & GF_Restrict) {
                builder.addMemberDecoration(id, i, spv::DecorationRestrict);
            }
            if (flags & GF_NonWritable) {
                builder.addMemberDecoration(id, i, spv::DecorationNonWritable);
            }
            if (flags & GF_NonReadable) {
                builder.addMemberDecoration(id, i, spv::DecorationNonReadable);
            }
            builder.addMemberDecoration(id, i, spv::DecorationOffset, ti->offsets[i]);
        }
        return id;
    }

    SCOPES_RESULT(spv::Id) create_spirv_type(const Type *type, uint64_t flags) {
        SCOPES_RESULT_TYPE(spv::Id);
        switch(type->kind()) {
        case TK_Qualify: {
            auto qt = cast<QualifyType>(type);
            auto lltype = SCOPES_GET_RESULT(type_to_spirv_type(qt->type, flags));
            auto rq = try_qualifier<ReferQualifier>(type);
            if (rq) {
                lltype =
                    builder.makePointer(
                        SCOPES_GET_RESULT(storage_class_from_extern_class(
                            rq->storage_class)),
                        lltype);
            }
            return lltype;
        } break;
        case TK_Integer: {
            if (type == TYPE_Bool)
                return builder.makeBoolType();
            auto it = cast<IntegerType>(type);
            return builder.makeIntegerType(it->width, it->issigned);
        } break;
        case TK_Real: {
            auto rt = cast<RealType>(type);
            return builder.makeFloatType(rt->width);
        } break;
        case TK_Pointer: {
            auto pt = cast<PointerType>(type);
            return builder.makePointer(
                SCOPES_GET_RESULT(storage_class_from_extern_class(pt->storage_class)),
                SCOPES_GET_RESULT(type_to_spirv_type(pt->element_type)));
        } break;
        case TK_Array: {
            auto ai = cast<ArrayType>(type);
            auto etype = SCOPES_GET_RESULT(type_to_spirv_type(ai->element_type));
            spv::Id ty;
            if (ai->is_unsized()) {
                ty = builder.makeRuntimeArray(etype);
            } else {
                ty = builder.makeArrayType(etype,
                    builder.makeUintConstant(ai->count), 0);
            }
            builder.addDecoration(ty,
                spv::DecorationArrayStride,
                SCOPES_GET_RESULT(size_of(ai->element_type)));
            return ty;
        } break;
        case TK_Vector: {
            auto vi = cast<VectorType>(type);

            if ((vi->count <= 1) || (vi->is_unsized())) {
                SCOPES_ERROR(CGenUnsupportedVectorSize, vi->element_type, vi->count);
            }
            return builder.makeVectorType(
                SCOPES_GET_RESULT(type_to_spirv_type(vi->element_type)),
                vi->count);
        } break;
        case TK_Arguments: {
            if (type == empty_arguments_type())
                return builder.makeVoidType();
            return type_to_spirv_type(cast<ArgumentsType>(type)->to_tuple_type(), flags);
        } break;
        case TK_Tuple: {
            return create_struct_type(type, flags);
        } break;
        case TK_Typename: {
            if (type == TYPE_Sampler)
                return builder.makeSamplerType();
            auto tn = cast<TypenameType>(type);
            if (!tn->is_opaque()) {
                if (tn->storage()->kind() == TK_Tuple) {
                    return create_struct_type(tn->storage(), flags, tn);
                } else {
                    return SCOPES_GET_RESULT(type_to_spirv_type(tn->storage(), flags));
                }
            } else {
                SCOPES_ERROR(CGenTypeUnsupportedInTarget, type);
            }
        } break;
        case TK_Function: {
            auto fi = cast<FunctionType>(type);
            if (fi->vararg()) {
                SCOPES_ERROR(CGenTypeUnsupportedInTarget, type);
            }
            size_t count = fi->argument_types.size();
            auto rtype = abi_return_type(fi);
            spv::Id rettype = SCOPES_GET_RESULT(type_to_spirv_type(rtype));
            std::vector<spv::Id> elements;
            for (size_t i = 0; i < count; ++i) {
                auto AT = fi->argument_types[i];
                elements.push_back(SCOPES_GET_RESULT(type_to_spirv_type(AT)));
            }
            return builder.makeFunctionType(rettype, elements);
        } break;
        case TK_SampledImage: {
            auto sit = cast<SampledImageType>(type);
            return builder.makeSampledImageType(SCOPES_GET_RESULT(type_to_spirv_type(sit->type)));
        } break;
        case TK_Image: {
            auto it = cast<ImageType>(type);
            auto ty = SCOPES_GET_RESULT(type_to_spirv_type(it->type));
            if (builder.isVectorType(ty)) {
                ty = builder.getContainedTypeId(ty);
            }
            return builder.makeImageType(ty,
                SCOPES_GET_RESULT(dim_from_symbol(it->dim)),
                (it->depth == 1),
                (it->arrayed == 1),
                (it->multisampled == 1),
                it->sampled,
                SCOPES_GET_RESULT(image_format_from_symbol(it->format)));
        } break;
        default: break;
        };
        SCOPES_ERROR(CGenFailedToTranslateType, type);
    }

    SCOPES_RESULT(spv::Id) type_to_spirv_type(const Type *type, uint64_t flags = 0) {
        SCOPES_RESULT_TYPE(spv::Id);
        auto it = type_cache.find({type, flags});
        if (it == type_cache.end()) {
            spv::Id result = SCOPES_GET_RESULT(create_spirv_type(type, flags));
            type_cache.insert({ { type, flags }, result});
            return result;
        } else {
            return it->second;
        }
    }

    void bind(const ValueIndex &node, spv::Id value) {
        assert(value);
        ref2value.insert({node, value});
    }

    SCOPES_RESULT(void) write_return(const Ids &values, bool is_except = false) {
        SCOPES_RESULT_TYPE(void);
        assert(active_function);
        auto fi = extract_function_type(active_function->get_type());
        auto rtype = abi_return_type(fi);
        auto abiretT = SCOPES_GET_RESULT(type_to_spirv_type(rtype));
        spv::Id value = 0;
        if (fi->has_exception()) {
            bool has_except_value = is_returning_value(fi->except_type);
            bool has_return_value = is_returning_value(fi->return_type);

            if (has_except_value || has_return_value) {
                auto abivalue = builder.createUndefined(abiretT);
                if (is_except) {
                    abivalue = builder.createCompositeInsert(
                        builder.makeBoolConstant(false), abivalue,
                        builder.getTypeId(abivalue), 0);
                    if (is_returning_value(fi->except_type)) {
                        auto exceptT = SCOPES_GET_RESULT(type_to_spirv_type(fi->except_type));
                        abivalue = builder.createCompositeInsert(
                            values_to_struct(exceptT, values), abivalue,
                            builder.getTypeId(abivalue), 1);
                    }
                } else {
                    abivalue = builder.createCompositeInsert(
                        builder.makeBoolConstant(true), abivalue,
                        builder.getTypeId(abivalue), 0);
                    if (is_returning_value(fi->return_type)) {
                        int retvalueindex = (has_except_value?2:1);
                        auto returnT = SCOPES_GET_RESULT(type_to_spirv_type(fi->return_type));
                        abivalue = builder.createCompositeInsert(
                            values_to_struct(returnT, values), abivalue,
                            builder.getTypeId(abivalue), retvalueindex);
                    }
                }
                value = abivalue;
            } else {
                if (is_except) {
                    value = builder.makeBoolConstant(false);
                } else {
                    value = builder.makeBoolConstant(true);
                }
            }
        } else {
            if (is_returning_value(fi->return_type)) {
                auto returnT = SCOPES_GET_RESULT(type_to_spirv_type(fi->return_type));
                value = values_to_struct(returnT, values);
            }
        }
        if (rtype == empty_arguments_type()) {
            builder.makeReturn(true, 0);
        } else {
            assert(value);
            builder.makeReturn(true, value);
        }
        return {};
    }

    SCOPES_RESULT(void) write_return(const TypedValues &values, bool is_except = false) {
        SCOPES_RESULT_TYPE(void);
        Ids refs;
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
        builder.createBranch(try_info.bb_except);
        return {};
    }

    SCOPES_RESULT(void) Function_finalize(const FunctionRef &node) {
        SCOPES_RESULT_TYPE(void);

        functions_generated++;

        active_function = node;
        auto it = func2func.find(node.unref());
        assert(it != func2func.end());
        spv::Function *func = it->second;
        assert(func);
        auto ilfunctype = node->get_type();
        auto fi = extract_function_type(ilfunctype);
        //auto rtype = abi_return_type(fi);

        auto bb = func->getEntryBlock();

        try_info.clear();

        if (!node->raises.empty()) {
            try_info.bb_except = &builder.makeNewBlock();
            position_builder_at_end(try_info.bb_except);
            //if (use_debug_info)
            //    set_debug_location(node->anchor());
            SCOPES_CHECK_RESULT(build_phi(try_info.except_values, fi->except_type));
            SCOPES_CHECK_RESULT(write_return(try_info.except_values, true));
        }

        position_builder_at_end(bb);

        auto &&params = node->params;
        size_t paramcount = params.size();

        //if (use_debug_info)
        //    set_debug_location(node->anchor());
        //size_t k = 0;
        for (size_t i = 0; i < paramcount; ++i) {
            ParameterRef param = params[i];
            auto val = func->getParamId(i);
            bind(ValueIndex(param), val);
        }
        SCOPES_CHECK_RESULT(translate_block(node->body));
        return {};
    }

    bool is_bool(spv::Id value) {
        auto T = builder.getTypeId(value);
        return
            (builder.isVectorType(T)
             && builder.isBoolType(builder.getContainedTypeId(T)))
            || builder.isBoolType(T);
    }

    SCOPES_RESULT(void) write_anchor(const Anchor *anchor) {
        //SCOPES_RESULT_TYPE(void);
        #if 0
        assert(anchor);
        assert(anchor->file);
        if (use_debug_info) {
            builder.addLine(
                SCOPES_GET_RESULT(
                    argument_to_value(anchor->path().name())),
                anchor->lineno, anchor->column);
        }
        #endif
        return {};
    }

    SCOPES_RESULT(spv::Function *) Function_to_function(const FunctionRef &node,
        bool root_function = false,
        Symbol funcname = SYM_Unnamed) {
        SCOPES_RESULT_TYPE(spv::Function *);
        {
            auto it = func2func.find(node.unref());
            if (it != func2func.end()) {
                return it->second;
            }
        }

        if (funcname == SYM_Unnamed) {
            funcname = node->name;
        }

        const char *name = nullptr;
        if (root_function && (funcname == SYM_Unnamed)) {
            name = "unnamed";
        } else {
            name = funcname.name()->data;
        }

        auto ilfunctype = extract_function_type(node->get_type());
        auto rtype = abi_return_type(ilfunctype);
        auto rettype = SCOPES_GET_RESULT(type_to_spirv_type(rtype));

        spv::Block* bb;
        std::vector<spv::Id> paramtypes;
        auto &&argtypes = ilfunctype->argument_types;
        for (auto it = argtypes.begin(); it != argtypes.end(); ++it) {
            paramtypes.push_back(SCOPES_GET_RESULT(type_to_spirv_type(*it)));
        }

        std::vector<std::vector<spv::Decoration>> decorations;

        auto old_bb = builder.getBuildPoint();
        auto func = builder.makeFunctionEntry(
            spv::NoPrecision, rettype, name,
            paramtypes, decorations, &bb);
        func2func[node.unref()] = func;
        builder.setBuildPoint(old_bb);

        //if (use_debug_info) {
        //    LLVMSetSubprogram(func, function_to_subprogram(node));
        //}

        function_todo.push_back(node);
        return func;
    }

    SCOPES_RESULT(spv::Id) Function_to_value(const FunctionRef &node) {
        SCOPES_RESULT_TYPE(spv::Id);
        SCOPES_ERROR(CGenFailedToTranslateValue, node->kind());
    }

    SCOPES_RESULT(spv::Id) Parameter_to_value(const ParameterRef &node) {
        SCOPES_RESULT_TYPE(spv::Id);
        SCOPES_ERROR(CGenFailedToTranslateValue, node->kind());
    }

    SCOPES_RESULT(spv::Id) LoopLabelArguments_to_value(const LoopLabelArgumentsRef &node) {
        SCOPES_RESULT_TYPE(spv::Id);
        SCOPES_ERROR(CGenFailedToTranslateValue, node->kind());
    }

    SCOPES_RESULT(spv::Id) Exception_to_value(const ExceptionRef &node) {
        SCOPES_RESULT_TYPE(spv::Id);
        SCOPES_ERROR(CGenFailedToTranslateValue, node->kind());
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

    LabelInfo *find_label_info_by_kind(LabelKind kind) {
        int i = label_info_stack.size();
        while (i-- > 0) {
            auto &&info = label_info_stack[i];
            assert(info.label);
            if (info.label->label_kind == kind)
                return &info;
        }
        return nullptr;
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

    void build_merge_phi(
        const Ids &phis, const Ids &values) {
        assert(phis.size() == values.size());

        auto bb = builder.getBuildPoint();
        assert(bb);
        int count = phis.size();
        for (int i = 0; i < count; ++i) {
            auto phi = phis[i];
            auto op = builder.getInstruction(phi);
            assert(op);
            op->addIdOperand(values[i]);
            op->addIdOperand(bb->getId());
        }
    }

    SCOPES_RESULT(void) build_merge_phi(
        const Ids &phis, const TypedValues &values) {
        SCOPES_RESULT_TYPE(void);
        assert(phis.size() == values.size());

        auto bb = builder.getBuildPoint();
        assert(bb);
        int count = phis.size();
        for (int i = 0; i < count; ++i) {
            auto phi = phis[i];
            auto spirval = SCOPES_GET_RESULT(ref_to_value(values[i]));
            auto op = builder.getInstruction(phi);
            assert(op);
            op->addIdOperand(spirval);
            op->addIdOperand(bb->getId());
        }
        return {};
    }

    SCOPES_RESULT(void) build_merge(const LabelRef &label, const TypedValues &values) {
        SCOPES_RESULT_TYPE(void);
        auto &&label_info = find_label_info(label);
        SCOPES_CHECK_RESULT(build_merge_phi(label_info.merge_values, values));
        assert(label_info.bb_merge);
        builder.createBranch(label_info.bb_merge);
        return {};
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
        builder.createBranch(loop_info.bb_loop);
        return {};
    }

    SCOPES_RESULT(void) translate_Label(const LabelRef &node) {
        SCOPES_RESULT_TYPE(void);
        LabelInfo label_info;
        label_info.label = node;
        auto bb = builder.getBuildPoint();
        label_info.bb_merge = nullptr;
        auto rtype = node->get_type();
        if (is_returning(rtype)) {
            label_info.bb_merge = &builder.makeNewBlock();
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

    SCOPES_RESULT(void) build_phi(Ids &refs, const Type *T) {
        SCOPES_RESULT_TYPE(void);
        assert(refs.empty());
        int count = get_argument_count(T);
        for (int i = 0; i < count; ++i) {
            auto argT = SCOPES_GET_RESULT(type_to_spirv_type(get_argument(T, i)));
            auto op = new spv::Instruction(
                builder.getUniqueId(), argT, spv::OpPhi);
            //builder.addName(op->getResultId(), param->name.name()->data);
            builder.getBuildPoint()->addInstruction(
                std::unique_ptr<spv::Instruction>(op));
            refs.push_back(op->getResultId());
        }
        return {};
    }

    void map_phi(const Ids &refs, const TypedValueRef &node) {
        for (int i = 0; i < refs.size(); ++i) {
            bind(ValueIndex(node, i), refs[i]);
        }
    }

    SCOPES_RESULT(void) build_phi(Ids &refs, const TypedValueRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_CHECK_RESULT(build_phi(refs, node->get_type()));
        map_phi(refs, node);
        return {};
    }

    SCOPES_RESULT(void) translate_LoopLabel(const LoopLabelRef &node) {
        SCOPES_RESULT_TYPE(void);
        LabelInfo *li_break = find_label_info_by_kind(LK_Break);
        //auto rtype = node->get_type();
        auto old_loop_info = loop_info;
        loop_info.loop = node;
        loop_info.repeat_values.clear();
        // continue label
        loop_info.bb_loop = &builder.makeNewBlock();

        auto bb = builder.getBuildPoint();

        // loop header label
        auto bb_header = &builder.makeNewBlock();
        position_builder_at_end(bb_header);
        Ids headerphi;
        SCOPES_CHECK_RESULT(build_phi(headerphi, node->args));
        if (li_break) {
            builder.createLoopMerge(li_break->bb_merge,
                loop_info.bb_loop, spv::LoopControlMaskNone);
        }
        auto bb_subheader = &builder.makeNewBlock();
        builder.createBranch(bb_subheader);
        position_builder_at_end(bb_subheader);

        // continue label
        position_builder_at_end(loop_info.bb_loop);
        SCOPES_CHECK_RESULT(build_phi(loop_info.repeat_values, node->args));
        build_merge_phi(headerphi, loop_info.repeat_values);
        builder.createBranch(bb_header);

        // branch into header
        position_builder_at_end(bb);
        SCOPES_CHECK_RESULT(build_merge_phi(headerphi, node->init));
        builder.createBranch(bb_header);

        // loop body
        position_builder_at_end(bb_subheader);
        SCOPES_CHECK_RESULT(translate_block(node->body));
        loop_info = old_loop_info;
        return {};
    }

    spv::Id values_to_struct(spv::Id T, const Ids &values) {
        int count = (int)values.size();
        if (count == 1) {
            return values[0];
        } else {
            auto value = builder.createUndefined(T);
            for (int i = 0; i < count; ++i) {
                value = builder.createCompositeInsert(values[i], value, T, i);
            }
            return value;
        }
    }

    void struct_to_values(Ids &values, const Type *T, spv::Id value) {
        assert(value);
        int count = get_argument_count(T);
        if (count == 1) {
            values.push_back(value);
        } else {
            auto spirvtype = builder.getTypeId(value);
            for (int i = 0; i < count; ++i) {
                values.push_back(builder.createCompositeExtract(value,
                    builder.getContainedTypeId(spirvtype, i), i));
            }
        }
    }

    SCOPES_RESULT(spv::Id) node_to_spirv_type(const ValueRef &node) {
        SCOPES_RESULT_TYPE(spv::Id);
        return type_to_spirv_type(SCOPES_GET_RESULT(extract_type_constant(node)));
    }

    spv::Id build_extract_image(spv::Id value) {
        spv::Id typeId = builder.getTypeId(value);
        if (builder.isSampledImageType(typeId)) {
            spv::Id imgtypeId = builder.getImageType(value);
            return builder.createUnaryOp(spv::OpImage, imgtypeId, value);
        }
        return value;
    }

    SCOPES_RESULT(void) translate_Unreachable(const UnreachableRef &node) {
        builder.makeUnreachable();
        return {};
    }

    SCOPES_RESULT(void) translate_Discard(const DiscardRef &node) {
        builder.makeDiscard();
        return {};
    }

    SCOPES_RESULT(void) translate_Call(const CallRef &call) {
        SCOPES_RESULT_TYPE(void);
        auto callee = call->callee;
        auto &&args = call->args;

        SCOPES_CHECK_RESULT(write_anchor(call.anchor()));

        const Type *rtype = strip_lifetime(callee->get_type());

        if (!is_function_pointer(rtype)) {
            SCOPES_ERROR(CGenInvalidCallee, callee->get_type());
        }
        if (callee.isa<Global>()) {
            auto name = callee.cast<Global>()->name;
            int ep = get_intrinsic(name);
            if (!ep) {
                SCOPES_ERROR(CGenUnsupportedIntrinsic, name);
            }
            auto ty = SCOPES_GET_RESULT(type_to_spirv_type(call->get_type()));
            size_t argcount = args.size();
            Ids values;
            values.reserve(argcount);
            for (size_t i = 0; i < argcount; ++i) {
                values.push_back(SCOPES_GET_RESULT(ref_to_value(args[i])));
            }
            auto val = builder.createBuiltinCall(
                ty, glsl_ext_inst, ep, values);
            map_phi({ val }, call);
        } else {
            auto func =
                SCOPES_GET_RESULT(Function_to_function(
                    SCOPES_GET_RESULT(
                        extract_function_constant(callee))));
            SCOPES_CHECK_RESULT(build_call(call,
                extract_function_type(rtype),
                func, args));
        }
        return {};
    }

    SCOPES_RESULT(void) translate_Sample(const SampleRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto sampler = SCOPES_GET_RESULT(ref_to_value(node->sampler));
        auto coords = SCOPES_GET_RESULT(ref_to_value(node->coords));
        spv::Builder::TextureParameters params;
        memset(&params, 0, sizeof(params));
        params.sampler = sampler;
        params.coords = coords;
        auto ST = SCOPES_GET_RESULT(storage_type(node->sampler->get_type()));
        if (ST->kind() == TK_SampledImage) {
            ST = SCOPES_GET_RESULT(storage_type(cast<SampledImageType>(ST)->type));
        }
        auto resultType = SCOPES_GET_RESULT(type_to_spirv_type(cast<ImageType>(ST)->type));
        bool sparse = false;
        bool fetch = false;
        bool proj = false;
        bool gather = false;
        bool explicitLod = false;
        for (auto &&opt : node->options) {
            Symbol key = opt.first;
            spv::Id value = 0;
            if (opt.second) {
                value = SCOPES_GET_RESULT(ref_to_value(opt.second));
            }
            switch (key.value()) {
                case SYM_SPIRV_ImageOperandLod: params.lod = value; break;
                case SYM_SPIRV_ImageOperandBias: params.bias = value; break;
                case SYM_SPIRV_ImageOperandDref: params.Dref = value; break;
                case SYM_SPIRV_ImageOperandProj: proj = true; break;
                case SYM_SPIRV_ImageOperandFetch: fetch = true; break;
                case SYM_SPIRV_ImageOperandGradX: params.gradX = value; break;
                case SYM_SPIRV_ImageOperandGradY: params.gradY = value; break;
                case SYM_SPIRV_ImageOperandOffset: params.offset = value; break;
                case SYM_SPIRV_ImageOperandConstOffsets: params.offsets = value; break;
                case SYM_SPIRV_ImageOperandMinLod: params.lodClamp = value; break;
                case SYM_SPIRV_ImageOperandSample: params.sample = value; break;
                case SYM_SPIRV_ImageOperandGather: {
                    params.component = value;
                    gather = true;
                } break;
                case SYM_SPIRV_ImageOperandSparse: {
                    params.texelOut = value;
                    sparse = true;
                } break;
                default: break;
            }
        }
        if (fetch) {
            params.sampler = build_extract_image(sampler);
        }
        auto val = builder.createTextureCall(
            spv::NoPrecision, resultType, sparse, fetch, proj, gather,
            explicitLod, params);
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_ImageQuerySize(const ImageQuerySizeRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto sampler = SCOPES_GET_RESULT(ref_to_value(node->sampler));
        spv::Builder::TextureParameters params;
        memset(&params, 0, sizeof(params));
        params.sampler = build_extract_image(sampler);
        spv::Op op = spv::OpImageQuerySize;
        if (node->has_lod()) {
            op = spv::OpImageQuerySizeLod;
            params.lod = SCOPES_GET_RESULT(ref_to_value(node->lod));
        }
        auto val = builder.createTextureQueryCall(op, params, false);
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_ImageQueryLod(const ImageQueryLodRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto sampler = SCOPES_GET_RESULT(ref_to_value(node->sampler));
        auto coords = SCOPES_GET_RESULT(ref_to_value(node->coords));
        spv::Builder::TextureParameters params;
        memset(&params, 0, sizeof(params));
        params.sampler = sampler;
        params.coords = coords;
        auto val = builder.createTextureQueryCall(
            spv::OpImageQueryLod, params, false);
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_ImageQueryLevels(const ImageQueryLevelsRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto sampler = SCOPES_GET_RESULT(ref_to_value(node->sampler));
        spv::Builder::TextureParameters params;
        memset(&params, 0, sizeof(params));
        params.sampler = build_extract_image(sampler);
        auto val = builder.createTextureQueryCall(
            spv::OpImageQueryLevels, params, false);
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_ImageQuerySamples(const ImageQuerySamplesRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto sampler = SCOPES_GET_RESULT(ref_to_value(node->sampler));
        spv::Builder::TextureParameters params;
        memset(&params, 0, sizeof(params));
        params.sampler = build_extract_image(sampler);
        auto val = builder.createTextureQueryCall(
            spv::OpImageQuerySamples, params, false);
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_ImageRead(const ImageReadRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto image = SCOPES_GET_RESULT(ref_to_value(node->image));
        auto coords = SCOPES_GET_RESULT(ref_to_value(node->coords));
        auto ST = SCOPES_GET_RESULT(storage_type(node->image->get_type()));
        auto resultType = SCOPES_GET_RESULT(type_to_spirv_type(cast<ImageType>(ST)->type));
        auto val = builder.createBinOp(spv::OpImageRead, resultType, image, coords);
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_ImageWrite(const ImageWriteRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto image = SCOPES_GET_RESULT(ref_to_value(node->image));
        auto coords = SCOPES_GET_RESULT(ref_to_value(node->coords));
        auto texel = SCOPES_GET_RESULT(ref_to_value(node->texel));
        builder.createNoResultOp(spv::OpImageWrite, { image, coords, texel });
        return {};
    }

    SCOPES_RESULT(void) translate_ExecutionMode(const ExecutionModeRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto em = SCOPES_GET_RESULT(execution_mode_from_symbol(node->mode));
        auto it = execution_modes.insert({ em, node.unref() });
        if (!it.second) {
            it.first->second = node.unref();
        }
        return {};
    }

    SCOPES_RESULT(void) translate_GetElementPtr(const GetElementPtrRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto pointer = SCOPES_GET_RESULT(ref_to_value(node->value));
        auto &&src_indices = node->indices;
        size_t count = src_indices.size();
        assert(count);
        // skip first index as we can't offset pointers in SPIR-V
        std::vector<spv::Id> indices;
        for (size_t i = 1; i < count; ++i) {
            indices.push_back(SCOPES_GET_RESULT(ref_to_value(src_indices[i])));
        }
        auto val = builder.createAccessChain(
            builder.getTypeStorageClass(builder.getTypeId(pointer)),
            pointer, indices);
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_ExtractValue(const ExtractValueRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto value = SCOPES_GET_RESULT(ref_to_value(node->value));
        auto index = node->index;
        auto val = builder.createCompositeExtract(
                value,
                builder.getContainedTypeId(builder.getTypeId(value), index),
                index);
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_InsertValue(const InsertValueRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto value = SCOPES_GET_RESULT(ref_to_value(node->value));
        auto val = builder.createCompositeInsert(
            SCOPES_GET_RESULT(ref_to_value(node->element)),
            value,
            builder.getTypeId(value), node->index);
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_ExtractElement(const ExtractElementRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto value = SCOPES_GET_RESULT(ref_to_value(node->value));
        spv::Id val;
        if (node->index.isa<ConstInt>()) {
            int i = extract_integer_constant(node->index).assert_ok();
            val = builder.createCompositeExtract(value,
                builder.getContainedTypeId(builder.getTypeId(value), i), i);
        } else {
            val = builder.createVectorExtractDynamic(value,
                builder.getContainedTypeId(builder.getTypeId(value)),
                SCOPES_GET_RESULT(ref_to_value(node->index)));
        }
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_InsertElement(const InsertElementRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto value = SCOPES_GET_RESULT(ref_to_value(node->value));
        auto element = SCOPES_GET_RESULT(ref_to_value(node->element));
        spv::Id val;
        if (node->index.isa<ConstInt>()) {
            int i = extract_integer_constant(node->index).assert_ok();
            val = builder.createCompositeInsert(element, value,
                builder.getTypeId(value), i);
        } else {
            val = builder.createVectorInsertDynamic(value,
                builder.getTypeId(value), element,
                SCOPES_GET_RESULT(ref_to_value(node->index)));
        }
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_ShuffleVector(const ShuffleVectorRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto v1 = SCOPES_GET_RESULT(ref_to_value(node->v1));
        auto v2 = SCOPES_GET_RESULT(ref_to_value(node->v2));
        auto &&mask = node->mask;

        auto ET = builder.getContainedTypeId(builder.getTypeId(v1));
        auto sz = mask.size();
        auto op = new spv::Instruction(
            builder.getUniqueId(),
            builder.makeVectorType(ET, sz),
            spv::OpVectorShuffle);
        op->addIdOperand(v1);
        op->addIdOperand(v2);
        for (int i = 0; i < sz; ++i) {
            op->addImmediateOperand(mask[i]);
        }
        builder.getBuildPoint()->addInstruction(
            std::unique_ptr<spv::Instruction>(op));
        auto val = op->getResultId();
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_Alloca(const AllocaRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto ty = SCOPES_GET_RESULT(type_to_spirv_type(node->type));
        if (node->is_array()) {
            SCOPES_ERROR(CGenUnsupportedArrayAlloc);
        }
        auto val = builder.createVariable(spv::StorageClassFunction, ty);
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_Malloc(const MallocRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_ERROR(CGenUnsupportedMalloc);
        return {};
    }

    SCOPES_RESULT(void) translate_Free(const FreeRef &node) {
        SCOPES_RESULT_TYPE(void);
        SCOPES_ERROR(CGenUnsupportedMalloc);
        return {};
    }

    SCOPES_RESULT(void) translate_Load(const LoadRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto ptr = SCOPES_GET_RESULT(ref_to_value(node->value));
        auto val = builder.createLoad(ptr);
        if (node->is_volatile) {
            builder.getInstruction(val)->addImmediateOperand(
                1<<spv::MemoryAccessVolatileShift);
        }
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_Store(const StoreRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto value = SCOPES_GET_RESULT(ref_to_value(node->value));
        auto ptr = SCOPES_GET_RESULT(ref_to_value(node->target));
        builder.createStore(value, ptr);
        return {};
    }

    SCOPES_RESULT(void) translate_AtomicRMW(const AtomicRMWRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto ty = SCOPES_GET_RESULT(type_to_spirv_type(node->get_type()));
        auto ptr = SCOPES_GET_RESULT(ref_to_value(node->target));
        auto value = SCOPES_GET_RESULT(ref_to_value(node->value));
        spv::Op op = spv::OpMax;
        switch(node->op) {
        case AtomicRMWOpXchg: op = spv::OpAtomicExchange; break;
        case AtomicRMWOpAdd: op = spv::OpAtomicIAdd; break;
        case AtomicRMWOpSub: op = spv::OpAtomicISub; break;
        case AtomicRMWOpAnd: op = spv::OpAtomicAnd; break;
        case AtomicRMWOpOr: op = spv::OpAtomicOr; break;
        case AtomicRMWOpXor: op = spv::OpAtomicXor; break;
        case AtomicRMWOpSMin: op = spv::OpAtomicSMin; break;
        case AtomicRMWOpSMax: op = spv::OpAtomicSMax; break;
        case AtomicRMWOpUMin: op = spv::OpAtomicUMin; break;
        case AtomicRMWOpUMax: op = spv::OpAtomicUMax; break;
        default: {
            SCOPES_ERROR(CGenUnsupportedAtomicOp);
        } break;
        }
        auto instr = new spv::Instruction(builder.getUniqueId(), ty, op);
        instr->addIdOperand(ptr);
        auto scope = spv::ScopeCrossDevice;
        auto memory_access =
            (1 << spv::MemorySemanticsSequentiallyConsistentShift)
            | (1 << spv::MemorySemanticsSubgroupMemoryShift)
            | (1 << spv::MemorySemanticsWorkgroupMemoryShift)
            | (1 << spv::MemorySemanticsCrossWorkgroupMemoryShift);
        instr->addIdOperand(builder.makeUintConstant(scope));
        instr->addIdOperand(builder.makeUintConstant(memory_access));
        instr->addIdOperand(value);
        builder.getBuildPoint()->addInstruction(
            std::unique_ptr<spv::Instruction>(instr));
        auto val = instr->getResultId();
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_CmpXchg(const CmpXchgRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto T = node->get_type();
        auto ty = SCOPES_GET_RESULT(type_to_spirv_type(get_argument(T, 0)));
        auto ptr = SCOPES_GET_RESULT(ref_to_value(node->target));
        auto cmp = SCOPES_GET_RESULT(ref_to_value(node->cmp));
        auto value = SCOPES_GET_RESULT(ref_to_value(node->value));
        auto instr = new spv::Instruction(builder.getUniqueId(), ty,
            spv::OpAtomicCompareExchange);
        instr->addIdOperand(ptr);
        auto scope = spv::ScopeCrossDevice;
        auto memory_access =
            (1 << spv::MemorySemanticsSequentiallyConsistentShift)
            | (1 << spv::MemorySemanticsSubgroupMemoryShift)
            | (1 << spv::MemorySemanticsWorkgroupMemoryShift)
            | (1 << spv::MemorySemanticsCrossWorkgroupMemoryShift);
        instr->addIdOperand(builder.makeUintConstant(scope));
        instr->addIdOperand(builder.makeUintConstant(memory_access));
        instr->addIdOperand(builder.makeUintConstant(memory_access));
        instr->addIdOperand(value);
        instr->addIdOperand(cmp);
        builder.getBuildPoint()->addInstruction(
            std::unique_ptr<spv::Instruction>(instr));
        auto val = instr->getResultId();
        auto eq = builder.createBinOp(
            (is_bool(cmp)?spv::OpLogicalEqual:spv::OpIEqual),
            builder.makeBoolType(), val, cmp);
        map_phi({ val, eq }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_Annotate(const AnnotateRef &node) {
        return {};
    }

    SCOPES_RESULT(void) translate_Select(const SelectRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto cond = SCOPES_GET_RESULT(ref_to_value(node->cond));
        auto value1 = SCOPES_GET_RESULT(ref_to_value(node->value1));
        auto value2 = SCOPES_GET_RESULT(ref_to_value(node->value2));
        if (builder.isScalar(cond) && builder.isVector(value1)) {
            // emulate LLVM's behavior for selecting full vectors with single booleans
            spv::Id elementT = builder.getTypeId(cond);
            spv::Id vectorT = builder.getTypeId(value1);
            cond = builder.smearScalar(spv::NoPrecision, cond,
                builder.makeVectorType(elementT, builder.getNumTypeComponents(vectorT)));
        }
        auto val = builder.createTriOp(spv::OpSelect,
            builder.getTypeId(value1), cond, value1, value2);
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_ICmp(const ICmpRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto a = SCOPES_GET_RESULT(ref_to_value(node->value1));
        auto b = SCOPES_GET_RESULT(ref_to_value(node->value2));

        spv::Op op = spv::OpMax;
#define BOOL_OR_INT_OP(BOOL_OP, INT_OP) (is_bool(a)?(BOOL_OP):(INT_OP))
        switch(node->cmp_kind) {
        case ICmpEQ: op = BOOL_OR_INT_OP(spv::OpLogicalEqual, spv::OpIEqual); break;
        case ICmpNE: op = BOOL_OR_INT_OP(spv::OpLogicalNotEqual, spv::OpINotEqual); break;
        case ICmpUGT: op = spv::OpUGreaterThan; break;
        case ICmpUGE: op = spv::OpUGreaterThanEqual; break;
        case ICmpULT: op = spv::OpULessThan; break;
        case ICmpULE: op = spv::OpULessThanEqual; break;
        case ICmpSGT: op = spv::OpSGreaterThan; break;
        case ICmpSGE: op = spv::OpSGreaterThanEqual; break;
        case ICmpSLT: op = spv::OpSLessThan; break;
        case ICmpSLE: op = spv::OpSLessThanEqual; break;
        default: break;
        }
#undef BOOL_OR_INT_OP
        auto T = builder.getTypeId(a);
        if (builder.isVectorType(T)) {
            T = builder.makeVectorType(builder.makeBoolType(),
                builder.getNumTypeComponents(T));
        } else {
            T = builder.makeBoolType();
        }
        auto val = builder.createBinOp(op, T, a, b);
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_FCmp(const FCmpRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto a = SCOPES_GET_RESULT(ref_to_value(node->value1));
        auto b = SCOPES_GET_RESULT(ref_to_value(node->value2));

        spv::Op op = spv::OpMax;
        switch(node->cmp_kind) {
        case FCmpOEQ: op = spv::OpFOrdEqual; break;
        case FCmpONE: op = spv::OpFOrdNotEqual; break;
        case FCmpORD: op = spv::OpOrdered; break;
        case FCmpOGT: op = spv::OpFOrdGreaterThan; break;
        case FCmpOGE: op = spv::OpFOrdGreaterThanEqual; break;
        case FCmpOLT: op = spv::OpFOrdLessThan; break;
        case FCmpOLE: op = spv::OpFOrdLessThanEqual; break;
        case FCmpUEQ: op = spv::OpFUnordEqual; break;
        case FCmpUNE: op = spv::OpFUnordNotEqual; break;
        case FCmpUNO: op = spv::OpUnordered; break;
        case FCmpUGT: op = spv::OpFUnordGreaterThan; break;
        case FCmpUGE: op = spv::OpFUnordGreaterThanEqual; break;
        case FCmpULT: op = spv::OpFUnordLessThan; break;
        case FCmpULE: op = spv::OpFUnordLessThanEqual; break;
        default: break;
        }
        auto T = builder.getTypeId(a);
        if (builder.isVectorType(T)) {
            T = builder.makeVectorType(builder.makeBoolType(),
                builder.getNumTypeComponents(T));
        } else {
            T = builder.makeBoolType();
        }
        auto val = builder.createBinOp(op, T, a, b);
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_UnOp(const UnOpRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto x = SCOPES_GET_RESULT(ref_to_value(node->value));
        GLSLstd450 _builtin = GLSLstd450Bad;
        auto rtype = builder.getTypeId(x);
        switch (node->op) {
        case UnOpLength:
            rtype = builder.getContainedTypeId(rtype);
            _builtin = GLSLstd450Length; break;
        case UnOpNormalize: _builtin = GLSLstd450Normalize; break;
        case UnOpSin: _builtin = GLSLstd450Sin; break;
        case UnOpCos: _builtin = GLSLstd450Cos; break;
        case UnOpTan: _builtin = GLSLstd450Tan; break;
        case UnOpAsin: _builtin = GLSLstd450Asin; break;
        case UnOpAcos: _builtin = GLSLstd450Acos; break;
        case UnOpAtan: _builtin = GLSLstd450Atan; break;
        case UnOpSinh: _builtin = GLSLstd450Sinh; break;
        case UnOpCosh: _builtin = GLSLstd450Cosh; break;
        case UnOpTanh: _builtin = GLSLstd450Tanh; break;
        case UnOpASinh: _builtin = GLSLstd450Asinh; break;
        case UnOpACosh: _builtin = GLSLstd450Acosh; break;
        case UnOpATanh: _builtin = GLSLstd450Atanh; break;
        case UnOpTrunc: _builtin = GLSLstd450Trunc; break;
        case UnOpFloor: _builtin = GLSLstd450Floor; break;
        case UnOpFAbs: _builtin = GLSLstd450FAbs; break;
        case UnOpFSign: _builtin = GLSLstd450FSign; break;
        case UnOpLog: _builtin = GLSLstd450Log; break;
        case UnOpLog2: _builtin = GLSLstd450Log2; break;
        case UnOpExp: _builtin = GLSLstd450Exp; break;
        case UnOpExp2: _builtin = GLSLstd450Exp2; break;
        case UnOpSqrt: _builtin = GLSLstd450Sqrt; break;
        case UnOpRadians: _builtin = GLSLstd450Radians; break;
        case UnOpDegrees: _builtin = GLSLstd450Degrees; break;
        default: {
            SCOPES_ERROR(CGenUnsupportedUnOp);
        } break;
        }
        auto val = builder.createBuiltinCall(rtype, glsl_ext_inst, _builtin, { x });
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_BinOp(const BinOpRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto a = SCOPES_GET_RESULT(ref_to_value(node->value1));
        auto b = SCOPES_GET_RESULT(ref_to_value(node->value2));

        auto rtype = builder.getTypeId(a);
        spv::Id val = 0;
        GLSLstd450 _builtin = GLSLstd450Bad;
        spv::Op op = spv::OpMax;
        switch(node->op) {
#define BOOL_OR_INT_OP(BOOL_OP, INT_OP) \
    (is_bool(a)?(BOOL_OP):(INT_OP))
        case BinOpAdd:
        case BinOpAddNUW:
        case BinOpAddNSW: op = spv::OpIAdd; break;
        case BinOpSub:
        case BinOpSubNUW:
        case BinOpSubNSW: op = spv::OpISub; break;
        case BinOpMul:
        case BinOpMulNUW:
        case BinOpMulNSW: op = spv::OpIMul; break;
        case BinOpSDiv: op = spv::OpSDiv; break;
        case BinOpUDiv: op = spv::OpUDiv; break;
        case BinOpSRem: op = spv::OpSRem; break;
        case BinOpURem: op = spv::OpUMod; break;
        case BinOpShl: op = spv::OpShiftLeftLogical; break;
        case BinOpLShr: op = spv::OpShiftRightLogical; break;
        case BinOpAShr: op = spv::OpShiftRightArithmetic; break;
        case BinOpBAnd: op = BOOL_OR_INT_OP(spv::OpLogicalAnd, spv::OpBitwiseAnd); break;
        case BinOpBOr: op = BOOL_OR_INT_OP(spv::OpLogicalOr, spv::OpBitwiseOr); break;
        case BinOpBXor: op = BOOL_OR_INT_OP(spv::OpLogicalNotEqual, spv::OpBitwiseXor); break;
        case BinOpFAdd: op = spv::OpFAdd; break;
        case BinOpFSub: op = spv::OpFSub; break;
        case BinOpFMul: op = spv::OpFMul; break;
        case BinOpFDiv: op = spv::OpFDiv; break;
        case BinOpFRem: op = spv::OpFRem; break;
#undef BOOL_OR_INT_OP
        case BinOpAtan2: _builtin = GLSLstd450Atan2; goto defbuiltin;
        case BinOpStep: _builtin = GLSLstd450Step; goto defbuiltin;
        case BinOpPow: _builtin = GLSLstd450Pow; goto defbuiltin;
        case BinOpCross: _builtin = GLSLstd450Cross; goto defbuiltin;
        default: {
            SCOPES_ERROR(CGenUnsupportedBinOp);
        } break;
        }
        val = builder.createBinOp(op, rtype, a, b);
        goto done;
    defbuiltin:
        val = builder.createBuiltinCall(rtype, glsl_ext_inst, _builtin, { a, b });
    done:
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_TriOp(const TriOpRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto a = SCOPES_GET_RESULT(ref_to_value(node->value1));
        auto b = SCOPES_GET_RESULT(ref_to_value(node->value2));
        auto c = SCOPES_GET_RESULT(ref_to_value(node->value3));
        GLSLstd450 _builtin = GLSLstd450Bad;
        switch(node->op) {
        case TriOpFMix: _builtin = GLSLstd450FMix; break;
        default: {
            SCOPES_ERROR(CGenUnsupportedTriOp);
        } break;
        }
        auto val = builder.createBuiltinCall(
            builder.getTypeId(a), glsl_ext_inst, _builtin, { a, b, c });
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_Cast(const CastRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto val = SCOPES_GET_RESULT(ref_to_value(node->value));
        auto ty = SCOPES_GET_RESULT(type_to_spirv_type(node->get_type()));
        if (builder.getTypeId(val) != ty) {
            spv::Op op = spv::Op::OpMax;
            switch(node->op) {
            case CastPtrToRef: break;
            case CastRefToPtr: break;
#define CAST_OP(SRC, OP) case SRC: op = OP; break;
            CAST_OP(CastBitcast, spv::OpBitcast)
            CAST_OP(CastIntToPtr, spv::OpConvertUToPtr)
            CAST_OP(CastPtrToInt, spv::OpConvertPtrToU)
            CAST_OP(CastSExt, spv::OpSConvert)
            CAST_OP(CastITrunc, spv::OpSConvert)
            CAST_OP(CastZExt, spv::OpUConvert)
            CAST_OP(CastFPTrunc, spv::OpFConvert)
            CAST_OP(CastFPExt, spv::OpFConvert)
            CAST_OP(CastFPToUI, spv::OpConvertFToU)
            CAST_OP(CastFPToSI, spv::OpConvertFToS)
            CAST_OP(CastUIToFP, spv::OpConvertUToF)
            CAST_OP(CastSIToFP, spv::OpConvertSToF)
#undef CAST_OP
            default: {
                SCOPES_ERROR(CGenUnsupportedCastOp);
            } break;
            }
            if (op != spv::Op::OpMax) {
                val = builder.createUnaryOp(op, ty, val);
            }
        }
        map_phi({ val }, node);
        return {};
    }

    SCOPES_RESULT(void) translate_Switch(const SwitchRef &node) {
        SCOPES_RESULT_TYPE(void);
        auto expr = SCOPES_GET_RESULT(ref_to_value(node->expr));
        auto bb = builder.getBuildPoint();
        auto bbdefault = &builder.makeNewBlock();
        position_builder_at_end(bb);
        int count = (int)node->cases.size();
        assert(count);
        auto _sw = new spv::Instruction(
            spv::NoResult, spv::NoType, spv::OpSwitch);
        bb->addInstruction(std::unique_ptr<spv::Instruction>(_sw));
        _sw->addIdOperand(expr);
        _sw->addIdOperand(bbdefault->getId());
        bbdefault->addPredecessor(bb);
        int i = count;
        spv::Block *lastbb = nullptr;
        while (i-- > 0) {
            auto &_case = *node->cases[i];
            spv::Block *bbcase = nullptr;
            if (_case.kind == CK_Default) {
                position_builder_at_end(bbdefault);
                bbcase = bbdefault;
            } else if (_case.body.empty()) {
                assert(lastbb);
                auto lit = SCOPES_GET_RESULT(
                    extract_integer_constant(_case.literal));

                _sw->addImmediateOperand(lit);
                _sw->addIdOperand(lastbb->getId());
                continue;
            } else {
                auto lit = SCOPES_GET_RESULT(
                    extract_integer_constant(_case.literal));
                bbcase = &builder.makeNewBlock();

                _sw->addImmediateOperand(lit);
                _sw->addIdOperand(bbcase->getId());
                bbcase->addPredecessor(bb);

                position_builder_at_end(bbcase);
            }
            SCOPES_CHECK_RESULT(translate_block(_case.body));
            if (!_case.body.terminator) {
                assert(lastbb);
                builder.createBranch(lastbb);
            }
            lastbb = bbcase;
        }
        return {};
    }

    void position_builder_at_end(spv::Block *bb) {
        builder.setBuildPoint(bb);
    }

    SCOPES_RESULT(void) translate_CondBr(const CondBrRef &node) {
        SCOPES_RESULT_TYPE(void);

        auto cond = SCOPES_GET_RESULT(ref_to_value(node->cond));
        assert(cond);
        auto bbthen = &builder.makeNewBlock();
        auto bbelse = &builder.makeNewBlock();

        spv::Block *merge_block = nullptr;
        for (auto it = label_info_stack.rbegin(); it != label_info_stack.rend(); ++it) {
            if (it->label->splitpoints.count(node.unref()) && it->bb_merge) {
                merge_block = it->bb_merge;
                break;
            }
        }

        if (merge_block) {
            builder.createSelectionMerge(merge_block, 0);
        }
        builder.createConditionalBranch(cond, bbthen, bbelse);

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
        SCOPES_RESULT_TYPE(void);
        SCOPES_TRACE_CODEGEN(node);
        switch(node->kind()) {
        #define T(NAME, BNAME, CLASS) \
            case NAME: SCOPES_CHECK_RESULT(translate_ ## CLASS(node.cast<CLASS>())); break;
        SCOPES_INSTRUCTION_VALUE_KIND()
        #undef T
            default: assert(false); break;
        }
        return {};
    }

    SCOPES_RESULT(spv::Id) ref_to_value(const ValueIndex &ref) {
        SCOPES_RESULT_TYPE(spv::Id);
        auto it = ref2value.find(ref);
        if (it != ref2value.end())
            return it->second;
        SCOPES_TRACE_CODEGEN(ref.value);
        spv::Id value = 0;
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

    SCOPES_RESULT(spv::Id) PureCast_to_value(const PureCastRef &node) {
        SCOPES_RESULT_TYPE(spv::Id);
        uint64_t flags = 0;
        if (node->value.isa<Global>()) {
            flags = node->value.cast<Global>()->flags;
        }
        auto LLT = SCOPES_GET_RESULT(type_to_spirv_type(node->get_type(), flags));
        auto val = SCOPES_GET_RESULT(ref_to_value(ValueIndex(node->value)));
        if (builder.getTypeId(val) == LLT)
            return val;
        return builder.createUnaryOp(spv::OpBitcast, LLT, val);
    }

    SCOPES_RESULT(spv::Id) Undef_to_value(const UndefRef &node) {
        SCOPES_RESULT_TYPE(spv::Id);
        auto ty = SCOPES_GET_RESULT(type_to_spirv_type(node->get_type()));
        return builder.createUndefined(ty);
    }

    SCOPES_RESULT(spv::Id) Global_to_value(const GlobalRef &node) {
        SCOPES_RESULT_TYPE(spv::Id);
        spv::StorageClass sc = SCOPES_GET_RESULT(storage_class_from_extern_class(
            node->storage_class));
        const char *name = nullptr;
        spv::BuiltIn builtin = spv::BuiltInMax;
        switch(node->name.value()) {
        #define T(NAME) \
        case SYM_SPIRV_BuiltIn ## NAME: \
            builtin = spv::BuiltIn ## NAME; break;
            B_SPIRV_BUILTINS()
        #undef T
            default:
                name = node->name.name()->data;
                break;
        }
        auto ty = SCOPES_GET_RESULT(type_to_spirv_type(node->element_type, node->flags));
        auto id = builder.createVariable(sc, ty, name);
        if (builtin != spv::BuiltInMax) {
            builder.addDecoration(id, spv::DecorationBuiltIn, builtin);
        }
        switch(sc) {
        case spv::StorageClassInput:
        case spv::StorageClassOutput: {
            assert(entry_point);
            entry_point->addIdOperand(id);
        } break;
        case spv::StorageClassUniformConstant:
        case spv::StorageClassUniform: {
            //builder.addDecoration(id, spv::DecorationDescriptorSet, 0);
            if (node->binding >= 0) {
                builder.addDecoration(id, spv::DecorationBinding, node->binding);
            }
            if (node->location >= 0) {
                builder.addDecoration(id, spv::DecorationLocation, node->location);
            }
            if (builder.isImageType(ty)) {
                auto flags = node->flags;
                if (flags & GF_Volatile) {
                    builder.addDecoration(id, spv::DecorationVolatile);
                }
                if (flags & GF_Coherent) {
                    builder.addDecoration(id, spv::DecorationCoherent);
                }
                if (flags & GF_Restrict) {
                    builder.addDecoration(id, spv::DecorationRestrict);
                }
                if (flags & GF_NonWritable) {
                    builder.addDecoration(id, spv::DecorationNonWritable);
                }
                if (flags & GF_NonReadable) {
                    builder.addDecoration(id, spv::DecorationNonReadable);
                }
            }
        } break;
        default: {
            if (node->location >= 0) {
                builder.addDecoration(id, spv::DecorationLocation, node->location);
            }
        } break;
        }
        return id;
    }

    SCOPES_RESULT(spv::Id) ConstInt_to_value(const ConstIntRef &node) {
        SCOPES_RESULT_TYPE(spv::Id);
        auto TT = SCOPES_GET_RESULT(storage_type(node->get_type()));
        //auto T = SCOPES_GET_RESULT(type_to_spirv_type(node->get_type()));
        auto it = cast<IntegerType>(TT);
        if (it->issigned) {
            int64_t value = node->value;
            switch(it->width) {
            case 8: return builder.makeIntConstant(
                builder.makeIntegerType(8, true), value);
            case 16: return builder.makeIntConstant(
                builder.makeIntegerType(16, true), value);
            case 32: return builder.makeIntConstant(value);
            case 64: return builder.makeInt64Constant(value);
            default: break;
            }
        } else {
            uint64_t value = node->value;
            switch(it->width) {
            case 1: return builder.makeBoolConstant(value);
            case 8: return builder.makeIntConstant(
                builder.makeIntegerType(8, false), value);
            case 16: return builder.makeIntConstant(
                builder.makeIntegerType(16, false), value);
            case 32: return builder.makeUintConstant(value);
            case 64: return builder.makeUint64Constant(value);
            default: break;
            }
        }
        SCOPES_ERROR(CGenTypeUnsupportedInTarget, node->get_type());
    }

    SCOPES_RESULT(spv::Id) ConstReal_to_value(const ConstRealRef &node) {
        SCOPES_RESULT_TYPE(spv::Id);
        auto TT = SCOPES_GET_RESULT(storage_type(node->get_type()));
        //auto T = SCOPES_GET_RESULT(type_to_spirv_type(node->get_type()));
        auto rt = cast<RealType>(TT);
        double value = node->value;
        switch(rt->width) {
        case 32: return builder.makeFloatConstant(value);
        case 64: return builder.makeDoubleConstant(value);
        default: break;
        }
        SCOPES_ERROR(CGenTypeUnsupportedInTarget, node->get_type());
    }

    SCOPES_RESULT(spv::Id) ConstPointer_to_value(const ConstPointerRef &node) {
        SCOPES_RESULT_TYPE(spv::Id);
        SCOPES_ERROR(CGenFailedToTranslateValue, node->kind());
#if 0
        auto TT = SCOPES_GET_RESULT(storage_type(node->get_type()));
        auto LLT = SCOPES_GET_RESULT(type_to_spirv_type(node->get_type()));
        if (is_function_pointer(TT)) {
            StyledString ss;
            ss.out << "IL->SPIR: function pointer constants are unsupported";
            SCOPES_LOCATION_ERROR(ss.str());
        }
        auto pt = cast<PointerType>(TT);
        auto val = SCOPES_GET_RESULT(ref_to_value(node->value));
        auto id = builder.createVariable(spv::StorageClassFunction,
            builder.getTypeId(val), nullptr);
        builder.getInstruction(id)->addIdOperand(val);
        return id;
#endif
    }

    SCOPES_RESULT(spv::Id) ConstAggregate_to_value(const ConstAggregateRef &node) {
        SCOPES_RESULT_TYPE(spv::Id);
        //auto TT = SCOPES_GET_RESULT(storage_type(node->get_type()));
        auto LLT = SCOPES_GET_RESULT(type_to_spirv_type(node->get_type()));
        size_t count = node->values.size();
        Ids values;
        for (size_t i = 0; i < count; ++i) {
            values.push_back(SCOPES_GET_RESULT(
                ref_to_value(ValueIndex(get_field(node, i)))));
        }
        return builder.makeCompositeConstant(LLT, values);
    }

    SCOPES_RESULT(void) build_call(const CallRef &call,
        const Type *functype, spv::Function *func, const TypedValues &args) {
        SCOPES_RESULT_TYPE(void);
        size_t argcount = args.size();

        auto fi = cast<FunctionType>(functype);

        //auto rtype = abi_return_type(fi);

        Ids values;
        values.reserve(argcount + 1);

        //auto retT = SCOPES_GET_RESULT(type_to_spirv_type(rtype));

        for (size_t i = 0; i < argcount; ++i) {
            values.push_back(SCOPES_GET_RESULT(ref_to_value(args[i])));
        }

        size_t fargcount = fi->argument_types.size();
        assert(argcount >= fargcount);
        // make variadic calls C compatible
        if (fi->flags & FF_Variadic) {
            SCOPES_ERROR(CGenTypeUnsupportedInTarget, functype);
        }

        auto ret = builder.createFunctionCall(func, values);

        if (fi->has_exception()) {
            bool has_except_value = is_returning_value(fi->except_type);
            bool has_return_value = is_returning_value(fi->return_type);
            if (has_except_value) {
                assert(call->except);
                auto except = builder.createCompositeExtract(ret,
                    builder.getContainedTypeId(builder.getTypeId(ret), 1),
                    1);
                Ids values;
                struct_to_values(values, fi->except_type, except);
                map_phi(values, call->except);
            }
            auto old_bb = builder.getBuildPoint();
            auto bb_except = &builder.makeNewBlock();
            position_builder_at_end(bb_except);
            SCOPES_CHECK_RESULT(translate_block(call->except_body));
            position_builder_at_end(old_bb);
            if (!is_returning(fi->return_type)) {
                // always raises
                builder.createBranch(bb_except);
                return {};
            } else {
                auto bb = &builder.makeNewBlock();
                auto ok = ret;
                if (has_except_value || has_return_value) {
                    ok = builder.createCompositeExtract(ret,
                        builder.getContainedTypeId(builder.getTypeId(ret), 0), 0);
                }
                builder.createConditionalBranch(ok, bb, bb_except);
                position_builder_at_end(bb);
                if (has_return_value) {
                    int retvalue_index = (has_except_value?2:1);
                    ret = builder.createCompositeExtract(ret,
                        builder.getContainedTypeId(builder.getTypeId(ret), retvalue_index),
                        retvalue_index);
                    Ids values;
                    struct_to_values(values, fi->return_type, ret);
                    map_phi(values, call);
                }
            }
        } else if (!is_returning(fi->return_type)) {
            builder.makeUnreachable();
        } else {
            Ids values;
            struct_to_values(values, fi->return_type, ret);
            map_phi(values, call);
        }
        return {};
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

    SCOPES_RESULT(void) generate(std::vector<unsigned int> &result,
        Symbol target, const FunctionRef &entry) {
        SCOPES_RESULT_TYPE(void);

        auto needfi = native_ro_pointer_type(
            function_type(empty_arguments_type(), {}));
        auto hasfi = entry->get_type();
        if (hasfi != needfi) {
            SCOPES_ERROR(CGenEntryFunctionSignatureMismatch, needfi, hasfi);
        }

        builder.setSource(spv::SourceLanguageGLSL, 450);
        glsl_ext_inst = builder.import("GLSL.std.450");

        auto func = SCOPES_GET_RESULT(Function_to_function(entry));

        switch(target.value()) {
        case SYM_TargetVertex: {
            builder.addCapability(spv::CapabilityShader);
            entry_point = builder.addEntryPoint(spv::ExecutionModelVertex, func, "main");
        } break;
        case SYM_TargetFragment: {
            builder.addCapability(spv::CapabilityShader);
            entry_point = builder.addEntryPoint(spv::ExecutionModelFragment, func, "main");
            execution_modes.insert({ spv::ExecutionModeOriginLowerLeft,
                 nullptr });
        } break;
        case SYM_TargetGeometry: {
            builder.addCapability(spv::CapabilityShader);
            entry_point = builder.addEntryPoint(spv::ExecutionModelGeometry, func, "main");
        } break;
        case SYM_TargetCompute: {
            builder.addCapability(spv::CapabilityShader);
            entry_point = builder.addEntryPoint(spv::ExecutionModelGLCompute, func, "main");
        } break;
        default: {
            SCOPES_ERROR(CGenUnsupportedTarget, target);
        } break;
        }

        SCOPES_CHECK_RESULT(process_functions());

        for (auto &&entry : execution_modes) {
            if (entry.second) {
                builder.addExecutionMode(func, (spv::ExecutionMode)entry.first,
                    entry.second->values[0],
                    entry.second->values[1],
                    entry.second->values[2]);
            } else {
                builder.addExecutionMode(func, (spv::ExecutionMode)entry.first);
            }
        }

        builder.dump(result);

        SCOPES_CHECK_RESULT(verify_spirv(result));

        return {};
    }

};

//------------------------------------------------------------------------------

SCOPES_RESULT(void) optimize_spirv(std::vector<unsigned int> &result, int opt_level) {
    SCOPES_RESULT_TYPE(void);
    spvtools::Optimizer optimizer(SPV_ENV_UNIVERSAL_1_2);
    /*
    optimizer.SetMessageConsumer([](spv_message_level_t level, const char* source,
        const spv_position_t& position,
        const char* message) {
    SCOPES_CERR << StringifyMessage(level, source, position, message)
    << std::endl;
    });*/
    StyledStream ss(SCOPES_CERR);
    optimizer.SetMessageConsumer([&ss](spv_message_level_t level, const char*,
        const spv_position_t& position,
        const char* message) {
        switch (level) {
        case SPV_MSG_FATAL:
        case SPV_MSG_INTERNAL_ERROR:
        case SPV_MSG_ERROR:
            ss << Style_Error << "error: " << Style_None
                << position.index << ": " << message << std::endl;
            break;
        case SPV_MSG_WARNING:
            ss << Style_Warning << "warning: " << Style_None
                << position.index << ": " << message << std::endl;
            break;
        case SPV_MSG_INFO:
            ss << Style_Comment << "info: " << Style_None
                << position.index << ": " << message << std::endl;
            break;
        default:
            break;
        }
    });

    if (opt_level == 3) {
        optimizer.RegisterPass(spvtools::CreateStripDebugInfoPass());
        optimizer.RegisterPass(spvtools::CreateFreezeSpecConstantValuePass());
    }

    if (opt_level == 3) {
        optimizer.RegisterPass(spvtools::CreateInlineExhaustivePass());
    }
    optimizer.RegisterPass(spvtools::CreateLocalAccessChainConvertPass());
    optimizer.RegisterPass(spvtools::CreateInsertExtractElimPass());
    optimizer.RegisterPass(spvtools::CreateLocalSingleBlockLoadStoreElimPass());
    optimizer.RegisterPass(spvtools::CreateLocalSingleStoreElimPass());
    optimizer.RegisterPass(spvtools::CreateBlockMergePass());
    optimizer.RegisterPass(spvtools::CreateEliminateDeadConstantPass());
    optimizer.RegisterPass(spvtools::CreateFoldSpecConstantOpAndCompositePass());
    optimizer.RegisterPass(spvtools::CreateUnifyConstantPass());

    optimizer.RegisterPass(spvtools::CreateDeadBranchElimPass());
    optimizer.RegisterPass(spvtools::CreateLocalMultiStoreElimPass());
    if (opt_level == 3) {
        optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
    }
    optimizer.RegisterPass(spvtools::CreateCommonUniformElimPass());

    optimizer.RegisterPass(spvtools::CreateFlattenDecorationPass());
    //optimizer.RegisterPass(spvtools::CreateCompactIdsPass());

    std::vector<unsigned int> oldresult = result;
    result.clear();
    if (!optimizer.Run(oldresult.data(), oldresult.size(), &result)) {
        SCOPES_ERROR(CGenBackendOptimizationFailed);
    }

    SCOPES_CHECK_RESULT(verify_spirv(result));
    return {};
}

SCOPES_RESULT(const String *) compile_spirv(Symbol target, const FunctionRef &fn, uint64_t flags) {
    SCOPES_RESULT_TYPE(const String *);
    Timer sum_compile_time(TIMER_CompileSPIRV);

    //SCOPES_CHECK_RESULT(fn->verify_compilable());

    SPIRVGenerator ctx;
    if (flags & CF_NoDebugInfo) {
        ctx.use_debug_info = false;
    }

    std::vector<unsigned int> result;
    {
        Timer generate_timer(TIMER_GenerateSPIRV);
        SCOPES_CHECK_RESULT(ctx.generate(result, target, fn));
    }

    if (flags & CF_O3) {
        int level = 0;
        if ((flags & CF_O3) == CF_O1)
            level = 1;
        else if ((flags & CF_O3) == CF_O2)
            level = 2;
        else if ((flags & CF_O3) == CF_O3)
            level = 3;
        SCOPES_CHECK_RESULT(optimize_spirv(result, level));
    }

    if (flags & CF_DumpModule) {
    } else if (flags & CF_DumpFunction) {
    }
    if (flags & CF_DumpDisassembly) {
        disassemble_spirv(result);
    }

    size_t bytesize = sizeof(unsigned int) * result.size();

    return String::from((char *)&result[0], bytesize);
}

SCOPES_RESULT(const String *) compile_glsl(int version, Symbol target, const FunctionRef &fn, uint64_t flags) {
    SCOPES_RESULT_TYPE(const String *);
    Timer sum_compile_time(TIMER_CompileSPIRV);

    //SCOPES_CHECK_RESULT(fn->verify_compilable());

    SPIRVGenerator ctx;
    if (flags & CF_NoDebugInfo) {
        ctx.use_debug_info = false;
    }

    std::vector<unsigned int> result;
    {
        Timer generate_timer(TIMER_GenerateSPIRV);
        SCOPES_CHECK_RESULT(ctx.generate(result, target, fn));
    }

    if (flags & CF_O3) {
        int level = 0;
        if ((flags & CF_O3) == CF_O1)
            level = 1;
        else if ((flags & CF_O3) == CF_O2)
            level = 2;
        else if ((flags & CF_O3) == CF_O3)
            level = 3;
        SCOPES_CHECK_RESULT(optimize_spirv(result, level));
    }

    if (flags & CF_DumpDisassembly) {
        disassemble_spirv(result);
    }

	spirv_cross::CompilerGLSL glsl(std::move(result));

    /*
    // The SPIR-V is now parsed, and we can perform reflection on it.
    spirv_cross::ShaderResources resources = glsl.get_shader_resources();
    // Get all sampled images in the shader.
    for (auto &resource : resources.sampled_images)
    {
        unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
        unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
        printf("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);

        // Modify the decoration to prepare it for GLSL.
        glsl.unset_decoration(resource.id, spv::DecorationDescriptorSet);

        // Some arbitrary remapping if we want.
        glsl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
    }
    */

    // Set some options.
    spirv_cross::CompilerGLSL::Options options;
    options.version = (version <= 0)?450:version;
    glsl.set_common_options(options);

    // Compile to GLSL, ready to give to GL driver.
    std::string source = glsl.compile();

    if (flags & (CF_DumpModule|CF_DumpFunction)) {
        std::cout << source << std::endl;
    }

    return String::from_stdstring(source);
}

} // namespace scopes
