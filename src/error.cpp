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
#include "stream_ast.hpp"
#include "type/arguments_type.hpp"
#include "type/function_type.hpp"
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

void Error::append_definition(Value *value) {
    definitions.push_back(value);
}

void Error::append_error_trace(Value *value) {
    // only report deepest call
    if (!trace.empty()) {
        Value *last = trace.back();
        if (isa<Call>(last) && isa<Call>(value))
            return;
    }
    trace.push_back(value);
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
    const Anchor *last_anchor = nullptr;
    while (i--) {
        auto value = exc->trace[i];
        if (last_anchor && value->anchor()->is_same(last_anchor))
            continue;
        last_anchor = value->anchor();
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
    /*
    if (exc->anchor) {
        exc->anchor->stream_source_line(ss);
    }
    */
    for (auto def : exc->definitions) {
        ss << def->anchor() << " defined here" << std::endl;
        def->anchor()->stream_source_line(ss);
    }
}

void print_error(const Error *value) {
    auto cerr = StyledStream(SCOPES_CERR);
    stream_error(cerr, value);
}

Error *make_location_error(const String *msg) {
    return new Error(_active_anchor, msg);
}

Error *make_location_error(const Anchor *anchor, const String *msg) {
    return new Error(anchor, msg);
}

Error *make_runtime_error(const String *msg) {
    return new Error(nullptr, msg);
}

//------------------------------------------------------------------------------

#define SCOPES_LOCATION_DEF_ERROR(DEF, MSG) { \
    auto err = make_location_error((MSG)); \
    err->append_definition((DEF)); \
    return Result<_result_type>::raise(err); \
}

SCOPES_RESULT(void) error_invalid_call_type(TypedValue *callee) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "unable to call value of type " << callee->get_type();
    SCOPES_LOCATION_DEF_ERROR(callee, ss.str());
}

SCOPES_RESULT(void) error_argument_count_mismatch(int needed, int got, const FunctionType *ft) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    if (got > needed)
        ss.out << "too many";
    else
        ss.out << "not enough";
    ss.out << " arguments in call";
    if (ft) {
        ss.out << " to function of type " << (const Type *)ft;
    }
    ss.out << " (";
    if (ft && ft->vararg()) {
        ss.out << "at least ";
    }
    ss.out << needed << " argument(s) expected, got " << got << ")";
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_argument_type_mismatch(const Type *expected, const Type *got) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "parameter is of type " << expected << ", but argument is of type " << got;
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_value_moved(TypedValue *value, Value *mover, const char *by) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << by << " cannot move value of type " << value->get_type() << " out of function because it will be moved";
    SCOPES_LOCATION_DEF_ERROR(mover, ss.str());
}

SCOPES_RESULT(void) error_value_in_use(TypedValue *value, Value *user, const char *by) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << by << " cannot move value of type " << value->get_type() << " out of function because it is still in use";
    SCOPES_LOCATION_DEF_ERROR(user, ss.str());
}

SCOPES_RESULT(void) error_value_is_viewed(Value *value, Value *user, const char *by) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << by << " cannot move view of unique value(s)";
    SCOPES_LOCATION_DEF_ERROR(user, ss.str());
}

SCOPES_RESULT(void) error_cannot_merge_moves(const char *by) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "conflicting attempt in " << by << " to move or view unique value(s)";
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_nonreturning_function_must_move() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "non-returning function must acquire all arguments";
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_invalid_operands(const Type *A, const Type *B) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "invalid operand types " << A << " and " << B;
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_invalid_condition_type(TypedValue *cond) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "condition of if-clause must be value of type "
        << TYPE_Bool << ", not value of type " << cond->get_type();
    SCOPES_LOCATION_DEF_ERROR(cond, ss.str());
}

SCOPES_RESULT(void) error_invalid_case_literal_type(TypedValue *lit) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "condition of switch-case must be constant of type "
        << TYPE_Integer << ", not value of type " << lit->get_type();
    SCOPES_LOCATION_DEF_ERROR(lit, ss.str());
}

SCOPES_RESULT(void) error_label_expected(Value *value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    if (isa<TypedValue>(value)) {
        ss.out << "expected label, not value of type " << cast<TypedValue>(value)->get_type();
    } else {
        ss.out << "expected label, not untyped value";
    }
    SCOPES_LOCATION_DEF_ERROR(value, ss.str());
}

SCOPES_RESULT(void) error_duplicate_default_case() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "duplicate default case";
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_missing_default_case() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "missing default case";
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_constant_expected(const Type *want, Value *value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "constant";
    if (want)
        ss.out << " of type " << want;
    ss.out << " expected, got "
        << get_value_class_name(value->kind());
    if (isa<TypedValue>(value)) {
        ss.out << " of type " << cast<TypedValue>(value)->get_type();
    }
    SCOPES_LOCATION_DEF_ERROR(value, ss.str());
}

SCOPES_RESULT(void) error_unbound_symbol(Parameter *value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "symbol " << value->name << " is unbound in scope";
    SCOPES_LOCATION_DEF_ERROR(value, ss.str());
}

SCOPES_RESULT(void) error_unbound_symbol(Value *value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "token " << value << " is unbound in scope";
    SCOPES_LOCATION_DEF_ERROR(value, ss.str());
}

SCOPES_RESULT(void) error_cannot_merge_expression_types(const char *context, const Type *T1, const Type *T2) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "conflicting " << context << " types " << T1 << " and " << T2;
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_noreturn_not_last_expression() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "non-returning expression isn't last expression in sequence";
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_recursion_overflow() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;

    ss.out << "maximum number of compile time recursions exceeded (> ";
    ss.out << SCOPES_MAX_RECURSIONS;
    ss.out << ")";
    SCOPES_LOCATION_ERROR(ss.str());
}

SCOPES_RESULT(void) error_noreturn_in_argument_list() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "non-returning expression in argument list";
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

SCOPES_RESULT(void) error_untyped_recursive_call(Function *func) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "recursive call to function " << func->name
         << " can not be typed because the function has no return type yet";
    SCOPES_LOCATION_DEF_ERROR(func, ss.str());
}

SCOPES_RESULT(void) error_recursive_function_changed_type(Function *func, const Type *T1, const Type *T2) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "recursive function " << func->name
         << " changed signature from " << T1 << " to " << T2 << " after first use";
    SCOPES_LOCATION_DEF_ERROR(func, ss.str());
}

SCOPES_RESULT(void) error_value_inaccessible_from_closure(TypedValue *value,
    const Function *frame) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "non-constant value of type " << value->get_type()
        << " is inaccessible from function";
    if (frame) {
        SCOPES_LOCATION_DEF_ERROR(const_cast<Function *>(frame), ss.str());
    } else {
        SCOPES_LOCATION_ERROR(ss.str());
    }
}

SCOPES_RESULT(void) error_cannot_deref_non_plain(const Type *T) {
    SCOPES_RESULT_TYPE(void)
    StyledString ss;
    ss.out << "can not implicitly dereference non-plain value of type " << T;
    SCOPES_LOCATION_ERROR(ss.str());
}

//------------------------------------------------------------------------------

SCOPES_RESULT(void) error_gen_invalid_call_type(const char *target, TypedValue *callee) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << target << ": cannot translate call to value of type " << callee->get_type();
    SCOPES_LOCATION_DEF_ERROR(callee, ss.str());
}

SCOPES_RESULT(void) error_gen_unbound_symbol(const char *target, Parameter *value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << target << ": symbol " << value->name << " is unbound";
    SCOPES_LOCATION_DEF_ERROR(value, ss.str());
}

SCOPES_RESULT(void) error_gen_unbound_symbol(const char *target, Value *value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << target << ": IL token " << value << " is unbound";
    SCOPES_LOCATION_DEF_ERROR(value, ss.str());
}

SCOPES_RESULT(void) error_cannot_translate(const char *target, Value *value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    if (isa<Instruction>(value)) {
        ss.out << target << ": instruction of kind "
            << Style_Keyword << get_value_class_name(value->kind()) << Style_None
            << " is referenced by argument but ";
        if (cast<Instruction>(value)->block) {
            ss.out << "its block has not been processed";
        } else {
            ss.out << "not associated with a block";
        }
    } else {
        ss.out << target << ": cannot translate value of kind "
            << Style_Keyword << get_value_class_name(value->kind()) << Style_None;
    }
    SCOPES_LOCATION_ERROR(ss.str());
}

//------------------------------------------------------------------------------

} // namespace scopes
