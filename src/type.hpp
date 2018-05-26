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

#ifndef SCOPES_TYPE_HPP
#define SCOPES_TYPE_HPP

#include <stddef.h>

namespace scopes {

struct StyledStream;

//------------------------------------------------------------------------------
// TYPE
//------------------------------------------------------------------------------

#define B_TYPE_KIND() \
    T(TK_Integer, "type-kind-integer") \
    T(TK_Real, "type-kind-real") \
    T(TK_Pointer, "type-kind-pointer") \
    T(TK_Array, "type-kind-array") \
    T(TK_Vector, "type-kind-vector") \
    T(TK_Tuple, "type-kind-tuple") \
    T(TK_Union, "type-kind-union") \
    T(TK_Typename, "type-kind-typename") \
    T(TK_ReturnLabel, "type-kind-return-label") \
    T(TK_Function, "type-kind-function") \
    T(TK_Extern, "type-kind-extern") \
    T(TK_Image, "type-kind-image") \
    T(TK_SampledImage, "type-kind-sampled-image")

enum TypeKind {
#define T(NAME, BNAME) \
    NAME,
    B_TYPE_KIND()
#undef T
};

//------------------------------------------------------------------------------

struct Type;

//------------------------------------------------------------------------------

#define B_TYPES() \
    /* types */ \
    T(TYPE_Void, "void") \
    T(TYPE_Nothing, "Nothing") \
    T(TYPE_Any, "Any") \
    \
    T(TYPE_Type, "type") \
    T(TYPE_Unknown, "Unknown") \
    T(TYPE_Symbol, "Symbol") \
    T(TYPE_Builtin, "Builtin") \
    \
    T(TYPE_Bool, "bool") \
    \
    T(TYPE_I8, "i8") \
    T(TYPE_I16, "i16") \
    T(TYPE_I32, "i32") \
    T(TYPE_I64, "i64") \
    \
    T(TYPE_U8, "u8") \
    T(TYPE_U16, "u16") \
    T(TYPE_U32, "u32") \
    T(TYPE_U64, "u64") \
    \
    T(TYPE_F16, "f16") \
    T(TYPE_F32, "f32") \
    T(TYPE_F64, "f64") \
    T(TYPE_F80, "f80") \
    \
    T(TYPE_List, "list") \
    T(TYPE_Syntax, "Syntax") \
    T(TYPE_Anchor, "Anchor") \
    T(TYPE_String, "string") \
    \
    T(TYPE_Scope, "Scope") \
    T(TYPE_SourceFile, "SourceFile") \
    T(TYPE_Exception, "Exception") \
    \
    T(TYPE_Parameter, "Parameter") \
    T(TYPE_Label, "Label") \
    T(TYPE_Frame, "Frame") \
    T(TYPE_Closure, "Closure") \
    \
    T(TYPE_USize, "usize") \
    \
    T(TYPE_Sampler, "Sampler") \
    \
    /* supertypes */ \
    T(TYPE_Integer, "integer") \
    T(TYPE_Real, "real") \
    T(TYPE_Pointer, "pointer") \
    T(TYPE_Array, "array") \
    T(TYPE_Vector, "vector") \
    T(TYPE_Tuple, "tuple") \
    T(TYPE_Union, "union") \
    T(TYPE_Typename, "typename") \
    T(TYPE_ReturnLabel, "ReturnLabel") \
    T(TYPE_Function, "function") \
    T(TYPE_Constant, "constant") \
    T(TYPE_Extern, "extern") \
    T(TYPE_Image, "Image") \
    T(TYPE_SampledImage, "SampledImage") \
    T(TYPE_CStruct, "CStruct") \
    T(TYPE_CUnion, "CUnion") \
    T(TYPE_CEnum, "CEnum")

#define T(TYPE, TYPENAME) \
    extern const Type *TYPE;
B_TYPES()
#undef T

//------------------------------------------------------------------------------

bool is_opaque(const Type *T);
size_t size_of(const Type *T);
size_t align_of(const Type *T);
const Type *storage_type(const Type *T);
StyledStream& operator<<(StyledStream& ost, const Type *type);

} // namespace scopes

#endif // SCOPES_TYPE_HPP
