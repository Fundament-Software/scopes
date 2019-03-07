/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_EXECUTION_HPP
#define SCOPES_EXECUTION_HPP

#include <llvm-c/OrcBindings.h>
#include <llvm-c/TargetMachine.h>

#include <stdint.h>

#include "result.hpp"

namespace scopes {

struct Symbol;

//extern LLVMOrcJITStackRef orc;
//extern LLVMTargetMachineRef target_machine;

void init_execution();
SCOPES_RESULT(void) add_module(LLVMModuleRef module);
uint64_t get_address(const char *name);
void *get_pointer_to_global(LLVMValueRef g);
void *local_aware_dlsym(Symbol name);
LLVMTargetMachineRef get_target_machine();

void init_llvm();

} // namespace scopes

#endif // SCOPES_EXECUTION_HPP