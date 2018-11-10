/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_QUALIFIED_TYPE_HPP
#define SCOPES_QUALIFIED_TYPE_HPP

#include "type.hpp"

#include <vector>
#include <unordered_map>

namespace scopes {

#define SCOPES_QUALIFIER_KIND() \
    T(QK_Key, "qualifier-kind-key", KeyQualifier) \
    T(QK_Move, "qualifier-kind-move", MoveQualifier) \
    T(QK_Mutate, "qualifier-kind-mutate", MutateQualifier) \
    T(QK_View, "qualifier-kind-view", ViewQualifier)

enum QualifierKind {
#define T(NAME, BNAME, CLASS) \
    NAME,
    SCOPES_QUALIFIER_KIND()
#undef T
};

//------------------------------------------------------------------------------

struct Qualifier {
    QualifierKind kind() const;

    Qualifier(QualifierKind kind);
    Qualifier(const Qualifier &other) = delete;

private:
    const QualifierKind _kind;
};

//------------------------------------------------------------------------------

typedef std::vector<const Qualifier *> Qualifiers;
typedef std::unordered_map<int, const Qualifier *> QualifierMap;

//------------------------------------------------------------------------------

struct QualifyType : Type {
    static bool classof(const Type *T);

    void stream_name(StyledStream &ss) const;

    QualifyType(const Type *type, const QualifierMap &qualifiers);

    const Type *type;
    QualifierMap qualifiers;
    Qualifiers sorted_qualifiers;
    std::size_t prehash;
};

//------------------------------------------------------------------------------

const Type *qualify(const Type *type, const Qualifiers &qualifiers);
const Qualifier *find_qualifier(const Type *type, QualifierKind kind);
const Type *strip_qualifiers(const Type *T);
const Type *strip_qualifier(const Type *T, QualifierKind kind);

//------------------------------------------------------------------------------

} // namespace scopes

#endif // SCOPES_QUALIFIED_TYPE_HPP
