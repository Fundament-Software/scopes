/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "quote.hpp"
#include "value.hpp"
#include "types.hpp"
#include "error.hpp"
#include "prover.hpp"
//#include "closure.hpp"
#include "stream_expr.hpp"
//#include "hash.hpp"
//#include "timer.hpp"
//#include "gc.hpp"
//#include "builtin.hpp"
//#include "verify_tools.inc"
#include "dyn_cast.inc"
//#include "compiler_flags.hpp"
//#include "gen_llvm.hpp"
//#include "list.hpp"
//#include "expander.hpp"
#include "globals.hpp"
#include "anchor.hpp"
#include "scopes/scopes.h"

#include "qualifier/refer_qualifier.hpp"
#include "qualifier.inc"
#include "absl/container/flat_hash_map.h"

//#pragma GCC diagnostic ignored "-Wvla-extension"

namespace scopes {

#define REF(X) ref(_anchor, (X))

#define LOCALREF(X) \
    ref(Anchor::from(Symbol(String::from(__FILE__)), __LINE__, 1), (X))

static ValueRef canonicalize(const ExpressionRef &expr) {
    if (expr->body.empty())
        return expr->value;
    return expr;
}

ValueRef build_quoted_argument_list(const Anchor *_anchor, const Values &values) {
    auto result = REF(Expression::unscoped_from());
    auto numvals = (int)values.size();
    auto numelems = REF(ConstInt::from(TYPE_I32, numvals));
    auto buf = REF(CallTemplate::from(g_alloca_array, {
            REF(ConstPointer::type_from(TYPE_ValueRef)),
            numelems
        }));
    result->append(buf);
    for (int i = 0; i < numvals; ++i) {
        auto idx = REF(ConstInt::from(TYPE_I32, i));
        result->append(
            REF(CallTemplate::from(g_store, {
                REF(CallTemplate::from(g_sc_identity, { values[i] })),
                REF(CallTemplate::from(g_getelementptr, { buf, idx }))
            })));
    }
    result->append(REF(CallTemplate::from(g_sc_argument_list_new,
        { numelems, buf })));
    return canonicalize(result);
}

//------------------------------------------------------------------------------

struct Quoter {
    Quoter(const ASTContext  &_ctx) :
        ctx(_ctx) {}

    SCOPES_RESULT(ValueRef) quote_Expression(int level, const ExpressionRef &node) {
        SCOPES_RESULT_TYPE(ValueRef);
        auto _anchor = node.anchor();
        auto value = REF(CallTemplate::from(g_sc_expression_new, {}));
        auto expr = REF(Expression::unscoped_from());
        if (node->scoped) {
            expr->append(REF(CallTemplate::from(g_sc_expression_set_scoped, { value })));
        }
        for (auto &&instr : node->body) {
            expr->append(REF(CallTemplate::from(g_sc_expression_append,
                { value, SCOPES_GET_RESULT(quote(level, instr)) })));
        }
        expr->append(REF(CallTemplate::from(g_sc_expression_append,
            { value, SCOPES_GET_RESULT(quote(level, node->value)) })));
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(ValueRef) quote_ArgumentListTemplate(int level, const ArgumentListTemplateRef &node) {
        SCOPES_RESULT_TYPE(ValueRef);
        /*if (node->values.size() == 1) {
            return quote(level, node->values[0]);
        } else*/ {
            Values newvalues;
            auto &&values = node->values();
            int count = (int)values.size();
            newvalues.reserve(count);
            for (int i = 0; i < count; ++i) {
                newvalues.push_back(SCOPES_GET_RESULT(quote(level, values[i])));
            }
            auto _anchor = node.anchor();
            return build_quoted_argument_list(_anchor, newvalues);
        }
    }

    SCOPES_RESULT(CallTemplateRef) quote_ExtractArgumentTemplate(int level, const ExtractArgumentTemplateRef &node) {
        SCOPES_RESULT_TYPE(CallTemplateRef);
        auto _anchor = node.anchor();
        if (node->vararg) {
            return REF(CallTemplate::from(g_sc_extract_argument_list_new, {
                    SCOPES_GET_RESULT(quote(level, node->value)),
                    REF(ConstInt::from(TYPE_I32, node->index)) }));
        } else {
            return REF(CallTemplate::from(g_sc_extract_argument_new, {
                    SCOPES_GET_RESULT(quote(level, node->value)),
                    REF(ConstInt::from(TYPE_I32, node->index)) }));
        }
    }

    SCOPES_RESULT(TypedValueRef) quote_param(const ParameterTemplateRef &node) {
        SCOPES_RESULT_TYPE(TypedValueRef);
        auto _anchor = node.anchor();
        auto newparam = REF(CallTemplate::from(g_sc_parameter_new, {
            REF(ConstInt::symbol_from(node->name)) }));
        auto typednewparam = SCOPES_GET_RESULT(prove(ctx, newparam));
        bind(node, typednewparam);
        ctx.frame->bind(node, typednewparam);
        return typednewparam;
    }

    SCOPES_RESULT(ValueRef) quote_Loop(int level, const LoopRef &node) {
        SCOPES_RESULT_TYPE(ValueRef);
        auto _anchor = node.anchor();
        auto value = REF(CallTemplate::from(g_sc_loop_new, {
            SCOPES_GET_RESULT(quote(level, node->init))
        }));
        auto args = REF(CallTemplate::from(g_sc_loop_arguments, { value }));
        auto typedargs = SCOPES_GET_RESULT(prove(ctx, args));
        ctx.frame->bind(args, typedargs);
        bind(node->args, typedargs);
        auto expr = REF(Expression::unscoped_from());
        expr->append(value);
        expr->append(args);
        expr->append(REF(CallTemplate::from(g_sc_loop_set_body, { value,
            SCOPES_GET_RESULT(quote(level, node->value)) })));
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(ValueRef) quote_CompileStage(int level, const CompileStageRef &node) {
        SCOPES_RESULT_TYPE(ValueRef);
        SCOPES_ERROR(QuoteUnsupportedValueKind, node->kind());
    }

    SCOPES_RESULT(CallTemplateRef) quote_KeyedTemplate(int level, const KeyedTemplateRef &node) {
        SCOPES_RESULT_TYPE(CallTemplateRef);
        auto value = SCOPES_GET_RESULT(quote(level, node->value));
        auto _anchor = node.anchor();
        return REF(CallTemplate::from(g_sc_keyed_new,
            { REF(ConstInt::symbol_from(node->key)), value }));
    }

    SCOPES_RESULT(ValueRef) quote_CallTemplate(int level, const CallTemplateRef &node) {
        SCOPES_RESULT_TYPE(ValueRef);
        auto _anchor = node.anchor();
        auto value = REF(CallTemplate::from(g_sc_call_new, {
            SCOPES_GET_RESULT(quote(level, node->callee))
        }));
        auto expr = REF(Expression::unscoped_from());
        if (node->is_rawcall()) {
            expr->append(REF(CallTemplate::from(g_sc_call_set_rawcall, { value,
                REF(ConstInt::from(TYPE_Bool, true)) })));
        }
        for (auto &&arg : node->args) {
            expr->append(REF(CallTemplate::from(g_sc_call_append_argument,
                { value, SCOPES_GET_RESULT(quote(level, arg)) })));
        }
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(TypedValueRef) quote_ParameterTemplate(int level, const ParameterTemplateRef &sym) {
        SCOPES_RESULT_TYPE(TypedValueRef);
        auto value = resolve(sym);
        if (!value) {
            SCOPES_ERROR(QuoteUnboundValue, sym);
        }
        return value;
    }

    SCOPES_RESULT(TypedValueRef) quote_LoopArguments(int level, const LoopArgumentsRef &sym) {
        SCOPES_RESULT_TYPE(TypedValueRef);
        auto value = resolve(sym);
        if (!value) {
            SCOPES_ERROR(QuoteUnboundValue, sym);
        }
        return value;
    }

    SCOPES_RESULT(ValueRef) quote_CaseTemplate(int level, const CaseTemplateRef &node) {
        SCOPES_RESULT_TYPE(ValueRef);
        auto _anchor = node.anchor();
        ValueRef result;
        switch(node->case_kind) {
        case CK_Case: {
            result = REF(CallTemplate::from(g_sc_case_new, {
                SCOPES_GET_RESULT(quote(level, node->literal)),
                SCOPES_GET_RESULT(quote(level, node->value)) }));
        } break;
        case CK_Pass: {
            result = REF(CallTemplate::from(g_sc_pass_case_new, {
                SCOPES_GET_RESULT(quote(level, node->literal)),
                SCOPES_GET_RESULT(quote(level, node->value)) }));
        } break;
        case CK_Do: {
            result = REF(CallTemplate::from(g_sc_do_case_new, {
                SCOPES_GET_RESULT(quote(level, node->value)) }));
        } break;
        case CK_Default: {
            result = REF(CallTemplate::from(g_sc_default_case_new, {
                SCOPES_GET_RESULT(quote(level, node->value)) }));
        } break;
        default: assert(false);
        }
        return result;
    }

    SCOPES_RESULT(ValueRef) quote_SwitchTemplate(int level, const SwitchTemplateRef &node) {
        SCOPES_RESULT_TYPE(ValueRef);
        auto _anchor = node.anchor();
        auto value = REF(CallTemplate::from(g_sc_switch_new, {
            SCOPES_GET_RESULT(quote(level, node->expr))
        }));
        auto expr = REF(Expression::unscoped_from());
        for (auto &&_case : node->cases) {
            auto _anchor = _case.anchor();
            expr->append(REF(CallTemplate::from(g_sc_switch_append, { value,
                SCOPES_GET_RESULT(quote(level, _case)) })));
        }
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(ValueRef) quote_CondTemplate(int level, const CondTemplateRef &node) {
        SCOPES_RESULT_TYPE(ValueRef);
        auto _anchor = node.anchor();
        return ValueRef(REF(CallTemplate::from(g_sc_cond_new, {
            SCOPES_GET_RESULT(quote(level, node->cond)),
            SCOPES_GET_RESULT(quote(level, node->then_value)),
            SCOPES_GET_RESULT(quote(level, node->else_value))
        })));
    }

    SCOPES_RESULT(ValueRef) quote_Template(int level, const TemplateRef &node) {
        SCOPES_RESULT_TYPE(ValueRef);
        auto _anchor = node.anchor();
        auto value = REF(CallTemplate::from(g_sc_template_new,
            { REF(ConstInt::symbol_from(node->name)) }));
        auto expr = REF(Expression::unscoped_from());
        if (node->is_inline()) {
            expr->append(REF(CallTemplate::from(g_sc_template_set_inline, { value })));
        }
        for (auto &&param : node->params) {
            expr->append(REF(CallTemplate::from(g_sc_template_append_parameter, {
                value, SCOPES_GET_RESULT(quote_param(param))
            })));
        }
        expr->append(REF(CallTemplate::from(g_sc_template_set_body, { value,
            SCOPES_GET_RESULT(quote(level, node->value)) })));
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(CallTemplateRef) quote_Quote(int level, const QuoteRef &node) {
        SCOPES_RESULT_TYPE(CallTemplateRef);
        auto _anchor = node.anchor();
        return REF(CallTemplate::from(g_sc_quote_new,
            { SCOPES_GET_RESULT(quote(level+1, node->value)) }));
    }

    ValueRef quote_typed_argument_list(const ArgumentListRef &node) {
        /*if (node->values.size() == 1) {
            return quote_typed(node->values[0]);
        } else*/ {
            Values newvalues;
            auto _anchor = node.anchor();
            int count = (int)node->values.size();
            newvalues.reserve(count);
            for (int i = 0; i < count; ++i) {
                newvalues.push_back(quote_typed(node->values[i]));
            }
            return build_quoted_argument_list(_anchor, newvalues);
        }
    }

    ValueRef quote_typed(const TypedValueRef &node) {
        auto T = node->get_type();
        if (T == TYPE_ValueRef)
            return node;
        if (is_reference(T) && (strip_qualifiers(T) == TYPE_ValueRef)) {
            auto _anchor = node.anchor();
            return REF(CallTemplate::from(g_deref, { node }));
        }

        if (node.isa<ArgumentList>()) {
            return quote_typed_argument_list(node.cast<ArgumentList>());
        } else {
            auto result = wrap_value(node->get_type(), node);
            assert(result);
            return result;
        }
    }

    SCOPES_RESULT(ValueRef) quote_Unquote(int level, const UnquoteRef &node) {
        SCOPES_RESULT_TYPE(ValueRef);
        assert(level >= 0);
        if (!level) {
            auto value = SCOPES_GET_RESULT(prove(ctx, node->value));
            auto T = value->get_type();
            if (is_arguments_type(T)) {
                auto at = cast<ArgumentsType>(T);
                auto _anchor = node.anchor();
                {
                    Values newvalues;
                    int count = (int)at->values.size();
                    newvalues.reserve(count);
                    for (int i = 0; i < count; ++i) {
                        newvalues.push_back(ExtractArgument::from(value, i));
                    }
                    return build_quoted_argument_list(_anchor, newvalues);
                }
            } else {
                return quote_typed(value);
            }
        } else {
            auto _anchor = node.anchor();
            return ValueRef(REF(CallTemplate::from(g_sc_unquote_new,
                { SCOPES_GET_RESULT(quote(level-1, node->value)) })));
        }
    }

    SCOPES_RESULT(CallTemplateRef) quote_MergeTemplate(int level, const MergeTemplateRef &node) {
        SCOPES_RESULT_TYPE(CallTemplateRef);
        auto _anchor = node.anchor();
        return REF(CallTemplate::from(g_sc_merge_new, {
            SCOPES_GET_RESULT(quote(level, node->label)),
            SCOPES_GET_RESULT(quote(level, node->value))
        }));
    }

    SCOPES_RESULT(ValueRef) quote_LabelTemplate(int level, const LabelTemplateRef &node) {
        SCOPES_RESULT_TYPE(ValueRef);
        auto _anchor = node.anchor();
        auto value = REF(CallTemplate::from(g_sc_label_new, {
            REF(ConstInt::from(TYPE_I32, node->label_kind)),
            REF(ConstInt::symbol_from(node->name))
        }));
        auto typedvalue = SCOPES_GET_RESULT(prove(ctx, value));
        bind(node, typedvalue);
        ctx.frame->bind(node, typedvalue);
        auto expr = REF(Expression::unscoped_from());
        expr->append(REF(CallTemplate::from(g_sc_label_set_body, { typedvalue,
            SCOPES_GET_RESULT(quote(level, node->value)) })));
        expr->append(typedvalue);
        return canonicalize(expr);
    }

    SCOPES_RESULT(ValueRef) quote_new_node(int level, const ValueRef &node) {
        SCOPES_RESULT_TYPE(ValueRef);
        assert(node);
        ValueRef result;
        if (node.isa<TypedValue>()) {
            result = quote_typed(node.cast<TypedValue>());
        } else {
            // we shouldn't set an anchor here because sometimes the parent context
            // is more indicative than the node position
            //SCOPES_CHECK_RESULT(verify_stack());
            switch(node->kind()) {
    #define T(NAME, BNAME, CLASS) \
            case NAME: result = SCOPES_GET_RESULT(quote_ ## CLASS(level, node.cast<CLASS>())); break;
            SCOPES_UNTYPED_VALUE_KIND()
    #undef T
            default: assert(false);
            }
            assert(result);
        }
        return result;
    }

    SCOPES_RESULT(TypedValueRef) quote(int level, const ValueRef &node) {
        SCOPES_RESULT_TYPE(TypedValueRef);
        assert(node);
        assert(ctx.frame);
        {
            // check if node is already typed
            TypedValueRef result = SCOPES_GET_RESULT(ctx.frame->resolve(node, ctx.function));
            if (result) {
                // check if we have an existing wrap for the node
                TypedValueRef wrapped = resolve(result);
                if (!wrapped) {
                    // wrap it anew and type it
                    wrapped = SCOPES_GET_RESULT(prove(ctx, quote_typed(result)));
                    bind(result, wrapped);
                }
                return wrapped;
            }
        }
        // check if we have an existing quote for the node
        TypedValueRef result = resolve(node);
        if (result) return result;
        // node is untyped or unbound yet
        ValueRef untyped_result = SCOPES_GET_RESULT(quote_new_node(level, node));
        if (untyped_result.isa<TypedValue>()) {
            result = untyped_result.cast<TypedValue>();
        } else {
            result = SCOPES_GET_RESULT(prove(ctx, untyped_result));
        }
        bind(node, result);
        if (!node.isa<TypedValue>()) {
            #if 0
            StyledStream ss;
            ss << "binding ";
            stream_ast(ss, node, StreamASTFormat());
            ss << " to ";
            stream_ast(ss, result, StreamASTFormat());
            ss << std::endl;
            #endif
            // ensure that the unquoted context can access the typed result
            ctx.frame->bind(node, result);
        }
        return result;
    }

    void bind(const ValueRef &oldnode, const TypedValueRef &newnode) {
        auto it = map.insert({oldnode.unref(), newnode});
        if (!it.second) {
            it.first->second = newnode;
        }
    }

    TypedValueRef resolve(const ValueRef &node) const {
        auto it = map.find(node.unref());
        if (it == map.end())
            return TypedValueRef();
        return it->second;
    }

    #define T(NAME, BNAME, CLASS) \
        SCOPES_RESULT(ConstAggregateRef) quote_ ## CLASS(int level, const ValueRef &node) { \
            return ConstAggregate::ast_from(node); \
        }
    SCOPES_PURE_VALUE_KIND()
    #undef T

    absl::flat_hash_map<Value *, TypedValueRef> map;
    const ASTContext &ctx;
};

ValueRef unwrap_value(const Type *T, const ValueRef &value) {
    auto _anchor = value.anchor();
    //T = strip_qualifiers(T);
    auto ST = storage_type(T).assert_ok();
    auto kind = ST->kind();
    switch(kind) {
    case TK_Pointer: {
        return REF(CallTemplate::from(g_bitcast, {
                REF(CallTemplate::from(g_sc_const_pointer_extract, { value })),
                REF(ConstPointer::type_from(T))
            }));
    } break;
    case TK_Integer: {
        auto ti = cast<IntegerType>(ST);
        if (ti->width <= 64ull) {
            return REF(CallTemplate::from(g_itrunc, {
                    REF(CallTemplate::from(g_sc_const_int_extract, { value })),
                    REF(ConstPointer::type_from(T))
                }));
        } else {
            // big integer
            auto result = REF(Expression::unscoped_from());
            size_t numwords = (ti->width + 63ull) / 64ull;
            auto mem = REF(CallTemplate::from(g_alloca_array, {
                    REF(ConstPointer::type_from(TYPE_U64)),
                    REF(ConstInt::from(TYPE_I32, numwords)),
                }));
            result->append(mem);
            for (size_t i = 0; i < numwords; ++i) {
                auto word = REF(CallTemplate::from(g_sc_const_int_extract_word, {
                    value,
                    REF(ConstInt::from(TYPE_I32, i)),
                }));
                result->append(word);
                auto ptr = REF(CallTemplate::from(g_getelementptr, { mem,
                    REF(ConstInt::from(TYPE_I32, i)) }));
                result->append(ptr);
                result->append(REF(CallTemplate::from(g_store, { word, ptr })));
            }
            auto castmem =
                REF(CallTemplate::from(g_bitcast, { mem,
                    REF(ConstPointer::type_from((local_ro_pointer_type(T)))) }));
            result->append(castmem);
            result->append(REF(CallTemplate::from(g_load, { castmem })));
            return result;
        }
    } break;
    case TK_Real: {
        return REF(CallTemplate::from(g_fptrunc, {
                REF(CallTemplate::from(g_sc_const_real_extract, { value })),
                REF(ConstPointer::type_from(T))
            }));
    } break;
    case TK_Vector: {
        auto vt = cast<VectorType>(ST);
        auto argT = vt->element_type;
        auto numvals = (int)vt->count();
        //auto numelems = ConstInt::from(anchor, TYPE_I32, numvals);
        ValueRef result = REF(Undef::from(T));
        for (int i = 0; i < numvals; ++i) {
            auto idx = REF(ConstInt::from(TYPE_I32, i));
            auto arg =
                REF(CallTemplate::from(g_sc_const_extract_at, { value, idx }));
            auto unwrapped_arg = unwrap_value(argT, arg);
            assert(unwrapped_arg);
            result = REF(CallTemplate::from(g_insertelement, { result, unwrapped_arg, idx }));
        }
        return result;
    } break;
    case TK_Array:
    case TK_Matrix: {
        auto at = cast<ArrayLikeType>(ST);
        auto argT = at->element_type;
        auto numvals = (int)at->count();
        //auto numelems = ConstInt::from(anchor, TYPE_I32, numvals);
        ValueRef result = REF(Undef::from(T));
        for (int i = 0; i < numvals; ++i) {
            auto idx = REF(ConstInt::from(TYPE_I32, i));
            auto arg =
                REF(CallTemplate::from(g_sc_const_extract_at, { value, idx }));
            auto unwrapped_arg = unwrap_value(argT, arg);
            assert(unwrapped_arg);
            result = REF(CallTemplate::from(g_insertvalue, { result, unwrapped_arg, idx }));
        }
        return result;
    } break;
    case TK_Tuple: {
        auto tt = cast<TupleType>(ST);
        //auto numelems = ConstInt::from(anchor, TYPE_I32, tt->values.size());
        ValueRef result = REF(Undef::from(T));
        for (int i = 0; i < tt->values.size(); ++i) {
            auto idx = REF(ConstInt::from(TYPE_I32, i));
            auto arg =
                REF(CallTemplate::from(g_sc_const_extract_at, { value, idx }));
            auto argT = tt->values[i];
            auto unwrapped_arg = unwrap_value(argT, arg);
            assert(unwrapped_arg);
            result = REF(CallTemplate::from(g_insertvalue, { result, unwrapped_arg, idx }));
        }
        //StyledStream ss;
        //stream_ast(ss, result, StreamASTFormat());
        return result;
    } break;
    default:
        break;
    }
    return ValueRef();
}

static ValueRef wrap_value_inner(const Type *T, const ValueRef &value, bool composite) {
    auto _anchor = value.anchor();
    bool isref = is_reference(T);
    T = strip_qualifiers(T);
    auto ST = storage_type(T).assert_ok();
    auto kind = ST->kind();
    switch(kind) {
    case TK_Pointer: {
        if (!composite && (T == TYPE_String)) {
            return REF(CallTemplate::from(g_sc_const_string_new, { value }));
        } else {
            return REF(CallTemplate::from(g_sc_const_pointer_new, {
                    REF(ConstPointer::type_from(T)),
                    REF(CallTemplate::from(g_bitcast, { value, g_voidstar })) }));
        }
    } break;
    case TK_Integer: {
        auto ti = cast<IntegerType>(ST);
        if (ti->width <= 64ull) {
            return REF(CallTemplate::from(g_sc_const_int_new, {
                    REF(ConstPointer::type_from(T)),
                    REF(CallTemplate::from(ti->issigned?g_sext:g_zext, { value,
                    g_u64 })) }));
        } else {
            // big integer
            auto targettype = local_ro_pointer_type(TYPE_U64);
            auto result = REF(Expression::unscoped_from());
            auto mem = REF(CallTemplate::from(g_alloca, {
                    REF(ConstPointer::type_from(T))
                }));
            result->append(mem);
            result->append(REF(CallTemplate::from(g_store, { value, mem })));
            auto castmem =
                REF(CallTemplate::from(g_bitcast, { mem,
                    REF(ConstPointer::type_from(targettype)) }));
            result->append(castmem);
            size_t numwords = (ti->width + 63ull) / 64ull;
            result->append(REF(CallTemplate::from(g_sc_const_int_words_new, {
                    REF(ConstPointer::type_from(T)),
                    REF(ConstInt::from(TYPE_I32, numwords)),
                    castmem
                })));
            return result;
        }
    } break;
    case TK_Real: {
        //auto ti = cast<RealType>(ST);
        return REF(CallTemplate::from(g_sc_const_real_new, {
                REF(ConstPointer::type_from(T)),
                REF(CallTemplate::from(g_fpext, { value,
                g_f64 })) }));
    } break;
    case TK_Vector: {
        auto at = cast<VectorType>(ST);
        auto result = REF(Expression::unscoped_from());
        auto ET = at->element_type;
        auto numvals = (int)at->count();
        auto numelems = REF(ConstInt::from(TYPE_I32, numvals));
        auto buf = REF(CallTemplate::from(g_alloca_array, {
                REF(ConstPointer::type_from(TYPE_ValueRef)),
                numelems
            }));
        result->append(buf);
        for (int i = 0; i < numvals; ++i) {
            auto idx = REF(ConstInt::from(TYPE_I32, i));
            auto arg =
                REF(CallTemplate::from(g_extractelement, { value, idx }));
            auto wrapped_arg = wrap_value(ET, arg, true);
            assert(wrapped_arg);
            result->append(
                REF(CallTemplate::from(g_store, {
                    wrapped_arg,
                    REF(CallTemplate::from(g_getelementptr, { buf, idx }))
                })));
        }
        result->append(REF(CallTemplate::from(g_sc_const_aggregate_new,
            { REF(ConstPointer::type_from(T)), numelems, buf })));
        return result;
    } break;
    case TK_Array:
    case TK_Matrix: {
        auto at = cast<ArrayLikeType>(ST);
        auto result = REF(Expression::unscoped_from());
        auto ET = at->element_type;
        auto numvals = (int)at->count();
        auto numelems = REF(ConstInt::from(TYPE_I32, numvals));
        bool is_stringarray = (ET == TYPE_Char);
        ValueRef buf;
        if (is_stringarray && isref) {
            const Type *rawstring = native_ro_pointer_type(TYPE_Char);
            buf = REF(CallTemplate::from(g_bitcast, {
                    REF(CallTemplate::from(g_reftoptr, { value })),
                    REF(ConstPointer::type_from(rawstring))
                }));
            result->append(buf);
        } else {
            if (is_stringarray) {
                buf = REF(CallTemplate::from(g_alloca_array, {
                    REF(ConstPointer::type_from(TYPE_Char)),
                    numelems
                }));
            } else {
                buf = REF(CallTemplate::from(g_alloca_array, {
                    REF(ConstPointer::type_from(TYPE_ValueRef)),
                    numelems
                }));
            }
            result->append(buf);
            for (int i = 0; i < numvals; ++i) {
                auto idx = REF(ConstInt::from(TYPE_I32, i));
                ValueRef arg =
                    REF(CallTemplate::from(g_extractvalue, { value, idx }));
                if (!is_stringarray) {
                    arg = wrap_value(ET, arg, true);
                    assert(arg);
                }
                result->append(
                    REF(CallTemplate::from(g_store, {
                        arg,
                        REF(CallTemplate::from(g_getelementptr, { buf, idx }))
                    })));
            }
        }
        if (is_stringarray) {
            result->append(REF(CallTemplate::from(g_sc_const_string_new,
                { REF(CallTemplate::from(g_sc_string_new, { buf, numelems })) })));
        } else {
            result->append(REF(CallTemplate::from(g_sc_const_aggregate_new,
                { REF(ConstPointer::type_from(T)), numelems, buf })));
        }
        return result;
    } break;
    case TK_Tuple: {
        auto tt = cast<TupleType>(ST);
        auto result = REF(Expression::unscoped_from());
        auto numelems = REF(ConstInt::from(TYPE_I32, tt->values.size()));
        auto buf = REF(CallTemplate::from(g_alloca_array, {
                REF(ConstPointer::type_from(TYPE_ValueRef)),
                numelems
            }));
        result->append(buf);
        for (int i = 0; i < tt->values.size(); ++i) {
            auto idx = REF(ConstInt::from(TYPE_I32, i));
            auto arg =
                REF(CallTemplate::from(g_extractvalue, { value, idx }));
            auto argT = tt->values[i];
            auto wrapped_arg = wrap_value(argT, arg, true);
            assert(wrapped_arg);
            result->append(
                REF(CallTemplate::from(g_store, {
                    wrapped_arg,
                    REF(CallTemplate::from(g_getelementptr, { buf, idx }))
                })));
        }
        result->append(REF(CallTemplate::from(g_sc_const_aggregate_new,
            { REF(ConstPointer::type_from(T)), numelems, buf })));
        return result;
    } break;
    default:
        break;
    }
    return ValueRef();
}

ValueRef wrap_value(const Type *T, const ValueRef &value, bool composite) {
    if (is_value_stage_constant(value)) {
        return ConstAggregate::ast_from(value);
    }
    if (!is_opaque(T)) {
        auto result = wrap_value_inner(T, value, composite);
        if (!is_plain(T) && !is_view(T)) {
            auto _anchor = value.anchor();
            auto expr = REF(Expression::unscoped_from());
            expr->append(result);
            expr->append(REF(CallTemplate::from(g_lose, {value})));
            expr->append(result);
            return expr;
        }
        return result;
    }
    return ValueRef();
}

SCOPES_RESULT(TypedValueRef) quote(const ASTContext &ctx, const ValueRef &node) {
    return Quoter(ctx).quote(0, node);
}

//------------------------------------------------------------------------------

} // namespace scopes
