/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_ERROR_HPP
#define SCOPES_ERROR_HPP

#include "any.hpp"
#include "result.hpp"
#include "scopes/config.h"

namespace scopes {

//------------------------------------------------------------------------------

struct String;
struct Anchor;
struct StyledStream;

void set_active_anchor(const Anchor *anchor);
const Anchor *get_active_anchor();

//------------------------------------------------------------------------------

struct Error {
    const Anchor *anchor;
    const String *msg;

    Error();

    Error(const Anchor *_anchor, const String *_msg);
};

//------------------------------------------------------------------------------

void set_last_error(const Any &err);
Any get_last_error();

void print_error(const Any &value);
void stream_error(StyledStream &ss, const Any &value);
void stream_error_string(StyledStream &ss, const Any &value);

void set_last_location_error(const String *msg);
Any make_location_error(const String *msg);
Any make_runtime_error(const String *msg);

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

struct ASTNode;
struct ASTSymbol;

// specializer errors
SCOPES_RESULT(void) error_invalid_call_type(ASTNode *callee);
SCOPES_RESULT(void) error_invalid_condition_type(ASTNode *cond);
SCOPES_RESULT(void) error_constant_expected(ASTNode *value);
SCOPES_RESULT(void) error_unbound_symbol(ASTSymbol *value);
SCOPES_RESULT(void) error_cannot_merge_expression_types(const Type *T1, const Type *T2);
SCOPES_RESULT(void) error_noreturn_not_last_expression();
SCOPES_RESULT(void) error_cannot_type_builtin(const Builtin &builtin);
SCOPES_RESULT(void) error_illegal_repeat_outside_loop();
SCOPES_RESULT(void) error_illegal_break_outside_loop();
SCOPES_RESULT(void) error_variadic_symbol_not_in_last_place();
SCOPES_RESULT(void) error_illegal_return_in_inline();

// code generator errors
SCOPES_RESULT(void) error_gen_invalid_call_type(const char *target, ASTNode *callee);
SCOPES_RESULT(void) error_gen_unbound_symbol(const char *target, ASTSymbol *value);

//------------------------------------------------------------------------------

} // namespace scopes

#endif // SCOPES_ERROR_HPP