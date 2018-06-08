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

namespace scopes {

struct Symbol;

extern LLVMOrcJITStackRef orc;
extern LLVMTargetMachineRef target_machine;

void init_orc();
void add_orc_module(LLVMModuleRef module);
uint64_t get_orc_address(const char *name);
void *get_orc_pointer_to_global(LLVMValueRef g);
void *local_aware_dlsym(Symbol name);

void init_llvm();

} // namespace scopes

#endif // SCOPES_EXECUTION_HPP