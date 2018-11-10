/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_UNION_HPP
#define SCOPES_UNION_HPP

#include "sized_storage_type.hpp"
#include "../result.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// UNION TYPE
//------------------------------------------------------------------------------

struct UnionType : StorageType {
    static bool classof(const Type *T);

    void stream_name(StyledStream &ss) const;
    UnionType(const ArgTypes &_values);

    SCOPES_RESULT(const Type *) type_at_index(size_t i) const;

    size_t field_index(Symbol name) const;

    SCOPES_RESULT(Symbol) field_name(size_t i) const;

    ArgTypes values;
    size_t largest_field;
    const Type *tuple_type;
};

SCOPES_RESULT(const Type *) union_type(const ArgTypes &types);

} // namespace scopes

#endif // SCOPES_UNION_HPP