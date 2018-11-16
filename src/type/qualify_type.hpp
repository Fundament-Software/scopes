/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_QUALIFIED_TYPE_HPP
#define SCOPES_QUALIFIED_TYPE_HPP

#include "../type.hpp"

#include <vector>
#include <unordered_map>

namespace scopes {

// key=mutate(move(view(refer(T))))
#define SCOPES_QUALIFIER_KIND() \
    T(QK_Refer, "qualifier-kind-refer", ReferQualifier) \
    T(QK_View, "qualifier-kind-view", ViewQualifier) \
    T(QK_Move, "qualifier-kind-move", MoveQualifier) \
    T(QK_Mutate, "qualifier-kind-mutate", MutateQualifier) \
    T(QK_Key, "qualifier-kind-key", KeyQualifier) \

enum QualifierKind {
#define T(NAME, BNAME, CLASS) \
    NAME,
    SCOPES_QUALIFIER_KIND()
#undef T
    QualifierCount
};

enum QualifierMask {
    QM_UniquenessTags =
          (1 << QK_View)
        | (1 << QK_Move)
        | (1 << QK_Mutate),
    QM_Annotations = QM_UniquenessTags
        | (1 << QK_Key),
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

//------------------------------------------------------------------------------

struct QualifyType : Type {
    static bool classof(const Type *T);

    void stream_name(StyledStream &ss) const;

    QualifyType(const Type *type, const Qualifier * const *qualifiers);

    const Type *type;
    // bits of available qualifiers are set
    uint32_t mask;
    const Qualifier *qualifiers[QualifierCount];
    std::size_t prehash;
};

//------------------------------------------------------------------------------

const Type *qualify(const Type *type, const Qualifiers &qualifiers);
const Qualifier *find_qualifier(const Type *type, QualifierKind kind);
const Type *strip_qualifiers(const Type *T, uint32_t mask = 0xffffffff);
bool has_qualifiers(const Type *T, uint32_t mask = ((1 << QualifierCount) - 1));
const Type *strip_qualifier(const Type *T, QualifierKind kind);
const Type *copy_qualifiers(const Type *type, const Type *from);

//------------------------------------------------------------------------------

} // namespace scopes

#endif // SCOPES_QUALIFIED_TYPE_HPP
