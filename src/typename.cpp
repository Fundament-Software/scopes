/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "typename.hpp"
#include "error.hpp"
#include "return.hpp"
#include "dyn_cast.inc"

namespace scopes {

//------------------------------------------------------------------------------
// TYPENAME
//------------------------------------------------------------------------------

bool TypenameType::classof(const Type *T) {
    return T->kind() == TK_Typename;
}

const String *TypenameType::name() const {
    return _name;
}

void TypenameType::stream_name(StyledStream &ss) const {
    ss << _name->data;
}

TypenameType::TypenameType(const String *name)
    : Type(TK_Typename), storage_type(nullptr), super_type(nullptr) {
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

SCOPES_RESULT(void) TypenameType::finalize(const Type *_type) {
    SCOPES_RESULT_TYPE(void);
    if (finalized()) {
        StyledString ss;
        ss.out << "typename " << _type << " is already final";
        SCOPES_LOCATION_ERROR(ss.str());
    }
    if (isa<TypenameType>(_type)) {
        StyledString ss;
        ss.out << "cannot use typename " << _type << " as storage type";
        SCOPES_LOCATION_ERROR(ss.str());
    }
    storage_type = _type;
    return true;
}

bool TypenameType::finalized() const { return storage_type != nullptr; }

const Type *TypenameType::super() const {
    if (!super_type) return TYPE_Typename;
    return super_type;
}

std::unordered_set<Symbol, Symbol::Hash> TypenameType::used_names;

//------------------------------------------------------------------------------

// always generates a new type
const Type *Typename(const String *name) {
    return new TypenameType(name);
}

SCOPES_RESULT(const Type *) storage_type(const Type *T) {
    SCOPES_RESULT_TYPE(const Type *);
    switch(T->kind()) {
    case TK_Typename: {
        const TypenameType *tt = cast<TypenameType>(T);
        if (!tt->finalized()) {
            StyledString ss;
            ss.out << "type " << T << " is opaque";
            SCOPES_LOCATION_ERROR(ss.str());
        }
        return tt->storage_type;
    } break;
    case TK_Return: {
        const ReturnType *rt = cast<ReturnType>(T);
        return storage_type(rt->return_type);
    } break;
    default: return T;
    }
}


} // namespace scopes
