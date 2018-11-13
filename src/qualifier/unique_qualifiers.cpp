/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "unique_qualifiers.hpp"
#include "../error.hpp"
#include "../dyn_cast.inc"
#include "../hash.hpp"
#include "../qualifier.inc"

#include <algorithm>

namespace scopes {

//------------------------------------------------------------------------------

namespace ViewSet {
struct Hash {
    std::size_t operator()(const ViewQualifier *s) const {
        return s->prehash;
    }
};

struct KeyEqual {
    bool operator()( const ViewQualifier *lhs, const ViewQualifier *rhs ) const {
        return lhs->ids == rhs->ids;
    }
};
} // namespace ViewSet

static std::unordered_set<const ViewQualifier *, ViewSet::Hash, ViewSet::KeyEqual> views;

//------------------------------------------------------------------------------

MoveQualifier::MoveQualifier()
    : Qualifier((QualifierKind)Kind) {}

void MoveQualifier::stream_prefix(StyledStream &ss) const {
    ss << "†";
}

void MoveQualifier::stream_postfix(StyledStream &ss) const {
}

//------------------------------------------------------------------------------

MutateQualifier::MutateQualifier()
    : Qualifier((QualifierKind)Kind) {}

void MutateQualifier::stream_prefix(StyledStream &ss) const {
}

void MutateQualifier::stream_postfix(StyledStream &ss) const {
    ss << "!";
}

//------------------------------------------------------------------------------

ViewQualifier::ViewQualifier(const IDSet &_ids)
    : Qualifier((QualifierKind)Kind), ids(_ids) {
    for (auto entry : ids) {
        sorted_ids.push_back(entry);
    }
    std::sort(sorted_ids.begin(), sorted_ids.end());
    std::size_t h = 0;
    for (auto &&entry : sorted_ids) {
        h = hash2(h, std::hash<int>{}(entry));
    }
    prehash = h;
}

void ViewQualifier::stream_prefix(StyledStream &ss) const {
    for (int i = 0; i < sorted_ids.size(); ++i) {
        if (i > 0) ss << "|";
        ss << sorted_ids[i];
    }
    ss << ":";
}

void ViewQualifier::stream_postfix(StyledStream &ss) const {
}

//------------------------------------------------------------------------------

static const MoveQualifier *_move_qualifier = nullptr;
const Type * move_type(const Type *type) {
    if (has_qualifier<MoveQualifier>(type))
        return type;
    if (!_move_qualifier) {
        _move_qualifier = new MoveQualifier();
    }
    return qualify(type, { _move_qualifier });
}

static const MutateQualifier *_mutate_qualifier = nullptr;
const Type *mutate_type(const Type *type) {
    if (has_qualifier<MutateQualifier>(type))
        return type;
    if (!_mutate_qualifier) {
        _mutate_qualifier = new MutateQualifier();
    }
    return qualify(type, { _mutate_qualifier });
}

const Type * view_type(const Type *type, IDSet ids) {
    auto vt = try_qualifier<ViewQualifier>(type);
    if (vt) {
        for (auto entry : vt->ids) {
            ids.insert(entry);
        }
    }
    const ViewQualifier *result = nullptr;
    ViewQualifier key(ids);
    auto it = views.find(&key);
    if (it != views.end()) {
        result = *it;
    } else {
        result = new ViewQualifier(ids);
        views.insert(result);
    }
    return qualify(type, { result });
}

} // namespace scopes
