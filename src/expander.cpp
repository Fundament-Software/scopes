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
#include "label.hpp"
#include "scope.hpp"
#include "stream_expr.hpp"
#include "anchor.hpp"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------
// MACRO EXPANDER
//------------------------------------------------------------------------------
// expands macros and generates the IL

static bool verify_list_parameter_count(const List *expr, int mincount, int maxcount) {
    assert(expr != EOL);
    if ((mincount <= 0) && (maxcount == -1)) {
        return true;
    }
    int argcount = (int)expr->count - 1;

    if ((maxcount >= 0) && (argcount > maxcount)) {
        location_error(
            format("excess argument. At most %i arguments expected", maxcount));
        return false;
    }
    if ((mincount >= 0) && (argcount < mincount)) {
        location_error(
            format("at least %i arguments expected, got %i", mincount, argcount));
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------

struct Expander {
    Label *state;
    Scope *env;
    const List *next;
    static bool verbose;

    const Type *list_expander_func_type;

    Expander(Label *_state, Scope *_env, const List *_next = EOL) :
        state(_state),
        env(_env),
        next(_next),
        list_expander_func_type(nullptr) {
        list_expander_func_type = Pointer(Function(
            ReturnLabel({unknown_of(TYPE_List), unknown_of(TYPE_Scope)}),
            {TYPE_List, TYPE_Scope}), PTF_NonWritable, SYM_Unnamed);
    }

    ~Expander() {}

    bool is_goto_label(Any enter) {
        return (enter.type == TYPE_Label)
            && (enter.label->params[0]->type == TYPE_Nothing);
    }

    // arguments must include continuation
    // enter and args must be passed with syntax object removed
    void br(Any enter, const Args &args, uint64_t flags = 0) {
        assert(!args.empty());
        const Anchor *anchor = get_active_anchor();
        assert(anchor);
        if (!state) {
            set_active_anchor(anchor);
            location_error(String::from("can not define body: continuation already exited."));
            return;
        }
        assert(!is_goto_label(enter) || (args[0].value.type == TYPE_Nothing));
        assert(state->body.enter.type == TYPE_Nothing);
        assert(state->body.args.empty());
        state->body.flags = flags;
        state->body.enter = enter;
        state->body.args = args;
        state->body.anchor = anchor;
        state = nullptr;
    }

    bool is_instanced_dest(Any val) {
        return (val.type == TYPE_Parameter)
            || (val.type == TYPE_Label)
            || (val.type == TYPE_Nothing);
    }

    void verify_dest_not_none(Any dest) {
        if (dest.type == TYPE_Nothing) {
            location_error(String::from("attempting to implicitly return from label"));
        }
    }

    Any write_dest(const Any &dest) {
        if (dest.type == TYPE_Symbol) {
            return none;
        } else if (is_instanced_dest(dest)) {
            if (last_expression()) {
                verify_dest_not_none(dest);
                br(dest, { none });
            }
            return none;
        } else {
            assert(false && "illegal dest type");
        }
        return none;
    }

    Any write_dest(const Any &dest, const Any &value) {
        if (dest.type == TYPE_Symbol) {
            return value;
        } else if (is_instanced_dest(dest)) {
            if (last_expression()) {
                verify_dest_not_none(dest);
                br(dest, { none, value });
            }
            return value;
        } else {
            assert(false && "illegal dest type");
        }
        return none;
    }

    void expand_block(const List *it, const Any &dest) {
        assert(is_instanced_dest(dest));
        if (it == EOL) {
            br(dest, { none });
        } else {
            while (it) {
                next = it->next;
                const Syntax *sx = it->at;
                Any expr = sx->datum;
                if (!last_expression() && (expr.type == TYPE_String)) {
                    env->set_doc(expr);
                }
                expand(it->at, dest);
                it = next;
            }
        }
    }

    Any expand_syntax_extend(const List *it, const Any &dest) {
        auto _anchor = get_active_anchor();

        verify_list_parameter_count(it, 1, -1);

        // skip head
        it = it->next;

        Label *func = Label::from(_anchor, Symbol(KW_SyntaxExtend));

        auto retparam = Parameter::from(_anchor, Symbol(SYM_Unnamed), TYPE_Unknown);
        auto scopeparam = Parameter::from(_anchor, Symbol(SYM_SyntaxScope), TYPE_Unknown);

        func->append(retparam);
        func->append(scopeparam);

        Scope *subenv = Scope::from(env);
        subenv->bind(Symbol(SYM_SyntaxScope), scopeparam);

        Expander subexpr(func, subenv);

        subexpr.expand_block(it, retparam);

        set_active_anchor(_anchor);

        Args args;
        args.reserve(4);
        Label *nextstate = nullptr;
        Any result = none;
        if (dest.type == TYPE_Symbol) {
            nextstate = Label::continuation_from(_anchor, Symbol(SYM_Unnamed));
            Parameter *param = Parameter::variadic_from(_anchor, Symbol(SYM_Unnamed), TYPE_Unknown);
            nextstate->append(param);
            args.push_back(nextstate);
            result = param;
        } else if (is_instanced_dest(dest)) {
            args.push_back(dest);
        } else {
            assert(false && "syntax extend: illegal dest type");
        }
        args.push_back(func);
        args.push_back(Syntax::from(_anchor, next));
        args.push_back(env);
        //state = subexp.state;
        set_active_anchor(_anchor);
        br(Builtin(KW_SyntaxExtend), args);
        state = nextstate;
        next = EOL;
        return result;
    }

    Parameter *expand_parameter(Any value) {
        const Syntax *sxvalue = value;
        const Anchor *anchor = sxvalue->anchor;
        Any _value = sxvalue->datum;
        if (_value.type == TYPE_Parameter) {
            return _value.parameter;
        } else if (_value.type == TYPE_List && _value.list == EOL) {
            return Parameter::from(anchor, Symbol(SYM_Unnamed), TYPE_Nothing);
        } else {
            _value.verify(TYPE_Symbol);
            Parameter *param = nullptr;
            if (ends_with_parenthesis(_value.symbol)) {
                param = Parameter::variadic_from(anchor, _value.symbol, TYPE_Unknown);
            } else {
                param = Parameter::from(anchor, _value.symbol, TYPE_Unknown);
            }
            env->bind(_value.symbol, param);
            return param;
        }
    }

    struct ExpandFnSetup {
        bool label;
        bool inlined;

        ExpandFnSetup() {
            label = false;
            inlined = false;
        };
    };

    Any expand_fn(const List *it, const Any &dest, const ExpandFnSetup &setup) {
        auto _anchor = get_active_anchor();

        verify_list_parameter_count(it, 1, -1);

        // skip head
        it = it->next;

        assert(it != EOL);

        bool continuing = false;
        Label *func = nullptr;
        Any tryfunc_name = unsyntax(it->at);
        if (tryfunc_name.type == TYPE_Symbol) {
            // named self-binding
            // see if we can find a forward declaration in the local scope
            Any result = none;
            if (env->lookup_local(tryfunc_name.symbol, result)
                && (result.type == TYPE_Label)
                && !result.label->is_valid()) {
                func = result.label;
                continuing = true;
            } else {
                func = Label::from(_anchor, tryfunc_name.symbol);
                env->bind(tryfunc_name.symbol, func);
            }
            it = it->next;
        } else if (tryfunc_name.type == TYPE_String) {
            // named lambda
            func = Label::from(_anchor, Symbol(tryfunc_name.string));
            it = it->next;
        } else {
            // unnamed lambda
            func = Label::from(_anchor, Symbol(SYM_Unnamed));
        }
        if (setup.inlined)
            func->set_inline();

        Parameter *retparam = nullptr;
        if (continuing) {
            assert(!func->params.empty());
            retparam = func->params[0];
        } else {
            retparam = Parameter::from(_anchor, Symbol(SYM_Unnamed), setup.label?TYPE_Nothing:TYPE_Unknown);
            func->append(retparam);
        }

        if (it == EOL) {
            // forward declaration
            if (tryfunc_name.type != TYPE_Symbol) {
                location_error(setup.label?
                    String::from("forward declared label must be named")
                    :String::from("forward declared function must be named"));
            }

            return write_dest(dest);
        }

        const Syntax *sxplist = it->at;
        const List *params = sxplist->datum;

        it = it->next;

        Scope *subenv = Scope::from(env);
        // hidden self-binding for subsequent macros
        subenv->bind(SYM_ThisFnCC, func);
        Any subdest = none;
        if (!setup.label) {
            subenv->bind(KW_Recur, func);

            subdest = retparam;
            subenv->bind(KW_Return, retparam);
        }
        // ensure the local scope does not contain special symbols
        subenv = Scope::from(subenv);

        Expander subexpr(func, subenv);

        while (params != EOL) {
            func->append(subexpr.expand_parameter(params->at));
            params = params->next;
        }

        if ((it != EOL) && (it->next != EOL)) {
            Any val = unsyntax(it->at);
            if (val.type == TYPE_String) {
                func->docstring = val.string;
                it = it->next;
            }
        }

        subexpr.expand_block(it, subdest);

        if (state) {
            func->body.scope_label = state;
        }

        set_active_anchor(_anchor);
        return write_dest(dest, func);
    }

    bool is_return_parameter(Any val) {
        return (val.type == TYPE_Parameter) && (val.parameter->index == 0);
    }

    bool last_expression() {
        return next == EOL;
    }

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

    Any expand_defer(const List *it, const Any &dest) {
        auto _anchor = get_active_anchor();

        it = it->next;
        const List *body = it;
        const List *block = next;
        next = EOL;

        Label *nextstate = Label::continuation_from(_anchor, Symbol(SYM_Unnamed));

        expand_block(block, nextstate);

        state = nextstate;
        // read parameter names
        it = unsyntax(it->at);
        while (it != EOL) {
            nextstate->append(expand_parameter(it->at));
            it = it->next;
        }
        return expand_do(body, dest, false);
    }

    Any expand_do(const List *it, const Any &dest, bool new_scope) {
        auto _anchor = get_active_anchor();

        it = it->next;

        Any result = none;
        Any subdest = none;
        Label *nextstate = make_nextstate(dest, result, subdest);

        Label *func = Label::continuation_from(_anchor, Symbol(SYM_Unnamed));
        Scope *subenv = env;
        if (new_scope) {
            subenv = Scope::from(env);
        }
        Expander subexpr(func, subenv);
        subexpr.expand_block(it, subdest);

        set_active_anchor(_anchor);
        br(func, { none });
        state = nextstate;
        return result;
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

    // (let x ... [= args ...])
    // (let name ([x ...]) [= args ...])
    // ...
    Any expand_let(const List *it, const Any &dest) {

        verify_list_parameter_count(it, 1, -1);
        it = it->next;

        auto _anchor = get_active_anchor();

        Symbol labelname = Symbol(SYM_Unnamed);
        const List *params = nullptr;
        const List *values = nullptr;

        if (it) {
            auto name = unsyntax(it->at);
            auto nextit = it->next;
            if ((name.type == TYPE_Symbol) && nextit) {
                auto val = unsyntax(nextit->at);
                if (val.type == TYPE_List) {
                    labelname = name.symbol;
                    params = val.list;
                    nextit = nextit->next;
                    it = params;
                    if (nextit != EOL) {
                        if (!is_equal_token(unsyntax(nextit->at))) {
                            location_error(String::from("equal sign (=) expected"));
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
                auto name = unsyntax(endit->at);
                if (is_equal_token(name))
                    break;
                endit = endit->next;
            }
            if (endit != EOL)
                values = endit;
        }

        Label *nextstate = nullptr;
        if (!values) {
            // no assignments, reimport parameter names into local scope
            if (labelname != SYM_Unnamed) {
                nextstate = Label::continuation_from(_anchor, labelname);
                env->bind(labelname, nextstate);
            }

            while (it != endit) {
                auto name = unsyntax(it->at);
                name.verify(TYPE_Symbol);
                AnyDoc entry = { none, nullptr };
                if (!env->lookup(name.symbol, entry)) {
                    StyledString ss;
                    ss.out << "no such name bound in parent scope: '"
                        << name.symbol.name()->data << "'. ";
                    print_name_suggestions(name.symbol, ss.out);
                    location_error(ss.str());
                }
                env->bind_with_doc(name.symbol, entry);
                it = it->next;
            }

            if (nextstate) {
                br(nextstate, { none });
                state = nextstate;
            }

            return write_dest(dest);
        }

        // small hack to simplify simple aliasing
        if (labelname == SYM_Unnamed) {
            // label is implicit
            if ((it->count == 3) && (values->count == 2)) {
                // single simple value is being assigned (k = v)
                auto key = unsyntax(it->at);
                auto val = unsyntax(values->next->at);
                if ((key.type == TYPE_Symbol) && (val.type == TYPE_Symbol)) {
                    Any value = none;
                    if (env->lookup(val.symbol, value)) {
                        env->bind(key.symbol, value);
                        return write_dest(dest);
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
            nextstate->append(expand_parameter(it->at));
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
                location_error(ss.str());
            }
            subexp.next = it->next;
            args.push_back(subexp.expand(it->at, Symbol(SYM_Unnamed)));
            it = subexp.next;
        }

        //
        for (auto kv = env->map->begin(); kv != env->map->end(); ++kv) {
            orig_env->bind(kv->first, kv->second.value);
        }
        env = orig_env;

        set_active_anchor(_anchor);
        state = subexp.state;
        br(nextstate, args);
        state = nextstate;

        return write_dest(dest);
    }

    // quote <value> ...
    Any expand_quote(const List *it, const Any &dest) {
        //auto _anchor = get_active_anchor();

        verify_list_parameter_count(it, 1, -1);
        it = it->next;

        Any result = none;
        if (it->count == 1) {
            result = it->at;
        } else {
            result = it;
        }
        return write_dest(dest, strip_syntax(result));
    }

    Any expand_syntax_log(const List *it, const Any &dest) {
        //auto _anchor = get_active_anchor();

        verify_list_parameter_count(it, 1, 1);
        it = it->next;

        Any val = unsyntax(it->at);
        val.verify(TYPE_Symbol);

        auto sym = val.symbol;
        if (sym == KW_True) {
            this->verbose = true;
        } else if (sym == KW_False) {
            this->verbose = false;
        } else {
            // ignore
        }

        return write_dest(dest);
    }

    // (if cond body ...)
    // [(elseif cond body ...)]
    // [(else body ...)]
    Any expand_if(const List *it, const Any &dest) {
        auto _anchor = get_active_anchor();

        std::vector<const List *> branches;

    collect_branch:
        verify_list_parameter_count(it, 1, -1);
        branches.push_back(it);

        it = next;
        if (it != EOL) {
            auto itnext = it->next;
            const Syntax *sx = it->at;
            if (sx->datum.type == TYPE_List) {
                it = sx->datum;
                if (it != EOL) {
                    auto head = unsyntax(it->at);
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

        Any result = none;
        Any subdest = none;
        Label *nextstate = make_nextstate(dest, result, subdest);

        if (subdest.type == TYPE_Label) {
            subdest.label->unset_inline();
            subdest.label->set_merge();
        }

        int lastidx = (int)branches.size() - 1;
        for (int idx = 0; idx < lastidx; ++idx) {
            it = branches[idx];
            it = it->next;

            Label *thenstate = Label::inline_from(_anchor, Symbol(SYM_Unnamed));
            Label *elsestate = Label::inline_from(_anchor, Symbol(SYM_Unnamed));
            if (state) {
                thenstate->body.scope_label = state;
                elsestate->body.scope_label = state;
            }

            Expander subexp(state, env);
            subexp.next = it->next;
            Any cond = subexp.expand(it->at, Symbol(SYM_Unnamed));
            it = subexp.next;

            set_active_anchor(_anchor);
            state = subexp.state;

            br(Builtin(FN_Branch), { subdest, cond, thenstate, elsestate });

            subexp.env = Scope::from(env);
            subexp.state = thenstate;
            subexp.expand_block(it, thenstate->params[0]);

            state = elsestate;
        }

        assert(!state->is_basic_block_like());

        it = branches[lastidx];
        if (it != EOL) {
            it = it->next;
            Expander subexp(state, Scope::from(env));
            subexp.expand_block(it, state->params[0]);
        } else {
            br(state->params[0], { none });
        }

        state = nextstate;

        return result;
    }

    static bool get_kwargs(Any it, Argument &value) {
        it = unsyntax(it);
        if (it.type != TYPE_List) return false;
        auto l = it.list;
        if (l == EOL) return false;
        if (l->count != 3) return false;
        it = unsyntax(l->at);
        if (it.type != TYPE_Symbol) return false;
        value.key = it.symbol;
        l = l->next;
        it = unsyntax(l->at);
        if (it.type != TYPE_Symbol) return false;
        if (it.symbol != OP_Set) return false;
        l = l->next;
        value.value = l->at;
        return true;
    }

    Any expand_call(const List *it, const Any &dest, bool rawcall = false) {
        if (it == EOL)
            return write_dest(dest, it);
        auto _anchor = get_active_anchor();
        Expander subexp(state, env, it->next);

        Args args;
        args.reserve(it->count);

        Any result = none;
        Any subdest = none;
        Label *nextstate = make_nextstate(dest, result, subdest);
        args.push_back(subdest);

        Any enter = subexp.expand(it->at, Symbol(SYM_Unnamed));
        if (is_return_parameter(enter)) {
            assert(enter.parameter->type != TYPE_Nothing);
            args[0] = none;
            if (!last_expression()) {
                location_error(
                    String::from("return call must be last in statement list"));
            }
        } else if (is_goto_label(enter)) {
            args[0] = none;
        }

        it = subexp.next;
        while (it) {
            subexp.next = it->next;
            Argument value;
            set_active_anchor(((const Syntax *)it->at)->anchor);
            if (get_kwargs(it->at, value)) {
                value.value = subexp.expand(
                    value.value, Symbol(SYM_Unnamed));
            } else {
                value = subexp.expand(it->at, Symbol(SYM_Unnamed));
            }
            args.push_back(value);
            it = subexp.next;
        }

        state = subexp.state;
        set_active_anchor(_anchor);
        br(enter, args, rawcall?LBF_RawCall:0);
        state = nextstate;
        return result;
    }

    Any expand(const Syntax *sx, const Any &dest) {
    expand_again:
        set_active_anchor(sx->anchor);
        if (sx->quoted) {
            if (verbose) {
                StyledStream ss(std::cerr);
                ss << "quoting ";
                stream_expr(ss, sx, StreamExprFormat::debug_digest());
            }
            // return as-is
            return write_dest(dest, sx->datum);
        }
        Any expr = sx->datum;
        if (expr.type == TYPE_List) {
            if (verbose) {
                StyledStream ss(std::cerr);
                ss << "expanding list ";
                stream_expr(ss, sx, StreamExprFormat::debug_digest());
            }

            const List *list = expr.list;
            if (list == EOL) {
                location_error(String::from("expression is empty"));
            }

            Any head = unsyntax(list->at);

            // resolve symbol
            if (head.type == TYPE_Symbol) {
                env->lookup(head.symbol, head);
            }

            if (head.type == TYPE_Builtin) {
                Builtin func = head.builtin;
                switch(func.value()) {
                case KW_SyntaxLog: return expand_syntax_log(list, dest);
                case KW_Fn: {
                    return expand_fn(list, dest, ExpandFnSetup());
                }
                case KW_Inline: {
                    ExpandFnSetup setup;
                    setup.inlined = true;
                    return expand_fn(list, dest, setup);
                }
                case KW_Label: {
                    ExpandFnSetup setup;
                    setup.label = true;
                    return expand_fn(list, dest, setup);
                }
                case KW_SyntaxExtend: return expand_syntax_extend(list, dest);
                case KW_Let: return expand_let(list, dest);
                case KW_If: return expand_if(list, dest);
                case KW_Quote: return expand_quote(list, dest);
                case KW_Defer: return expand_defer(list, dest);
                case KW_Do: return expand_do(list, dest, true);
                case KW_DoIn: return expand_do(list, dest, false);
                case KW_RawCall:
                case KW_Call: {
                    verify_list_parameter_count(list, 1, -1);
                    list = list->next;
                    assert(list != EOL);
                    return expand_call(list, dest, func.value() == KW_RawCall);
                } break;
                default: break;
                }
            }

            Any list_handler = none;
            if (env->lookup(Symbol(SYM_ListWildcard), list_handler)) {
                if (list_handler.type != list_expander_func_type) {
                    StyledString ss;
                    ss.out << "custom list expander has wrong type "
                        << list_handler.type << ", must be "
                        << list_expander_func_type;
                    location_error(ss.str());
                }
                struct ListScopePair { const List *topit; Scope *env; };
                typedef ListScopePair (*HandlerFuncType)(const List *, Scope *);
                HandlerFuncType f = (HandlerFuncType)list_handler.pointer;
                auto result = f(List::from(sx, next), env);
                const Syntax *newsx = result.topit->at;
                if (newsx != sx) {
                    sx = newsx;
                    next = result.topit->next;
                    env = result.env;
                    goto expand_again;
                } else if (verbose) {
                    StyledStream ss(std::cerr);
                    ss << "ignored by list handler" << std::endl;
                }
            }
            return expand_call(list, dest);
        } else if (expr.type == TYPE_Symbol) {
            if (verbose) {
                StyledStream ss(std::cerr);
                ss << "expanding symbol ";
                stream_expr(ss, sx, StreamExprFormat::debug_digest());
            }

            Symbol name = expr.symbol;

            Any result = none;
            if (!env->lookup(name, result)) {
                Any symbol_handler = none;
                if (env->lookup(Symbol(SYM_SymbolWildcard), symbol_handler)) {
                    if (symbol_handler.type != list_expander_func_type) {
                        StyledString ss;
                        ss.out << "custom symbol expander has wrong type "
                            << symbol_handler.type << ", must be "
                            << list_expander_func_type;
                        location_error(ss.str());
                    }
                    struct ListScopePair { const List *topit; Scope *env; };
                    typedef ListScopePair (*HandlerFuncType)(const List *, Scope *);
                    HandlerFuncType f = (HandlerFuncType)symbol_handler.pointer;
                    auto result = f(List::from(sx, next), env);
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
                location_error(ss.str());
            }
            return write_dest(dest, result);
        } else {
            if (verbose) {
                StyledStream ss(std::cerr);
                ss << "ignoring ";
                stream_expr(ss, sx, StreamExprFormat::debug_digest());
            }
            return write_dest(dest, expr);
        }
    }

};

bool Expander::verbose = false;

Label *expand_inline(Any expr, Scope *scope) {
    const Anchor *anchor = get_active_anchor();
    if (expr.type == TYPE_Syntax) {
        anchor = expr.syntax->anchor;
        set_active_anchor(anchor);
        expr = expr.syntax->datum;
    }
    expr.verify(TYPE_List);
    assert(anchor);
    Label *mainfunc = Label::function_from(anchor, SYM_Unnamed);
    mainfunc->set_inline();
    Any retparam = mainfunc->params[0];

    Scope *subenv = scope?scope:globals;

    Expander subexpr(mainfunc, subenv);
    subexpr.expand_block(expr, retparam);

    return mainfunc;
}

Label *expand_module(Any expr, Scope *scope) {
    const Anchor *anchor = get_active_anchor();
    if (expr.type == TYPE_Syntax) {
        anchor = expr.syntax->anchor;
        set_active_anchor(anchor);
        expr = expr.syntax->datum;
    }
    expr.verify(TYPE_List);
    assert(anchor);
    Label *mainfunc = Label::function_from(anchor, anchor->path());
    Any retparam = mainfunc->params[0];

    Scope *subenv = scope?scope:globals;
    // can't insert the block below because it interplays badly with syntax-extend
    #if 0
    subenv = Scope::from(subenv);
    //subenv->bind(KW_Return, retparam);
    // ensure the local scope does not contain special symbols
    subenv = Scope::from(subenv);
    #endif

    Expander subexpr(mainfunc, subenv);
    subexpr.expand_block(expr, retparam);

    return mainfunc;
}

} // namespace scopes
