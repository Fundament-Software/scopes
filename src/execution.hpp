/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_EXECUTION_HPP
#define SCOPES_EXECUTION_HPP

//#include <llvm-c/OrcBindings.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/ExecutionEngine.h>

#include <stdint.h>
#include "absl/container/flat_hash_map.h"
#include <string>

#include "result.hpp"

namespace scopes {

struct Symbol;
struct String;

//extern LLVMOrcJITStackRef orc;
//extern LLVMTargetMachineRef target_machine;

typedef absl::flat_hash_map<std::string, const void *> PointerMap;

const String *get_default_target_triple();
SCOPES_RESULT(void) init_execution();
SCOPES_RESULT(void) add_module(LLVMModuleRef module,
    const PointerMap &map, uint64_t compiler_flags);
SCOPES_RESULT(uint64_t) get_address(const char *name);
//SCOPES_RESULT(void *) get_pointer_to_global(LLVMValueRef g);
void *local_aware_dlsym(Symbol name);
LLVMTargetMachineRef get_jit_target_machine();
LLVMTargetMachineRef get_object_target_machine();
SCOPES_RESULT(void) add_object(const char *path);
void build_and_run_opt_passes(LLVMModuleRef module, int opt_level);
void print_disassembly(std::string symbol, void *pfunc);
void enable_disassembly(bool enable);

void init_llvm();

} // namespace scopes

#endif // SCOPES_EXECUTION_HPP