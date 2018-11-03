/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_TYPENAME_HPP
#define SCOPES_TYPENAME_HPP

#include "type.hpp"
#include "result.hpp"

#include <unordered_set>

namespace scopes {

//------------------------------------------------------------------------------
// TYPENAME
//------------------------------------------------------------------------------

struct TypenameType : Type {
    static std::unordered_set<Symbol, Symbol::Hash> used_names;

    void stream_name(StyledStream &ss) const;
    const String *name() const;
    static bool classof(const Type *T);

    TypenameType(const String *name);

    SCOPES_RESULT(void) finalize(const Type *_type);

    bool finalized() const;
    bool is_unique() const;
    void set_unique();

    const Type *super() const;

    const Type *storage_type;
    const Type *super_type;
    const String *_name;
    bool unique;
};

// always generates a new type
const Type *typename_type(const String *name);

SCOPES_RESULT(const Type *) storage_type(const Type *T);

} // namespace scopes

#endif // SCOPES_TYPENAME_HPP