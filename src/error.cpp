/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "error.hpp"
#include "anchor.hpp"
#include "type.hpp"
#include "boot.hpp"
#include "ast.hpp"
#include "return.hpp"

#include "scopes/config.h"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------
// ERROR HANDLING
//------------------------------------------------------------------------------

static const Anchor *_active_anchor = nullptr;

void set_active_anchor(const Anchor *anchor) {
    assert(anchor);
    _active_anchor = anchor;
}

const Anchor *get_active_anchor() {
    return _active_anchor;
}

//------------------------------------------------------------------------------

Error::Error() :
    anchor(nullptr),
    msg(nullptr) {}

Error::Error(const Anchor *_anchor, const String *_msg) :
    anchor(_anchor),
    msg(_msg) {}

//------------------------------------------------------------------------------

static Any _last_error = none;

void set_last_error(const Any &err) {
    _last_error = err;
}

Any get_last_error() {
    Any result = _last_error;
    _last_error = none;
    return result;
}

//------------------------------------------------------------------------------

void location_message(const Anchor *anchor, const String* str) {
    assert(anchor);
    auto cerr = StyledStream(SCOPES_CERR);
    cerr << anchor << str->data << std::endl;
    anchor->stream_source_line(cerr);
}

void stream_error_string(StyledStream &ss, const Any &value) {
    if (value.type == TYPE_Error) {
        const Error *exc = value;
        ss << exc->msg->data;
    } else {
        ss << "exception raised: " << value;
    }
}

void stream_error(StyledStream &ss, const Any &value) {
    if (value.type == TYPE_Error) {
        const Error *exc = value;
        if (exc->anchor) {
            ss << exc->anchor << " ";
        }
        ss << Style_Error << "error:" << Style_None << " "
            << exc->msg->data << std::endl;
        if (exc->anchor) {
            exc->anchor->stream_source_line(ss);
        }
    } else {
        ss << "exception raised: " << value << std::endl;
    }
}

void print_error(const Any &value) {
    auto cerr = StyledStream(SCOPES_CERR);
    stream_error(cerr, value);
}

Any make_location_error(const String *msg) {
    const Error *exc = new Error(_active_anchor, msg);
    return exc;
}

Any make_runtime_error(const String *msg) {
    const Error *exc = new Error(nullptr, msg);
    return exc;
}

void set_last_location_error(const String *msg) {
    set_last_error(make_location_error(msg));
}

//------------------------------------------------------------------------------

static void print_definition_anchor(ASTNode *node) {
    location_message(node->anchor(), String::from("defined here"));
}

SCOPES_RESULT(void) error_invalid_call_type(ASTNode *callee) {
    SCOPES_RESULT_TYPE(void);
    print_definition_anchor(callee);
    StyledString ss;
    ss.out << "unable to call value of type " << callee->get_type();
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_invalid_condition_type(ASTNode *cond) {
    SCOPES_RESULT_TYPE(void);
    print_definition_anchor(cond);
    StyledString ss;
    ss.out << "condition of if-clause must be value of type "
        << TYPE_Bool << ", not value of type " << cond->get_type();
    SCOPES_LOCATION_ERROR(ss.str());
}


SCOPES_RESULT(void) error_constant_expected(ASTNode *value) {
    SCOPES_RESULT_TYPE(void);
    print_definition_anchor(value);
    StyledString ss;
    ss.out << "constant expected, got expression of type " << value->get_type();
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_unbound_symbol(ASTSymbol *value) {
    SCOPES_RESULT_TYPE(void);
    print_definition_anchor(value);
    StyledString ss;
    ss.out << "symbol " << value->name << " is unbound";
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_cannot_merge_expression_types(const Type *T1, const Type *T2) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "cannot merge expression types " << T1 << " and " << T2;
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_noreturn_not_last_expression() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "non-returning expression isn't last expression in sequence";
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_cannot_type_builtin(const Builtin &builtin) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "can not type builtin " << builtin;
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_illegal_repeat_outside_loop() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "illegal repeat outside loop";
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_illegal_break_outside_loop() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "illegal break outside loop";
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_variadic_symbol_not_in_last_place() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "variadic symbol is not in last place";
    SCOPES_LOCATION_ERROR(ss.str());
}

//------------------------------------------------------------------------------

SCOPES_RESULT(void) error_gen_invalid_call_type(const char *target, ASTNode *callee) {
    SCOPES_RESULT_TYPE(void);
    print_definition_anchor(callee);
    StyledString ss;
    ss.out << target << ": cannot translate call to value of type " << callee->get_type();
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_gen_unbound_symbol(const char *target, ASTSymbol *value) {
    SCOPES_RESULT_TYPE(void);
    print_definition_anchor(value);
    StyledString ss;
    ss.out << target << ": symbol " << value->name << " is unbound";
    SCOPES_LOCATION_ERROR(ss.str());
}

//------------------------------------------------------------------------------

} // namespace scopes
