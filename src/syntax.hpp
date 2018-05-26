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

#ifndef SCOPES_SYNTAX_HPP
#define SCOPES_SYNTAX_HPP

#include "any.hpp"

namespace scopes {

struct Anchor;

//------------------------------------------------------------------------------
// SYNTAX OBJECTS
//------------------------------------------------------------------------------

struct Syntax {
protected:
    Syntax(const Anchor *_anchor, const Any &_datum, bool _quoted);

public:
    const Anchor *anchor;
    Any datum;
    bool quoted;

    static const Syntax *from(const Anchor *_anchor, const Any &_datum, bool quoted = false);

    static const Syntax *from_quoted(const Anchor *_anchor, const Any &_datum);
};

Any unsyntax(const Any &e);

Any maybe_unsyntax(const Any &e);

Any strip_syntax(Any e);

Any wrap_syntax(const Anchor *anchor, Any e, bool quoted = false);

StyledStream& operator<<(StyledStream& ost, const Syntax *value);

} // namespace scopes

#endif // SCOPES_SYNTAX_HPP