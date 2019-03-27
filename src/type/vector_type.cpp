/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "vector_type.hpp"
#include "../error.hpp"
#include "../dyn_cast.inc"
#include "../hash.hpp"

#include <unordered_set>

namespace scopes {

namespace VectorSet {
struct Hash {
    std::size_t operator()(const VectorType *s) const {
        return
            hash2(
                std::hash<const Type *>{}(s->element_type),
                std::hash<size_t>{}(s->count));
    }
};

struct KeyEqual {
    bool operator()( const VectorType *lhs, const VectorType *rhs ) const {
        return lhs->element_type == rhs->element_type
            && lhs->count == rhs->count;
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
    ss << " x " << count << ">";
}

VectorType::VectorType(const Type *_element_type, size_t _count)
    : ArrayLikeType(TK_Vector, _element_type, _count) {
}

SCOPES_RESULT(const Type *) vector_type(const Type *element_type, size_t count) {
    SCOPES_RESULT_TYPE(const Type *);
    SCOPES_TYPE_KEY(VectorType, key);
    key->element_type = element_type;
    key->count = count;
    auto it = vectors.find(key);
    if (it != vectors.end())
        return *it;
    if (is_opaque(element_type)) {
        StyledString ss;
        ss.out << "can not construct vector type for values of opaque type "
            << element_type;
        SCOPES_ERROR(ss.str());
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
        StyledString ss;
        ss.out << "integer scalar or vector type expected, got " << type;
        SCOPES_ERROR(ss.str());
    }
    return {};
}

SCOPES_RESULT(void) verify_real_vector(const Type *type) {
    SCOPES_RESULT_TYPE(void);
    if (type->kind() == TK_Vector) {
        type = cast<VectorType>(type)->element_type;
    }
    if (type->kind() != TK_Real) {
        StyledString ss;
        ss.out << "real scalar or vector type expected, got " << type;
        SCOPES_ERROR(ss.str());
    }
    return {};
}

SCOPES_RESULT(void) verify_bool_vector(const Type *type) {
    SCOPES_RESULT_TYPE(void);
    if (type->kind() == TK_Vector) {
        type = cast<VectorType>(type)->element_type;
    }
    if (type != TYPE_Bool) {
        StyledString ss;
        ss.out << "bool value or vector type expected, got " << type;
        SCOPES_ERROR(ss.str());
    }
    return {};
}

SCOPES_RESULT(void) verify_real_vector(const Type *type, size_t fixedsz) {
    SCOPES_RESULT_TYPE(void);
    if (type->kind() == TK_Vector) {
        auto T = cast<VectorType>(type);
        if (T->count == fixedsz)
            return {};
    }
    StyledString ss;
    ss.out << "vector type of size " << fixedsz << " expected, got " << type;
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) verify_vector_sizes(const Type *type1, const Type *type2) {
    SCOPES_RESULT_TYPE(void);
    bool type1v = (type1->kind() == TK_Vector);
    bool type2v = (type2->kind() == TK_Vector);
    if (type1v == type2v) {
        if (type1v) {
            if (cast<VectorType>(type1)->count
                    == cast<VectorType>(type2)->count) {
                return {};
            }
        } else {
            return {};
        }
    }
    StyledString ss;
    ss.out << "operands must be of scalar type or vector type of equal size";
    SCOPES_ERROR(ss.str());
}

} // namespace scopes
