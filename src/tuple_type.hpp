/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_TUPLE_HPP
#define SCOPES_TUPLE_HPP

#include "sized_storage_type.hpp"
#include "result.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// TUPLE TYPE
//------------------------------------------------------------------------------

struct TupleType : StorageType {
    static bool classof(const Type *T);

    void stream_name(StyledStream &ss) const;
    TupleType(const KeyedTypes &_values, bool _packed, size_t _alignment);

    SCOPES_RESULT(void *) getelementptr(void *src, size_t i) const;

    SCOPES_RESULT(const Type *) type_at_index(size_t i) const;
    const Type *type_at_index_or_nothing(size_t i) const;

    size_t field_index(Symbol name) const;

    SCOPES_RESULT(Symbol) field_name(size_t i) const;

    KeyedTypes values;
    bool packed;
    bool explicit_alignment;
    std::vector<size_t> offsets;
};

//------------------------------------------------------------------------------

SCOPES_RESULT(const Type *) keyed_tuple_type(const KeyedTypes &values,
    bool packed = false, size_t alignment = 0);

SCOPES_RESULT(const Type *) tuple_type(const ArgTypes &types,
    bool packed = false, size_t alignment = 0);

} // namespace scopes

#endif // SCOPES_TUPLE_HPP