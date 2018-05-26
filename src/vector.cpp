/*
Scopes Compiler
Copyright (c) 2016, 2017, 2018 Leonard Ritter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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
