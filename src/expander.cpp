/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "expander.hpp"
#include "list.hpp"
#include "error.hpp"
#include "type.hpp"
#include "type/arguments_type.hpp"
#include "type/pointer_type.hpp"
#include "type/function_type.hpp"
#include "scope.hpp"
#include "stream_expr.hpp"
#include "anchor.hpp"
#include "value.hpp"
#include "prover.hpp"
#include "timer.hpp"
#include "stream_ast.hpp"
#include "gc.hpp"
#include "dyn_cast.inc"
#include "scopes/scopes.h"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------
// MACRO EXPANDER
//------------------------------------------------------------------------------
// expands macros and generates the AST

static SCOPES_RESULT(void) verify_list_parameter_count(const char *context, const List *expr, int mincount, int maxcount, int starti = 1) {
    SCOPES_RESULT_TYPE(void);
    if (!expr) {
        SCOPES_LOCATION_ERROR(format("%s: expression is empty", context));
    }
    if ((mincount <= 0) && (maxcount == -1)) {
        return {};
    }
    int argcount = (int)expr->count - starti;

    if ((maxcount >= 0) && (argcount > maxcount)) {
        SCOPES_LOCATION_ERROR(
            format("%s: excess argument. At most %i arguments expected", context, maxcount));
    }
    if ((mincount >= 0) && (argcount < mincount)) {
        SCOPES_LOCATION_ERROR(
            format("%s: at least %i arguments expected, got %i", context, mincount, argcount));
    }
    return {};
}

//------------------------------------------------------------------------------

static Symbol try_extract_symbol(Value *node) {
    auto ptr = dyn_cast<ConstInt>(node);
    if (ptr && (ptr->get_type() == TYPE_Symbol))
        return Symbol::wrap(ptr->value);
    return SYM_Unnamed;
}

//------------------------------------------------------------------------------

struct Expander {
    Scope *env;
    Template *astscope;
    const List *next;
    static bool verbose;

    static const Type *list_expander_func_type;

    Expander(Scope *_env, Template *_astscope, const List *_next = EOL) :
        env(_env),
        astscope(_astscope),
        next(_next) {
        if (!list_expander_func_type) {
            list_expander_func_type = pointer_type(raising_function_type(
                arguments_type({TYPE_List, TYPE_Scope}),
                {TYPE_List, TYPE_Scope}), PTF_NonWritable, SYM_Unnamed);
        }
    }

    ~Expander() {}

    SCOPES_RESULT(Value *) expand_expression(const Anchor *anchor, const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        Expression *expr = nullptr;
        while (it) {
            next = it->next;
            if (!last_expression()) {
                const String *doc = try_extract_string(it->at);
                if (doc) {
                    env->set_doc(doc);
                }
            }
            if (!expr) {
                expr = Expression::from(anchor);
            }
            expr->append(SCOPES_GET_RESULT(expand(it->at)));
            it = next;
        }
        if (expr) {
            #if 1
            if (expr->body.empty())
                return expr->value;
            #endif
            return expr;
        }
        return ArgumentListTemplate::from(anchor);
    }

    SCOPES_RESULT(Value *) expand_run_stage(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();

        SCOPES_CHECK_RESULT(verify_list_parameter_count("run-stage", it, 0, 0));

        auto node = CompileStage::from(_anchor, next, env);
        next = EOL;
        return node;
    }

    SCOPES_RESULT(ParameterTemplate *) expand_parameter(Value *value, Value *node = nullptr) {
        SCOPES_RESULT_TYPE(ParameterTemplate *);
        const Anchor *anchor = value->anchor();
        if (isa<ParameterTemplate>(value)) {
            return cast<ParameterTemplate>(value);
        } else {
            Symbol sym = SCOPES_GET_RESULT(extract_symbol_constant(value));
            if (node && isa<Pure>(node)) {
                env->bind(sym, node);
                return nullptr;
            } else {
                ParameterTemplate *param = nullptr;
                if (ends_with_parenthesis(sym)) {
                    param = ParameterTemplate::variadic_from(anchor, sym);
                } else {
                    param = ParameterTemplate::from(anchor, sym);
                }
                env->bind(sym, param);
                return param;
            }
        }
    }

    struct ExpandFnSetup {
        bool inlined;

        ExpandFnSetup() {
            inlined = false;
        };
    };

    SCOPES_RESULT(Value *) expand_fn(const List *it, const ExpandFnSetup &setup) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();

        SCOPES_CHECK_RESULT(verify_list_parameter_count("fn", it, 1, -1));

        // skip head
        it = it->next;

        assert(it != EOL);

        bool continuing = false;
        Template *func = nullptr;
        Value *result = nullptr;
        //Any tryfunc_name = SCOPES_GET_RESULT(unsyntax(it->at));
        const Type *T = try_get_const_type(it->at);
        if (T == TYPE_Symbol) {
            auto sym = SCOPES_GET_RESULT(extract_symbol_constant(it->at));
            // named self-binding
            // see if we can find a forward declaration in the local scope
            if (env->lookup_local(sym, result)
                && isa<Template>(result)
                && cast<Template>(result)->is_forward_decl()) {
                func = cast<Template>(result);
                continuing = true;
            } else {
                func = Template::from(_anchor, sym);
                result = func;
                env->bind(sym, func);
            }
            it = it->next;
        } else if (T == TYPE_String) {
            auto str = try_extract_string(it->at);
            assert(str);
            // named lambda
            func = Template::from(_anchor, Symbol(str));
            result = func;
            it = it->next;
        } else {
            // unnamed lambda
            func = Template::from(_anchor, Symbol(SYM_Unnamed));
            result = func;
        }
        if (setup.inlined)
            func->set_inline();
        /*
        if (setup.quoted)
            result = ast_quote(func);
        */

        if (it == EOL) {
            // forward declaration
            if (T != TYPE_Symbol) {
                SCOPES_LOCATION_ERROR(
                    String::from("forward declared function must be named"));
            }
            return result;
        }

        const List *params = SCOPES_GET_RESULT(extract_list_constant(it->at));

        it = it->next;

        Scope *subenv = Scope::from(env);
        subenv->bind(KW_Recur, func);
        // ensure the local scope does not contain special symbols
        subenv = Scope::from(subenv);

        Expander subexpr(subenv, func);

        while (params != EOL) {
            func->append_param(SCOPES_GET_RESULT(subexpr.expand_parameter(params->at)));
            params = params->next;
        }

        if ((it != EOL) && (it->next != EOL)) {
            auto str = try_extract_string(it->at);
            if (str) {
                func->docstring = str;
                it = it->next;
            }
        }

        func->value = SCOPES_GET_RESULT(subexpr.expand_expression(_anchor, it));

        return result;
    }

    bool last_expression() {
        return next == EOL;
    }

#if 0
    SCOPES_RESULT(Any) expand_defer(const List *it, const Any &dest) {
        SCOPES_RESULT_TYPE(Any);
        auto _anchor = get_active_anchor();

        it = it->next;
        const List *body = it;
        const List *block = next;
        next = EOL;

        Label *nextstate = Label::continuation_from(_anchor, Symbol(SYM_Unnamed));

        SCOPES_CHECK_RESULT(expand_expression(block, nextstate));

        state = nextstate;
        // read parameter names
        it = SCOPES_GET_RESULT(unsyntax(it->at));
        while (it != EOL) {
            nextstate->append(SCOPES_GET_RESULT(expand_parameter(it->at)));
            it = it->next;
        }
        return expand_do(body, dest, false);
    }
#endif

    SCOPES_RESULT(Value *) expand_label(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();

        it = it->next;

        auto name = SCOPES_GET_RESULT(extract_symbol_constant(it->at));
        it = it->next;

        Scope *subenv = Scope::from(env);
        auto label = LabelTemplate::from(_anchor, LK_User, name);
        subenv->bind(name, label);
        Expander subexpr(subenv, astscope);
        label->value = SCOPES_GET_RESULT(subexpr.expand_expression(_anchor, it));
        return label;
    }

    SCOPES_RESULT(Value *) expand_do(const List *it) {
        //SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();

        it = it->next;

        Scope *subenv = Scope::from(env);
        Expander subexpr(subenv, astscope);
        return subexpr.expand_expression(_anchor, it);
    }

    SCOPES_RESULT(Value *) expand_inline_do(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();

        it = it->next;

        Expander subexpr(env, astscope);
        auto expr = SCOPES_GET_RESULT(subexpr.expand_expression(_anchor, it));
        if (isa<Expression>(expr)) {
            auto ex = cast<Expression>(expr);
            ex->scoped = false;
        }
        env = subexpr.env;
        return expr;
    }

    bool is_equal_token(Value *name) {
        auto tok = try_extract_symbol(name);
        return tok == OP_Set;
    }

    bool is_except_token(Value *name) {
        auto tok = try_extract_symbol(name);
        return tok == KW_Except;
    }

    void print_name_suggestions(Symbol name, StyledStream &ss) {
        auto syms = env->find_closest_match(name);
        if (!syms.empty()) {
            ss << "Did you mean '" << syms[0].name()->data << "'";
            for (size_t i = 1; i < syms.size(); ++i) {
                if ((i + 1) == syms.size()) {
                    ss << " or ";
                } else {
                    ss << ", ";
                }
                ss << "'" << syms[i].name()->data << "'";
            }
            ss << "?";
        }
    }

    // (loop ([x...] [= init...]) body...)
    SCOPES_RESULT(Value *) expand_loop(const List *it) {
        SCOPES_RESULT_TYPE(Value *);

        /*
        we're building this structure

        label break
            loop x... (init...)
                body...

        break labels never type themselves from their body
        */

        SCOPES_CHECK_RESULT(verify_list_parameter_count("loop", it, 1, -1));
        it = it->next;

        auto _anchor = get_active_anchor();

        const List *params = SCOPES_GET_RESULT(extract_list_constant(it->at));
        const List *values = nullptr;
        const List *body = it->next;
        auto endit = params;
        while (endit) {
            if (is_equal_token(endit->at))
                break;
            endit = endit->next;
        }
        if (endit != EOL)
            values = endit->next;

        it = values;

        Values initargs;
        if (it) {
            Expander subexp(env, astscope, it->next);
            SCOPES_CHECK_RESULT(subexp.expand_arguments(initargs, it));
        }

        Loop *loop = Loop::from(_anchor, ArgumentListTemplate::from(_anchor, initargs));
        LabelTemplate *break_label = LabelTemplate::from(_anchor, LK_Break, KW_Break, loop);

        Expander bodyexp(Scope::from(env), astscope);

        auto expr = Expression::from(_anchor);
        {
            int index = 0;
            it = params;
            // read parameter names
            while (it != endit) {
                auto paramval = it->at;
                Symbol sym = SCOPES_GET_RESULT(extract_symbol_constant(paramval));
                Value *node = nullptr;
                if (!ends_with_parenthesis(sym)) {
                    node = extract_argument(paramval->anchor(), loop->args, index);
                } else {
                    if (it->next != endit) {
                        SCOPES_ANCHOR(paramval->anchor());
                        SCOPES_EXPECT_ERROR(error_variadic_symbol_not_in_last_place());
                    }
                    node = extract_argument(paramval->anchor(), loop->args, index, true);
                }
                bodyexp.env->bind(sym, node);
                expr->append(node);
                it = it->next;
                index++;
            }
        }

        auto value = SCOPES_GET_RESULT(bodyexp.expand_expression(_anchor, body));
        expr->append(value);
        loop->value = expr;
        return break_label;
    }

    static Value *extract_argument(const Anchor *anchor, Value *node, int index, bool vararg = false) {
        if (isa<Const>(node)) {
            assert(!is_arguments_type(cast<Const>(node)->get_type()));
            if (index == 0) {
                return node;
            } else {
                return ConstAggregate::none_from(anchor);
            }
        }
        auto result = ExtractArgumentTemplate::from(anchor, node, index, vararg);
        if (!vararg) {
            result = KeyedTemplate::from(anchor, SYM_Unnamed, result);
        }
        return result;
    }

    // (let x ... [= args ...])
    // (let (x ... = args ...) ...
    // ...
    SCOPES_RESULT(Value *) expand_let(const List *it) {
        SCOPES_RESULT_TYPE(Value *);

        SCOPES_CHECK_RESULT(verify_list_parameter_count("let", it, 1, -1));
        it = it->next;

        auto _anchor = get_active_anchor();

        Values exprs;
        Values args;

        const Type *T = try_get_const_type(it->at);
        if (T == TYPE_List) {
            // alternative format
            const List *equit = it;
            while (equit) {
                SCOPES_ANCHOR(equit->at->anchor());
                it = SCOPES_GET_RESULT(extract_list_constant(equit->at));
                SCOPES_CHECK_RESULT(verify_list_parameter_count("=", it, 3, -1, 0));
                auto paramval = it->at;
                it = it->next;
                if (!is_equal_token(it->at)) {
                    SCOPES_ANCHOR(it->at->anchor());
                    SCOPES_LOCATION_ERROR(String::from("= token expected"));
                }
                it = it->next;
                // read init values
                Expander subexp(env, astscope);
                subexp.next = it->next;
                Value *node = SCOPES_GET_RESULT(subexp.expand(it->at));
                it = subexp.next;
                env = subexp.env;
                if (it) {
                    SCOPES_ANCHOR(it->at->anchor());
                    SCOPES_LOCATION_ERROR(String::from("extraneous argument"));
                }
                exprs.push_back(node);
                Symbol sym = SCOPES_GET_RESULT(extract_symbol_constant(paramval));
                if (!ends_with_parenthesis(sym)) {
                    node = extract_argument(paramval->anchor(), node, 0);
                }
                args.push_back(node);
                env->bind(sym, node);
                equit = equit->next;
            }
        } else {
            const List *values = nullptr;
            auto endit = it;
            // read parameter names
            while (endit) {
                if (is_equal_token(endit->at))
                    break;
                endit = endit->next;
            }
            if (endit != EOL)
                values = endit;

            if (!values) {
                // no assignments, reimport parameter names into local scope
                Value *last_entry = nullptr;
                while (it != endit) {
                    auto name = SCOPES_GET_RESULT(extract_symbol_constant(it->at));
                    ScopeEntry entry;
                    if (!env->lookup(name, entry)) {
                        SCOPES_ANCHOR(it->at->anchor());
                        StyledString ss;
                        ss.out << "no such name bound in parent scope: '"
                            << name.name()->data << "'. ";
                        print_name_suggestions(name, ss.out);
                        SCOPES_LOCATION_ERROR(ss.str());
                    }
                    env->bind_with_doc(name, entry);
                    last_entry = entry.expr;
                    it = it->next;
                }
                if (!last_entry) {
                    last_entry = ConstAggregate::none_from(_anchor);
                }
                return last_entry;
            }

            const List *params = it;

            it = values;
            it = it->next;
            // read init values
            Expander subexp(env, astscope);
            while (it) {
                subexp.next = it->next;
                auto node = SCOPES_GET_RESULT(subexp.expand(it->at));
                exprs.push_back(node);
                it = subexp.next;
            }
            env = subexp.env;

            Value *srcval = ArgumentListTemplate::from(_anchor, exprs);

            int index = 0;
            //int lastarg = (int)args.size() - 1;
            it = params;
            // read parameter names
            while (it != endit) {
                auto paramval = it->at;
                Symbol sym = SCOPES_GET_RESULT(extract_symbol_constant(paramval));
                Value *node = nullptr;
                if (!ends_with_parenthesis(sym)) {
                    node = extract_argument(paramval->anchor(), srcval, index);
                } else {
                    if (it->next != endit) {
                        SCOPES_ANCHOR(paramval->anchor());
                        SCOPES_EXPECT_ERROR(error_variadic_symbol_not_in_last_place());
                    }
                    node = extract_argument(paramval->anchor(), srcval, index, true);
                }
                args.push_back(node);
                env->bind(sym, node);
                it = it->next;
                index++;
            }
        }

        return Expression::from(_anchor, exprs,
            ArgumentListTemplate::from(_anchor, args));
    }

    // quote <value> ...
    SCOPES_RESULT(Value *) expand_syntax_quote(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();

        SCOPES_CHECK_RESULT(verify_list_parameter_count("syntax-quote", it, 1, -1));
        it = it->next;

        if (it->count == 1) {
            return it->at;
        } else {
            return ConstPointer::list_from(_anchor, it);
        }
    }

    SCOPES_RESULT(Value *) expand_syntax_log(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();

        SCOPES_CHECK_RESULT(verify_list_parameter_count("syntax-log", it, 1, 1));
        it = it->next;

        auto sym = SCOPES_GET_RESULT(extract_symbol_constant(it->at));
        if (sym == KW_True) {
            this->verbose = true;
        } else if (sym == KW_False) {
            this->verbose = false;
        } else {
            // ignore
        }

        return ConstAggregate::none_from(_anchor);
    }

    // (try [x ...]) (except (param ...) [expr ...])
    SCOPES_RESULT(Value *) expand_try(const List *it) {
        SCOPES_RESULT_TYPE(Value *);

        /*
        # we're building this structure
        label try
            let except-param ... =
                label except
                    merge try
                        do
                            try-body ...
                            (any raise merges to except)
            except-body ...
        */

        SCOPES_CHECK_RESULT(verify_list_parameter_count("try", it, 0, -1));
        it = it->next;

        auto _anchor = get_active_anchor();

        Expander subexp(Scope::from(env), astscope);

        Value *try_value = SCOPES_GET_RESULT(subexp.expand_expression(_anchor, it));
        LabelTemplate *try_label = LabelTemplate::try_from(_anchor);
        LabelTemplate *except_label = LabelTemplate::except_from(_anchor,
            MergeTemplate::from(_anchor, try_label, try_value));

        if (next != EOL) {
            auto _next_anchor = next->at->anchor();
            SCOPES_ANCHOR(_next_anchor);
            it = SCOPES_GET_RESULT(extract_list_constant(next->at));
            if (it != EOL) {
                if (is_except_token(it->at)) {
                    SCOPES_CHECK_RESULT(verify_list_parameter_count("except", it, 1, -1));
                    it = it->next;
                    next = next->next;

                    const List *params = SCOPES_GET_RESULT(extract_list_constant(it->at));
                    const List *body = it->next;

                    Expander subexp(Scope::from(env), astscope);
                    auto expr = Expression::from(_next_anchor);
                    expr->append(except_label);
                    {
                        int index = 0;
                        it = params;
                        // read parameter names
                        while (it != EOL) {
                            auto paramval = it->at;
                            Symbol sym = SCOPES_GET_RESULT(extract_symbol_constant(paramval));
                            Value *node = nullptr;
                            if (!ends_with_parenthesis(sym)) {
                                node = extract_argument(paramval->anchor(), except_label, index);
                            } else {
                                if (it->next != EOL) {
                                    SCOPES_ANCHOR(paramval->anchor());
                                    SCOPES_EXPECT_ERROR(error_variadic_symbol_not_in_last_place());
                                }
                                node = extract_argument(paramval->anchor(), except_label, index, true);
                            }
                            subexp.env->bind(sym, node);
                            expr->append(node);
                            it = it->next;
                            index++;
                        }
                    }

                    it = body;
                    Value *except_value = SCOPES_GET_RESULT(subexp.expand_expression(_anchor, it));
                    expr->append(except_value);
                    try_label->value = expr;
                    return try_label;
                }
            }
            SCOPES_LOCATION_ERROR(String::from("except block expected"));
        }
        SCOPES_LOCATION_ERROR(String::from("except block expected"));
    }

    // (switch cond)
    // [(case literal body ...)]
    // (default body ...)
    SCOPES_RESULT(Value *) expand_switch(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();

        SCOPES_CHECK_RESULT(verify_list_parameter_count("switch", it, 1, -1));
        it = it->next;

        Expander subexp(env, astscope);
        assert(it);
        subexp.next = it->next;
        auto expr = SCOPES_GET_RESULT(subexp.expand(it->at));
        it = subexp.next;

        SwitchTemplate::Cases cases;

        it = next;
    collect_case:
        if ((it == EOL) || (try_get_const_type(it->at) != TYPE_List)) {
            SCOPES_EXPECT_ERROR(error_missing_default_case());
        }
        next = it->next;
        auto _case = SwitchTemplate::Case();
        _case.anchor = it->at->anchor();
        it = SCOPES_GET_RESULT(extract_list_constant(it->at));
        SCOPES_CHECK_RESULT(verify_list_parameter_count("case", it, 1, -1));
        auto head = try_extract_symbol(it->at);
        if ((head == KW_Case) || (head == KW_Pass)) {
            if (head == KW_Pass) {
                _case.kind = CK_Pass;
            }

            it = it->next;
            subexp.next = it->next;
            _case.literal = SCOPES_GET_RESULT(subexp.expand(it->at));
            it = subexp.next;

            Expander nativeexp(Scope::from(env), astscope);
            _case.value = SCOPES_GET_RESULT(nativeexp.expand_expression(_case.anchor, it));
            cases.push_back(_case);

            it = next;
            goto collect_case;
        } else if (head == KW_Default) {
            it = it->next;

            _case.kind = CK_Default;

            Expander nativeexp(Scope::from(env), astscope);
            _case.value = SCOPES_GET_RESULT(nativeexp.expand_expression(_case.anchor, it));
            cases.push_back(_case);
        }

        return SwitchTemplate::from(_anchor, expr, cases);
    }

    // (if cond body ...)
    // [(elseif cond body ...)]
    // [(else body ...)]
    SCOPES_RESULT(Value *) expand_if(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();

        std::vector<const List *> branches;

    collect_branch:
        SCOPES_CHECK_RESULT(verify_list_parameter_count("if", it, 1, -1));
        branches.push_back(it);

        it = next;
        if (it != EOL) {
            auto itnext = it->next;
            auto T = try_get_const_type(it->at);
            if (T == TYPE_List) {
                it = SCOPES_GET_RESULT(extract_list_constant(it->at));
                if (it != EOL) {
                    auto head = try_extract_symbol(it->at);
                    if (head == KW_ElseIf) {
                        next = itnext;
                        goto collect_branch;
                    } else if (head == KW_Else) {
                        next = itnext;
                        branches.push_back(it);
                    } else {
                        branches.push_back(EOL);
                    }
                } else {
                    branches.push_back(EOL);
                }
            } else {
                branches.push_back(EOL);
            }
        } else {
            branches.push_back(EOL);
        }

        auto ifexpr = If::from(_anchor);

        int lastidx = (int)branches.size() - 1;
        for (int idx = 0; idx < lastidx; ++idx) {
            it = branches[idx];
            const Anchor *anchor = it->at->anchor();
            SCOPES_CHECK_RESULT(verify_list_parameter_count("branch", it, 1, -1));
            it = it->next;

            Expander subexp(env, astscope);
            assert(it);
            subexp.next = it->next;
            Value *cond = SCOPES_GET_RESULT(subexp.expand(it->at));
            it = subexp.next;

            subexp.env = Scope::from(env);
            ifexpr->append_then(anchor, cond,
                SCOPES_GET_RESULT(subexp.expand_expression(anchor, it)));
        }

        it = branches[lastidx];
        if (it != EOL) {
            const Anchor *anchor = it->at->anchor();
            it = it->next;
            Expander subexp(Scope::from(env), astscope);

            ifexpr->append_else(anchor,
                SCOPES_GET_RESULT(subexp.expand_expression(anchor, it)));
        }

        return ifexpr;
    }

    static SCOPES_RESULT(bool) get_kwargs(Value *it, Symbol &key, Value *&value) {
        SCOPES_RESULT_TYPE(bool);
        auto T = try_get_const_type(it);
        if (T != TYPE_List) return false;
        auto l = SCOPES_GET_RESULT(extract_list_constant(it));
        if (l == EOL) return false;
        if (l->count != 3) return false;
        it = l->at;
        T = try_get_const_type(it);
        if (T != TYPE_Symbol) return false;
        key = SCOPES_GET_RESULT(extract_symbol_constant(it));
        l = l->next;
        it = l->at;
        T = try_get_const_type(it);
        if (T != TYPE_Symbol) return false;
        auto sym = SCOPES_GET_RESULT(extract_symbol_constant(it));
        if (sym != OP_Set) return false;
        l = l->next;
        value = l->at;
        return true;
    }

    SCOPES_RESULT(void) expand_arguments(Values &args, const List *it) {
        SCOPES_RESULT_TYPE(void);
        while (it) {
            next = it->next;
            Symbol key = SYM_Unnamed;
            Value *value;
            SCOPES_ANCHOR(it->at->anchor());
            if (SCOPES_GET_RESULT(get_kwargs(it->at, key, value))) {
                args.push_back(
                    KeyedTemplate::from(get_active_anchor(), key,
                        SCOPES_GET_RESULT(expand(value))));
            } else {
                args.push_back(SCOPES_GET_RESULT(expand(it->at)));
            }
            it = next;
        }
        return {};
    }

    Value *ast_quote(Value *expr, const Anchor *anchor = nullptr) {
        if (isa<Expression>(expr)) {
            auto ex = cast<Expression>(expr);
            ex->scoped = false;
        }
        return Quote::from(anchor?anchor:(expr->anchor()), expr);
    }

    SCOPES_RESULT(Value *) expand_ast_quote(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();

        it = it->next;

        Expander subexpr(env, astscope);
        auto expr = SCOPES_GET_RESULT(subexpr.expand_expression(_anchor, it));
        return ast_quote(expr, _anchor);
    }

    SCOPES_RESULT(Value *) expand_ast_unquote(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();
        SCOPES_CHECK_RESULT(verify_list_parameter_count("ast-unquote", it, 0, -1));
        it = it->next;
        Expander subexp(env, astscope);
        return Unquote::from(_anchor,
            SCOPES_GET_RESULT(subexp.expand_expression(_anchor, it)));
    }

    SCOPES_RESULT(Value *) expand_ast_unquote_arguments(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();
        SCOPES_CHECK_RESULT(verify_list_parameter_count("ast-unquote-arguments", it, 0, -1));
        it = it->next;
        Values args;
        if (it) {
            Expander subexp(env, astscope, it->next);
            SCOPES_CHECK_RESULT(subexp.expand_arguments(args, it));
        }
        return Unquote::from(_anchor, ArgumentListTemplate::from(_anchor, args));
    }

    template<typename T>
    SCOPES_RESULT(Value *) build_terminator(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();
        Values args;
        if (it) {
            Expander subexp(env, astscope, it->next);
            SCOPES_CHECK_RESULT(subexp.expand_arguments(args, it));
        }
        return T::from(_anchor, ArgumentListTemplate::from(_anchor, args));
    }

    template<typename T>
    SCOPES_RESULT(Value *) build_terminator(const List *it, LabelTemplate *label) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();
        Values args;
        if (it) {
            Expander subexp(env, astscope, it->next);
            SCOPES_CHECK_RESULT(subexp.expand_arguments(args, it));
        }
        return T::from(_anchor, label, ArgumentListTemplate::from(_anchor, args));
    }

    SCOPES_RESULT(Value *) expand_return(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        SCOPES_CHECK_RESULT(verify_list_parameter_count("return", it, 0, -1));
        it = it->next;
        return build_terminator<ReturnTemplate>(it);
    }

    SCOPES_RESULT(Value *) expand_raise(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        SCOPES_CHECK_RESULT(verify_list_parameter_count("raise", it, 0, -1));
        it = it->next;
        return build_terminator<RaiseTemplate>(it);
    }

    SCOPES_RESULT(Value *) expand_break(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        SCOPES_CHECK_RESULT(verify_list_parameter_count("break", it, 0, -1));
        it = it->next;
        return build_terminator<Break>(it);
    }

    SCOPES_RESULT(Value *) expand_repeat(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        SCOPES_CHECK_RESULT(verify_list_parameter_count("repeat", it, 0, -1));
        it = it->next;
        return build_terminator<RepeatTemplate>(it);
    }

    SCOPES_RESULT(Value *) expand_merge(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        SCOPES_CHECK_RESULT(verify_list_parameter_count("merge", it, 1, -1));
        it = it->next;

        Expander subexp(Scope::from(env), astscope, it->next);
        Value *label = SCOPES_GET_RESULT(subexp.expand(it->at));
        it = subexp.next;

        if (!isa<LabelTemplate>(label)) {
            SCOPES_EXPECT_ERROR(error_label_expected(label));
        }
        return build_terminator<MergeTemplate>(it, cast<LabelTemplate>(label));
    }

    SCOPES_RESULT(Value *) expand_forward(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();
        SCOPES_CHECK_RESULT(verify_list_parameter_count("_", it, 0, -1));
        it = it->next;
        Values args;
        if (it) {
            Expander subexp(env, astscope, it->next);
            SCOPES_CHECK_RESULT(subexp.expand_arguments(args, it));
        }
        return ArgumentListTemplate::from(_anchor, args);
    }

    SCOPES_RESULT(Value *) expand_call(const List *it, uint32_t flags = 0) {
        SCOPES_RESULT_TYPE(Value *);

        auto _anchor = get_active_anchor();
        SCOPES_CHECK_RESULT(verify_list_parameter_count("call", it, 0, -1));
        Expander subexp(env, astscope, it->next);
        Value *enter = SCOPES_GET_RESULT(subexp.expand(it->at));

        auto call = CallTemplate::from(_anchor, enter);
        call->flags = flags;

        it = subexp.next;
        SCOPES_CHECK_RESULT(subexp.expand_arguments(call->args, it));
        return call;
    }

    SCOPES_RESULT(sc_list_scope_tuple_t) expand_symbol(Const *node) {
        SCOPES_RESULT_TYPE(sc_list_scope_tuple_t);
        Value *symbol_handler_node;
        if (env->lookup(Symbol(SYM_SymbolWildcard), symbol_handler_node)) {
            auto T = try_get_const_type(symbol_handler_node);
            if (T != list_expander_func_type) {
                StyledString ss;
                ss.out << "custom symbol expander has wrong type "
                    << T << ", must be " << list_expander_func_type;
                SCOPES_LOCATION_ERROR(ss.str());
            }
            sc_syntax_wildcard_func_t f = (sc_syntax_wildcard_func_t)cast<ConstPointer>(symbol_handler_node)->value;
            auto ok_result = f(List::from(node, next), env);
            if (!ok_result.ok) {
                SCOPES_RETURN_ERROR(ok_result.except);
            }
            return ok_result._0;
        }
        sc_list_scope_tuple_t result = { nullptr, nullptr };
        return result;
    }

    SCOPES_RESULT(Value *) expand(Value *anynode) {
        SCOPES_RESULT_TYPE(Value *);
    expand_again:
        SCOPES_CHECK_RESULT(verify_stack());
        SCOPES_ANCHOR(anynode->anchor());
        if (!isa<Const>(anynode)) {
            if (verbose) {
                StyledStream ss(SCOPES_CERR);
                ss << "ignoring " << anynode << std::endl;
            }
            // return as-is
            return anynode;
        }
        Const *node = cast<Const>(anynode);
        auto T = node->get_type();
        if (T == TYPE_List) {
            if (verbose) {
                StyledStream ss(SCOPES_CERR);
                ss << "expanding list " << node << std::endl;
            }

            const List *list = SCOPES_GET_RESULT(extract_list_constant(node));
            if (list == EOL) {
                SCOPES_LOCATION_ERROR(String::from("expression is empty"));
            }

            Value *head = list->at;
            if (isa<Const>(head)) {
            }

            auto headT = try_get_const_type(head);
            // resolve symbol
            if (headT == TYPE_Symbol) {
                Value *headnode = nullptr;
                if (env->lookup(SCOPES_GET_RESULT(extract_symbol_constant(head)), headnode)) {
                    head = headnode;
                    headT = try_get_const_type(head);
                }
            }

            if (headT == TYPE_Builtin) {
                Builtin func = SCOPES_GET_RESULT(extract_builtin_constant(head));
                switch(func.value()) {
                case FN_GetSyntaxScope:
                    SCOPES_CHECK_RESULT(verify_list_parameter_count("this-scope", list, 0, 0));
                    return ConstPointer::scope_from(node->anchor(), env);
                case KW_SyntaxLog: return expand_syntax_log(list);
                case KW_Fn: return expand_fn(list, ExpandFnSetup());
                case KW_Inline: {
                    ExpandFnSetup setup;
                    setup.inlined = true;
                    return expand_fn(list, setup);
                }
                case KW_RunStage: return expand_run_stage(list);
                case KW_Let: return expand_let(list);
                case KW_Loop: return expand_loop(list);
                case KW_Try: return expand_try(list);
                case KW_If: return expand_if(list);
                case KW_Switch: return expand_switch(list);
                case KW_SyntaxQuote: return expand_syntax_quote(list);
                case KW_ASTQuote: return expand_ast_quote(list);
                case KW_ASTUnquote: return expand_ast_unquote(list);
                case KW_ASTUnquoteArguments: return expand_ast_unquote_arguments(list);
                case KW_Return: return expand_return(list);
                case KW_Raise: return expand_raise(list);
                case KW_Break: return expand_break(list);
                case KW_Repeat: return expand_repeat(list);
                case KW_Merge: return expand_merge(list);
                case KW_Forward: return expand_forward(list);
                //case KW_Defer: return expand_defer(list);
                case KW_Do: return expand_do(list);
                case KW_DoIn: return expand_inline_do(list);
                case KW_Label: return expand_label(list);
                case KW_RawCall:
                case KW_Call: {
                    SCOPES_CHECK_RESULT(verify_list_parameter_count("special call", list, 1, -1));
                    list = list->next;
                    assert(list != EOL);
                    uint32_t flags = 0;
                    switch(func.value()) {
                    case KW_RawCall: flags |= CF_RawCall; break;
                    default: break;
                    }
                    return expand_call(list, flags);
                } break;
                default: break;
                }
            }

            Value *list_handler_node;
            if (env->lookup(Symbol(SYM_ListWildcard), list_handler_node)) {
                auto T = try_get_const_type(list_handler_node);
                if (T != list_expander_func_type) {
                    StyledString ss;
                    ss.out << "custom list expander has wrong type "
                        << T << ", must be " << list_expander_func_type;
                    SCOPES_LOCATION_ERROR(ss.str());
                }
                sc_syntax_wildcard_func_t f =
                    (sc_syntax_wildcard_func_t)cast<ConstPointer>(list_handler_node)->value;
                auto ok_result = f(List::from(node, next), env);
                if (!ok_result.ok) {
                    SCOPES_RETURN_ERROR(ok_result.except);
                }
                auto result = ok_result._0;
                if (result._0) {
                    Value *newnode = result._0->at;
                    if (newnode != node) {
                        anynode = newnode;
                        next = result._0->next;
                        env = result._1;
                        goto expand_again;
                    } else if (verbose) {
                        StyledStream ss(SCOPES_CERR);
                        ss << "ignored by list handler" << std::endl;
                    }
                }
            }
            SCOPES_ANCHOR(node->anchor());
            return expand_call(list);
        } else if (T == TYPE_Symbol) {
            if (verbose) {
                StyledStream ss(SCOPES_CERR);
                ss << "expanding symbol " << node << std::endl;
            }

            Symbol name = SCOPES_GET_RESULT(extract_symbol_constant(node));

            Value *result = nullptr;
            if (!env->lookup(name, result)) {
                sc_list_scope_tuple_t result = SCOPES_GET_RESULT(expand_symbol(node));
                if (result._0) {
                    Value *newnode = result._0->at;
                    if (newnode != node) {
                        anynode = newnode;
                        next = result._0->next;
                        env = result._1;
                        goto expand_again;
                    }
                }

                StyledString ss;
                ss.out << "use of undeclared identifier '" << name.name()->data << "'. ";
                print_name_suggestions(name, ss.out);
                SCOPES_LOCATION_ERROR(ss.str());
            }
            return result;
        } else {
            if (verbose) {
                StyledStream ss(SCOPES_CERR);
                ss << "ignoring " << node << std::endl;
            }
            return node;
        }
    }

};

bool Expander::verbose = false;
const Type *Expander::list_expander_func_type = nullptr;

SCOPES_RESULT(sc_value_list_scope_tuple_t) expand(Value *expr, const List *next, Scope *scope) {
    SCOPES_RESULT_TYPE(sc_value_list_scope_tuple_t);
    Scope *subenv = scope?scope:sc_get_globals();
    Expander subexpr(subenv, nullptr, next);
    Value *value = SCOPES_GET_RESULT(subexpr.expand(expr));
    sc_value_list_scope_tuple_t result = { value, subexpr.next, subexpr.env };
    return result;
}

SCOPES_RESULT(Template *) expand_inline(const Anchor *anchor, Template *astscope, const List *expr, Scope *scope) {
    SCOPES_RESULT_TYPE(Template *);
    Timer sum_expand_time(TIMER_Expand);
    //const Anchor *anchor = expr->anchor();
    //auto list = SCOPES_GET_RESULT(extract_list_constant(expr));
    assert(anchor);
    Template *mainfunc = Template::from(anchor, SYM_Unnamed);
    mainfunc->set_inline();

    Scope *subenv = scope?scope:sc_get_globals();
    Expander subexpr(subenv, astscope);
    mainfunc->value = SCOPES_GET_RESULT(subexpr.expand_expression(anchor, expr));

    return mainfunc;
}

SCOPES_RESULT(Template *) expand_module(const Anchor *anchor, const List *expr, Scope *scope) {
    SCOPES_RESULT_TYPE(Template *);
    Timer sum_expand_time(TIMER_Expand);
    //const Anchor *anchor = expr->anchor();
    //auto list = SCOPES_GET_RESULT(extract_list_constant(expr));
    assert(anchor);
    Template *mainfunc = Template::from(anchor, anchor->path());

    Scope *subenv = scope?scope:sc_get_globals();
    Expander subexpr(subenv, mainfunc);
    mainfunc->value = SCOPES_GET_RESULT(subexpr.expand_expression(anchor, expr));

    return mainfunc;
}

} // namespace scopes
