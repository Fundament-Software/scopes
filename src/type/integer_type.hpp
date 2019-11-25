/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_INTEGER_HPP
#define SCOPES_INTEGER_HPP

#include "../type.hpp"
#include "../utils.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// INTEGER TYPE
//------------------------------------------------------------------------------

struct IntegerType : Type {
    static bool classof(const Type *T);

    void stream_name(StyledStream &ss) const;
    IntegerType(size_t _width, bool _issigned);

    size_t width;
    size_t align;
    size_t size;
    bool issigned;
};

const Type *integer_type(size_t _width, bool _issigned);

int integer_type_bit_size(const Type *T);

} // namespace scopes

#endif // SCOPES_INTEGER_HPP