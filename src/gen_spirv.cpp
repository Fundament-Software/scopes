/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "gen_spirv.hpp"
#include "styled_stream.hpp"
#include "string.hpp"
#include "error.hpp"
#include "scc.hpp"
#include "types.hpp"
#include "anchor.hpp"
#include "stream_label.hpp"
#include "timer.hpp"
#include "compiler_flags.hpp"
#include "hash.hpp"

#include "glslang/SpvBuilder.h"
#include "glslang/disassemble.h"
#include "glslang/GLSL.std.450.h"
#include "SPIRV-Cross/spirv_glsl.hpp"
#include "spirv-tools/libspirv.hpp"
#include "spirv-tools/optimizer.hpp"

#include <vector>

#include "verify_tools.inc"
#include "dyn_cast.inc"

namespace scopes {

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
        SCOPES_LOCATION_ERROR(String::from("SPIR-V validation found errors"));
    }
    return true;
}


struct SPIRVGenerator {
    struct HashFuncLabelPair {
        size_t operator ()(const std::pair<spv::Function *, Label *> &value) const {
            return
                hash2(std::hash<spv::Function *>()(value.first),
                    std::hash<Label *>()(value.second));
        }
    };

    typedef std::pair<spv::Function *, Parameter *> ParamKey;
    struct HashFuncParamPair {
        size_t operator ()(const ParamKey &value) const {
            return
                hash2(std::hash<spv::Function *>()(value.first),
                    std::hash<Parameter *>()(value.second));
        }
    };

    typedef std::pair<const Type *, uint64_t> TypeKey;
    struct HashTypeFlagsPair {
        size_t operator ()(const TypeKey &value) const {
            return
                hash2(std::hash<const Type *>{}(value.first),
                    std::hash<uint64_t>{}(value.second));
        }
    };

    spv::SpvBuildLogger logger;
    spv::Builder builder;
    SCCBuilder scc;

    Label *active_function;
    spv::Function *active_function_value;
    spv::Id glsl_ext_inst;

    bool use_debug_info;

    std::unordered_map<Label *, spv::Function *> label2func;
    std::unordered_map< std::pair<spv::Function *, Label *>,
        spv::Block *, HashFuncLabelPair> label2bb;
    std::vector< std::pair<Label *, Label *> > bb_label_todo;

    //std::unordered_map<Label *, LLVMValueRef> label2md;
    //std::unordered_map<SourceFile *, LLVMValueRef> file2value;
    std::unordered_map< ParamKey, spv::Id, HashFuncParamPair> param2value;

    std::unordered_map<std::pair<const Type *, uint64_t>,
        spv::Id, HashTypeFlagsPair> type_cache;

    std::unordered_map<Any, spv::Id, Any::Hash> const_cache;

    Label::UserMap user_map;

    SPIRVGenerator() :
        builder('S' << 24 | 'C' << 16 | 'O' << 8 | 'P', &logger),
        active_function(nullptr),
        active_function_value(nullptr),
        glsl_ext_inst(0),
        use_debug_info(true) {

    }

    static SCOPES_RESULT(spv::Dim) dim_from_symbol(Symbol sym) {
        SCOPES_RESULT_TYPE(spv::Dim);
        switch(sym.value()) {
        #define T(NAME) \
            case SYM_SPIRV_Dim ## NAME: return spv::Dim ## NAME;
            B_SPIRV_DIM()
        #undef T
            default:
                SCOPES_LOCATION_ERROR(
                    String::from(
                        "IL->SPIR: unsupported dimensionality"));
                break;
        }
        return spv::DimMax;
    }

    SCOPES_RESULT(spv::ImageFormat) image_format_from_symbol(Symbol sym) {
        SCOPES_RESULT_TYPE(spv::ImageFormat);
        switch(sym.value()) {
        #define T(NAME) \
            case SYM_SPIRV_ImageFormat ## NAME: return spv::ImageFormat ## NAME;
            B_SPIRV_IMAGE_FORMAT()
        #undef T
            default:
                SCOPES_LOCATION_ERROR(
                    String::from(
                        "IL->SPIR: unsupported image format"));
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
                SCOPES_LOCATION_ERROR(
                    String::from(
                        "IL->SPIR: unsupported execution mode"));
                break;
        }
        return spv::ExecutionModeMax;
    }

    SCOPES_RESULT(spv::StorageClass) storage_class_from_extern_class(Symbol sym) {
        SCOPES_RESULT_TYPE(spv::StorageClass);
        switch(sym.value()) {
        #define T(NAME) \
            case SYM_SPIRV_StorageClass ## NAME: return spv::StorageClass ## NAME;
            B_SPIRV_STORAGE_CLASS()
        #undef T
            case SYM_Unnamed:
                SCOPES_LOCATION_ERROR(
                    String::from(
                        "IL->SPIR: pointers with C storage class"
                        " are unsupported"));
                break;
            default:
                SCOPES_LOCATION_ERROR(
                    String::from(
                        "IL->SPIR: unsupported storage class for pointer"));
                break;
        }
        return spv::StorageClassMax;
    }

    bool parameter_is_bound(Parameter *param) {
        return param2value.find({active_function_value, param}) != param2value.end();
    }

    SCOPES_RESULT(spv::Id) resolve_parameter(Parameter *param) {
        SCOPES_RESULT_TYPE(spv::Id);
        auto it = param2value.find({active_function_value, param});
        if (it == param2value.end()) {
            assert(active_function_value);
            if (param->label) {
                location_message(param->label->anchor, String::from("declared here"));
            }
            StyledString ss;
            ss.out << "IL->SPIR: can't access free variable " << param;
            SCOPES_LOCATION_ERROR(ss.str());
        }
        assert(it->second);
        return it->second;
    }

    SCOPES_RESULT(spv::Id) argument_to_value(Any value) {
        SCOPES_RESULT_TYPE(spv::Id);
        if (value.type == TYPE_Parameter) {
            return resolve_parameter(value.parameter);
        }
        if (value.type != TYPE_String) {
            switch(value.type->kind()) {
            case TK_Integer: {
                auto it = cast<IntegerType>(value.type);
                if (it->issigned) {
                    switch(it->width) {
                    case 8: return builder.makeIntConstant(
                        builder.makeIntegerType(8, true), value.i8);
                    case 16: return builder.makeIntConstant(
                        builder.makeIntegerType(16, true), value.i16);
                    case 32: return builder.makeIntConstant(value.i32);
                    case 64: return builder.makeInt64Constant(value.i64);
                    default: break;
                    }
                } else {
                    switch(it->width) {
                    case 1: return builder.makeBoolConstant(value.i1);
                    case 8: return builder.makeIntConstant(
                        builder.makeIntegerType(8, false), value.i8);
                    case 16: return builder.makeIntConstant(
                        builder.makeIntegerType(16, false), value.i16);
                    case 32: return builder.makeUintConstant(value.u32);
                    case 64: return builder.makeUint64Constant(value.u64);
                    default: break;
                    }
                }
                StyledString ss;
                ss.out << "IL->SPIR: unsupported integer constant type " << value.type;
                SCOPES_LOCATION_ERROR(ss.str());
            } break;
            case TK_Real: {
                auto rt = cast<RealType>(value.type);
                switch(rt->width) {
                case 32: return builder.makeFloatConstant(value.f32);
                case 64: return builder.makeDoubleConstant(value.f64);
                default: break;
                }
                StyledString ss;
                ss.out << "IL->SPIR: unsupported real constant type " << value.type;
                SCOPES_LOCATION_ERROR(ss.str());
            } break;
            case TK_Pointer: {
                if (is_function_pointer(value.type)) {
                    StyledString ss;
                    ss.out << "IL->SPIR: function pointer constants are unsupported";
                    SCOPES_LOCATION_ERROR(ss.str());
                }
                auto pt = cast<PointerType>(value.type);
                auto val = SCOPES_GET_RESULT(argument_to_value(SCOPES_GET_RESULT(pt->unpack(value.pointer))));
                auto id = builder.createVariable(spv::StorageClassFunction,
                    builder.getTypeId(val), nullptr);
                builder.getInstruction(id)->addIdOperand(val);
                return id;
            } break;
            case TK_Typename: {
                auto tn = cast<TypenameType>(value.type);
                assert(tn->finalized());
                Any storage_value = value;
                storage_value.type = tn->storage_type;
                return argument_to_value(storage_value);
            } break;
            case TK_Array: {
                auto ai = cast<ArrayType>(value.type);
                size_t count = ai->count;
                std::vector<spv::Id> values;
                for (size_t i = 0; i < count; ++i) {
                    values.push_back(SCOPES_GET_RESULT(argument_to_value(SCOPES_GET_RESULT(ai->unpack(value.pointer, i)))));
                }
                return builder.makeCompositeConstant(
                    SCOPES_GET_RESULT(type_to_spirv_type(value.type)), values);
            } break;
            case TK_Vector: {
                auto vi = cast<VectorType>(value.type);
                size_t count = vi->count;
                std::vector<spv::Id> values;
                for (size_t i = 0; i < count; ++i) {
                    values.push_back(SCOPES_GET_RESULT(argument_to_value(SCOPES_GET_RESULT(vi->unpack(value.pointer, i)))));
                }
                return builder.makeCompositeConstant(
                    SCOPES_GET_RESULT(type_to_spirv_type(value.type)), values);
            } break;
            case TK_Tuple: {
                auto ti = cast<TupleType>(value.type);
                size_t count = ti->types.size();
                std::vector<spv::Id> values;
                for (size_t i = 0; i < count; ++i) {
                    values.push_back(SCOPES_GET_RESULT(argument_to_value(SCOPES_GET_RESULT(ti->unpack(value.pointer, i)))));
                }
                return builder.makeCompositeConstant(
                    SCOPES_GET_RESULT(type_to_spirv_type(value.type)), values);
            } break;
            case TK_Union: {
                auto ui = cast<UnionType>(value.type);
                value.type = ui->tuple_type;
                return argument_to_value(value);
            } break;
            default: {
            } break;
            }
        }
        auto it = const_cache.find(value);
        if (it != const_cache.end()) {
            return it->second;
        }
        auto id = SCOPES_GET_RESULT(create_spirv_value(value));
        const_cache.insert({ value, id });
        return id;
    }

    SCOPES_RESULT(spv::Id) create_spirv_value(Any value) {
        SCOPES_RESULT_TYPE(spv::Id);
        if (value.type == TYPE_String) {
            return builder.createString(value.string->data);
        }
        switch(value.type->kind()) {
        case TK_Extern: {
            auto et = cast<ExternType>(value.type);
            spv::StorageClass sc = SCOPES_GET_RESULT(storage_class_from_extern_class(
                et->storage_class));
            const char *name = nullptr;
            spv::BuiltIn builtin = spv::BuiltInMax;
            switch(value.symbol.value()) {
            #define T(NAME) \
            case SYM_SPIRV_BuiltIn ## NAME: \
                builtin = spv::BuiltIn ## NAME; break;
                B_SPIRV_BUILTINS()
            #undef T
                default:
                    name = value.symbol.name()->data;
                    break;
            }
            auto ty = SCOPES_GET_RESULT(type_to_spirv_type(et->type, et->flags));
            auto id = builder.createVariable(sc, ty, name);
            if (builtin != spv::BuiltInMax) {
                builder.addDecoration(id, spv::DecorationBuiltIn, builtin);
            }
            switch(sc) {
            case spv::StorageClassUniformConstant:
            case spv::StorageClassUniform: {
                //builder.addDecoration(id, spv::DecorationDescriptorSet, 0);
                if (et->binding >= 0) {
                    builder.addDecoration(id, spv::DecorationBinding, et->binding);
                }
                if (et->location >= 0) {
                    builder.addDecoration(id, spv::DecorationLocation, et->location);
                }
                if (builder.isImageType(ty)) {
                    auto flags = et->flags;
                    if (flags & EF_Volatile) {
                        builder.addDecoration(id, spv::DecorationVolatile);
                    }
                    if (flags & EF_Coherent) {
                        builder.addDecoration(id, spv::DecorationCoherent);
                    }
                    if (flags & EF_Restrict) {
                        builder.addDecoration(id, spv::DecorationRestrict);
                    }
                    if (flags & EF_NonWritable) {
                        builder.addDecoration(id, spv::DecorationNonWritable);
                    }
                    if (flags & EF_NonReadable) {
                        builder.addDecoration(id, spv::DecorationNonReadable);
                    }
                }
            } break;
            default: {
                if (et->location >= 0) {
                    builder.addDecoration(id, spv::DecorationLocation, et->location);
                }
            } break;
            }
            return id;
        } break;
        default: break;
        };

        StyledString ss;
        ss.out << "IL->SPIR: cannot convert argument of type " << value.type;
        SCOPES_LOCATION_ERROR(ss.str());
        return 0;
    }

    bool is_bool(spv::Id value) {
        auto T = builder.getTypeId(value);
        return
            (builder.isVectorType(T)
             && builder.isBoolType(builder.getContainedTypeId(T)))
            || builder.isBoolType(T);
    }

    SCOPES_RESULT(void) write_anchor(const Anchor *anchor) {
        SCOPES_RESULT_TYPE(void);
        assert(anchor);
        assert(anchor->file);
        if (use_debug_info) {
            builder.addLine(
                SCOPES_GET_RESULT(argument_to_value(anchor->path().name())),
                anchor->lineno, anchor->column);
        }
        return true;
    }

    // set of processed SCC groups
    std::unordered_set<size_t> handled_loops;

    Label *lowest_common_ancestor(Label *a, Label *b, bool verbose = false) {
        // walk both labels simultaneously and stop as soon as we find a
        // collision with the other label's set
        std::unordered_set<Label *> done[2];
        Label *labels[2] = { a, b };
        while (labels[0] || labels[1]) {
            for (int i = 0; i < 2; ++i) {
                Label *l = labels[i];
                if (!l)
                    continue;
                if (!done[i].count(l)) {
                    if (verbose) {
                        StyledStream ss;
                        ss << "argument " << i << std::endl;
                        StreamLabelFormat fmt = StreamLabelFormat::single();
                        fmt.anchors = StreamLabelFormat::Line;
                        stream_label(ss, l, fmt);
                    }
                    if (done[(i+1)&1].count(l)) {
                        return l;
                    }
                    done[i].insert(l);
                    assert (l->is_basic_block_like());
                    auto &&enter = l->body.enter;
                    auto &&args = l->body.args;
                    if ((enter.type == TYPE_Builtin)
                        && (enter.builtin.value() == FN_Branch)) {
                        assert(args.size() >= 4);
                        Label *then_label = args[2].value;
                        //Label *else_label = args[3].value;
                        labels[i] = then_label;
                        continue;
                    } else if (!l->is_jumping()) {
                        Any arg = l->body.args[0].value;
                        if (arg.type == TYPE_Label) {
                            labels[i] = arg;
                            continue;
                        }
                    } else if (enter.type == TYPE_Label) {
                        labels[i] = enter;
                        continue;
                    }
                }
                labels[i] = nullptr;
            }
        }

        return nullptr;
    }

    SCOPES_RESULT(bool) handle_loop_label (Label *label,
        Label *&continue_label,
        Label *&break_label) {
        SCOPES_RESULT_TYPE(bool);
        auto &&group = scc.group(label);
        if (group.labels.size() <= 1)
            return false;
        if (handled_loops.count(group.index))
            return false;
        handled_loops.insert(group.index);
        auto &&labels = group.labels;
        Label *header_label = label;
        continue_label = nullptr;
        break_label = nullptr;
        std::unordered_set<Label *> break_labels;
        size_t count = labels.size();
        for (size_t i = 0; i < count; ++i) {
            Label *l = labels[i];
            if (l->is_calling(header_label)
                || l->is_continuing_to(header_label)) {
                if (continue_label) {
                    StyledStream ss;
                    ss << header_label->anchor << " for this loop" << std::endl;
                    ss << continue_label->body.anchor << " previous continue is here" << std::endl;
                    //stream_label(ss, continue_label, StreamLabelFormat::debug_single());
                    //stream_label(ss, l, StreamLabelFormat::debug_single());
                    set_active_anchor(l->body.anchor);
                    SCOPES_LOCATION_ERROR(String::from(
                        "IL->SPIR: duplicate continue label found. only one continue label is permitted per loop."));
                }
                continue_label = l;
            }
            auto &&enter = l->body.enter;
            if ((enter.type == TYPE_Builtin)
                && (enter.builtin.value() == FN_Branch)) {
                auto &&args = l->body.args;
                assert(args.size() >= 4);
                Label *then_label = args[2].value;
                Label *else_label = args[3].value;
                if (scc.group_id(then_label) != group.index) {
                    break_labels.insert(then_label);
                } else if (scc.group_id(else_label) != group.index) {
                    break_labels.insert(else_label);
                }
            }
        }
        assert(continue_label);
        assert(continue_label->is_basic_block_like());

        if (break_labels.empty()) {
            SCOPES_LOCATION_ERROR(String::from(
                "IL->SPIR: loop is infinite"));
        }

        // as long as the continue label is dominated by a single predecessor, the
        // predecessor becomes the continue label; that way we move as much
        // unconditional code as possible into the continue section
        while (true) {
            Label *label = get_single_caller(continue_label);
            if (!label)
                break;
            continue_label = label;
        }

        if (break_labels.size() > 1) {
            Label *lca = nullptr;
            for (auto label : break_labels) {
                if (!lca) {
                    lca = label;
                } else {
                    lca = lowest_common_ancestor(lca, label);
                    if (!lca)
                        break;
                }
            }
            if (!lca) {
                StyledStream ss;
                ss << header_label->anchor << " for this loop" << std::endl;
                for (auto label : break_labels) {
                    ss << label->anchor << " found exit point" << std::endl;
                    label->anchor->stream_source_line(ss);
                    if (!lca) {
                        lca = label;
                    } else {
                        Label *old_lca = lca;
                        lca = lowest_common_ancestor(lca, label);
                        if (lca) {
                            ss << lca->anchor << " found merge candidate" << std::endl;
                            lca->anchor->stream_source_line(ss);
                        } else {
                            ss << label->anchor << " but exits function" << std::endl;
                            lca = old_lca;
                        }
                    }
                }
                SCOPES_LOCATION_ERROR(String::from(
                    "IL->SPIR: cannot merge multiple loop exit points."));
            }
            break_label = lca;
        } else {
            break_label = *break_labels.begin();
        }

        assert(break_label->is_basic_block_like());
        #if 0
        StyledStream ss;
        ss << "loop found:" << std::endl;
        ss << "    labels in group:";
        for (size_t i = 0; i < count; ++i) {
            ss << " " << labels[i];
        }
        ss << std::endl;
        ss << "    entry: " << label << std::endl;
        if (continue_label) {
            ss << "    continue: " << continue_label << std::endl;
        }
        if (break_label) {
            ss << "    break: " << break_label << std::endl;
        }
        #endif
        return true;
    }

    spv::Id build_extract_image(spv::Id value) {
        spv::Id typeId = builder.getTypeId(value);
        if (builder.isSampledImageType(typeId)) {
            spv::Id imgtypeId = builder.getImageType(value);
            return builder.createUnaryOp(spv::OpImage, imgtypeId, value);
        }
        return value;
    }

    void bind_parameter(Parameter *param, spv::Id value) {
        assert(value);
        param2value[{active_function_value, param}] = value;
    }

    SCOPES_RESULT(void) write_label_body(Label *label) {
    repeat:
        SCOPES_RESULT_TYPE(void);
        assert(label->body.is_complete());

        auto &&body = label->body;
        auto &&enter = body.enter;
        auto &&args = body.args;

        set_active_anchor(label->body.anchor);

        SCOPES_CHECK_RESULT(write_anchor(label->body.anchor));

        Label *continue_label = nullptr;
        Label *break_label = nullptr;
        if (SCOPES_GET_RESULT(handle_loop_label(label, continue_label, break_label))) {
            spv::Block *bb_continue = nullptr;
            spv::Block *bb_merge = nullptr;
            unsigned int control = spv::LoopControlMaskNone;
            bb_continue = SCOPES_GET_RESULT(label_to_basic_block(continue_label, true));
            bb_merge = SCOPES_GET_RESULT(label_to_basic_block(break_label, true));
            builder.createLoopMerge(bb_merge, bb_continue, control);
            auto bb = &builder.makeNewBlock();
            builder.createBranch(bb);
            builder.setBuildPoint(bb);
        }

        assert(!args.empty());
        size_t argcount = args.size() - 1;
        size_t argn = 1;
#define READ_ANY(NAME) \
        assert(argn <= argcount); \
        Any &NAME = args[argn++].value;
#define READ_VALUE(NAME) \
        assert(argn <= argcount); \
        auto && _arg_ ## NAME = args[argn++]; \
        auto && _ ## NAME = _arg_ ## NAME .value; \
        spv::Id NAME = SCOPES_GET_RESULT(argument_to_value(_ ## NAME));
#define READ_LABEL_BLOCK(NAME) \
        assert(argn <= argcount); \
        spv::Block *NAME = SCOPES_GET_RESULT(label_to_basic_block(args[argn++].value)); \
        assert(NAME);
#define READ_TYPE(NAME) \
        assert(argn <= argcount); \
        assert(args[argn].value.type == TYPE_Type); \
        spv::Id NAME = SCOPES_GET_RESULT(type_to_spirv_type(args[argn++].value.typeref));

        spv::Id retvalue = 0;
        ReturnTraits rtraits;
        if (enter.type == TYPE_Builtin) {
            switch(enter.builtin.value()) {
            case FN_Sample: {
                READ_VALUE(sampler);
                READ_VALUE(coords);
                spv::Builder::TextureParameters params;
                memset(&params, 0, sizeof(params));
                params.sampler = sampler;
                params.coords = coords;
                auto ST = SCOPES_GET_RESULT(storage_type(_sampler.indirect_type()));
                if (ST->kind() == TK_SampledImage) {
                    ST = SCOPES_GET_RESULT(storage_type(cast<SampledImageType>(ST)->type));
                }
                auto resultType = SCOPES_GET_RESULT(type_to_spirv_type(cast<ImageType>(ST)->type));
                bool sparse = false;
                bool fetch = false;
                bool proj = false;
                bool gather = false;
                bool explicitLod = false;
                while (argn <= argcount) {
                    READ_VALUE(value);
                    switch (_arg_value.key.value()) {
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
                retvalue = builder.createTextureCall(
                    spv::NoPrecision, resultType, sparse, fetch, proj, gather,
                    explicitLod, params);
            } break;
            case FN_ImageQuerySize: {
                READ_VALUE(sampler);
                spv::Builder::TextureParameters params;
                memset(&params, 0, sizeof(params));
                params.sampler = build_extract_image(sampler);
                spv::Op op = spv::OpImageQuerySize;
                while (argn <= argcount) {
                    READ_VALUE(value);
                    switch (_arg_value.key.value()) {
                        case SYM_SPIRV_ImageOperandLod:
                            op = spv::OpImageQuerySizeLod;
                            params.lod = value; break;
                        default: break;
                    }
                }
                retvalue = builder.createTextureQueryCall(op, params, false);
            } break;
            case FN_ImageQueryLod: {
                READ_VALUE(sampler);
                READ_VALUE(coords);
                spv::Builder::TextureParameters params;
                memset(&params, 0, sizeof(params));
                params.sampler = sampler;
                params.coords = coords;
                retvalue = builder.createTextureQueryCall(
                    spv::OpImageQueryLod, params, false);
            } break;
            case FN_ImageQueryLevels: {
                READ_VALUE(sampler);
                spv::Builder::TextureParameters params;
                memset(&params, 0, sizeof(params));
                params.sampler = build_extract_image(sampler);
                retvalue = builder.createTextureQueryCall(
                    spv::OpImageQueryLevels, params, false);
            } break;
            case FN_ImageQuerySamples: {
                READ_VALUE(sampler);
                spv::Builder::TextureParameters params;
                memset(&params, 0, sizeof(params));
                params.sampler = build_extract_image(sampler);
                retvalue = builder.createTextureQueryCall(
                    spv::OpImageQuerySamples, params, false);
            } break;
            case FN_ImageRead: {
                READ_VALUE(image);
                READ_VALUE(coords);
                auto ST = _image.indirect_type();
                auto resultType = SCOPES_GET_RESULT(type_to_spirv_type(cast<ImageType>(ST)->type));
                retvalue = builder.createBinOp(spv::OpImageRead,
                    resultType, image, coords);
            } break;
            case FN_ImageWrite: {
                READ_VALUE(image);
                READ_VALUE(coords);
                READ_VALUE(texel);
                builder.createNoResultOp(spv::OpImageWrite, { image, coords, texel });
            } break;
            case FN_Branch: {
                READ_VALUE(cond);
                READ_LABEL_BLOCK(then_block);
                READ_LABEL_BLOCK(else_block);
                builder.createConditionalBranch(cond, then_block, else_block);
                rtraits.terminated = true;
            } break;
            case OP_Tertiary: {
                READ_VALUE(cond);
                READ_VALUE(then_value);
                READ_VALUE(else_value);
                if (builder.isScalar(cond) && builder.isVector(then_value)) {
                    // emulate LLVM's behavior for selecting full vectors with single booleans
                    spv::Id elementT = builder.getTypeId(cond);
                    spv::Id vectorT = builder.getTypeId(then_value);

                    cond = builder.smearScalar(spv::NoPrecision, cond,
                        builder.makeVectorType(elementT, builder.getNumTypeComponents(vectorT)));
                }
                retvalue = builder.createTriOp(spv::OpSelect,
                    builder.getTypeId(then_value), cond,
                    then_value, else_value);
            } break;
            case OP_FMix: {
                READ_VALUE(a);
                READ_VALUE(b);
                READ_VALUE(x);
                retvalue = builder.createBuiltinCall(
                    builder.getTypeId(a),
                    glsl_ext_inst, GLSLstd450FMix, { a, b, x });
            } break;
            case FN_Length:
            case FN_Normalize:
            case OP_Sin:
            case OP_Cos:
            case OP_Tan:
            case OP_Asin:
            case OP_Acos:
            case OP_Atan:
            case OP_Trunc:
            case OP_Floor:
            case OP_FAbs:
            case OP_FSign:
            case OP_Log:
            case OP_Log2:
            case OP_Exp:
            case OP_Exp2:
            case OP_Sqrt: {
                READ_VALUE(val);
                GLSLstd450 builtin = GLSLstd450Bad;
                auto rtype = builder.getTypeId(val);
                switch (enter.symbol.value()) {
                case FN_Length:
                    rtype = builder.getContainedTypeId(rtype);
                    builtin = GLSLstd450Length; break;
                case FN_Normalize: builtin = GLSLstd450Normalize; break;
                case OP_Sin: builtin = GLSLstd450Sin; break;
                case OP_Cos: builtin = GLSLstd450Cos; break;
                case OP_Tan: builtin = GLSLstd450Tan; break;
                case OP_Asin: builtin = GLSLstd450Asin; break;
                case OP_Acos: builtin = GLSLstd450Acos; break;
                case OP_Atan: builtin = GLSLstd450Atan; break;
                case OP_Trunc: builtin = GLSLstd450Trunc; break;
                case OP_Floor: builtin = GLSLstd450Floor; break;
                case OP_FAbs: builtin = GLSLstd450FAbs; break;
                case OP_FSign: builtin = GLSLstd450FSign; break;
                case OP_Sqrt: builtin = GLSLstd450Sqrt; break;
                case OP_Log: builtin = GLSLstd450Log; break;
                case OP_Log2: builtin = GLSLstd450Log2; break;
                case OP_Exp: builtin = GLSLstd450Exp; break;
                case OP_Exp2: builtin = GLSLstd450Exp2; break;
                default: {
                    StyledString ss;
                    ss.out << "IL->SPIR: unsupported unary intrinsic " << enter << " encountered";
                    SCOPES_LOCATION_ERROR(ss.str());
                } break;
                }
                retvalue = builder.createBuiltinCall(rtype, glsl_ext_inst, builtin, { val });
            } break;
            case FN_Cross:
            case OP_Step:
            case OP_Pow: {
                READ_VALUE(a);
                READ_VALUE(b);
                GLSLstd450 builtin = GLSLstd450Bad;
                auto rtype = builder.getTypeId(a);
                switch (enter.symbol.value()) {
                case OP_Step: builtin = GLSLstd450Step; break;
                case OP_Pow: builtin = GLSLstd450Pow; break;
                case FN_Cross: builtin = GLSLstd450Cross; break;
                default: {
                    StyledString ss;
                    ss.out << "IL->SPIR: unsupported binary intrinsic " << enter << " encountered";
                    SCOPES_LOCATION_ERROR(ss.str());
                } break;
                }
                retvalue = builder.createBuiltinCall(rtype, glsl_ext_inst, builtin, { a, b });
            } break;
            case FN_Unconst: {
                READ_VALUE(val);
                retvalue = val;
            } break;
            case FN_ExtractValue: {
                READ_VALUE(val);
                READ_ANY(index);
                int i = SCOPES_GET_RESULT(cast_number<unsigned>(index));
                retvalue = builder.createCompositeExtract(val,
                    builder.getContainedTypeId(builder.getTypeId(val), i),
                    i);
            } break;
            case FN_InsertValue: {
                READ_VALUE(val);
                READ_VALUE(eltval);
                READ_ANY(index);
                retvalue = builder.createCompositeInsert(eltval, val,
                    builder.getTypeId(val),
                    SCOPES_GET_RESULT(cast_number<unsigned>(index)));
            } break;
            case FN_ExtractElement: {
                READ_VALUE(val);
                READ_VALUE(index);
                if (_index.is_const()) {
                    int i = SCOPES_GET_RESULT(cast_number<unsigned>(_index));
                    retvalue = builder.createCompositeExtract(val,
                        builder.getContainedTypeId(builder.getTypeId(val), i),
                        i);
                } else {
                    retvalue = builder.createVectorExtractDynamic(val,
                        builder.getContainedTypeId(builder.getTypeId(val)),
                        index);
                }
            } break;
            case FN_InsertElement: {
                READ_VALUE(val);
                READ_VALUE(eltval);
                READ_VALUE(index);
                if (_index.is_const()) {
                    retvalue = builder.createCompositeInsert(eltval, val,
                        builder.getTypeId(val),
                        SCOPES_GET_RESULT(cast_number<unsigned>(_index)));
                } else {
                    retvalue = builder.createVectorInsertDynamic(val,
                        builder.getTypeId(val), eltval, index);
                }
            } break;
            case FN_ShuffleVector: {
                READ_VALUE(v1);
                READ_VALUE(v2);
                READ_VALUE(mask);
                auto ET = builder.getContainedTypeId(builder.getTypeId(v1));
                auto sz = builder.getNumTypeComponents(builder.getTypeId(mask));
                auto op = new spv::Instruction(
                    builder.getUniqueId(),
                    builder.makeVectorType(ET, sz),
                    spv::OpVectorShuffle);
                op->addIdOperand(v1);
                op->addIdOperand(v2);
                auto vt = cast<VectorType>(SCOPES_GET_RESULT(storage_type(_mask.type)));
                for (int i = 0; i < sz; ++i) {
                    op->addImmediateOperand(
                        SCOPES_GET_RESULT(cast_number<unsigned int>(SCOPES_GET_RESULT(vt->unpack(_mask.pointer, i)))));
                }
                retvalue = op->getResultId();
                builder.getBuildPoint()->addInstruction(
                    std::unique_ptr<spv::Instruction>(op));
            } break;
            case FN_Undef: { READ_TYPE(ty);
                retvalue = builder.createUndefined(ty); } break;
            case FN_Alloca: { READ_TYPE(ty);
                retvalue = builder.createVariable(
                    spv::StorageClassFunction, ty); } break;
            /*
            case FN_AllocaArray: { READ_TYPE(ty); READ_VALUE(val);
                retvalue = LLVMBuildArrayAlloca(builder, ty, val, ""); } break;
            */
            case FN_AllocaOf: {
                READ_VALUE(val);
                retvalue = builder.createVariable(spv::StorageClassFunction,
                    builder.getTypeId(val));
                builder.createStore(val, retvalue);
            } break;
            /*
            case FN_Malloc: { READ_TYPE(ty);
                retvalue = LLVMBuildMalloc(builder, ty, ""); } break;
            case FN_MallocArray: { READ_TYPE(ty); READ_VALUE(val);
                retvalue = LLVMBuildArrayMalloc(builder, ty, val, ""); } break;
            case FN_Free: { READ_VALUE(val);
                retvalue = LLVMBuildFree(builder, val); } break;
            */
            case SFXFN_ExecutionMode: {
                assert(active_function_value);
                READ_ANY(mode);
                auto em = SCOPES_GET_RESULT(execution_mode_from_symbol(mode.symbol));
                int values[3] = { -1, -1, -1 };
                int c = 0;
                while ((c < 3) && (argn <= argcount)) {
                    READ_ANY(val);
                    values[c] = SCOPES_GET_RESULT(cast_number<int>(val));
                    c++;
                }
                builder.addExecutionMode(active_function_value, em,
                    values[0], values[1], values[2]);
            } break;
            case FN_GetElementPtr: {
                READ_VALUE(pointer);
                assert(argcount > 1);
                size_t count = argcount - 1;
                std::vector<spv::Id> indices;
                for (size_t i = 1; i < count; ++i) {
                    indices.push_back(SCOPES_GET_RESULT(argument_to_value(args[argn + i].value)));
                }

                retvalue = builder.createAccessChain(
                    builder.getTypeStorageClass(builder.getTypeId(pointer)),
                    pointer, indices);
            } break;
            case FN_Bitcast:
            case FN_IntToPtr:
            case FN_PtrToInt:
            case FN_ITrunc:
            case FN_SExt:
            case FN_ZExt:
            case FN_FPTrunc:
            case FN_FPExt:
            case FN_FPToUI:
            case FN_FPToSI:
            case FN_UIToFP:
            case FN_SIToFP:
            {
                READ_VALUE(val); READ_TYPE(ty);
                spv::Op op = spv::OpMax;
                switch(enter.builtin.value()) {
                case FN_Bitcast:
                    if (builder.getTypeId(val) == ty) {
                        // do nothing
                        retvalue = val;
                    } else {
                        op = spv::OpBitcast;
                    }
                    break;
                case FN_IntToPtr: op = spv::OpConvertUToPtr; break;
                case FN_PtrToInt: op = spv::OpConvertPtrToU; break;
                case FN_SExt: op = spv::OpSConvert; break;
                case FN_ZExt: op = spv::OpUConvert; break;
                case FN_ITrunc: op = spv::OpSConvert; break;
                case FN_FPTrunc: op = spv::OpFConvert; break;
                case FN_FPExt: op = spv::OpFConvert; break;
                case FN_FPToUI: op = spv::OpConvertFToU; break;
                case FN_FPToSI: op = spv::OpConvertFToS; break;
                case FN_UIToFP: op = spv::OpConvertUToF; break;
                case FN_SIToFP: op = spv::OpConvertSToF; break;
                default: break;
                }
                if (op != spv::OpMax) {
                    retvalue = builder.createUnaryOp(op, ty, val);
                }
            } break;
            case FN_VolatileLoad:
            case FN_Load: {
                READ_VALUE(ptr);
                retvalue = builder.createLoad(ptr);
                if (enter.builtin == FN_VolatileLoad) {
                    builder.getInstruction(retvalue)->addImmediateOperand(
                        1<<spv::MemoryAccessVolatileShift);
                }
            } break;
            case FN_VolatileStore:
            case FN_Store: {
                READ_VALUE(val); READ_VALUE(ptr);
                builder.createStore(val, ptr);
                if (enter.builtin == FN_VolatileStore) {
                    builder.getInstruction(retvalue)->addImmediateOperand(
                        1<<spv::MemoryAccessVolatileShift);
                }
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
            case OP_ICmpSLE:
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
            case OP_FCmpULE: { READ_VALUE(a); READ_VALUE(b);
                spv::Op op = spv::OpMax;
#define BOOL_OR_INT_OP(BOOL_OP, INT_OP) \
    (is_bool(a)?(BOOL_OP):(INT_OP))
                switch(enter.builtin.value()) {
                case OP_ICmpEQ: op = BOOL_OR_INT_OP(spv::OpLogicalEqual, spv::OpIEqual); break;
                case OP_ICmpNE: op = BOOL_OR_INT_OP(spv::OpLogicalNotEqual, spv::OpINotEqual); break;
                case OP_ICmpUGT: op = spv::OpUGreaterThan; break;
                case OP_ICmpUGE: op = spv::OpUGreaterThanEqual; break;
                case OP_ICmpULT: op = spv::OpULessThan; break;
                case OP_ICmpULE: op = spv::OpULessThanEqual; break;
                case OP_ICmpSGT: op = spv::OpSGreaterThan; break;
                case OP_ICmpSGE: op = spv::OpSGreaterThanEqual; break;
                case OP_ICmpSLT: op = spv::OpSLessThan; break;
                case OP_ICmpSLE: op = spv::OpSLessThanEqual; break;
                case OP_FCmpOEQ: op = spv::OpFOrdEqual; break;
                case OP_FCmpONE: op = spv::OpFOrdNotEqual; break;
                case OP_FCmpORD: op = spv::OpOrdered; break;
                case OP_FCmpOGT: op = spv::OpFOrdGreaterThan; break;
                case OP_FCmpOGE: op = spv::OpFOrdGreaterThanEqual; break;
                case OP_FCmpOLT: op = spv::OpFOrdLessThan; break;
                case OP_FCmpOLE: op = spv::OpFOrdLessThanEqual; break;
                case OP_FCmpUEQ: op = spv::OpFUnordEqual; break;
                case OP_FCmpUNE: op = spv::OpFUnordNotEqual; break;
                case OP_FCmpUNO: op = spv::OpUnordered; break;
                case OP_FCmpUGT: op = spv::OpFUnordGreaterThan; break;
                case OP_FCmpUGE: op = spv::OpFUnordGreaterThanEqual; break;
                case OP_FCmpULT: op = spv::OpFUnordLessThan; break;
                case OP_FCmpULE: op = spv::OpFUnordLessThanEqual; break;
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
                retvalue = builder.createBinOp(op, T, a, b); } break;
            case OP_Add:
            case OP_AddNUW:
            case OP_AddNSW:
            case OP_Sub:
            case OP_SubNUW:
            case OP_SubNSW:
            case OP_Mul:
            case OP_MulNUW:
            case OP_MulNSW:
            case OP_SDiv:
            case OP_UDiv:
            case OP_SRem:
            case OP_URem:
            case OP_Shl:
            case OP_LShr:
            case OP_AShr:
            case OP_BAnd:
            case OP_BOr:
            case OP_BXor:
            case OP_FAdd:
            case OP_FSub:
            case OP_FMul:
            case OP_FDiv:
            case OP_FRem: { READ_VALUE(a); READ_VALUE(b);
                spv::Op op = spv::OpMax;
                switch(enter.builtin.value()) {
#define BOOL_OR_INT_OP(BOOL_OP, INT_OP) \
    (is_bool(a)?(BOOL_OP):(INT_OP))
                case OP_Add:
                case OP_AddNUW:
                case OP_AddNSW: op = spv::OpIAdd; break;
                case OP_Sub:
                case OP_SubNUW:
                case OP_SubNSW: op = spv::OpISub; break;
                case OP_Mul:
                case OP_MulNUW:
                case OP_MulNSW: op = spv::OpIMul; break;
                case OP_SDiv: op = spv::OpSDiv; break;
                case OP_UDiv: op = spv::OpUDiv; break;
                case OP_SRem: op = spv::OpSRem; break;
                case OP_URem: op = spv::OpUMod; break;
                case OP_Shl: op = spv::OpShiftLeftLogical; break;
                case OP_LShr: op = spv::OpShiftRightLogical; break;
                case OP_AShr: op = spv::OpShiftRightArithmetic; break;
                case OP_BAnd: op = BOOL_OR_INT_OP(spv::OpLogicalAnd, spv::OpBitwiseAnd); break;
                case OP_BOr: op = BOOL_OR_INT_OP(spv::OpLogicalOr, spv::OpBitwiseOr); break;
                case OP_BXor: op = BOOL_OR_INT_OP(spv::OpLogicalNotEqual, spv::OpBitwiseXor); break;
                case OP_FAdd: op = spv::OpFAdd; break;
                case OP_FSub: op = spv::OpFSub; break;
                case OP_FMul: op = spv::OpFMul; break;
                case OP_FDiv: op = spv::OpFDiv; break;
                case OP_FRem: op = spv::OpFRem; break;
                default: break;
                }
#undef BOOL_OR_INT_OP
                retvalue = builder.createBinOp(op,
                    builder.getTypeId(a), a, b); } break;
            case SFXFN_Unreachable:
                builder.makeUnreachable();
                rtraits.terminated = true; break;
            case SFXFN_Discard:
                builder.makeDiscard();
                rtraits.terminated = true; break;
            default: {
                StyledString ss;
                ss.out << "IL->SPIR: unsupported builtin " << enter.builtin << " encountered";
                SCOPES_LOCATION_ERROR(ss.str());
            } break;
            }
        } else if (enter.type == TYPE_Label) {
            if (enter.label->is_basic_block_like()) {
                auto block = SCOPES_GET_RESULT(label_to_basic_block(enter.label));
                if (!block) {
                    // no basic block was generated - just generate assignments
                    auto &&params = enter.label->params;
                    for (size_t i = 1; i < params.size(); ++i) {
                        bind_parameter(params[i], SCOPES_GET_RESULT(argument_to_value(args[i].value)));
                    }
                    label = enter.label;
                    goto repeat;
                } else {
                    auto bbfrom = builder.getBuildPoint();
                    // assign phi nodes
                    auto &&params = enter.label->params;
                    bool single_caller = has_single_caller(enter.label);
                    for (size_t i = 1; i < params.size(); ++i) {
                        Parameter *param = params[i];
                        auto value = SCOPES_GET_RESULT(argument_to_value(args[i].value));
                        if (single_caller) {
                            assert(!parameter_is_bound(param));
                            bind_parameter(param, value);
                        } else {
                            auto phinode = SCOPES_GET_RESULT(argument_to_value(param));
                            auto op = builder.getInstruction(phinode);
                            assert(op);
                            op->addIdOperand(value);
                            op->addIdOperand(bbfrom->getId());
                        }
                    }
                    builder.createBranch(block);
                    rtraits.terminated = true;
                }
            } else {
                /*if (use_debug_info) {
                    LLVMSetCurrentDebugLocation(builder, diloc);
                }*/
                auto func = SCOPES_GET_RESULT(label_to_function(enter.label));
                retvalue = SCOPES_GET_RESULT(build_call(
                    enter.label->get_function_type(),
                    func, args, rtraits));
            }
        } else if (enter.type == TYPE_Closure) {
            StyledString ss;
            ss.out << "IL->SPIR: invalid call of compile time closure at runtime";
            SCOPES_LOCATION_ERROR(ss.str());
        } else if (enter.type == TYPE_Parameter) {
            assert (enter.parameter->type != TYPE_Nothing);
            assert(enter.parameter->type != TYPE_Unknown);
            std::vector<spv::Id> values;
            for (size_t i = 0; i < argcount; ++i) {
                values.push_back(SCOPES_GET_RESULT(argument_to_value(args[i + 1].value)));
            }
            // must be a return
            assert(enter.parameter->index == 0);
            // must be returning from this function
            assert(enter.parameter->label == active_function);

            //Label *label = enter.parameter->label;
            if (argcount > 1) {
                auto ilfunctype = cast<FunctionType>(active_function->get_function_type());
                auto rettype = SCOPES_GET_RESULT(type_to_spirv_type(ilfunctype->return_type));
                auto id = builder.createUndefined(rettype);
                for (size_t i = 0; i < values.size(); ++i) {
                    id = builder.createCompositeInsert(
                        values[i], id, rettype, i);
                }
                builder.makeReturn(true, id);
            } else if (argcount == 1) {
                builder.makeReturn(true, values[0]);
            } else {
                builder.makeReturn(true, 0);
            }
            rtraits.terminated = true;
        } else {
            StyledString ss;
            ss.out << "IL->SPIR: cannot translate call to " << enter;
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
            //Label *label = contarg.parameter->label;
            if (retvalue) {
                builder.makeReturn(true, retvalue);
            } else {
                builder.makeReturn(true, 0);
            }
        } else if (contarg.type == TYPE_Label) {
            auto bb = SCOPES_GET_RESULT(label_to_basic_block(contarg.label));
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
                spv::Id incoval = builder.createCompositeExtract( \
                    retvalue, \
                    builder.getContainedTypeId(rtype, i), i); \
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
            if (bb) {
                if (retvalue) {
                    auto bbfrom = builder.getBuildPoint();
                    // assign phi nodes
                    auto rtype = builder.getTypeId(retvalue);

                    bool single_caller = has_single_caller(contarg.label);
                    #define T(PARAM, VALUE) \
                        if (single_caller) { \
                            assert(!parameter_is_bound(PARAM)); \
                            bind_parameter(PARAM, VALUE); \
                        } else { \
                            auto phinode = SCOPES_GET_RESULT(argument_to_value(PARAM)); \
                            auto op = builder.getInstruction(phinode); \
                            assert(op); \
                            op->addIdOperand(VALUE); \
                            op->addIdOperand(bbfrom->getId()); \
                        }
                    UNPACK_RET_ARGS()
                    #undef T
                }
                builder.createBranch(bb);
            } else {
                if (retvalue) {
                    // no basic block - just add assignments and continue
                    auto rtype = builder.getTypeId(retvalue);
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
            SCOPES_LOCATION_ERROR(String::from("IL->SPIR: unexpected end of function"));
        } else {
            StyledStream ss(SCOPES_CERR);
            stream_label(ss, label, StreamLabelFormat::debug_single());
            SCOPES_LOCATION_ERROR(String::from("IL->SPIR: continuation is of invalid type"));
        }

        //LLVMSetCurrentDebugLocation(builder, nullptr);
        return true;
    }
    #undef READ_ANY
    #undef READ_VALUE
    #undef READ_TYPE
    #undef READ_LABEL_BLOCK

    struct ReturnTraits {
        bool multiple_return_values;
        bool terminated;
        const ReturnLabelType *rtype;

        ReturnTraits() :
            multiple_return_values(false),
            terminated(false),
            rtype(nullptr) {}
    };

    SCOPES_RESULT(spv::Id) build_call(const Type *functype, spv::Function* func, Args &args,
        ReturnTraits &traits) {
        SCOPES_RESULT_TYPE(spv::Id);
        size_t argcount = args.size() - 1;

        auto fi = cast<FunctionType>(functype);

        std::vector<spv::Id> values;
        for (size_t i = 0; i < argcount; ++i) {
            auto &&arg = args[i + 1];
            values.push_back(SCOPES_GET_RESULT(argument_to_value(arg.value)));
        }

        size_t fargcount = fi->argument_types.size();
        assert(argcount >= fargcount);
        if (fi->flags & FF_Variadic) {
            SCOPES_LOCATION_ERROR(String::from("IL->SPIR: variadic calls not supported"));
        }

        auto ret = builder.createFunctionCall(func, values);
        auto rlt = cast<ReturnLabelType>(fi->return_type);
        traits.rtype = rlt;
        traits.multiple_return_values = rlt->has_multiple_return_values();
        if (rlt->return_type == TYPE_Void) {
            return 0;
        } else if (!rlt->is_returning()) {
            builder.makeUnreachable();
            traits.terminated = true;
            return 0;
        } else {
            return ret;
        }
    }

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
            spv::Block *bb = it2->second;
            builder.setBuildPoint(bb);

            SCOPES_CHECK_RESULT(write_label_body(label));
        }
        return true;
    }

    Label *get_single_caller(Label *l) {
        auto it = user_map.label_map.find(l);
        assert(it != user_map.label_map.end());
        auto &&users = it->second;
        if (users.size() != 1)
            return nullptr;
        Label *userl = *users.begin();
        if (userl->body.enter == Any(l))
            return userl;
        if (userl->body.args[0] == Any(l))
            return userl;
        return nullptr;
    }

    bool has_single_caller(Label *l) {
        Label *userl = get_single_caller(l);
        return (userl != nullptr);
    }

    SCOPES_RESULT(spv::Id) create_struct_type(const Type *type, uint64_t flags,
        const TypenameType *tname = nullptr) {
        SCOPES_RESULT_TYPE(spv::Id);
        // todo: packed tuples
        auto ti = cast<TupleType>(type);
        size_t count = ti->types.size();
        std::vector<spv::Id> members;
        for (size_t i = 0; i < count; ++i) {
            members.push_back(SCOPES_GET_RESULT(type_to_spirv_type(ti->types[i])));
        }
        const char *name = "tuple";
        if (tname) {
            name = tname->name()->data;
        }
        auto id = builder.makeStructType(members, name);
        if (flags & EF_BufferBlock) {
            builder.addDecoration(id, spv::DecorationBufferBlock);
        } else if (flags & EF_Block) {
            builder.addDecoration(id, spv::DecorationBlock);
        }
        for (size_t i = 0; i < count; ++i) {
            builder.addMemberName(id, i, ti->values[i].key.name()->data);
            if (flags & EF_Volatile) {
                builder.addMemberDecoration(id, i, spv::DecorationVolatile);
            }
            if (flags & EF_Coherent) {
                builder.addMemberDecoration(id, i, spv::DecorationCoherent);
            }
            if (flags & EF_Restrict) {
                builder.addMemberDecoration(id, i, spv::DecorationRestrict);
            }
            if (flags & EF_NonWritable) {
                builder.addMemberDecoration(id, i, spv::DecorationNonWritable);
            }
            if (flags & EF_NonReadable) {
                builder.addMemberDecoration(id, i, spv::DecorationNonReadable);
            }
            builder.addMemberDecoration(id, i, spv::DecorationOffset, ti->offsets[i]);
        }
        return id;
    }

    SCOPES_RESULT(spv::Id) create_spirv_type(const Type *type, uint64_t flags) {
        SCOPES_RESULT_TYPE(spv::Id);
        switch(type->kind()) {
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
            if (!ai->count) {
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
            return builder.makeVectorType(
                SCOPES_GET_RESULT(type_to_spirv_type(vi->element_type)),
                vi->count);
        } break;
        case TK_Tuple: {
            return create_struct_type(type, flags);
        } break;
        case TK_Union: {
            auto ui = cast<UnionType>(type);
            return type_to_spirv_type(ui->tuple_type);
        } break;
        case TK_Extern: {
            auto et = cast<ExternType>(type);
            spv::StorageClass sc = SCOPES_GET_RESULT(storage_class_from_extern_class(
                et->storage_class));
            auto ty = SCOPES_GET_RESULT(type_to_spirv_type(et->type, et->flags));
            return builder.makePointer(sc, ty);
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
        case TK_SampledImage: {
            auto sit = cast<SampledImageType>(type);
            return builder.makeSampledImageType(SCOPES_GET_RESULT(type_to_spirv_type(sit->type)));
        } break;
        case TK_Typename: {
            if (type == TYPE_Void)
                return builder.makeVoidType();
            else if (type == TYPE_Sampler)
                return builder.makeSamplerType();
            auto tn = cast<TypenameType>(type);
            if (tn->finalized()) {
                if (tn->storage_type->kind() == TK_Tuple) {
                    return create_struct_type(tn->storage_type, flags, tn);
                } else {
                    return SCOPES_GET_RESULT(type_to_spirv_type(tn->storage_type, flags));
                }
            } else {
                SCOPES_LOCATION_ERROR(String::from("IL->SPIR: opaque types are not supported"));
            }
        } break;
        case TK_ReturnLabel: {
            auto rlt = cast<ReturnLabelType>(type);
            return SCOPES_GET_RESULT(type_to_spirv_type(rlt->return_type));
        } break;
        case TK_Function: {
            auto fi = cast<FunctionType>(type);
            if (fi->vararg()) {
                SCOPES_LOCATION_ERROR(String::from("IL->SPIR: vararg functions are not supported"));
            }
            size_t count = fi->argument_types.size();
            spv::Id rettype = SCOPES_GET_RESULT(type_to_spirv_type(fi->return_type));
            std::vector<spv::Id> elements;
            for (size_t i = 0; i < count; ++i) {
                auto AT = fi->argument_types[i];
                elements.push_back(SCOPES_GET_RESULT(type_to_spirv_type(AT)));
            }
            return builder.makeFunctionType(rettype, elements);
        } break;
        };

        StyledString ss;
        ss.out << "IL->SPIR: cannot convert type " << type;
        SCOPES_LOCATION_ERROR(ss.str());
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

    SCOPES_RESULT(spv::Block *) label_to_basic_block(Label *label, bool force = false) {
        SCOPES_RESULT_TYPE(spv::Block *);
        auto old_bb = builder.getBuildPoint();
        auto func = &old_bb->getParent();
        auto it = label2bb.find({func, label});
        if (it == label2bb.end()) {
            bool single_caller = has_single_caller(label);
            if (single_caller && !force) {
                // not generating basic blocks for single user labels
                return nullptr;
            }
            //const char *name = label->name.name()->data;
            auto bb = &builder.makeNewBlock();
            label2bb.insert({{func, label}, bb});
            bb_label_todo.push_back({active_function, label});
            builder.setBuildPoint(bb);

            auto &&params = label->params;
            if (!params.empty()) {
                size_t paramcount = label->params.size() - 1;
                for (size_t i = 0; i < paramcount; ++i) {
                    Parameter *param = params[i + 1];
                    auto ptype = SCOPES_GET_RESULT(type_to_spirv_type(param->type));
                    if (!single_caller) {
                        auto op = new spv::Instruction(
                            builder.getUniqueId(), ptype, spv::OpPhi);
                        builder.addName(op->getResultId(), param->name.name()->data);
                        bind_parameter(param, op->getResultId());
                        bb->addInstruction(std::unique_ptr<spv::Instruction>(op));
                    }
                }
            }

            builder.setBuildPoint(old_bb);
            return bb;
        } else {
            return it->second;
        }
    }

    SCOPES_RESULT(spv::Function *) label_to_function(Label *label,
        bool root_function = false,
        Symbol funcname = SYM_Unnamed) {
        SCOPES_RESULT_TYPE(spv::Function *);
        auto it = label2func.find(label);
        if (it == label2func.end()) {

            const Anchor *old_anchor = get_active_anchor();
            set_active_anchor(label->anchor);
            Label *last_function = active_function;

            auto old_bb = builder.getBuildPoint();

            if (funcname == SYM_Unnamed) {
                funcname = label->name;
            }

            const char *name;
            if (root_function && (funcname == SYM_Unnamed)) {
                name = "unnamed";
            } else {
                name = funcname.name()->data;
            }

            SCOPES_CHECK_RESULT(label->verify_compilable());
            auto ilfunctype = cast<FunctionType>(label->get_function_type());
            //auto fi = cast<FunctionType>(ilfunctype);

            auto rettype = SCOPES_GET_RESULT(type_to_spirv_type(ilfunctype->return_type));

            spv::Block* bb;
            std::vector<spv::Id> paramtypes;

            auto &&argtypes = ilfunctype->argument_types;
            for (auto it = argtypes.begin(); it != argtypes.end(); ++it) {
                paramtypes.push_back(SCOPES_GET_RESULT(type_to_spirv_type(*it)));
            }

            std::vector<std::vector<spv::Decoration>> decorations;

            auto func = builder.makeFunctionEntry(
                spv::NoPrecision, rettype, name,
                paramtypes, decorations, &bb);
            //LLVMSetLinkage(func, LLVMPrivateLinkage);

            label2func[label] = func;
            set_active_function(label);

            if (use_debug_info) {
                // LLVMSetFunctionSubprogram(func, label_to_subprogram(label));
            }

            builder.setBuildPoint(bb);
            SCOPES_CHECK_RESULT(write_anchor(label->anchor));

            auto &&params = label->params;
            size_t paramcount = params.size() - 1;
            for (size_t i = 0; i < paramcount; ++i) {
                Parameter *param = params[i + 1];
                auto val = func->getParamId(i);
                bind_parameter(param, val);
            }

            SCOPES_CHECK_RESULT(write_label_body(label));

            builder.setBuildPoint(old_bb);

            set_active_function(last_function);
            set_active_anchor(old_anchor);
            return func;
        } else {
            return it->second;
        }
    }

    SCOPES_RESULT(void) generate(std::vector<unsigned int> &result, Symbol target, Label *entry) {
        SCOPES_RESULT_TYPE(void);
        //assert(all_parameters_lowered(entry));
        assert(!entry->is_basic_block_like());

        builder.setSource(spv::SourceLanguageGLSL, 450);
        glsl_ext_inst = builder.import("GLSL.std.450");

        auto needfi = Function(TYPE_Void, {}, 0);
        auto hasfi = entry->get_function_type();
        if (hasfi != needfi) {
            set_active_anchor(entry->anchor);
            StyledString ss;
            ss.out << "Entry function must have type " << needfi
                << " but has type " << hasfi;
            SCOPES_LOCATION_ERROR(ss.str());
        }

        {
            std::unordered_set<Label *> visited;
            //Labels labels;
            entry->build_reachable(visited, nullptr);
            for (auto it = visited.begin(); it != visited.end(); ++it) {
                (*it)->insert_into_usermap(user_map);
            }
        }

        scc.walk(entry);

        //const char *name = entry->name.name()->data;
        //module = LLVMModuleCreateWithName(name);

        if (use_debug_info) {
            /*
            const char *DebugStr = "Debug Info Version";
            LLVMValueRef DbgVer[3];
            DbgVer[0] = LLVMConstInt(i32T, 1, 0);
            DbgVer[1] = LLVMMDString(DebugStr, strlen(DebugStr));
            DbgVer[2] = LLVMConstInt(i32T, 3, 0);
            LLVMAddNamedMetadataOperand(module, "llvm.module.flags",
                LLVMMDNode(DbgVer, 3));

            LLVMDIBuilderCreateCompileUnit(di_builder,
                llvm::dwarf::DW_LANG_C99, "file", "directory", "scopes",
                false, "", 0, "", 0);*/
        }

        auto func = SCOPES_GET_RESULT(label_to_function(entry, true));

        switch(target.value()) {
        case SYM_TargetVertex: {
            builder.addCapability(spv::CapabilityShader);
            builder.addEntryPoint(spv::ExecutionModelVertex, func, "main");
        } break;
        case SYM_TargetFragment: {
            builder.addCapability(spv::CapabilityShader);
            builder.addEntryPoint(spv::ExecutionModelFragment, func, "main");
        } break;
        case SYM_TargetGeometry: {
            builder.addCapability(spv::CapabilityShader);
            builder.addEntryPoint(spv::ExecutionModelGeometry, func, "main");
        } break;
        case SYM_TargetCompute: {
            builder.addCapability(spv::CapabilityShader);
            builder.addEntryPoint(spv::ExecutionModelGLCompute, func, "main");
        } break;
        default: {
            StyledString ss;
            ss.out << "IL->SPIR: unsupported target: " << target << ", try one of "
                << Symbol(SYM_TargetVertex) << " "
                << Symbol(SYM_TargetFragment) << " "
                << Symbol(SYM_TargetGeometry) << " "
                << Symbol(SYM_TargetCompute);
            SCOPES_LOCATION_ERROR(ss.str());
        } break;
        }

        SCOPES_CHECK_RESULT(process_labels());

        //size_t k = finalize_types();
        //assert(!k);

        builder.dump(result);

        SCOPES_CHECK_RESULT(verify_spirv(result));
        return true;
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
        SCOPES_LOCATION_ERROR(String::from(
            "IL->SPIR: error while running optimization passes"));
    }

    SCOPES_CHECK_RESULT(verify_spirv(result));
    return true;
}

SCOPES_RESULT(const String *) compile_spirv(Symbol target, Label *fn, uint64_t flags) {
    SCOPES_RESULT_TYPE(const String *);
    Timer sum_compile_time(TIMER_CompileSPIRV);

    SCOPES_CHECK_RESULT(fn->verify_compilable());

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

SCOPES_RESULT(const String *) compile_glsl(Symbol target, Label *fn, uint64_t flags) {
    SCOPES_RESULT_TYPE(const String *);
    Timer sum_compile_time(TIMER_CompileSPIRV);

    SCOPES_CHECK_RESULT(fn->verify_compilable());

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
    /*
    spirv_cross::CompilerGLSL::Options options;
    options.version = 450;
    glsl.set_options(options);*/

    // Compile to GLSL, ready to give to GL driver.
    std::string source = glsl.compile();

    if (flags & (CF_DumpModule|CF_DumpFunction)) {
        std::cout << source << std::endl;
    }

    return String::from_stdstring(source);
}

} // namespace scopes
