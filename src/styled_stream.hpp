/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_STYLED_STREAM_HPP
#define SCOPES_STYLED_STREAM_HPP

#include "symbol_enum.hpp"
#include "scopes/config.h"
#include "valueref.inc"

#include <iostream>

namespace scopes {

//------------------------------------------------------------------------------
// ANSI COLOR FORMATTING
//------------------------------------------------------------------------------

typedef KnownSymbol Style;
#if SCOPES_USE_WCHAR

typedef std::wostream OStream;

#define SCOPES_COUT std::wcout
#define SCOPES_CERR std::wcerr
#else
typedef std::ostream OStream;
#define SCOPES_COUT std::cout
#define SCOPES_CERR std::cerr
#endif

// support 24-bit ANSI colors (ISO-8613-3)
// works on most bash shells as well as windows 10
void ansi_from_style(OStream &ost, Style style);

typedef void (*StreamStyleFunction)(OStream &, Style);

void stream_ansi_style(OStream &ost, Style style);

void stream_plain_style(OStream &ost, Style style);

extern StreamStyleFunction stream_default_style;

struct StyledStream {
    StreamStyleFunction _ssf;
    OStream &_ost;

    StyledStream(OStream &ost, StreamStyleFunction ssf);
    StyledStream(OStream &ost);

    StyledStream();

    static StyledStream plain(OStream &ost);
    static StyledStream plain(StyledStream &ost);

#if SCOPES_USE_WCHAR
    StyledStream& operator<<(const char * const s);
    StyledStream& operator<<(const std::string &s);

#endif

    template<typename T>
    StyledStream& operator<<(const T &o) { _ost << o; return *this; }
    template<typename T>
    StyledStream& operator<<(const T *o) { _ost << o; return *this; }
    template<typename T>
    StyledStream& operator<<(T &o) { _ost << o; return *this; }

    StyledStream& operator<<(OStream &(*o)(OStream&));

    StyledStream& operator<<(Style s);

    StyledStream& operator<<(bool s);


    StyledStream& stream_number(int8_t x);
    StyledStream& stream_number(uint8_t x);

    StyledStream& stream_number(double x, const char *fmt);
    StyledStream& stream_number(double x);
    StyledStream& stream_number(float x);

    template<typename T>
    StyledStream& stream_number(T x) {
        _ssf(_ost, Style_Number);
        _ost << x;
        _ssf(_ost, Style_None);
        return *this;
    }
};

#define STREAM_STYLED_NUMBER(T) \
    inline StyledStream& operator<<(StyledStream& ss, T x) { \
        ss.stream_number(x); \
        return ss; \
    }
STREAM_STYLED_NUMBER(int8_t)
STREAM_STYLED_NUMBER(int16_t)
STREAM_STYLED_NUMBER(int32_t)
STREAM_STYLED_NUMBER(int64_t)
STREAM_STYLED_NUMBER(uint8_t)
STREAM_STYLED_NUMBER(uint16_t)
STREAM_STYLED_NUMBER(uint32_t)
STREAM_STYLED_NUMBER(uint64_t)
STREAM_STYLED_NUMBER(float)
STREAM_STYLED_NUMBER(double)

#undef STREAM_STYLED_NUMBER

#define SCOPES_DUMP(EXPR) \
    { StyledStream ss; ss << __FILE__ << ":" << __LINE__ << ": " <<  (EXPR) << std::endl; }

// we need to ensure all stream operators are visible, so they're grouped here
// but implemented elsewhere.

struct Anchor;
struct Closure;
struct Builtin;
struct List;
struct Nothing;
struct Scope;
struct String;
struct Symbol;
struct Syntax;
struct Type;
struct ValueIndex;
struct Value;
struct TypedValue;

StyledStream& operator<<(StyledStream& ost, Symbol &sym);
StyledStream& operator<<(StyledStream& ost, const Symbol &sym);
StyledStream& operator<<(StyledStream& ost, Builtin &builtin);
StyledStream& operator<<(StyledStream& ost, const Builtin &builtin);
StyledStream& operator<<(StyledStream& ost, const Anchor *anchor);
StyledStream& operator<<(StyledStream& ss, const Closure *closure);
StyledStream& operator<<(StyledStream& ost, const List *list);
StyledStream& operator<<(StyledStream& ost, const Nothing &value);
StyledStream& operator<<(StyledStream& ost, Scope *scope);
StyledStream& operator<<(StyledStream& ost, const String *s);
StyledStream& operator<<(StyledStream& ost, const Syntax *value);
StyledStream& operator<<(StyledStream& ost, const Type *type);
StyledStream& operator<<(StyledStream& ost, const ValueIndex &arg);
StyledStream& operator<<(StyledStream& ost, ValueIndex &arg);
StyledStream& operator<<(StyledStream& ost, const Value *value);
StyledStream& operator<<(StyledStream& ost, Value *value);
StyledStream& operator<<(StyledStream& ost, const TypedValue *value);
StyledStream& operator<<(StyledStream& ost, TypedValue *value);
template<typename T>
StyledStream& operator<<(StyledStream& ost, TValueRef<T> value) {
    if (value)
        ost << value.anchor() << value.unref();
    else
        ost << Style_Error << "<null>" << Style_None;
    return ost;
}

void stream_uid(StyledStream &ss, uint64_t uid);
void stream_address(StyledStream &ss, const void *ptr);
void set_address_name(const void *ptr, const String *name);

} // namespace scopes

#endif // SCOPES_STYLED_STREAM_HPP
