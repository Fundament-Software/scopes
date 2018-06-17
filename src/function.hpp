/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_FUNCTION_HPP
#define SCOPES_FUNCTION_HPP

#include "type.hpp"
#include "result.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// FUNCTION TYPE
//------------------------------------------------------------------------------

enum {
    // takes variable number of arguments
    FF_Variadic = (1 << 0),
};

struct FunctionType : Type {
    static bool classof(const Type *T);

    FunctionType(
        const Type *_return_type, const ArgTypes &_argument_types, uint32_t _flags);

    void stream_name(StyledStream &ss) const;

    bool vararg() const;

    SCOPES_RESULT(const Type *) type_at_index(size_t i) const;

    const Type *return_type;
    ArgTypes argument_types;
    uint32_t flags;
};

const Type *Function(const Type *return_type,
    const ArgTypes &argument_types, uint32_t flags = 0);

bool is_function_pointer(const Type *type);

const FunctionType *extract_function_type(const Type *T);

SCOPES_RESULT(void) verify_function_pointer(const Type *type);

} // namespace scopes

#endif // SCOPES_FUNCTION_HPP