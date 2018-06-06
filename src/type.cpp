/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "types.hpp"
#include "error.hpp"
#include "gc.hpp"
#include "anchor.hpp"
#include "list.hpp"
#include "syntax.hpp"
#include "dyn_cast.inc"

#include <memory.h>

namespace scopes {

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

//------------------------------------------------------------------------------

#define DEFINE_TYPENAME(NAME, T) \
    T = Typename(String::from(NAME));

#define DEFINE_BASIC_TYPE(NAME, CT, T, BODY) { \
        T = Typename(String::from(NAME)); \
        auto tn = cast<TypenameType>(const_cast<Type *>(T)); \
        tn->finalize(BODY); \
        assert(sizeof(CT) == size_of(T)); \
    }

#define DEFINE_STRUCT_TYPE(NAME, CT, T, ...) { \
        T = Typename(String::from(NAME)); \
        auto tn = cast<TypenameType>(const_cast<Type *>(T)); \
        tn->finalize(Tuple({ __VA_ARGS__ })); \
        assert(sizeof(CT) == size_of(T)); \
    }

#define DEFINE_STRUCT_HANDLE_TYPE(NAME, CT, T, ...) { \
        T = Typename(String::from(NAME)); \
        auto tn = cast<TypenameType>(const_cast<Type *>(T)); \
        auto ET = Tuple({ __VA_ARGS__ }); \
        assert(sizeof(CT) == size_of(ET)); \
        tn->finalize(NativeROPointer(ET)); \
    }

#define DEFINE_OPAQUE_HANDLE_TYPE(NAME, CT, T) { \
        T = Typename(String::from(NAME)); \
        auto tn = cast<TypenameType>(const_cast<Type *>(T)); \
        tn->finalize(NativeROPointer(Typename(String::from("_" NAME)))); \
    }

void init_types() {
    DEFINE_TYPENAME("typename", TYPE_Typename);

    DEFINE_TYPENAME("void", TYPE_Void);
    DEFINE_TYPENAME("Nothing", TYPE_Nothing);

    DEFINE_TYPENAME("Sampler", TYPE_Sampler);

    DEFINE_TYPENAME("integer", TYPE_Integer);
    DEFINE_TYPENAME("real", TYPE_Real);
    DEFINE_TYPENAME("pointer", TYPE_Pointer);
    DEFINE_TYPENAME("array", TYPE_Array);
    DEFINE_TYPENAME("vector", TYPE_Vector);
    DEFINE_TYPENAME("tuple", TYPE_Tuple);
    DEFINE_TYPENAME("union", TYPE_Union);
    DEFINE_TYPENAME("ReturnLabel", TYPE_ReturnLabel);
    DEFINE_TYPENAME("constant", TYPE_Constant);
    DEFINE_TYPENAME("function", TYPE_Function);
    DEFINE_TYPENAME("extern", TYPE_Extern);
    DEFINE_TYPENAME("Image", TYPE_Image);
    DEFINE_TYPENAME("SampledImage", TYPE_SampledImage);
    DEFINE_TYPENAME("CStruct", TYPE_CStruct);
    DEFINE_TYPENAME("CUnion", TYPE_CUnion);
    DEFINE_TYPENAME("CEnum", TYPE_CEnum);

    TYPE_Bool = Integer(1, false);

    TYPE_I8 = Integer(8, true);
    TYPE_I16 = Integer(16, true);
    TYPE_I32 = Integer(32, true);
    TYPE_I64 = Integer(64, true);

    TYPE_U8 = Integer(8, false);
    TYPE_U16 = Integer(16, false);
    TYPE_U32 = Integer(32, false);
    TYPE_U64 = Integer(64, false);

    TYPE_F16 = Real(16);
    TYPE_F32 = Real(32);
    TYPE_F64 = Real(64);
    TYPE_F80 = Real(80);

    DEFINE_BASIC_TYPE("usize", size_t, TYPE_USize, TYPE_U64);

    TYPE_Type = Typename(String::from("type"));
    TYPE_Unknown = Typename(String::from("Unknown"));
    const Type *_TypePtr = NativeROPointer(Typename(String::from("_type")));
    cast<TypenameType>(const_cast<Type *>(TYPE_Type))->finalize(_TypePtr);
    cast<TypenameType>(const_cast<Type *>(TYPE_Unknown))->finalize(_TypePtr);

    cast<TypenameType>(const_cast<Type *>(TYPE_Nothing))->finalize(Tuple({}));

    DEFINE_BASIC_TYPE("Symbol", Symbol, TYPE_Symbol, TYPE_U64);
    DEFINE_BASIC_TYPE("Builtin", Builtin, TYPE_Builtin, TYPE_U64);

    DEFINE_STRUCT_TYPE("Any", Any, TYPE_Any,
        TYPE_Type,
        TYPE_U64
    );

    DEFINE_OPAQUE_HANDLE_TYPE("SourceFile", SourceFile, TYPE_SourceFile);
    DEFINE_OPAQUE_HANDLE_TYPE("Label", Label, TYPE_Label);
    DEFINE_OPAQUE_HANDLE_TYPE("Parameter", Parameter, TYPE_Parameter);
    DEFINE_OPAQUE_HANDLE_TYPE("Scope", Scope, TYPE_Scope);
    DEFINE_OPAQUE_HANDLE_TYPE("Frame", Frame, TYPE_Frame);
    DEFINE_OPAQUE_HANDLE_TYPE("Closure", Closure, TYPE_Closure);

    DEFINE_STRUCT_HANDLE_TYPE("Anchor", Anchor, TYPE_Anchor,
        NativeROPointer(TYPE_SourceFile),
        TYPE_I32,
        TYPE_I32,
        TYPE_I32
    );

    {
        TYPE_List = Typename(String::from("list"));

        const Type *cellT = Typename(String::from("_list"));
        auto tn = cast<TypenameType>(const_cast<Type *>(cellT));
        auto ET = Tuple({ TYPE_Any,
            NativeROPointer(cellT), TYPE_USize });
        assert(sizeof(List) == size_of(ET));
        tn->finalize(ET);

        cast<TypenameType>(const_cast<Type *>(TYPE_List))
            ->finalize(NativeROPointer(cellT));
    }

    DEFINE_STRUCT_HANDLE_TYPE("Syntax", Syntax, TYPE_Syntax,
        TYPE_Anchor,
        TYPE_Any,
        TYPE_Bool);

    DEFINE_STRUCT_HANDLE_TYPE("string", String, TYPE_String,
        TYPE_USize,
        Array(TYPE_I8, 1)
    );

    DEFINE_STRUCT_HANDLE_TYPE("Exception", Exception, TYPE_Exception,
        TYPE_Anchor,
        TYPE_String);

    DEFINE_TYPENAME("LabelMacro", TYPE_LabelMacro);
    {
        cast<TypenameType>(const_cast<Type *>(TYPE_LabelMacro))
            ->finalize(
                Pointer(Function(TYPE_Label, { TYPE_Label }),
                    PTF_NonWritable, SYM_Unnamed));
    }

#define T(TYPE, TYPENAME) \
    assert(TYPE);
    B_TYPES()
#undef T
}

#undef DEFINE_TYPENAME
#undef DEFINE_BASIC_TYPE
#undef DEFINE_STRUCT_TYPE
#undef DEFINE_STRUCT_HANDLE_TYPE
#undef DEFINE_OPAQUE_HANDLE_TYPE
#undef DEFINE_STRUCT_TYPE

} // namespace scopes

