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
#include "stream_ast.hpp"
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
#include "scopes/scopes.h"

//#pragma GCC diagnostic ignored "-Wvla-extension"

namespace scopes {

//------------------------------------------------------------------------------

struct Quoter {
    Quoter(const ASTContext  &_ctx) :
        ctx(_ctx) {}

    Value *canonicalize(Expression *expr) {
        if (expr->body.empty())
            return expr->value;
        return expr;
    }

    SCOPES_RESULT(Value *) quote_Expression(int level, Expression *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        auto value = CallTemplate::from(_anchor, g_sc_expression_new, {
            ConstPointer::anchor_from(_anchor)
        });
        auto expr = Expression::unscoped_from(_anchor);
        if (node->scoped) {
            expr->append(CallTemplate::from(_anchor, g_sc_expression_set_scoped, { value }));
        }
        for (auto &&instr : node->body) {
            expr->append(CallTemplate::from(_anchor, g_sc_expression_append,
                { value, SCOPES_GET_RESULT(quote(level, instr)) }));
        }
        expr->append(CallTemplate::from(_anchor, g_sc_expression_append,
            { value, SCOPES_GET_RESULT(quote(level, node->value)) }));
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(Value *) quote_ArgumentListTemplate(int level, ArgumentListTemplate *node) {
        SCOPES_RESULT_TYPE(Value *);
        /*if (node->values.size() == 1) {
            return quote(level, node->values[0]);
        } else*/ {
            auto _anchor = node->anchor();
            auto value = CallTemplate::from(_anchor, g_sc_argument_list_new, {
                ConstPointer::anchor_from(_anchor)
            });
            auto expr = Expression::unscoped_from(_anchor);
            int count = (int)node->values.size();
            for (int i = 0; i < count; ++i) {
                expr->append(CallTemplate::from(_anchor, g_sc_argument_list_append,
                    { value, SCOPES_GET_RESULT(quote(level, node->values[i])) }));
            }
            expr->append(value);
            return canonicalize(expr);
        }
    }

    SCOPES_RESULT(Value *) quote_ExtractArgumentTemplate(int level, ExtractArgumentTemplate *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        if (node->vararg) {
            return CallTemplate::from(_anchor, g_sc_extract_argument_list_new, {
                    ConstPointer::anchor_from(_anchor),
                    SCOPES_GET_RESULT(quote(level, node->value)),
                    ConstInt::from(_anchor, TYPE_I32, node->index) });
        } else {
            return CallTemplate::from(_anchor, g_sc_extract_argument_new, {
                    ConstPointer::anchor_from(_anchor),
                    SCOPES_GET_RESULT(quote(level, node->value)),
                    ConstInt::from(_anchor, TYPE_I32, node->index) });
        }
    }

    SCOPES_RESULT(TypedValue *) quote_param(ParameterTemplate *node) {
        SCOPES_RESULT_TYPE(TypedValue *);
        auto _anchor = node->anchor();
        auto newparam = CallTemplate::from(node->anchor(), g_sc_parameter_new, {
            ConstPointer::anchor_from(_anchor),
            ConstInt::symbol_from(_anchor, node->name) });
        auto typednewparam = SCOPES_GET_RESULT(prove(ctx, newparam));
        bind(node, typednewparam);
        ctx.frame->bind(node, typednewparam);
        return typednewparam;
    }

    SCOPES_RESULT(Value *) quote_Loop(int level, Loop *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        auto value = CallTemplate::from(_anchor, g_sc_loop_new, {
            ConstPointer::anchor_from(_anchor),
            SCOPES_GET_RESULT(quote(level, node->init))
        });
        auto args = CallTemplate::from(_anchor, g_sc_loop_arguments, { value });
        auto typedargs = SCOPES_GET_RESULT(prove(ctx, args));
        ctx.frame->bind(args, typedargs);
        bind(node->args, typedargs);
        auto expr = Expression::unscoped_from(_anchor);
        expr->append(value);
        expr->append(args);
        expr->append(CallTemplate::from(_anchor, g_sc_loop_set_body, { value,
            SCOPES_GET_RESULT(quote(level, node->value)) }));
        expr->append(value);
        return canonicalize(expr);
    }

    #define T(NAME, BNAME, CLASS) \
        SCOPES_RESULT(Value *) quote_ ## CLASS(int level, Value *node) { \
            return ConstPointer::ast_from(node->anchor(), node); \
        }
    SCOPES_PURE_VALUE_KIND()
    #undef T

    SCOPES_RESULT(Value *) quote_Break(int level, Break *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        return CallTemplate::from(_anchor, g_sc_break_new, {
            ConstPointer::anchor_from(_anchor),
            SCOPES_GET_RESULT(quote(level, node->value))
        });
    }

    SCOPES_RESULT(Value *) quote_RepeatTemplate(int level, RepeatTemplate *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        return CallTemplate::from(_anchor, g_sc_repeat_new, {
            ConstPointer::anchor_from(_anchor),
            SCOPES_GET_RESULT(quote(level, node->value))
        });
    }

    SCOPES_RESULT(Value *) quote_ReturnTemplate(int level, ReturnTemplate *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        return CallTemplate::from(_anchor, g_sc_return_new, {
            ConstPointer::anchor_from(_anchor),
            SCOPES_GET_RESULT(quote(level, node->value))
        });
    }

    SCOPES_RESULT(Value *) quote_RaiseTemplate(int level, RaiseTemplate *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        return CallTemplate::from(_anchor, g_sc_raise_new, {
            ConstPointer::anchor_from(_anchor),
            SCOPES_GET_RESULT(quote(level, node->value))
        });
    }

    SCOPES_RESULT(Value *) quote_CompileStage(int level, CompileStage *node) {
        SCOPES_RESULT_TYPE(Value *);
        SCOPES_LOCATION_ERROR(String::from("cannot quote compile stage"));
    }

    SCOPES_RESULT(Value *) quote_KeyedTemplate(int level, KeyedTemplate *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto value = SCOPES_GET_RESULT(quote(level, node->value));
        auto _anchor = node->anchor();
        return CallTemplate::from(_anchor, g_sc_keyed_new,
            {
                ConstPointer::anchor_from(_anchor),
                ConstInt::symbol_from(_anchor, node->key), value });
    }

    SCOPES_RESULT(Value *) quote_Anchored(int level, Anchored *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto value = SCOPES_GET_RESULT(quote(level, node->value));
        auto _anchor = node->anchor;
        return CallTemplate::from(_anchor, g_sc_anchored_new, {
            ConstPointer::anchor_from(_anchor), value });
    }

    SCOPES_RESULT(Value *) quote_CallTemplate(int level, CallTemplate *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        auto value = CallTemplate::from(_anchor, g_sc_call_new, {
            ConstPointer::anchor_from(_anchor),
            SCOPES_GET_RESULT(quote(level, node->callee))
        });
        auto expr = Expression::unscoped_from(_anchor);
        if (node->is_rawcall()) {
            expr->append(CallTemplate::from(_anchor, g_sc_call_set_rawcall, { value,
                ConstInt::from(_anchor, TYPE_Bool, true) }));
        }
        for (auto &&arg : node->args) {
            expr->append(CallTemplate::from(_anchor, g_sc_call_append_argument,
                { value, SCOPES_GET_RESULT(quote(level, arg)) }));
        }
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(Value *) quote_ParameterTemplate(int level, ParameterTemplate *sym) {
        SCOPES_RESULT_TYPE(Value *);
        auto value = resolve(sym);
        if (!value) {
            SCOPES_EXPECT_ERROR(error_unbound_symbol(sym));
        }
        return value;
    }

    SCOPES_RESULT(Value *) quote_LoopArguments(int level, LoopArguments *sym) {
        SCOPES_RESULT_TYPE(Value *);
        auto value = resolve(sym);
        if (!value) {
            SCOPES_EXPECT_ERROR(error_unbound_symbol(sym));
        }
        return value;
    }

    SCOPES_RESULT(Value *) quote_SwitchTemplate(int level, SwitchTemplate *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        auto value = CallTemplate::from(_anchor, g_sc_switch_new, {
            ConstPointer::anchor_from(_anchor),
            SCOPES_GET_RESULT(quote(level, node->expr))
        });
        auto expr = Expression::unscoped_from(_anchor);
        for (auto &&_case : node->cases) {
            auto _case_anchor = _case.anchor;
            switch(_case.kind) {
            case CK_Case: {
                expr->append(CallTemplate::from(_case_anchor, g_sc_switch_append_case, { value,
                    ConstPointer::anchor_from(_case_anchor),
                    SCOPES_GET_RESULT(quote(level, _case.literal)),
                    SCOPES_GET_RESULT(quote(level, _case.value)) }));
            } break;
            case CK_Pass: {
                expr->append(CallTemplate::from(_case_anchor, g_sc_switch_append_pass, { value,
                    ConstPointer::anchor_from(_case_anchor),
                    SCOPES_GET_RESULT(quote(level, _case.literal)),
                    SCOPES_GET_RESULT(quote(level, _case.value)) }));
            } break;
            case CK_Default: {
                expr->append(CallTemplate::from(_case_anchor, g_sc_switch_append_default, { value,
                    ConstPointer::anchor_from(_case_anchor),
                    SCOPES_GET_RESULT(quote(level, _case.value)) }));
            } break;
            default: assert(false);
            }
        }
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(Value *) quote_If(int level, If *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        auto value = CallTemplate::from(_anchor, g_sc_if_new, {
            ConstPointer::anchor_from(_anchor)
        });
        auto expr = Expression::unscoped_from(_anchor);
        for (auto &&clause : node->clauses) {
            if (clause.is_then()) {
                expr->append(CallTemplate::from(_anchor, g_sc_if_append_then_clause, { value,
                    ConstPointer::anchor_from(_anchor),
                    SCOPES_GET_RESULT(quote(level, clause.cond)),
                    SCOPES_GET_RESULT(quote(level, clause.value)) }));
            } else {
                expr->append(CallTemplate::from(_anchor, g_sc_if_append_else_clause, { value,
                    ConstPointer::anchor_from(_anchor),
                    SCOPES_GET_RESULT(quote(level, clause.value)) }));
            }
        }
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(Value *) quote_Template(int level, Template *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        auto value = CallTemplate::from(_anchor, g_sc_template_new,
            {
                ConstPointer::anchor_from(_anchor),
                ConstInt::symbol_from(_anchor, node->name) });
        auto expr = Expression::unscoped_from(_anchor);
        if (node->is_inline()) {
            expr->append(CallTemplate::from(_anchor, g_sc_template_set_inline, { value }));
        }
        for (auto &&param : node->params) {
            expr->append(CallTemplate::from(_anchor, g_sc_template_append_parameter, {
                value, SCOPES_GET_RESULT(quote_param(param))
            }));
        }
        expr->append(CallTemplate::from(_anchor, g_sc_template_set_body, { value,
            SCOPES_GET_RESULT(quote(level, node->value)) }));
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(Value *) quote_Quote(int level, Quote *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        return CallTemplate::from(_anchor, g_sc_quote_new,
            {
                ConstPointer::anchor_from(_anchor),
                SCOPES_GET_RESULT(quote(level+1, node->value)) });
    }

    Value *quote_typed_argument_list(ArgumentList *node) {
        /*if (node->values.size() == 1) {
            return quote_typed(node->values[0]);
        } else*/ {
            auto _anchor = node->anchor();
            auto value = CallTemplate::from(_anchor, g_sc_argument_list_new, {
                ConstPointer::anchor_from(_anchor)
            });
            auto expr = Expression::unscoped_from(_anchor);
            int count = (int)node->values.size();
            for (int i = 0; i < count; ++i) {
                expr->append(CallTemplate::from(_anchor, g_sc_argument_list_append,
                    { value, quote_typed(node->values[i]) }));
            }
            expr->append(value);
            return canonicalize(expr);
        }
    }

    Value *quote_typed(TypedValue *node) {
        if (node->get_type() == TYPE_Value)
            return node;
        if (is_value_stage_constant(node)) {
            return ConstPointer::ast_from(node->anchor(), node);
        } else if (isa<ArgumentList>(node)) {
            return quote_typed_argument_list(cast<ArgumentList>(node));
        } else {
            auto result = wrap_value(node->get_type(), node);
            assert(result);
            return result;
        }
    }

    SCOPES_RESULT(Value *) quote_Unquote(int level, Unquote *node) {
        SCOPES_RESULT_TYPE(Value *);
        SCOPES_ANCHOR(node->anchor());
        assert(level >= 0);
        if (!level) {
            auto value = SCOPES_GET_RESULT(prove(ctx, node->value));
            auto T = value->get_type();
            if (is_arguments_type(T)) {
                auto at = cast<ArgumentsType>(T);
                auto _anchor = node->anchor();
                {
                    auto result = CallTemplate::from(_anchor, g_sc_argument_list_new, {
                        ConstPointer::anchor_from(_anchor)
                    });
                    auto expr = Expression::unscoped_from(_anchor);
                    int count = (int)at->values.size();
                    for (int i = 0; i < count; ++i) {
                        expr->append(CallTemplate::from(_anchor, g_sc_argument_list_append,
                            { result,
                                ExtractArgument::from(_anchor, value, i) }));
                    }
                    expr->append(result);
                    return canonicalize(expr);
                }
            } else {
                return quote_typed(value);
            }
        } else {
            auto _anchor = node->anchor();
            return CallTemplate::from(_anchor, g_sc_unquote_new,
                {
                    ConstPointer::anchor_from(_anchor),
                    SCOPES_GET_RESULT(quote(level-1, node->value)) });
        }
    }

    SCOPES_RESULT(Value *) quote_MergeTemplate(int level, MergeTemplate *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        return CallTemplate::from(_anchor, g_sc_merge_new, {
            ConstPointer::anchor_from(_anchor),
            SCOPES_GET_RESULT(quote(level, node->label)),
            SCOPES_GET_RESULT(quote(level, node->value))
        });
    }

    SCOPES_RESULT(Value *) quote_LabelTemplate(int level, LabelTemplate *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        auto value = CallTemplate::from(_anchor, g_sc_label_new, {
            ConstPointer::anchor_from(_anchor),
            ConstInt::from(_anchor, TYPE_I32, node->label_kind),
            ConstInt::symbol_from(_anchor, node->name)
        });
        auto typedvalue = SCOPES_GET_RESULT(prove(ctx, value));
        bind(node, typedvalue);
        ctx.frame->bind(node, typedvalue);
        auto expr = Expression::unscoped_from(_anchor);
        expr->append(CallTemplate::from(_anchor, g_sc_label_set_body, { typedvalue,
            SCOPES_GET_RESULT(quote(level, node->value)) }));
        expr->append(typedvalue);
        return canonicalize(expr);
    }

    SCOPES_RESULT(Value *) quote_new_node(int level, Value *node) {
        SCOPES_RESULT_TYPE(Value *);
        assert(node);
        Value *result = nullptr;
        if (isa<TypedValue>(node)) {
            result = quote_typed(cast<TypedValue>(node));
        } else {
            // we shouldn't set an anchor here because sometimes the parent context
            // is more indicative than the node position
            //SCOPES_CHECK_RESULT(verify_stack());
            switch(node->kind()) {
    #define T(NAME, BNAME, CLASS) \
            case NAME: result = SCOPES_GET_RESULT(quote_ ## CLASS(level, cast<CLASS>(node))); break;
            SCOPES_UNTYPED_VALUE_KIND()
    #undef T
            default: assert(false);
            }
            assert(result);
        }
        return result;
    }

    SCOPES_RESULT(TypedValue *) quote(int level, Value *node) {
        SCOPES_RESULT_TYPE(TypedValue *);
        assert(node);
        assert(ctx.frame);
        {
            // check if node is already typed
            TypedValue *result = SCOPES_GET_RESULT(ctx.frame->resolve(node, ctx.function));
            if (result) {
                // check if we have an existing wrap for the node
                TypedValue *wrapped = resolve(result);
                if (!wrapped) {
                    // wrap it anew and type it
                    wrapped = SCOPES_GET_RESULT(prove(ctx, quote_typed(result)));
                    bind(result, wrapped);
                }
                return wrapped;
            }
        }
        // check if we have an existing quote for the node
        TypedValue *result = resolve(node);
        if (result) return result;
        // node is untyped or unbound yet
        Value *untyped_result = SCOPES_GET_RESULT(quote_new_node(level, node));
        if (isa<TypedValue>(untyped_result)) {
            result = cast<TypedValue>(untyped_result);
        } else {
            result = SCOPES_GET_RESULT(prove(ctx, untyped_result));
        }
        bind(node, result);
        if (!isa<TypedValue>(node)) {
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

    void bind(Value *oldnode, TypedValue *newnode) {
        auto it = map.insert({oldnode, newnode});
        if (!it.second) {
            it.first->second = newnode;
        }
    }

    TypedValue *resolve(Value *node) const {
        auto it = map.find(node);
        if (it == map.end())
            return nullptr;
        return it->second;
    }

    std::unordered_map<Value *, TypedValue *> map;
    const ASTContext &ctx;
};

Value *unwrap_value(const Type *T, Value *value) {
    auto anchor = value->anchor();
    auto ST = storage_type(T).assert_ok();
    auto kind = ST->kind();
    switch(kind) {
    case TK_Pointer: {
        return CallTemplate::from(anchor, g_bitcast, {
                CallTemplate::from(anchor, g_sc_const_pointer_extract, { value }),
                ConstPointer::type_from(anchor, T)
            });
    } break;
    case TK_Integer: {
        return CallTemplate::from(anchor, g_itrunc, {
                CallTemplate::from(anchor, g_sc_const_int_extract, { value }),
                ConstPointer::type_from(anchor, T)
            });
    } break;
    case TK_Real: {
        return CallTemplate::from(anchor, g_fptrunc, {
                CallTemplate::from(anchor, g_sc_const_real_extract, { value }),
                ConstPointer::type_from(anchor, T)
            });
    } break;
    case TK_Vector: {
        auto vt = cast<VectorType>(ST);
        auto argT = vt->element_type;
        auto numvals = (int)vt->count;
        //auto numelems = ConstInt::from(anchor, TYPE_I32, numvals);
        auto result = CallTemplate::from(anchor, g_undef, {
                ConstPointer::type_from(anchor, T)
            });
        for (int i = 0; i < numvals; ++i) {
            auto idx = ConstInt::from(anchor, TYPE_I32, i);
            auto arg =
                CallTemplate::from(anchor, g_sc_const_extract_at, { value, idx });
            auto unwrapped_arg = unwrap_value(argT, arg);
            result = CallTemplate::from(anchor, g_insertelement, { result, unwrapped_arg, idx });
        }
        return result;
    } break;
    case TK_Array: {
        auto at = cast<ArrayType>(ST);
        auto argT = at->element_type;
        auto numvals = (int)at->count;
        //auto numelems = ConstInt::from(anchor, TYPE_I32, numvals);
        auto result = CallTemplate::from(anchor, g_undef, {
                ConstPointer::type_from(anchor, T)
            });
        for (int i = 0; i < numvals; ++i) {
            auto idx = ConstInt::from(anchor, TYPE_I32, i);
            auto arg =
                CallTemplate::from(anchor, g_sc_const_extract_at, { value, idx });
            auto unwrapped_arg = unwrap_value(argT, arg);
            result = CallTemplate::from(anchor, g_insertvalue, { result, unwrapped_arg, idx });
        }
        return result;
    } break;
    case TK_Tuple: {
        auto tt = cast<TupleType>(ST);
        //auto numelems = ConstInt::from(anchor, TYPE_I32, tt->values.size());
        auto result = CallTemplate::from(anchor, g_undef, {
                ConstPointer::type_from(anchor, T)
            });
        for (int i = 0; i < tt->values.size(); ++i) {
            auto idx = ConstInt::from(anchor, TYPE_I32, i);
            auto arg =
                CallTemplate::from(anchor, g_sc_const_extract_at, { value, idx });
            auto argT = tt->values[i];
            auto unwrapped_arg = unwrap_value(argT, arg);
            result = CallTemplate::from(anchor, g_insertvalue, { result, unwrapped_arg, idx });
        }
        //StyledStream ss;
        //stream_ast(ss, result, StreamASTFormat());
        return result;
    } break;
    default:
        break;
    }
    return nullptr;
}

Value *wrap_value(const Type *T, Value *value) {
    auto anchor = value->anchor();
    if (isa<Const>(value)) {
        if (T == TYPE_Value)
            return value;
        return ConstPointer::ast_from(value->anchor(), value);
    }
    if (!is_opaque(T)) {
        auto ST = storage_type(T).assert_ok();
        auto kind = ST->kind();
        switch(kind) {
        case TK_Pointer: {
            return CallTemplate::from(anchor, g_sc_const_pointer_new,
                {
                    ConstPointer::anchor_from(anchor),
                    ConstPointer::type_from(anchor, T),
                    CallTemplate::from(anchor, g_bitcast, { value, g_voidstar }) });
        } break;
        case TK_Integer: {
            auto ti = cast<IntegerType>(ST);
            return CallTemplate::from(anchor, g_sc_const_int_new,
                {
                    ConstPointer::anchor_from(anchor),
                    ConstPointer::type_from(anchor, T),
                    CallTemplate::from(anchor, ti->issigned?g_sext:g_zext, { value,
                    g_u64 }) });
        } break;
        case TK_Real: {
            //auto ti = cast<RealType>(ST);
            return CallTemplate::from(anchor, g_sc_const_real_new,
                {
                    ConstPointer::anchor_from(anchor),
                    ConstPointer::type_from(anchor, T),
                    CallTemplate::from(anchor, g_fpext, { value,
                    g_f64 }) });
        } break;
        case TK_Vector: {
            auto at = cast<VectorType>(ST);
            auto result = Expression::unscoped_from(anchor);
            auto ET = at->element_type;
            auto numvals = (int)at->count;
            auto numelems = ConstInt::from(anchor, TYPE_I32, numvals);
            auto buf = CallTemplate::from(anchor, g_alloca_array, {
                    ConstPointer::type_from(anchor, TYPE_Value),
                    numelems
                });
            result->append(buf);
            for (int i = 0; i < numvals; ++i) {
                auto idx = ConstInt::from(anchor, TYPE_I32, i);
                auto arg =
                    CallTemplate::from(anchor, g_extractelement, { value, idx });
                auto wrapped_arg = wrap_value(ET, arg);
                result->append(
                    CallTemplate::from(anchor, g_store, {
                        wrapped_arg,
                        CallTemplate::from(anchor, g_getelementptr, { buf, idx })
                    }));
            }
            result->append(CallTemplate::from(anchor, g_sc_const_aggregate_new,
                { ConstPointer::anchor_from(anchor),
                ConstPointer::type_from(anchor, T), numelems, buf }));
            return result;
        } break;
        case TK_Array: {
            auto at = cast<ArrayType>(ST);
            auto result = Expression::unscoped_from(anchor);
            auto ET = at->element_type;
            auto numvals = (int)at->count;
            auto numelems = ConstInt::from(anchor, TYPE_I32, numvals);
            auto buf = CallTemplate::from(anchor, g_alloca_array, {
                    ConstPointer::type_from(anchor, TYPE_Value),
                    numelems
                });
            result->append(buf);
            for (int i = 0; i < numvals; ++i) {
                auto idx = ConstInt::from(anchor, TYPE_I32, i);
                auto arg =
                    CallTemplate::from(anchor, g_extractvalue, { value, idx });
                auto wrapped_arg = wrap_value(ET, arg);
                result->append(
                    CallTemplate::from(anchor, g_store, {
                        wrapped_arg,
                        CallTemplate::from(anchor, g_getelementptr, { buf, idx })
                    }));
            }
            result->append(CallTemplate::from(anchor, g_sc_const_aggregate_new,
                { ConstPointer::anchor_from(anchor),
                 ConstPointer::type_from(anchor, T), numelems, buf }));
            return result;
        } break;
        case TK_Tuple: {
            auto tt = cast<TupleType>(ST);
            auto result = Expression::unscoped_from(anchor);
            auto numelems = ConstInt::from(anchor, TYPE_I32, tt->values.size());
            auto buf = CallTemplate::from(anchor, g_alloca_array, {
                    ConstPointer::type_from(anchor, TYPE_Value),
                    numelems
                });
            result->append(buf);
            for (int i = 0; i < tt->values.size(); ++i) {
                auto idx = ConstInt::from(anchor, TYPE_I32, i);
                auto arg =
                    CallTemplate::from(anchor, g_extractvalue, { value, idx });
                auto argT = tt->values[i];
                auto wrapped_arg = wrap_value(argT, arg);
                result->append(
                    CallTemplate::from(anchor, g_store, {
                        wrapped_arg,
                        CallTemplate::from(anchor, g_getelementptr, { buf, idx })
                    }));
            }
            result->append(CallTemplate::from(anchor, g_sc_const_aggregate_new,
                { ConstPointer::anchor_from(anchor),
                    ConstPointer::type_from(anchor, T), numelems, buf }));
            return result;
        } break;
        default:
            break;
        }
    }
    return nullptr;
}

SCOPES_RESULT(TypedValue *) quote(const ASTContext &ctx, Value *node) {
    return Quoter(ctx).quote(0, node);
}

//------------------------------------------------------------------------------

} // namespace scopes
