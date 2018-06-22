/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_AST_SPECIALIZER_HPP
#define SCOPES_AST_SPECIALIZER_HPP

#include "result.hpp"
#include "type.hpp"

namespace scopes {

struct ASTFunction;
struct Template;

SCOPES_RESULT(ASTFunction *) specialize(ASTFunction *frame, Template *func, const ArgTypes &types);

} // namespace scopes

#endif // SCOPES_AST_SPECIALIZER_HPP