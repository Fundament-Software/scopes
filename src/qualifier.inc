/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_QUALIFIER_INC
#define SCOPES_QUALIFIER_INC

#include "qualify_type.hpp"
#include "dyn_cast.inc"

namespace scopes {

template<typename T>
static bool has_qualifier(const Type *type) {
    return find_qualifier(type, (QualifierKind)T::Kind) != nullptr;
}

template<typename T>
static const T *try_qualifier(const Type *type) {
    auto q = find_qualifier(type, (QualifierKind)T::Kind);
    if (!q) return nullptr;
    return cast<T>(q);
}

template<typename T>
static const Type *strip_qualifier(const Type *type) {
    return strip_qualifier(type, (QualifierKind)T::Kind);
}

}

#endif // SCOPES_QUALIFIER_INC