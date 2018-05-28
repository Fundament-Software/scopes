/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_STYLED_STREAM_HPP
#define SCOPES_STYLED_STREAM_HPP

#include "symbol_enum.hpp"

#include <iostream>

namespace scopes {

//------------------------------------------------------------------------------
// ANSI COLOR FORMATTING
//------------------------------------------------------------------------------

typedef KnownSymbol Style;

// support 24-bit ANSI colors (ISO-8613-3)
// works on most bash shells as well as windows 10
void ansi_from_style(std::ostream &ost, Style style);

typedef void (*StreamStyleFunction)(std::ostream &, Style);

void stream_ansi_style(std::ostream &ost, Style style);

void stream_plain_style(std::ostream &ost, Style style);

extern StreamStyleFunction stream_default_style;

struct StyledStream {
    StreamStyleFunction _ssf;
    std::ostream &_ost;

    StyledStream(std::ostream &ost, StreamStyleFunction ssf);
    StyledStream(std::ostream &ost);

    StyledStream();

    static StyledStream plain(std::ostream &ost);
    static StyledStream plain(StyledStream &ost);

    template<typename T>
    StyledStream& operator<<(const T &o) { _ost << o; return *this; }
    template<typename T>
    StyledStream& operator<<(const T *o) { _ost << o; return *this; }
    template<typename T>
    StyledStream& operator<<(T &o) { _ost << o; return *this; }

    StyledStream& operator<<(std::ostream &(*o)(std::ostream&));

    StyledStream& operator<<(Style s);

    StyledStream& operator<<(bool s);

    StyledStream& stream_number(int8_t x);
    StyledStream& stream_number(uint8_t x);

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

// we need to ensure all stream operators are visible, so they're grouped here
// but implemented elsewhere.

struct Any;
struct Anchor;
struct Closure;
struct Argument;
struct Builtin;
struct Label;
struct List;
struct Nothing;
struct Parameter;
struct Scope;
struct String;
struct Symbol;
struct Syntax;
struct Type;

StyledStream& operator<<(StyledStream& ost, Symbol &sym);
StyledStream& operator<<(StyledStream& ost, const Symbol &sym);
StyledStream& operator<<(StyledStream& ost, Any &value);
StyledStream& operator<<(StyledStream& ost, const Any &value);
StyledStream& operator<<(StyledStream& ost, Argument &value);
StyledStream& operator<<(StyledStream& ost, const Argument &value);
StyledStream& operator<<(StyledStream& ost, Builtin &builtin);
StyledStream& operator<<(StyledStream& ost, const Builtin &builtin);
StyledStream& operator<<(StyledStream& ost, const Anchor *anchor);
StyledStream& operator<<(StyledStream& ss, const Closure *closure);
StyledStream& operator<<(StyledStream& ss, Label *label);
StyledStream& operator<<(StyledStream& ss, const Label *label);
StyledStream& operator<<(StyledStream& ost, const List *list);
StyledStream& operator<<(StyledStream& ost, const Nothing &value);
StyledStream& operator<<(StyledStream& ss, Parameter *param);
StyledStream& operator<<(StyledStream& ost, Scope *scope);
StyledStream& operator<<(StyledStream& ost, const String *s);
StyledStream& operator<<(StyledStream& ost, const Syntax *value);
StyledStream& operator<<(StyledStream& ost, const Type *type);

} // namespace scopes

#endif // SCOPES_STYLED_STREAM_HPP
