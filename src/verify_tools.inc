/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_VERIFY_TOOLS_INC
#define SCOPES_VERIFY_TOOLS_INC

#include "dyn_cast.inc"
#include "error.hpp"

namespace scopes {

template<TypeKind tk>
inline bool is_kind(const Type *T) {
    return (T->kind() == tk);
}

template<TypeKind tk>
inline SCOPES_RESULT(void) verify_kind(const Type *T) {
    SCOPES_RESULT_TYPE(void);
    if (T->kind() != tk) {
        SCOPES_ERROR(TypeKindMismatch, tk, T);
    }
    return {};
}

template<ValueKind tk>
inline SCOPES_RESULT(void) verify_kind(const Value *V) {
    SCOPES_RESULT_TYPE(void);
    if (V->kind() != tk) {
        SCOPES_ERROR(ValueKindMismatch, tk, V->kind());
    }
    return {};
}

template<int mincount, int maxcount>
inline SCOPES_RESULT(int) checkargs(size_t argsize, bool allow_overshoot = false) {
    SCOPES_RESULT_TYPE(int);
    int count = (int)argsize;
    if ((mincount <= 0) && (maxcount == -1)) {
        return count;
    }

    if ((maxcount >= 0) && (count > maxcount)) {
        if (allow_overshoot) {
            count = maxcount;
        } else {
            SCOPES_ERROR(TooManyArguments, maxcount, count);
        }
    }
    if ((mincount >= 0) && (count < mincount)) {
        SCOPES_ERROR(NotEnoughArguments, mincount, count);
    }
    return count;
}

} // namespace scopes

#endif // SCOPES_VERIFY_TOOLS_INC

