/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_UNIQUE_TYPE_HPP
#define SCOPES_UNIQUE_TYPE_HPP

#include "type.hpp"
#include "qualify_type.hpp"

#include <vector>
#include <unordered_set>

namespace scopes {

//------------------------------------------------------------------------------

struct MoveQualifier : Qualifier {
    enum { Kind = QK_Move };
    static bool classof(const Qualifier *T);

    void stream_prefix(StyledStream &ss) const;
    void stream_postfix(StyledStream &ss) const;

    MoveQualifier();
};

//------------------------------------------------------------------------------

struct MutateQualifier : Qualifier {
    enum { Kind = QK_Mutate };
    static bool classof(const Qualifier *T);

    void stream_prefix(StyledStream &ss) const;
    void stream_postfix(StyledStream &ss) const;

    MutateQualifier();
};

//------------------------------------------------------------------------------

typedef std::unordered_set<int> IDSet;
typedef std::vector<int> IDs;

struct ViewQualifier : Qualifier {
    enum { Kind = QK_View };
    static bool classof(const Qualifier *T);

    void stream_prefix(StyledStream &ss) const;
    void stream_postfix(StyledStream &ss) const;

    ViewQualifier(const IDSet &ids);

    IDSet ids;
    IDs sorted_ids;
    std::size_t prehash;
};

//------------------------------------------------------------------------------

const Type *move_type(const Type *type);
const Type *mutate_type(const Type *type);
const Type *view_type(const Type *type, IDSet ids);

} // namespace scopes

#endif // SCOPES_UNIQUE_TYPE_HPP