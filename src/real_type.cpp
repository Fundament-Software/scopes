/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "real_type.hpp"
#include "utils.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// REAL TYPE
//------------------------------------------------------------------------------

void RealType::stream_name(StyledStream &ss) const {
    ss << "f" << width;
}

RealType::RealType(size_t _width)
    : Type(TK_Real), width(_width) {
}

static const Type *_Real(size_t _width) {
    return new RealType(_width);
}
static auto m_Real = memoize(_Real);

const Type *real_type(size_t _width) {
    return m_Real(_width);
}

} // namespace scopes
