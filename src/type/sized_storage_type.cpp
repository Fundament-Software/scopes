/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "sized_storage_type.hpp"

namespace scopes {

CompositeType::CompositeType(TypeKind kind) : Type(kind) {}

//------------------------------------------------------------------------------

bool ArrayLikeType::classof(const Type *T) {
    switch(T->kind()) {
    case TK_Array:
    case TK_Vector:
        return true;
    default: break;
    }
    return false;
}

ArrayLikeType::ArrayLikeType(TypeKind kind, const Type *_element_type, size_t count)
    : CompositeType(kind), element_type(_element_type), _count(count) {
    stride = size_of(element_type).assert_ok();
}

SCOPES_RESULT(void *) ArrayLikeType::getelementptr(void *src, size_t i) const {
    SCOPES_RESULT_TYPE(void *);
    SCOPES_CHECK_RESULT(verify_range(i, _count));
    return (void *)((char *)src + stride * i);
}

SCOPES_RESULT(const Type *) ArrayLikeType::type_at_index(size_t i) const {
    SCOPES_RESULT_TYPE(const Type *);
    SCOPES_CHECK_RESULT(verify_range(i, _count));
    return element_type;
}

size_t ArrayLikeType::count() const {
    return is_unsized()?0:_count;
}

bool ArrayLikeType::is_unsized() const {
    return _count == UNSIZED_COUNT;
}

//------------------------------------------------------------------------------

bool TupleLikeType::classof(const Type *T) {
    switch(T->kind()) {
    case TK_Tuple:
        return true;
    default: break;
    }
    return false;
}

bool TupleLikeType::is_plain() const { return _is_plain; }

TupleLikeType::TupleLikeType(TypeKind kind, const Types &_values)
    : CompositeType(kind), values(_values), _is_plain(all_plain(_values))
{
}

//------------------------------------------------------------------------------

} // namespace scopes
