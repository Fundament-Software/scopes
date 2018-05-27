/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_ARRAY_HPP
#define SCOPES_ARRAY_HPP

#include "sized_storage.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// ARRAY TYPE
//------------------------------------------------------------------------------

struct ArrayType : SizedStorageType {
    static bool classof(const Type *T);

    ArrayType(const Type *_element_type, size_t _count);
};

//------------------------------------------------------------------------------

const Type *Array(const Type *element_type, size_t count);

} // namespace scopes

#endif // SCOPES_ARRAY_HPP