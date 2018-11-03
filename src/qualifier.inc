/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_QUALIFIER_INC
#define SCOPES_QUALIFIER_INC

#include "type.hpp"
#include "dyn_cast.inc"

namespace scopes {

template<typename T>
static bool has_qualifier(const Type *type) {
    while (is_qualifier(type)) {
        if (isa<T>(type)) return true;
        type = next_qualifier(type);
    }
    return false;
}

template<typename T>
static const T *try_qualifier(const Type *type) {
    while (is_qualifier(type)) {
        auto val = dyn_cast<T>(type);
        if (val) return val;
        type = next_qualifier(type);
    }
    return nullptr;
}

}

#endif // SCOPES_QUALIFIER_INC