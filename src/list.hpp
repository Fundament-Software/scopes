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

#ifndef SCOPES_LIST_HPP
#define SCOPES_LIST_HPP

#include "any.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// LIST
//------------------------------------------------------------------------------

struct List {
protected:
    List(const Any &_at, const List *_next, size_t _count);

public:
    Any at;
    const List *next;
    size_t count;

    Any first() const;

    static const List *from(const Any &_at, const List *_next);

    static const List *from(const Any *values, int N);

    template<unsigned N>
    static const List *from(const Any (&values)[N]) {
        return from(values, N);
    }

    static const List *join(const List *a, const List *b);
};

const List * const EOL = nullptr;

// (a . (b . (c . (d . NIL)))) -> (d . (c . (b . (a . NIL))))
// this is the mutating version; input lists are modified, direction is inverted
const List *reverse_list_inplace(
    const List *l, const List *eol = EOL, const List *cat_to = EOL);

StyledStream& operator<<(StyledStream& ost, const List *list);

} // namespace scopes

#endif // SCOPES_LIST_HPP