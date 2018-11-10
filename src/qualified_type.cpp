/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "qualified_type.hpp"
#include "hash.hpp"
#include "dyn_cast.inc"

#include <algorithm>
#include <unordered_set>

namespace scopes {

//------------------------------------------------------------------------------
// QUALIFIER
//------------------------------------------------------------------------------

bool Qualifier::classof(const Type *T) {
    switch (T->kind()) {
    case TK_Keyed:
    case TK_Move:
    case TK_View:
    case TK_Mutated:
        return true;
    default: return false;
    }
}

Qualifier::Qualifier(TypeKind kind)
    : Type(kind)
{}

bool is_qualifier(const Type *T) {
    return isa<Qualifier>(T);
}

//------------------------------------------------------------------------------
// QUALIFIED
//------------------------------------------------------------------------------

namespace QualifiedSet {
struct Hash {
    std::size_t operator()(const QualifiedType *s) const {
        return s->prehash;
    }
};

struct KeyEqual {
    bool operator()( const QualifiedType *lhs, const QualifiedType *rhs ) const {
        return lhs->type == rhs->type
            && lhs->sorted_qualifiers == rhs->sorted_qualifiers;
    }
};
} // namespace QualifiedSet

static std::unordered_set<const QualifiedType *, QualifiedSet::Hash, QualifiedSet::KeyEqual> qualifieds;

//------------------------------------------------------------------------------

bool prefix_qualifier_name(TypeKind kind) {
    switch(kind) {
    case TK_Mutated: return false;
    default: break;
    }
    return true;
}

void QualifiedType::stream_name(StyledStream &ss) const {
    for (auto entry : sorted_qualifiers) {
        if (prefix_qualifier_name(entry->kind()))
            stream_type_name(ss, entry);
    }
    stream_type_name(ss, type);
    for (auto entry : sorted_qualifiers) {
        if (!prefix_qualifier_name(entry->kind()))
            stream_type_name(ss, entry);
    }
}

bool comp_qualifier(const Qualifier *a, const Qualifier *b) {
    return a->kind() < b->kind();
}

QualifiedType::QualifiedType(const Type *_type, const QualifierMap &_qualifiers)
    : Type(TK_Qualified), type(_type), qualifiers(_qualifiers) {
    for (auto entry : qualifiers) {
        sorted_qualifiers.push_back(entry.second);
    }
    std::sort(sorted_qualifiers.begin(), sorted_qualifiers.end(), comp_qualifier);
    std::size_t h = std::hash<const Type *>{}(type);
    for (auto &&entry : sorted_qualifiers) {
        h = hash2(h, std::hash<const Qualifier *>{}(entry));
    }
    prehash = h;
}

//------------------------------------------------------------------------------

const Type *qualify(const Type *type, const Qualifiers &qualifiers) {
    if (qualifiers.empty())
        return type;
    QualifierMap map;
    for (auto q : qualifiers) {
        map.insert({q->kind(), q});
    }
    if (isa<QualifiedType>(type)) {
        auto qt = cast<QualifiedType>(type);
        for (auto q : qt->sorted_qualifiers) {
            map.insert({q->kind(), q});
        }
        type = qt->type;
    }
    QualifiedType key(type, map);
    auto it = qualifieds.find(&key);
    if (it != qualifieds.end())
        return *it;
    auto result = new QualifiedType(type, map);
    qualifieds.insert(result);
    return result;
}

const Type *find_qualifier(const Type *type, TypeKind kind) {
    if (isa<QualifiedType>(type)) {
        auto qt = cast<QualifiedType>(type);
        auto it = qt->qualifiers.find(kind);
        if (it != qt->qualifiers.end())
            return it->second;
    }
    return nullptr;
}

const Type *strip_qualifiers(const Type *T) {
    if (isa<QualifiedType>(T))
        return cast<QualifiedType>(T)->type;
    return T;
}

const Type *strip_qualifier(const Type *T, TypeKind kind) {
    if (isa<QualifiedType>(T)) {
        auto qt = cast<QualifiedType>(T);
        Qualifiers qual;
        for (auto key : qt->qualifiers) {
            if (key.first != kind) {
                qual.push_back(key.second);
            }
        }
        return qualify(qt->type, qual);
    }
    return T;
}

} // namespace scopes