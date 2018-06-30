/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_TYPE_HPP
#define SCOPES_TYPE_HPP

#include "symbol.hpp"
#include "result.hpp"

#include <stddef.h>

#include <unordered_map>
#include <vector>

namespace scopes {

struct StyledStream;
struct Const;

#define SCOPES_TYPE_KEY(T, NAME) \
    char NAME ## _buf[sizeof(T)]; \
    T *NAME = reinterpret_cast<T *>(NAME ## _buf);

//------------------------------------------------------------------------------
// TYPE
//------------------------------------------------------------------------------

#define B_TYPE_KIND() \
    T(TK_Integer, "type-kind-integer", IntegerType) \
    T(TK_Real, "type-kind-real", RealType) \
    T(TK_Pointer, "type-kind-pointer", PointerType) \
    T(TK_Array, "type-kind-array", ArrayType) \
    T(TK_Vector, "type-kind-vector", VectorType) \
    T(TK_Tuple, "type-kind-tuple", TupleType) \
    T(TK_Union, "type-kind-union", UnionType) \
    T(TK_Typename, "type-kind-typename", TypenameType) \
    T(TK_Return, "type-kind-return", ReturnType) \
    T(TK_Function, "type-kind-function", FunctionType) \
    T(TK_Image, "type-kind-image", ImageType) \
    T(TK_SampledImage, "type-kind-sampled-image", SampledImageType)

enum TypeKind {
#define T(NAME, BNAME, CLASS) \
    NAME,
    B_TYPE_KIND()
#undef T
};

//------------------------------------------------------------------------------

struct KeyedType {
    Symbol key;
    const Type *type;

    KeyedType();
    KeyedType(const Type *type);
    KeyedType(Symbol key, const Type *type);
    size_t hash() const;
    bool operator ==(const KeyedType &other) const;
    bool operator !=(const KeyedType &other) const;
};

typedef std::vector<KeyedType> KeyedTypes;

//------------------------------------------------------------------------------

struct Type {
    typedef std::unordered_map<Symbol, Const *, Symbol::Hash> Map;

    TypeKind kind() const;

    Type(TypeKind kind);
    Type(const Type &other) = delete;

    StyledStream& stream(StyledStream& ost) const;

    void bind(Symbol name, Const *value);

    void del(Symbol name);

    bool lookup(Symbol name, Const *&dest) const;

    bool lookup_local(Symbol name, Const *&dest) const;

    bool lookup_call_handler(Const *&dest) const;

    bool lookup_return_handler(Const *&dest) const;

    const Map &get_symbols() const;

private:
    const TypeKind _kind;

protected:
    Map symbols;
};

typedef std::vector<const Type *> ArgTypes;

//------------------------------------------------------------------------------

#define B_TYPES() \
    /* types */ \
    T(TYPE_Void, "void") \
    T(TYPE_Nothing, "Nothing") \
    \
    T(TYPE_Type, "type") \
    T(TYPE_Unknown, "Unknown") \
    T(TYPE_Symbol, "Symbol") \
    T(TYPE_Builtin, "Builtin") \
    T(TYPE_ASTNode, "ASTNode") \
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
    T(TYPE_Anchor, "Anchor") \
    T(TYPE_String, "string") \
    \
    T(TYPE_Scope, "Scope") \
    T(TYPE_SourceFile, "SourceFile") \
    T(TYPE_Error, "Error") \
    \
    T(TYPE_Closure, "Closure") \
    T(TYPE_ASTMacro, "ASTMacro") \
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
    T(TYPE_Return, "Return") \
    T(TYPE_Function, "function") \
    T(TYPE_Constant, "constant") \
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
SCOPES_RESULT(size_t) size_of(const Type *T);
SCOPES_RESULT(size_t) align_of(const Type *T);
const Type *superof(const Type *T);
bool is_invalid_argument_type(const Type *T);
void stream_type_name(StyledStream &ss, const Type *T);

//------------------------------------------------------------------------------
// TYPE CHECK PREDICATES
//------------------------------------------------------------------------------

SCOPES_RESULT(void) verify(const Type *typea, const Type *typeb);
SCOPES_RESULT(void) verify_integer(const Type *type);
SCOPES_RESULT(void) verify_real(const Type *type);
SCOPES_RESULT(void) verify_range(size_t idx, size_t count);

//------------------------------------------------------------------------------

void init_types();

} // namespace scopes

#endif // SCOPES_TYPE_HPP
