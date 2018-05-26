/*
Scopes Compiler
Copyright (c) 2016, 2017, 2018 Leonard Ritter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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

} // namespace scopes

#endif // SCOPES_FUNCTION_HPP