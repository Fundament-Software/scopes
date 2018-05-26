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

#include "any.hpp"

#include "type.hpp"

namespace scopes {

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
