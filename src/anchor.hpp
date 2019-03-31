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
    Anchor(SourceFile *_file, int _lineno, int _column, int _offset);

public:
    SourceFile *file;
    int lineno;
    int column;
    int offset;

    Symbol path() const;

    bool is_boring() const;
    bool is_same(const Anchor *other) const;

    static const Anchor *from(
        SourceFile *_file, int _lineno, int _column, int _offset = 0);

    StyledStream& stream(StyledStream& ost) const;

    StyledStream &stream_source_line(StyledStream &ost, const char *indent = "    ") const;
};

const Anchor *builtin_anchor();
const Anchor *unknown_anchor();

} // namespace scopes

#endif // SCOPES_ANCHOR_HPP