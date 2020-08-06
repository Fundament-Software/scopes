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

#include <assert.h>

#pragma GCC diagnostic ignored "-Wvla-extension"

namespace scopes { namespace abi_windows_x64 {

size_t classify(const Type *T, ABIClass *classes) {
    classes[0] = ABI_CLASS_NO_CLASS;
    if (is_opaque(T))
        return 1;
    T = storage_type(T).assert_ok();
    size_t sz = size_of(T).assert_ok();
    if (sz > 8)
        return 0;
    switch(T->kind()) {
    case TK_Array:
    case TK_Matrix:
    case TK_Tuple:
        if (sz <= 1)
            classes[0] = ABI_CLASS_INTEGERSI8;
        else if (sz <= 2)
            classes[0] = ABI_CLASS_INTEGERSI16;
        else if (sz <= 4)
            classes[0] = ABI_CLASS_INTEGERSI;
        else
            classes[0] = ABI_CLASS_INTEGER;
        return 1;
    case TK_Integer:
    case TK_Pointer:
    case TK_Real:
    case TK_Typename:
    case TK_Vector:
    default:
        return 1;
    }
}

}} // namespace scopes::abi_windows_x64
