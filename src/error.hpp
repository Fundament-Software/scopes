/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_ERROR_HPP
#define SCOPES_ERROR_HPP

#include "result.hpp"
#include "builtin.hpp"
#include "value.hpp"
#include "scopes/config.h"
#include "valueref.inc"

#include <vector>

namespace scopes {

/*
for any error, we're interested in this information:

* what went wrong?
* optional: what is expected behavior?
* optional: what can be done to fix it?
* which source line caused the error?
    * location of expression that produced the quote
    * location of expression that produced the expression
    * etc

insights:
* we can always tag a value with an anchor after it has been created
* the same values need different anchors (once when returned, once when referenced)
    * which means one origin stored in the value
    * one stored in the value reference
* we need to get the definition anchor as well as the reference anchor
* when an error occurs, we can always tag the error as it comes out
* we can auto-recognize value pointers in code and anchor them post-call?
* we can auto-recognize errors in code and add anchors on the failcase



*/

//------------------------------------------------------------------------------

struct Type;
struct String;
struct Anchor;
struct StyledStream;

#define SCOPES_CAT(a, ...) SCOPES_PRIMITIVE_CAT(a, __VA_ARGS__)
#define SCOPES_PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

//------------------------------------------------------------------------------

struct Error {
    Error();

    Error(const Anchor *_anchor, const String *_msg);

    std::vector<ValueRef> definitions;
    std::vector<const String *> messages;
    const Anchor *anchor;
    const String *msg;

    void append_definition(const ValueRef &value);
};

//------------------------------------------------------------------------------

void print_error(const Error *value);
void stream_error(StyledStream &ss, const Error *value);
void stream_error_string(StyledStream &ss, const Error *value);

Error *make_location_error(const Anchor *anchor, const String *msg);
Error *make_error(const String *msg);

#if SCOPES_EARLY_ABORT
#define SCOPES_LOCATION_ERROR(ANCHOR, MSG) \
    assert (false); \
    return Result<_result_type>::raise(make_location_error((ANCHOR),(MSG)));
#define SCOPES_ERROR(MSG) \
    assert (false); \
    return Result<_result_type>::raise(make_error((MSG)));
#else
#define SCOPES_LOCATION_ERROR(ANCHOR, MSG) \
    return Result<_result_type>::raise(make_location_error((ANCHOR),(MSG)));
#define SCOPES_ERROR(MSG) \
    return Result<_result_type>::raise(make_error((MSG)));
#endif
#define SCOPES_DEF_ERROR(DEF, MSG) { \
    auto err = make_error((MSG)); \
    err->append_definition((DEF)); \
    return Result<_result_type>::raise(err); \
}
#define SCOPES_LOCATION_DEF_ERROR(DEF, ANCHOR, MSG) { \
    auto err = make_location_error((ANCHOR),(MSG)); \
    err->append_definition((DEF)); \
    return Result<_result_type>::raise(err); \
}

void location_message(const Anchor *anchor, const String* str);

//------------------------------------------------------------------------------

struct Value;
struct Parameter;
struct Template;
struct Function;
struct FunctionType;

// specializer errors
SCOPES_RESULT(void) error_invalid_call_type(const TypedValueRef &callee);
SCOPES_RESULT(void) error_invalid_condition_type(const TypedValueRef &cond);
SCOPES_RESULT(void) error_invalid_case_literal_type(const TypedValueRef &lit);
SCOPES_RESULT(void) error_label_expected(const ValueRef &value);
SCOPES_RESULT(void) error_duplicate_default_case();
SCOPES_RESULT(void) error_missing_default_case();
SCOPES_RESULT(void) error_argument_count_mismatch(int needed, int got, const FunctionType *ft = nullptr);
SCOPES_RESULT(void) error_invalid_operands(const Type *A, const Type *B);
SCOPES_RESULT(void) error_argument_type_mismatch(const Type *expected, const Type *got);
SCOPES_RESULT(void) error_something_expected(const char *want, const ValueRef &value);
SCOPES_RESULT(void) error_constant_expected(const Type *want, const ValueRef &value);
SCOPES_RESULT(void) error_unbound_symbol(const ParameterRef &value);
SCOPES_RESULT(void) error_unbound_symbol(const ValueRef &value);
SCOPES_RESULT(void) error_cannot_merge_expression_types(const char *context, const Type *T1, const Type *T2);
SCOPES_RESULT(void) error_noreturn_not_last_expression();
SCOPES_RESULT(void) error_recursion_overflow();
SCOPES_RESULT(void) error_noreturn_in_argument_list();
SCOPES_RESULT(void) error_cannot_type_builtin(const Builtin &builtin);
SCOPES_RESULT(void) error_illegal_repeat_outside_loop();
SCOPES_RESULT(void) error_illegal_break_outside_loop();
SCOPES_RESULT(void) error_variadic_symbol_not_in_last_place(const ValueRef &value);
SCOPES_RESULT(void) error_untyped_recursive_call(const FunctionRef &func);
SCOPES_RESULT(void) error_recursive_function_changed_type(const FunctionRef &func, const Type *T1, const Type *T2);
SCOPES_RESULT(void) error_value_inaccessible_from_closure(TypedValue *value, const FunctionRef &frame);
SCOPES_RESULT(void) error_cannot_deref_non_plain(const Type *T);

// lifetime checker errors
SCOPES_RESULT(void) error_cannot_view_moved(const TypedValueRef &value);
SCOPES_RESULT(void) error_cannot_access_moved(const Type *T, const char *by);
SCOPES_RESULT(void) error_cannot_return_view(const TypedValueRef &value);
SCOPES_RESULT(void) error_value_not_unique(const TypedValueRef &value, const char *by);
SCOPES_RESULT(void) error_value_not_plain(const TypedValueRef &value);
SCOPES_RESULT(void) error_altering_parent_scope_in_pass(const Type *valuetype);
SCOPES_RESULT(void) error_altering_parent_scope_in_loop(const Type *valuetype);
SCOPES_RESULT(void) error_cannot_cast_plain_to_unique(const Type *SrcT, const Type *DestT);
SCOPES_RESULT(void) error_plain_not_storage_of_unique(const Type *SrcT, const Type *DestT);
SCOPES_RESULT(void) error_value_moved(const TypedValueRef &value, const ValueRef &mover, const char *by);
SCOPES_RESULT(void) error_value_in_use(const TypedValueRef &value, const ValueRef &user, const char *by);
SCOPES_RESULT(void) error_value_is_viewed(const ValueRef &value, const ValueRef &user, const char *by);
SCOPES_RESULT(void) error_cannot_merge_moves(const char *by);
SCOPES_RESULT(void) error_nonreturning_function_must_move();

// code generator errors
SCOPES_RESULT(void) error_gen_invalid_call_type(const char *target, const TypedValueRef &callee);
SCOPES_RESULT(void) error_gen_unbound_symbol(const char *target, const ParameterRef &value);
SCOPES_RESULT(void) error_gen_unbound_symbol(const char *target, const ValueRef &value);
SCOPES_RESULT(void) error_cannot_translate(const char *target, const ValueRef &value);

//------------------------------------------------------------------------------

} // namespace scopes

#endif // SCOPES_ERROR_HPP