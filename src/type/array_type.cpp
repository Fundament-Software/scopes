/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "array_type.hpp"
#include "../error.hpp"
#include "../hash.hpp"

#include "absl/container/flat_hash_set.h"

namespace scopes {

namespace ArraySet {
struct Hash {
    std::size_t operator()(const ArrayType *s) const {
        return
            hash2(
                std::hash<const Type *>{}(s->element_type),
                hash2(std::hash<size_t>{}(s->_count),
                    std::hash<size_t>{}(s->_zterm)));
    }
};

struct KeyEqual {
    bool operator()( const ArrayType *lhs, const ArrayType *rhs ) const {
        return lhs->element_type == rhs->element_type
            && lhs->_count == rhs->_count
            && lhs->_zterm == rhs->_zterm;
    }
};
} // namespace ArraySet

static absl::flat_hash_set<const ArrayType *, ArraySet::Hash, ArraySet::KeyEqual> arrays;

//------------------------------------------------------------------------------
// ARRAY TYPE
//------------------------------------------------------------------------------

void ArrayType::stream_name(StyledStream &ss) const {
    ss << "(";
    if (is_zterm()) {
        ss << "zarray ";
    } else {
        ss << "array ";
    }
    stream_type_name(ss, element_type);
    if (!is_unsized()) {
        ss << " ";
        ss << _count;
    }
    ss << ")";
}

ArrayType::ArrayType(const Type *_element_type, size_t _count, bool zterm)
    : ArrayLikeType(TK_Array, _element_type, _count, zterm) {
    size = stride * full_count();
    align = qualified_align_of(element_type).assert_ok();
}

//------------------------------------------------------------------------------

SCOPES_RESULT(const Type *) array_type(
        const Type *element_type, size_t count, bool zterm) {
    SCOPES_RESULT_TYPE(const Type *);
    SCOPES_TYPE_KEY(ArrayType, key);
    key->element_type = element_type;
    key->_count = count;
    key->_zterm = zterm;
    auto it = arrays.find(key);
    if (it != arrays.end())
        return *it;
    if (is_opaque(element_type)) {
        SCOPES_ERROR(OpaqueType, element_type);
    }
    const ArrayType *result = new ArrayType(element_type, count, zterm);
    arrays.insert(result);
    return result;
}

} // namespace scopes
