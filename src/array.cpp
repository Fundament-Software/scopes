/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "array.hpp"
#include "error.hpp"
#include "hash.hpp"

#include <unordered_set>

namespace scopes {

namespace ArraySet {
struct Hash {
    std::size_t operator()(const ArrayType *s) const {
        return
            hash2(
                std::hash<const Type *>{}(s->element_type),
                std::hash<size_t>{}(s->count));
    }
};

struct KeyEqual {
    bool operator()( const ArrayType *lhs, const ArrayType *rhs ) const {
        return lhs->element_type == rhs->element_type
            && lhs->count == rhs->count;
    }
};
} // namespace ArraySet

static std::unordered_set<const ArrayType *, ArraySet::Hash, ArraySet::KeyEqual> arrays;

//------------------------------------------------------------------------------
// ARRAY TYPE
//------------------------------------------------------------------------------

bool ArrayType::classof(const Type *T) {
    return T->kind() == TK_Array;
}

ArrayType::ArrayType(const Type *_element_type, size_t _count)
    : SizedStorageType(TK_Array, _element_type, _count) {
    if (is_opaque(_element_type)) {
        StyledString ss;
        ss.out << "can not construct array type for values of opaque type "
            << _element_type;
        location_error(ss.str());
    }
    std::stringstream ss;
    ss << "[" << element_type->name()->data << " x " << count << "]";
    _name = String::from_stdstring(ss.str());
}

//------------------------------------------------------------------------------

const Type *Array(const Type *element_type, size_t count) {
    SCOPES_TYPE_KEY(ArrayType, key);
    key->element_type = element_type;
    key->count = count;
    auto it = arrays.find(key);
    if (it != arrays.end())
        return *it;
    const ArrayType *result = new ArrayType(element_type, count);
    arrays.insert(result);
    return result;
}

} // namespace scopes
