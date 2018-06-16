/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_VECTOR_HPP
#define SCOPES_VECTOR_HPP

#include "sized_storage.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// VECTOR TYPE
//------------------------------------------------------------------------------

struct VectorType : SizedStorageType {
    static bool classof(const Type *T);

    void stream_name(StyledStream &ss) const;
    VectorType(const Type *_element_type, size_t _count);
};

SCOPES_RESULT(const Type *) Vector(const Type *element_type, size_t count);

SCOPES_RESULT(void) verify_integer_vector(const Type *type);
SCOPES_RESULT(void) verify_real_vector(const Type *type);
SCOPES_RESULT(void) verify_bool_vector(const Type *type);
SCOPES_RESULT(void) verify_real_vector(const Type *type, size_t fixedsz);
SCOPES_RESULT(void) verify_vector_sizes(const Type *type1, const Type *type2);

} // namespace scopes

#endif // SCOPES_VECTOR_HPP