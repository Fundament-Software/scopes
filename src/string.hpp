/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_STRING_HPP
#define SCOPES_STRING_HPP

#include <stddef.h>

#include "styled_stream.hpp"
#include "scopes/config.h"

#include <sstream>
#include <vector>

namespace scopes {

//------------------------------------------------------------------------------
// STRING
//------------------------------------------------------------------------------

#if SCOPES_USE_WCHAR
typedef std::wstring CppString;
typedef std::wstringstream StringStream;
#else
typedef std::string CppString;
typedef std::stringstream StringStream;
#endif

struct String {
protected:
    String(const char *_data, size_t _count);

public:
    static const String *from(const char *s, size_t count);
    static const String *from_cstr(const char *s);
    static const String *join(const String *a, const String *b);

    template<unsigned N>
    static const String *from(const char (&s)[N]) {
        return from(s, N - 1);
    }

    static const String *from_stdstring(const std::string &s);
    static const String *from_stdstring(const std::wstring &ws);
    StyledStream& stream(StyledStream& ost, const char *escape_chars) const;
    const String *substr(int64_t i0, int64_t i1) const;

    std::size_t hash() const;

    struct Hash {
        std::size_t operator()(const String *s) const;
    };

    struct KeyEqual {
        bool operator()( const String *lhs, const String *rhs ) const;
    };

    const char *data;
    size_t count;
};

struct StyledString {
    StringStream _ss;
    StyledStream out;

    StyledString();
    StyledString(StreamStyleFunction ssf);

    static StyledString plain();
    const String *str() const;
    CppString cppstr() const;
};

typedef std::vector<const String *> Strings;

const String *vformat( const char *fmt, va_list va );

const String *format( const char *fmt, ...);

// computes the levenshtein distance between two strings
size_t distance(const String *_s, const String *_t);

int unescape_string(char *buf);
int escape_string(char *buf, const char *str, int strcount, const char *quote_chars);

} // namespace scopes

#endif // SCOPES_STRING_HPP