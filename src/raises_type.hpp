/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_RAISES_HPP
#define SCOPES_RAISES_HPP

#include "type.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// TYPENAME
//------------------------------------------------------------------------------

struct RaisesType : Type {
    void stream_name(StyledStream &ss) const;
    static bool classof(const Type *T);

    RaisesType(const Type *except_type, const Type *result_type);

    const Type *except_type;
    const Type *result_type;
};

const Type *raises_type(const Type *except_type, const Type *result_type);
// assumes the exception type to be Error
const Type *raises_type(const Type *result_type);

} // namespace scopes

#endif // SCOPES_RAISES_HPP
