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
    ASTContext for_loop(const LoopLabelRef &loop) const;

    ASTContext for_try(const LabelRef &except) const;
    ASTContext for_break(const LabelRef &_break) const;

    ASTContext with_block(Block &_block) const;
    ASTContext with_frame(const FunctionRef &frame) const;

    ASTContext();

    ASTContext(const FunctionRef &_function, const FunctionRef &_frame,
        const LoopLabelRef &_loop, const LabelRef &_except,
        const LabelRef &_break,
        Block *_block);

    static ASTContext from_function(const FunctionRef &fn);

    void append(const TypedValueRef &value) const;
    void merge_block(Block &_block) const;

    const Type *fix_merge_type(const Type *T) const;
    int unique_id() const;
    void move(int id, const ValueRef &mover) const;

    FunctionRef function;
    FunctionRef frame;
    LoopLabelRef loop;
    LabelRef except;
    LabelRef _break;
    Block *block;
};

SCOPES_RESULT(const Type *) extract_type_constant(const ValueRef &value);
SCOPES_RESULT(const Closure *) extract_closure_constant(const ValueRef &value);
SCOPES_RESULT(const List *) extract_list_constant(const ValueRef &value);
SCOPES_RESULT(const String *) extract_string_constant(const ValueRef &value);
SCOPES_RESULT(Builtin) extract_builtin_constant(const ValueRef &value);
SCOPES_RESULT(Symbol) extract_symbol_constant(const ValueRef &value);
SCOPES_RESULT(uint64_t) extract_integer_constant(const ValueRef &value);
SCOPES_RESULT(FunctionRef) extract_function_constant(const ValueRef &value);
SCOPES_RESULT(TemplateRef) extract_template_constant(const ValueRef &value);
SCOPES_RESULT(ConstAggregateRef) extract_vector_constant(const ValueRef &value);
const Type *try_get_const_type(const ValueRef &node);
const String *try_extract_string(const ValueRef &node);
bool is_value_stage_constant(const ValueRef &value);
SCOPES_RESULT(void) map_keyed_arguments(const Anchor *anchor,
    Values &outargs, const Values &values, const Symbols &symbols, bool varargs);

SCOPES_RESULT(FunctionRef) prove(const FunctionRef &frame, const TemplateRef &func, const Types &types);
SCOPES_RESULT(TypedValueRef) prove(const ASTContext &ctx, const ValueRef &node);
SCOPES_RESULT(TypedValueRef) prove(const ValueRef &node);

} // namespace scopes

#endif // SCOPES_AST_PROVER_HPP
