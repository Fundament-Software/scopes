/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_UNIQUE_TYPE_HPP
#define SCOPES_UNIQUE_TYPE_HPP

#include "type.hpp"

#include <vector>
#include <unordered_set>

namespace scopes {

//------------------------------------------------------------------------------

struct MoveType : Qualifier {
    static bool classof(const Type *T);

    void stream_name(StyledStream &ss) const;

    MoveType(const Type *type);
};

//------------------------------------------------------------------------------

typedef std::unordered_set<int> IDSet;
typedef std::vector<int> IDs;

struct ViewType : Qualifier {
    static bool classof(const Type *T);

    void stream_name(StyledStream &ss) const;

    ViewType(const Type *type, const IDSet &ids);

    IDSet ids;
    IDs sorted_ids;
    std::size_t prehash;
};

//------------------------------------------------------------------------------

const Type *move_type(const Type *type);
const Type *view_type(const Type *type, IDSet ids);

} // namespace scopes

#endif // SCOPES_UNIQUE_TYPE_HPP