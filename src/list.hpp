/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_LIST_HPP
#define SCOPES_LIST_HPP

#include "valueref.inc"

#include <stddef.h>
#include <cstddef>

namespace scopes {

struct Value;

//------------------------------------------------------------------------------
// LIST
//------------------------------------------------------------------------------

struct List {
protected:
    List(const ValueRef &_at, const List *_next, size_t _count);

public:
    ValueRef at;
    const List *next;
    size_t count;

    ValueRef first() const;

    static const List *from(const ValueRef &_at, const List *_next);

    static const List *from(ValueRef const *values, int N);


    static inline const List *from(const ValueRef &v0, const ValueRef &v1) {
        ValueRef values[] = { v0, v1 }; return from(values, 2); }

    template<unsigned N>
    static const List *from(ValueRef const (&values)[N]) {
        return from(values, N);
    }

    static const List *from_arglist() {
        return nullptr;
    }

    static const List *from_arglist(const ValueRef &last) {
        return from(last, nullptr);
    }

    template<class ... Args>
    static const List *from_arglist(const ValueRef &first, Args ... args) {
        return from(first, from_arglist(args ...));
    }

    static const List *join(const List *a, const List *b);

    // these only work with lists that aren't EOL

    struct KeyEqual {
        bool operator()( const List *lhs, const List *rhs ) const;
    };

    struct Hash {
        std::size_t operator()(const List *l) const;
    };

};

const List * const EOL = nullptr;

#if 0
// (a . (b . (c . (d . NIL)))) -> (d . (c . (b . (a . NIL))))
// this is the mutating version; input lists are modified, direction is inverted
const List *reverse_list_inplace(
    const List *l, const List *eol = EOL, const List *cat_to = EOL);
#endif
const List *reverse_list(
    const List *l, const List *eol = EOL, const List *cat_to = EOL);

} // namespace scopes

#endif // SCOPES_LIST_HPP