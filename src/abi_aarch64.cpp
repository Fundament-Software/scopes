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

enum {
    AARCH64_RET_VOID	= 0,
    AARCH64_RET_INT64	= 1,
    AARCH64_RET_INT128	= 2,

    AARCH64_RET_UNUSED3	= 3,
    AARCH64_RET_UNUSED4	= 4,
    AARCH64_RET_UNUSED5	= 5,
    AARCH64_RET_UNUSED6	= 6,
    AARCH64_RET_UNUSED7	= 7,

    // Note that FFI_TYPE_FLOAT == 2, _DOUBLE == 3, _LONGDOUBLE == 4,
    // so _S4 through _Q1 are layed out as (TYPE * 4) + (4 - COUNT).
    AARCH64_RET_S4		= 8,
    AARCH64_RET_S3		= 9,
    AARCH64_RET_S2		= 10,
    AARCH64_RET_S1		= 11,

    AARCH64_RET_D4		= 12,
    AARCH64_RET_D3		= 13,
    AARCH64_RET_D2		= 14,
    AARCH64_RET_D1		= 15,

    AARCH64_RET_Q4		= 16,
    AARCH64_RET_Q3		= 17,
    AARCH64_RET_Q2		= 18,
    AARCH64_RET_Q1		= 19,

    // Note that each of the sub-64-bit integers gets two entries.
    AARCH64_RET_UINT8	= 20,
    AARCH64_RET_UINT16	= 22,
    AARCH64_RET_UINT32	= 24,

    AARCH64_RET_SINT8	= 26,
    AARCH64_RET_SINT16	= 28,
    AARCH64_RET_SINT32	= 30,

    AARCH64_RET_MASK	= 31,

    AARCH64_RET_IN_MEM	= (1 << 5),
    AARCH64_RET_NEED_COPY = (1 << 6),

    AARCH64_FLAG_ARG_V_BIT	= 7,
    AARCH64_FLAG_ARG_V	= (1 << AARCH64_FLAG_ARG_V_BIT),

    N_X_ARG_REG		= 8,
    N_V_ARG_REG		= 8,
    CALL_CONTEXT_SIZE	= (N_V_ARG_REG * 16 + N_X_ARG_REG * 8),
};

#if 0
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
    } break;
    case TK_Tuple: {
        auto tt = cast<TupleType>(ST);
        size_t count = tt->values.size();
        for (size_t i = 0; i < count; ++i) {
            auto T = is_hfa0(qualified_storage_type(tt->values[i]).assert_ok());
            if (T) return T;
        }
    } break;
    default: break;
    }
    return nullptr;
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
#endif

size_t classify(const Type *T, ABIClass *classes) {
    switch(T->kind()) {
    case TK_Integer:
    case TK_Pointer: {
        size_t size = size_of(T).assert_ok();
        if (size <= 1) {
            classes[0] = ABI_CLASS_INTEGERSI8;
            return 1;
        } else if (size <= 2) {
            classes[0] = ABI_CLASS_INTEGERSI16;
            return 1;
        } else if (size <= 4) {
            classes[0] = ABI_CLASS_INTEGERSI;
            return 1;
        } else if (size <= 8) {
            classes[0] = ABI_CLASS_INTEGER;
            return 1;
        } else if (size <= 16) {
            classes[0] = ABI_CLASS_INTEGER128;
            return 1;
        } else {
            return 0;
        }
    } break;
    case TK_Typename: {
        if (is_opaque(T)) {
            classes[0] = ABI_CLASS_NO_CLASS;
            return 1;
        } else {
            return classify(storage_type(T).assert_ok(), classes);
        }
    } break;
    case TK_Real:
    case TK_Vector:
    case TK_Array:
    case TK_Matrix:
    case TK_Tuple: {
        //const Type *ET = nullptr;
        //int count = is_vfp_type(T, ET);
        //if (!count) {
            size_t s = size_of(T).assert_ok();
            if (s > 16) {
                classes[0] = ABI_CLASS_NO_CLASS;
                return 1;
            } else if (s == 16) {
                classes[0] = ABI_CLASS_INTEGER128;
                return 1;
            } else if (s == 8) {
                classes[0] = ABI_CLASS_INTEGER;
                return 1;
            } else {
                classes[0] = ABI_CLASS_INTEGER128;
                return 1;
            }
        //}
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
