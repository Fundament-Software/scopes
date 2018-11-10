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

//------------------------------------------------------------------------------

struct Qualifier : Type {
    static bool classof(const Type *T);

    void stream_name(StyledStream &ss) const;

    Qualifier(TypeKind kind);
};

bool is_qualifier(const Type *T);

//------------------------------------------------------------------------------

typedef std::vector<const Qualifier *> Qualifiers;
typedef std::unordered_map<int, const Qualifier *> QualifierMap;

//------------------------------------------------------------------------------

struct QualifiedType : Type {
    static bool classof(const Type *T);

    void stream_name(StyledStream &ss) const;

    QualifiedType(const Type *type, const QualifierMap &qualifiers);

    const Type *type;
    QualifierMap qualifiers;
    Qualifiers sorted_qualifiers;
    std::size_t prehash;
};

//------------------------------------------------------------------------------

const Type *qualify(const Type *type, const Qualifiers &qualifiers);
const Type *find_qualifier(const Type *type, TypeKind kind);
const Type *strip_qualifiers(const Type *T);
const Type *strip_qualifier(const Type *T, TypeKind kind);

//------------------------------------------------------------------------------

} // namespace scopes

#endif // SCOPES_QUALIFIED_TYPE_HPP
