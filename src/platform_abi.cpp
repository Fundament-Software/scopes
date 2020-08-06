/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "platform_abi.hpp"
#include "types.hpp"
#include "type.hpp"
#include "utils.hpp"
#include "dyn_cast.inc"

#include "abi_x86_64.cpp"
#include "abi_windows_x64.cpp"
#include "abi_aarch64.cpp"

#include <assert.h>

#pragma GCC diagnostic ignored "-Wvla-extension"

namespace scopes {

//------------------------------------------------------------------------------
// PLATFORM ABI
//------------------------------------------------------------------------------

// life is unfair, which is why we need to implement the remaining platform ABI
// support in the front-end, particularly whether an argument is passed by
// value or not.

const char *abi_class_to_string(ABIClass class_) {
    switch(class_) {
    #define T(X) case ABI_CLASS_ ## X: return #X;
    DEF_ABI_CLASS_NAMES
    #undef T
    default: return "?";
    }
}

#undef DEF_ABI_CLASS_NAMES

ABIClass merge_abi_classes(ABIClass class1, ABIClass class2) {
    if (class1 == class2)
        return class1;

    if (class1 == ABI_CLASS_NO_CLASS)
        return class2;
    if (class2 == ABI_CLASS_NO_CLASS)
        return class1;

    if (class1 == ABI_CLASS_MEMORY || class2 == ABI_CLASS_MEMORY)
        return ABI_CLASS_MEMORY;

    if ((class1 == ABI_CLASS_INTEGERSI && class2 == ABI_CLASS_SSESF)
        || (class2 == ABI_CLASS_INTEGERSI && class1 == ABI_CLASS_SSESF))
        return ABI_CLASS_INTEGERSI;
    if (class1 == ABI_CLASS_INTEGER || class1 == ABI_CLASS_INTEGERSI
        || class2 == ABI_CLASS_INTEGER || class2 == ABI_CLASS_INTEGERSI)
        return ABI_CLASS_INTEGER;

    if (class1 == ABI_CLASS_X87
        || class1 == ABI_CLASS_X87UP
        || class1 == ABI_CLASS_COMPLEX_X87
        || class2 == ABI_CLASS_X87
        || class2 == ABI_CLASS_X87UP
        || class2 == ABI_CLASS_COMPLEX_X87)
        return ABI_CLASS_MEMORY;

    return ABI_CLASS_SSE;
}

size_t abi_classify(const Type *T, ABIClass *classes) {
    //const Type *ST = strip_qualifiers(T);
    if (T->kind() == TK_Arguments) {
        if (T == empty_arguments_type()) {
            classes[0] = ABI_CLASS_NO_CLASS;
            return 1;
        }
        T = cast<ArgumentsType>(T)->to_tuple_type();
    }
    T = qualified_storage_type(T).assert_ok();
    size_t sz;
#if defined(SCOPES_WIN32)
    sz = abi_windows_x64::classify(T, classes);
#elif defined(__amd64__)
    sz = abi_x86_64::classify(T, classes);
#elif defined(__aarch64__)
    sz = abi_aarch64::classify(T, classes);
#else
#error unsupported platform ABI
#endif

#if 0
    if (sz) {
        StyledStream ss(std::cout);
        ss << T << " -> " << sz;
        for (int i = 0; i < sz; ++i) {
            ss << " " << abi_class_to_string(classes[i]);
        }
        ss << std::endl;
    }
#endif
    return sz;
}

bool is_memory_class(const Type *T) {
    ABIClass classes[MAX_ABI_CLASSES];
    return !abi_classify(T, classes);
}


} // namespace scopes
