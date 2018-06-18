/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "expander.hpp"
#include "list.hpp"
#include "error.hpp"
#include "syntax.hpp"
#include "argument.hpp"
#include "type.hpp"
#include "return.hpp"
#include "pointer.hpp"
#include "function.hpp"
#include "scope.hpp"
#include "stream_expr.hpp"
#include "anchor.hpp"
#include "ast.hpp"
#include "dyn_cast.inc"
#include "scopes/scopes.h"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------
// MACRO EXPANDER
//------------------------------------------------------------------------------
// expands macros and generates the IL

static SCOPES_RESULT(void) verify_list_parameter_count(const char *context, const List *expr, int mincount, int maxcount) {
    SCOPES_RESULT_TYPE(void);
    if (!expr) {
        SCOPES_LOCATION_ERROR(format("%s: expression is empty", context));
    }
    if ((mincount <= 0) && (maxcount == -1)) {
        return true;
    }
    int argcount = (int)expr->count - 1;

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

struct Expander {
    Scope *env;
    const List *next;
    static bool verbose;

    static const Type *list_expander_func_type;

    Expander(Scope *_env, const List *_next = EOL) :
        env(_env),
        next(_next) {
        if (!list_expander_func_type) {
            list_expander_func_type = Pointer(Function(
                ReturnLabel({unknown_of(TYPE_List), unknown_of(TYPE_Scope)}, RLF_Raising),
                {TYPE_List, TYPE_Scope}), PTF_NonWritable, SYM_Unnamed);
        }
    }

    ~Expander() {}

#if 0
    bool is_goto_label(Any enter) {
        return (enter.type == TYPE_Label)
            && (enter.label->params[0]->type == TYPE_Nothing);
    }

    // arguments must include continuation
    // enter and args must be passed with syntax object removed
    SCOPES_RESULT(void) br(Any enter, const Args &args, uint64_t flags = 0) {
        SCOPES_RESULT_TYPE(void);
        assert(!args.empty());
        const Anchor *anchor = get_active_anchor();
        assert(anchor);
        if (!state) {
            set_active_anchor(anchor);
            SCOPES_LOCATION_ERROR(
                String::from("can not define body: continuation already exited."));
        }
        assert(!is_goto_label(enter) || (args[0].value.type == TYPE_Nothing));
        assert(state->body.enter.type == TYPE_Nothing);
        assert(state->body.args.empty());
        state->body.flags = flags;
        state->body.enter = enter;
        state->body.args = args;
        state->body.anchor = anchor;
        state = nullptr;
        return true;
    }

    bool is_instanced_dest(Any val) {
        return (val.type == TYPE_Parameter)
            || (val.type == TYPE_Label)
            || (val.type == TYPE_Nothing);
    }

    SCOPES_RESULT(void) verify_dest_not_none(Any dest) {
        SCOPES_RESULT_TYPE(void);
        if (dest.type == TYPE_Nothing) {
            SCOPES_LOCATION_ERROR(String::from("attempting to implicitly return from label"));
        }
        return true;
    }

    SCOPES_RESULT(Any) write_dest(const Any &dest) {
        SCOPES_RESULT_TYPE(Any);
        if (dest.type == TYPE_Symbol) {
            return Any(none);
        } else if (is_instanced_dest(dest)) {
            if (last_expression()) {
                SCOPES_CHECK_RESULT(verify_dest_not_none(dest));
                SCOPES_CHECK_RESULT(br(dest, { none }));
            }
            return Any(none);
        } else {
            assert(false && "illegal dest type");
        }
        return Any(none);
    }

    SCOPES_RESULT(Any) write_dest(const Any &dest, const Any &value) {
        SCOPES_RESULT_TYPE(Any);
        if (dest.type == TYPE_Symbol) {
            return value;
        } else if (is_instanced_dest(dest)) {
            if (last_expression()) {
                SCOPES_CHECK_RESULT(verify_dest_not_none(dest));
                SCOPES_CHECK_RESULT(br(dest, { none, value }));
            }
            return value;
        } else {
            assert(false && "illegal dest type");
        }
        return Any(none);
    }
#endif

    SCOPES_RESULT(void) expand_block(Block *block, const List *it) {
        SCOPES_RESULT_TYPE(void);
        if (it == EOL) {
            return true;
        } else {
            while (it) {
                next = it->next;
                const Syntax *sx = it->at;
                Any expr = sx->datum;
                if (!last_expression() && (expr.type == TYPE_String)) {
                    env->set_doc(expr);
                }
                block->append(SCOPES_GET_RESULT(expand(it->at)));
                it = next;
            }
        }
        return true;
    }

    SCOPES_RESULT(ASTNode *) expand_syntax_extend(const List *it) {
        SCOPES_RESULT_TYPE(ASTNode *);
        auto _anchor = get_active_anchor();

        SCOPES_CHECK_RESULT(verify_list_parameter_count("syntax-extend", it, 1, -1));

        // skip head
        it = it->next;

        auto scopeparam = ASTSymbol::from(_anchor, SYM_Unnamed, TYPE_Scope);
        ASTFunction *func = ASTFunction::from(_anchor, Symbol(KW_SyntaxExtend), {scopeparam});

        Scope *subenv = Scope::from(env);
        subenv->bind(Symbol(SYM_SyntaxScope), scopeparam);

        Expander subexpr(subenv);

        SCOPES_CHECK_RESULT(subexpr.expand_block(func->ensure_body(), it));

        set_active_anchor(_anchor);

        auto call = Call::from(_anchor, Const::from(_anchor, Builtin(KW_SyntaxExtend)),
            {
                { SYM_Unnamed, func },
                { SYM_Unnamed, Const::from(_anchor, Syntax::from(_anchor, next)) },
                { SYM_Unnamed, Const::from(_anchor, env) } });
        next = EOL;
        return call;
    }

    SCOPES_RESULT(ASTSymbol *) expand_parameter(Any value) {
        SCOPES_RESULT_TYPE(ASTSymbol *);
        const Syntax *sxvalue = value;
        const Anchor *anchor = sxvalue->anchor;
        Any _value = sxvalue->datum;
        if (_value.type == TYPE_ASTNode
            && isa<ASTSymbol>(_value.astnode)) {
            return cast<ASTSymbol>(_value.astnode);
        } else {
            SCOPES_CHECK_RESULT(_value.verify(TYPE_Symbol));
            ASTSymbol *param = nullptr;
            if (ends_with_parenthesis(_value.symbol)) {
                param = ASTSymbol::variadic_from(anchor, _value.symbol);
            } else {
                param = ASTSymbol::from(anchor, _value.symbol);
            }
            env->bind(_value.symbol, param);
            return param;
        }
    }

    struct ExpandFnSetup {
        bool inlined;

        ExpandFnSetup() {
            inlined = false;
        };
    };

    SCOPES_RESULT(ASTNode *) expand_fn(const List *it, const ExpandFnSetup &setup) {
        SCOPES_RESULT_TYPE(ASTNode *);
        auto _anchor = get_active_anchor();

        SCOPES_CHECK_RESULT(verify_list_parameter_count("fn", it, 1, -1));

        // skip head
        it = it->next;

        assert(it != EOL);

        bool continuing = false;
        ASTFunction *func = nullptr;
        Any tryfunc_name = SCOPES_GET_RESULT(unsyntax(it->at));
        if (tryfunc_name.type == TYPE_Symbol) {
            // named self-binding
            // see if we can find a forward declaration in the local scope
            ASTNode *result = nullptr;
            if (env->lookup_local(tryfunc_name.symbol, result)
                && isa<ASTFunction>(result)
                && cast<ASTFunction>(result)->is_forward_decl()) {
                func = cast<ASTFunction>(result);
                continuing = true;
            } else {
                func = ASTFunction::from(_anchor, tryfunc_name.symbol);
                env->bind(tryfunc_name.symbol, func);
            }
            it = it->next;
        } else if (tryfunc_name.type == TYPE_String) {
            // named lambda
            func = ASTFunction::from(_anchor, Symbol(tryfunc_name.string));
            it = it->next;
        } else {
            // unnamed lambda
            func = ASTFunction::from(_anchor, Symbol(SYM_Unnamed));
        }
        if (setup.inlined)
            func->set_inline();

        if (it == EOL) {
            // forward declaration
            if (tryfunc_name.type != TYPE_Symbol) {
                SCOPES_LOCATION_ERROR(
                    String::from("forward declared function must be named"));
            }
            return func;
        }

        // not a forward declaration

        const Syntax *sxplist = it->at;
        const List *params = sxplist->datum;

        it = it->next;

        Scope *subenv = Scope::from(env);
        // hidden self-binding for subsequent macros
        subenv->bind(SYM_ThisFnCC, func);
        subenv->bind(KW_Recur, func);
        // ensure the local scope does not contain special symbols
        subenv = Scope::from(subenv);

        Expander subexpr(subenv);

        while (params != EOL) {
            func->append_param(SCOPES_GET_RESULT(subexpr.expand_parameter(params->at)));
            params = params->next;
        }

        if ((it != EOL) && (it->next != EOL)) {
            Any val = SCOPES_GET_RESULT(unsyntax(it->at));
            if (val.type == TYPE_String) {
                func->docstring = val.string;
                it = it->next;
            }
        }

        SCOPES_CHECK_RESULT(subexpr.expand_block(func->ensure_body(), it));

        set_active_anchor(_anchor);
        return func;
    }

#if 0
    bool is_return_parameter(Any val) {
        return (val.type == TYPE_Parameter) && (val.parameter->index == 0);
    }
#endif

    bool last_expression() {
        return next == EOL;
    }

#if 0
    Label *make_nextstate(const Any &dest, Any &result, Any &subdest) {
        auto _anchor = get_active_anchor();
        Label *nextstate = nullptr;
        subdest = dest;
        if (dest.type == TYPE_Symbol) {
            nextstate = Label::continuation_from(_anchor, Symbol(SYM_Unnamed));
            Parameter *param = Parameter::variadic_from(_anchor,
                Symbol(SYM_Unnamed), TYPE_Unknown);
            nextstate->append(param);
            nextstate->set_inline();
            if (state) {
                nextstate->body.scope_label = state;
            }
            subdest = nextstate;
            result = param;
        } else if (is_instanced_dest(dest)) {
            if (!last_expression()) {
                nextstate = Label::continuation_from(_anchor, Symbol(SYM_Unnamed));
                if (state) {
                    nextstate->body.scope_label = state;
                }
                subdest = nextstate;
            }
        } else {
            assert(false && "illegal dest type");
        }
        return nextstate;
    }
#endif

#if 0
    SCOPES_RESULT(Any) expand_defer(const List *it, const Any &dest) {
        SCOPES_RESULT_TYPE(Any);
        auto _anchor = get_active_anchor();

        it = it->next;
        const List *body = it;
        const List *block = next;
        next = EOL;

        Label *nextstate = Label::continuation_from(_anchor, Symbol(SYM_Unnamed));

        SCOPES_CHECK_RESULT(expand_block(block, nextstate));

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

    SCOPES_RESULT(ASTNode *) expand_do(const List *it) {
        SCOPES_RESULT_TYPE(ASTNode *);
        auto _anchor = get_active_anchor();

        it = it->next;

        Block *block = Block::from(_anchor);
        Scope *subenv = Scope::from(env);
        Expander subexpr(subenv);
        SCOPES_CHECK_RESULT(subexpr.expand_block(block, it));
        return block;
    }

    bool is_equal_token(const Any &name) {
        return (name.type == TYPE_Symbol) && (name.symbol == OP_Set);
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

#if 0
    // (let x ... [= args ...])
    // (let name ([x ...]) [= args ...])
    // ...
    SCOPES_RESULT(ASTNode *) expand_let(const List *it) {
        SCOPES_RESULT_TYPE(ASTNode *);

        SCOPES_CHECK_RESULT(verify_list_parameter_count("let", it, 1, -1));
        it = it->next;

        auto _anchor = get_active_anchor();

        Symbol labelname = Symbol(SYM_Unnamed);
        const List *params = nullptr;
        const List *values = nullptr;

        if (it) {
            auto name = SCOPES_GET_RESULT(unsyntax(it->at));
            auto nextit = it->next;
            if ((name.type == TYPE_Symbol) && nextit) {
                auto val = SCOPES_GET_RESULT(unsyntax(nextit->at));
                if (val.type == TYPE_List) {
                    labelname = name.symbol;
                    params = val.list;
                    nextit = nextit->next;
                    it = params;
                    if (nextit != EOL) {
                        if (!is_equal_token(SCOPES_GET_RESULT(unsyntax(nextit->at)))) {
                            SCOPES_LOCATION_ERROR(String::from("equal sign (=) expected"));
                        }
                        values = nextit;
                    }
                }
            }
        }

        auto endit = EOL;
        if (!params) {
            endit = it;
            // read parameter names
            while (endit) {
                auto name = SCOPES_GET_RESULT(unsyntax(endit->at));
                if (is_equal_token(name))
                    break;
                endit = endit->next;
            }
            if (endit != EOL)
                values = endit;
        }

        if (!values) {
            // no assignments, reimport parameter names into local scope
            ASTNode *last_entry = nullptr;
            while (it != endit) {
                auto name = SCOPES_GET_RESULT(unsyntax(it->at));
                SCOPES_CHECK_RESULT(name.verify(TYPE_Symbol));
                ScopeEntry entry;
                if (!env->lookup(name.symbol, entry)) {
                    StyledString ss;
                    ss.out << "no such name bound in parent scope: '"
                        << name.symbol.name()->data << "'. ";
                    print_name_suggestions(name.symbol, ss.out);
                    SCOPES_LOCATION_ERROR(ss.str());
                }
                env->bind_with_doc(name.symbol, entry);
                last_entry = entry.expr;
                it = it->next;
            }
            if (!last_entry) {
                last_entry = Const::from(_anchor, none);
            }
            return last_entry;
        }

        // small hack to simplify simple aliasing
        if (labelname == SYM_Unnamed) {
            // label is implicit
            if ((it->count == 3) && (values->count == 2)) {
                // single simple value is being assigned (k = v)
                auto key = SCOPES_GET_RESULT(unsyntax(it->at));
                auto val = SCOPES_GET_RESULT(unsyntax(values->next->at));
                if ((key.type == TYPE_Symbol) && (val.type == TYPE_Symbol)) {
                    ScopeEntry entry;
                    if (env->lookup(val.symbol, entry)) {
                        env->bind_with_doc(key.symbol, entry);
                        return entry.expr;
                    }
                }
            }
        }

        nextstate = Label::continuation_from(_anchor, labelname);
        if (state) {
            nextstate->body.scope_label = state;
        }
        if (labelname != SYM_Unnamed) {
            env->bind(labelname, nextstate);
        } else {
            nextstate->set_inline();
        }

        size_t numparams = 0;
        // bind to fresh env so the rhs expressions don't see the symbols yet
        Scope *orig_env = env;
        env = Scope::from();
        // read parameter names
        while (it != endit) {
            nextstate->append(SCOPES_GET_RESULT(expand_parameter(it->at)));
            numparams++;
            it = it->next;
        }

        if (nextstate->is_variadic()) {
            // accepts maximum number of arguments
            numparams = (size_t)-1;
        }

        it = values;

        Args args;
        args.reserve(it->count);
        args.push_back(none);

        it = it->next;

        // read init values
        Expander subexp(state, orig_env);
        size_t numvalues = 0;
        while (it) {
            numvalues++;
            if (numvalues > numparams) {
                set_active_anchor(((const Syntax *)it->at)->anchor);
                StyledString ss;
                ss.out << "number of arguments exceeds number of defined names ("
                    << numvalues << " > " << numparams << ")";
                SCOPES_LOCATION_ERROR(ss.str());
            }
            subexp.next = it->next;
            args.push_back(SCOPES_GET_RESULT(subexp.expand(it->at, Symbol(SYM_Unnamed))));
            it = subexp.next;
        }

        //
        for (auto kv = env->map->begin(); kv != env->map->end(); ++kv) {
            orig_env->bind(kv->first, kv->second.value);
        }
        env = orig_env;

        set_active_anchor(_anchor);
        state = subexp.state;
        SCOPES_CHECK_RESULT(br(nextstate, args));
        state = nextstate;

        return write_dest(dest);
    }
#endif

    // (let x ... [= args ...])
    // (let name ([x ...]) [= args ...])
    // ...
    SCOPES_RESULT(ASTNode *) expand_let(const List *it) {
        SCOPES_RESULT_TYPE(ASTNode *);

        SCOPES_CHECK_RESULT(verify_list_parameter_count("let", it, 1, -1));
        it = it->next;

        auto _anchor = get_active_anchor();

        const List *values = nullptr;

        auto endit = it;
        // read parameter names
        while (endit) {
            auto name = SCOPES_GET_RESULT(unsyntax(endit->at));
            if (is_equal_token(name))
                break;
            endit = endit->next;
        }
        if (endit != EOL)
            values = endit;

        if (!values) {
            // no assignments, reimport parameter names into local scope
            ASTNode *last_entry = nullptr;
            while (it != endit) {
                auto name = SCOPES_GET_RESULT(unsyntax(it->at));
                SCOPES_CHECK_RESULT(name.verify(TYPE_Symbol));
                ScopeEntry entry;
                if (!env->lookup(name.symbol, entry)) {
                    StyledString ss;
                    ss.out << "no such name bound in parent scope: '"
                        << name.symbol.name()->data << "'. ";
                    print_name_suggestions(name.symbol, ss.out);
                    SCOPES_LOCATION_ERROR(ss.str());
                }
                env->bind_with_doc(name.symbol, entry);
                last_entry = entry.expr;
                it = it->next;
            }
            if (!last_entry) {
                last_entry = Const::from(_anchor, none);
            }
            return last_entry;
        }

        auto let = Let::from(_anchor);

        const List *params = it;

        it = values;
        it = it->next;
        // read init values
        size_t numvalues = 0;
        Expander subexp(env);
        while (it) {
            numvalues++;
            subexp.next = it->next;
            let->append_expr(SCOPES_GET_RESULT(subexp.expand(it->at)));
            it = subexp.next;
        }

        it = params;
        size_t numparams = 0;
        // read parameter names
        while (it != endit) {
            let->append_symbol(SCOPES_GET_RESULT(expand_parameter(it->at)));
            numparams++;
            it = it->next;
        }
        if (let->is_variadic()) {
            // accepts maximum number of arguments
            numparams = (size_t)-1;
        }

        if (numvalues > numparams) {
            set_active_anchor(let->exprs[numparams]->get_anchor());
            StyledString ss;
            ss.out << "number of arguments exceeds number of defined names ("
                << numvalues << " > " << numparams << ")";
            SCOPES_LOCATION_ERROR(ss.str());
        }

        return let;
    }

    // quote <value> ...
    SCOPES_RESULT(ASTNode *) expand_quote(const List *it) {
        SCOPES_RESULT_TYPE(ASTNode *);
        auto _anchor = get_active_anchor();

        SCOPES_CHECK_RESULT(verify_list_parameter_count("quote", it, 1, -1));
        it = it->next;

        Any result = none;
        if (it->count == 1) {
            result = it->at;
        } else {
            result = it;
        }
        return Const::from(_anchor, strip_syntax(result));
    }

    SCOPES_RESULT(ASTNode *) expand_syntax_log(const List *it) {
        SCOPES_RESULT_TYPE(ASTNode *);
        auto _anchor = get_active_anchor();

        SCOPES_CHECK_RESULT(verify_list_parameter_count("syntax-log", it, 1, 1));
        it = it->next;

        Any val = SCOPES_GET_RESULT(unsyntax(it->at));
        SCOPES_CHECK_RESULT(val.verify(TYPE_Symbol));

        auto sym = val.symbol;
        if (sym == KW_True) {
            this->verbose = true;
        } else if (sym == KW_False) {
            this->verbose = false;
        } else {
            // ignore
        }

        return Const::from(_anchor, none);
    }

    // (if cond body ...)
    // [(elseif cond body ...)]
    // [(else body ...)]
    SCOPES_RESULT(ASTNode *) expand_if(const List *it) {
        SCOPES_RESULT_TYPE(ASTNode *);
        auto _anchor = get_active_anchor();

        std::vector<const List *> branches;

    collect_branch:
        SCOPES_CHECK_RESULT(verify_list_parameter_count("if", it, 1, -1));
        branches.push_back(it);

        it = next;
        if (it != EOL) {
            auto itnext = it->next;
            const Syntax *sx = it->at;
            if (sx->datum.type == TYPE_List) {
                it = sx->datum;
                if (it != EOL) {
                    auto head = SCOPES_GET_RESULT(unsyntax(it->at));
                    if (head == Symbol(KW_ElseIf)) {
                        next = itnext;
                        goto collect_branch;
                    } else if (head == Symbol(KW_Else)) {
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
            const Anchor *anchor = ((const Syntax *)it->at)->anchor;
            it = it->next;

            Block *thenblock = Block::from(anchor);

            Expander subexp(env);
            subexp.next = it->next;
            ASTNode *cond = SCOPES_GET_RESULT(subexp.expand(it->at));
            it = subexp.next;

            subexp.env = Scope::from(env);
            SCOPES_CHECK_RESULT(subexp.expand_block(thenblock, it));

            ifexpr->append_expr(cond, thenblock);
        }

        it = branches[lastidx];
        if (it != EOL) {
            const Anchor *anchor = ((const Syntax *)it->at)->anchor;
            it = it->next;
            Expander subexp(Scope::from(env));

            Block *elseblock = Block::from(anchor);
            SCOPES_CHECK_RESULT(subexp.expand_block(elseblock, it));
            ifexpr->append_expr(elseblock);
        }

        return ifexpr;
    }

    static SCOPES_RESULT(bool) get_kwargs(Any it, Symbol &key, Any &value) {
        SCOPES_RESULT_TYPE(bool);
        it = SCOPES_GET_RESULT(unsyntax(it));
        if (it.type != TYPE_List) return false;
        auto l = it.list;
        if (l == EOL) return false;
        if (l->count != 3) return false;
        it = SCOPES_GET_RESULT(unsyntax(l->at));
        if (it.type != TYPE_Symbol) return false;
        key = it.symbol;
        l = l->next;
        it = SCOPES_GET_RESULT(unsyntax(l->at));
        if (it.type != TYPE_Symbol) return false;
        if (it.symbol != OP_Set) return false;
        l = l->next;
        value = l->at;
        return true;
    }

    SCOPES_RESULT(ASTNode *) expand_call(const List *it, uint32_t flags = 0) {
        SCOPES_RESULT_TYPE(ASTNode *);

        auto _anchor = get_active_anchor();
        SCOPES_CHECK_RESULT(verify_list_parameter_count("call", it, 0, -1));
        Expander subexp(env, it->next);
        ASTNode *enter = SCOPES_GET_RESULT(subexp.expand(it->at));

        auto call = Call::from(_anchor, enter);
        call->flags = flags;

        it = subexp.next;
        while (it) {
            subexp.next = it->next;
            Symbol key = SYM_Unnamed;
            Any value;
            set_active_anchor(((const Syntax *)it->at)->anchor);
            if (SCOPES_GET_RESULT(get_kwargs(it->at, key, value))) {
                call->append(key,
                    SCOPES_GET_RESULT(subexp.expand(value)));
            } else {
                call->append(SCOPES_GET_RESULT(subexp.expand(it->at)));
            }
            it = subexp.next;
        }
        return call;
    }

    struct ListScopePair { const List *topit; Scope *env; };
    struct OKListScopePair { bool ok; ListScopePair pair; };
    typedef OKListScopePair (*HandlerFuncType)(const List *, Scope *);

    SCOPES_RESULT(ASTNode *) expand(const Syntax *sx) {
        SCOPES_RESULT_TYPE(ASTNode *);
    expand_again:
        sc_verify_stack();
        set_active_anchor(sx->anchor);
        if (sx->quoted) {
            if (verbose) {
                StyledStream ss(SCOPES_CERR);
                ss << "quoting ";
                stream_expr(ss, sx, StreamExprFormat::debug_digest());
            }
            // return as-is
            return Const::from(sx->anchor, sx->datum);
        }
        Any expr = sx->datum;
        if (expr.type == TYPE_List) {
            if (verbose) {
                StyledStream ss(SCOPES_CERR);
                ss << "expanding list ";
                stream_expr(ss, sx, StreamExprFormat::debug_digest());
            }

            const List *list = expr.list;
            if (list == EOL) {
                SCOPES_LOCATION_ERROR(String::from("expression is empty"));
            }

            Any head = SCOPES_GET_RESULT(unsyntax(list->at));
            // resolve symbol
            if (head.type == TYPE_Symbol) {
                ASTNode *headnode = nullptr;
                if (env->lookup(head.symbol, headnode)) {
                    if (isa<Const>(headnode)) {
                        head = cast<Const>(headnode)->value;
                    }
                }
            }

            if (head.type == TYPE_Builtin) {
                Builtin func = head.builtin;
                switch(func.value()) {
                case KW_SyntaxLog: return expand_syntax_log(list);
                case KW_Fn: {
                    return expand_fn(list, ExpandFnSetup());
                }
                case KW_Inline: {
                    ExpandFnSetup setup;
                    setup.inlined = true;
                    return expand_fn(list, setup);
                }
                case KW_SyntaxExtend: return expand_syntax_extend(list);
                case KW_Let: return expand_let(list);
                case KW_If: return expand_if(list);
                case KW_Quote: return expand_quote(list);
                //case KW_Defer: return expand_defer(list);
                case KW_Do: return expand_do(list);
                case KW_TryCall:
                case KW_RawCall:
                case KW_Call: {
                    SCOPES_CHECK_RESULT(verify_list_parameter_count("special call", list, 1, -1));
                    list = list->next;
                    assert(list != EOL);
                    uint32_t flags = 0;
                    switch(func.value()) {
                    case KW_RawCall: flags |= CF_RawCall; break;
                    case KW_TryCall: flags |= CF_TryCall; break;
                    default: break;
                    }
                    return expand_call(list, flags);
                } break;
                default: break;
                }
            }

            ASTNode *list_handler_node;
            if (env->lookup(Symbol(SYM_ListWildcard), list_handler_node)) {
                Any list_handler = list_handler_node;
                if (isa<Const>(list_handler_node))
                    list_handler = cast<Const>(list_handler_node)->value;
                if (list_handler.type != list_expander_func_type) {
                    StyledString ss;
                    ss.out << "custom list expander has wrong type "
                        << list_handler.type << ", must be "
                        << list_expander_func_type;
                    SCOPES_LOCATION_ERROR(ss.str());
                }
                HandlerFuncType f = (HandlerFuncType)list_handler.pointer;
                auto ok_result = f(List::from(sx, next), env);
                if (!ok_result.ok) {
                    SCOPES_RETURN_ERROR();
                }
                auto result = ok_result.pair;
                const Syntax *newsx = result.topit->at;
                if (newsx != sx) {
                    sx = newsx;
                    next = result.topit->next;
                    env = result.env;
                    goto expand_again;
                } else if (verbose) {
                    StyledStream ss(SCOPES_CERR);
                    ss << "ignored by list handler" << std::endl;
                }
            }
            set_active_anchor(sx->anchor);
            return expand_call(list);
        } else if (expr.type == TYPE_Symbol) {
            if (verbose) {
                StyledStream ss(SCOPES_CERR);
                ss << "expanding symbol ";
                stream_expr(ss, sx, StreamExprFormat::debug_digest());
            }

            Symbol name = expr.symbol;

            ASTNode *result = nullptr;
            if (!env->lookup(name, result)) {
                ASTNode *symbol_handler_node;
                if (env->lookup(Symbol(SYM_SymbolWildcard), symbol_handler_node)) {
                    Any symbol_handler = symbol_handler_node;
                    if (isa<Const>(symbol_handler_node))
                        symbol_handler = cast<Const>(symbol_handler_node)->value;
                    if (symbol_handler.type != list_expander_func_type) {
                        StyledString ss;
                        ss.out << "custom symbol expander has wrong type "
                            << symbol_handler.type << ", must be "
                            << list_expander_func_type;
                        SCOPES_LOCATION_ERROR(ss.str());
                    }
                    HandlerFuncType f = (HandlerFuncType)symbol_handler.pointer;
                    auto ok_result = f(List::from(sx, next), env);
                    if (!ok_result.ok) {
                        SCOPES_RETURN_ERROR();
                    }
                    auto result = ok_result.pair;
                    const Syntax *newsx = result.topit->at;
                    if (newsx != sx) {
                        sx = newsx;
                        next = result.topit->next;
                        env = result.env;
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
                ss << "ignoring ";
                stream_expr(ss, sx, StreamExprFormat::debug_digest());
            }
            return Const::from(sx->anchor, expr);
        }
    }

};

bool Expander::verbose = false;
const Type *Expander::list_expander_func_type = nullptr;

SCOPES_RESULT(ASTFunction *) expand_inline(Any expr, Scope *scope) {
    SCOPES_RESULT_TYPE(ASTFunction *);
    const Anchor *anchor = get_active_anchor();
    if (expr.type == TYPE_Syntax) {
        anchor = expr.syntax->anchor;
        set_active_anchor(anchor);
        expr = expr.syntax->datum;
    }
    SCOPES_CHECK_RESULT(expr.verify(TYPE_List));
    assert(anchor);
    ASTFunction *mainfunc = ASTFunction::from(anchor, SYM_Unnamed);
    mainfunc->set_inline();

    Scope *subenv = scope?scope:globals;
    Expander subexpr(subenv);
    SCOPES_CHECK_RESULT(subexpr.expand_block(mainfunc->ensure_body(), expr));

    return mainfunc;
}

SCOPES_RESULT(ASTFunction *) expand_module(Any expr, Scope *scope) {
    SCOPES_RESULT_TYPE(ASTFunction *);
    const Anchor *anchor = get_active_anchor();
    if (expr.type == TYPE_Syntax) {
        anchor = expr.syntax->anchor;
        set_active_anchor(anchor);
        expr = expr.syntax->datum;
    }
    SCOPES_CHECK_RESULT(expr.verify(TYPE_List));
    assert(anchor);
    ASTFunction *mainfunc = ASTFunction::from(anchor, anchor->path());

    Scope *subenv = scope?scope:globals;
    Expander subexpr(subenv);
    SCOPES_CHECK_RESULT(subexpr.expand_block(mainfunc->ensure_body(), expr));

    return mainfunc;
}

} // namespace scopes
