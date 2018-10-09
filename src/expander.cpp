/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "expander.hpp"
#include "list.hpp"
#include "error.hpp"
#include "type.hpp"
#include "arguments_type.hpp"
#include "pointer_type.hpp"
#include "function_type.hpp"
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
        return true;
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
    return true;
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
        if (expr)
            return expr;
        return ArgumentList::from(anchor);
    }

    SCOPES_RESULT(Value *) expand_compile_stage(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();

        SCOPES_CHECK_RESULT(verify_list_parameter_count("compile-stage", it, 0, 0));

        auto node = CompileStage::from(_anchor, next, env);
        next = EOL;
        return node;
    }

    SCOPES_RESULT(Parameter *) expand_parameter(Value *value, Value *node = nullptr) {
        SCOPES_RESULT_TYPE(Parameter *);
        const Anchor *anchor = value->anchor();
        if (isa<Parameter>(value)) {
            return cast<Parameter>(value);
        } else {
            Symbol sym = SCOPES_GET_RESULT(extract_symbol_constant(value));
            if (node && node->is_pure()) {
                env->bind(sym, node);
                return nullptr;
            } else {
                Parameter *param = nullptr;
                if (ends_with_parenthesis(sym)) {
                    param = Parameter::variadic_from(anchor, sym);
                } else {
                    param = Parameter::from(anchor, sym);
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
            Value *result = nullptr;
            if (env->lookup_local(sym, result)
                && isa<Template>(result)
                && cast<Template>(result)->is_forward_decl()) {
                func = cast<Template>(result);
                continuing = true;
            } else {
                func = Template::from(_anchor, sym);
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

        if (it == EOL) {
            // forward declaration
            if (T != TYPE_Symbol) {
                SCOPES_LOCATION_ERROR(
                    String::from("forward declared function must be named"));
            }
            return func;
        }

        // not a forward declaration
        func->scope = astscope;

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

        return func;
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

    SCOPES_RESULT(Value *) expand_do(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
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

    // (loop ([x ...]) [= args ...] body ...)
    SCOPES_RESULT(Value *) expand_loop(const List *it) {
        SCOPES_RESULT_TYPE(Value *);

        SCOPES_CHECK_RESULT(verify_list_parameter_count("loop", it, 1, -1));
        it = it->next;

        auto _anchor = get_active_anchor();

        const List *params = SCOPES_GET_RESULT(extract_list_constant(it->at));
        const List *values = nullptr;
        {
            auto nextit = it->next;
            if (nextit != EOL) {
                if (!is_equal_token(nextit->at)) {
                    SCOPES_LOCATION_ERROR(String::from("equal sign (=) expected"));
                }
                values = nextit->next;
            }
        }

        auto loop = Loop::from(_anchor);

        Parameters syms;
        Values exprs;

        it = values;
        // read init values
        Expander subexp(env, astscope);
        while (it) {
            subexp.next = it->next;
            exprs.push_back(SCOPES_GET_RESULT(subexp.expand(it->at)));
            it = subexp.next;
        }

        it = params;
        // read parameter names
        while (it != EOL) {
            syms.push_back(SCOPES_GET_RESULT(expand_parameter(it->at)));
            it = it->next;
        }

        loop->params = syms;
        loop->args = exprs;

        loop->value = SCOPES_GET_RESULT(expand_expression(_anchor, next));
        return loop;
    }

    static Value *extract_argument(const Anchor *anchor, Value *node, int index, bool vararg = false) {
        if (isa<Const>(node)) {
            assert(!is_arguments_type(node->get_type()));
            if (index == 0) {
                return node;
            } else {
                return ConstAggregate::none_from(anchor);
            }
        }
        return ExtractArgument::from(anchor, node, index, vararg);
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
                if (!node->is_pure())
                    exprs.push_back(node);
                it = subexp.next;
                if (it) {
                    SCOPES_ANCHOR(it->at->anchor());
                    SCOPES_LOCATION_ERROR(String::from("extraneous argument"));
                }

                Symbol sym = SCOPES_GET_RESULT(extract_symbol_constant(paramval));
                if (!ends_with_parenthesis(sym)) {
                    node = extract_argument(paramval->anchor(), node, 0);
                    exprs.push_back(node);
                }
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

            Values args;

            const List *params = it;

            it = values;
            it = it->next;
            // read init values
            Expander subexp(env, astscope);
            while (it) {
                subexp.next = it->next;
                auto node = SCOPES_GET_RESULT(subexp.expand(it->at));
                if (!node->is_pure())
                    exprs.push_back(node);
                args.push_back(node);
                it = subexp.next;
            }

            int index = 0;
            int lastarg = (int)args.size() - 1;
            it = params;
            // read parameter names
            while (it != endit) {
                auto paramval = it->at;
                Symbol sym = SCOPES_GET_RESULT(extract_symbol_constant(paramval));
                if (!ends_with_parenthesis(sym)) {
                    if (lastarg == -1) {
                        env->bind(sym, ConstAggregate::none_from(paramval->anchor()));
                    } else {
                        auto arg = args[std::min(lastarg, index)];
                        auto indexed = extract_argument(paramval->anchor(), arg, std::max(0, index - lastarg));
                        exprs.push_back(indexed);
                        env->bind(sym, indexed);
                    }
                } else {
                    if (it->next != endit) {
                        SCOPES_ANCHOR(paramval->anchor());
                        SCOPES_EXPECT_ERROR(error_variadic_symbol_not_in_last_place());
                    }
                    if (lastarg == -1) {
                        env->bind(sym, ArgumentList::from(_anchor));
                    } else if (index == lastarg) {
                        env->bind(sym, args[index]);
                    } else if (index < lastarg) {
                        auto arglist = ArgumentList::from(paramval->anchor());
                        for (int j = index; j <= lastarg; ++j) {
                            arglist->append(args[j]);
                        }
                        env->bind(sym, arglist);
                    } else {
                        int offset = index - lastarg;
                        env->bind(sym,
                            extract_argument(
                                paramval->anchor(), args[lastarg],
                                index - lastarg, true));
                    }
                }

                it = it->next;
                index++;
            }
        }

        if (exprs.empty())
            return ArgumentList::from(_anchor);
        auto result = Expression::from(_anchor, exprs, ArgumentList::from(_anchor));
        result->scoped = false;
        return result;
    }

    // quote <value> ...
    SCOPES_RESULT(Value *) expand_quote(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();

        SCOPES_CHECK_RESULT(verify_list_parameter_count("quote", it, 1, -1));
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

        SCOPES_CHECK_RESULT(verify_list_parameter_count("try", it, 0, -1));
        it = it->next;

        auto _anchor = get_active_anchor();

        Expander subexp(Scope::from(env), astscope);

        Value *try_value = SCOPES_GET_RESULT(subexp.expand_expression(_anchor, it));

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

                    Parameter* except_param = Parameter::from(it->at->anchor(), SYM_Unnamed);
                    Expander subexp(Scope::from(env), astscope);

                    Block exprs;

                    it = params;
                    int index = 0;
                    // read parameter names
                    while (it != EOL) {
                        auto paramval = it->at;
                        Symbol sym = SCOPES_GET_RESULT(extract_symbol_constant(paramval));
                        if (!ends_with_parenthesis(sym)) {
                            auto indexed = extract_argument(paramval->anchor(), except_param, index);
                            exprs.append(indexed);
                            subexp.env->bind(sym, indexed);
                        } else {
                            // not supported yet
                            SCOPES_ANCHOR(paramval->anchor());
                            SCOPES_EXPECT_ERROR(error_variadic_symbol_not_in_last_place());
                        }
                        it = it->next;
                        index++;
                    }

                    it = body;
                    Value *except_value = SCOPES_GET_RESULT(subexp.expand_expression(_anchor, it));

                    auto _try = Try::from(_anchor, try_value, except_param, except_value);
                    _try->except_body = exprs;
                    return _try;
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

        Switch::Cases cases;

        it = next;
    collect_case:
        if ((it == EOL) || (try_get_const_type(it->at) != TYPE_List)) {
            SCOPES_EXPECT_ERROR(error_missing_default_case());
        }
        next = it->next;
        auto _case = Switch::Case();
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
            _case.value = SCOPES_GET_RESULT(subexp.expand_expression(_case.anchor, it));
            cases.push_back(_case);

            it = next;
            goto collect_case;
        } else if (head == KW_Default) {
            it = it->next;

            _case.kind = CK_Default;

            Expander nativeexp(Scope::from(env), astscope);
            _case.value = SCOPES_GET_RESULT(subexp.expand_expression(_case.anchor, it));
            cases.push_back(_case);
        }

        return Switch::from(_anchor, expr, cases);
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
            ifexpr->append(anchor, cond,
                SCOPES_GET_RESULT(subexp.expand_expression(anchor, it)));
        }

        it = branches[lastidx];
        if (it != EOL) {
            const Anchor *anchor = it->at->anchor();
            it = it->next;
            Expander subexp(Scope::from(env), astscope);

            ifexpr->append(anchor,
                SCOPES_GET_RESULT(subexp.expand_expression(anchor, it)));
        }

        return ifexpr->canonicalize();
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
                    Keyed::from(get_active_anchor(), key,
                        SCOPES_GET_RESULT(expand(value))));
            } else {
                args.push_back(SCOPES_GET_RESULT(expand(it->at)));
            }
            it = next;
        }
        return true;
    }

    SCOPES_RESULT(Value *) expand_return(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();
        SCOPES_CHECK_RESULT(verify_list_parameter_count("return", it, 0, -1));
        it = it->next;
        ArgumentList *args = ArgumentList::from(_anchor);
        if (it) {
            Expander subexp(env, astscope, it->next);
            SCOPES_CHECK_RESULT(subexp.expand_arguments(args->values, it));
        }
        return Return::from(_anchor, args);
    }

    SCOPES_RESULT(Value *) expand_raise(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();
        SCOPES_CHECK_RESULT(verify_list_parameter_count("raise", it, 0, -1));
        it = it->next;
        ArgumentList *args = ArgumentList::from(_anchor);
        if (it) {
            Expander subexp(env, astscope, it->next);
            SCOPES_CHECK_RESULT(subexp.expand_arguments(args->values, it));
        }
        return Raise::from(_anchor, args);
    }

    SCOPES_RESULT(Value *) expand_break(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();
        SCOPES_CHECK_RESULT(verify_list_parameter_count("break", it, 0, -1));
        it = it->next;
        ArgumentList *args = ArgumentList::from(_anchor);
        if (it) {
            Expander subexp(env, astscope, it->next);
            SCOPES_CHECK_RESULT(subexp.expand_arguments(args->values, it));
        }
        return Break::from(_anchor, args);
    }

    SCOPES_RESULT(Value *) expand_repeat(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();
        SCOPES_CHECK_RESULT(verify_list_parameter_count("repeat", it, 0, -1));
        it = it->next;
        auto rep = Repeat::from(_anchor);
        if (it) {
            Expander subexp(env, astscope, it->next);
            SCOPES_CHECK_RESULT(subexp.expand_arguments(rep->args, it));
        }
        return rep;
    }

    SCOPES_RESULT(Value *) expand_forward(const List *it) {
        SCOPES_RESULT_TYPE(Value *);
        auto _anchor = get_active_anchor();
        SCOPES_CHECK_RESULT(verify_list_parameter_count("_", it, 0, -1));
        it = it->next;
        auto args = ArgumentList::from(_anchor);
        if (it) {
            Expander subexp(env, astscope, it->next);
            SCOPES_CHECK_RESULT(subexp.expand_arguments(args->values, it));
        }
        return args;
    }

    SCOPES_RESULT(Value *) expand_call(const List *it, uint32_t flags = 0) {
        SCOPES_RESULT_TYPE(Value *);

        auto _anchor = get_active_anchor();
        SCOPES_CHECK_RESULT(verify_list_parameter_count("call", it, 0, -1));
        Expander subexp(env, astscope, it->next);
        Value *enter = SCOPES_GET_RESULT(subexp.expand(it->at));

        auto call = Call::from(_anchor, enter);
        call->flags = flags;

        it = subexp.next;
        SCOPES_CHECK_RESULT(subexp.expand_arguments(call->args, it));
        return call;
    }

    struct ListScopePair { const List *topit; Scope *env; };

    SCOPES_TYPEDEF_RESULT_RAISES(sc_list_scope_raises, ListScopePair);
    typedef sc_list_scope_raises_t (*HandlerFuncType)(const List *, Scope *);

    SCOPES_RESULT(Value *) expand(Value *node) {
        SCOPES_RESULT_TYPE(Value *);
    expand_again:
        SCOPES_CHECK_RESULT(verify_stack());
        SCOPES_ANCHOR(node->anchor());
        if (!isa<Const>(node)) {
            if (verbose) {
                StyledStream ss(SCOPES_CERR);
                ss << "ignoring " << node << std::endl;
            }
            // return as-is
            return node;
        }
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
                case KW_Fn: {
                    return expand_fn(list, ExpandFnSetup());
                }
                case KW_Inline: {
                    ExpandFnSetup setup;
                    setup.inlined = true;
                    return expand_fn(list, setup);
                }
                case KW_CompileStage: return expand_compile_stage(list);
                case KW_Let: return expand_let(list);
                case KW_Loop: return expand_loop(list);
                case KW_Try: return expand_try(list);
                case KW_If: return expand_if(list);
                case KW_Switch: return expand_switch(list);
                case KW_Quote: return expand_quote(list);
                case KW_Return: return expand_return(list);
                case KW_Raise: return expand_raise(list);
                case KW_Break: return expand_break(list);
                case KW_Repeat: return expand_repeat(list);
                case KW_Forward: return expand_forward(list);
                //case KW_Defer: return expand_defer(list);
                case KW_Do: return expand_do(list);
                case KW_DoIn: return expand_inline_do(list);
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
                HandlerFuncType f = (HandlerFuncType)cast<ConstPointer>(list_handler_node)->value;
                auto ok_result = f(List::from(node, next), env);
                if (!ok_result.ok) {
                    set_last_error(ok_result.except);
                    SCOPES_RETURN_ERROR();
                }
                auto result = ok_result._0;
                if (result.topit) {
                    Value *newnode = result.topit->at;
                    if (newnode != node) {
                        node = newnode;
                        next = result.topit->next;
                        env = result.env;
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
                Value *symbol_handler_node;
                if (env->lookup(Symbol(SYM_SymbolWildcard), symbol_handler_node)) {
                    auto T = try_get_const_type(symbol_handler_node);
                    if (T != list_expander_func_type) {
                        StyledString ss;
                        ss.out << "custom symbol expander has wrong type "
                            << T << ", must be " << list_expander_func_type;
                        SCOPES_LOCATION_ERROR(ss.str());
                    }
                    HandlerFuncType f = (HandlerFuncType)cast<ConstPointer>(symbol_handler_node)->value;
                    auto ok_result = f(List::from(node, next), env);
                    if (!ok_result.ok) {
                        set_last_error(ok_result.except);
                        SCOPES_RETURN_ERROR();
                    }
                    auto result = ok_result._0;
                    if (result.topit) {
                        Value *newnode = result.topit->at;
                        if (newnode != node) {
                            node = newnode;
                            next = result.topit->next;
                            env = result.env;
                            goto expand_again;
                        }
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

SCOPES_RESULT(Value *) expand(Value *expr, Scope *scope) {
    SCOPES_RESULT_TYPE(Value *);
    Scope *subenv = scope?scope:sc_get_globals();
    Expander subexpr(subenv, nullptr);
    return subexpr.expand(expr);
}

SCOPES_RESULT(Template *) expand_inline(const Anchor *anchor, Template *astscope, const List *expr, Scope *scope) {
    SCOPES_RESULT_TYPE(Template *);
    Timer sum_expand_time(TIMER_Expand);
    //const Anchor *anchor = expr->anchor();
    //auto list = SCOPES_GET_RESULT(extract_list_constant(expr));
    assert(anchor);
    Template *mainfunc = Template::from(anchor, SYM_Unnamed);
    mainfunc->set_inline();
    mainfunc->scope = astscope;

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
