/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_MATRIX_HPP
#define SCOPES_MATRIX_HPP

#include "sized_storage_type.hpp"

namespace scopes {

struct VectorType;

//------------------------------------------------------------------------------
// MATRIX TYPE
//------------------------------------------------------------------------------

struct MatrixType : ArrayLikeType {
    static bool classof(const Type *T);

    void stream_name(StyledStream &ss) const;
    MatrixType(const Type *_element_type, size_t _count);

    const VectorType *column_type() const;

    const Type *column_element_type;
    size_t row_count;
};

SCOPES_RESULT(const Type *) matrix_type(const Type *element_type, size_t count);

} // namespace scopes

#endif // SCOPES_MATRIX_HPP