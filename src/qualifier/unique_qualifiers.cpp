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

namespace UniqueSet {
struct Hash {
    std::size_t operator()(const UniqueQualifier *s) const {
        return std::hash<int>{}(s->id);
    }
};

struct KeyEqual {
    bool operator()( const UniqueQualifier *lhs, const UniqueQualifier *rhs ) const {
        return lhs->id == rhs->id;
    }
};
} // namespace ViewSet

static std::unordered_set<const UniqueQualifier *, UniqueSet::Hash, UniqueSet::KeyEqual> uniques;

//------------------------------------------------------------------------------

MutateQualifier::MutateQualifier()
    : Qualifier((QualifierKind)Kind) {}

void MutateQualifier::stream_prefix(StyledStream &ss) const {
}

void MutateQualifier::stream_postfix(StyledStream &ss) const {
    ss << "!";
}

//------------------------------------------------------------------------------

void stream_id(StyledStream &ss, int id) {
    if (id >= 0) {
        ss << id;
    } else if (id < LastUniqueOutput) {
        ss << "E" << -id+LastUniqueOutput;
    } else if (id < 0) {
        ss << "R" << -id;
    }
}

//------------------------------------------------------------------------------

void map_unique_id(ID2SetMap &idmap, int fromid, int toid) {
    auto result = idmap.insert({fromid, { toid }});
    if (!result.second) {
        result.first->second.insert(toid);
    }
}

void dump_idmap(const ID2SetMap &idmap) {
    StyledStream ss;
    for (auto && entry : idmap) {
        ss << entry.first << ": ";
        for (auto && id : entry.second) {
            ss << id << " ";
        }
        ss << std::endl;
    }
    ss << idmap.size() << " entries" << std::endl;
}

IDSet difference_idset(const IDSet &a, const IDSet &b) {
    IDSet c;
    c.reserve(a.size());
    for (auto id : a) {
        assert(id);
        if (!b.count(id))
            c.insert(id);
    }
    return c;
}

IDSet intersect_idset(const IDSet &a, const IDSet &b) {
    IDSet c;
    c.reserve(std::min(a.size(), b.size()));
    for (auto id : a) {
        assert(id);
        if (b.count(id))
            c.insert(id);
    }
    return c;
}

IDSet union_idset(const IDSet &a, const IDSet &b) {
    IDSet c;
    c.reserve(std::max(a.size(), b.size()));
    for (auto id : a) {
        assert(id);
        c.insert(id);
    }
    for (auto id : b) {
        assert(id);
        c.insert(id);
    }
    return c;
}

void dump_idset(const IDSet &a) {
    StyledStream ss;
    for (auto id : a) {
        ss << id << " ";
    }
    ss << std::endl;
}

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
    ss << "(viewof ";
}

void ViewQualifier::stream_postfix(StyledStream &ss) const {
    if (!sorted_ids.empty()) {
        for (int i = 0; i < sorted_ids.size(); ++i) {
            ss << " ";
            stream_id(ss, sorted_ids[i]);
        }
    }
    ss << ")";
}

//------------------------------------------------------------------------------

void UniqueQualifier::stream_prefix(StyledStream &ss) const {
    ss << "(uniqueof ";
}

void UniqueQualifier::stream_postfix(StyledStream &ss) const {
    ss << " ";
    stream_id(ss, id);
    ss << ")";
}

UniqueQualifier::UniqueQualifier(int _id)
    : Qualifier((QualifierKind)Kind), id(_id) {
}

//------------------------------------------------------------------------------

bool is_view(const Type *T) {
    return has_qualifier<ViewQualifier>(T);
}

const Type *strip_view(const Type *T) {
    return strip_qualifier<ViewQualifier>(T);
}

const ViewQualifier *get_view(const Type *T) {
    return get_qualifier<ViewQualifier>(T);
}

const ViewQualifier *try_view(const Type *T) {
    return try_qualifier<ViewQualifier>(T);
}

//------------------------------------------------------------------------------

bool is_unique(const Type *T) {
    return has_qualifier<UniqueQualifier>(T);
}

bool is_movable(const Type *T) {
    return is_unique(T) || is_plain(T);
}

const Type *strip_unique(const Type *T) {
    return strip_qualifier<UniqueQualifier>(T);
}

const UniqueQualifier *get_unique(const Type *T) {
    return get_qualifier<UniqueQualifier>(T);
}

const UniqueQualifier *try_unique(const Type *T) {
    return try_qualifier<UniqueQualifier>(T);
}

//------------------------------------------------------------------------------

const Type *strip_lifetime(const Type *T) {
    return strip_qualifiers(T, QM_UniquenessTags);
}

//------------------------------------------------------------------------------

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
    auto ut = try_qualifier<UniqueQualifier>(type);
    if (ut) {
        ids.insert(ut->id);
        type = strip_qualifier<UniqueQualifier>(type);
    }
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

const Type * unique_type(const Type *type, int id) {
    type = strip_qualifier<ViewQualifier>(type);
    const UniqueQualifier *result = nullptr;
    UniqueQualifier key(id);
    auto it = uniques.find(&key);
    if (it != uniques.end()) {
        result = *it;
    } else {
        result = new UniqueQualifier(id);
        uniques.insert(result);
    }
    return qualify(type, { result });
}

} // namespace scopes
