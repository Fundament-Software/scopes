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

struct ASTContext {
    ASTContext for_loop(LoopLabel *loop) const;

    ASTContext for_try(Label *except) const;
    ASTContext for_break(Label *_break) const;

    ASTContext with_block(Block &_block) const;
    ASTContext with_frame(Function *frame) const;

    ASTContext();

    ASTContext(Function *_function, Function *_frame,
        LoopLabel *_loop, Label *_except, Label *_break, Block *_block);

    static ASTContext from_function(Function *fn);

    void append(TypedValue *value) const;
    void merge_block(Block &_block) const;

    const Type *fix_merge_type(const Type *T) const;
    int unique_id() const;
    void move(int id) const;

    Function *function;
    Function *frame;
    LoopLabel *loop;
    Label *except;
    Label *_break;
    Block *block;
};

SCOPES_RESULT(const Type *) extract_type_constant(Value *value);
SCOPES_RESULT(const Closure *) extract_closure_constant(Value *value);
SCOPES_RESULT(const List *) extract_list_constant(Value *value);
SCOPES_RESULT(const String *) extract_string_constant(Value *value);
SCOPES_RESULT(Builtin) extract_builtin_constant(Value *value);
SCOPES_RESULT(Symbol) extract_symbol_constant(Value *value);
SCOPES_RESULT(uint64_t) extract_integer_constant(Value *value);
SCOPES_RESULT(Function *) extract_function_constant(Value *value);
SCOPES_RESULT(ConstAggregate *) extract_vector_constant(Value *value);
const Type *try_get_const_type(Value *node);
const String *try_extract_string(Value *node);
bool is_value_stage_constant(Value *value);
SCOPES_RESULT(void) map_keyed_arguments(const Anchor *anchor,
    Values &outargs, const Values &values, const Symbols &symbols, bool varargs);

SCOPES_RESULT(Function *) prove(Function *frame, Template *func, const Types &types);
SCOPES_RESULT(TypedValue *) prove(const ASTContext &ctx, Value *node);

} // namespace scopes

#endif // SCOPES_AST_PROVER_HPP
