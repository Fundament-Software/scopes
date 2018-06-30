/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_EXPANDER_HPP
#define SCOPES_EXPANDER_HPP

#include "result.hpp"

namespace scopes {

struct ASTNode;
struct Scope;
struct Template;

SCOPES_RESULT(Template *) expand_module(ASTNode *expr, Scope *scope = nullptr);
SCOPES_RESULT(Template *) expand_inline(ASTNode *expr, Scope *scope = nullptr);

} // namespace scopes

#endif // SCOPES_EXPANDER_HPP