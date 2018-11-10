/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "qualify_type.hpp"
#include "../hash.hpp"
#include "../dyn_cast.inc"
#include "../qualifiers.hpp"

#include <algorithm>
#include <unordered_set>

namespace scopes {

#define T(NAME, BNAME, CLASS) \
    bool CLASS::classof(const Qualifier *T) { \
        return T->kind() == NAME; \
    }
SCOPES_QUALIFIER_KIND()
#undef T

//------------------------------------------------------------------------------
// QUALIFIER
//------------------------------------------------------------------------------

QualifierKind Qualifier::kind() const { return _kind; }

Qualifier::Qualifier(QualifierKind kind)
    : _kind(kind) {}

//------------------------------------------------------------------------------
// QUALIFY
//------------------------------------------------------------------------------

namespace QualifySet {
struct Hash {
    std::size_t operator()(const QualifyType *s) const {
        return s->prehash;
    }
};

struct KeyEqual {
    bool operator()( const QualifyType *lhs, const QualifyType *rhs ) const {
        return lhs->type == rhs->type
            && lhs->sorted_qualifiers == rhs->sorted_qualifiers;
    }
};
} // namespace QualifySet

static std::unordered_set<const QualifyType *, QualifySet::Hash, QualifySet::KeyEqual> qualifys;

//------------------------------------------------------------------------------

void QualifyType::stream_name(StyledStream &ss) const {
    for (auto entry : sorted_qualifiers) {
        switch(entry->kind()) {
#define T(NAME, BNAME, CLASS) \
        case NAME: cast<CLASS>(entry)->stream_prefix(ss); break;
SCOPES_QUALIFIER_KIND()
#undef T
        }
    }
    stream_type_name(ss, type);
    for (auto entry : sorted_qualifiers) {
        switch(entry->kind()) {
#define T(NAME, BNAME, CLASS) \
        case NAME: cast<CLASS>(entry)->stream_postfix(ss); break;
SCOPES_QUALIFIER_KIND()
#undef T
        }
    }
}

bool comp_qualifier(const Qualifier *a, const Qualifier *b) {
    return a->kind() < b->kind();
}

QualifyType::QualifyType(const Type *_type, const QualifierMap &_qualifiers)
    : Type(TK_Qualify), type(_type), qualifiers(_qualifiers) {
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
    if (isa<QualifyType>(type)) {
        auto qt = cast<QualifyType>(type);
        for (auto q : qt->sorted_qualifiers) {
            map.insert({q->kind(), q});
        }
        type = qt->type;
    }
    QualifyType key(type, map);
    auto it = qualifys.find(&key);
    if (it != qualifys.end())
        return *it;
    auto result = new QualifyType(type, map);
    qualifys.insert(result);
    return result;
}

const Qualifier *find_qualifier(const Type *type, QualifierKind kind) {
    if (isa<QualifyType>(type)) {
        auto qt = cast<QualifyType>(type);
        auto it = qt->qualifiers.find(kind);
        if (it != qt->qualifiers.end())
            return it->second;
    }
    return nullptr;
}

const Type *strip_qualifiers(const Type *T) {
    if (isa<QualifyType>(T))
        return cast<QualifyType>(T)->type;
    return T;
}

const Type *strip_qualifier(const Type *T, QualifierKind kind) {
    if (isa<QualifyType>(T)) {
        auto qt = cast<QualifyType>(T);
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