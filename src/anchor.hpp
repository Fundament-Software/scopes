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

    static const Anchor *from(
        SourceFile *_file, int _lineno, int _column, int _offset = 0);

    StyledStream& stream(StyledStream& ost) const;

    StyledStream &stream_source_line(StyledStream &ost, const char *indent = "    ") const;
};

StyledStream& operator<<(StyledStream& ost, const Anchor *anchor);

} // namespace scopes

#endif // SCOPES_ANCHOR_HPP