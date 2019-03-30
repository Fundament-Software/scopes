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
#include "stream_ast.hpp"
#include "type/arguments_type.hpp"
#include "type/function_type.hpp"
#include "stream_expr.hpp"
#include "dyn_cast.inc"

#include "scopes/config.h"

#include <assert.h>

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
        case TK_Union: ss << "union"; break;
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
        if (value.isa<UntypedValue>()) {
            auto uv = value.cast<UntypedValue>();
            auto _anchor = uv->def_anchor();
            if (_anchor != unknown_anchor())
                anchor = uv->def_anchor();
        }
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
        if (value.cast<Template>()->is_inline()) {
            ss << "inline";
        } else {
            ss << "fn";
        }
        ss << Style_None << " ";
        ss << value.cast<Template>()->name.name()->data;
        ss << std::endl;
        anchor->stream_source_line(ss);
    } break;
    case BTK_ProveArgument: {
        ss << anchor;
        ss << " while checking type of argument" << std::endl;
        anchor->stream_source_line(ss);
    } break;
    case BTK_ProveExpression: {
        ss << "While checking expression" << std::endl;
        if (value.isa<UntypedValue>()) {
            auto uv = value.cast<UntypedValue>();
            auto _anchor = uv->def_anchor();
            if (_anchor != unknown_anchor())
                anchor = uv->def_anchor();
        }
        const List *list = ast_to_list(value);
        StreamExprFormat fmt;
        fmt.maxdepth = 3;
        fmt.maxlength = 4;
        stream_expr(ss, list, fmt);
        ss << anchor << " defined here";
        ss << std::endl;
        anchor->stream_source_line(ss);
    } break;
    }
}

static bool good_delta(const Backtrace *newer, const Backtrace *older) {
    return newer->kind != older->kind;
}

void stream_error(StyledStream &ss, const Error *err) {
    std::vector<const Backtrace *> traceback;
    {
        const Backtrace *bt = err->get_trace();
        const Backtrace *last_bt = nullptr;
        while (bt) {
            if (!last_bt || good_delta(last_bt, bt)) {
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
