/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "typename.hpp"
#include "error.hpp"
#include "return.hpp"

#include "llvm/Support/Casting.h"

namespace scopes {

using llvm::isa;
using llvm::cast;
using llvm::dyn_cast;

//------------------------------------------------------------------------------
// TYPENAME
//------------------------------------------------------------------------------

bool TypenameType::classof(const Type *T) {
    return T->kind() == TK_Typename;
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

void TypenameType::finalize(const Type *_type) {
    if (finalized()) {
        StyledString ss;
        ss.out << "typename " << _type << " is already final";
        location_error(ss.str());
    }
    if (isa<TypenameType>(_type)) {
        StyledString ss;
        ss.out << "cannot use typename " << _type << " as storage type";
        location_error(ss.str());
    }
    storage_type = _type;
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

const Type *storage_type(const Type *T) {
    switch(T->kind()) {
    case TK_Typename: {
        const TypenameType *tt = cast<TypenameType>(T);
        if (!tt->finalized()) {
            StyledString ss;
            ss.out << "type " << T << " is opaque";
            location_error(ss.str());
        }
        return tt->storage_type;
    } break;
    case TK_ReturnLabel: {
        const ReturnLabelType *rlt = cast<ReturnLabelType>(T);
        return storage_type(rlt->return_type);
    } break;
    default: return T;
    }
}


} // namespace scopes
