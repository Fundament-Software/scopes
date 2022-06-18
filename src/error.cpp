/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "error.hpp"
#include "anchor.hpp"
#include "type.hpp"
#include "boot.hpp"
#include "value.hpp"
#include "stream_expr.hpp"
#include "type/arguments_type.hpp"
#include "type/function_type.hpp"
#include "type/tuple_type.hpp"
#include "stream_expr.hpp"
#include "dyn_cast.inc"
#include "scope.hpp"
#include "prover.hpp"

#include "scopes/config.h"

#include <algorithm>
#include <assert.h>
#include <string.h>

namespace scopes {

template<typename T> struct ArgFormatter {
    // if you land here, a specialization is missing
    //void value(StyledStream &ss, const T &arg);
};

template<> struct ArgFormatter<ValueKind> {
    void value(StyledStream &ss, ValueKind arg) {
        ss << get_value_class_name(arg);
    }
};

template<> struct ArgFormatter<TypeKind> {
    void value(StyledStream &ss, TypeKind arg) {
        switch(arg) {
        case TK_Integer: ss << "integer"; break;
        case TK_Real: ss << "real"; break;
        case TK_Pointer: ss << "pointer"; break;
        case TK_Array: ss << "array"; break;
        case TK_Vector: ss << "vector"; break;
        case TK_Tuple: ss << "tuple"; break;
        case TK_Typename: ss << "typename"; break;
        case TK_Function: ss << "function"; break;
        case TK_Image: ss << "image"; break;
        case TK_SampledImage: ss << "sampled image"; break;
        default: ss << "?unknown type kind?"; break;
        }
    }
};

template<> struct ArgFormatter<const Anchor *> {
    void value(StyledStream &ss, const Anchor *arg) {
        ss << std::endl << arg;
    }
};

template<> struct ArgFormatter<const char *> {
    void value(StyledStream &ss, const char *arg) { ss << arg; }
};

template<> struct ArgFormatter<const String *> {
    void value(StyledStream &ss, const String *arg) { ss << arg->data; }
};

template<> struct ArgFormatter<const Type *> {
    void value(StyledStream &ss, const Type *arg) { ss << arg; }
};

template<> struct ArgFormatter<int> {
    void value(StyledStream &ss, int arg) { ss << arg; }
};

template<> struct ArgFormatter<char> {
    void value(StyledStream &ss, char arg) { ss << arg; }
};

template<> struct ArgFormatter<Symbol> {
    void value(StyledStream &ss, Symbol arg) { ss << arg.name()->data; }
};

template<> struct ArgFormatter<Builtin> {
    void value(StyledStream &ss, Builtin arg) { ss << arg; }
};

template<typename T> struct ArgFormatter< TValueRef<T> > {
    void value(StyledStream &ss, const TValueRef<T> &arg) { ss << arg.unref(); }
};

template<> struct ArgFormatter<ErrnoValue> {
    void value(StyledStream &ss, ErrnoValue arg) { ss << strerror(arg.value); }
};

static void _format_token(StyledStream &ss, char c, int n) {
    // no match
    ss << "?illegal index?";
}

template<typename T, class ... Types>
static void _format_token(StyledStream &ss, char c, int n,
    const T &arg, Types ... args) {
    if (n)
        return _format_token(ss, c, n - 1, args ... );
    ArgFormatter<T> _a;
    // handle it
    switch(c) {
    case 'V': _a.value(ss, arg); break;
    default: ss << "?illegal control code?"; break;
    }
}

template<class ... Types>
static const char *format_token(StyledStream &ss, const char *tok, Types ... args) {
    char c = *tok;
    if (c) tok++;
    char cn;
    if (c >= '0' && c <= '9') {
        cn = c;
        c = 'V';
    } else {
        cn = *tok;
        if (cn) tok++;
    }
    int n = cn - '0';
    _format_token(ss, c, n, args ...);
    return tok;
}

template<class ... Types>
static void format_error(StyledStream &ss, const char *msg, Types ... args) {
    const char *p = msg;
    while(char c = *p++) {
        if (c == '%') {
            p = format_token(ss, p, args ...);
        } else {
            ss << c;
        }
    }
}

// custom formatter
template<class ... Types, class ... FuncTypes>
static void format_error(StyledStream &ss, void (*f)(StyledStream&, FuncTypes ... ), Types ... args) {
    f(ss, args ...);
}

#if 0
static std::vector<Symbol> find_closest_match(Symbol name, const Symbols &symbols) {
    const String *s = name.name();
    absl::flat_hash_set<Symbol, Symbol::Hash> done;
    done.insert(SYM_Unnamed);
    std::vector<Symbol> best_syms;
    size_t best_dist = (size_t)-1;
    for (auto sym : symbols) {
        if (done.count(sym))
            continue;
        size_t dist = distance(s, sym.name());
        if (dist == best_dist) {
            best_syms.push_back(sym);
        } else if (dist < best_dist) {
            best_dist = dist;
            best_syms = { sym };
        }
        done.insert(sym);
    }
    std::sort(best_syms.begin(), best_syms.end());
    return best_syms;
}
#endif

static void print_name_suggestions(StyledStream &ss, const Symbols &syms) {
    if (syms.empty()) return;
    ss << ". Did you mean '" << syms[0].name()->data << "'";
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

static void print_all_suggestions(StyledStream &ss, const Symbols &syms) {
    if (syms.empty()) return;
    ss << ". Try '" << syms[0].name()->data << "'";
    for (size_t i = 1; i < syms.size(); ++i) {
        if ((i + 1) == syms.size()) {
            ss << " or ";
        } else {
            ss << ", ";
        }
        ss << "'" << syms[i].name()->data << "'";
    }
}

static void syntax_undeclared_identifier_print_suggestions(StyledStream &ss, Symbol symbol, PScope scope) {
    ss << "syntax: identifier '" << symbol.name()->data;
    ss << "' is not declared in scope";
    print_name_suggestions(ss, scope->find_closest_match(symbol));
}

static void unknown_parameter_key_print_suggestions(StyledStream &ss, Symbol symbol, const Symbols &symbols) {
    ss << "no parameter named '" << symbol.name()->data;
    ss << "' in function";
    print_all_suggestions(ss, symbols);
}

static void rt_missing_scope_attribute_print_suggestions(StyledStream &ss, Symbol symbol, PScope scope) {
    ss << "runtime: no attribute named '" << symbol.name()->data;
    ss << "' in scope";
    print_name_suggestions(ss, scope->find_closest_match(symbol));
}

static void rt_missing_type_attribute_print_suggestions(StyledStream &ss, Symbol symbol, PType type) {
    ss << "runtime: no attribute named '" << symbol.name()->data;
    ss << "' in type " << type;
    print_name_suggestions(ss, type->find_closest_match(symbol));
}

static void missing_tuple_field_print_suggestions(StyledStream &ss, Symbol symbol, PType type) {
    ss << "no field named '" << symbol.name()->data;
    ss << "' in tuple " << type;
    print_name_suggestions(ss, cast<TupleType>(type)->find_closest_field_match(symbol));
}

static void rt_missing_tuple_field_print_suggestions(StyledStream &ss, Symbol symbol, PType type) {
    ss << "runtime: no field named '" << symbol.name()->data;
    ss << "' in tuple " << type;
    print_name_suggestions(ss, cast<TupleType>(type)->find_closest_field_match(symbol));
}

//------------------------------------------------------------------------------

#define TA(N, CLS) CLS _arg ## N
#define TC(N, CLS) err->arg ## N = _arg ## N
#define TD(N, CLS) , arg ## N
#define TDTOK()
#define T(CLASS, STR, ...) /*
*/bool Error ## CLASS::classof(const Error *T) {/*
*/  return T->kind() == EK_ ## CLASS;/*
*/}/*
*/Error ## CLASS::Error ## CLASS() : Error(EK_ ## CLASS) {}/*
*/Error ## CLASS *Error ## CLASS::from(SCOPES_FOREACH_EXPR(TA, ##__VA_ARGS__)) {/*
*/  Error ## CLASS *err = new Error ## CLASS();/*
*/  SCOPES_FOREACH_STMT(TC, ##__VA_ARGS__);/*
*/  return err;/*
*/}/*
*/void Error ## CLASS::stream(StyledStream &ss) const {/*
*/  format_error(ss, STR SCOPES_FOREACH(TD, TDTOK, ##__VA_ARGS__));/*
*/}

SCOPES_ERROR_KIND()
#undef T
#undef TA
#undef TC
#undef TD
#undef TDTOK

//------------------------------------------------------------------------------
// ERROR HANDLING
//------------------------------------------------------------------------------

ErrorKind Error::kind() const { return _kind; }

Error::Error(ErrorKind kind) : _kind(kind) {}

Error *Error::trace(const Backtrace &bt) {
    if (bt.kind == BTK_Dummy)
        return this;
    auto ptr = new Backtrace(bt);
    ptr->next = _trace;
    _trace = ptr;
    return this;
}

const Backtrace *Error::get_trace() const {
    return _trace;
}

//------------------------------------------------------------------------------

void stream_error_message(StyledStream &ss, const Error *err) {
    switch(err->kind()) {
#define T(CLASS, STR, ...) \
    case EK_ ## CLASS: cast<Error ## CLASS>(err)->stream(ss); break;
SCOPES_ERROR_KIND()
#undef T
    default: ss << "?corrupted error object?"; break;
    }
}

void stream_backtrace(StyledStream &ss, const Backtrace *bt) {
    if (bt->kind == BTK_Dummy) return;
    auto &&value = bt->context;
    auto anchor = value.anchor();
    switch(bt->kind) {
    case BTK_Dummy: break;
    case BTK_Parser: {
        ss << anchor << " while ";
        ss << "parsing";
        ss << std::endl;
        anchor->stream_source_line(ss);
    } break;
    case BTK_Expander: {
        ss << anchor << " while ";
        ss << "expanding";
        ss << std::endl;
        anchor->stream_source_line(ss);
    } break;
    case BTK_ProveTemplate: {
        auto templ = value.cast<Template>();
        bool is_inline = templ->is_inline();
        if (is_inline && templ->name == SYM_Unnamed)
            return;
        anchor = get_best_anchor(value);
        #if 0
        const List *list = ast_to_list(value);
        StreamExprFormat fmt;
        fmt.maxdepth = 2;
        fmt.depth = 1;
        fmt.maxlength = 3;
        stream_expr(ss, list, fmt);
        #endif
        ss << anchor << " in ";
        ss << Style_Keyword;
        if (is_inline) {
            ss << "inline";
        } else {
            ss << "fn";
        }
        ss << Style_None << " ";
        ss << Style_Function;
        ss << templ->name.name()->data;
        ss << Style_None;
        ss << std::endl;
        anchor->stream_source_line(ss);
    } break;
    case BTK_InvokeHook: {
        ss << anchor;
        ss << " while attempting to invoke hook" << std::endl;
        anchor->stream_source_line(ss);
    } break;
    case BTK_ProveArgument: {
        ss << anchor;
        ss << " while checking type of argument" << std::endl;
        anchor->stream_source_line(ss);
    } break;
    case BTK_ProveParamMap: {
        ss << anchor;
        ss << " while mapping arguments to function" << std::endl;
        anchor->stream_source_line(ss);
        if (value.isa<Const>()
            && value.cast<Const>()->get_type() == TYPE_Closure) {
            auto fanchor = extract_closure_constant(value).assert_ok()->func.anchor();
            if (fanchor != anchor) {
                ss << anchor;
                ss << " defined here" << std::endl;
                anchor->stream_source_line(ss);
            }
        }
    } break;
    case BTK_ProveArgumentLifetime: {
        ss << anchor;
        ss << " while checking lifetime of argument" << std::endl;
        anchor->stream_source_line(ss);
    } break;
    case BTK_User: {
        ss << anchor;
        if (try_get_const_type(value) == TYPE_String) {
            ss << " " << extract_string_constant(value).assert_ok()->data;
        } else {
            ss << " while executing";
        }
        ss << std::endl;
        anchor->stream_source_line(ss);
    } break;
    case BTK_Translate: {
        #if 1
        anchor = get_best_anchor(value);
        #endif
        #if 1
        ss << "While translating" << std::endl;
        StreamValueFormat fmt;
        fmt.maxdepth = 3;
        fmt.maxlength = 4;
        stream_value(ss, value, fmt);
        ss << anchor << " defined here";
        #else
        ss << anchor << " while checking expression";
        #endif
        ss << std::endl;
        anchor->stream_source_line(ss);
    } break;
    case BTK_ConvertForeignType: {
        ss << anchor;
        ss << " while converting foreign type ";
        ss << extract_string_constant(value).assert_ok()->data;
        ss << std::endl;
        anchor->stream_source_line(ss);
    } break;
    case BTK_ProveExpression: {
        #if 1
        anchor = get_best_anchor(value);
        #endif
        #ifdef SCOPES_DEBUG
        ss << "While checking expression" << std::endl;
        StreamValueFormat fmt;
        fmt.maxdepth = 3;
        fmt.maxlength = 4;
        stream_value(ss, value, fmt);
        ss << anchor << " defined here";
        #else
        ss << anchor << " while checking expression";
        #endif
        ss << std::endl;
        anchor->stream_source_line(ss);
    } break;
    }
}

static bool good_delta(const Backtrace *older, const Backtrace *newer) {
    bool same_kind = (newer->kind == older->kind);
    bool same_anchor = (newer->context.anchor() == older->context.anchor());
#if 0
    return true;
#else
    if (older->kind == BTK_User) return true;
    if (same_kind) {
        if (older->kind == BTK_Expander) return false;
        else if (older->kind == BTK_ProveExpression)
            return !same_anchor;
    }
    return true;
#endif
}

void stream_error(StyledStream &ss, const Error *err) {
    std::vector<const Backtrace *> traceback;
    {
        const Backtrace *bt = err->get_trace();
        const Backtrace *last_bt = nullptr;
        while (bt) {
            if (last_bt && !good_delta(last_bt, bt)) {
                // replace newer
                traceback.back() = bt;
            } else {
                // append
                traceback.push_back(bt);
            }
            last_bt = bt;
            bt = bt->next;
        }
    }
    for (auto bt : traceback) {
        stream_backtrace(ss, bt);
    }

    ss << Style_Error << "error:" << Style_None << " ";
    stream_error_message(ss, err);
    ss << std::endl;
}

void print_error(const Error *value) {
    auto cerr = StyledStream(SCOPES_CERR);
    stream_error(cerr, value);
}

//------------------------------------------------------------------------------

Backtrace _backtrace = { nullptr, BTK_Dummy, ValueRef() };

//------------------------------------------------------------------------------

#if 0
void collect_trace(StyledStream &ss,
    Values &values, ValueRef value) {
    values.push_back(value);
}

void stream_error(StyledStream &ss, const Error *exc) {
#if 0
    size_t i = exc->trace.size();
    const Anchor *last_anchor = nullptr;
    while (i--) {
        auto value = exc->trace[i];
        if (last_anchor && value->anchor()->is_same(last_anchor))
            continue;
        last_anchor = value->anchor();
        ss << value->anchor() << " in ";
        if (isa<Function>(value)) {
            auto fn = cast<Function>(value);
            if (fn->name == SYM_Unnamed) {
                ss << Style_Function << "unnamed" << Style_None;
            } else {
                ss << Style_Function << fn->name.name()->data << Style_None;
            }
        } else {
            ss << "expression";
        }
        ss << std::endl;
        value->anchor()->stream_source_line(ss);
    }
#endif
    if (exc->anchor) {
        ss << exc->anchor << " ";
    }
    ss << Style_Error << "error:" << Style_None << " "
        << exc->msg->data << std::endl;
    /*
    if (exc->anchor) {
        exc->anchor->stream_source_line(ss);
    }
    */
    Values values;
    for (auto def : exc->definitions) {
        values.clear();
        collect_trace(ss, values, def);
        if (!values.size())
            continue;
        size_t i = values.size();
        const Anchor *last_anchor = nullptr;
        while (i--) {
            auto value = values[i];
            const Anchor *anchor = value.anchor();
            if (last_anchor && anchor->is_same(last_anchor))
                continue;
            last_anchor = anchor;
            ss << anchor << " in ";
            if (value.isa<Function>()) {
                auto fn = value.cast<Function>();
                if (fn->name == SYM_Unnamed) {
                    ss << Style_Function << "unnamed" << Style_None;
                } else {
                    ss << Style_Function << fn->name.name()->data << Style_None;
                }
            } else {
                ss << "expression";
            }
            ss << std::endl;
            anchor->stream_source_line(ss);
        }
    }
}

void print_error(const Error *value) {
    auto cerr = StyledStream(SCOPES_CERR);
    stream_error(cerr, value);
}

Error *make_location_error(const Anchor *anchor, const String *msg) {
    return new Error(anchor, msg);
}

Error *make_error(const String *msg) {
    return new Error(nullptr, msg);
}
#endif

} // namespace scopes
