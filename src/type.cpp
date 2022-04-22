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
#include "hash.hpp"
#include "type/qualify_type.hpp"
#include "dyn_cast.inc"
#include "qualifier.inc"
#include "qualifiers.hpp"

#include <memory.h>
#include <algorithm>

namespace scopes {

#define T(NAME, BNAME, CLASS) \
    bool CLASS::classof(const Type *T) { \
        return T->kind() == NAME; \
    }
B_TYPE_KIND()
#undef T

//------------------------------------------------------------------------------

TypeKind Type::kind() const { return _kind; } // for this codebase

Type::Type(TypeKind kind) : _kind(kind) {}

StyledStream& Type::stream(StyledStream& ost) const {
    StyledString ss = StyledString::plain();
    stream_type_name(ss.out, this);
    ost << Style_Type;
    ost << ss.str()->data;
    ost << Style_None;
    return ost;
}

void Type::bind_with_doc(Symbol name, const TypeEntry &entry) const {
    symbols.replace(name, entry);
}

void Type::bind(Symbol name, const ValueRef &value) const {
    TypeEntry entry = { value, nullptr };
    bind_with_doc(name, entry);
}

void Type::unbind(Symbol name) const {
    symbols.discard(name);
}

bool Type::lookup(Symbol name, TypeEntry &dest) const {
    const Type *self = this;
    do {
        auto index = self->symbols.find_index(name);
        if (index >= 0) {
            dest = self->symbols.entries[index].second;
            return true;
        }
        if (self == TYPE_Typename)
            break;
        self = superof(self);
    } while (self);
    return false;
}

bool Type::lookup(Symbol name, ValueRef &dest) const {
    TypeEntry entry;
    if (lookup(name, entry)) {
        dest = entry.expr;
        return true;
    }
    return false;
}

bool Type::lookup_local(Symbol name, TypeEntry &dest) const {
    auto index = symbols.find_index(name);
    if (index >= 0) {
        dest = symbols.entries[index].second;
        return true;
    }
    return false;
}

bool Type::lookup_local(Symbol name, ValueRef &dest) const {
    TypeEntry entry;
    if (lookup_local(name, entry)) {
        dest = entry.expr;
        return true;
    }
    return false;
}

bool Type::lookup_call_handler(ValueRef &dest) const {
    return lookup(SYM_CallHandler, dest);
}

bool Type::lookup_return_handler(ValueRef &dest) const {
    return lookup(SYM_ReturnHandler, dest);
}

bool Type::lookup_quote_handler(ValueRef &dest) const {
    return lookup(SYM_QuoteHandler, dest);
}

const Type::Map &Type::get_symbols() const {
    return symbols;
}

std::vector<Symbol> Type::find_closest_match(Symbol name) const {
    const String *s = name.name();
    std::unordered_set<Symbol, Symbol::Hash> done;
    std::vector<Symbol> best_syms;
    size_t best_dist = (size_t)-1;
    const Type *self = this;
    do {
        auto &&map = self->symbols;
        int count = map.entries.size();
        auto &&keys = map.entries;
        //auto &&values = map.values;
        for (int i = 0; i < count; ++i) {
            Symbol sym = keys[i].first;
            if (done.count(sym))
                continue;
            size_t dist = distance(s, sym.name());
            if (dist == best_dist) {
                best_syms.push_back(sym);
            } else if (dist < best_dist) {
                best_dist = dist;
                best_syms = { sym };
            }
            done.insert(sym);
        }
        if (self == TYPE_Typename)
            break;
        self = superof(self);
    } while (true);
    std::sort(best_syms.begin(), best_syms.end());
    return best_syms;
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

void stream_type_name(StyledStream &ss, const Type *T) {
    switch (T->kind()) {
#define T(TYPE, TYPENAME, CLASS) \
    case TYPE: cast<CLASS>(T)->stream_name(ss); break;
B_TYPE_KIND()
#undef T
        default:
            ss << "???"; break;
    }
}

bool all_plain(const Types &types) {
    for (auto type : types) {
        if (!is_plain(type))
            return false;
    }
    return true;
}

bool is_plain(const Type *T) {
repeat:
    switch(T->kind()) {
    case TK_Qualify:
        T = cast<QualifyType>(T)->type;
        goto repeat;
    case TK_Pointer:
        T = cast<PointerType>(T)->element_type;
        goto repeat;
    case TK_Integer:
    case TK_Real:
    case TK_Image:
    case TK_SampledImage:
    case TK_Sampler:
    case TK_Function:
        return true;
    case TK_Array:
    case TK_Vector:
    case TK_Matrix:
        T = cast<ArrayLikeType>(T)->element_type;
        goto repeat;
    case TK_Tuple:
        return cast<TupleLikeType>(T)->is_plain();
    case TK_Arguments:
        return all_plain(cast<ArgumentsType>(T)->values);
    case TK_Typename:
        return cast<TypenameType>(T)->is_plain();
    }
    return false;
}

TypeKind storage_kind(const Type *T) {
    if (is_opaque(T))
        return T->kind();
    return storage_type(T).assert_ok()->kind();
}

bool is_opaque(const Type *T) {
    switch(T->kind()) {
    case TK_Qualify:
        return is_opaque(cast<QualifyType>(T)->type);
    case TK_Typename: {
        const TypenameType *tt = cast<TypenameType>(T);
        if (tt->is_opaque()) {
            return true;
        } else {
            // does this make sense?
            return is_opaque(tt->storage());
        }
    } break;
    case TK_Arguments:
    //case TK_Image: // can be loaded
    //case TK_SampledImage: // can be loaded
    case TK_Function: return true;
    default: break;
    }
    return false;
}

SCOPES_RESULT(size_t) size_of(const Type *T) {
    SCOPES_RESULT_TYPE(size_t);
    switch(T->kind()) {
    case TK_Qualify:
        return size_of(cast<QualifyType>(T)->type);
    case TK_Integer: {
        const IntegerType *it = cast<IntegerType>(T);
        return it->size;
    }
    case TK_Real: {
        const RealType *rt = cast<RealType>(T);
        return (rt->width + 7) / 8;
    }
    case TK_Pointer: return PointerType::size();
    case TK_Array: return cast<ArrayType>(T)->size;
    case TK_Vector: return cast<VectorType>(T)->size;
    case TK_Matrix: return cast<MatrixType>(T)->size;
    case TK_Tuple: return cast<TupleType>(T)->size;
    case TK_Typename: return size_of(SCOPES_GET_RESULT(storage_type(T)));
    default: break;
    }

    SCOPES_ERROR(OpaqueType, T);
}

SCOPES_RESULT(size_t) bitsize_of(const Type *T) {
    SCOPES_RESULT_TYPE(size_t);
    switch(T->kind()) {
    case TK_Qualify:
        return bitsize_of(cast<QualifyType>(T)->type);
    case TK_Integer: {
        const IntegerType *it = cast<IntegerType>(T);
        return it->width;
    }
    case TK_Real: {
        const RealType *rt = cast<RealType>(T);
        return rt->width;
    }
    case TK_Pointer: return PointerType::size() * 8;
    case TK_Array: return cast<ArrayType>(T)->size * 8;
    case TK_Vector: {
        auto VT = cast<VectorType>(T);
        return VT->count() * SCOPES_GET_RESULT(bitsize_of(VT->element_type));
    }
    case TK_Matrix: return cast<MatrixType>(T)->size * 8;
    case TK_Tuple: return cast<TupleType>(T)->size * 8;
    case TK_Typename: return bitsize_of(SCOPES_GET_RESULT(storage_type(T)));
    default: break;
    }

    SCOPES_ERROR(OpaqueType, T);
}

SCOPES_RESULT(size_t) qualified_size_of(const Type *T) {
    SCOPES_RESULT_TYPE(size_t);
    return size_of(SCOPES_GET_RESULT(qualified_storage_type(T)));
}

SCOPES_RESULT(size_t) align_of(const Type *T) {
    SCOPES_RESULT_TYPE(size_t);
    switch(T->kind()) {
    case TK_Qualify:
        return align_of(cast<QualifyType>(T)->type);
    case TK_Integer: {
        const IntegerType *it = cast<IntegerType>(T);
        return it->align;
    }
    case TK_Real: {
        const RealType *rt = cast<RealType>(T);
        switch(rt->width) {
        case 16: return 2;
        case 32: return 4;
        case 64: return 8;
        case 80: return 16;
        case 128: return 16;
        default: break;
        }
    }
    case TK_Pointer: return PointerType::size();
    case TK_Array: return cast<ArrayType>(T)->align;
    case TK_Vector: return cast<VectorType>(T)->align;
    case TK_Matrix: return cast<MatrixType>(T)->align;
    case TK_Tuple: return cast<TupleType>(T)->align;
    case TK_Typename: return align_of(SCOPES_GET_RESULT(storage_type(T)));
    default: break;
    }

    SCOPES_ERROR(OpaqueType, T);
}

SCOPES_RESULT(size_t) qualified_align_of(const Type *T) {
    SCOPES_RESULT_TYPE(size_t);
    return align_of(SCOPES_GET_RESULT(qualified_storage_type(T)));
}

const Type *superof(const Type *T) {
    switch(T->kind()) {
    case TK_Qualify: return TYPE_Qualify;
    case TK_Arguments: return TYPE_Arguments;
    case TK_Integer: return TYPE_Integer;
    case TK_Real: return TYPE_Real;
    case TK_Pointer: return TYPE_Pointer;
    case TK_Array:
        return (cast<ArrayType>(T)->is_zterm())?TYPE_ZArray:TYPE_Array;
    case TK_Vector: return TYPE_Vector;
    case TK_Matrix: return TYPE_Matrix;
    case TK_Tuple: return TYPE_Tuple;
    case TK_Typename: return cast<TypenameType>(T)->super();
    case TK_Function: return TYPE_Function;
    case TK_Image: return TYPE_Image;
    case TK_SampledImage: return TYPE_SampledImage;
    case TK_Sampler: return TYPE_Immutable;
    }
    assert(false && "unhandled type kind; corrupt pointer?");
    return nullptr;
}

bool is_returning(const Type *T) {
    return (T != TYPE_NoReturn);
}

bool is_returning_value(const Type *T) {
    return is_returning(T) && (T != empty_arguments_type());
}

bool types_compatible(const Type *paramT, const Type *argT) {
    if (paramT == argT)
        return true;
    if (has_qualifier<ReferQualifier>(paramT) && has_qualifier<ReferQualifier>(argT)) {
        auto pa = get_qualifier<ReferQualifier>(argT);
        auto pb = get_qualifier<ReferQualifier>(paramT);
        if (types_compatible(strip_qualifier<ReferQualifier>(paramT), strip_qualifier<ReferQualifier>(argT))
            && pointer_flags_compatible(pb->flags, pa->flags)
            && pointer_storage_classes_compatible(pb->storage_class, pa->storage_class))
            return true;
    }
    if (!is_opaque(argT)) {
        argT = storage_type(argT).assert_ok();
    }
    if (!is_opaque(paramT)) {
        paramT = storage_type(paramT).assert_ok();
    }
    if (isa<PointerType>(paramT) && isa<PointerType>(argT)) {
        auto pa = cast<PointerType>(argT);
        auto pb = cast<PointerType>(paramT);
        if (types_compatible(pb->element_type, pa->element_type)
            && pointer_flags_compatible(pb->flags, pa->flags)
            && pointer_storage_classes_compatible(pb->storage_class, pa->storage_class))
            return true;
    }
    return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

SCOPES_RESULT(const Type *) storage_type(const Type *T) {
    SCOPES_RESULT_TYPE(const Type *);
    T = strip_qualifiers(T);
    switch(T->kind()) {
    case TK_Typename: {
        const TypenameType *tt = cast<TypenameType>(T);
        if (!tt->is_complete()) {
            SCOPES_ERROR(TypenameIncomplete, T);
        }
        if (tt->is_opaque()) {
            SCOPES_ERROR(OpaqueType, T);
        }
        return tt->storage();
    } break;
    default: return T;
    }
}

SCOPES_RESULT(const Type *) qualified_storage_type(const Type *T) {
    //SCOPES_RESULT_TYPE(const Type *);
    auto rq = try_qualifier<ReferQualifier>(T);
    if (rq) {
        T = strip_qualifiers(T);
        return pointer_type(T, rq->flags, rq->storage_class);
    } else {
        return storage_type(T);
    }
}

//------------------------------------------------------------------------------
// TYPE CHECK PREDICATES
//------------------------------------------------------------------------------

SCOPES_RESULT(void) verify(const Type *need, const Type *have) {
    SCOPES_RESULT_TYPE(void);
    if (strip_lifetime(need) != strip_lifetime(have)) {
        SCOPES_ERROR(ParameterTypeMismatch, need, have);
    }
    return {};
}

SCOPES_RESULT(void) verify_integer(const Type *type) {
    SCOPES_RESULT_TYPE(void);
    if (type->kind() != TK_Integer) {
        SCOPES_ERROR(ParameterTypeMismatch, TYPE_Integer, type);
    }
    return {};
}

SCOPES_RESULT(void) verify_real(const Type *type) {
    SCOPES_RESULT_TYPE(void);
    if (type->kind() != TK_Real) {
        SCOPES_ERROR(ParameterTypeMismatch, TYPE_Real, type);
    }
    return {};
}

SCOPES_RESULT(void) verify_range(size_t idx, size_t count) {
    SCOPES_RESULT_TYPE(void);
    if (idx >= count) {
        SCOPES_ERROR(IndexOutOfRange, idx, count);
    }
    return {};
}

//------------------------------------------------------------------------------

#define DEFINE_OPAQUE_TYPENAME(NAME, T, SUPERT) \
    T = opaque_typename_type(String::from(NAME), SUPERT);

#define DEFINE_BASIC_TYPE(NAME, CT, T, SUPERT, BODY) { \
        auto tn = plain_typename_type(String::from(NAME), SUPERT, BODY).assert_ok(); \
        assert(sizeof(CT) == size_of(tn).assert_ok()); \
        T = tn; \
    }

#define DEFINE_OPAQUE_HANDLE_TYPE(NAME, CT, T, SUPERT) { \
        auto tn = plain_typename_type(String::from(NAME), SUPERT, \
            native_opaque_pointer_type( \
                opaque_typename_type(String::from("_" NAME), nullptr))).assert_ok(); \
        T = tn; \
    }

void init_types() {
    DEFINE_OPAQUE_TYPENAME("typename", TYPE_Typename, nullptr);

    TYPE_Nothing = incomplete_typename_type(String::from("Nothing"), nullptr);
    DEFINE_OPAQUE_TYPENAME("noreturn", TYPE_NoReturn, nullptr);

    DEFINE_OPAQUE_TYPENAME("immutable", TYPE_Immutable, nullptr);
    DEFINE_OPAQUE_TYPENAME("aggregate", TYPE_Aggregate, nullptr);
    DEFINE_OPAQUE_TYPENAME("opaquepointer", TYPE_OpaquePointer, nullptr);

    TYPE_Sampler = sampler_type();

    DEFINE_OPAQUE_TYPENAME("integer", TYPE_Integer, TYPE_Immutable);
    DEFINE_OPAQUE_TYPENAME("real", TYPE_Real, TYPE_Immutable);
    DEFINE_OPAQUE_TYPENAME("vector", TYPE_Vector, TYPE_Immutable);
    DEFINE_OPAQUE_TYPENAME("matrix", TYPE_Matrix, TYPE_Immutable);
    DEFINE_OPAQUE_TYPENAME("CEnum", TYPE_CEnum, TYPE_Immutable);

    DEFINE_OPAQUE_TYPENAME("array", TYPE_Array, TYPE_Aggregate);
    DEFINE_OPAQUE_TYPENAME("zarray", TYPE_ZArray, TYPE_Array);
    DEFINE_OPAQUE_TYPENAME("tuple", TYPE_Tuple, TYPE_Aggregate);
    DEFINE_OPAQUE_TYPENAME("union", TYPE_Union, nullptr);

    DEFINE_OPAQUE_TYPENAME("pointer", TYPE_Pointer, nullptr);
    DEFINE_OPAQUE_TYPENAME("Qualify", TYPE_Qualify, nullptr);
    DEFINE_OPAQUE_TYPENAME("Arguments", TYPE_Arguments, nullptr);
    DEFINE_OPAQUE_TYPENAME("Raises", TYPE_Raises, nullptr);
    DEFINE_OPAQUE_TYPENAME("constant", TYPE_Constant, nullptr);
    DEFINE_OPAQUE_TYPENAME("function", TYPE_Function, nullptr);
    DEFINE_OPAQUE_TYPENAME("Image", TYPE_Image, nullptr);
    DEFINE_OPAQUE_TYPENAME("SampledImage", TYPE_SampledImage, nullptr);
    DEFINE_OPAQUE_TYPENAME("CStruct", TYPE_CStruct, nullptr);
    DEFINE_OPAQUE_TYPENAME("CUnion", TYPE_CUnion, nullptr);

    TYPE_Bool = integer_type(1, false);

    TYPE_I8 = integer_type(8, true);
    TYPE_I16 = integer_type(16, true);
    TYPE_I32 = integer_type(32, true);
    TYPE_I64 = integer_type(64, true);

    TYPE_U8 = integer_type(8, false);
    TYPE_U16 = integer_type(16, false);
    TYPE_U32 = integer_type(32, false);
    TYPE_U64 = integer_type(64, false);

    TYPE_F16 = real_type(16);
    TYPE_F32 = real_type(32);
    TYPE_F64 = real_type(64);
    TYPE_F80 = real_type(80);
    TYPE_F128 = real_type(128);

#if defined(__aarch64__)
    TYPE_Char = TYPE_U8;
#else
    TYPE_Char = TYPE_I8;
#endif

    DEFINE_BASIC_TYPE("usize", size_t, TYPE_USize, TYPE_Integer, TYPE_U64);

    const Type *_TypePtr = native_ro_pointer_type(opaque_typename_type(String::from("_type"),nullptr));
    TYPE_Type = plain_typename_type(String::from("type"), TYPE_OpaquePointer, _TypePtr).assert_ok();
    TYPE_Unknown = plain_typename_type(String::from("Unknown"), nullptr, _TypePtr).assert_ok();
    TYPE_Variadic = opaque_typename_type(String::from("..."), nullptr);
    cast<TypenameType>(TYPE_Nothing)->complete(tuple_type({}).assert_ok(), TNF_Plain).assert_ok();

    DEFINE_BASIC_TYPE("Symbol", Symbol, TYPE_Symbol, TYPE_Immutable, TYPE_U64);
    DEFINE_BASIC_TYPE("Builtin", Builtin, TYPE_Builtin, nullptr, TYPE_U64);

    DEFINE_OPAQUE_HANDLE_TYPE("_Value", Value, TYPE__Value, nullptr);

    DEFINE_OPAQUE_HANDLE_TYPE("SourceFile", SourceFile, TYPE_SourceFile, nullptr);
    DEFINE_OPAQUE_HANDLE_TYPE("Closure", Closure, TYPE_Closure, nullptr);
    DEFINE_OPAQUE_HANDLE_TYPE("Scope", Scope, TYPE_Scope, nullptr);
    DEFINE_OPAQUE_HANDLE_TYPE("string", String, TYPE_String, TYPE_OpaquePointer);
    DEFINE_OPAQUE_HANDLE_TYPE("List", List, TYPE_List, nullptr);
    DEFINE_OPAQUE_HANDLE_TYPE("Error", Error, TYPE_Error, nullptr);

    DEFINE_OPAQUE_HANDLE_TYPE("Anchor", Anchor, TYPE_Anchor, nullptr);


    DEFINE_BASIC_TYPE("Value", ValueRef, TYPE_ValueRef, nullptr,
        tuple_type({ TYPE__Value, TYPE_Anchor }).assert_ok());

    DEFINE_BASIC_TYPE("CompileStage", ValueRef, TYPE_CompileStage,
        nullptr, storage_type(TYPE_ValueRef).assert_ok());

    DEFINE_BASIC_TYPE("SpiceMacro", void*, TYPE_ASTMacro, nullptr,
        native_opaque_pointer_type(
            raising_function_type(
                TYPE_ValueRef, { TYPE_ValueRef })
        ));

#define T(TYPE, TYPENAME) \
    assert(TYPE);
    B_TYPES()
#undef T
}

#undef DEFINE_OPAQUE_TYPENAME
#undef DEFINE_BASIC_TYPE
#undef DEFINE_OPAQUE_HANDLE_TYPE

} // namespace scopes

