/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_REAL_HPP
#define SCOPES_REAL_HPP

#include "type.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// REAL TYPE
//------------------------------------------------------------------------------

struct RealType : Type {
    static bool classof(const Type *T);

    void stream_name(StyledStream &ss) const;
    RealType(size_t _width);

    size_t width;
};

const Type *real_type(size_t _width);

} // namespace scopes

#endif // SCOPES_REAL_HPP