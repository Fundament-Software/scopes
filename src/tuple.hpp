/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_TUPLE_HPP
#define SCOPES_TUPLE_HPP

#include "sized_storage.hpp"
#include "argument.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// TUPLE TYPE
//------------------------------------------------------------------------------

struct TupleType : StorageType {
    static bool classof(const Type *T);

    TupleType(const Args &_values, bool _packed, size_t _alignment);

    void *getelementptr(void *src, size_t i) const;

    Any unpack(void *src, size_t i) const;

    const Type *type_at_index(size_t i) const;

    size_t field_index(Symbol name) const;

    Symbol field_name(size_t i) const;

    Args values;
    ArgTypes types;
    bool packed;
    std::vector<size_t> offsets;
};

//------------------------------------------------------------------------------

const Type *MixedTuple(const Args &values,
    bool packed = false, size_t alignment = 0);

const Type *Tuple(const ArgTypes &types,
    bool packed = false, size_t alignment = 0);

} // namespace scopes

#endif // SCOPES_TUPLE_HPP