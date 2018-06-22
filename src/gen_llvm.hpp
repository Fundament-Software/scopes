/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_GEN_LLVM_HPP
#define SCOPES_GEN_LLVM_HPP

#include "any.hpp"
#include "result.hpp"

namespace scopes {

struct ASTFunction;

SCOPES_RESULT(void) compile_object(const String *path, Scope *scope, uint64_t flags);
SCOPES_RESULT(Any) compile(ASTFunction *fn, uint64_t flags);

} // namespace scopes

#endif // SCOPES_GEN_LLVM_HPP