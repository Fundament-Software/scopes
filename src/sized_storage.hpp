/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_SIZED_STORAGE_HPP
#define SCOPES_SIZED_STORAGE_HPP

#include "type.hpp"

namespace scopes {

struct StorageType : Type {

    StorageType(TypeKind kind);

    size_t size;
    size_t align;
};

//------------------------------------------------------------------------------

struct SizedStorageType : StorageType {
    SizedStorageType(TypeKind kind, const Type *_element_type, size_t _count);

    void *getelementptr(void *src, size_t i) const;

    Any unpack(void *src, size_t i) const;

    const Type *type_at_index(size_t i) const;

    const Type *element_type;
    size_t count;
    size_t stride;
};

} // namespace scopes

#endif // SCOPES_SIZED_STORAGE_HPP