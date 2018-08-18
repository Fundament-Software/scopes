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

const Type *arguments_type(const ArgTypes &values);
const Type *keyed_arguments_type(const KeyedTypes &values);
const Type *empty_arguments_type();

bool is_arguments_type(const Type *T);

} // namespace scopes

#endif // SCOPES_ARGUMENTS_HPP