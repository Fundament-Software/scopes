/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "any.hpp"

#include "type.hpp"
#include "error.hpp"
#include "vector.hpp"
#include "integer.hpp"
#include "real.hpp"
#include "array.hpp"
#include "tuple.hpp"
#include "parameter.hpp"

#include <memory.h>

#include "cityhash/city.h"

#include "llvm/Support/Casting.h"

namespace scopes {

using llvm::isa;
using llvm::cast;
using llvm::dyn_cast;

//------------------------------------------------------------------------------
// ANY
//------------------------------------------------------------------------------

struct Syntax;
struct List;
struct Label;
struct Parameter;
struct Scope;
struct Exception;
struct Frame;
struct Closure;

std::size_t Any::Hash::operator()(const Any & s) const {
    return s.hash();
}

Any::Any(Nothing x) : type(TYPE_Nothing), u64(0) {}
Any::Any(const Type *x) : type(TYPE_Type), typeref(x) {}
Any::Any(bool x) : type(TYPE_Bool), u64(0) { i1 = x; }
Any::Any(int8_t x) : type(TYPE_I8), u64(0) { i8 = x; }
Any::Any(int16_t x) : type(TYPE_I16), u64(0) { i16 = x; }
Any::Any(int32_t x) : type(TYPE_I32), u64(0) { i32 = x; }
Any::Any(int64_t x) : type(TYPE_I64), i64(x) {}
Any::Any(uint8_t x) : type(TYPE_U8), u64(0) { u8 = x; }
Any::Any(uint16_t x) : type(TYPE_U16), u64(0) { u16 = x; }
Any::Any(uint32_t x) : type(TYPE_U32), u64(0) { u32 = x; }
Any::Any(uint64_t x) : type(TYPE_U64), u64(x) {}
#ifdef SCOPES_MACOS
Any::Any(unsigned long x) : type(TYPE_U64), u64(x) {}
#endif
Any::Any(float x) : type(TYPE_F32), u64(0) { f32 = x; }
Any::Any(double x) : type(TYPE_F64), f64(x) {}
Any::Any(const String *x) : type(TYPE_String), string(x) {}
Any::Any(Symbol x) : type(TYPE_Symbol), symbol(x) {}
Any::Any(const Syntax *x) : type(TYPE_Syntax), syntax(x) {}
Any::Any(const Anchor *x) : type(TYPE_Anchor), anchor(x) {}
Any::Any(const List *x) : type(TYPE_List), list(x) {}
Any::Any(const Exception *x) : type(TYPE_Exception), exception(x) {}
Any::Any(Label *x) : type(TYPE_Label), label(x) {}
Any::Any(Parameter *x) : type(TYPE_Parameter), parameter(x) {}
Any::Any(Builtin x) : type(TYPE_Builtin), builtin(x) {}
Any::Any(Scope *x) : type(TYPE_Scope), scope(x) {}
Any::Any(Frame *x) : type(TYPE_Frame), frame(x) {}
Any::Any(const Closure *x) : type(TYPE_Closure), closure(x) {}
#if 0
template<unsigned N>
Any(const char (&str)[N]) : type(TYPE_String), string(String::from(str)) {}
#endif

Any Any::toref() {
    return from_pointer(TYPE_Any, new Any(*this));
}

Any Any::from_opaque(const Type *type) {
    Any val = none;
    val.type = type;
    return val;
}

Any Any::from_pointer(const Type *type, void *ptr) {
    Any val = none;
    val.type = type;
    val.pointer = ptr;
    return val;
}

Any::operator const Type *() const { verify(TYPE_Type); return typeref; }
Any::operator const List *() const { verify(TYPE_List); return list; }
Any::operator const Syntax *() const { verify(TYPE_Syntax); return syntax; }
Any::operator const Anchor *() const { verify(TYPE_Anchor); return anchor; }
Any::operator const String *() const { verify(TYPE_String); return string; }
Any::operator const Exception *() const { verify(TYPE_Exception); return exception; }
Any::operator Label *() const { verify(TYPE_Label); return label; }
Any::operator Scope *() const { verify(TYPE_Scope); return scope; }
Any::operator Parameter *() const { verify(TYPE_Parameter); return parameter; }
Any::operator const Closure *() const { verify(TYPE_Closure); return closure; }
Any::operator Frame *() const { verify(TYPE_Frame); return frame; }

bool Any::operator !=(const Any &other) const {
    return !(*this == other);
}

void Any::verify(const Type *T) const {
    scopes::verify(T, type);
}

StyledStream& Any::stream(StyledStream& ost, bool annotate_type) const {
    AnyStreamer as(ost, type, annotate_type);
    if (type == TYPE_Nothing) { as.naked(none); }
    else if (type == TYPE_Type) { as.naked(typeref); }
    else if (type == TYPE_Bool) { as.naked(i1); }
    else if (type == TYPE_I8) { as.typed(i8); }
    else if (type == TYPE_I16) { as.typed(i16); }
    else if (type == TYPE_I32) { as.naked(i32); }
    else if (type == TYPE_I64) { as.typed(i64); }
    else if (type == TYPE_U8) { as.typed(u8); }
    else if (type == TYPE_U16) { as.typed(u16); }
    else if (type == TYPE_U32) { as.typed(u32); }
    else if (type == TYPE_U64) { as.typed(u64); }
    else if (type == TYPE_USize) { as.typed(u64); }
    else if (type == TYPE_F32) { as.naked(f32); }
    else if (type == TYPE_F64) { as.typed(f64); }
    else if (type == TYPE_String) { as.naked(string); }
    else if (type == TYPE_Symbol) { as.naked(symbol); }
    else if (type == TYPE_Syntax) { as.naked(syntax); }
    else if (type == TYPE_Anchor) { as.typed(anchor); }
    else if (type == TYPE_List) { as.naked(list); }
    else if (type == TYPE_Builtin) { as.typed(builtin); }
    else if (type == TYPE_Label) { as.typed(label); }
    else if (type == TYPE_Parameter) { as.typed(parameter); }
    else if (type == TYPE_Scope) { as.typed(scope); }
    else if (type == TYPE_Frame) { as.typed(frame); }
    else if (type == TYPE_Closure) { as.typed(closure); }
    else if (type == TYPE_Any) {
        ost << Style_Operator << "[" << Style_None;
        ((Any *)pointer)->stream(ost);
        ost << Style_Operator << "]" << Style_None;
        as.stream_type_suffix();
    } else if (type->kind() == TK_Extern) {
        ost << symbol;
        as.stream_type_suffix();
    } else if (type->kind() == TK_Vector) {
        auto vt = cast<VectorType>(type);
        ost << Style_Operator << "<" << Style_None;
        for (size_t i = 0; i < vt->count; ++i) {
            if (i != 0) {
                ost << " ";
            }
            vt->unpack(pointer, i).stream(ost, false);
        }
        ost << Style_Operator << ">" << Style_None;
        auto ET = vt->element_type;
        if (!((ET == TYPE_Bool)
            || (ET == TYPE_I32)
            || (ET == TYPE_F32)
            ))
            as.stream_type_suffix();
    } else { as.typed(pointer); }
    return ost;
}

size_t Any::hash() const {
    if (type == TYPE_String) {
        if (!string) return 0; // can happen with nullof
        return CityHash64(string->data, string->count);
    }
    if (is_opaque(type))
        return 0;
    const Type *T = storage_type(type);
    switch(T->kind()) {
    case TK_Integer: {
        switch(cast<IntegerType>(T)->width) {
        case 1: return std::hash<bool>{}(i1);
        case 8: return std::hash<uint8_t>{}(u8);
        case 16: return std::hash<uint16_t>{}(u16);
        case 32: return std::hash<uint32_t>{}(u32);
        case 64: return std::hash<uint64_t>{}(u64);
        default: break;
        }
    } break;
    case TK_Real: {
        switch(cast<RealType>(T)->width) {
        case 32: return std::hash<float>{}(f32);
        case 64: return std::hash<double>{}(f64);
        default: break;
        }
    } break;
    case TK_Extern: {
        return std::hash<uint64_t>{}(u64);
    } break;
    case TK_Pointer: return std::hash<void *>{}(pointer);
    case TK_Array: {
        auto ai = cast<ArrayType>(T);
        size_t h = 0;
        for (size_t i = 0; i < ai->count; ++i) {
            h = HashLen16(h, ai->unpack(pointer, i).hash());
        }
        return h;
    } break;
    case TK_Vector: {
        auto vi = cast<VectorType>(T);
        size_t h = 0;
        for (size_t i = 0; i < vi->count; ++i) {
            h = HashLen16(h, vi->unpack(pointer, i).hash());
        }
        return h;
    } break;
    case TK_Tuple: {
        auto ti = cast<TupleType>(T);
        size_t h = 0;
        for (size_t i = 0; i < ti->types.size(); ++i) {
            h = HashLen16(h, ti->unpack(pointer, i).hash());
        }
        return h;
    } break;
    case TK_Union:
        return CityHash64((const char *)pointer, size_of(T));
    default: break;
    }

    StyledStream ss(std::cout);
    ss << "unhashable value: " << T << std::endl;
    assert(false && "unhashable value");
    return 0;
}

bool Any::operator ==(const Any &other) const {
    if (type != other.type) return false;
    if (type == TYPE_String) {
        if (string == other.string) return true;
        if (!string || !other.string) return false;
        if (string->count != other.string->count)
            return false;
        return !memcmp(string->data, other.string->data, string->count);
    }
    if (is_opaque(type))
        return true;
    const Type *T = storage_type(type);
    switch(T->kind()) {
    case TK_Integer: {
        switch(cast<IntegerType>(T)->width) {
        case 1: return (i1 == other.i1);
        case 8: return (u8 == other.u8);
        case 16: return (u16 == other.u16);
        case 32: return (u32 == other.u32);
        case 64: return (u64 == other.u64);
        default: break;
        }
    } break;
    case TK_Real: {
        switch(cast<RealType>(T)->width) {
        case 32: return (f32 == other.f32);
        case 64: return (f64 == other.f64);
        default: break;
        }
    } break;
    case TK_Extern: return symbol == other.symbol;
    case TK_Pointer: return pointer == other.pointer;
    case TK_Array: {
        auto ai = cast<ArrayType>(T);
        for (size_t i = 0; i < ai->count; ++i) {
            if (ai->unpack(pointer, i) != ai->unpack(other.pointer, i))
                return false;
        }
        return true;
    } break;
    case TK_Vector: {
        auto vi = cast<VectorType>(T);
        for (size_t i = 0; i < vi->count; ++i) {
            if (vi->unpack(pointer, i) != vi->unpack(other.pointer, i))
                return false;
        }
        return true;
    } break;
    case TK_Tuple: {
        auto ti = cast<TupleType>(T);
        for (size_t i = 0; i < ti->types.size(); ++i) {
            if (ti->unpack(pointer, i) != ti->unpack(other.pointer, i))
                return false;
        }
        return true;
    } break;
    case TK_Union:
        return !memcmp(pointer, other.pointer, size_of(T));
    default: break;
    }

    StyledStream ss(std::cout);
    ss << "incomparable value: " << T << std::endl;
    assert(false && "incomparable value");
    return false;
}

void Any::verify_indirect(const Type *T) const {
    scopes::verify(T, indirect_type());
}

bool Any::is_const() const {
    return !((type == TYPE_Parameter) && parameter->label);
}

const Type *Any::indirect_type() const {
    if (!is_const()) {
        return parameter->type;
    } else {
        return type;
    }
}

//------------------------------------------------------------------------------

StyledStream& operator<<(StyledStream& ost, Any value) {
    return value.stream(ost);
}

//------------------------------------------------------------------------------

bool is_unknown(const Any &value) {
    return value.type == TYPE_Unknown;
}

bool is_typed(const Any &value) {
    return (value.type != TYPE_Unknown) || (value.typeref != TYPE_Unknown);
}

Any unknown_of(const Type *T) {
    Any result(T);
    result.type = TYPE_Unknown;
    return result;
}

Any untyped() {
    return unknown_of(TYPE_Unknown);
}

//------------------------------------------------------------------------------

Any::AnyStreamer::AnyStreamer(StyledStream& _ost, const Type *_type, bool _annotate_type) :
    ost(_ost), type(_type), annotate_type(_annotate_type) {}

void Any::AnyStreamer::stream_type_suffix() const {
    if (annotate_type) {
        ost << Style_Operator << ":" << Style_None;
        ost << type;
    }
}

} // namespace scopes
