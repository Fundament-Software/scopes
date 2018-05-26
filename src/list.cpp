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

#include "list.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// LIST
//------------------------------------------------------------------------------

List::List(const Any &_at, const List *_next, size_t _count) :
    at(_at),
    next(_next),
    count(_count) {}

Any List::first() const {
    if (this == EOL) {
        return none;
    } else {
        return at;
    }
}

const List *List::from(const Any &_at, const List *_next) {
    return new List(_at, _next, (_next != EOL)?(_next->count + 1):1);
}

const List *List::from(const Any *values, int N) {
    const List *list = EOL;
    for (int i = N - 1; i >= 0; --i) {
        list = from(values[i], list);
    }
    return list;
}

const List *List::join(const List *la, const List *lb) {
    const List *l = lb;
    while (la != EOL) {
        l = List::from(la->at, l);
        la = la->next;
    }
    return reverse_list_inplace(l, lb, lb);
}

//------------------------------------------------------------------------------

// (a . (b . (c . (d . NIL)))) -> (d . (c . (b . (a . NIL))))
// this is the mutating version; input lists are modified, direction is inverted
const List *reverse_list_inplace(
    const List *l, const List *eol, const List *cat_to) {
    const List *next = cat_to;
    size_t count = 0;
    if (cat_to != EOL) {
        count = cat_to->count;
    }
    while (l != eol) {
        count = count + 1;
        const List *iternext = l->next;
        const_cast<List *>(l)->next = next;
        const_cast<List *>(l)->count = count;
        next = l;
        l = iternext;
    }
    return next;
}

} // namespace scopes
