/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_TYPENAME_HPP
#define SCOPES_TYPENAME_HPP

#include "../type.hpp"
#include "../result.hpp"

#include <unordered_set>

namespace scopes {

enum TypenameFlags {
    //
    TNF_Plain = (1 << 0),
    TNF_Complete = (1 << 1),
};

//------------------------------------------------------------------------------
// TYPENAME
//------------------------------------------------------------------------------

struct TypenameType : Type {
    static std::unordered_set<Symbol, Symbol::Hash> used_names;

    void stream_name(StyledStream &ss) const;
    const String *name() const;
    static bool classof(const Type *T);

    TypenameType(const String *name, const Type *super_type);

    SCOPES_RESULT(void) complete() const;
    SCOPES_RESULT(void) complete(const Type *_type, uint32_t flags) const;

    bool is_complete() const;
    bool is_plain() const;
    bool is_opaque() const;

    const Type *super() const;

    const Type *storage() const;

protected:
    mutable const Type *storage_type;
    const Type *super_type;
    const String *_name;
    mutable uint32_t flags;
};

// always generates a new type
const TypenameType *incomplete_typename_type(const String *name, const Type *supertype);
const TypenameType *opaque_typename_type(const String *name, const Type *supertype);
SCOPES_RESULT(const TypenameType *) plain_typename_type(const String *name, const Type *supertype, const Type *storage_type);
SCOPES_RESULT(const TypenameType *) unique_typename_type(const String *name, const Type *supertype, const Type *storage_type);

SCOPES_RESULT(const Type *) storage_type(const Type *T);

} // namespace scopes

#endif // SCOPES_TYPENAME_HPP