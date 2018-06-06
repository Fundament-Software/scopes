/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_H
#define SCOPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined(SCOPESRT_IMPL) && defined(__cplusplus)
#include "../src/any.hpp"
#endif

#if defined __cplusplus
extern "C" {
#endif

#define SCOPES_VERSION_MAJOR 0
#define SCOPES_VERSION_MINOR 14
#define SCOPES_VERSION_PATCH 0

// trace partial evaluation and code generation
// produces a firehose of information
#define SCOPES_DEBUG_CODEGEN 0

// run LLVM optimization passes
// turning this on is detrimental to startup time
// scopes output is typically clean enough to provide fairly good performance
// on its own.
#define SCOPES_OPTIMIZE_ASSEMBLY 0

// any exception aborts immediately and can not be caught
#define SCOPES_EARLY_ABORT 0

// print a list of cumulative timers on program exit
#define SCOPES_PRINT_TIMERS 0

// maximum number of recursions permitted during partial evaluation
// if you think you need more, ask yourself if ad-hoc compiling a pure C function
// that you can then use at compile time isn't the better choice;
// 100% of the time, the answer is yes because the performance is much better.
#define SCOPES_MAX_RECURSIONS 32

// maximum number of jump skips permitted
#define SCOPES_MAX_SKIP_JUMPS 256

// compile native code with debug info if not otherwise specified
#define SCOPES_COMPILE_WITH_DEBUG_INFO 1

#ifndef SCOPES_WIN32
#   ifdef _WIN32
#   define SCOPES_WIN32
#   endif
#endif

// maximum size of process stack
#ifdef SCOPES_WIN32
// on windows, we only get 1 MB of stack
// #define SCOPES_MAX_STACK_SIZE ((1 << 10) * 768)
// but we build with "-Wl,--stack,8388608"
#define SCOPES_MAX_STACK_SIZE ((1 << 20) * 7)
#else
// on linux, the system typically gives us 8 MB
#define SCOPES_MAX_STACK_SIZE ((1 << 20) * 7)
#endif

#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

extern const char *scopes_compiler_path;
extern const char *scopes_compiler_dir;
extern const char *scopes_clang_include_dir;
extern const char *scopes_include_dir;
extern size_t scopes_argc;
extern char **scopes_argv;

void scopes_strtod(double *v, const char *str, char **str_end, int base );
void scopes_strtoll(int64_t *v, const char* str, char** endptr);
void scopes_strtoull(uint64_t *v, const char* str, char** endptr);

bool scopes_is_debug();

const char *scopes_compile_time_date();

#if defined(SCOPESRT_IMPL) && defined(__cplusplus)
} // extern "C"

namespace scopes {
    struct Type;
    struct Scope;
    struct Any;
    struct Symbol;
    struct String;
    struct List;
    struct Syntax;
    struct Anchor;
    struct Parameter;
    struct Label;
    struct Frame;
    struct Closure;
    struct ExceptionPad;
    struct Argument;
}

extern "C" {

typedef scopes::Type sc_type_t;
typedef scopes::Scope sc_scope_t;
typedef scopes::Symbol sc_symbol_t;
typedef scopes::Any sc_any_t;
typedef scopes::String sc_string_t;
typedef scopes::List sc_list_t;
typedef scopes::Syntax sc_syntax_t;
typedef scopes::Anchor sc_anchor_t;
typedef scopes::Parameter sc_parameter_t;
typedef scopes::Label sc_label_t;
typedef scopes::Frame sc_frame_t;
typedef scopes::Closure sc_closure_t;
typedef scopes::ExceptionPad sc_exception_pad_t;

// some of the return types are technically illegal in C, but we take care
// that the alignment is correct
#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"

#else

typedef struct sc_type_ sc_type_t;
typedef struct sc_scope_ sc_scope_t;
typedef struct sc_string_ sc_string_t;
typedef struct sc_list_ sc_list_t;
typedef struct sc_syntax_ sc_syntax_t;
typedef struct sc_anchor_ sc_anchor_t;
typedef struct sc_parameter_ sc_parameter_t;
typedef struct sc_label_ sc_label_t;
typedef struct sc_frame_ sc_frame_t;
typedef struct sc_closure_ sc_closure_t;
typedef struct sc_exception_pad_ sc_exception_pad_t;

typedef uint64_t sc_symbol_t;

typedef struct sc_any_ {
    const sc_type_t *type;
    uint64_t payload;
} sc_any_t;

#endif

typedef struct sc_any_bool_tuple_ {
    sc_any_t _0;
    bool _1;
} sc_any_bool_tuple_t;

typedef struct sc_symbol_any_tuple_ {
    sc_symbol_t _0;
    sc_any_t _1;
} sc_symbol_any_tuple_t;

typedef struct sc_i32_i32_i32_tuple_ {
    int32_t _0, _1, _2;
} sc_i32_i32_i32_tuple_t;

typedef struct sc_string_bool_tuple_ {
    const sc_string_t *_0;
    bool _1;
} sc_string_bool_tuple_t;

// compiler

sc_i32_i32_i32_tuple_t sc_compiler_version();
sc_label_t *sc_eval(const sc_syntax_t *expr, sc_scope_t *scope);
sc_label_t *sc_typify(sc_closure_t *srcl, int numtypes, const sc_type_t **typeargs);
sc_any_t sc_compile(sc_label_t *srcl, uint64_t flags);
const sc_string_t *sc_compile_spirv(sc_symbol_t target, sc_label_t *srcl, uint64_t flags);
const sc_string_t *sc_compile_glsl(sc_symbol_t target, sc_label_t *srcl, uint64_t flags);
void sc_compile_object(const sc_string_t *path, sc_scope_t *table, uint64_t flags);
void sc_enter_solver_cli ();
size_t sc_verify_stack ();

// stdin/out

const sc_string_t *sc_default_styler(sc_symbol_t style, const sc_string_t *str);
sc_string_bool_tuple_t sc_prompt(const sc_string_t *s, const sc_string_t *pre);
void sc_set_autocomplete_scope(const sc_scope_t* scope);
const sc_string_t *sc_format_message(const sc_anchor_t *anchor, const sc_string_t *message);
void sc_write(const sc_string_t *value);

// file I/O

const sc_string_t *sc_realpath(const sc_string_t *path);
const sc_string_t *sc_dirname(const sc_string_t *path);
const sc_string_t *sc_basename(const sc_string_t *path);
bool sc_is_file(const sc_string_t *path);
bool sc_is_directory(const sc_string_t *path);

// globals

sc_scope_t *sc_get_globals();
void sc_set_globals(sc_scope_t *s);

// error handling

void sc_error(const sc_string_t *msg);
void sc_anchor_error(const sc_string_t *msg);
void sc_raise(sc_any_t value);
void sc_set_signal_abort(bool value);
sc_exception_pad_t *sc_set_exception_pad(sc_exception_pad_t *pad);
sc_any_t sc_exception_value(sc_exception_pad_t *pad);

// hashing

uint64_t sc_hash (uint64_t data, size_t size);
uint64_t sc_hash2x64(uint64_t a, uint64_t b);
uint64_t sc_hashbytes (const char *data, size_t size);

// C bridge

sc_scope_t *sc_import_c(const sc_string_t *path,
    const sc_string_t *content, const sc_list_t *arglist);
void sc_load_library(const sc_string_t *name);

// anchors

void sc_set_active_anchor(const sc_anchor_t *anchor);
const sc_anchor_t *sc_get_active_anchor();

// lexical scopes

void sc_scope_set_symbol(sc_scope_t *scope, sc_symbol_t sym, sc_any_t value);
sc_any_bool_tuple_t sc_scope_at(sc_scope_t *scope, sc_symbol_t key);
sc_any_bool_tuple_t sc_scope_local_at(sc_scope_t *scope, sc_symbol_t key);
const sc_string_t *sc_scope_get_docstring(sc_scope_t *scope, sc_symbol_t key);
void sc_scope_set_docstring(sc_scope_t *scope, sc_symbol_t key, const sc_string_t *str);
sc_scope_t *sc_scope_new();
sc_scope_t *sc_scope_clone(sc_scope_t *clone);
sc_scope_t *sc_scope_new_subscope(sc_scope_t *scope);
sc_scope_t *sc_scope_clone_subscope(sc_scope_t *scope, sc_scope_t *clone);
sc_scope_t *sc_scope_get_parent(sc_scope_t *scope);
void sc_scope_del_symbol(sc_scope_t *scope, sc_symbol_t sym);
sc_symbol_any_tuple_t sc_scope_next(sc_scope_t *scope, sc_symbol_t key);

// symbols

sc_symbol_t sc_symbol_new(const sc_string_t *str);
const sc_string_t *sc_symbol_to_string(sc_symbol_t sym);

// strings

const sc_string_t *sc_string_new(const char *ptr, size_t count);
const sc_string_t *sc_string_join(const sc_string_t *a, const sc_string_t *b);
bool sc_string_match(const sc_string_t *pattern, const sc_string_t *text);

// any

const sc_string_t *sc_any_repr(sc_any_t value);
const sc_string_t *sc_any_string(sc_any_t value);
bool sc_any_eq(sc_any_t a, sc_any_t b);

// lists

const sc_list_t *sc_list_cons(sc_any_t at, const sc_list_t *next);
const sc_list_t *sc_list_join(const sc_list_t *a, const sc_list_t *b);
const sc_list_t *sc_list_dump(const sc_list_t *l);

// syntax objects

const sc_syntax_t *sc_syntax_from_path(const sc_string_t *path);
const sc_syntax_t *sc_syntax_from_string(const sc_string_t *str);
const sc_syntax_t *sc_syntax_new(const sc_anchor_t *anchor, sc_any_t value, bool quoted);
sc_any_t sc_syntax_wrap(const sc_anchor_t *anchor, sc_any_t e, bool quoted);
sc_any_t sc_syntax_strip(sc_any_t e);

// types

sc_any_bool_tuple_t sc_type_at(const sc_type_t *T, sc_symbol_t key);
size_t sc_type_sizeof(const sc_type_t *T);
size_t sc_type_alignof(const sc_type_t *T);
int sc_type_countof(const sc_type_t *T);
const sc_type_t *sc_type_element_at(const sc_type_t *T, int i);
int sc_type_field_index(const sc_type_t *T, sc_symbol_t name);
sc_symbol_t sc_type_field_name(const sc_type_t *T, int index);
int32_t sc_type_kind(const sc_type_t *T);
void sc_type_debug_abi(const sc_type_t *T);
const sc_type_t *sc_type_storage(const sc_type_t *T);
bool sc_type_is_opaque(const sc_type_t *T);
const sc_string_t *sc_type_string(const sc_type_t *T);
sc_symbol_any_tuple_t sc_type_next(const sc_type_t *type, sc_symbol_t key);

// pointer types

const sc_type_t *sc_pointer_type(const sc_type_t *T, uint64_t flags, sc_symbol_t storage_class);
uint64_t sc_pointer_type_get_flags(const sc_type_t *T);
const sc_type_t *sc_pointer_type_set_flags(const sc_type_t *T, uint64_t flags);
sc_symbol_t sc_pointer_type_get_storage_class(const sc_type_t *T);
const sc_type_t *sc_pointer_type_set_storage_class(const sc_type_t *T, sc_symbol_t storage_class);
const sc_type_t *sc_pointer_type_set_element_type(const sc_type_t *T, const sc_type_t *ET);

// extern types

int32_t sc_extern_type_location(const sc_type_t *T);
int32_t sc_extern_type_binding(const sc_type_t *T);

// numerical types

int32_t sc_type_bitcountof(const sc_type_t *T);

// integer types

const sc_type_t *sc_integer_type(int width, bool issigned);
bool sc_integer_type_is_signed(const sc_type_t *T);

// typename types

const sc_type_t *sc_typename_type(const sc_string_t *str);
void sc_typename_type_set_super(const sc_type_t *T, const sc_type_t *ST);
const sc_type_t *sc_typename_type_get_super(const sc_type_t *T);

// array types

const sc_type_t *sc_array_type(const sc_type_t *element_type, size_t count);

// vector types

const sc_type_t *sc_vector_type(const sc_type_t *element_type, size_t count);

// function types

bool sc_function_type_is_variadic(const sc_type_t *T);

// image types

const sc_type_t *sc_image_type(const sc_type_t *_type, sc_symbol_t _dim,
    int _depth, int _arrayed, int _multisampled, int _sampled,
    sc_symbol_t _format, sc_symbol_t _access);

// sampled image types

const sc_type_t *sc_sampled_image_type(const sc_type_t *_type);

// parameters

sc_parameter_t *sc_parameter_new(const sc_anchor_t *anchor, sc_symbol_t symbol, const sc_type_t *type);
int sc_parameter_index(const sc_parameter_t *param);
sc_symbol_t sc_parameter_name(const sc_parameter_t *param);

// labels

void sc_label_dump(sc_label_t *label);
void sc_label_set_inline (sc_label_t *label);
const sc_anchor_t *sc_label_anchor(sc_label_t *label);
sc_symbol_t sc_label_name(sc_label_t *label);
size_t sc_label_parameter_count(sc_label_t *label);
sc_parameter_t *sc_label_parameter(sc_label_t *label, size_t index);
size_t sc_label_countof_reachable(sc_label_t *label);
const sc_string_t *sc_label_docstring(sc_label_t *label);
sc_any_t sc_label_get_enter(sc_label_t *label);
void sc_label_set_enter(sc_label_t *label, sc_any_t value);
size_t sc_label_argument_count(sc_label_t *label);
sc_symbol_any_tuple_t sc_label_argument(sc_label_t *label, size_t index);
void sc_label_clear_arguments(sc_label_t *label);
void sc_label_append_argument(sc_label_t *label, sc_symbol_t key, sc_any_t value);

// frames

void sc_frame_dump(sc_frame_t *frame);

// closures

sc_label_t *sc_closure_label(const sc_closure_t *closure);
sc_frame_t *sc_closure_frame(const sc_closure_t *closure);

#if defined __cplusplus
}
#endif

#endif // SCOPES_H
