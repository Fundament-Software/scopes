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

namespace MoveSet {
struct Hash {
    std::size_t operator()(const MoveType *s) const {
        return std::hash<const Type *>{}(s->type);
    }
};

struct KeyEqual {
    bool operator()( const MoveType *lhs, const MoveType *rhs ) const {
        return lhs->type == rhs->type;
    }
};
} // namespace MoveSet

static std::unordered_set<const MoveType *, MoveSet::Hash, MoveSet::KeyEqual> moves;

namespace ViewSet {
struct Hash {
    std::size_t operator()(const ViewType *s) const {
        return s->prehash;
    }
};

struct KeyEqual {
    bool operator()( const ViewType *lhs, const ViewType *rhs ) const {
        return lhs->type == rhs->type
            && lhs->ids == rhs->ids;
    }
};
} // namespace ViewSet

static std::unordered_set<const ViewType *, ViewSet::Hash, ViewSet::KeyEqual> views;

//------------------------------------------------------------------------------

MoveType::MoveType(const Type *type)
    : Qualifier(TK_Move, type) {}

void MoveType::stream_name(StyledStream &ss) const {
    ss << "†";
    stream_type_name(ss, type);
}

//------------------------------------------------------------------------------

ViewType::ViewType(const Type *type, const IDSet &_ids)
    : Qualifier(TK_View, type), ids(_ids) {
    for (auto entry : ids) {
        sorted_ids.push_back(entry);
    }
    std::sort(sorted_ids.begin(), sorted_ids.end());
    std::size_t h = std::hash<const Type *>{}(type);
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
    stream_type_name(ss, type);
}

//------------------------------------------------------------------------------

const Type * move_type(const Type *type) {
    if (has_qualifier<MoveType>(type))
        return type;
    MoveType key(type);
    auto it = moves.find(&key);
    if (it != moves.end())
        return *it;
    auto result = new MoveType(type);
    moves.insert(result);
    return result;
}

const Type * view_type(const Type *type, IDSet ids) {
    auto vt = try_qualifier<ViewType>(type);
    if (vt) {
        for (auto entry : vt->ids) {
            ids.insert(entry);
        }
        type = vt->type;
    }
    ViewType key(type, ids);
    auto it = views.find(&key);
    if (it != views.end())
        return *it;
    auto result = new ViewType(type, ids);
    views.insert(result);
    return result;
}

} // namespace scopes
