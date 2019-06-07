/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_LIFETIME_HPP
#define SCOPES_LIFETIME_HPP

#include "result.hpp"
#include "valueref.inc"

namespace scopes {

struct ASTContext;

SCOPES_RESULT(void) tag_instruction(const ASTContext &ctx, const InstructionRef &node);

} // namespace scopes

#endif // SCOPES_LIFETIME_HPP
