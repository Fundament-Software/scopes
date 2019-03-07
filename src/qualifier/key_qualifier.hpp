/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_KEYED_HPP
#define SCOPES_KEYED_HPP

#include "../symbol.hpp"
#include "../type/qualify_type.hpp"
#include "scopes/scopes.h"

namespace scopes {

//------------------------------------------------------------------------------
// KEYED TYPE
//------------------------------------------------------------------------------

struct KeyQualifier : Qualifier {
    enum { Kind = QK_Key };
    static bool classof(const Qualifier *T);

    KeyQualifier(Symbol key);
    void stream_prefix(StyledStream &ss) const;
    void stream_postfix(StyledStream &ss) const;

    Symbol key;
};

//------------------------------------------------------------------------------

const Type *key_type(Symbol key, const Type *type);

sc_symbol_type_tuple_t type_key(const Type *type);

} // namespace scopes

#endif // SCOPES_KEYED_HPP
