/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_EXECUTION_HPP
#define SCOPES_EXECUTION_HPP

#include <llvm-c/OrcBindings.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/ExecutionEngine.h>

#include <stdint.h>
#include <unordered_map>

#include "result.hpp"

namespace scopes {

struct Symbol;

//extern LLVMOrcJITStackRef orc;
//extern LLVMTargetMachineRef target_machine;

typedef std::unordered_map<std::string, const void *> PointerMap;

SCOPES_RESULT(void) init_execution();
SCOPES_RESULT(void) add_module(LLVMModuleRef module,
    const PointerMap &map, uint64_t compiler_flags);
SCOPES_RESULT(uint64_t) get_address(const char *name);
//SCOPES_RESULT(void *) get_pointer_to_global(LLVMValueRef g);
void *local_aware_dlsym(Symbol name);
LLVMTargetMachineRef get_target_machine();
void add_jit_event_listener(LLVMJITEventListenerRef listener);

void init_llvm();

} // namespace scopes

#endif // SCOPES_EXECUTION_HPP