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
    struct Symbol;
    struct String;
    struct List;
    struct Error;
    struct Anchor;
    struct Parameter;
    struct Frame;
    struct Closure;
    struct Value;
}

extern "C" {

typedef scopes::Type sc_type_t;
typedef scopes::Scope sc_scope_t;
typedef scopes::Symbol sc_symbol_t;
typedef scopes::String sc_string_t;
typedef scopes::List sc_list_t;
typedef scopes::Error sc_error_t;
typedef scopes::Anchor sc_anchor_t;
typedef scopes::Parameter sc_parameter_t;
typedef scopes::Frame sc_frame_t;
typedef scopes::Closure sc_closure_t;
typedef scopes::Value sc_value_t;

// some of the return types are technically illegal in C, but we take care
// that the alignment is correct
#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"

#else

typedef struct sc_type_ sc_type_t;
typedef struct sc_scope_ sc_scope_t;
typedef struct sc_string_ sc_string_t;
typedef struct sc_list_ sc_list_t;
typedef struct sc_error_ sc_error_t;
typedef struct sc_anchor_ sc_anchor_t;
typedef struct sc_parameter_ sc_parameter_t;
typedef struct sc_frame_ sc_frame_t;
typedef struct sc_closure_ sc_closure_t;
typedef struct sc_value_ sc_value_t;

typedef uint64_t sc_symbol_t;

#endif

typedef struct sc_bool_string_tuple_ { bool _0; const sc_string_t *_1; } sc_bool_string_tuple_t;
typedef struct sc_bool_value_tuple_ { bool _0; sc_value_t *_1; } sc_bool_value_tuple_t;

typedef struct sc_value_list_tuple_ { sc_value_t *_0; const sc_list_t *_1; } sc_value_list_tuple_t;
typedef struct sc_value_list_scope_tuple_ { sc_value_t *_0; const sc_list_t *_1; sc_scope_t *_2; } sc_value_list_scope_tuple_t;

typedef struct sc_symbol_value_tuple_ { sc_symbol_t _0; sc_value_t *_1; } sc_symbol_value_tuple_t;
typedef struct sc_symbol_type_tuple_ { sc_symbol_t _0; const sc_type_t *_1; } sc_symbol_type_tuple_t;

typedef struct sc_i32_i32_i32_tuple_ { int32_t _0, _1, _2; } sc_i32_i32_i32_tuple_t;

typedef struct sc_rawstring_size_t_tuple_ { const char *_0; size_t _1; } sc_rawstring_size_t_tuple_t;

typedef struct sc_rawstring_i32_array_tuple_ { int _0; char **_1; } sc_rawstring_i32_array_tuple_t;

typedef struct sc_type_type_tuple_ { const sc_type_t *_0; const sc_type_t *_1; } sc_type_type_tuple_t;

typedef struct sc_list_scope_tuple_ { const sc_list_t *_0; sc_scope_t *_1; } sc_list_scope_tuple_t;

// raising types

typedef struct sc_void_raises_ { bool ok; sc_error_t *except; } sc_void_raises_t;
#define SCOPES_TYPEDEF_RESULT_RAISES(NAME, RESULT_TYPE) \
    typedef struct NAME ## _ { bool ok; sc_error_t *except; RESULT_TYPE _0; } NAME ## _t

SCOPES_TYPEDEF_RESULT_RAISES(sc_value_raises, sc_value_t *);
SCOPES_TYPEDEF_RESULT_RAISES(sc_string_raises, const sc_string_t *);
SCOPES_TYPEDEF_RESULT_RAISES(sc_size_raises, size_t);
SCOPES_TYPEDEF_RESULT_RAISES(sc_scope_raises, sc_scope_t *);
SCOPES_TYPEDEF_RESULT_RAISES(sc_int_raises, int32_t);
SCOPES_TYPEDEF_RESULT_RAISES(sc_symbol_raises, sc_symbol_t);
SCOPES_TYPEDEF_RESULT_RAISES(sc_type_raises, const sc_type_t *);
SCOPES_TYPEDEF_RESULT_RAISES(sc_bool_raises, bool);

SCOPES_TYPEDEF_RESULT_RAISES(sc_value_list_scope_raises, sc_value_list_scope_tuple_t);
SCOPES_TYPEDEF_RESULT_RAISES(sc_list_scope_raises, sc_list_scope_tuple_t);

// prototypes

typedef sc_value_raises_t (*sc_ast_macro_func_t)(sc_value_t *);
typedef sc_list_scope_raises_t (*sc_syntax_wildcard_func_t)(const sc_list_t *, sc_scope_t *);

// compiler

sc_i32_i32_i32_tuple_t sc_compiler_version();
sc_value_list_scope_raises_t sc_expand(sc_value_t *expr, const sc_list_t *next, sc_scope_t *scope);
sc_value_raises_t sc_eval(const sc_anchor_t *anchor, const sc_list_t *expr, sc_scope_t *scope);
sc_value_raises_t sc_typify_template(sc_value_t *f, int numtypes, const sc_type_t **typeargs);
sc_value_raises_t sc_typify(sc_closure_t *srcl, int numtypes, const sc_type_t **typeargs);
sc_value_raises_t sc_compile(sc_value_t *srcl, uint64_t flags);
sc_string_raises_t sc_compile_spirv(sc_symbol_t target, sc_value_t *srcl, uint64_t flags);
sc_string_raises_t sc_compile_glsl(sc_symbol_t target, sc_value_t *srcl, uint64_t flags);
sc_void_raises_t sc_compile_object(const sc_string_t *path, sc_scope_t *table, uint64_t flags);
void sc_enter_solver_cli ();
sc_size_raises_t sc_verify_stack ();
sc_value_raises_t sc_eval_inline(const sc_anchor_t *anchor, const sc_list_t *expr, sc_scope_t *scope);
sc_rawstring_i32_array_tuple_t sc_launch_args();

// value

const sc_string_t *sc_value_repr (sc_value_t *value);
const sc_string_t *sc_value_ast_repr (sc_value_t *value);
const sc_string_t *sc_value_tostring (sc_value_t *value);
const sc_type_t *sc_value_type (sc_value_t *value);
const sc_type_t *sc_value_qualified_type (sc_value_t *value);
const sc_anchor_t *sc_value_anchor (sc_value_t *value);
bool sc_value_is_constant (sc_value_t *value);
bool sc_value_is_pure (sc_value_t *value);
bool sc_value_compare (sc_value_t *a, sc_value_t *b);
int sc_value_kind (sc_value_t *value);
sc_value_t *sc_value_wrap(const sc_type_t *type, sc_value_t *value);
sc_value_t *sc_value_unwrap(const sc_type_t *type, sc_value_t *value);

sc_value_t *sc_keyed_new(const sc_anchor_t *anchor, sc_symbol_t key, sc_value_t *value);

sc_value_t *sc_argument_list_new(const sc_anchor_t *anchor);
void sc_argument_list_append(sc_value_t *alist, sc_value_t *value);
sc_value_t *sc_extract_argument_new(const sc_anchor_t *anchor, sc_value_t *value, int index);
sc_value_t *sc_extract_argument_list_new(const sc_anchor_t *anchor, sc_value_t *value, int index);
int sc_argcount(sc_value_t *value);
sc_value_t *sc_getarg(sc_value_t *value, int index);
sc_value_t *sc_getarglist(sc_value_t *value, int index);

sc_value_t *sc_template_new(const sc_anchor_t *anchor, sc_symbol_t name);
void sc_template_set_name(sc_value_t *fn, sc_symbol_t name);
sc_symbol_t sc_template_get_name(sc_value_t *fn);
void sc_template_append_parameter(sc_value_t *fn, sc_value_t *symbol);
void sc_template_set_body(sc_value_t *fn, sc_value_t *value);
void sc_template_set_inline(sc_value_t *fn);

sc_value_t *sc_expression_new(const sc_anchor_t *anchor);
void sc_expression_append(sc_value_t *expr, sc_value_t *value);
void sc_expression_set_scoped(sc_value_t *expr);

sc_value_t *sc_global_new(const sc_anchor_t *anchor, sc_symbol_t name,
    const sc_type_t *type, uint32_t flags /* = 0 */, sc_symbol_t storage_class /* = unnamed */,
    int location /* = -1 */, int binding /* = -1 */);

sc_value_t *sc_if_new(const sc_anchor_t *anchor);
void sc_if_append_then_clause(sc_value_t *value, const sc_anchor_t *anchor, sc_value_t *cond, sc_value_t *body);
void sc_if_append_else_clause(sc_value_t *value, const sc_anchor_t *anchor, sc_value_t *body);

sc_value_t *sc_switch_new(const sc_anchor_t *anchor, sc_value_t *expr);
void sc_switch_append_case(sc_value_t *value, const sc_anchor_t *anchor, sc_value_t *literal, sc_value_t *body);
void sc_switch_append_pass(sc_value_t *value, const sc_anchor_t *anchor, sc_value_t *literal, sc_value_t *body);
void sc_switch_append_default(sc_value_t *value, const sc_anchor_t *anchor, sc_value_t *body);

sc_value_t *sc_parameter_new(const sc_anchor_t *anchor, sc_symbol_t name);
bool sc_parameter_is_variadic(sc_value_t *param);

sc_value_t *sc_call_new(const sc_anchor_t *anchor, sc_value_t *callee);
void sc_call_append_argument(sc_value_t *call, sc_value_t *value);
bool sc_call_is_rawcall(sc_value_t *value);
void sc_call_set_rawcall(sc_value_t *value, bool enable);

sc_value_t *sc_loop_new(const sc_anchor_t *anchor, sc_value_t *init);
sc_value_t *sc_loop_arguments(sc_value_t *loop);
void sc_loop_set_body(sc_value_t *loop, sc_value_t *body);

sc_value_t *sc_const_int_new(const sc_anchor_t *anchor, const sc_type_t *type, uint64_t value);
sc_value_t *sc_const_real_new(const sc_anchor_t *anchor, const sc_type_t *type, double value);
sc_value_t *sc_const_aggregate_new(const sc_anchor_t *anchor, const sc_type_t *type, int numconsts, sc_value_t **consts);
sc_value_t *sc_const_pointer_new(const sc_anchor_t *anchor, const sc_type_t *type, const void *pointer);
uint64_t sc_const_int_extract(const sc_value_t *value);
double sc_const_real_extract(const sc_value_t *value);
sc_value_t *sc_const_extract_at(const sc_value_t *value, int index);
const void *sc_const_pointer_extract(const sc_value_t *value);

sc_value_t *sc_break_new(const sc_anchor_t *anchor, sc_value_t *value);
sc_value_t *sc_repeat_new(const sc_anchor_t *anchor, sc_value_t *value);
sc_value_t *sc_return_new(const sc_anchor_t *anchor, sc_value_t *value);
sc_value_t *sc_raise_new(const sc_anchor_t *anchor, sc_value_t *value);

sc_value_t *sc_quote_new(const sc_anchor_t *anchor, sc_value_t *value);
sc_value_t *sc_unquote_new(const sc_anchor_t *anchor, sc_value_t *value);

sc_value_t *sc_label_new(const sc_anchor_t *anchor, int kind, sc_symbol_t name);
void sc_label_set_body(sc_value_t *label, sc_value_t *body);
sc_value_t *sc_merge_new(const sc_anchor_t *anchor, sc_value_t *label, sc_value_t *value);

// parsing

sc_value_raises_t sc_parse_from_path(const sc_string_t *path);
sc_value_raises_t sc_parse_from_string(const sc_string_t *str);

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
sc_scope_t *sc_get_original_globals();
void sc_set_globals(sc_scope_t *s);

// error handling

const sc_string_t *sc_format_error(const sc_error_t *err);
sc_error_t *sc_location_error_new(const sc_anchor_t *anchor, const sc_string_t *msg);
sc_error_t *sc_runtime_error_new(const sc_string_t *msg);
void sc_set_signal_abort(bool value);
void sc_abort();
void sc_exit(int c);

// memoization

sc_value_t *sc_map_get(sc_value_t *key);
void sc_map_set(sc_value_t *key, sc_value_t *value);

// hashing

uint64_t sc_hash (uint64_t data, size_t size);
uint64_t sc_hash2x64(uint64_t a, uint64_t b);
uint64_t sc_hashbytes (const char *data, size_t size);

// C bridge

sc_scope_raises_t sc_import_c(const sc_string_t *path,
    const sc_string_t *content, const sc_list_t *arglist);
sc_void_raises_t sc_load_library(const sc_string_t *name);

// anchors

void sc_set_active_anchor(const sc_anchor_t *anchor);
const sc_anchor_t *sc_get_active_anchor();

// lexical scopes

void sc_scope_set_symbol(sc_scope_t *scope, sc_symbol_t sym, sc_value_t *value);
sc_value_raises_t sc_scope_at(sc_scope_t *scope, sc_symbol_t key);
sc_value_raises_t sc_scope_local_at(sc_scope_t *scope, sc_symbol_t key);
const sc_string_t *sc_scope_get_docstring(sc_scope_t *scope, sc_symbol_t key);
void sc_scope_set_docstring(sc_scope_t *scope, sc_symbol_t key, const sc_string_t *str);
sc_scope_t *sc_scope_new();
sc_scope_t *sc_scope_clone(sc_scope_t *clone);
sc_scope_t *sc_scope_new_subscope(sc_scope_t *scope);
sc_scope_t *sc_scope_clone_subscope(sc_scope_t *scope, sc_scope_t *clone);
sc_scope_t *sc_scope_get_parent(sc_scope_t *scope);
void sc_scope_del_symbol(sc_scope_t *scope, sc_symbol_t sym);
sc_symbol_value_tuple_t sc_scope_next(sc_scope_t *scope, sc_symbol_t key);

// symbols

sc_symbol_t sc_symbol_new(const sc_string_t *str);
sc_symbol_t sc_symbol_new_unique(const sc_string_t *str);
const sc_string_t *sc_symbol_to_string(sc_symbol_t sym);
bool sc_symbol_is_variadic(sc_symbol_t sym);

// strings

const sc_string_t *sc_string_new(const char *ptr, size_t count);
const sc_string_t *sc_string_new_from_cstr(const char *ptr);
const sc_string_t *sc_string_join(const sc_string_t *a, const sc_string_t *b);
sc_bool_raises_t sc_string_match(const sc_string_t *pattern, const sc_string_t *text);
size_t sc_string_count(const sc_string_t *str);
sc_rawstring_size_t_tuple_t sc_string_buffer(const sc_string_t *str);
const sc_string_t *sc_string_lslice(const sc_string_t *str, size_t offset);
const sc_string_t *sc_string_rslice(const sc_string_t *str, size_t offset);
int sc_string_compare(const sc_string_t *a, const sc_string_t *b);

// lists

const sc_list_t *sc_list_cons(sc_value_t *at, const sc_list_t *next);
const sc_list_t *sc_list_join(const sc_list_t *a, const sc_list_t *b);
const sc_list_t *sc_list_dump(const sc_list_t *l);
const sc_string_t *sc_list_repr(const sc_list_t *l);
sc_value_list_tuple_t sc_list_decons(const sc_list_t *l);
int sc_list_count(const sc_list_t *l);
sc_value_t *sc_list_at(const sc_list_t *l);
const sc_list_t *sc_list_next(const sc_list_t *l);
const sc_list_t *sc_list_reverse(const sc_list_t *l);
bool sc_list_compare(const sc_list_t *a, const sc_list_t *b);

// closures

const sc_string_t *sc_closure_get_docstring(sc_closure_t *func);
sc_value_t *sc_closure_get_template(sc_closure_t *func);
sc_value_t *sc_closure_get_context(sc_closure_t *func);

// types

sc_value_raises_t sc_type_at(const sc_type_t *T, sc_symbol_t key);
sc_value_raises_t sc_type_local_at(const sc_type_t *T, sc_symbol_t key);
sc_size_raises_t sc_type_sizeof(const sc_type_t *T);
sc_size_raises_t sc_type_alignof(const sc_type_t *T);
sc_int_raises_t sc_type_countof(const sc_type_t *T);
sc_type_raises_t sc_type_element_at(const sc_type_t *T, int i);
sc_int_raises_t sc_type_field_index(const sc_type_t *T, sc_symbol_t name);
sc_symbol_raises_t sc_type_field_name(const sc_type_t *T, int index);
int32_t sc_type_kind(const sc_type_t *T);
void sc_type_debug_abi(const sc_type_t *T);
sc_type_raises_t sc_type_storage(const sc_type_t *T);
bool sc_type_is_opaque(const sc_type_t *T);
bool sc_type_is_superof(const sc_type_t *super, const sc_type_t *T);
const sc_string_t *sc_type_string(const sc_type_t *T);
sc_symbol_value_tuple_t sc_type_next(const sc_type_t *type, sc_symbol_t key);
void sc_type_set_symbol(const sc_type_t *T, sc_symbol_t sym, sc_value_t *value);

// pointer types

const sc_type_t *sc_pointer_type(const sc_type_t *T, uint64_t flags, sc_symbol_t storage_class);
uint64_t sc_pointer_type_get_flags(const sc_type_t *T);
const sc_type_t *sc_pointer_type_set_flags(const sc_type_t *T, uint64_t flags);
sc_symbol_t sc_pointer_type_get_storage_class(const sc_type_t *T);
const sc_type_t *sc_pointer_type_set_storage_class(const sc_type_t *T, sc_symbol_t storage_class);
const sc_type_t *sc_pointer_type_set_element_type(const sc_type_t *T, const sc_type_t *ET);

// numerical types

int32_t sc_type_bitcountof(const sc_type_t *T);

// integer types

const sc_type_t *sc_integer_type(int width, bool issigned);
bool sc_integer_type_is_signed(const sc_type_t *T);

// typename types

const sc_type_t *sc_typename_type(const sc_string_t *str);
sc_void_raises_t sc_typename_type_set_super(const sc_type_t *T, const sc_type_t *ST);
const sc_type_t *sc_typename_type_get_super(const sc_type_t *T);
sc_void_raises_t sc_typename_type_set_storage(const sc_type_t *T, const sc_type_t *T2, uint32_t flags);

// array types

sc_type_raises_t sc_array_type(const sc_type_t *element_type, size_t count);

// vector types

sc_type_raises_t sc_vector_type(const sc_type_t *element_type, size_t count);

// tuple types

sc_type_raises_t sc_tuple_type(int numtypes, const sc_type_t **types);

// union types

sc_type_raises_t sc_union_type(int numtypes, const sc_type_t **types);

// argument types

const sc_type_t *sc_arguments_type(int numtypes, const sc_type_t **types);
const sc_type_t *sc_arguments_type_join(const sc_type_t *T1, const sc_type_t *T2);
int sc_arguments_type_argcount(sc_type_t *T);
const sc_type_t *sc_arguments_type_getarg(sc_type_t *T, int index);

// qualifiers

const sc_type_t *sc_key_type(sc_symbol_t name, const sc_type_t *T);
sc_symbol_type_tuple_t sc_type_key(const sc_type_t *T);
bool sc_type_is_refer(const sc_type_t *T);
const sc_type_t *sc_view_type(const sc_type_t *type, int id);
const sc_type_t *sc_unique_type(const sc_type_t *type, int id);
const sc_type_t *sc_mutate_type(const sc_type_t *type);
const sc_type_t *sc_refer_type(const sc_type_t *type, uint64_t flags, sc_symbol_t storage_class);
const sc_type_t *sc_strip_qualifiers(const sc_type_t *type);

// function types

bool sc_function_type_is_variadic(const sc_type_t *T);
const sc_type_t *sc_function_type(const sc_type_t *return_type,
    int numtypes, const sc_type_t **typeargs);
const sc_type_t *sc_function_type_raising(const sc_type_t *T,
    const sc_type_t *except_type);
sc_type_type_tuple_t sc_function_type_return_type(const sc_type_t *T);

// image types

const sc_type_t *sc_image_type(const sc_type_t *_type, sc_symbol_t _dim,
    int _depth, int _arrayed, int _multisampled, int _sampled,
    sc_symbol_t _format, sc_symbol_t _access);

// sampled image types

const sc_type_t *sc_sampled_image_type(const sc_type_t *_type);

#if defined __cplusplus
}
#endif

#endif // SCOPES_H
