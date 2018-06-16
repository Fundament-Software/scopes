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
        case TK_ReturnLabel: ss.out << "return label"; break;
        case TK_Function: ss.out << "function"; break;
        case TK_Extern: ss.out << "extern"; break;
        case TK_Image: ss.out << "image"; break;
        case TK_SampledImage: ss.out << "sampled image"; break;
        }
        ss.out << " kind expected, got " << T;
        SCOPES_LOCATION_ERROR(ss.str());
    }
    return true;
}

template<typename T>
static SCOPES_RESULT(T) cast_number(const Any &value) {
    SCOPES_RESULT_TYPE(T);
    auto ST = SCOPES_GET_RESULT(storage_type(value.type));
    auto it = dyn_cast<IntegerType>(ST);
    if (it) {
        if (it->issigned) {
            switch(it->width) {
            case 8: return (T)value.i8;
            case 16: return (T)value.i16;
            case 32: return (T)value.i32;
            case 64: return (T)value.i64;
            default: break;
            }
        } else {
            switch(it->width) {
            case 1: return (T)value.i1;
            case 8: return (T)value.u8;
            case 16: return (T)value.u16;
            case 32: return (T)value.u32;
            case 64: return (T)value.u64;
            default: break;
            }
        }
    }
    auto ft = dyn_cast<RealType>(ST);
    if (ft) {
        switch(ft->width) {
        case 32: return (T)value.f32;
        case 64: return (T)value.f64;
        default: break;
        }
    }
    StyledString ss;
    ss.out << "can not extract constant from ";
    if (value.is_const()) {
        ss.out << "value of type " << value.type;
    } else {
        ss.out << "variable of type " << value.indirect_type();
    }
    SCOPES_LOCATION_ERROR(ss.str());
}

template<int mincount, int maxcount>
inline SCOPES_RESULT(int) checkargs(size_t argsize, bool allow_overshoot = false) {
    SCOPES_RESULT_TYPE(int);
    int count = (int)argsize - 1;
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

