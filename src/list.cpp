/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "list.hpp"
#include "hash.hpp"
#include "value.hpp"
#include "error.hpp"
#include "globals.hpp"

#include "absl/container/flat_hash_set.h"

namespace scopes {

//------------------------------------------------------------------------------
// LIST
//------------------------------------------------------------------------------

bool List::KeyEqual::operator()( const List *lhs, const List *rhs ) const {
    if (lhs->next != rhs->next)
        return false;
    return lhs->at == rhs->at;
}

std::size_t List::Hash::operator()(const List *l) const {
    return hash2(
        hash2(
            std::hash<const Anchor *>{}(l->at.anchor()),
            std::hash<Value *>{}(l->at.unref())
        ), std::hash<const List *>{}(l->next));
}

static absl::flat_hash_set<const List *, List::Hash, List::KeyEqual> list_map;

List::List(const ValueRef &_at, const List *_next, size_t count) :
    at(_at),
    next(_next),
    _count(count) {}

ValueRef List::first() const {
    if (this == EOL) {
        return g_none;
    } else {
        return at;
    }
}

const List *List::from(const ValueRef &_at, const List *_next) {
    List list(_at, _next, 0);
    auto it = list_map.find(&list);
    if (it != list_map.end()) {
        return *it;
    }
    const List *l = new List(_at, _next, (_next != EOL)?(List::count(_next) + 1):1);
    list_map.insert(l);
    return l;
}

size_t List::count(const List *l) {
    return l?l->_count:0;
}

const List *List::from(ValueRef const *values, int N) {
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
    //return reverse_list_inplace(l, lb, lb);
    return reverse_list(l, lb, lb);
}

//------------------------------------------------------------------------------

#if 0
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
#endif

const List *reverse_list(
    const List *l, const List *eol, const List *cat_to) {
    const List *next = cat_to;
    while (l != eol) {
        next = List::from(l->at, next);
        l = l->next;
    }
    return next;
}

} // namespace scopes
