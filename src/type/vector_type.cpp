/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "vector_type.hpp"
#include "../error.hpp"
#include "../dyn_cast.inc"
#include "../hash.hpp"
#include "../utils.hpp"

#include <unordered_set>

namespace scopes {

namespace VectorSet {
struct Hash {
    std::size_t operator()(const VectorType *s) const {
        return
            hash2(
                std::hash<const Type *>{}(s->element_type),
                std::hash<size_t>{}(s->_count));
    }
};

struct KeyEqual {
    bool operator()( const VectorType *lhs, const VectorType *rhs ) const {
        return lhs->element_type == rhs->element_type
            && lhs->_count == rhs->_count;
    }
};
} // namespace VectorSet

static std::unordered_set<const VectorType *, VectorSet::Hash, VectorSet::KeyEqual> vectors;

//------------------------------------------------------------------------------
// VECTOR TYPE
//------------------------------------------------------------------------------

void VectorType::stream_name(StyledStream &ss) const {
    ss << "<";
    stream_type_name(ss, element_type);
    ss << " x ";
    if (is_unsized())
        ss << "?";
    else
        ss << _count;
    ss << ">";
}

VectorType::VectorType(const Type *_element_type, size_t _count)
    : ArrayLikeType(TK_Vector, _element_type, _count) {
    if (is_unsized()) {
        size = 0;
        align = align_of(element_type).assert_ok();
    } else {
        size = ceilpow2(stride * count());
        align = size;
    }
}

SCOPES_RESULT(const Type *) vector_type(const Type *element_type, size_t count) {
    SCOPES_RESULT_TYPE(const Type *);
    SCOPES_TYPE_KEY(VectorType, key);
    key->element_type = element_type;
    key->_count = count;
    auto it = vectors.find(key);
    if (it != vectors.end())
        return *it;
    if (is_opaque(element_type)) {
        SCOPES_ERROR(OpaqueType, element_type);
    }
    auto result = new VectorType(element_type, count);
    vectors.insert(result);
    return result;
}

SCOPES_RESULT(void) verify_integer_vector(const Type *type) {
    SCOPES_RESULT_TYPE(void);
    if (type->kind() == TK_Vector) {
        type = cast<VectorType>(type)->element_type;
    }
    if (type->kind() != TK_Integer) {
        SCOPES_ERROR(ScalarOrVectorExpected, TYPE_Integer, type);
    }
    return {};
}

SCOPES_RESULT(void) verify_real_vector(const Type *type) {
    SCOPES_RESULT_TYPE(void);
    if (type->kind() == TK_Vector) {
        type = cast<VectorType>(type)->element_type;
    }
    if (type->kind() != TK_Real) {
        SCOPES_ERROR(ScalarOrVectorExpected, TYPE_Real, type);
    }
    return {};
}

SCOPES_RESULT(void) verify_bool_vector(const Type *type) {
    SCOPES_RESULT_TYPE(void);
    if (type->kind() == TK_Vector) {
        type = cast<VectorType>(type)->element_type;
    }
    if (type != TYPE_Bool) {
        SCOPES_ERROR(ScalarOrVectorExpected, TYPE_Bool, type);
    }
    return {};
}

SCOPES_RESULT(void) verify_real_vector(const Type *type, size_t fixedsz) {
    SCOPES_RESULT_TYPE(void);
    if (type->kind() == TK_Vector) {
        auto T = cast<VectorType>(type);
        if (T->_count == fixedsz)
            return {};
    }
    SCOPES_ERROR(FixedVectorSizeMismatch, fixedsz, type);
}

SCOPES_RESULT(void) verify_vector_sizes(const Type *type1, const Type *type2) {
    SCOPES_RESULT_TYPE(void);
    bool type1v = (type1->kind() == TK_Vector);
    bool type2v = (type2->kind() == TK_Vector);
    if (type1v == type2v) {
        if (type1v) {
            if (cast<VectorType>(type1)->_count
                    == cast<VectorType>(type2)->_count) {
                return {};
            }
        } else {
            return {};
        }
    }
    SCOPES_ERROR(VectorSizeMismatch, type1, type2);
}

} // namespace scopes
