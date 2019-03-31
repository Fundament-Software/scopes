/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_GEN_LLVM_HPP
#define SCOPES_GEN_LLVM_HPP

#include "result.hpp"
#include "valueref.inc"

#include <stdint.h>

namespace scopes {

struct Function;
struct String;
struct ConstPointer;
struct Scope;

SCOPES_RESULT(void) compile_object(const String *path, Scope *scope, uint64_t flags);
SCOPES_RESULT(ConstPointerRef) compile(const FunctionRef &fn, uint64_t flags);

} // namespace scopes

#endif // SCOPES_GEN_LLVM_HPP