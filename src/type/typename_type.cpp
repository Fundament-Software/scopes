/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "typename_type.hpp"
#include "../error.hpp"
#include "tuple_type.hpp"
#include "pointer_type.hpp"
#include "arguments_type.hpp"
#include "qualify_type.hpp"
#include "../dyn_cast.inc"

namespace scopes {

//------------------------------------------------------------------------------
// TYPENAME
//------------------------------------------------------------------------------

const String *TypenameType::name() const {
    return _name;
}

void TypenameType::stream_name(StyledStream &ss) const {
    ss << _name->data;
}

TypenameType::TypenameType(const String *name, const Type *_super_type)
    : Type(TK_Typename), storage_type(nullptr), super_type(_super_type),
        _name(nullptr), flags(0) {
    auto newname = Symbol(name);
    size_t idx = 2;
    while (used_names.count(newname)) {
        // keep testing until we hit a name that's free
        auto ss = StyledString::plain();
        ss.out << name->data << "$" << idx++;
        newname = Symbol(ss.str());
    }
    used_names.insert(newname);
    _name = newname.name();
}

const Type *TypenameType::storage() const {
    return storage_type;
}

SCOPES_RESULT(void) TypenameType::complete() const {
    SCOPES_RESULT_TYPE(void);
    if (is_complete()) {
        SCOPES_ERROR(TypenameComplete, this);
    }
    flags = TNF_Complete;
    return {};
}

SCOPES_RESULT(void) TypenameType::complete(const Type *_type, uint32_t _flags) const {
    SCOPES_RESULT_TYPE(void);
    _flags |= TNF_Complete;
    if (is_complete()) {
        SCOPES_ERROR(TypenameComplete, this);
    }
    if (isa<TypenameType>(_type)) {
        SCOPES_ERROR(StorageTypeExpected, _type);
    }
    if ((_flags & TNF_Plain) && !::scopes::is_plain(_type)) {
        SCOPES_ERROR(PlainStorageTypeExpected, _type);
    }
    storage_type = _type;
    flags = _flags;
    return {};
}

bool TypenameType::is_opaque() const { return storage_type == nullptr; }
bool TypenameType::is_complete() const { return (flags & TNF_Complete) == TNF_Complete; }
bool TypenameType::is_plain() const { return (flags & TNF_Plain) == TNF_Plain; }

const Type *TypenameType::super() const {
    if (!super_type) return TYPE_Typename;
    return super_type;
}

std::unordered_set<Symbol, Symbol::Hash> TypenameType::used_names;

//------------------------------------------------------------------------------

// always generates a new type
const TypenameType *incomplete_typename_type(const String *name, const Type *supertype) {
    return new TypenameType(name, supertype);
}

const TypenameType *opaque_typename_type(const String *name, const Type *supertype) {
    auto TT = incomplete_typename_type(name, supertype);
    TT->complete().assert_ok();
    return TT;
}

SCOPES_RESULT(const TypenameType *) plain_typename_type(const String *name, const Type *supertype, const Type *storage_type) {
    SCOPES_RESULT_TYPE(const TypenameType *);
    auto TT = incomplete_typename_type(name, supertype);
    SCOPES_CHECK_RESULT(TT->complete(storage_type, TNF_Plain));
    return TT;
}

SCOPES_RESULT(const TypenameType *) unique_typename_type(const String *name, const Type *supertype, const Type *storage_type) {
    SCOPES_RESULT_TYPE(const TypenameType *);
    auto TT = incomplete_typename_type(name, supertype);
    SCOPES_CHECK_RESULT(TT->complete(storage_type, 0));
    return TT;
}

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


} // namespace scopes
