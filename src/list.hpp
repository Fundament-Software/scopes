/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_LIST_HPP
#define SCOPES_LIST_HPP

#include <stddef.h>
#include <cstddef>

namespace scopes {

struct Value;

//------------------------------------------------------------------------------
// LIST
//------------------------------------------------------------------------------

struct List {
protected:
    List(Value *_at, const List *_next, size_t _count);

public:
    Value *at;
    const List *next;
    size_t count;

    Value *first() const;

    static const List *from(Value *_at, const List *_next);

    static const List *from(Value * const *values, int N);

    template<unsigned N>
    static const List *from(Value * const (&values)[N]) {
        return from(values, N);
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