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
        StyledString ss;
        ss.out << "value of ";
        switch(tk) {
        case TK_Integer: ss.out << "integer"; break;
        case TK_Real: ss.out << "real"; break;
        case TK_Pointer: ss.out << "pointer"; break;
        case TK_Array: ss.out << "array"; break;
        case TK_Vector: ss.out << "vector"; break;
        case TK_Tuple: ss.out << "tuple"; break;
        case TK_Union: ss.out << "union"; break;
        case TK_Typename: ss.out << "typename"; break;
        case TK_Return: ss.out << "return"; break;
        case TK_Function: ss.out << "function"; break;
        case TK_Image: ss.out << "image"; break;
        case TK_SampledImage: ss.out << "sampled image"; break;
        }
        ss.out << " kind expected, got " << T;
        SCOPES_LOCATION_ERROR(ss.str());
    }
    return true;
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
            SCOPES_LOCATION_ERROR(
                format("at most %i argument(s) expected, got %i", maxcount, count));
        }
    }
    if ((mincount >= 0) && (count < mincount)) {
        SCOPES_LOCATION_ERROR(
            format("at least %i argument(s) expected, got %i", mincount, count));
    }
    return count;
}

} // namespace scopes

#endif // SCOPES_VERIFY_TOOLS_INC

