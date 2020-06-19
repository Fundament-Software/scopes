/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "array_type.hpp"
#include "../error.hpp"
#include "../hash.hpp"

#include <unordered_set>

namespace scopes {

namespace ArraySet {
struct Hash {
    std::size_t operator()(const ArrayType *s) const {
        return
            hash2(
                std::hash<const Type *>{}(s->element_type),
                std::hash<size_t>{}(s->_count));
    }
};

struct KeyEqual {
    bool operator()( const ArrayType *lhs, const ArrayType *rhs ) const {
        return lhs->element_type == rhs->element_type
            && lhs->_count == rhs->_count;
    }
};
} // namespace ArraySet

static std::unordered_set<const ArrayType *, ArraySet::Hash, ArraySet::KeyEqual> arrays;

//------------------------------------------------------------------------------
// ARRAY TYPE
//------------------------------------------------------------------------------

void ArrayType::stream_name(StyledStream &ss) const {
    ss << "[";
    stream_type_name(ss, element_type);
    ss << " x ";
    if (is_unsized())
        ss << "?";
    else
        ss << _count;
    ss << "]";
}

ArrayType::ArrayType(const Type *_element_type, size_t _count)
    : ArrayLikeType(TK_Array, _element_type, _count) {
    size = stride * count();
    align = qualified_align_of(element_type).assert_ok();
}

//------------------------------------------------------------------------------

SCOPES_RESULT(const Type *) array_type(const Type *element_type, size_t count) {
    SCOPES_RESULT_TYPE(const Type *);
    SCOPES_TYPE_KEY(ArrayType, key);
    key->element_type = element_type;
    key->_count = count;
    auto it = arrays.find(key);
    if (it != arrays.end())
        return *it;
    if (is_opaque(element_type)) {
        SCOPES_ERROR(OpaqueType, element_type);
    }
    const ArrayType *result = new ArrayType(element_type, count);
    arrays.insert(result);
    return result;
}

} // namespace scopes
