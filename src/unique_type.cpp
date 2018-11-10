/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "unique_type.hpp"
#include "error.hpp"
#include "dyn_cast.inc"
#include "hash.hpp"
#include "qualifier.inc"

#include <algorithm>

namespace scopes {

//------------------------------------------------------------------------------

namespace ViewSet {
struct Hash {
    std::size_t operator()(const ViewType *s) const {
        return s->prehash;
    }
};

struct KeyEqual {
    bool operator()( const ViewType *lhs, const ViewType *rhs ) const {
        return lhs->ids == rhs->ids;
    }
};
} // namespace ViewSet

static std::unordered_set<const ViewType *, ViewSet::Hash, ViewSet::KeyEqual> views;

//------------------------------------------------------------------------------

MoveType::MoveType()
    : Qualifier(TK_Move) {}

void MoveType::stream_name(StyledStream &ss) const {
    ss << "†";
}

//------------------------------------------------------------------------------

MutatedType::MutatedType()
    : Qualifier(TK_Mutated) {}

void MutatedType::stream_name(StyledStream &ss) const {
    ss << "!";
}

//------------------------------------------------------------------------------

ViewType::ViewType(const IDSet &_ids)
    : Qualifier(TK_View), ids(_ids) {
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

void ViewType::stream_name(StyledStream &ss) const {
    ss << "%";
    for (int i = 0; i < sorted_ids.size(); ++i) {
        if (i > 0) ss << "|";
        ss << sorted_ids[i];
    }
    ss << ":";
}

//------------------------------------------------------------------------------

static const MoveType *_move_type = nullptr;
const Type * move_type(const Type *type) {
    if (has_qualifier<MoveType>(type))
        return type;
    if (!_move_type) {
        _move_type = new MoveType();
    }
    return qualify(type, { _move_type });
}

static const MutatedType *_mutated_type = nullptr;
const Type *mutated_type(const Type *type) {
    if (has_qualifier<MutatedType>(type))
        return type;
    if (!_mutated_type) {
        _mutated_type = new MutatedType();
    }
    return qualify(type, { _mutated_type });
}

const Type * view_type(const Type *type, IDSet ids) {
    auto vt = try_qualifier<ViewType>(type);
    if (vt) {
        for (auto entry : vt->ids) {
            ids.insert(entry);
        }
    }
    const ViewType *result = nullptr;
    ViewType key(ids);
    auto it = views.find(&key);
    if (it != views.end()) {
        result = *it;
    } else {
        result = new ViewType(ids);
        views.insert(result);
    }
    return qualify(type, { result });
}

} // namespace scopes
