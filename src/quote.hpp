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

ValueRef unwrap_value(const Type *T, const ValueRef &value);
ValueRef wrap_value(const Type *T, const ValueRef &value, bool composite = false);
SCOPES_RESULT(TypedValueRef) quote(const ASTContext &ctx, const ValueRef &node);
ValueRef build_quoted_argument_list(const Anchor *_anchor, const Values &values);

} // namespace scopes

#endif // SCOPES_QUOTE_HPP
