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

TypenameType::TypenameType(const String *name)
    : Type(TK_Typename), storage_type(nullptr), super_type(nullptr),
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

SCOPES_RESULT(void) TypenameType::finalize(const Type *_type, uint32_t _flags) {
    SCOPES_RESULT_TYPE(void);
    if (finalized()) {
        SCOPES_ERROR(TypenameIsFinal, this, storage_type);
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

bool TypenameType::finalized() const { return storage_type != nullptr; }
bool TypenameType::is_plain() const {
    return (flags & TNF_Plain) == TNF_Plain;
}

const Type *TypenameType::super() const {
    if (!super_type) return TYPE_Typename;
    return super_type;
}

std::unordered_set<Symbol, Symbol::Hash> TypenameType::used_names;

//------------------------------------------------------------------------------

// always generates a new type
const Type *typename_type(const String *name) {
    return new TypenameType(name);
}

SCOPES_RESULT(const Type *) storage_type(const Type *T) {
    SCOPES_RESULT_TYPE(const Type *);
    T = strip_qualifiers(T);
    switch(T->kind()) {
    case TK_Typename: {
        const TypenameType *tt = cast<TypenameType>(T);
        if (!tt->finalized()) {
            SCOPES_ERROR(OpaqueType, T);
        }
        return tt->storage_type;
    } break;
    default: return T;
    }
}


} // namespace scopes
