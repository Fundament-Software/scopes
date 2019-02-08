/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_ANCHOR_HPP
#define SCOPES_ANCHOR_HPP

#include "symbol.hpp"

namespace scopes {

struct StyledStream;
struct SourceFile;

//------------------------------------------------------------------------------
// ANCHOR
//------------------------------------------------------------------------------

struct Anchor {
protected:
    Anchor(SourceFile *_file, int _lineno, int _column, int _offset, const Anchor *_next);

public:
    SourceFile *file;
    int lineno;
    int column;
    int offset;
    const Anchor *next;

    Symbol path() const;

    bool is_same(const Anchor *other) const;

    static const Anchor *from(
        SourceFile *_file, int _lineno, int _column, int _offset = 0, const Anchor *_next = nullptr);

    StyledStream& stream(StyledStream& ost) const;

    StyledStream &stream_source_line(StyledStream &ost, const char *indent = "    ") const;
};

} // namespace scopes

#endif // SCOPES_ANCHOR_HPP