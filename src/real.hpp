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

    RealType(size_t _width);

    size_t width;
};

const Type *Real(size_t _width);

} // namespace scopes

#endif // SCOPES_REAL_HPP