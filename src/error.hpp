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
    const Anchor *anchor;
    const String *msg;
};

//------------------------------------------------------------------------------

void set_last_error(Error *err);
const Error *get_last_error();
void add_error_trace(Value *value);

void print_error(const Error *value);
void stream_error(StyledStream &ss, const Error *value);
void stream_error_string(StyledStream &ss, const Error *value);

void set_last_location_error(const String *msg);
Error *make_location_error(const String *msg);
Error *make_runtime_error(const String *msg);

#if SCOPES_EARLY_ABORT
#define SCOPES_LOCATION_ERROR(MSG) \
    set_last_location_error((MSG)); \
    assert(false); \
    return Result<_result_type>();
#else
#define SCOPES_LOCATION_ERROR(MSG) \
    set_last_location_error((MSG)); \
    return Result<_result_type>();
#endif

void location_message(const Anchor *anchor, const String* str);

//------------------------------------------------------------------------------

struct Value;
struct SymbolValue;
struct Template;

// specializer errors
SCOPES_RESULT(void) error_invalid_call_type(Value *callee);
SCOPES_RESULT(void) error_invalid_condition_type(Value *cond);
SCOPES_RESULT(void) error_invalid_operands(const Type *A, const Type *B);
SCOPES_RESULT(void) error_constant_expected(Value *value);
SCOPES_RESULT(void) error_unbound_symbol(SymbolValue *value);
SCOPES_RESULT(void) error_cannot_merge_expression_types(const Type *T1, const Type *T2);
SCOPES_RESULT(void) error_noreturn_not_last_expression();
SCOPES_RESULT(void) error_cannot_type_builtin(const Builtin &builtin);
SCOPES_RESULT(void) error_illegal_repeat_outside_loop();
SCOPES_RESULT(void) error_illegal_break_outside_loop();
SCOPES_RESULT(void) error_variadic_symbol_not_in_last_place();
SCOPES_RESULT(void) error_illegal_return_in_inline();
SCOPES_RESULT(void) error_untyped_recursive_call();
SCOPES_RESULT(void) error_cannot_find_frame(Template *func);

// code generator errors
SCOPES_RESULT(void) error_gen_invalid_call_type(const char *target, Value *callee);
SCOPES_RESULT(void) error_gen_unbound_symbol(const char *target, SymbolValue *value);

//------------------------------------------------------------------------------

} // namespace scopes

#endif // SCOPES_ERROR_HPP