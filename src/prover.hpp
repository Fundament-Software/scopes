/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_AST_PROVER_HPP
#define SCOPES_AST_PROVER_HPP

#include "result.hpp"
#include "type.hpp"
#include "value.hpp"

#include <stdint.h>

namespace scopes {

struct Type;
struct Closure;
struct List;
struct Builtin;
struct Symbol;

SCOPES_RESULT(const Type *) extract_type_constant(Value *value);
SCOPES_RESULT(const Closure *) extract_closure_constant(Value *value);
SCOPES_RESULT(const List *) extract_list_constant(Value *value);
SCOPES_RESULT(const String *) extract_string_constant(Value *value);
SCOPES_RESULT(Builtin) extract_builtin_constant(Value *value);
SCOPES_RESULT(Symbol) extract_symbol_constant(Value *value);
SCOPES_RESULT(uint64_t) extract_integer_constant(Value *value);
const Type *try_get_const_type(Value *node);
const String *try_extract_string(Value *node);
Value *rekey(const Anchor *anchor, Symbol key, Value *value);
SCOPES_RESULT(void) map_keyed_arguments(const Anchor *anchor,
    Values &outargs, const Values &values, const Symbols &symbols, bool varargs);

SCOPES_RESULT(Function *) prove(Function *frame, Template *func, const ArgTypes &types);

} // namespace scopes

#endif // SCOPES_AST_PROVER_HPP
