/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "vector.hpp"
#include "error.hpp"
#include "typefactory.hpp"

#include "llvm/Support/Casting.h"

namespace scopes {

using llvm::isa;
using llvm::cast;
using llvm::dyn_cast;

//------------------------------------------------------------------------------
// VECTOR TYPE
//------------------------------------------------------------------------------

bool VectorType::classof(const Type *T) {
    return T->kind() == TK_Vector;
}

VectorType::VectorType(const Type *_element_type, size_t _count)
    : SizedStorageType(TK_Vector, _element_type, _count) {
    if (is_opaque(_element_type)) {
        StyledString ss;
        ss.out << "can not construct vector type for values of opaque type "
            << _element_type;
        location_error(ss.str());
    }
    std::stringstream ss;
    ss << "<" << element_type->name()->data << " x " << count << ">";
    _name = String::from_stdstring(ss.str());
}

const Type *Vector(const Type *element_type, size_t count) {
    static TypeFactory<VectorType> vectors;
    return vectors.insert(element_type, count);
}

void verify_integer_vector(const Type *type) {
    if (type->kind() == TK_Vector) {
        type = cast<VectorType>(type)->element_type;
    }
    if (type->kind() != TK_Integer) {
        StyledString ss;
        ss.out << "integer scalar or vector type expected, got " << type;
        location_error(ss.str());
    }
}

void verify_real_vector(const Type *type) {
    if (type->kind() == TK_Vector) {
        type = cast<VectorType>(type)->element_type;
    }
    if (type->kind() != TK_Real) {
        StyledString ss;
        ss.out << "real scalar or vector type expected, got " << type;
        location_error(ss.str());
    }
}

void verify_bool_vector(const Type *type) {
    if (type->kind() == TK_Vector) {
        type = cast<VectorType>(type)->element_type;
    }
    if (type != TYPE_Bool) {
        StyledString ss;
        ss.out << "bool value or vector type expected, got " << type;
        location_error(ss.str());
    }
}

void verify_real_vector(const Type *type, size_t fixedsz) {
    if (type->kind() == TK_Vector) {
        auto T = cast<VectorType>(type);
        if (T->count == fixedsz)
            return;
    }
    StyledString ss;
    ss.out << "vector type of size " << fixedsz << " expected, got " << type;
    location_error(ss.str());
}

void verify_vector_sizes(const Type *type1, const Type *type2) {
    bool type1v = (type1->kind() == TK_Vector);
    bool type2v = (type2->kind() == TK_Vector);
    if (type1v == type2v) {
        if (type1v) {
            if (cast<VectorType>(type1)->count
                    == cast<VectorType>(type2)->count) {
                return;
            }
        } else {
            return;
        }
    }
    StyledString ss;
    ss.out << "operands must be of scalar type or vector type of equal size";
    location_error(ss.str());
}

} // namespace scopes
