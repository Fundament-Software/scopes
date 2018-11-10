/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "sized_storage_type.hpp"

namespace scopes {

StorageType::StorageType(TypeKind kind) : Type(kind) {}

//------------------------------------------------------------------------------

SizedStorageType::SizedStorageType(TypeKind kind, const Type *_element_type, size_t _count)
    : StorageType(kind), element_type(_element_type), count(_count) {
    stride = size_of(element_type).assert_ok();
    size = stride * count;
    align = align_of(element_type).assert_ok();
}

SCOPES_RESULT(void *) SizedStorageType::getelementptr(void *src, size_t i) const {
    SCOPES_RESULT_TYPE(void *);
    SCOPES_CHECK_RESULT(verify_range(i, count));
    return (void *)((char *)src + stride * i);
}

SCOPES_RESULT(const Type *) SizedStorageType::type_at_index(size_t i) const {
    SCOPES_RESULT_TYPE(const Type *);
    SCOPES_CHECK_RESULT(verify_range(i, count));
    return element_type;
}

} // namespace scopes
