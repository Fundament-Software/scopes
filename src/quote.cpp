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
        auto value = Call::from(_anchor, g_sc_expression_new, {});
        auto expr = Expression::unscoped_from(_anchor);
        if (node->scoped) {
            expr->append(Call::from(_anchor, g_sc_expression_set_scoped, { value }));
        }
        for (auto &&instr : node->body) {
            expr->append(Call::from(_anchor, g_sc_expression_append,
                { value, SCOPES_GET_RESULT(quote(level, instr)) }));
        }
        expr->append(Call::from(_anchor, g_sc_expression_append,
            { value, SCOPES_GET_RESULT(quote(level, node->value)) }));
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(Value *) quote_ArgumentList(int level, ArgumentList *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        auto value = Call::from(_anchor, g_sc_argument_list_new, {});
        auto expr = Expression::unscoped_from(_anchor);
        int count = (int)node->values.size();
        for (int i = 0; i < count; ++i) {
            expr->append(Call::from(_anchor, g_sc_argument_list_append,
                { value, SCOPES_GET_RESULT(quote(level, node->values[i])) }));
        }
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(Value *) quote_ExtractArgument(int level, ExtractArgument *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        if (node->vararg) {
            return Call::from(_anchor, g_sc_extract_argument_list_new, {
                    SCOPES_GET_RESULT(quote(level, node->value)),
                    ConstInt::from(_anchor, TYPE_I32, node->index) });
        } else {
            return Call::from(_anchor, g_sc_extract_argument_new, {
                    SCOPES_GET_RESULT(quote(level, node->value)),
                    ConstInt::from(_anchor, TYPE_I32, node->index) });
        }
    }

    SCOPES_RESULT(Value *) quote_param(Parameter *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto newparam = Call::from(node->anchor(), g_sc_parameter_new, {
            ConstInt::symbol_from(node->anchor(), node->name) });
        auto typednewparam = SCOPES_GET_RESULT(prove(ctx, newparam));
        bind(node, typednewparam);
        ctx.frame->bind(node, typednewparam);
        return typednewparam;
    }

    SCOPES_RESULT(Value *) quote_Loop(int level, Loop *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        auto value = Call::from(_anchor, g_sc_loop_new, {
            SCOPES_GET_RESULT(quote(level, node->init))
        });
        bind(node, value);
        auto expr = Expression::unscoped_from(_anchor);
        expr->append(value);
        expr->append(Call::from(_anchor, g_sc_loop_set_body, { value,
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
        return Call::from(_anchor, g_sc_break_new, {
            SCOPES_GET_RESULT(quote(level, node->value))
        });
    }

    SCOPES_RESULT(Value *) quote_Repeat(int level, Repeat *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        return Call::from(_anchor, g_sc_repeat_new, {
            SCOPES_GET_RESULT(quote(level, node->value))
        });
    }

    SCOPES_RESULT(Value *) quote_Return(int level, Return *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        return Call::from(_anchor, g_sc_return_new, {
            SCOPES_GET_RESULT(quote(level, node->value))
        });
    }

    SCOPES_RESULT(Value *) quote_Raise(int level, Raise *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        return Call::from(_anchor, g_sc_raise_new, {
            SCOPES_GET_RESULT(quote(level, node->value))
        });
    }

    SCOPES_RESULT(Value *) quote_CompileStage(int level, CompileStage *node) {
        SCOPES_RESULT_TYPE(Value *);
        SCOPES_LOCATION_ERROR(String::from("cannot quote compile stage"));
    }

    SCOPES_RESULT(Value *) quote_Keyed(int level, Keyed *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto value = SCOPES_GET_RESULT(quote(level, node->value));
        auto _anchor = node->anchor();
        return Call::from(_anchor, g_sc_keyed_new,
            { ConstInt::symbol_from(_anchor, node->key), value });
    }

    SCOPES_RESULT(Value *) quote_Call(int level, Call *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        auto value = Call::from(_anchor, g_sc_call_new, {
            SCOPES_GET_RESULT(quote(level, node->callee))
        });
        auto expr = Expression::unscoped_from(_anchor);
        if (node->is_rawcall()) {
            expr->append(Call::from(_anchor, g_sc_call_set_rawcall, { value,
                ConstInt::from(_anchor, TYPE_Bool, true) }));
        }
        for (auto &&arg : node->args) {
            expr->append(Call::from(_anchor, g_sc_call_append_argument,
                { value, SCOPES_GET_RESULT(quote(level, arg)) }));
        }
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(Value *) quote_Parameter(int level, Parameter *sym) {
        SCOPES_RESULT_TYPE(Value *);
        auto value = resolve(sym);
        if (!value) {
            SCOPES_EXPECT_ERROR(error_unbound_symbol(sym));
        }
        return value;
    }

    SCOPES_RESULT(Value *) quote_Switch(int level, Switch *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        auto value = Call::from(_anchor, g_sc_switch_new, {
            SCOPES_GET_RESULT(quote(level, node->expr))
        });
        auto expr = Expression::unscoped_from(_anchor);
        for (auto &&_case : node->cases) {
            switch(_case.kind) {
            case CK_Case: {
                expr->append(Call::from(_anchor, g_sc_switch_append_case, { value,
                    SCOPES_GET_RESULT(quote(level, _case.literal)),
                    SCOPES_GET_RESULT(quote(level, _case.value)) }));
            } break;
            case CK_Pass: {
                expr->append(Call::from(_anchor, g_sc_switch_append_pass, { value,
                    SCOPES_GET_RESULT(quote(level, _case.literal)),
                    SCOPES_GET_RESULT(quote(level, _case.value)) }));
            } break;
            case CK_Default: {
                expr->append(Call::from(_anchor, g_sc_switch_append_default, { value,
                    SCOPES_GET_RESULT(quote(level, _case.value)) }));
            } break;
            default: assert(false);
            }
        }
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(Value *) quote_CondBr(int level, CondBr *node) {
        SCOPES_RESULT_TYPE(Value *);
        assert(false);
        return nullptr;
    }

    SCOPES_RESULT(Value *) quote_LoopLabel(int level, LoopLabel *node) {
        SCOPES_RESULT_TYPE(Value *);
        assert(false);
        return nullptr;
    }

    SCOPES_RESULT(Value *) quote_Exception(int level, Exception *node) {
        SCOPES_RESULT_TYPE(Value *);
        assert(false);
        return nullptr;
    }

    SCOPES_RESULT(Value *) quote_Label(int level, Label *node) {
        SCOPES_RESULT_TYPE(Value *);
        assert(false);
        return nullptr;
    }

    SCOPES_RESULT(Value *) quote_Merge(int level, Merge *node) {
        SCOPES_RESULT_TYPE(Value *);
        assert(false);
        return nullptr;
    }

    SCOPES_RESULT(Value *) quote_If(int level, If *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        auto value = Call::from(_anchor, g_sc_if_new, {});
        auto expr = Expression::unscoped_from(_anchor);
        for (auto &&clause : node->clauses) {
            if (clause.is_then()) {
                expr->append(Call::from(_anchor, g_sc_if_append_then_clause, { value,
                    SCOPES_GET_RESULT(quote(level, clause.cond)),
                    SCOPES_GET_RESULT(quote(level, clause.value)) }));
            } else {
                expr->append(Call::from(_anchor, g_sc_if_append_else_clause, { value,
                    SCOPES_GET_RESULT(quote(level, clause.value)) }));
            }
        }
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(Value *) quote_Template(int level, Template *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        auto value = Call::from(_anchor, g_sc_template_new,
            { ConstInt::symbol_from(_anchor, node->name) });
        auto expr = Expression::unscoped_from(_anchor);
        if (node->is_inline()) {
            expr->append(Call::from(_anchor, g_sc_template_set_inline, { value }));
        }
        for (auto &&param : node->params) {
            expr->append(Call::from(_anchor, g_sc_template_append_parameter, {
                value, SCOPES_GET_RESULT(quote_param(param))
            }));
        }
        expr->append(Call::from(_anchor, g_sc_template_set_body, { value,
            SCOPES_GET_RESULT(quote(level, node->value)) }));
        expr->append(value);
        return canonicalize(expr);
    }

    SCOPES_RESULT(Value *) quote_Quote(int level, Quote *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        return Call::from(_anchor, g_sc_quote_new,
            { SCOPES_GET_RESULT(quote(level+1, node->value)) });
    }

    Value *quote_typed(Value *node) {
        if (node->get_type() == TYPE_Value)
            return node;
        if (isa<Pure>(node)) {
            return ConstPointer::ast_from(node->anchor(), node);
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
                auto result = Call::from(_anchor, g_sc_argument_list_new, {});
                auto expr = Expression::unscoped_from(_anchor);
                int count = (int)at->values.size();
                for (int i = 0; i < count; ++i) {
                    expr->append(Call::from(_anchor, g_sc_argument_list_append,
                        { result, quote_typed(
                        extract_argument(ctx, value, i)) }));
                }
                expr->append(result);
                return canonicalize(expr);
            } else {
                return quote_typed(value);
            }
        } else {
            auto _anchor = node->anchor();
            return Call::from(_anchor, g_sc_unquote_new,
                { SCOPES_GET_RESULT(quote(level-1, node->value)) });
        }
    }

    SCOPES_RESULT(Value *) quote_MergeTemplate(int level, MergeTemplate *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        return Call::from(_anchor, g_sc_merge_new, {
            SCOPES_GET_RESULT(quote(level, node->label)),
            SCOPES_GET_RESULT(quote(level, node->value))
        });
    }

    SCOPES_RESULT(Value *) quote_LabelTemplate(int level, LabelTemplate *node) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = node->anchor();
        auto value = Call::from(_anchor, g_sc_label_new, {
            ConstInt::from(_anchor, TYPE_I32, node->label_kind),
            ConstInt::symbol_from(_anchor, node->name)
        });
        bind(node, value);
        auto typedvalue = SCOPES_GET_RESULT(prove(ctx, value));
        ctx.frame->bind(node, typedvalue);
        auto expr = Expression::unscoped_from(_anchor);
        expr->append(Call::from(_anchor, g_sc_label_set_body, { typedvalue,
            SCOPES_GET_RESULT(quote(level, node->value)) }));
        expr->append(typedvalue);
        return canonicalize(expr);
    }

    SCOPES_RESULT(Value *) quote_new_node(int level, Value *node) {
        SCOPES_RESULT_TYPE(Value *);
        assert(node);
        Value *result = nullptr;
        if (node->is_typed()) {
            result = quote_typed(node);
        } else {
            // we shouldn't set an anchor here because sometimes the parent context
            // is more indicative than the node position
            //SCOPES_CHECK_RESULT(verify_stack());
            switch(node->kind()) {
    #define T(NAME, BNAME, CLASS) \
            case NAME: result = SCOPES_GET_RESULT(quote_ ## CLASS(level, cast<CLASS>(node))); break;
            SCOPES_VALUE_KIND()
    #undef T
            default: assert(false);
            }
            assert(result);
        }
        return result;
    }

    SCOPES_RESULT(Value *) quote(int level, Value *node) {
        SCOPES_RESULT_TYPE(Value *);
        assert(node);
        assert(ctx.frame);
        // check if node is already typed
        Value *result = SCOPES_GET_RESULT(ctx.frame->resolve(node, ctx.function));
        if (result) {
            assert(result->is_typed());
            // check if we have an existing wrap for the node
            Value *wrapped = resolve(result);
            if (!wrapped) {
                // wrap it anew
                wrapped = quote_typed(result);
                bind(result, wrapped);
            }
            return wrapped;
        }
        // check if we have an existing quote for the node
        result = resolve(node);
        if (!result) {
            // node is untyped or unbound yet
            result = SCOPES_GET_RESULT(quote_new_node(level, node));
            if (!result->is_typed()) {
                // type node
                result = SCOPES_GET_RESULT(prove(ctx, result));
            }
            bind(node, result);
            if (!node->is_typed() && !node->is_pure()) {
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
        }
        return result;
    }

    void bind(Value *oldnode, Value *newnode) {
        auto it = map.insert({oldnode, newnode});
        if (!it.second) {
            it.first->second = newnode;
        }
    }

    Value *resolve(Value *node) const {
        auto it = map.find(node);
        if (it == map.end())
            return nullptr;
        return it->second;
    }

    std::unordered_map<Value *, Value *> map;
    const ASTContext &ctx;
};

Value *unwrap_value(const Type *T, Value *value) {
    auto anchor = value->anchor();
    auto ST = storage_type(T).assert_ok();
    auto kind = ST->kind();
    switch(kind) {
    case TK_Pointer: {
        return Call::from(anchor, g_bitcast, {
                Call::from(anchor, g_sc_const_pointer_extract, { value }),
                ConstPointer::type_from(anchor, T)
            });
    } break;
    case TK_Integer: {
        return Call::from(anchor, g_itrunc, {
                Call::from(anchor, g_sc_const_int_extract, { value }),
                ConstPointer::type_from(anchor, T)
            });
    } break;
    case TK_Real: {
        return Call::from(anchor, g_fptrunc, {
                Call::from(anchor, g_sc_const_real_extract, { value }),
                ConstPointer::type_from(anchor, T)
            });
    } break;
    case TK_Vector: {
        auto vt = cast<VectorType>(ST);
        auto argT = vt->element_type;
        auto numvals = (int)vt->count;
        auto numelems = ConstInt::from(anchor, TYPE_I32, numvals);
        auto result = Call::from(anchor, g_undef, {
                ConstPointer::type_from(anchor, T)
            });
        for (int i = 0; i < numvals; ++i) {
            auto idx = ConstInt::from(anchor, TYPE_I32, i);
            auto arg =
                Call::from(anchor, g_sc_const_extract_at, { value, idx });
            auto unwrapped_arg = unwrap_value(argT, arg);
            result = Call::from(anchor, g_insertelement, { result, unwrapped_arg, idx });
        }
        return result;
    } break;
    case TK_Array: {
        auto at = cast<ArrayType>(ST);
        auto argT = at->element_type;
        auto numvals = (int)at->count;
        auto numelems = ConstInt::from(anchor, TYPE_I32, numvals);
        auto result = Call::from(anchor, g_undef, {
                ConstPointer::type_from(anchor, T)
            });
        for (int i = 0; i < numvals; ++i) {
            auto idx = ConstInt::from(anchor, TYPE_I32, i);
            auto arg =
                Call::from(anchor, g_sc_const_extract_at, { value, idx });
            auto unwrapped_arg = unwrap_value(argT, arg);
            result = Call::from(anchor, g_insertvalue, { result, unwrapped_arg, idx });
        }
        return result;
    } break;
    case TK_Tuple: {
        auto tt = cast<TupleType>(ST);
        auto numelems = ConstInt::from(anchor, TYPE_I32, tt->values.size());
        auto result = Call::from(anchor, g_undef, {
                ConstPointer::type_from(anchor, T)
            });
        for (int i = 0; i < tt->values.size(); ++i) {
            auto idx = ConstInt::from(anchor, TYPE_I32, i);
            auto arg =
                Call::from(anchor, g_sc_const_extract_at, { value, idx });
            auto argT = tt->values[i];
            auto unwrapped_arg = unwrap_value(argT, arg);
            result = Call::from(anchor, g_insertvalue, { result, unwrapped_arg, idx });
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
            return Call::from(anchor, g_sc_const_pointer_new,
                { ConstPointer::type_from(anchor, T),
                    Call::from(anchor, g_bitcast, { value, g_voidstar }) });
        } break;
        case TK_Integer: {
            auto ti = cast<IntegerType>(ST);
            return Call::from(anchor, g_sc_const_int_new,
                { ConstPointer::type_from(anchor, T),
                    Call::from(anchor, ti->issigned?g_sext:g_zext, { value,
                    g_u64 }) });
        } break;
        case TK_Real: {
            auto ti = cast<RealType>(ST);
            return Call::from(anchor, g_sc_const_real_new,
                { ConstPointer::type_from(anchor, T),
                    Call::from(anchor, g_fpext, { value,
                    g_f64 }) });
        } break;
        case TK_Vector: {
            auto at = cast<VectorType>(ST);
            auto result = Expression::unscoped_from(anchor);
            auto ET = at->element_type;
            auto numvals = (int)at->count;
            auto numelems = ConstInt::from(anchor, TYPE_I32, numvals);
            auto buf = Call::from(anchor, g_alloca_array, {
                    ConstPointer::type_from(anchor, TYPE_Value),
                    numelems
                });
            result->append(buf);
            for (int i = 0; i < numvals; ++i) {
                auto idx = ConstInt::from(anchor, TYPE_I32, i);
                auto arg =
                    Call::from(anchor, g_extractelement, { value, idx });
                auto wrapped_arg = wrap_value(ET, arg);
                result->append(
                    Call::from(anchor, g_store, {
                        wrapped_arg,
                        Call::from(anchor, g_getelementptr, { buf, idx })
                    }));
            }
            result->append(Call::from(anchor, g_sc_const_aggregate_new,
                { ConstPointer::type_from(anchor, T), numelems, buf }));
            return result;
        } break;
        case TK_Array: {
            auto at = cast<ArrayType>(ST);
            auto result = Expression::unscoped_from(anchor);
            auto ET = at->element_type;
            auto numvals = (int)at->count;
            auto numelems = ConstInt::from(anchor, TYPE_I32, numvals);
            auto buf = Call::from(anchor, g_alloca_array, {
                    ConstPointer::type_from(anchor, TYPE_Value),
                    numelems
                });
            result->append(buf);
            for (int i = 0; i < numvals; ++i) {
                auto idx = ConstInt::from(anchor, TYPE_I32, i);
                auto arg =
                    Call::from(anchor, g_extractvalue, { value, idx });
                auto wrapped_arg = wrap_value(ET, arg);
                result->append(
                    Call::from(anchor, g_store, {
                        wrapped_arg,
                        Call::from(anchor, g_getelementptr, { buf, idx })
                    }));
            }
            result->append(Call::from(anchor, g_sc_const_aggregate_new,
                { ConstPointer::type_from(anchor, T), numelems, buf }));
            return result;
        } break;
        case TK_Tuple: {
            auto tt = cast<TupleType>(ST);
            auto result = Expression::unscoped_from(anchor);
            auto numelems = ConstInt::from(anchor, TYPE_I32, tt->values.size());
            auto buf = Call::from(anchor, g_alloca_array, {
                    ConstPointer::type_from(anchor, TYPE_Value),
                    numelems
                });
            result->append(buf);
            for (int i = 0; i < tt->values.size(); ++i) {
                auto idx = ConstInt::from(anchor, TYPE_I32, i);
                auto arg =
                    Call::from(anchor, g_extractvalue, { value, idx });
                auto argT = tt->values[i];
                auto wrapped_arg = wrap_value(argT, arg);
                result->append(
                    Call::from(anchor, g_store, {
                        wrapped_arg,
                        Call::from(anchor, g_getelementptr, { buf, idx })
                    }));
            }
            result->append(Call::from(anchor, g_sc_const_aggregate_new,
                { ConstPointer::type_from(anchor, T), numelems, buf }));
            return result;
        } break;
        default:
            break;
        }
    }
    return nullptr;
}

SCOPES_RESULT(Value *) quote(const ASTContext &ctx, Value *node) {
    return Quoter(ctx).quote(0, node);
}

//------------------------------------------------------------------------------

} // namespace scopes
