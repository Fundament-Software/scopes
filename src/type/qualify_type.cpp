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
#include <cstring>

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
        if (lhs->type != rhs->type) return false;
        if (lhs->mask != rhs->mask) return false;
        for (int i = 0; i < QualifierCount; ++i) {
            if (lhs->qualifiers[i] != rhs->qualifiers[i])
                return false;
        }
        return true;
    }
};
} // namespace QualifySet

static std::unordered_set<const QualifyType *, QualifySet::Hash, QualifySet::KeyEqual> qualifys;

//------------------------------------------------------------------------------

void QualifyType::stream_name(StyledStream &ss) const {
    for (int i = QualifierCount; i-- > 0;) {
        auto entry = qualifiers[i];
        if (!entry)
            continue;
        switch(entry->kind()) {
#define T(NAME, BNAME, CLASS) \
        case NAME: cast<CLASS>(entry)->stream_prefix(ss); break;
SCOPES_QUALIFIER_KIND()
#undef T
        default: break;
        }
    }
    stream_type_name(ss, type);
    for (int i = 0; i < QualifierCount; ++i) {
        auto entry = qualifiers[i];
        if (!entry)
            continue;
        switch(entry->kind()) {
#define T(NAME, BNAME, CLASS) \
        case NAME: cast<CLASS>(entry)->stream_postfix(ss); break;
SCOPES_QUALIFIER_KIND()
#undef T
        default: break;
        }
    }
}

QualifyType::QualifyType(const Type *_type, const Qualifier * const *_qualifiers)
    : Type(TK_Qualify), type(_type), mask(0) {
    std::size_t h = std::hash<const Type *>{}(type);
    for (int i = 0; i < QualifierCount; ++i) {
        qualifiers[i] = _qualifiers[i];
        if (_qualifiers[i]) {
            mask |= 1 << i;
            h = hash2(h, std::hash<const Qualifier *>{}(qualifiers[i]));
        }
    }
    assert(mask);
    prehash = h;
}

//------------------------------------------------------------------------------

static const Type *_qualify(const Type *type, const Qualifier * const * quals) {
    QualifyType key(type, quals);
    auto it = qualifys.find(&key);
    if (it != qualifys.end())
        return *it;
    auto result = new QualifyType(type, quals);
    qualifys.insert(result);
    return result;
}

const Type *qualify(const Type *type, const Qualifiers &qualifiers) {
    if (qualifiers.empty())
        return type;
    const Qualifier *quals[QualifierCount];
    if (isa<QualifyType>(type)) {
        auto qt = cast<QualifyType>(type);
        for (int i = 0; i < QualifierCount; ++i) {
            quals[i] = qt->qualifiers[i];
        }
        type = qt->type;
    } else {
        std::memset(quals, 0, sizeof(quals));
    }
    for (auto q : qualifiers) {
        auto kind = q->kind();
        assert(kind < QualifierCount);
        quals[kind] = q;
    }
    return _qualify(type, quals);
}

const Type *copy_qualifiers(const Type *type, const Type *from) {
    auto qt = dyn_cast<QualifyType>(from);
    if (qt) {
        return _qualify(type, qt->qualifiers);
    }
    return type;
}

const Qualifier *find_qualifier(const Type *type, QualifierKind kind) {
    if (isa<QualifyType>(type)) {
        auto qt = cast<QualifyType>(type);
        assert(kind < QualifierCount);
        return qt->qualifiers[kind];
    }
    return nullptr;
}

bool has_qualifiers(const Type *T, uint32_t mask) {
    if (isa<QualifyType>(T)) {
        auto qt = cast<QualifyType>(T);
        return ((qt->mask & mask) == mask);
    }
    return false;
}

const Type *strip_qualifiers(const Type *T, uint32_t mask) {
    if (isa<QualifyType>(T)) {
        auto qt = cast<QualifyType>(T);
        if (!(qt->mask & mask))
            return T;
        uint32_t outmask = 0;
        const Qualifier *quals[QualifierCount];
        for (int i = 0; i < QualifierCount; ++i) {
            if ((mask & (1 << i)) || !qt->qualifiers[i]) {
                quals[i] = nullptr;
            } else {
                quals[i] = qt->qualifiers[i];
                outmask |= (1 << i);
            }
        }
        if (!outmask)
            return qt->type;
        else
            return _qualify(qt->type, quals);
    }
    return T;
}

const Type *strip_qualifier(const Type *T, QualifierKind kind) {
    return strip_qualifiers(T, 1 << kind);
}

} // namespace scopes