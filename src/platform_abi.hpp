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

#ifndef SCOPES_PLATFORM_ABI_HPP
#define SCOPES_PLATFORM_ABI_HPP

#include <stddef.h>

namespace scopes {

struct Type;

//------------------------------------------------------------------------------
// PLATFORM ABI
//------------------------------------------------------------------------------

// based on x86-64 PS ABI (SystemV)
#define DEF_ABI_CLASS_NAMES \
    /* This class consists of integral types that fit into one of the general */ \
    /* purpose registers. */ \
    T(INTEGER) \
    T(INTEGERSI) \
    /* special types for windows, not used anywhere else */ \
    T(INTEGERSI16) \
    T(INTEGERSI8) \
    /* The class consists of types that fit into a vector register. */ \
    T(SSE) \
    T(SSESF) \
    T(SSEDF) \
    /* The class consists of types that fit into a vector register and can be */ \
    /* passed and returned in the upper bytes of it. */ \
    T(SSEUP) \
    /* These classes consists of types that will be returned via the x87 FPU */ \
    T(X87) \
    T(X87UP) \
    /* This class consists of types that will be returned via the x87 FPU */ \
    T(COMPLEX_X87) \
    /* This class is used as initializer in the algorithms. It will be used for */ \
    /* padding and empty structures and unions. */ \
    T(NO_CLASS) \
    /* This class consists of types that will be passed and returned in memory */ \
    /* via the stack. */ \
    T(MEMORY)

enum ABIClass {
#define T(X) ABI_CLASS_ ## X,
    DEF_ABI_CLASS_NAMES
#undef T
};

const size_t MAX_ABI_CLASSES = 4;

const char *abi_class_to_string(ABIClass class_);

size_t abi_classify(const Type *T, ABIClass *classes);

bool is_memory_class(const Type *T);

} // namespace scopes

#endif // SCOPES_PLATFORM_ABI_HPP