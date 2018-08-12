/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_ARGUMENTS_HPP
#define SCOPES_ARGUMENTS_HPP

#include "type.hpp"
#include "symbol.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// ARGUMENTS LABEL TYPE
//------------------------------------------------------------------------------

struct ArgumentsType : Type {
    static bool classof(const Type *T);

    const Type *type_at_index(size_t i) const;

    void stream_name(StyledStream &ss) const;
    ArgumentsType(const KeyedTypes &_values);

    KeyedTypes values;
};

const Type *arguments_type(const ArgTypes &values);
const Type *keyed_arguments_type(const KeyedTypes &values);

} // namespace scopes

#endif // SCOPES_ARGUMENTS_HPP