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

namespace scopes { namespace abi_aarch64 {

// A subroutine of is_vfp_type.  Given a structure type, return the type
// of the first non-structure element.  Recurse for structure elements.
// Return NULL if the structure is in fact empty, i.e. no nested elements.
static const Type *is_hfa0 (const Type *ST) {
    switch (ST->kind()) {
    case TK_Vector:
    case TK_Array:
    case TK_Matrix: {
        auto tt = cast<ArrayLikeType>(ST);
        auto T = is_hfa0(qualified_storage_type(tt->element_type).assert_ok());
        if (T) return T;
        return nullptr;
    } break;
    case TK_Tuple: {
        auto tt = cast<TupleType>(ST);
        size_t count = tt->values.size();
        for (size_t i = 0; i < count; ++i) {
            auto T = is_hfa0(qualified_storage_type(tt->values[i]).assert_ok());
            if (T) return T;
        }
        return nullptr;
    } break;
    default: break;
    }
    return ST;
}

// A subroutine of is_vfp_type.  Given a structure type, return true if all
// of the non-structure elements are the same as CANDIDATE.
static bool is_hfa1 (const Type *ST, const Type *candidate) {
    switch (ST->kind()) {
    case TK_Vector:
    case TK_Array:
    case TK_Matrix: {
        auto tt = cast<ArrayLikeType>(ST);
        return is_hfa1(qualified_storage_type(tt->element_type).assert_ok(), candidate);
    } break;
    case TK_Tuple: {
        auto tt = cast<TupleType>(ST);
        size_t count = tt->values.size();
        for (size_t i = 0; i < count; ++i) {
            if (!is_hfa1(qualified_storage_type(tt->values[i]).assert_ok(), candidate))
                return false;
        }
        return true;
    } break;
    default: break;
    }
    return (ST == candidate);
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.
   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */
static int is_vfp_type (const Type *ty, const Type *&candidate) {
    size_t size, ele_count;

    candidate = nullptr;
    switch (ty->kind()) {
    case TK_Real: {
        candidate = ty;
        return 1;
    } break;
    case TK_Vector:
    case TK_Array:
    case TK_Matrix:
    case TK_Tuple: {
    } break;
    default: return 0;
    }

    // No HFA types are smaller than 4 bytes, or larger than 64 bytes.
    size = size_of(ty).assert_ok();
    if (size < 4 || size > 64)
        return 0;

    // Find the type of the first non-structure member.
    candidate = is_hfa0(ty);

    if (!candidate)
        return 0;

    // If the first member is not a floating point type, it's not an HFA.
    // Also quickly re-check the size of the structure.
    switch (candidate->kind()) {
    case TK_Real: {
        size_t c_size = size_of(candidate).assert_ok();
        ele_count = size / c_size;
        if (size != ele_count * c_size)
            return 0;
    } break;
    default: return 0;
    }
    if (ele_count > 4)
        return 0;

    if (!is_hfa1(ty, candidate))
        return 0;

    return ele_count;
}

size_t classify(const Type *T, ABIClass *classes) {
    classes[0] = ABI_CLASS_NO_CLASS;
    if (is_opaque(T))
        return 1;
    size_t sz = size_of(T).assert_ok();
    if (sz > 16)
        return 0;
    switch(T->kind()) {
    case TK_Integer:
    case TK_Pointer:
    case TK_Real:
    case TK_Typename:
    case TK_Vector:
        return 1;
    case TK_Array:
    case TK_Matrix:
    case TK_Tuple: {
        const Type *ET = nullptr;
        int count = is_vfp_type(T, ET);
        if (count) {
            assert(ET->kind() == TK_Real);
            assert((count >= 1) && (count <= 4));
            if (ET == TYPE_F32) {
                switch (count) {
                case 1: classes[0] = ABI_CLASS_SSESF; return 1;
                case 2: classes[0] = ABI_CLASS_FLOATx2; return 1;
                case 3: classes[0] = ABI_CLASS_FLOATx3; return 1;
                case 4: classes[0] = ABI_CLASS_FLOATx4; return 1;
                default: break;
                }
            } else if (ET == TYPE_F64) {
                switch (count) {
                case 1: classes[0] = ABI_CLASS_SSEDF; return 1;
                case 2: classes[0] = ABI_CLASS_DOUBLEx2; return 1;
                default: break;
                }
            }
        } else {
            size_t s = size_of(T).assert_ok();
            if (s > 8) {
                classes[0] = ABI_CLASS_INTEGERx2;
                return 1;
            } else {
                classes[0] = ABI_CLASS_INTEGER;
                return 1;
            }
        }
    } break;
    default: {
        StyledStream ss;
        ss << "internal error: type " << T << " unsupported in ABI" << std::endl;
        assert(false && "not supported in ABI");
        return 0;
    } break;
    }
    return 0;
}

}} // namespace scopes::abi_aarch64
