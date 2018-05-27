/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_EXECUTION_HPP
#define SCOPES_EXECUTION_HPP

#include <llvm-c/ExecutionEngine.h>

namespace scopes {

extern LLVMExecutionEngineRef ee;

void init_llvm();

} // namespace scopes

#endif // SCOPES_EXECUTION_HPP