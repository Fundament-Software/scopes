/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_ERROR_HPP
#define SCOPES_ERROR_HPP

#include "result.hpp"
#include "builtin.hpp"
#include "scopes/config.h"

#include <vector>

namespace scopes {

//------------------------------------------------------------------------------

struct Type;
struct String;
struct Anchor;
struct StyledStream;
struct Value;

void _set_active_anchor(const Anchor *anchor);
const Anchor *get_active_anchor();

struct ScopedAnchor {
    ScopedAnchor(const Anchor *anchor);
    ~ScopedAnchor();
    const Anchor *parent_anchor;
};

#define SCOPES_CAT(a, ...) SCOPES_PRIMITIVE_CAT(a, __VA_ARGS__)
#define SCOPES_PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__
#define SCOPES_ANCHOR(ANCHOR) ScopedAnchor SCOPES_CAT(_scoped_anchor, __LINE__)(ANCHOR)

//------------------------------------------------------------------------------

struct Error {
    Error();

    Error(const Anchor *_anchor, const String *_msg);

    std::vector<Value *> trace;
    std::vector<Value *> definitions;
    std::vector<const String *> messages;
    const Anchor *anchor;
    const String *msg;

    void append_error_trace(Value *value);
    void append_definition(Value *value);
};

//------------------------------------------------------------------------------

void print_error(const Error *value);
void stream_error(StyledStream &ss, const Error *value);
void stream_error_string(StyledStream &ss, const Error *value);

Error *make_location_error(const String *msg);
Error *make_location_error(const Anchor *anchor, const String *msg);
Error *make_runtime_error(const String *msg);

#if SCOPES_EARLY_ABORT
#define SCOPES_LOCATION_ERROR(MSG) \
    assert (false); \
    return Result<_result_type>::raise(make_location_error((MSG)));
#else
#define SCOPES_LOCATION_ERROR(MSG) \
    return Result<_result_type>::raise(make_location_error((MSG)));
#endif

void location_message(const Anchor *anchor, const String* str);

//------------------------------------------------------------------------------

struct Value;
struct Parameter;
struct Template;
struct Function;

// specializer errors
SCOPES_RESULT(void) error_invalid_call_type(Value *callee);
SCOPES_RESULT(void) error_invalid_condition_type(Value *cond);
SCOPES_RESULT(void) error_invalid_case_literal_type(Value *lit);
SCOPES_RESULT(void) error_label_expected(Value *value);
SCOPES_RESULT(void) error_duplicate_default_case();
SCOPES_RESULT(void) error_missing_default_case();
SCOPES_RESULT(void) error_argument_count_mismatch(int needed, int got);
SCOPES_RESULT(void) error_invalid_operands(const Type *A, const Type *B);
SCOPES_RESULT(void) error_argument_type_mismatch(const Type *expected, const Type *got);
SCOPES_RESULT(void) error_constant_expected(const Type *want, Value *value);
SCOPES_RESULT(void) error_unbound_symbol(Parameter *value);
SCOPES_RESULT(void) error_cannot_merge_expression_types(const char *context, const Type *T1, const Type *T2);
SCOPES_RESULT(void) error_noreturn_not_last_expression();
SCOPES_RESULT(void) error_noreturn_in_argument_list();
SCOPES_RESULT(void) error_cannot_type_builtin(const Builtin &builtin);
SCOPES_RESULT(void) error_illegal_repeat_outside_loop();
SCOPES_RESULT(void) error_illegal_break_outside_loop();
SCOPES_RESULT(void) error_variadic_symbol_not_in_last_place();
SCOPES_RESULT(void) error_untyped_recursive_call(Function *func);
SCOPES_RESULT(void) error_value_inaccessible_from_closure(Value *value, const Function *frame);

// code generator errors
SCOPES_RESULT(void) error_gen_invalid_call_type(const char *target, Value *callee);
SCOPES_RESULT(void) error_gen_unbound_symbol(const char *target, Parameter *value);

//------------------------------------------------------------------------------

} // namespace scopes

#endif // SCOPES_ERROR_HPP