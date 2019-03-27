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

Error::Error() :
    anchor(nullptr),
    msg(nullptr) {}

Error::Error(const Anchor *_anchor, const String *_msg) :
    anchor(_anchor),
    msg(_msg) {}

void Error::append_definition(const ValueRef &value) {
    definitions.push_back(value);
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

void collect_trace(StyledStream &ss,
    Values &values, ValueRef value) {
    values.push_back(value);
}

void stream_error(StyledStream &ss, const Error *exc) {
#if 0
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
#endif
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
    Values values;
    for (auto def : exc->definitions) {
        values.clear();
        collect_trace(ss, values, def);
        if (!values.size())
            continue;
        size_t i = values.size();
        const Anchor *last_anchor = nullptr;
        while (i--) {
            auto value = values[i];
            const Anchor *anchor = value.anchor();
            if (last_anchor && anchor->is_same(last_anchor))
                continue;
            last_anchor = anchor;
            ss << anchor << " in ";
            if (value.isa<Function>()) {
                auto fn = value.cast<Function>();
                if (fn->name == SYM_Unnamed) {
                    ss << Style_Function << "unnamed" << Style_None;
                } else {
                    ss << Style_Function << fn->name.name()->data << Style_None;
                }
            } else {
                ss << "expression";
            }
            ss << std::endl;
            anchor->stream_source_line(ss);
        }
    }
}

void print_error(const Error *value) {
    auto cerr = StyledStream(SCOPES_CERR);
    stream_error(cerr, value);
}

Error *make_location_error(const Anchor *anchor, const String *msg) {
    return new Error(anchor, msg);
}

Error *make_error(const String *msg) {
    return new Error(nullptr, msg);
}

//------------------------------------------------------------------------------

SCOPES_RESULT(void) error_invalid_call_type(const TypedValueRef &callee) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "unable to call value of type " << callee->get_type();
    SCOPES_DEF_ERROR(callee, ss.str());
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
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_argument_type_mismatch(const Type *expected, const Type *got) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "parameter is of type " << expected << ", but argument is of type " << got;
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_cannot_view_moved(const TypedValueRef &value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "cannot view value of type " << value->get_type() << " because it has been moved";
    SCOPES_ERROR(ss.str());
    //SCOPES_DEF_ERROR(mover, ss.str());
}

SCOPES_RESULT(void) error_cannot_access_moved(const Type *T, const char *by) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << by << " cannot access value of type " << T << " because it has been moved";
    SCOPES_ERROR(ss.str());
    //SCOPES_DEF_ERROR(mover, ss.str());
}

SCOPES_RESULT(void) error_cannot_cast_plain_to_unique(const Type *SrcT, const Type *DestT) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "cannot cast value of plain type " << SrcT << " to unique type " << DestT;
    SCOPES_ERROR(ss.str());
    //SCOPES_DEF_ERROR(mover, ss.str());
}

SCOPES_RESULT(void) error_plain_not_storage_of_unique(const Type *SrcT, const Type *DestT) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "type " << SrcT << " is not the storage type of unique type " << DestT;
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_value_not_unique(const TypedValueRef &value, const char *by) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << by << " value of type " << value->get_type() << " is not unique";
    if (!is_plain(value->get_type())) {
        ss.out << ", but type is unique" << std::endl;
    }
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_value_not_plain(const TypedValueRef &value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "value of type " << value->get_type() << " is not plain";
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_altering_parent_scope_in_pass(const Type *value_type) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "skippable switch pass moved value of type " << value_type << " which is from a parent scope";
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_altering_parent_scope_in_loop(const Type *value_type) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "loop moved value of type " << value_type << " which is from a parent scope";
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_cannot_return_view(const TypedValueRef &value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "cannot return view of value of type " << value->get_type() << " because the value is local to scope";
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_value_moved(const TypedValueRef &value, const ValueRef &mover, const char *by) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << by << " cannot move value of type " << value->get_type() << " out of function because it will be moved";
    SCOPES_DEF_ERROR(mover, ss.str());
}

SCOPES_RESULT(void) error_value_in_use(const TypedValueRef &value, const ValueRef &user, const char *by) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << by << " cannot move value of type " << value->get_type() << " out of function because it is still in use";
    SCOPES_DEF_ERROR(user, ss.str());
}

SCOPES_RESULT(void) error_value_is_viewed(const ValueRef &value, const ValueRef &user, const char *by) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << by << " cannot move view of unique value(s)";
    SCOPES_DEF_ERROR(user, ss.str());
}

SCOPES_RESULT(void) error_cannot_merge_moves(const char *by) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "conflicting attempt in " << by << " to move or view unique value(s)";
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_nonreturning_function_must_move() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "non-returning function must acquire all arguments";
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_invalid_operands(const Type *A, const Type *B) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "invalid operand types " << A << " and " << B;
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_invalid_condition_type(const TypedValueRef &cond) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "condition of if-clause must be value of type "
        << TYPE_Bool << ", not value of type " << cond->get_type();
    SCOPES_DEF_ERROR(cond, ss.str());
}

SCOPES_RESULT(void) error_invalid_case_literal_type(const TypedValueRef &lit) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "condition of switch-case must be constant of type "
        << TYPE_Integer << ", not value of type " << lit->get_type();
    SCOPES_DEF_ERROR(lit, ss.str());
}

SCOPES_RESULT(void) error_label_expected(const ValueRef &value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    if (value.isa<TypedValue>()) {
        ss.out << "expected label, not value of type " << value.cast<TypedValue>()->get_type();
    } else {
        ss.out << "expected label, not untyped value";
    }
    SCOPES_DEF_ERROR(value, ss.str());
}

SCOPES_RESULT(void) error_duplicate_default_case() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "duplicate default case";
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_missing_default_case() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "missing default case";
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_something_expected(const char *want, const ValueRef &value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << want << " expected, got "
        << get_value_class_name(value->kind());
    if (value.isa<TypedValue>()) {
        ss.out << " of type " << value.cast<TypedValue>()->get_type();
    }
    SCOPES_DEF_ERROR(value, ss.str());
}

SCOPES_RESULT(void) error_constant_expected(const Type *want, const ValueRef &value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "constant";
    if (want)
        ss.out << " of type " << want;
    ss.out << " expected, got "
        << get_value_class_name(value->kind());
    if (value.isa<TypedValue>()) {
        ss.out << " of type " << value.cast<TypedValue>()->get_type();
    }
    SCOPES_DEF_ERROR(value, ss.str());
}

SCOPES_RESULT(void) error_unbound_symbol(const ParameterRef &value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "symbol " << value->name << " is unbound in scope";
    SCOPES_DEF_ERROR(value, ss.str());
}

SCOPES_RESULT(void) error_unbound_symbol(const ValueRef &value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "token " << value << " is unbound in scope";
    SCOPES_DEF_ERROR(value, ss.str());
}

SCOPES_RESULT(void) error_cannot_merge_expression_types(const char *context, const Type *T1, const Type *T2) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "conflicting " << context << " types " << T1 << " and " << T2;
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_noreturn_not_last_expression() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "non-returning expression isn't last expression in sequence";
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_recursion_overflow() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;

    ss.out << "maximum number of compile time recursions exceeded (> ";
    ss.out << SCOPES_MAX_RECURSIONS;
    ss.out << ")";
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_noreturn_in_argument_list() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "non-returning expression in argument list";
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_cannot_type_builtin(const Builtin &builtin) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "can not type builtin " << builtin;
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_illegal_repeat_outside_loop() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "illegal repeat outside loop";
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_illegal_break_outside_loop() {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "illegal break outside loop";
    SCOPES_ERROR(ss.str());
}

SCOPES_RESULT(void) error_variadic_symbol_not_in_last_place(const ValueRef &value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "variadic symbol is not in last place";
    SCOPES_DEF_ERROR(value, ss.str());
}

SCOPES_RESULT(void) error_untyped_recursive_call(const FunctionRef &func) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "recursive call to function " << func->name
         << " can not be typed because the function has no return type yet";
    SCOPES_DEF_ERROR(func, ss.str());
}

SCOPES_RESULT(void) error_recursive_function_changed_type(const FunctionRef &func, const Type *T1, const Type *T2) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "recursive function " << func->name
         << " changed signature from " << T1 << " to " << T2 << " after first use";
    SCOPES_DEF_ERROR(func, ss.str());
}

SCOPES_RESULT(void) error_value_inaccessible_from_closure(const TypedValueRef &value,
    const FunctionRef &frame) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << "non-constant value of type " << value->get_type()
        << " is inaccessible from function";
    if (frame) {
        SCOPES_DEF_ERROR(frame, ss.str());
    } else {
        SCOPES_ERROR(ss.str());
    }
}

SCOPES_RESULT(void) error_cannot_deref_non_plain(const Type *T) {
    SCOPES_RESULT_TYPE(void)
    StyledString ss;
    ss.out << "can not implicitly dereference non-plain value of type " << T;
    SCOPES_ERROR(ss.str());
}

//------------------------------------------------------------------------------

SCOPES_RESULT(void) error_gen_invalid_call_type(const char *target, const TypedValueRef &callee) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << target << ": cannot translate call to value of type " << callee->get_type();
    SCOPES_DEF_ERROR(callee, ss.str());
}

SCOPES_RESULT(void) error_gen_unbound_symbol(const char *target, const ParameterRef &value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << target << ": symbol " << value->name << " is unbound";
    SCOPES_DEF_ERROR(value, ss.str());
}

SCOPES_RESULT(void) error_gen_unbound_symbol(const char *target, const ValueRef &value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    ss.out << target << ": IL token " << value << " is unbound";
    SCOPES_DEF_ERROR(value, ss.str());
}

SCOPES_RESULT(void) error_cannot_translate(const char *target, const ValueRef &value) {
    SCOPES_RESULT_TYPE(void);
    StyledString ss;
    if (value.isa<Instruction>()) {
        ss.out << target << ": instruction of kind "
            << Style_Keyword << get_value_class_name(value->kind()) << Style_None
            << " is referenced by argument but ";
        if (value.cast<Instruction>()->block) {
            ss.out << "its block has not been processed";
        } else {
            ss.out << "not associated with a block";
        }
    } else {
        ss.out << target << ": cannot translate value of kind "
            << Style_Keyword << get_value_class_name(value->kind()) << Style_None;
    }
    SCOPES_ERROR(ss.str());
}

//------------------------------------------------------------------------------

} // namespace scopes
