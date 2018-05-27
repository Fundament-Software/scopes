/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_FUNCTION_HPP
#define SCOPES_FUNCTION_HPP

#include "type.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// FUNCTION TYPE
//------------------------------------------------------------------------------

enum {
    // takes variable number of arguments
    FF_Variadic = (1 << 0),
    // can be evaluated at compile time
    FF_Pure = (1 << 1),
    // never returns
    FF_Divergent = (1 << 2),
};

struct FunctionType : Type {
    static bool classof(const Type *T);

    FunctionType(
        const Type *_return_type, const ArgTypes &_argument_types, uint32_t _flags);

    bool vararg() const;
    bool pure() const;
    bool divergent() const;

    const Type *type_at_index(size_t i) const;

    const Type *return_type;
    ArgTypes argument_types;
    uint32_t flags;
};

const Type *Function(const Type *return_type,
    const ArgTypes &argument_types, uint32_t flags = 0);

bool is_function_pointer(const Type *type);

bool is_pure_function_pointer(const Type *type);

const FunctionType *extract_function_type(const Type *T);

void verify_function_pointer(const Type *type);

} // namespace scopes

#endif // SCOPES_FUNCTION_HPP