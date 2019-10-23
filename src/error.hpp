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
#include "ppmacro.inc"

#include <vector>

namespace scopes {

//------------------------------------------------------------------------------

struct Type;
struct String;
struct Anchor;
struct StyledStream;

typedef const Anchor *PAnchor;
typedef const Type *PType;
typedef const String *PString;
typedef const char *Rawstring;
typedef const Scope *PScope;

struct ErrnoValue {
    int value;

    ErrnoValue() {}
    ErrnoValue(int _value) : value(_value) {}
};

/*
formatters:
%n      nth value
*/

#define SCOPES_C_IMPORT_ERROR_KIND() \
    T(CImportUnsupportedRecordType, \
        "c-import: can't translate record of unuspported type %1" \
        "%0 defined here", \
        PAnchor, Symbol) \
    T(CImportDuplicateTypeDefinition, \
        "c-import: duplicate body defined for type %1" \
        "%0 defined here", \
        PAnchor, PType) \
    T(CImportCannotConvertType, \
        "c-import: cannot convert type: %0 (%1)", \
        Rawstring, Rawstring) \
    T(CImportCompilationFailed, \
        "c-import: compilation failed") \

// parser errors
#define SCOPES_PARSER_ERROR_KIND() \
    T(ParserBadTaste, \
        "format: please use spaces instead of tabs") \
    T(ParserUnterminatedSequence, \
        "format: character sequence never ended") \
    T(ParserUnexpectedLineBreak, \
        "format: unexpected line break in character sequence") \
    T(ParserInvalidIntegerSuffix, \
        "format: invalid suffix for integer literal: '%0'", \
        PString) \
    T(ParserInvalidRealSuffix, \
        "format: invalid suffix for real literal: '%0'", \
        PString) \
    T(ParserUnclosedOpenBracket, \
        "format: parenthesis never closed" \
        "%0 opened here", \
        PAnchor) \
    T(ParserStrayClosingBracket, \
        "format: stray closing bracket") \
    T(ParserUnterminatedQuote, \
        "format: quote character quotes nothing") \
    T(ParserUnexpectedToken, \
        "format: unexpected token '%0' (%1)", \
        char, int) \
    T(ParserStrayEscapeToken, \
        "format: list continuation character must be at beginning or end of sublist line") \
    T(ParserIndentationMismatch, \
        "format: indentation mismatch") \
    T(ParserBadIndentationLevel, \
        "format: indentations must nest by 4 spaces") \
    T(ParserStrayStatementToken, \
        "format: unexpected list separation character") \


// expander errors
#define SCOPES_SYNTAX_ERROR_KIND() \
    T(SyntaxCallExpressionEmpty, \
        "syntax: call expression is empty") \
    T(SyntaxTooManyArguments, \
        "syntax: excess argument. At most %0 arguments expected", \
        int) \
    T(SyntaxNotEnoughArguments, \
        "syntax: at least %0 arguments expected, got %1", \
        int, int) \
    T(SyntaxUnnamedForwardDeclaration, \
        "syntax: forward declared function must be named") \
    T(SyntaxVariadicSymbolNotLast, \
        "syntax: variadic symbol is not in last place") \
    T(SyntaxAssignmentTokenExpected, \
        "syntax: assignment token (=) expected") \
    T(SyntaxKeyedArgumentMismatch, \
        "syntax: keyed call argument must be singular") \
    T(SyntaxUnexpectedExtraToken, \
        "syntax: unexpected extra token") \
    T(SyntaxUndeclaredIdentifier, \
        syntax_undeclared_identifier_print_suggestions, \
        Symbol, PScope) \
    T(SyntaxExceptBlockExpected, \
        "syntax: except block expected") \
    T(SyntaxMissingDefaultCase, \
        "syntax: missing default case") \
    T(SyntaxCaseBlockExpected, \
        "syntax: case, pass, do or default block expected") \
    T(SyntaxLabelExpected, \
        "syntax: label template expected, got %0", \
        ValueKind) \
    T(SyntaxSymbolExpanderTypeMismatch, \
        "syntax: symbol expander has wrong type %0, must be constant of type %1", \
        PType, PType) \
    T(SyntaxListExpanderTypeMismatch, \
        "syntax: list expander has wrong type %0, must be constant of type %1", \
        PType, PType) \
    T(SyntaxExcessBindingArgument, \
        "syntax: excess argument is not bound to a name") \

// typechecking
#define SCOPES_TYPECHECK_ERROR_KIND() \
    T(TypeKindMismatch, \
        "type of %0 kind expected, got %1", \
        TypeKind, PType) \
    T(ValueKindMismatch, \
        "value of %0 kind expected, got %1", \
        ValueKind, ValueKind) \
    T(CannotCreateConstantOf, \
        "cannot create constant of type %0", \
        PType) \
    T(ConstantExpected, \
        "constant expected, got %1", \
        ValueKind) \
    T(ConstantValueKindMismatch, \
        "constant value of type %0 expected, got %1", \
        ValueKind, ValueKind) \
    T(TypedConstantValueKindMismatch, \
        "constant value of type %0 expected, got %1", \
        PType, ValueKind) \
    T(TooManyArguments, \
        "at most %0 argument(s) expected, got %1", \
        int, int) \
    T(NotEnoughArguments, \
        "at least %0 argument(s) expected, got %1", \
        int, int) \
    T(CannotTypeInline, \
        "inline can not be instantiated outside functions") \
    T(ValueMustBeReference, \
        "value of type %0 must be reference", \
        PType) \
    T(NonReadableReference, \
        "can not dereference value of type %0 because the reference is non-readable", \
        PType) \
    T(NonWritableReference, \
        "can not dereference value of type %0 because the reference is non-writable", \
        PType) \
    T(NonReadablePointer, \
        "can not load value of type %0 because the pointer type is non-readable", \
        PType) \
    T(NonWritablePointer, \
        "can not store to value of type %0 because the pointer type is non-writable", \
        PType) \
    T(MergeConflict, \
        "new %0 result type %2 conflicts with previous type %1" \
        "%4 conflicting here" \
        "%3 previously defined here", \
        Rawstring, PType, PType, PAnchor, PAnchor) \
    T(InaccessibleValue, \
        "cannot access value of type %0 because it has been moved" \
        "%1 lifetime ended here", \
        PType, PAnchor) \
    T(DropReturnsArguments, \
        "drop operation must not return any arguments") \
    T(SwitchPassMovedValue, \
        "skippable switch pass moved value of type %0 out of parent scope", \
        PType) \
    T(LoopMovedValue, \
        "loop moved value of type %0 out of parent scope", \
        PType) \
    T(ViewExitingScope, \
        "view of value of type %0 can not be moved out of its scope", \
        PType) \
    T(DuplicateParameterKey, \
        "duplicate parameter key '%0'", \
        Symbol) \
    T(UnknownParameterKey, \
        unknown_parameter_key_print_suggestions, \
        Symbol, Symbols) \
    T(BreakOutsideLoop, \
        "break can only be used within the scope of a loop") \
    T(RepeatOutsideLoop, \
        "repeat can only be used within the scope of a loop") \
    T(LabelExpected, \
        "label expected, got value of type %0", \
        PType) \
    T(RecursiveFunctionChangedType, \
        "recursive function must not change type signature after first use (changed from %0 to %1)", \
        PType, PType) \
    T(UnknownTupleField, \
        missing_tuple_field_print_suggestions, \
        Symbol, PType) \
    T(PlainToUniqueCast, \
        "cannot cast value of plain type %0 to unique type %1", \
        PType, PType) \
    T(UniqueValueExpected, \
        "value of type %0 must be unique", \
        PType) \
    T(IncompatibleStorageTypeForUnique, \
        "type %0 is not the storage type of unique type %1", \
        PType, PType) \
    T(SpiceMacroReturnedNull, \
        "spice macro returned null") \
    T(UnsupportedDimensionality, \
        "unsupported dimensionality: %0", \
        Symbol) \
    T(UnsupportedExecutionMode, \
        "unsupported execution mode: %0", \
        Symbol) \
    T(CastCategoryError, \
        "cannot cast value of type %0 to %1 because both types must be of same storage category", \
        PType, PType) \
    T(CastIncompatibleAggregateType, \
        "cannot cast value of aggregate storage type %0", \
        PType) \
    T(InvalidOperands, \
        "invalid operand types %1 and %2 for builtin %0", \
        Builtin, PType, PType) \
    T(InvalidArgumentTypeForBuiltin, \
        "invalid argument type %1 for builtin %0", \
        Builtin, PType) \
    T(UnsupportedBuiltin, \
        "builtin %0 is unimplemented or deprecated", \
        Builtin) \
    T(InvalidCallee, \
        "cannot call value of type %0", \
        PType) \
    T(TooManyFunctionArguments, \
        "too many arguments (%1) in call to function of type %0", \
        PType, int) \
    T(NotEnoughFunctionArguments, \
        "not enough arguments (%1) in call to function of type %0", \
        PType, int) \
    T(DuplicateSwitchDefaultCase, \
        "switch expression must only have one default case") \
    T(MissingDefaultCase, \
        "missing default case") \
    T(UnclosedPass, \
        "pass block must be followed by pass, do or default block") \
    T(DoWithoutPass, \
        "do block must follow pass block") \
    T(VariadicParameterNotLast, \
        "variadic function parameter is not in last place") \
    T(RecursionOverflow, \
        "exceeded maximum number of compile time recursions (%0)", \
        int) \
    T(ResultMustBePure, \
        "result must be pure") \
    T(GlobalInitializerMustBePure, \
        "global initializer must be pure") \
    T(ParameterTypeMismatch, \
        "parameter is of type %0, but argument is of incompatible type %1", \
        PType, PType) \
    T(FunctionPointerExpected, \
        "function pointer expected, but argument is of incompatible type %0", \
        PType) \
    T(ScalarOrVectorExpected, \
        "scalar or vector of type %0 expected, but argument is of incompatible type %1", \
        PType, PType) \
    T(FixedVectorSizeMismatch, \
        "vector of size %0 expected, but argument is of incompatible type %1", \
        int, PType) \
    T(VectorSizeMismatch, \
        "arguments of type %0 and %1 must be of scalar type or vector type of equal size", \
        PType, PType) \
    T(ConditionNotBool, \
        "branching condition must be of boolean type, but is of type %0", \
        PType) \
    T(UnexpectedValueKind, \
        "unexpected %0", \
        ValueKind) \
    T(PrematureReturnFromExpression, \
        "non-returning expression causes expression block to end before completion") \
    T(OpaqueType, \
        "opaque type %0 is non-aggregable, has no size, alignment or storage type, and can not be passed as argument", \
        PType) \
    T(UntrackedType, \
        "non-plain type %0 has no unique or view qualifier", \
        PType) \
    T(MovableTypeMismatch, \
        "arguments of type %0 and %1 must both be movable", \
        PType, PType) \
    T(DupeUniqueStorage, \
        "cannot dupe because storage type %0 is unique", \
        PType) \
    T(IndexOutOfRange, \
        "index %0 is out of range (%1)", \
        int, int) \
    T(TypenameComplete, \
        "typename %0 is already complete", \
        PType) \
    T(TypenameIncomplete, \
        "attempting to use incomplete typename %0", \
        PType) \
    T(StorageTypeExpected, \
        "storage type expected, not typename %0", \
        PType) \
    T(PlainStorageTypeExpected, \
        "plain storage type expected, not %0", \
        PType) \
    T(VariableOutOfScope, \
        "value of type %0 is only known at run-time and outside of function scope" \
        "%1: defined here", \
        PType, PAnchor) \
    T(UnboundValue, \
        "value %0 is unbound", \
        ValueRef) \
    T(CannotProveForwardDeclaration, \
        "cannot instantiate forward declared function") \

// quoting
#define SCOPES_QUOTE_ERROR_KIND() \
    T(QuoteUnsupportedValueKind, \
        "quote: cannot quote %0", \
        ValueKind) \
    T(QuoteUnboundValue, \
        "quote: unbound value %0", \
        ValueRef) \

// code generation
#define SCOPES_CODEGEN_ERROR_KIND() \
    T(CGenTypeUnsupportedInTarget, \
        "codegen: type %0 is unsupported for this target", \
        PType) \
    T(CGenFailedToTranslateType, \
        "codegen: failed to translate type %0", \
        PType) \
    T(CGenUnboundValue, \
        "codegen: value %0 is unbound", \
        ValueRef) \
    T(CGenUnsupportedBuiltin, \
        "codegen: builtin %0 is unsupported for this target", \
        Builtin) \
    T(CGenUnsupportedArrayAlloc, \
        "codegen: array allocations are unsupported for this target") \
    T(CGenUnsupportedMalloc, \
        "codegen: heap allocations are unsupported for this target") \
    T(CGenUnsupportedUnOp, \
        "codegen: unary operator unsupported for this target") \
    T(CGenUnsupportedBinOp, \
        "codegen: binary operator unsupported for this target") \
    T(CGenUnsupportedTriOp, \
        "codegen: ternary operator unsupported for this target") \
    T(CGenUnsupportedImageOp, \
        "codegen: image operator unsupported for this target") \
    T(CGenUnsupportedCastOp, \
        "codegen: cast operator unsupported for this target") \
    T(CGenUnsupportedAtomicOp, \
        "codegen: atomic operator unsupported for this target") \
    T(CGenUnsupportedTarget, \
        "codegen: unsupported target: %0", /* todo: list supported targets */ \
        Symbol) \
    T(CGenInvalidCallee, \
        "codegen: cannot translate call to value of type %0", \
        PType) \
    T(CGenFailedToTranslateValue, \
        "codegen: failed to translate value of %0 kind", \
        ValueKind) \
    T(CGenFailedToResolveExtern, \
        "codegen: failed to resolve %0", \
        GlobalRef) \
    T(CGenBackendFailed, \
        "codegen backend failed: %0", \
        Rawstring) \
    T(CGenBackendFailedErrno, \
        "codegen backend failed: %0 (%1)", \
        Rawstring, ErrnoValue) \
    T(CGenCannotSerializeMemory, \
        "codegen: unable to serialize memory for value of type %0", \
        PType) \
    T(CGenBackendValidationFailed, \
        "codegen: backend failed to validate generated code") \
    T(CGenBackendOptimizationFailed, \
        "codegen: backend failed to optimize generated code") \
    T(CGenUnsupportedDimensionality, \
        "codegen: unsupported dimensionality: %0", \
        Symbol) \
    T(CGenUnsupportedImageFormat, \
        "codegen: unsupported image format: %0", \
        Symbol) \
    T(CGenUnsupportedExecutionMode, \
        "codegen: unsupported execution mode: %0", \
        Symbol) \
    T(CGenUnsupportedPointerStorageClass, \
        "codegen: unsupported pointer storage class: %0", \
        Symbol) \
    T(CGenUnsupportedIntrinsic, \
        "codegen: unspported intrinsic function: '%0'", \
        Symbol) \
    T(CGenEntryFunctionSignatureMismatch, \
        "codegen: entry function must have type %0 but has type %1", \
        PType, PType) \
    T(CGenUnsupportedVectorSize, \
        "codegen: a vector of type %0 and size %1 is unsupported", \
        PType, int) \

// runtime
#define SCOPES_RUNTIME_ERROR_KIND() \
    T(RTLoadLibraryFailed, \
        "runtime: error loading library %0: %1", \
        PString, Rawstring) \
    T(RTGetAddressFailed, \
        "runtime: could not find symbol '%0' in C namespace", \
        Symbol) \
    T(RTMissingKey, \
        "runtime: no such key in map") \
    T(RTMissingScopeAnyAttribute, \
        "runtime: no attribute %0 in scope", \
        ValueRef) \
    T(RTMissingLocalScopeAnyAttribute, \
        "runtime: no attribute %0 in local scope", \
        ValueRef) \
    T(RTMissingScopeAttribute, \
        rt_missing_scope_attribute_print_suggestions, \
        Symbol, PScope) \
    T(RTMissingLocalScopeAttribute, \
        "runtime: no attribute %0 in local scope", \
        Symbol) \
    T(RTMissingTypeAttribute, \
        rt_missing_type_attribute_print_suggestions, \
        Symbol, PType) \
    T(RTMissingLocalTypeAttribute, \
        "runtime: no attribute %0 in local type", \
        Symbol) \
    T(RTMissingTupleAttribute, \
        rt_missing_tuple_field_print_suggestions, \
        Symbol, PType) \
    T(RTRegExError, \
        "runtime: error in regular expression: %0", \
        PString) \
    T(RTUnableToOpenFile, \
        "runtime: can't open file: %0", \
        PString) \
    T(RTUncountableStorageType, \
        "runtime: storage type %0 has no count", \
        PType) \
    T(RTNoElementsInStorageType, \
        "runtime: storage type %0 has no elements", \
        PType) \
    T(RTNoNamedElementsInStorageType, \
        "runtime: storage type %0 has no named elements", \
        PType) \
    T(RTIllegalSupertype, \
        "runtime: typename %0 can not be a supertype because it is not opaque", \
        PType) \

// main
#define SCOPES_MAIN_ERROR_KIND() \
    T(MainInaccessibleBinary, \
        "main: can't open executable file for reading") \
    T(InvalidFooter, \
        "main: invalid footer. Footer must have the format (core-size <size of script in bytes>)") \
    T(CoreModuleFunctionTypeMismatch, \
        "main: core function has wrong type %0, must be of type %1", \
        PType, PType) \
    T(CoreMissing, \
        "main: no core module at tail of executable or at path %0", \
        Symbol) \

#define SCOPES_ERROR_KIND() \
    T(User, "%0", PString) \
    T(ExecutionEngineFailed, \
        "execution engine failed: %0", \
        Rawstring) \
    T(StackOverflow, \
        "stack overflow encountered") \
    SCOPES_C_IMPORT_ERROR_KIND() \
    SCOPES_SYNTAX_ERROR_KIND() \
    SCOPES_TYPECHECK_ERROR_KIND() \
    SCOPES_CODEGEN_ERROR_KIND() \
    SCOPES_RUNTIME_ERROR_KIND() \
    SCOPES_PARSER_ERROR_KIND() \
    SCOPES_QUOTE_ERROR_KIND() \
    SCOPES_MAIN_ERROR_KIND() \

enum ErrorKind {
#define T(CLASS, STR, ...) \
    EK_ ## CLASS,
    SCOPES_ERROR_KIND()
#undef T
};

//------------------------------------------------------------------------------

// what were we doing when the error occurred?
enum BacktraceKind {
    // signifies nothing
    BTK_Dummy,
    // context = none, location in anchor
    BTK_Parser,
    // context = symbol or expression being expanded
    BTK_Expander,
    // context = function used as hook
    BTK_InvokeHook,
    // context = expression being typechecked
    BTK_ProveExpression,
    // context = template being typechecked
    BTK_ProveTemplate,
    // context = argument being typechecked
    BTK_ProveArgument,
    // context = target whose parameters are being mapped
    BTK_ProveParamMap,
    // context = argument being lifetime checked
    BTK_ProveArgumentLifetime,
    // context = value being translated
    BTK_Translate,
    // context = foreign type as string
    BTK_ConvertForeignType,
    // invoked in user code at runtime; context = call
    BTK_User,
};

struct Backtrace {
    const Backtrace *next;
    BacktraceKind kind;
    ValueRef context;
};

#define SCOPES_TRACE(KIND, VALUE) \
    Backtrace _backtrace = { nullptr, BTK_ ## KIND, (VALUE) };
// globals must be included for g_none
#define SCOPES_TRACE_PARSER(ANCHOR) \
    SCOPES_TRACE(Parser, ref((ANCHOR), g_none))
#define SCOPES_TRACE_EXPANDER(VALUEREF) \
    SCOPES_TRACE(Expander, (VALUEREF))
#define SCOPES_TRACE_HOOK(VALUEREF) \
    SCOPES_TRACE(InvokeHook, (VALUEREF))
#define SCOPES_TRACE_CODEGEN(VALUEREF) \
    SCOPES_TRACE(Translate, (VALUEREF))
#define SCOPES_TRACE_CONVERT_FOREIGN_TYPE(VALUEREF) \
    SCOPES_TRACE(ConvertForeignType, (VALUEREF))
#define SCOPES_TRACE_PROVE_EXPR(VALUEREF) \
    SCOPES_TRACE(ProveExpression, (VALUEREF))
#define SCOPES_TRACE_PROVE_TEMPLATE(VALUEREF) \
    SCOPES_TRACE(ProveTemplate, (VALUEREF))
#define SCOPES_TRACE_PROVE_ARG(VALUEREF) \
    SCOPES_TRACE(ProveArgument, (VALUEREF))
#define SCOPES_TRACE_PROVE_PARAM_MAP(VALUEREF) \
    SCOPES_TRACE(ProveParamMap, (VALUEREF))
#define SCOPES_TRACE_PROVE_ARG_LIFETIME(VALUEREF) \
    SCOPES_TRACE(ProveArgumentLifetime, (VALUEREF))

extern Backtrace _backtrace;

//------------------------------------------------------------------------------

struct Error {
    ErrorKind kind() const;

    Error(ErrorKind _kind);
    Error(const Error &other) = delete;

    Error *trace(const Backtrace &bt);
    const Backtrace *get_trace() const;
private:
    const Backtrace *_trace = nullptr;
    const ErrorKind _kind;
};

//------------------------------------------------------------------------------

#define TA(N, CLS) CLS
#define TB(N, CLS) CLS arg ## N
#define T(CLASS, STR, ...) /*
*/struct Error ## CLASS : Error { /*
*/  static bool classof(const Error *T);/*
*/  Error ## CLASS();/*
*/  static Error ## CLASS *from(SCOPES_FOREACH_EXPR(TA, ##__VA_ARGS__)); /*
*/  void stream(StyledStream &ss) const;/*
*/  SCOPES_FOREACH_STMT(TB, ##__VA_ARGS__); /*
*/};

SCOPES_ERROR_KIND()
#undef T
#undef TA
#undef TB

//------------------------------------------------------------------------------

void print_error(const Error *value);
void stream_error_message(StyledStream &ss, const Error *value);
void stream_error(StyledStream &ss, const Error *value);

#define SCOPES_RETURN_TRACE_ERROR(ERR) \
    SCOPES_RETURN_ERROR((ERR)->trace(_backtrace))

#if SCOPES_EARLY_ABORT
#define SCOPES_ERROR(CLASS, ...) \
    assert(false); \
    SCOPES_RETURN_TRACE_ERROR(Error ## CLASS::from(__VA_ARGS__));
#else
#define SCOPES_ERROR(CLASS, ...) \
    SCOPES_RETURN_TRACE_ERROR(Error ## CLASS::from(__VA_ARGS__));
#endif

// if ok fails, return
#define SCOPES_CHECK_OK(OK, ERR) if (!OK) { SCOPES_RETURN_TRACE_ERROR(ERR); }
// if an expression returning a result fails, return
#define SCOPES_CHECK_RESULT(EXPR) { \
    auto _result = (EXPR); \
    SCOPES_CHECK_OK(_result.ok(), _result.unsafe_error()); \
}
// try to extract a value from a result or return
#define SCOPES_GET_RESULT(EXPR) ({ \
        auto _result = (EXPR); \
        SCOPES_CHECK_OK(_result.ok(), _result.unsafe_error()); \
        _result.unsafe_extract(); \
    })
#define SCOPES_CHECK_CAST(T, EXPR) ({ \
        Result<T> _result = (EXPR); \
        SCOPES_CHECK_OK(_result.ok(), _result.unsafe_error()); \
        _result.unsafe_extract(); \
    })

} // namespace scopes

#endif // SCOPES_ERROR_HPP