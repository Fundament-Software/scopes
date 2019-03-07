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

Value *unwrap_value(const Type *T, Value *value);
Value *wrap_value(const Type *T, Value *value);
SCOPES_RESULT(TypedValue *) quote(const ASTContext &ctx, Value *node);

} // namespace scopes

#endif // SCOPES_QUOTE_HPP
