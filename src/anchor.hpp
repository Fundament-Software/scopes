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
    Anchor(Symbol _path, int _lineno, int _column, int _offset, const String *buffer);

public:
    Symbol path;
    int lineno;
    int column;
    int offset;
    const String *buffer;

    bool is_boring() const;
    bool is_same(const Anchor *other) const;

    static const Anchor *from(
        Symbol _path, int _lineno, int _column, int _offset = 0, const String *buffer = nullptr);
    static const Anchor *from(
        const std::unique_ptr<SourceFile> &file, int _lineno, int _column, int _offset = 0);

    StyledStream& stream(StyledStream& ost) const;

    StyledStream &stream_source_line(StyledStream &ost, const char *indent = "    ") const;
};

const Anchor *builtin_anchor();
const Anchor *unknown_anchor();

} // namespace scopes

#endif // SCOPES_ANCHOR_HPP