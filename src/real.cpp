/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "real.hpp"
#include "utils.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// REAL TYPE
//------------------------------------------------------------------------------

bool RealType::classof(const Type *T) {
    return T->kind() == TK_Real;
}

RealType::RealType(size_t _width)
    : Type(TK_Real), width(_width) {
    std::stringstream ss;
    ss << "f" << width;
    _name = String::from_stdstring(ss.str());
}

static const Type *_Real(size_t _width) {
    return new RealType(_width);
}
static auto m_Real = memoize(_Real);

const Type *Real(size_t _width) {
    return m_Real(_width);
}

} // namespace scopes
