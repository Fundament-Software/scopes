/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "error.hpp"
#include "anchor.hpp"
#include "type.hpp"
#include "boot.hpp"
#include "value.hpp"
#include "arguments_type.hpp"
#include "dyn_cast.inc"

#include "scopes/config.h"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------
// ERROR HANDLING
//------------------------------------------------------------------------------

static const Anchor *_active_anchor = nullptr;

void _set_active_anchor(const Anchor *anchor) {
    assert(anchor);
    _active_anchor = anchor;
}

const Anchor *get_active_anchor() {
    return _active_anchor;
}

//------------------------------------------------------------------------------

ScopedAnchor::ScopedAnchor(const Anchor *anchor) : parent_anchor(get_active_anchor()) {
    _set_active_anchor(anchor);
}
ScopedAnchor::~ScopedAnchor() {
    _set_active_anchor(parent_anchor);
}

//------------------------------------------------------------------------------

Error::Error() :
    anchor(nullptr),
    msg(nullptr) {}

Error::Error(const Anchor *_anchor, const String *_msg) :
    anchor(_anchor),
    msg(_msg) {}

//------------------------------------------------------------------------------

static Error *_last_error = nullptr;

void set_last_error(Error *err) {
    _last_error = err;
}

Error *get_last_error() {
    auto result = _last_error;
    _last_error = nullptr;
    return result;
}

void add_error_trace(Value *value) {
    if (_last_error) {
        // only report deepest call
        if (!_last_error->trace.empty()) {
            Value *last = _last_error->trace.back();
            if (isa<Call>(last) && isa<Call>(value))
                return;
        }
        _last_error->trace.push_back(value);
    }
}

//------------------------------------------------------------------------------

void location_message(const Anchor *anchor, const String* str) {
    assert(anchor);
    auto cerr = StyledStream(SCOPES_CERR);
    cerr << anchor << str->data << std::endl;
    anchor->stream_source_line(cerr);
}

void stream_error_string(StyledStream &ss, const Error *exc) {
    ss << exc->msg->data;
}

void stream_error(StyledStream &ss, const Error *exc) {
    size_t i = exc->trace.size();
    while (i--) {
        auto value = exc->trace[i];
        ss << value->anchor() << " in ";
        if (isa<Function>(value)) {
            auto fn = cast<Function>(value);
            if (fn->name == SYM_Unnamed) {
                ss << Style_Function << "unnamed" << Style_None;
            } else {
                ss << Style_Function << fn->name.name()->data << Style_None;
            }
        } else {
            ss << "expression";
        }
        ss << std::endl;
        value->anchor()->stream_source_line(ss);
    }
    if (exc->anchor) {
        ss << exc->anchor << " ";
    }
    ss << Style_Error << "error:" << Style_None << " "
        << exc->msg->data << std::endl;
    if (exc->anchor) {
        exc->anchor->stream_source_line(ss);
    }
}

void print_error(const Error *value) {
    auto cerr = StyledStream(SCOPES_CERR);
    stream_error(cerr, value);
}

Error *make_location_error(const String *msg) {
    return new Error(_active_anchor, msg);
}

Error *make_runtime_error(const String *msg) {
    return new Error(nullptr, msg);
}

void set_last_location_error(const String *msg) {
    set_last_error(make_location_error(msg));
}

//------------------------------------------------------------------------------

static void print_definition_anchor(Value *node) {
    location_message(node->anchor(), String::from("defined here"));
}

SCOPES_RESULT(void) error_invalid_call_type(Value *callee) {
    SCOPES_RESULT_TYPE(void);
    print_definition_anchor(callee);
    StyledString ss;
    ss.out << "unable to call value of type " << callee->get_type();
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_argument_count_mismatch(int needed, int got) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    if (got > needed)
        ss.out << "too many";
    else
        ss.out << "not enough";
    ss.out << " arguments in call (" << needed << " argument(s) expected, got " << got << ")";
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_argument_type_mismatch(const Type *expected, const Type *got) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "parameter of type " << expected << " expected, but argument is of type " << got;
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_invalid_operands(const Type *A, const Type *B) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "invalid operand types " << A << " and " << B;
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_invalid_condition_type(Value *cond) {
    SCOPES_RESULT_TYPE(void);
    print_definition_anchor(cond);
    StyledString ss;
    ss.out << "condition of if-clause must be value of type "
        << TYPE_Bool << ", not value of type " << cond->get_type();
    SCOPES_LOCATION_ERROR(ss.str());
}


SCOPES_RESULT(void) error_constant_expected(Value *value) {
    SCOPES_RESULT_TYPE(void);
    print_definition_anchor(value);
    StyledString ss;
    ss.out << "constant expected, got "
        << get_value_class_name(value->kind())
        << " of type " << value->get_type();
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_unbound_symbol(SymbolValue *value) {
    SCOPES_RESULT_TYPE(void);
    print_definition_anchor(value);
    StyledString ss;
    ss.out << "symbol " << value->name << " is unbound in scope";
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

SCOPES_RESULT(void) error_illegal_return_in_inline() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "illegal return inside inline";
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_untyped_recursive_call() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "recursive call to function can not be typed because the function has no return type yet";
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_cannot_find_frame(Template *func) {
    SCOPES_RESULT_TYPE(void);
    print_definition_anchor(func->scope);
    StyledString ss;
    ss.out << "couldn't find frame for scope of function";
    SCOPES_LOCATION_ERROR(ss.str());
}

//------------------------------------------------------------------------------

SCOPES_RESULT(void) error_gen_invalid_call_type(const char *target, Value *callee) {
    SCOPES_RESULT_TYPE(void);
    print_definition_anchor(callee);
    StyledString ss;
    ss.out << target << ": cannot translate call to value of type " << callee->get_type();
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_gen_unbound_symbol(const char *target, SymbolValue *value) {
    SCOPES_RESULT_TYPE(void);
    print_definition_anchor(value);
    StyledString ss;
    ss.out << target << ": symbol " << value->name << " is unbound";
    SCOPES_LOCATION_ERROR(ss.str());
}

//------------------------------------------------------------------------------

} // namespace scopes
