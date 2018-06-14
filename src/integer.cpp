/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "integer.hpp"
#include "utils.hpp"
#include "dyn_cast.inc"

namespace scopes {

//------------------------------------------------------------------------------
// INTEGER TYPE
//------------------------------------------------------------------------------

bool IntegerType::classof(const Type *T) {
    return T->kind() == TK_Integer;
}

void IntegerType::stream_name(StyledStream &ss) const {
    if ((width == 1) && !issigned) {
        ss << "bool";
    } else {
        if (issigned) {
            ss << "i";
        } else {
            ss << "u";
        }
        ss << width;
    }
}

IntegerType::IntegerType(size_t _width, bool _issigned)
    : Type(TK_Integer), width(_width), issigned(_issigned) {
}

static const Type *_Integer(size_t _width, bool _issigned) {
    return new IntegerType(_width, _issigned);
}
static auto m_Integer = memoize(_Integer);

const Type *Integer(size_t _width, bool _issigned) {
    return m_Integer(_width, _issigned);
}

int integer_type_bit_size(const Type *T) {
    return (int)cast<IntegerType>(T)->width;
}

} // namespace scopes

