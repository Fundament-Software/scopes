/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_TYPENAME_HPP
#define SCOPES_TYPENAME_HPP

#include "type.hpp"

#include <unordered_set>

namespace scopes {

//------------------------------------------------------------------------------
// TYPENAME
//------------------------------------------------------------------------------

struct TypenameType : Type {
    static std::unordered_set<Symbol, Symbol::Hash> used_names;

    static bool classof(const Type *T);

    TypenameType(const String *name);

    void finalize(const Type *_type);

    bool finalized() const;

    const Type *super() const;

    const Type *storage_type;
    const Type *super_type;
};

// always generates a new type
const Type *Typename(const String *name);

const Type *storage_type(const Type *T);

} // namespace scopes

#endif // SCOPES_TYPENAME_HPP