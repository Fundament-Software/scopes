/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "sized_storage.hpp"

namespace scopes {

StorageType::StorageType(TypeKind kind) : Type(kind) {}

//------------------------------------------------------------------------------

SizedStorageType::SizedStorageType(TypeKind kind, const Type *_element_type, size_t _count)
    : StorageType(kind), element_type(_element_type), count(_count) {
    stride = size_of(element_type);
    size = stride * count;
    align = align_of(element_type);
}

void *SizedStorageType::getelementptr(void *src, size_t i) const {
    verify_range(i, count);
    return (void *)((char *)src + stride * i);
}

Any SizedStorageType::unpack(void *src, size_t i) const {
    return wrap_pointer(type_at_index(i), getelementptr(src, i));
}

const Type *SizedStorageType::type_at_index(size_t i) const {
    verify_range(i, count);
    return element_type;
}

} // namespace scopes
