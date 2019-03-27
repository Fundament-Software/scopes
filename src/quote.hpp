/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_QUOTE_HPP
#define SCOPES_QUOTE_HPP

#include "result.hpp"
#include "type.hpp"
#include "value.hpp"

namespace scopes {

struct ASTContext;

const ValueRef &unwrap_value(const Type *T, const ValueRef &value);
const ValueRef &wrap_value(const Type *T, const ValueRef &value);
SCOPES_RESULT(TypedValueRef) quote(const ASTContext &ctx, const ValueRef &node);

} // namespace scopes

#endif // SCOPES_QUOTE_HPP
