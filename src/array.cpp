/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "array.hpp"
#include "error.hpp"
#include "typefactory.hpp"

namespace scopes {

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
    static TypeFactory<ArrayType> arrays;
    return arrays.insert(element_type, count);
}

} // namespace scopes
