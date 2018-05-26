/*
Scopes Compiler
Copyright (c) 2016, 2017, 2018 Leonard Ritter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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
