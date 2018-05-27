/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_GEN_LLVM_HPP
#define SCOPES_GEN_LLVM_HPP

#include "any.hpp"

namespace scopes {

struct Label;

void compile_object(const String *path, Scope *scope, uint64_t flags);
Any compile(Label *fn, uint64_t flags);

} // namespace scopes

#endif // SCOPES_GEN_LLVM_HPP