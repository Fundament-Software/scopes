/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
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