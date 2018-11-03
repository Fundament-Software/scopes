/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_KEYED_HPP
#define SCOPES_KEYED_HPP

#include "type.hpp"
#include "symbol.hpp"
#include "scopes/scopes.h"

namespace scopes {

//------------------------------------------------------------------------------
// KEYED TYPE
//------------------------------------------------------------------------------

struct KeyedType : Qualifier {
    static bool classof(const Type *T);

    KeyedType(Symbol key, const Type *type);
    void stream_name(StyledStream &ss) const;

    Symbol key;
};

//------------------------------------------------------------------------------

const Type *keyed_type(Symbol key, const Type *type);

sc_symbol_type_tuple_t key_type(const Type *type);

} // namespace scopes

#endif // SCOPES_KEYED_HPP
