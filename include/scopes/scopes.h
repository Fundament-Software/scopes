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

#include "config.h"

#if defined __cplusplus
extern "C" {
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

typedef struct sc_bool_any_tuple_ { bool _0; sc_any_t _1; } sc_bool_any_tuple_t;
typedef struct sc_bool_list_tuple_ { bool _0; const sc_list_t *_1; } sc_bool_list_tuple_t;
typedef struct sc_bool_label_tuple_ { bool _0; sc_label_t *_1; } sc_bool_label_tuple_t;
typedef struct sc_bool_string_tuple_ { bool _0; const sc_string_t *_1; } sc_bool_string_tuple_t;
typedef struct sc_bool_scope_tuple_ { bool _0; sc_scope_t *_1; } sc_bool_scope_tuple_t;
typedef struct sc_bool_size_tuple_ { bool _0; size_t _1; } sc_bool_size_tuple_t;
typedef struct sc_bool_bool_tuple_ { bool _0; bool _1; } sc_bool_bool_tuple_t;
typedef struct sc_bool_syntax_tuple_ { bool _0; const sc_syntax_t *_1; } sc_bool_syntax_tuple_t;
typedef struct sc_bool_type_tuple_ { bool _0; const sc_type_t *_1; } sc_bool_type_tuple_t;
typedef struct sc_bool_symbol_tuple_ { bool _0; sc_symbol_t _1; } sc_bool_symbol_tuple_t;
typedef struct sc_bool_int_tuple_ { bool _0; int32_t _1; } sc_bool_int_tuple_t;

typedef struct sc_any_list_tuple_ { sc_any_t _0; const sc_list_t *_1; } sc_any_list_tuple_t;

typedef struct sc_symbol_any_tuple_ { sc_symbol_t _0; sc_any_t _1; } sc_symbol_any_tuple_t;

typedef struct sc_i32_i32_i32_tuple_ { int32_t _0, _1, _2; } sc_i32_i32_i32_tuple_t;

typedef struct sc_rawstring_size_t_tuple_ { const char *_0; size_t _1; } sc_rawstring_size_t_tuple_t;

// compiler

sc_i32_i32_i32_tuple_t sc_compiler_version();
sc_bool_label_tuple_t sc_eval(const sc_syntax_t *expr, sc_scope_t *scope);
sc_bool_label_tuple_t sc_typify(sc_closure_t *srcl, int numtypes, const sc_type_t **typeargs);
sc_bool_any_tuple_t sc_compile(sc_label_t *srcl, uint64_t flags);
sc_bool_string_tuple_t sc_compile_spirv(sc_symbol_t target, sc_label_t *srcl, uint64_t flags);
sc_bool_string_tuple_t sc_compile_glsl(sc_symbol_t target, sc_label_t *srcl, uint64_t flags);
bool sc_compile_object(const sc_string_t *path, sc_scope_t *table, uint64_t flags);
void sc_enter_solver_cli ();
sc_bool_size_tuple_t sc_verify_stack ();
sc_bool_label_tuple_t sc_eval_inline(const sc_list_t *expr, sc_scope_t *scope);
const sc_list_t *sc_launch_args();

// stdin/out

const sc_string_t *sc_default_styler(sc_symbol_t style, const sc_string_t *str);
sc_bool_string_tuple_t sc_prompt(const sc_string_t *s, const sc_string_t *pre);
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

void sc_set_last_error(sc_any_t err);
void sc_set_last_runtime_error(const sc_string_t *msg);
void sc_set_last_location_error(const sc_string_t *msg);
sc_any_t sc_get_last_error();
void sc_set_signal_abort(bool value);
void sc_abort();
void sc_exit(int c);

// memoization

sc_bool_list_tuple_t sc_map_load(const sc_list_t *key);
void sc_map_store(const sc_list_t *key, const sc_list_t *value);

// hashing

uint64_t sc_hash (uint64_t data, size_t size);
uint64_t sc_hash2x64(uint64_t a, uint64_t b);
uint64_t sc_hashbytes (const char *data, size_t size);

// C bridge

sc_bool_scope_tuple_t sc_import_c(const sc_string_t *path,
    const sc_string_t *content, const sc_list_t *arglist);
bool sc_load_library(const sc_string_t *name);

// anchors

void sc_set_active_anchor(const sc_anchor_t *anchor);
const sc_anchor_t *sc_get_active_anchor();

// lexical scopes

void sc_scope_set_symbol(sc_scope_t *scope, sc_symbol_t sym, sc_any_t value);
sc_bool_any_tuple_t sc_scope_at(sc_scope_t *scope, sc_symbol_t key);
sc_bool_any_tuple_t sc_scope_local_at(sc_scope_t *scope, sc_symbol_t key);
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
const sc_string_t *sc_string_new_from_cstr(const char *ptr);
const sc_string_t *sc_string_join(const sc_string_t *a, const sc_string_t *b);
sc_bool_bool_tuple_t sc_string_match(const sc_string_t *pattern, const sc_string_t *text);
size_t sc_string_count(sc_string_t *str);
sc_rawstring_size_t_tuple_t sc_string_buffer(sc_string_t *str);
const sc_string_t *sc_string_lslice(sc_string_t *str, size_t offset);
const sc_string_t *sc_string_rslice(sc_string_t *str, size_t offset);

// any

const sc_string_t *sc_any_repr(sc_any_t value);
const sc_string_t *sc_any_string(sc_any_t value);
bool sc_any_eq(sc_any_t a, sc_any_t b);

// lists

const sc_list_t *sc_list_cons(sc_any_t at, const sc_list_t *next);
const sc_list_t *sc_list_join(const sc_list_t *a, const sc_list_t *b);
const sc_list_t *sc_list_dump(const sc_list_t *l);
sc_any_list_tuple_t sc_list_decons(const sc_list_t *l);
size_t sc_list_count(const sc_list_t *l);
sc_any_t sc_list_at(const sc_list_t *l);
const sc_list_t *sc_list_next(const sc_list_t *l);
const sc_list_t *sc_list_reverse(const sc_list_t *l);

// syntax objects

sc_bool_syntax_tuple_t sc_syntax_from_path(const sc_string_t *path);
sc_bool_syntax_tuple_t sc_syntax_from_string(const sc_string_t *str);
const sc_syntax_t *sc_syntax_new(const sc_anchor_t *anchor, sc_any_t value, bool quoted);
sc_any_t sc_syntax_wrap(const sc_anchor_t *anchor, sc_any_t e, bool quoted);
sc_any_t sc_syntax_strip(sc_any_t e);

// types

sc_bool_any_tuple_t sc_type_at(const sc_type_t *T, sc_symbol_t key);
sc_bool_size_tuple_t sc_type_sizeof(const sc_type_t *T);
sc_bool_size_tuple_t sc_type_alignof(const sc_type_t *T);
sc_bool_int_tuple_t sc_type_countof(const sc_type_t *T);
sc_bool_type_tuple_t sc_type_element_at(const sc_type_t *T, int i);
sc_bool_int_tuple_t sc_type_field_index(const sc_type_t *T, sc_symbol_t name);
sc_bool_symbol_tuple_t sc_type_field_name(const sc_type_t *T, int index);
int32_t sc_type_kind(const sc_type_t *T);
void sc_type_debug_abi(const sc_type_t *T);
sc_bool_type_tuple_t sc_type_storage(const sc_type_t *T);
bool sc_type_is_opaque(const sc_type_t *T);
const sc_string_t *sc_type_string(const sc_type_t *T);
sc_symbol_any_tuple_t sc_type_next(const sc_type_t *type, sc_symbol_t key);
void sc_type_set_symbol(sc_type_t *T, sc_symbol_t sym, sc_any_t value);

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
bool sc_typename_type_set_super(const sc_type_t *T, const sc_type_t *ST);
const sc_type_t *sc_typename_type_get_super(const sc_type_t *T);
bool sc_typename_type_set_storage(const sc_type_t *T, const sc_type_t *T2);

// array types

sc_bool_type_tuple_t sc_array_type(const sc_type_t *element_type, size_t count);

// vector types

sc_bool_type_tuple_t sc_vector_type(const sc_type_t *element_type, size_t count);

// tuple types

sc_bool_type_tuple_t sc_tuple_type(int numtypes, const sc_type_t **types);

// function types

bool sc_function_type_is_variadic(const sc_type_t *T);
const sc_type_t *sc_function_type(const sc_type_t *return_type,
    int numtypes, const sc_type_t **typeargs);

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
const sc_type_t *sc_parameter_type(const sc_parameter_t *param);

// labels

void sc_label_dump(sc_label_t *label);
void sc_label_set_inline (sc_label_t *label);
const sc_anchor_t *sc_label_anchor(sc_label_t *label);
const sc_anchor_t *sc_label_body_anchor(sc_label_t *label);
sc_symbol_t sc_label_name(sc_label_t *label);
size_t sc_label_countof_reachable(sc_label_t *label);
const sc_string_t *sc_label_docstring(sc_label_t *label);
sc_any_t sc_label_get_enter(sc_label_t *label);
void sc_label_set_enter(sc_label_t *label, sc_any_t value);
const sc_list_t *sc_label_get_arguments(sc_label_t *label);
void sc_label_set_arguments(sc_label_t *label, const sc_list_t *list);
const sc_list_t *sc_label_get_keyed(sc_label_t *label);
bool sc_label_set_keyed(sc_label_t *label, const sc_list_t *list);
const sc_list_t *sc_label_get_parameters(sc_label_t *label);
sc_label_t *sc_label_new_cont();
sc_label_t *sc_label_new_cont_template();
sc_label_t *sc_label_new_function_template();
sc_label_t *sc_label_new_inline_template();
void sc_label_set_complete(sc_label_t *label);
bool sc_label_append_parameter(sc_label_t *label, sc_parameter_t *param);
const sc_type_t *sc_label_function_type(sc_label_t *label);
void sc_label_set_rawcall(sc_label_t *label);
sc_frame_t *sc_label_frame(sc_label_t *label);

// frames

void sc_frame_dump(sc_frame_t *frame);
sc_frame_t *sc_frame_root();

// closures

const sc_closure_t *sc_closure_new(sc_label_t *label, sc_frame_t *frame);
sc_label_t *sc_closure_label(const sc_closure_t *closure);
sc_frame_t *sc_closure_frame(const sc_closure_t *closure);

#if defined __cplusplus
}
#endif

#endif // SCOPES_H
