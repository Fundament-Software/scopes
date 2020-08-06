/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
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
    /* extra types not used by x86-64 PS */ \
    T(INTEGERSI16) \
    T(INTEGERSI8) \
    T(INTEGER128) \
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

ABIClass merge_abi_classes(ABIClass class1, ABIClass class2);
size_t abi_classify(const Type *T, ABIClass *classes);

bool is_memory_class(const Type *T);

} // namespace scopes

#endif // SCOPES_PLATFORM_ABI_HPP