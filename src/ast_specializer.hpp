/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_AST_SPECIALIZER_HPP
#define SCOPES_AST_SPECIALIZER_HPP

#include "result.hpp"
#include "type.hpp"

#include <stdint.h>

namespace scopes {

struct ASTFunction;
struct Template;
struct Type;
struct Closure;
struct List;
struct Builtin;
struct Symbol;

SCOPES_RESULT(const Type *) extract_type_constant(ASTNode *value);
SCOPES_RESULT(const Closure *) extract_closure_constant(ASTNode *value);
SCOPES_RESULT(const List *) extract_list_constant(ASTNode *value);
SCOPES_RESULT(const String *) extract_string_constant(ASTNode *value);
SCOPES_RESULT(Builtin) extract_builtin_constant(ASTNode *value);
SCOPES_RESULT(Symbol) extract_symbol_constant(ASTNode *value);
SCOPES_RESULT(uint64_t) extract_integer_constant(ASTNode *value);
const Type *try_get_const_type(ASTNode *node);
const String *try_extract_string(ASTNode *node);

SCOPES_RESULT(ASTFunction *) specialize(ASTFunction *frame, Template *func, const ArgTypes &types);

} // namespace scopes

#endif // SCOPES_AST_SPECIALIZER_HPP