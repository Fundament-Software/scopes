/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_UNION_HPP
#define SCOPES_UNION_HPP

#include "sized_storage.hpp"
#include "argument.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// UNION TYPE
//------------------------------------------------------------------------------

struct UnionType : StorageType {
    static bool classof(const Type *T);

    UnionType(const Args &_values);

    Any unpack(void *src, size_t i) const;

    const Type *type_at_index(size_t i) const;

    size_t field_index(Symbol name) const;

    Symbol field_name(size_t i) const;

    Args values;
    ArgTypes types;
    size_t largest_field;
    const Type *tuple_type;
};

const Type *MixedUnion(const Args &values);

const Type *Union(const ArgTypes &types);

} // namespace scopes

#endif // SCOPES_UNION_HPP