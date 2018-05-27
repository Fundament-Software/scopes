/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "anchor.hpp"
#include "source_file.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// ANCHOR
//------------------------------------------------------------------------------

Anchor::Anchor(SourceFile *_file, int _lineno, int _column, int _offset) :
    file(_file),
    lineno(_lineno),
    column(_column),
    offset(_offset) {}

Symbol Anchor::path() const {
    return file->path;
}

const Anchor *Anchor::from(
    SourceFile *_file, int _lineno, int _column, int _offset) {
    return new Anchor(_file, _lineno, _column, _offset);
}

StyledStream& Anchor::stream(StyledStream& ost) const {
    ost << Style_Location;
    auto ss = StyledStream::plain(ost);
    ss << path().name()->data << ":" << lineno << ":" << column << ":";
    ost << Style_None;
    return ost;
}

StyledStream &Anchor::stream_source_line(StyledStream &ost, const char *indent) const {
    file->stream(ost, offset, indent);
    return ost;
}

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ost, const Anchor *anchor) {
    return anchor->stream(ost);
}

} // namespace scopes
