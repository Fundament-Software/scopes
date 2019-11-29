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
#include "stream_expr.hpp"
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
        SCOPES_ERROR(SyntaxCallExpressionEmpty);
    }
    if ((mincount <= 0) && (maxcount == -1)) {
        return {};
    }
    int argcount = (int)List::count(expr) - starti;

    if ((maxcount >= 0) && (argcount > maxcount)) {
        SCOPES_ERROR(SyntaxTooManyArguments, maxcount);
    }
    if ((mincount >= 0) && (argcount < mincount)) {
        SCOPES_ERROR(SyntaxNotEnoughArguments, mincount, argcount);
    }
    return {};
}

//------------------------------------------------------------------------------

static Symbol try_extract_symbol(const ValueRef &node) {
    auto ptr = node.dyn_cast<ConstInt>();
    if (ptr && (ptr->get_type() == TYPE_Symbol))
        return Symbol::wrap(ptr->value());
    return SYM_Unnamed;
}

//------------------------------------------------------------------------------

struct Expander {
    const Scope *env;
    const String *next_doc;
    TemplateRef astscope;
    const List *next;
    static bool verbose;

    static const Type *list_expander_func_type;

    Expander(const Scope *_env, const TemplateRef &_astscope, const List *_next = EOL) :
        env(_env),
        next_doc(nullptr),
        astscope(_astscope),
        next(_next) {
        if (!list_expander_func_type) {
            list_expander_func_type = pointer_type(raising_function_type(
                arguments_type({TYPE_List, TYPE_Scope}),
                {TYPE_List, TYPE_Scope}), PTF_NonWritable, SYM_Unnamed);
        }
    }

    ~Expander() {}

    SCOPES_RESULT(ValueRef) expand_expression(const ListRef &src, bool scoped) {
        SCOPES_RESULT_TYPE(ValueRef);
        ExpressionRef expr;
        const List *it = src.unref();
        while (it) {
            next = it->next;
            if (!last_expression()) {
                const String *doc = try_extract_string(it->at);
                if (doc) {
                    next_doc = doc;
                }
            }
            if (!expr) {
                if (scoped)
                    expr = ref(src.anchor(), Expression::scoped_from());
                else
                    expr = ref(src.anchor(), Expression::unscoped_from());
            }
            expr->append(SCOPES_GET_RESULT(expand(it->at)));
            it = next;
        }
        if (expr) {
            if (expr->body.empty() && !expr->scoped)
                return expr->value;
            return ValueRef(expr);
        }
        return ref(src.anchor(), ArgumentListTemplate::from());
    }

    SCOPES_RESULT(ValueRef) expand_run_stage(const List *it) {
        SCOPES_RESULT_TYPE(ValueRef);
        SCOPES_CHECK_RESULT(verify_list_parameter_count("run-stage", it, 0, 0));
        assert(it);

        auto anchor = it->at.anchor();
        auto node = ref(anchor, CompileStage::from(anchor, next, env));
        next = EOL;
        return ValueRef(node);
    }

    void bind(const ConstRef &name, const ValueRef &value, const String *doc) {
        env = Scope::bind_from(name, value, doc, env);
    }

    void bind(const ConstRef &name, const ValueRef &value) {
        const String *doc = nullptr;
        if (next_doc) {
            doc = next_doc;
            next_doc = nullptr;
        }
        bind(name, value, doc);
    }

    SCOPES_RESULT(ParameterTemplateRef) expand_parameter(
        const ValueRef &value, const ValueRef &node = ValueRef()) {
        SCOPES_RESULT_TYPE(ParameterTemplateRef);
        if (value.isa<ParameterTemplate>()) {
            return value.cast<ParameterTemplate>();
        } else {
            Symbol sym = SCOPES_GET_RESULT(extract_symbol_constant(value));
            if (node && node.isa<Pure>()) {
                bind(ConstInt::symbol_from(sym), node);
                return ParameterTemplateRef();
            } else {
                ParameterTemplateRef param;
                if (ends_with_parenthesis(sym)) {
                    param = ref(value.anchor(), ParameterTemplate::variadic_from(sym));
                } else {
                    param = ref(value.anchor(), ParameterTemplate::from(sym));
                }
                bind(ConstInt::symbol_from(sym), param);
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

    SCOPES_RESULT(ValueRef) expand_fn(const List *it, const ExpandFnSetup &setup) {
        SCOPES_RESULT_TYPE(ValueRef);
        SCOPES_CHECK_RESULT(verify_list_parameter_count("fn", it, 1, -1));

        auto anchor = it->at.anchor();

        // skip head
        it = it->next;

        assert(it != EOL);

        bool continuing = false;
        Symbol bind_name = SYM_Unnamed;
        TemplateRef func;
        ValueRef result;
        //Any tryfunc_name = SCOPES_GET_RESULT(unsyntax(it->at));
        const Type *T = try_get_const_type(it->at);
        if (T == TYPE_Symbol) {
            auto sym = SCOPES_GET_RESULT(extract_symbol_constant(it->at));
            // named self-binding
            // see if we can find a forward declaration in the local scope
            if (env->lookup_local(ConstInt::symbol_from(sym), result)
                && result.isa<Template>()
                && result.cast<Template>()->is_forward_decl()) {
                func = result.cast<Template>();
                func = ref(anchor, func);
                continuing = true;
            } else {
                func = ref(anchor, Template::from(sym));
                result = func;
                bind_name = sym;
            }
            it = it->next;
        } else if (T == TYPE_String) {
            auto str = try_extract_string(it->at);
            assert(str);
            // named lambda
            func = ref(anchor, Template::from(Symbol(str)));
            result = func;
            it = it->next;
        } else {
            // unnamed lambda
            func = ref(anchor, Template::from(Symbol(SYM_Unnamed)));
            result = func;
        }
        if (setup.inlined)
            func->set_inline();
        func->set_def_anchor(anchor);
        /*
        if (setup.quoted)
            result = ast_quote(func);
        */

        if (it == EOL) {
            // forward declaration
            if (bind_name == SYM_Unnamed) {
                SCOPES_ERROR(SyntaxUnnamedForwardDeclaration);
            }
            bind(ConstInt::symbol_from(bind_name), func);
            return result;
        }

        const List *params = SCOPES_GET_RESULT(extract_list_constant(it->at));

        it = it->next;

        const Scope *subenv = Scope::from(nullptr, env);
        if (!func->is_hidden()) {
            subenv = Scope::bind_from(ConstInt::symbol_from(KW_Recur), func, nullptr, subenv);
            // ensure the local scope does not contain special symbols
            subenv = Scope::from(nullptr, subenv);
        }

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

        func->value = SCOPES_GET_RESULT(subexpr.expand_expression(
            ref(anchor, it), false));
        if (bind_name != SYM_Unnamed) {
            bind(ConstInt::symbol_from(bind_name), func);
        }

        return result;
    }

    bool last_expression() {
        return next == EOL;
    }

    SCOPES_RESULT(ValueRef) expand_label(const List *it) {
        SCOPES_RESULT_TYPE(ValueRef);
        it = it->next;

        auto name = SCOPES_GET_RESULT(extract_symbol_constant(it->at));
        auto anchor = it->at.anchor();
        it = it->next;

        const Scope *subenv = Scope::from(nullptr, env);
        auto label = ref(anchor, LabelTemplate::from(LK_User, name));
        Expander subexpr(subenv, astscope);
        subexpr.bind(ConstInt::symbol_from(name), label);
        label->value = SCOPES_GET_RESULT(subexpr.expand_expression(
            ref(anchor, it), false));
        return ValueRef(label);
    }

    SCOPES_RESULT(ValueRef) expand_do(const List *it) {
        //SCOPES_RESULT_TYPE(ValueRef);
        assert(it);
        auto anchor = it->at.anchor();
        it = it->next;

        const Scope *subenv = Scope::from(nullptr, env);
        Expander subexpr(subenv, astscope);
        return subexpr.expand_expression(ref(anchor, it), true);
    }

    SCOPES_RESULT(ValueRef) expand_inline_do(const List *it) {
        SCOPES_RESULT_TYPE(ValueRef);
        auto anchor = it->at.anchor();
        it = it->next;

        Expander subexpr(env, astscope);
        auto expr = SCOPES_GET_RESULT(
            subexpr.expand_expression(ref(anchor, it), false));
        env = subexpr.env;
        return expr;
    }

    bool is_then_token(const ValueRef &name) {
        auto tok = try_extract_symbol(name);
        return tok == KW_Then;
    }

    bool is_equal_token(const ValueRef &name) {
        auto tok = try_extract_symbol(name);
        return tok == OP_Set;
    }

    bool is_except_token(const ValueRef &name) {
        auto tok = try_extract_symbol(name);
        return tok == KW_Except;
    }

    // (loop ([x...] [= init...]) body...)
    SCOPES_RESULT(ValueRef) expand_loop(const List *it) {
        SCOPES_RESULT_TYPE(ValueRef);

        /*
        we're building this structure

        label break
            loop x... (init...)
                body...

        break labels never type themselves from their body
        */

        SCOPES_CHECK_RESULT(verify_list_parameter_count("loop", it, 1, -1));
        auto anchor = it->at.anchor();
        it = it->next;

        const List *params = SCOPES_GET_RESULT(extract_list_constant(it->at));
        auto param_anchor = it->at.anchor();
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

        LoopRef loop = ref(anchor, Loop::from(ref(param_anchor,
            ArgumentListTemplate::from(initargs))));
        LabelTemplateRef break_label = ref(anchor,
            LabelTemplate::from(LK_Break, KW_Break, loop));

        Expander bodyexp(Scope::from(nullptr, env), astscope);

        auto expr = Expression::unscoped_from();
        {
            int index = 0;
            it = params;
            // read parameter names
            while (it != endit) {
                auto paramval = it->at;
                Symbol sym = SCOPES_GET_RESULT(extract_symbol_constant(paramval));
                ValueRef node;
                if (!ends_with_parenthesis(sym)) {
                    node = ref(paramval.anchor(), extract_argument(loop->args, index));
                } else {
                    if (it->next != endit) {
                        SCOPES_ERROR(SyntaxVariadicSymbolNotLast);
                    }
                    node = ref(paramval.anchor(), extract_argument(loop->args, index, true));
                }
                bodyexp.bind(ConstInt::symbol_from(sym), node);
                expr->append(node);
                it = it->next;
                index++;
            }
        }

        auto value = SCOPES_GET_RESULT(bodyexp.expand_expression(
            ref(anchor, body), false));
        expr->append(value);
        loop->value = ref(anchor, expr);
        return ValueRef(break_label);
    }

    static ValueRef extract_argument(const ValueRef &node, int index, bool vararg = false) {
        if (node.isa<Const>()) {
            assert(!is_arguments_type(node.cast<Const>()->get_type()));
            if (index == 0) {
                return node;
            } else {
                return ref(node.anchor(), ConstAggregate::none_from());
            }
        }
        auto result = ExtractArgumentTemplate::from(node, index, vararg);
        if (!vararg) {
            result = KeyedTemplate::from(SYM_Unnamed, result);
        }
        return result;
    }

    // (let x ... [= args ...])
    // (let (x ... = args ...) ...
    // ...
    SCOPES_RESULT(ValueRef) expand_let(const List *it, bool indirect = false) {
        SCOPES_RESULT_TYPE(ValueRef);

        SCOPES_CHECK_RESULT(verify_list_parameter_count("let", it, 1, -1));
        auto anchor = it->at.anchor();
        it = it->next;

        Values exprs;
        Values args;

        const Type *T = try_get_const_type(it->at);
        if (T == TYPE_List) {
            // alternative format where the symbols are only bound
            // after all expressions have been evaluated.
            std::vector< std::pair<Symbol, ValueRef> > late_bound;
            late_bound.reserve(List::count(it));
            const List *equit = it;
            while (equit) {
                it = SCOPES_GET_RESULT(extract_list_constant(equit->at));
                SCOPES_CHECK_RESULT(verify_list_parameter_count("=", it, 3, -1, 0));
                auto paramval = it->at;
                it = it->next;
                if (!is_equal_token(it->at)) {
                    SCOPES_ERROR(SyntaxAssignmentTokenExpected);
                }
                it = it->next;
                // read init values
                Expander subexp(env, astscope);
                subexp.next = it->next;
                auto node = SCOPES_GET_RESULT(subexp.expand(it->at));
                it = subexp.next;
                env = subexp.env;
                if (it) {
                    SCOPES_ERROR(SyntaxUnexpectedExtraToken);
                }
                exprs.push_back(node);
                Symbol sym = SCOPES_GET_RESULT(extract_symbol_constant(paramval));
                if (indirect) {
                    ValueRef value;
                    if (env->lookup(paramval.cast<Const>(), value)) {
                        sym = SCOPES_GET_RESULT(extract_symbol_constant(value));
                    }
                }
                if (!ends_with_parenthesis(sym)) {
                    node = extract_argument(node, 0);
                }
                args.push_back(node);
                late_bound.push_back({sym, node});
                equit = equit->next;
            }
            for (auto entry : late_bound) {
                bind(ConstInt::symbol_from(entry.first), entry.second);
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
                ValueRef last_entry;
                while (it != endit) {
                    auto name = SCOPES_GET_RESULT(extract_symbol_constant(it->at));
                    if (indirect) {
                        ValueRef value;
                        if (env->lookup(it->at.cast<Const>(), value)) {
                            name = SCOPES_GET_RESULT(extract_symbol_constant(value));
                        }
                    }
                    auto _name = ConstInt::symbol_from(name);
                    ValueRef value; const String *doc;
                    if (!env->lookup(_name, value, doc)) {
                        SCOPES_ERROR(SyntaxUndeclaredIdentifier, name, env);
                    }
                    if (!doc) {
                        bind(_name, value);
                    } else {
                        bind(_name, value, doc);
                    }
                    last_entry = value;
                    it = it->next;
                }
                if (!last_entry) {
                    last_entry = ref(anchor, ConstAggregate::none_from());
                }
                return ref(anchor, last_entry);
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

            ValueRef srcval = ref(anchor, ArgumentListTemplate::from(exprs));

            int index = 0;
            bool variadic = false;
            //int lastarg = (int)args.size() - 1;
            it = params;
            // read parameter names
            while (it != endit) {
                auto paramval = it->at;
                Symbol sym = SCOPES_GET_RESULT(extract_symbol_constant(paramval));
                if (indirect) {
                    ValueRef value;
                    if (env->lookup(paramval.cast<Const>(), value)) {
                        sym = SCOPES_GET_RESULT(extract_symbol_constant(value));
                    }
                }
                ValueRef node;
                if (!ends_with_parenthesis(sym)) {
                    node = ref(paramval.anchor(), extract_argument(srcval, index));
                } else {
                    if (it->next != endit) {
                        SCOPES_ERROR(SyntaxVariadicSymbolNotLast);
                    }
                    node = ref(paramval.anchor(), extract_argument(srcval, index, true));
                    variadic = true;
                }
                args.push_back(node);
                bind(ConstInt::symbol_from(sym), node);
                it = it->next;
                index++;
            }

            if (!variadic && (index < exprs.size())) {
                SCOPES_TRACE_EXPANDER(exprs[index]);
                SCOPES_ERROR(SyntaxExcessBindingArgument);
            }

        }

        return ValueRef(ref(anchor, Expression::unscoped_from(exprs,
            ref(anchor, ArgumentListTemplate::from(args)))));
    }

    // quote <value> ...
    SCOPES_RESULT(ValueRef) expand_syntax_quote(const List *it) {
        SCOPES_RESULT_TYPE(ValueRef);
        SCOPES_CHECK_RESULT(verify_list_parameter_count("syntax-quote", it, 1, -1));
        auto anchor = it->at.anchor();
        it = it->next;

        if (List::count(it) == 1) {
            return it->at;
        } else {
            return ValueRef(ref(anchor, ConstPointer::list_from(it)));
        }
    }

    SCOPES_RESULT(ValueRef) expand_syntax_log(const List *it) {
        SCOPES_RESULT_TYPE(ValueRef);
        SCOPES_CHECK_RESULT(verify_list_parameter_count("syntax-log", it, 1, 1));
        auto anchor = it->at.anchor();
        it = it->next;

        auto sym = SCOPES_GET_RESULT(extract_symbol_constant(it->at));
        if (sym == KW_True) {
            this->verbose = true;
        } else if (sym == KW_False) {
            this->verbose = false;
        } else {
            // ignore
        }

        return ValueRef(ref(anchor, ArgumentList::from({})));
    }

    // (try [x ...]) (except (param ...) [expr ...])
    SCOPES_RESULT(ValueRef) expand_try(const List *it) {
        SCOPES_RESULT_TYPE(ValueRef);

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
        auto try_anchor = it->at.anchor();
        //ValueRef src = it->at;
        it = it->next;

        Expander subexp(Scope::from(nullptr, env), astscope);

        ValueRef try_value = SCOPES_GET_RESULT(subexp.expand_expression(
            ref(try_anchor, it), false));
        LabelTemplateRef try_label = ref(try_anchor, LabelTemplate::try_from());
        LabelTemplateRef except_label = ref(try_anchor, LabelTemplate::except_from(
            ref(try_anchor, MergeTemplate::from(try_label, try_value))));

        if (next != EOL) {
            //auto _next_def = next->at;
            it = SCOPES_GET_RESULT(extract_list_constant(next->at));
            if (it != EOL) {
                if (is_except_token(it->at)) {
                    SCOPES_CHECK_RESULT(verify_list_parameter_count("except", it, 1, -1));
                    auto except_anchor = it->at.anchor();
                    except_label = ref(except_anchor, except_label);
                    it = it->next;
                    next = next->next;

                    const List *params = SCOPES_GET_RESULT(extract_list_constant(it->at));
                    const List *body = it->next;

                    Expander subexp(Scope::from(nullptr, env), astscope);
                    auto expr = Expression::unscoped_from();
                    expr->append(except_label);
                    {
                        int index = 0;
                        it = params;
                        // read parameter names
                        while (it != EOL) {
                            auto paramval = it->at;
                            Symbol sym = SCOPES_GET_RESULT(extract_symbol_constant(paramval));
                            ValueRef node;
                            if (!ends_with_parenthesis(sym)) {
                                node = ref(paramval.anchor(), extract_argument(except_label, index));
                            } else {
                                if (it->next != EOL) {
                                    SCOPES_ERROR(SyntaxVariadicSymbolNotLast);
                                }
                                node = ref(paramval.anchor(), extract_argument(except_label, index, true));
                            }
                            subexp.bind(ConstInt::symbol_from(sym), node);
                            expr->append(node);
                            it = it->next;
                            index++;
                        }
                    }

                    it = body;
                    ValueRef except_value = SCOPES_GET_RESULT(
                        subexp.expand_expression(ref(except_anchor, it), false));
                    expr->append(except_value);
                    try_label->value = ref(except_anchor, expr);
                    return ValueRef(try_label);
                }
            }
        }
        SCOPES_ERROR(SyntaxExceptBlockExpected);
    }

    // (switch cond)
    // [(case literal body ...)]
    // (default body ...)
    SCOPES_RESULT(ValueRef) expand_switch(const List *it) {
        SCOPES_RESULT_TYPE(ValueRef);
        SCOPES_CHECK_RESULT(verify_list_parameter_count("switch", it, 1, -1));
        auto anchor = it->at.anchor();
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
            SCOPES_ERROR(SyntaxMissingDefaultCase);
        }
        next = it->next;
        auto _case = SwitchTemplate::Case();
        _case.anchor = it->at.anchor();
        it = SCOPES_GET_RESULT(extract_list_constant(it->at));
        if (it == EOL) {
            SCOPES_ERROR(SyntaxCaseBlockExpected);
        }
        auto case_anchor = it->at.anchor();
        auto head = try_extract_symbol(it->at);
        if ((head == KW_Case) || (head == KW_Pass)) {
            SCOPES_CHECK_RESULT(verify_list_parameter_count("case", it, 1, -1));
            if (head == KW_Pass) {
                _case.kind = CK_Pass;
            }

            it = it->next;
            subexp.next = it->next;
            _case.literal = SCOPES_GET_RESULT(subexp.expand(it->at));
            it = subexp.next;

            Expander nativeexp(Scope::from(nullptr, env), astscope);
            _case.value = SCOPES_GET_RESULT(
                nativeexp.expand_expression(ref(case_anchor, it), false));
            cases.push_back(_case);

            it = next;
            goto collect_case;
        } else if (head == KW_Do) {
            it = it->next;

            _case.kind = CK_Do;

            Expander nativeexp(Scope::from(nullptr, env), astscope);
            _case.value = SCOPES_GET_RESULT(
                nativeexp.expand_expression(ref(case_anchor, it), false));
            cases.push_back(_case);

            it = next;
            goto collect_case;
        } else if (head == KW_Default) {
            it = it->next;

            if (head == KW_Do) {
                _case.kind = CK_Do;
            } else {
                _case.kind = CK_Default;
            }

            Expander nativeexp(Scope::from(nullptr, env), astscope);
            _case.value = SCOPES_GET_RESULT(
                nativeexp.expand_expression(ref(case_anchor, it), false));
            cases.push_back(_case);
        }

        return ValueRef(ref(anchor, SwitchTemplate::from(expr, cases)));
    }

    // (if cond body ...)
    // [(elseif cond body ...)]
    // [(else body ...)]
    SCOPES_RESULT(ValueRef) expand_if(const List *it) {
        SCOPES_RESULT_TYPE(ValueRef);
        std::vector<const List *> branches;

        assert(it);
        auto anchor = it->at.anchor();

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

        auto ifexpr = If::from();

        int lastidx = (int)branches.size() - 1;
        for (int idx = 0; idx < lastidx; ++idx) {
            it = branches[idx];
            //const Anchor *anchor = it->at->anchor();
            SCOPES_CHECK_RESULT(verify_list_parameter_count("branch", it, 1, -1));
            auto branch_anchor = it->at.anchor();

            #if 0
            // check for new form
            auto new_form = false;
            {
                auto subit = it->next;
                while (subit != EOL) {
                    if (is_then_token(subit->at)) {
                        SCOPES_CHECK_RESULT(verify_list_parameter_count(
                            "branch", it, 2, -1));
                        new_form = true;
                        break;
                    }
                    subit = subit->next;
                }
            }
            #endif
            it = it->next;

            {
                Expander subexp(env, astscope);
                assert(it);
                subexp.next = it->next;
                ValueRef cond = SCOPES_GET_RESULT(subexp.expand(it->at));
                it = subexp.next;
                env = subexp.env;

                subexp.env = Scope::from(nullptr, env);
                ifexpr->append_then(cond,
                    SCOPES_GET_RESULT(
                        subexp.expand_expression(ref(branch_anchor, it), false)));
            }
        }

        it = branches[lastidx];
        if (it != EOL) {
            auto else_anchor = it->at.anchor();
            it = it->next;
            Expander subexp(Scope::from(nullptr, env), astscope);

            ifexpr->append_else(
                SCOPES_GET_RESULT(
                    subexp.expand_expression(ref(else_anchor, it), false)));
        }

        return ValueRef(ref(anchor, ifexpr));
    }

    static SCOPES_RESULT(bool) get_kwargs(
        ValueRef it, Symbol &key, ValueRef &value) {
        SCOPES_RESULT_TYPE(bool);
        auto T = try_get_const_type(it);
        if (T != TYPE_List) return false;
        auto l = SCOPES_GET_RESULT(extract_list_constant(it));
        if (l == EOL) return false;
        it = l->at;
        T = try_get_const_type(it);
        if (T != TYPE_Symbol) return false;
        key = SCOPES_GET_RESULT(extract_symbol_constant(it));
        l = l->next;
        if (l == EOL) return false;
        it = l->at;
        T = try_get_const_type(it);
        if (T != TYPE_Symbol) return false;
        auto sym = SCOPES_GET_RESULT(extract_symbol_constant(it));
        if (sym != OP_Set) return false;
        l = l->next;
        if (l == EOL) return false;
        if (List::count(l) > 1) {
            SCOPES_TRACE_EXPANDER(it);
            SCOPES_ERROR(SyntaxKeyedArgumentMismatch);
        }
        value = l->at;
        return true;
    }

    SCOPES_RESULT(void) expand_arguments(Values &args, const List *it) {
        SCOPES_RESULT_TYPE(void);
        while (it) {
            next = it->next;
            Symbol key = SYM_Unnamed;
            ValueRef value;
            //SCOPES_ANCHOR(it->at->anchor());
            if (SCOPES_GET_RESULT(get_kwargs(it->at, key, value))) {
                args.push_back(
                    KeyedTemplate::from(key,
                        SCOPES_GET_RESULT(expand(value))));
            } else {
                args.push_back(SCOPES_GET_RESULT(expand(it->at)));
            }
            it = next;
        }
        return {};
    }

    ValueRef ast_quote(const ValueRef &expr) {
        if (expr.isa<Expression>()) {
            auto ex = expr.cast<Expression>();
            ex->scoped = false;
        }
        return ref(expr.anchor(), Quote::from(expr));
    }

    SCOPES_RESULT(ValueRef) expand_ast_quote(const List *it) {
        SCOPES_RESULT_TYPE(ValueRef);
        auto anchor = it->at.anchor();
        it = it->next;

        Expander subexpr(env, astscope);
        auto expr = SCOPES_GET_RESULT(subexpr.expand_expression(
            ref(anchor, it), false));
        env = subexpr.env;
        return ref(anchor, ast_quote(expr));
    }

    SCOPES_RESULT(ValueRef) expand_ast_unquote(const List *it) {
        SCOPES_RESULT_TYPE(ValueRef);
        SCOPES_CHECK_RESULT(verify_list_parameter_count("ast-unquote", it, 0, -1));
        auto anchor = it->at.anchor();
        it = it->next;
        Expander subexp(env, astscope);
        auto expr = ValueRef(ref(anchor, Unquote::from(
            SCOPES_GET_RESULT(subexp.expand_expression(
                ref(anchor, it), false)))));
        env = subexp.env;
        return expr;
    }

    SCOPES_RESULT(ValueRef) expand_ast_unquote_arguments(const List *it) {
        SCOPES_RESULT_TYPE(ValueRef);
        SCOPES_CHECK_RESULT(verify_list_parameter_count("ast-unquote-arguments", it, 0, -1));
        auto anchor = it->at.anchor();
        it = it->next;
        Values args;
        if (it) {
            Expander subexp(env, astscope, it->next);
            SCOPES_CHECK_RESULT(subexp.expand_arguments(args, it));
            env = subexp.env;
        }
        return ValueRef(ref(anchor, Unquote::from(
            ref(anchor, ArgumentListTemplate::from(args)))));
    }

    template<typename T>
    SCOPES_RESULT(ValueRef) build_terminator(const ListRef &src) {
        SCOPES_RESULT_TYPE(ValueRef);
        auto it = src.unref();
        Values args;
        if (it) {
            Expander subexp(env, astscope, it->next);
            SCOPES_CHECK_RESULT(subexp.expand_arguments(args, it));
            env = subexp.env;
        }
        return ValueRef(ref(src.anchor(), T::from(ref(src.anchor(),
            ArgumentListTemplate::from(args)))));
    }

    template<typename T>
    SCOPES_RESULT(ValueRef) build_terminator(const ListRef &src,
        const LabelTemplateRef &label) {
        SCOPES_RESULT_TYPE(ValueRef);
        auto it = src.unref();
        Values args;
        if (it) {
            Expander subexp(env, astscope, it->next);
            SCOPES_CHECK_RESULT(subexp.expand_arguments(args, it));
        }
        return ValueRef(ref(src.anchor(), T::from(label, ref(src.anchor(),
                ArgumentListTemplate::from(args)))));
    }

    SCOPES_RESULT(ValueRef) expand_merge(const List *it) {
        SCOPES_RESULT_TYPE(ValueRef);
        SCOPES_CHECK_RESULT(verify_list_parameter_count("merge", it, 1, -1));
        auto anchor = it->at.anchor();
        it = it->next;

        Expander subexp(Scope::from(nullptr, env), astscope, it->next);
        ValueRef label = SCOPES_GET_RESULT(subexp.expand(it->at));
        it = subexp.next;

        if (!label.isa<LabelTemplate>()) {
            SCOPES_ERROR(SyntaxLabelExpected, label->kind());
        }
        return build_terminator<MergeTemplate>(ref(anchor, it),
            label.cast<LabelTemplate>());
    }

    SCOPES_RESULT(ValueRef) expand_forward(const List *it) {
        SCOPES_RESULT_TYPE(ValueRef);
        SCOPES_CHECK_RESULT(verify_list_parameter_count("_", it, 0, -1));
        auto anchor = it->at.anchor();
        it = it->next;
        Values args;
        if (it) {
            Expander subexp(env, astscope, it->next);
            SCOPES_CHECK_RESULT(subexp.expand_arguments(args, it));
        }
        return ref(anchor, ArgumentListTemplate::from(args));
    }

    SCOPES_RESULT(ValueRef) expand_call(const List *it, uint32_t flags = 0) {
        SCOPES_RESULT_TYPE(ValueRef);

        SCOPES_CHECK_RESULT(verify_list_parameter_count("call", it, 0, -1));
        auto anchor = it->at.anchor();
        Expander subexp(env, astscope, it->next);
        ValueRef enter = SCOPES_GET_RESULT(subexp.expand(it->at));

        auto call = CallTemplate::from(enter);
        call->flags = flags;
        //call->set_def_anchor(anchor);

        it = subexp.next;
        SCOPES_CHECK_RESULT(subexp.expand_arguments(call->args, it));
        env = subexp.env;
        return ValueRef(ref(anchor, call));
    }

    SCOPES_RESULT(sc_list_scope_tuple_t) expand_symbol(const ConstRef &node) {
        SCOPES_RESULT_TYPE(sc_list_scope_tuple_t);
        ValueRef symbol_handler_node;
        const String *doc;
        if (env->lookup(ConstInt::symbol_from(SYM_SymbolWildcard), symbol_handler_node, doc)) {
            auto T = try_get_const_type(symbol_handler_node);
            if (T != list_expander_func_type) {
                SCOPES_TRACE_HOOK(symbol_handler_node);
                // symbol_handler_node.cast<TypedValue>()->get_type()
                SCOPES_ERROR(SyntaxSymbolExpanderTypeMismatch, T,
                    list_expander_func_type);
            }
            sc_syntax_wildcard_func_t f =
                (sc_syntax_wildcard_func_t)symbol_handler_node.cast<ConstPointer>()->value;
            auto ok_result = f(List::from(node, next), env);
            if (!ok_result.ok) {
                SCOPES_RETURN_ERROR(ok_result.except);
            }
            return ok_result._0;
        }
        sc_list_scope_tuple_t result = { nullptr, nullptr };
        return result;
    }

    SCOPES_RESULT(ValueRef) expand(ValueRef anynode) {
        SCOPES_RESULT_TYPE(ValueRef);
        SCOPES_TRACE_EXPANDER(anynode);
    expand_again:
        const Anchor *anchor = anynode.anchor();
        if (!anynode.isa<Const>()) {
            if (verbose) {
                StyledStream ss(SCOPES_CERR);
                ss << "ignoring " << anynode << std::endl;
            }
            // return as-is
            return anynode;
        }
        ConstRef node = anynode.cast<Const>();
        auto T = node->get_type();
        if (T == TYPE_List) {
            if (verbose) {
                StyledStream ss(SCOPES_CERR);
                ss << "expanding list " << node << std::endl;
            }

            const List *list = SCOPES_GET_RESULT(extract_list_constant(node));
            if (list == EOL) {
                // zero arguments
                return ref(anchor, ArgumentListTemplate::from());
            }

            ValueRef head = list->at;

            auto headT = try_get_const_type(head);
            // resolve symbol
            if (headT == TYPE_Symbol) {
                ValueRef headnode;
                const String *doc;
                if (env->lookup(head.cast<Const>(), headnode, doc)) {
                    head = ref(head.anchor(), headnode);
                    headT = try_get_const_type(head);
                }
            }

            if (headT == TYPE_Builtin) {
                Builtin func = SCOPES_GET_RESULT(extract_builtin_constant(head));
                switch(func.value()) {
                case FN_GetSyntaxScope:
                    SCOPES_CHECK_RESULT(verify_list_parameter_count("this-scope", list, 0, 0));
                    return ValueRef(ref(anchor, ConstPointer::scope_from(env)));
                case KW_SyntaxLog: return SCOPES_GET_RESULT(expand_syntax_log(list));
                case KW_Fn: return SCOPES_GET_RESULT(expand_fn(list, ExpandFnSetup()));
                case KW_Inline: {
                    ExpandFnSetup setup;
                    setup.inlined = true;
                    return SCOPES_GET_RESULT(expand_fn(list, setup));
                }
                case KW_RunStage: return SCOPES_GET_RESULT(expand_run_stage(list));
                case KW_Let: return SCOPES_GET_RESULT(expand_let(list));
                case KW_IndirectLet: return SCOPES_GET_RESULT(expand_let(list, true));
                case KW_Loop: return SCOPES_GET_RESULT(expand_loop(list));
                case KW_Try: return SCOPES_GET_RESULT(expand_try(list));
                case KW_If: return SCOPES_GET_RESULT(expand_if(list));
                case KW_Switch: return SCOPES_GET_RESULT(expand_switch(list));
                case KW_SyntaxQuote: return SCOPES_GET_RESULT(expand_syntax_quote(list));
                case KW_ASTQuote: return SCOPES_GET_RESULT(expand_ast_quote(list));
                case KW_ASTUnquote: return SCOPES_GET_RESULT(expand_ast_unquote(list));
                case KW_ASTUnquoteArguments: return SCOPES_GET_RESULT(expand_ast_unquote_arguments(list));
                case KW_Merge: return SCOPES_GET_RESULT(expand_merge(list));
                case KW_Forward: return SCOPES_GET_RESULT(expand_forward(list));
                //case KW_Defer: return expand_defer(list);
                case KW_Do: return SCOPES_GET_RESULT(expand_do(list));
                case KW_DoIn: return SCOPES_GET_RESULT(expand_inline_do(list));
                case KW_Label: return SCOPES_GET_RESULT(expand_label(list));
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
                    return ValueRef(ref(anchor,
                        SCOPES_GET_RESULT(expand_call(list, flags))));
                } break;
                default: break;
                }
            }

            ValueRef list_handler_node;
            const String *doc;
            if (env->lookup(ConstInt::symbol_from(Symbol(SYM_ListWildcard)), list_handler_node, doc)) {
                auto T = try_get_const_type(list_handler_node);
                if (T != list_expander_func_type) {
                    SCOPES_TRACE_HOOK(list_handler_node);
                    //list_handler_node.cast<TypedValue>()->get_type(),
                    SCOPES_ERROR(SyntaxListExpanderTypeMismatch, T,
                        list_expander_func_type);
                }
                sc_syntax_wildcard_func_t f =
                    (sc_syntax_wildcard_func_t)list_handler_node.cast<ConstPointer>()->value;
                auto ok_result = f(List::from(node, next), env);
                if (!ok_result.ok) {
                    SCOPES_RETURN_ERROR(ok_result.except);
                }
                auto result = ok_result._0;
                if (result._0) {
                    ValueRef newnode = result._0->at;
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
            return ValueRef(ref(anchor, SCOPES_GET_RESULT(expand_call(list))));
        } else if (T == TYPE_Symbol) {
            if (verbose) {
                StyledStream ss(SCOPES_CERR);
                ss << "expanding symbol " << node << std::endl;
            }

            Symbol name = SCOPES_GET_RESULT(extract_symbol_constant(node));

            ValueRef result;
            const String *doc;
            if (!env->lookup(node.cast<ConstInt>(), result, doc)) {
                sc_list_scope_tuple_t result = SCOPES_GET_RESULT(expand_symbol(node));
                if (result._0) {
                    ValueRef newnode = result._0->at;
                    if (newnode != node) {
                        anynode = newnode;
                        next = result._0->next;
                        env = result._1;
                        goto expand_again;
                    }
                }

                SCOPES_ERROR(SyntaxUndeclaredIdentifier, name, env);
            }
            // update anchor to lookup position
            return ref(anchor, result);
        } else {
            if (verbose) {
                StyledStream ss(SCOPES_CERR);
                ss << "ignoring " << node << std::endl;
            }
            return ValueRef(node);
        }
    }

};

bool Expander::verbose = false;
const Type *Expander::list_expander_func_type = nullptr;

SCOPES_RESULT(sc_valueref_list_scope_tuple_t) expand(const ValueRef &expr, const List *next, const Scope *scope) {
    SCOPES_RESULT_TYPE(sc_valueref_list_scope_tuple_t);
    const Scope *subenv = scope?scope:sc_get_globals();
    Expander subexpr(subenv, TemplateRef(), next);
    ValueRef value = SCOPES_GET_RESULT(subexpr.expand(expr));
    sc_valueref_list_scope_tuple_t result = { value, subexpr.next, subexpr.env };
    return result;
}

SCOPES_RESULT(TemplateRef) expand_inline(const Anchor *anchor, const TemplateRef &astscope, const List *expr, const Scope *scope) {
    SCOPES_RESULT_TYPE(TemplateRef);
    Timer sum_expand_time(TIMER_Expand);
    //const Anchor *anchor = expr->anchor();
    //auto list = SCOPES_GET_RESULT(extract_list_constant(expr));
    assert(anchor);
    TemplateRef mainfunc = ref(anchor, Template::from(SYM_Unnamed));
    mainfunc->set_inline();

    const Scope *subenv = scope?scope:sc_get_globals();
    Expander subexpr(subenv, astscope);
    mainfunc->value = SCOPES_GET_RESULT(subexpr.expand_expression(ref(anchor, expr), false));

    return mainfunc;
}

SCOPES_RESULT(TemplateRef) expand_module(const Anchor *anchor, const List *expr, const Scope *scope) {
    SCOPES_RESULT_TYPE(TemplateRef);
    Timer sum_expand_time(TIMER_Expand);
    //const Anchor *anchor = expr->anchor();
    //auto list = SCOPES_GET_RESULT(extract_list_constant(expr));
    assert(anchor);
    StyledString ss = StyledString::plain();
    ss.out << anchor->path.name()->data << ":" << anchor->lineno;
    TemplateRef mainfunc = ref(anchor, Template::from(Symbol(ss.str())));

    const Scope *subenv = scope?scope:sc_get_globals();
    const String *doc = nullptr;
    if (expr && (expr->next != EOL)) {
        doc = try_extract_string(expr->at);
        if (doc) {
            expr = expr->next;
        }
    }
    subenv = Scope::from(doc, subenv);

    Expander subexpr(subenv, mainfunc);
    mainfunc->value = SCOPES_GET_RESULT(subexpr.expand_expression(ref(anchor, expr), false));

    return mainfunc;
}

} // namespace scopes
