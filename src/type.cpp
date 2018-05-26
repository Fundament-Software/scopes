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

#include "type.hpp"
#include "typename.hpp"
#include "return.hpp"
#include "integer.hpp"
#include "real.hpp"
#include "pointer.hpp"
#include "array.hpp"
#include "vector.hpp"
#include "tuple.hpp"
#include "union.hpp"
#include "error.hpp"
#include "gc.hpp"

#include "llvm/Support/Casting.h"

#include <memory.h>

namespace scopes {

using llvm::isa;
using llvm::cast;
using llvm::dyn_cast;

//------------------------------------------------------------------------------

TypeKind Type::kind() const { return _kind; } // for this codebase

Type::Type(TypeKind kind) : _kind(kind), _name(Symbol(SYM_Unnamed).name()) {}

const String *Type::name() const {
    return _name;
}

StyledStream& Type::stream(StyledStream& ost) const {
    ost << Style_Type;
    ost << name()->data;
    ost << Style_None;
    return ost;
}

void Type::bind(Symbol name, const Any &value) {
    auto ret = symbols.insert({ name, value });
    if (!ret.second) {
        ret.first->second = value;
    }
}

void Type::del(Symbol name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        symbols.erase(it);
    }
}

bool Type::lookup(Symbol name, Any &dest) const {
    const Type *self = this;
    do {
        auto it = self->symbols.find(name);
        if (it != self->symbols.end()) {
            dest = it->second;
            return true;
        }
        if (self == TYPE_Typename)
            break;
        self = superof(self);
    } while (self);
    return false;
}

bool Type::lookup_local(Symbol name, Any &dest) const {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        dest = it->second;
        return true;
    }
    return false;
}

bool Type::lookup_call_handler(Any &dest) const {
    return lookup(SYM_CallHandler, dest);
}

const Type::Map &Type::get_symbols() const {
    return symbols;
}

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ost, const Type *type) {
    if (!type) {
        ost << Style_Error;
        ost << "<null type>";
        ost << Style_None;
        return ost;
    } else {
        return type->stream(ost);
    }
}

//------------------------------------------------------------------------------

#define T(TYPE, TYPENAME) \
    const Type *TYPE = nullptr;
B_TYPES()
#undef T

//------------------------------------------------------------------------------
// TYPE INQUIRIES
//------------------------------------------------------------------------------

bool is_invalid_argument_type(const Type *T) {
    switch(T->kind()) {
    case TK_Typename: {
        const TypenameType *tt = cast<TypenameType>(T);
        if (!tt->finalized()) {
            return true;
        } else {
            return is_invalid_argument_type(tt->storage_type);
        }
    } break;
    case TK_ReturnLabel: {
        const ReturnLabelType *rlt = cast<ReturnLabelType>(T);
        return is_invalid_argument_type(rlt->return_type);
    } break;
    case TK_Function: return true;
    default: break;
    }
    return false;
}

bool is_opaque(const Type *T) {
    switch(T->kind()) {
    case TK_Typename: {
        const TypenameType *tt = cast<TypenameType>(T);
        if (!tt->finalized()) {
            return true;
        } else {
            return is_opaque(tt->storage_type);
        }
    } break;
    case TK_ReturnLabel: {
        const ReturnLabelType *rlt = cast<ReturnLabelType>(T);
        return is_opaque(rlt->return_type);
    } break;
    case TK_Image:
    case TK_SampledImage:
    case TK_Function: return true;
    default: break;
    }
    return false;
}

size_t size_of(const Type *T) {
    switch(T->kind()) {
    case TK_Integer: {
        const IntegerType *it = cast<IntegerType>(T);
        return (it->width + 7) / 8;
    }
    case TK_Real: {
        const RealType *rt = cast<RealType>(T);
        return (rt->width + 7) / 8;
    }
    case TK_Extern:
    case TK_Pointer: return PointerType::size();
    case TK_Array: return cast<ArrayType>(T)->size;
    case TK_Vector: return cast<VectorType>(T)->size;
    case TK_Tuple: return cast<TupleType>(T)->size;
    case TK_Union: return cast<UnionType>(T)->size;
    case TK_ReturnLabel: {
        return size_of(cast<ReturnLabelType>(T)->return_type);
    } break;
    case TK_Typename: return size_of(storage_type(cast<TypenameType>(T)));
    default: break;
    }

    StyledString ss;
    ss.out << "opaque type " << T << " has no size";
    location_error(ss.str());
    return -1;
}

size_t align_of(const Type *T) {
    switch(T->kind()) {
    case TK_Integer: {
        const IntegerType *it = cast<IntegerType>(T);
        return (it->width + 7) / 8;
    }
    case TK_Real: {
        const RealType *rt = cast<RealType>(T);
        switch(rt->width) {
        case 16: return 2;
        case 32: return 4;
        case 64: return 8;
        case 80: return 16;
        default: break;
        }
    }
    case TK_Extern:
    case TK_Pointer: return PointerType::size();
    case TK_Array: return cast<ArrayType>(T)->align;
    case TK_Vector: return cast<VectorType>(T)->align;
    case TK_Tuple: return cast<TupleType>(T)->align;
    case TK_Union: return cast<UnionType>(T)->align;
    case TK_ReturnLabel: {
        return size_of(cast<ReturnLabelType>(T)->return_type);
    } break;
    case TK_Typename: return align_of(storage_type(cast<TypenameType>(T)));
    default: break;
    }

    StyledString ss;
    ss.out << "opaque type " << T << " has no alignment";
    location_error(ss.str());
    return 1;
}

const Type *superof(const Type *T) {
    switch(T->kind()) {
    case TK_Integer: return TYPE_Integer;
    case TK_Real: return TYPE_Real;
    case TK_Pointer: return TYPE_Pointer;
    case TK_Array: return TYPE_Array;
    case TK_Vector: return TYPE_Vector;
    case TK_Tuple: return TYPE_Tuple;
    case TK_Union: return TYPE_Union;
    case TK_Typename: return cast<TypenameType>(T)->super();
    case TK_ReturnLabel: return TYPE_ReturnLabel;
    case TK_Function: return TYPE_Function;
    case TK_Extern: return TYPE_Extern;
    case TK_Image: return TYPE_Image;
    case TK_SampledImage: return TYPE_SampledImage;
    }
    assert(false && "unhandled type kind; corrupt pointer?");
    return nullptr;
}

//------------------------------------------------------------------------------

Any wrap_pointer(const Type *type, void *ptr) {
    Any result = none;
    result.type = type;

    type = storage_type(type);
    switch(type->kind()) {
    case TK_Integer:
    case TK_Real:
    case TK_Pointer:
        memcpy(result.content, ptr, size_of(type));
        return result;
    case TK_Array:
    case TK_Vector:
    case TK_Tuple:
    case TK_Union:
        result.pointer = ptr;
        return result;
    default: break;
    }

    StyledString ss;
    ss.out << "cannot wrap data of type " << type;
    location_error(ss.str());
    return none;
}

void *get_pointer(const Type *type, Any &value, bool create) {
    if (type == TYPE_Void) {
        return value.content;
    }
    switch(type->kind()) {
    case TK_Integer: {
        auto it = cast<IntegerType>(type);
        switch(it->width) {
        case 1: return (void *)&value.i1;
        case 8: return (void *)&value.u8;
        case 16: return (void *)&value.u16;
        case 32: return (void *)&value.u32;
        case 64: return (void *)&value.u64;
        default: break;
        }
    } break;
    case TK_Real: {
        auto rt = cast<RealType>(type);
        switch(rt->width) {
        case 32: return (void *)&value.f32;
        case 64: return (void *)&value.f64;
        default: break;
        }
    } break;
    case TK_Pointer: return (void *)&value.pointer;
    case TK_Typename: {
        return get_pointer(storage_type(type), value, create);
    } break;
    case TK_Array:
    case TK_Vector:
    case TK_Tuple:
    case TK_Union:
        if (create) {
            value.pointer = tracked_malloc(size_of(type));
        }
        return value.pointer;
    default: break;
    };

    StyledString ss;
    ss.out << "cannot extract pointer from type " << type;
    location_error(ss.str());
    return nullptr;
}

//------------------------------------------------------------------------------
// TYPE CHECK PREDICATES
//------------------------------------------------------------------------------

void verify(const Type *typea, const Type *typeb) {
    if (typea != typeb) {
        StyledString ss;
        ss.out << "type " << typea << " expected, got " << typeb;
        location_error(ss.str());
    }
}

void verify_integer(const Type *type) {
    if (type->kind() != TK_Integer) {
        StyledString ss;
        ss.out << "integer type expected, got " << type;
        location_error(ss.str());
    }
}

void verify_real(const Type *type) {
    if (type->kind() != TK_Real) {
        StyledString ss;
        ss.out << "real type expected, got " << type;
        location_error(ss.str());
    }
}

void verify_range(size_t idx, size_t count) {
    if (idx >= count) {
        StyledString ss;
        ss.out << "index out of range (" << idx
            << " >= " << count << ")";
        location_error(ss.str());
    }
}

} // namespace scopes

