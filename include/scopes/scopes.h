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

#ifdef SCOPES_WIN32
#define SCOPES_LIBEXPORT __declspec(dllexport)
#elif defined(SCOPES_LINUX)
#define SCOPES_LIBEXPORT //__attribute__ ((visibility ("default")))
#else
#define SCOPES_LIBEXPORT
#endif

#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

SCOPES_LIBEXPORT extern const char *scopes_compiler_path;
SCOPES_LIBEXPORT extern const char *scopes_compiler_dir;
SCOPES_LIBEXPORT extern const char *scopes_working_dir;
SCOPES_LIBEXPORT extern size_t scopes_argc;
SCOPES_LIBEXPORT extern char **scopes_argv;

SCOPES_LIBEXPORT void scopes_strtod(double *v, const char *str, char **str_end, int base );
SCOPES_LIBEXPORT void scopes_strtoll(int64_t *v, const char* str, char** endptr);
SCOPES_LIBEXPORT void scopes_strtoull(uint64_t *v, const char* str, char** endptr);

SCOPES_LIBEXPORT bool scopes_is_debug();

SCOPES_LIBEXPORT const char *scopes_compile_time_date();

#if defined(SCOPESRT_IMPL) && defined(__cplusplus)
} // extern "C"

namespace scopes {
    struct Type;
    struct Scope;
    struct String;
    struct List;
    struct Error;
    struct Anchor;
    struct Parameter;
    struct Frame;
    struct Value;
    struct Closure;
}

extern "C" {

typedef scopes::Type sc_type_t;
typedef scopes::Scope sc_scope_t;
typedef scopes::String sc_string_t;
typedef scopes::List sc_list_t;
typedef scopes::Error sc_error_t;
typedef scopes::Anchor sc_anchor_t;
typedef scopes::Parameter sc_parameter_t;
typedef scopes::Frame sc_frame_t;
typedef scopes::Value sc_value_t;
typedef scopes::Closure sc_closure_t;

typedef scopes::ValueRef sc_valueref_t;

// some of the return types are technically illegal in C, but we take care
// that the alignment is correct
#if !defined(__clang__) && defined(_MSC_VER)
#pragma warning( disable : 4190 )
#else
#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"
#endif

#else

typedef struct sc_type_ sc_type_t;
typedef struct sc_scope_ sc_scope_t;
typedef struct sc_string_ sc_string_t;
typedef struct sc_list_ sc_list_t;
typedef struct sc_error_ sc_error_t;
typedef struct sc_anchor_ sc_anchor_t;
typedef struct sc_parameter_ sc_parameter_t;
typedef struct sc_frame_ sc_frame_t;
typedef struct sc_value_ sc_value_t;
typedef struct sc_closure_ sc_closure_t;

typedef struct sc_valueref_ { sc_value_t *_0; const sc_anchor_t *_1; } sc_valueref_t;

#endif

// aliasing uint64_t to a struct with a single uint64_t member is UB, and hasn't
// been a problem on any GCC version we've seen, on any platform, but is known
// to cause trouble with clang-cl, so we stick to a fixed definition.
typedef uint64_t sc_symbol_t;

typedef struct sc_bool_string_tuple_ { bool _0; const sc_string_t *_1; } sc_bool_string_tuple_t;
typedef struct sc_bool_valueref_tuple_ { bool _0; sc_valueref_t _1; } sc_bool_valueref_tuple_t;
typedef struct sc_bool_i32_i32_tuple_ { bool _0; int32_t _1, _2; } sc_bool_i32_i32_tuple_t;

typedef struct sc_valueref_list_tuple_ { sc_valueref_t _0; const sc_list_t *_1; } sc_valueref_list_tuple_t;
typedef struct sc_valueref_list_scope_tuple_ { sc_valueref_t _0; const sc_list_t *_1; const sc_scope_t *_2; } sc_valueref_list_scope_tuple_t;

typedef struct sc_valueref_valueref_i32_tuple_ { sc_valueref_t _0; sc_valueref_t _1; int _2; } sc_valueref_valueref_i32_tuple_t;
typedef struct sc_scope_valueref_valueref_i32_tuple_ { const sc_scope_t *_0; sc_valueref_t _1; sc_valueref_t _2; int _3; } sc_scope_valueref_valueref_i32_tuple_t;
typedef struct sc_valueref_i32_tuple_ { sc_valueref_t _0; int _1; } sc_valueref_i32_tuple_t;

typedef struct sc_symbol_valueref_tuple_ { sc_symbol_t _0; sc_valueref_t _1; } sc_symbol_valueref_tuple_t;
typedef struct sc_symbol_type_tuple_ { sc_symbol_t _0; const sc_type_t *_1; } sc_symbol_type_tuple_t;

typedef struct sc_i32_i32_i32_tuple_ { int32_t _0, _1, _2; } sc_i32_i32_i32_tuple_t;

typedef struct sc_rawstring_size_t_tuple_ { const char *_0; size_t _1; } sc_rawstring_size_t_tuple_t;

typedef struct sc_rawstring_i32_array_tuple_ { int _0; char **_1; } sc_rawstring_i32_array_tuple_t;

typedef struct sc_type_type_tuple_ { const sc_type_t *_0; const sc_type_t *_1; } sc_type_type_tuple_t;

typedef struct sc_list_scope_tuple_ { const sc_list_t *_0; const sc_scope_t *_1; } sc_list_scope_tuple_t;

// raising types

typedef struct sc_void_raises_ { bool ok; sc_error_t *except; } sc_void_raises_t;
#define SCOPES_TYPEDEF_RESULT_RAISES(NAME, RESULT_TYPE) \
    typedef struct NAME ## _ { bool ok; sc_error_t *except; RESULT_TYPE _0; } NAME ## _t

SCOPES_TYPEDEF_RESULT_RAISES(sc_valueref_raises, sc_valueref_t);
SCOPES_TYPEDEF_RESULT_RAISES(sc_string_raises, const sc_string_t *);
SCOPES_TYPEDEF_RESULT_RAISES(sc_size_raises, size_t);
SCOPES_TYPEDEF_RESULT_RAISES(sc_scope_raises, const sc_scope_t *);
SCOPES_TYPEDEF_RESULT_RAISES(sc_int_raises, int32_t);
SCOPES_TYPEDEF_RESULT_RAISES(sc_uint_raises, uint32_t);
SCOPES_TYPEDEF_RESULT_RAISES(sc_symbol_raises, sc_symbol_t);
SCOPES_TYPEDEF_RESULT_RAISES(sc_type_raises, const sc_type_t *);
SCOPES_TYPEDEF_RESULT_RAISES(sc_bool_raises, bool);

SCOPES_TYPEDEF_RESULT_RAISES(sc_valueref_list_scope_raises, sc_valueref_list_scope_tuple_t);
SCOPES_TYPEDEF_RESULT_RAISES(sc_list_scope_raises, sc_list_scope_tuple_t);
SCOPES_TYPEDEF_RESULT_RAISES(sc_bool_i32_i32_raises, sc_bool_i32_i32_tuple_t);

// prototypes

typedef sc_valueref_raises_t (*sc_ast_macro_func_t)(sc_valueref_t);
typedef sc_valueref_raises_t (*sc_typecast_func_t)(sc_valueref_t, const sc_type_t *);
typedef sc_list_scope_raises_t (*sc_syntax_wildcard_func_t)(const sc_list_t *, const sc_scope_t *);
typedef void (*sc_autocomplete_func_t)(const char *, void *);

// booting

SCOPES_LIBEXPORT sc_valueref_raises_t sc_load_from_executable(const char *path);
SCOPES_LIBEXPORT void sc_init(void *c_main, int argc, char *argv[]);
SCOPES_LIBEXPORT int sc_main();

// stats & info

SCOPES_LIBEXPORT sc_i32_i32_i32_tuple_t sc_compiler_version();
SCOPES_LIBEXPORT int sc_cache_misses();

// compiler

SCOPES_LIBEXPORT sc_valueref_list_scope_raises_t sc_expand(sc_valueref_t expr, const sc_list_t *next, const sc_scope_t *scope);
SCOPES_LIBEXPORT sc_valueref_raises_t sc_eval(const sc_anchor_t *anchor, const sc_list_t *expr, const sc_scope_t *scope);
SCOPES_LIBEXPORT sc_valueref_raises_t sc_eval_stage(const sc_anchor_t *anchor, const sc_list_t *expr, const sc_scope_t *scope);
SCOPES_LIBEXPORT sc_valueref_raises_t sc_prove(sc_valueref_t expr);
SCOPES_LIBEXPORT sc_valueref_raises_t sc_typify(const sc_closure_t *f, int numtypes, const sc_type_t **typeargs);
SCOPES_LIBEXPORT sc_valueref_raises_t sc_typify_template(sc_valueref_t f, int numtypes, const sc_type_t **typeargs);
SCOPES_LIBEXPORT sc_valueref_raises_t sc_compile(sc_valueref_t srcl, uint64_t flags);
SCOPES_LIBEXPORT sc_string_raises_t sc_compile_spirv(int version, sc_symbol_t target, sc_valueref_t srcl, uint64_t flags);
SCOPES_LIBEXPORT sc_string_raises_t sc_compile_glsl(int version, sc_symbol_t target, sc_valueref_t srcl, uint64_t flags);
SCOPES_LIBEXPORT const sc_string_t *sc_spirv_to_glsl(const sc_string_t *binary);
SCOPES_LIBEXPORT const sc_string_t *sc_default_target_triple();
SCOPES_LIBEXPORT sc_void_raises_t sc_compile_object(const sc_string_t *target_triple, int file_kind, const sc_string_t *path, const sc_scope_t *table, uint64_t flags);
SCOPES_LIBEXPORT sc_string_raises_t sc_compile_object_to_buffer(const sc_string_t *target_triple, int file_kind, const sc_string_t *module_name, const sc_scope_t *table, uint64_t flags);
SCOPES_LIBEXPORT void sc_enter_solver_cli ();
SCOPES_LIBEXPORT void sc_show_targets();
SCOPES_LIBEXPORT sc_valueref_raises_t sc_eval_inline(const sc_anchor_t *anchor, const sc_list_t *expr, const sc_scope_t *scope);
SCOPES_LIBEXPORT sc_rawstring_i32_array_tuple_t sc_launch_args();
SCOPES_LIBEXPORT void sc_set_typecast_handler(sc_typecast_func_t func);

// value

SCOPES_LIBEXPORT const sc_string_t *sc_value_repr (sc_valueref_t value);
SCOPES_LIBEXPORT const sc_string_t *sc_value_content_repr (sc_valueref_t value);
SCOPES_LIBEXPORT const sc_string_t *sc_value_ast_repr (sc_valueref_t value);
SCOPES_LIBEXPORT const sc_string_t *sc_value_tostring (sc_valueref_t value);
SCOPES_LIBEXPORT const sc_type_t *sc_value_type (sc_valueref_t value);
SCOPES_LIBEXPORT const sc_type_t *sc_value_qualified_type (sc_valueref_t value);
SCOPES_LIBEXPORT const sc_anchor_t *sc_value_anchor (sc_valueref_t value);
SCOPES_LIBEXPORT sc_valueref_t sc_valueref_tag(sc_anchor_t *anchor, sc_valueref_t value);
SCOPES_LIBEXPORT bool sc_value_is_constant (sc_valueref_t value);
SCOPES_LIBEXPORT bool sc_value_is_pure (sc_valueref_t value);
SCOPES_LIBEXPORT bool sc_value_compare (sc_valueref_t a, sc_valueref_t b);
SCOPES_LIBEXPORT int sc_value_kind (sc_valueref_t value);
SCOPES_LIBEXPORT int sc_value_block_depth (sc_valueref_t value);
SCOPES_LIBEXPORT sc_valueref_t sc_identity(sc_valueref_t value);
SCOPES_LIBEXPORT sc_valueref_t sc_value_wrap(const sc_type_t *type, sc_valueref_t value);
SCOPES_LIBEXPORT sc_valueref_t sc_value_unwrap(const sc_type_t *type, sc_valueref_t value);
SCOPES_LIBEXPORT const sc_string_t *sc_value_kind_string(int kind);

SCOPES_LIBEXPORT sc_valueref_t sc_keyed_new(sc_symbol_t key, sc_valueref_t value);

SCOPES_LIBEXPORT sc_valueref_t sc_empty_argument_list();
SCOPES_LIBEXPORT sc_valueref_t sc_argument_list_new(int count, const sc_valueref_t *values);
SCOPES_LIBEXPORT sc_valueref_t sc_extract_argument_new(sc_valueref_t value, int index);
SCOPES_LIBEXPORT sc_valueref_t sc_extract_argument_list_new(sc_valueref_t value, int index);
SCOPES_LIBEXPORT int sc_argcount(sc_valueref_t value);
SCOPES_LIBEXPORT sc_valueref_t sc_getarg(sc_valueref_t value, int index);
SCOPES_LIBEXPORT sc_valueref_t sc_getarglist(sc_valueref_t value, int index);

SCOPES_LIBEXPORT sc_valueref_t sc_template_new(sc_symbol_t name);
SCOPES_LIBEXPORT void sc_template_set_name(sc_valueref_t fn, sc_symbol_t name);
SCOPES_LIBEXPORT sc_symbol_t sc_template_get_name(sc_valueref_t fn);
SCOPES_LIBEXPORT void sc_template_append_parameter(sc_valueref_t fn, sc_valueref_t symbol);
SCOPES_LIBEXPORT void sc_template_set_body(sc_valueref_t fn, sc_valueref_t value);
SCOPES_LIBEXPORT void sc_template_set_inline(sc_valueref_t fn);
SCOPES_LIBEXPORT bool sc_template_is_inline(sc_valueref_t fn);
SCOPES_LIBEXPORT int sc_template_parameter_count(sc_valueref_t fn);
SCOPES_LIBEXPORT sc_valueref_t sc_template_parameter(sc_valueref_t fn, int index);

SCOPES_LIBEXPORT sc_valueref_t sc_expression_new();
SCOPES_LIBEXPORT void sc_expression_append(sc_valueref_t expr, sc_valueref_t value);
SCOPES_LIBEXPORT void sc_expression_set_scoped(sc_valueref_t expr);

SCOPES_LIBEXPORT sc_valueref_t sc_global_new(sc_symbol_t name,
    const sc_type_t *type, uint32_t flags /* = 0 */, sc_symbol_t storage_class /* = unnamed */);
SCOPES_LIBEXPORT sc_void_raises_t sc_global_set_initializer(sc_valueref_t value,
    sc_valueref_t init);
SCOPES_LIBEXPORT sc_void_raises_t sc_global_set_constructor(sc_valueref_t value,
    sc_valueref_t func);
SCOPES_LIBEXPORT sc_void_raises_t sc_global_set_location(sc_valueref_t value, int location);
SCOPES_LIBEXPORT sc_void_raises_t sc_global_set_binding(sc_valueref_t value, int binding);
SCOPES_LIBEXPORT sc_void_raises_t sc_global_set_descriptor_set(sc_valueref_t value, int set);
SCOPES_LIBEXPORT sc_int_raises_t sc_global_location(sc_valueref_t value);
SCOPES_LIBEXPORT sc_int_raises_t sc_global_binding(sc_valueref_t value);
SCOPES_LIBEXPORT sc_int_raises_t sc_global_descriptor_set(sc_valueref_t value);
SCOPES_LIBEXPORT sc_symbol_raises_t sc_global_storage_class(sc_valueref_t value);
SCOPES_LIBEXPORT sc_symbol_raises_t sc_global_name(sc_valueref_t value);
SCOPES_LIBEXPORT sc_uint_raises_t sc_global_flags(sc_valueref_t value);

SCOPES_LIBEXPORT sc_valueref_t sc_cond_new(sc_valueref_t cond, sc_valueref_t then_value, sc_valueref_t else_value);

SCOPES_LIBEXPORT sc_valueref_t sc_case_new(sc_valueref_t literal, sc_valueref_t body);
SCOPES_LIBEXPORT sc_valueref_t sc_pass_case_new(sc_valueref_t literal, sc_valueref_t body);
SCOPES_LIBEXPORT sc_valueref_t sc_do_case_new(sc_valueref_t body);
SCOPES_LIBEXPORT sc_valueref_t sc_default_case_new(sc_valueref_t body);

SCOPES_LIBEXPORT sc_valueref_t sc_switch_new(sc_valueref_t expr);
SCOPES_LIBEXPORT void sc_switch_append(sc_valueref_t value, sc_valueref_t _case);
SCOPES_LIBEXPORT void sc_switch_append_case(sc_valueref_t value, sc_valueref_t literal, sc_valueref_t body);
SCOPES_LIBEXPORT void sc_switch_append_pass(sc_valueref_t value, sc_valueref_t literal, sc_valueref_t body);
SCOPES_LIBEXPORT void sc_switch_append_do(sc_valueref_t value, sc_valueref_t body);
SCOPES_LIBEXPORT void sc_switch_append_default(sc_valueref_t value, sc_valueref_t body);

SCOPES_LIBEXPORT sc_valueref_t sc_parameter_new(sc_symbol_t name);
SCOPES_LIBEXPORT bool sc_parameter_is_variadic(sc_valueref_t param);
SCOPES_LIBEXPORT sc_symbol_t sc_parameter_name(sc_valueref_t value);

SCOPES_LIBEXPORT sc_valueref_t sc_call_new(sc_valueref_t callee);
SCOPES_LIBEXPORT void sc_call_append_argument(sc_valueref_t call, sc_valueref_t value);
SCOPES_LIBEXPORT bool sc_call_is_rawcall(sc_valueref_t value);
SCOPES_LIBEXPORT void sc_call_set_rawcall(sc_valueref_t value, bool enable);

SCOPES_LIBEXPORT sc_valueref_t sc_loop_new(sc_valueref_t init);
SCOPES_LIBEXPORT sc_valueref_t sc_loop_arguments(sc_valueref_t loop);
SCOPES_LIBEXPORT void sc_loop_set_body(sc_valueref_t loop, sc_valueref_t body);

SCOPES_LIBEXPORT sc_valueref_t sc_const_int_new(const sc_type_t *type, uint64_t value);
SCOPES_LIBEXPORT sc_valueref_t sc_const_int_words_new(const sc_type_t *type, int numwords, uint64_t *words);
SCOPES_LIBEXPORT sc_valueref_t sc_const_real_new(const sc_type_t *type, double value);
SCOPES_LIBEXPORT sc_valueref_raises_t sc_const_aggregate_new(const sc_type_t *type, int numconsts, sc_valueref_t *consts);
SCOPES_LIBEXPORT sc_valueref_t sc_const_pointer_new(const sc_type_t *type, const void *pointer);
SCOPES_LIBEXPORT sc_valueref_raises_t sc_const_null_new(const sc_type_t *type);
SCOPES_LIBEXPORT uint64_t sc_const_int_extract(const sc_valueref_t value);
SCOPES_LIBEXPORT uint64_t sc_const_int_extract_word(const sc_valueref_t value, int index);
SCOPES_LIBEXPORT int sc_const_int_word_count(const sc_valueref_t value);
SCOPES_LIBEXPORT double sc_const_real_extract(const sc_valueref_t value);
SCOPES_LIBEXPORT sc_valueref_t sc_const_extract_at(const sc_valueref_t value, int index);
SCOPES_LIBEXPORT const void *sc_const_pointer_extract(const sc_valueref_t value);
SCOPES_LIBEXPORT sc_valueref_t sc_const_string_new(const sc_string_t *str);
SCOPES_LIBEXPORT const sc_string_t *sc_const_string_extract(sc_valueref_t value);

SCOPES_LIBEXPORT sc_valueref_t sc_quote_new(sc_valueref_t value);
SCOPES_LIBEXPORT sc_valueref_t sc_unquote_new(sc_valueref_t value);

SCOPES_LIBEXPORT sc_valueref_t sc_label_new(int kind, sc_symbol_t name);
SCOPES_LIBEXPORT void sc_label_set_body(sc_valueref_t label, sc_valueref_t body);
SCOPES_LIBEXPORT sc_valueref_t sc_merge_new(sc_valueref_t label, sc_valueref_t value);

// parsing

SCOPES_LIBEXPORT sc_valueref_raises_t sc_parse_from_path(const sc_string_t *path);
SCOPES_LIBEXPORT sc_valueref_raises_t sc_parse_from_string(const sc_string_t *str);

// stdin/out

SCOPES_LIBEXPORT const sc_string_t *sc_default_styler(sc_symbol_t style, const sc_string_t *str);
SCOPES_LIBEXPORT sc_bool_string_tuple_t sc_prompt(const sc_string_t *s, const sc_string_t *pre);
SCOPES_LIBEXPORT void sc_prompt_save_history(const sc_string_t *path);
SCOPES_LIBEXPORT void sc_prompt_load_history(const sc_string_t *path);
SCOPES_LIBEXPORT void sc_prompt_set_autocomplete_handler(sc_autocomplete_func_t func);
SCOPES_LIBEXPORT void sc_prompt_add_completion(void *ctx, const char *text);
SCOPES_LIBEXPORT void sc_prompt_add_completion_from_scope(void *ctx,
    const char *searchtext, int offset, const sc_scope_t* scope);

SCOPES_LIBEXPORT const sc_string_t *sc_format_message(const sc_anchor_t *anchor, const sc_string_t *message);
SCOPES_LIBEXPORT void sc_write(const sc_string_t *value);

// file I/O

SCOPES_LIBEXPORT const sc_string_t *sc_realpath(const sc_string_t *path);
SCOPES_LIBEXPORT const sc_string_t *sc_dirname(const sc_string_t *path);
SCOPES_LIBEXPORT const sc_string_t *sc_basename(const sc_string_t *path);
SCOPES_LIBEXPORT bool sc_is_file(const sc_string_t *path);
SCOPES_LIBEXPORT bool sc_is_directory(const sc_string_t *path);

// globals

SCOPES_LIBEXPORT const sc_scope_t *sc_get_globals();
SCOPES_LIBEXPORT const sc_scope_t *sc_get_original_globals();
SCOPES_LIBEXPORT void sc_set_globals(const sc_scope_t *s);

// error handling

SCOPES_LIBEXPORT void sc_error_append_calltrace(sc_error_t *err, sc_valueref_t callexpr);
SCOPES_LIBEXPORT const sc_string_t *sc_format_error(const sc_error_t *err);
SCOPES_LIBEXPORT void sc_dump_error(const sc_error_t *err);
SCOPES_LIBEXPORT sc_error_t *sc_error_new(const sc_string_t *msg);
SCOPES_LIBEXPORT void sc_set_signal_abort(bool value);
SCOPES_LIBEXPORT void sc_abort();
SCOPES_LIBEXPORT void sc_exit(int c);

// memoization

SCOPES_LIBEXPORT sc_valueref_raises_t sc_map_get(sc_valueref_t key);
SCOPES_LIBEXPORT void sc_map_set(sc_valueref_t key, sc_valueref_t value);

// hashing

SCOPES_LIBEXPORT uint64_t sc_hash (uint64_t data, size_t size);
SCOPES_LIBEXPORT uint64_t sc_hash2x64(uint64_t a, uint64_t b);
SCOPES_LIBEXPORT uint64_t sc_hashbytes (const char *data, size_t size);

// C bridge

SCOPES_LIBEXPORT sc_scope_raises_t sc_import_c(const sc_string_t *path,
    const sc_string_t *content, const sc_list_t *arglist, const sc_scope_t *scope);
SCOPES_LIBEXPORT sc_void_raises_t sc_load_library(const sc_string_t *name);
SCOPES_LIBEXPORT sc_void_raises_t sc_load_object(const sc_string_t *path);

// lexical scopes

SCOPES_LIBEXPORT const sc_scope_t *sc_scope_bind(const sc_scope_t *scope, sc_valueref_t key, sc_valueref_t value);
SCOPES_LIBEXPORT const sc_scope_t *sc_scope_bind_with_docstring(const sc_scope_t *scope, sc_valueref_t key, sc_valueref_t value, const sc_string_t *doc);
SCOPES_LIBEXPORT sc_valueref_raises_t sc_scope_at(const sc_scope_t *scope, sc_valueref_t key);
SCOPES_LIBEXPORT sc_valueref_raises_t sc_scope_local_at(const sc_scope_t *scope, sc_valueref_t key);
SCOPES_LIBEXPORT const sc_string_t *sc_scope_module_docstring(const sc_scope_t *scope);
SCOPES_LIBEXPORT const sc_string_t *sc_scope_docstring(const sc_scope_t *scope, sc_valueref_t key);
SCOPES_LIBEXPORT void sc_scope_set_docstring(const sc_scope_t *scope, sc_valueref_t key, const sc_string_t *str);
SCOPES_LIBEXPORT const sc_scope_t *sc_scope_new();
SCOPES_LIBEXPORT const sc_scope_t *sc_scope_new_with_docstring(const sc_string_t *doc);
SCOPES_LIBEXPORT const sc_scope_t *sc_scope_reparent(const sc_scope_t *scope, const sc_scope_t *parent);
SCOPES_LIBEXPORT const sc_scope_t *sc_scope_unparent(const sc_scope_t *scope);
SCOPES_LIBEXPORT const sc_scope_t *sc_scope_new_subscope(const sc_scope_t *scope);
SCOPES_LIBEXPORT const sc_scope_t *sc_scope_new_subscope_with_docstring(const sc_scope_t *scope, const sc_string_t *doc);
SCOPES_LIBEXPORT const sc_scope_t *sc_scope_get_parent(const sc_scope_t *scope);
SCOPES_LIBEXPORT const sc_scope_t *sc_scope_unbind(const sc_scope_t *scope, sc_valueref_t sym);
SCOPES_LIBEXPORT sc_valueref_valueref_i32_tuple_t sc_scope_next(const sc_scope_t *scope, int index);
SCOPES_LIBEXPORT sc_scope_valueref_valueref_i32_tuple_t sc_scope_any_next(const sc_scope_t *scope, int index);
SCOPES_LIBEXPORT sc_valueref_i32_tuple_t sc_scope_next_deleted(const sc_scope_t *scope, int index);

// symbols

SCOPES_LIBEXPORT sc_symbol_t sc_symbol_new(const sc_string_t *str);
SCOPES_LIBEXPORT sc_symbol_t sc_symbol_new_unique(const sc_string_t *str);
SCOPES_LIBEXPORT const sc_string_t *sc_symbol_to_string(sc_symbol_t sym);
SCOPES_LIBEXPORT bool sc_symbol_is_variadic(sc_symbol_t sym);
SCOPES_LIBEXPORT size_t sc_symbol_count();
SCOPES_LIBEXPORT sc_symbol_t sc_symbol_style(sc_symbol_t name);

// strings

SCOPES_LIBEXPORT const sc_string_t *sc_string_new(const char *ptr, size_t count);
SCOPES_LIBEXPORT const sc_string_t *sc_string_new_from_cstr(const char *ptr);
SCOPES_LIBEXPORT const sc_string_t *sc_string_join(const sc_string_t *a, const sc_string_t *b);
SCOPES_LIBEXPORT sc_bool_i32_i32_raises_t sc_string_match(const sc_string_t *pattern, const sc_string_t *text);
SCOPES_LIBEXPORT size_t sc_string_count(const sc_string_t *str);
SCOPES_LIBEXPORT sc_rawstring_size_t_tuple_t sc_string_buffer(const sc_string_t *str);
SCOPES_LIBEXPORT const sc_string_t *sc_string_lslice(const sc_string_t *str, size_t offset);
SCOPES_LIBEXPORT const sc_string_t *sc_string_rslice(const sc_string_t *str, size_t offset);
SCOPES_LIBEXPORT int sc_string_compare(const sc_string_t *a, const sc_string_t *b);
SCOPES_LIBEXPORT const sc_string_t *sc_string_unescape(const sc_string_t *str);

// lists

SCOPES_LIBEXPORT const sc_list_t *sc_list_cons(sc_valueref_t at, const sc_list_t *next);
SCOPES_LIBEXPORT const sc_list_t *sc_list_join(const sc_list_t *a, const sc_list_t *b);
SCOPES_LIBEXPORT const sc_list_t *sc_list_dump(const sc_list_t *l);
SCOPES_LIBEXPORT const sc_string_t *sc_list_repr(const sc_list_t *l);
SCOPES_LIBEXPORT const sc_string_t *sc_list_serialize(const sc_list_t *l);
SCOPES_LIBEXPORT sc_valueref_list_tuple_t sc_list_decons(const sc_list_t *l);
SCOPES_LIBEXPORT int sc_list_count(const sc_list_t *l);
SCOPES_LIBEXPORT sc_valueref_t sc_list_at(const sc_list_t *l);
SCOPES_LIBEXPORT const sc_list_t *sc_list_next(const sc_list_t *l);
SCOPES_LIBEXPORT const sc_list_t *sc_list_reverse(const sc_list_t *l);
SCOPES_LIBEXPORT bool sc_list_compare(const sc_list_t *a, const sc_list_t *b);

// anchors

SCOPES_LIBEXPORT const sc_anchor_t *sc_anchor_new(sc_symbol_t path, int lineno, int column, int offset);
SCOPES_LIBEXPORT sc_symbol_t sc_anchor_path(const sc_anchor_t *anchor);
SCOPES_LIBEXPORT int sc_anchor_lineno(const sc_anchor_t *anchor);
SCOPES_LIBEXPORT int sc_anchor_column(const sc_anchor_t *anchor);
SCOPES_LIBEXPORT const sc_anchor_t *sc_anchor_offset(const sc_anchor_t *anchor, int offset);

// closures

SCOPES_LIBEXPORT const sc_string_t *sc_closure_get_docstring(const sc_closure_t *func);
SCOPES_LIBEXPORT sc_valueref_t sc_closure_get_template(const sc_closure_t *func);
SCOPES_LIBEXPORT sc_valueref_t sc_closure_get_context(const sc_closure_t *func);

// types

SCOPES_LIBEXPORT sc_valueref_raises_t sc_type_at(const sc_type_t *T, sc_symbol_t key);
SCOPES_LIBEXPORT sc_valueref_raises_t sc_type_local_at(const sc_type_t *T, sc_symbol_t key);
SCOPES_LIBEXPORT const sc_string_t *sc_type_get_docstring(const sc_type_t *T, sc_symbol_t key);
SCOPES_LIBEXPORT void sc_type_set_docstring(const sc_type_t *T, sc_symbol_t key, const sc_string_t *str);
SCOPES_LIBEXPORT sc_size_raises_t sc_type_sizeof(const sc_type_t *T);
SCOPES_LIBEXPORT sc_size_raises_t sc_type_alignof(const sc_type_t *T);
SCOPES_LIBEXPORT sc_size_raises_t sc_type_offsetof(const sc_type_t *T, int index);
SCOPES_LIBEXPORT sc_int_raises_t sc_type_countof(const sc_type_t *T);
SCOPES_LIBEXPORT sc_bool_raises_t sc_type_is_unsized(const sc_type_t *T);
SCOPES_LIBEXPORT sc_type_raises_t sc_type_element_at(const sc_type_t *T, int i);
SCOPES_LIBEXPORT sc_int_raises_t sc_type_field_index(const sc_type_t *T, sc_symbol_t name);
SCOPES_LIBEXPORT sc_symbol_raises_t sc_type_field_name(const sc_type_t *T, int index);
SCOPES_LIBEXPORT int32_t sc_type_kind(const sc_type_t *T);
SCOPES_LIBEXPORT void sc_type_debug_abi(const sc_type_t *T);
SCOPES_LIBEXPORT sc_type_raises_t sc_type_storage(const sc_type_t *T);
SCOPES_LIBEXPORT bool sc_type_is_opaque(const sc_type_t *T);
SCOPES_LIBEXPORT bool sc_type_is_plain(const sc_type_t *T);
SCOPES_LIBEXPORT bool sc_type_is_superof(const sc_type_t *super, const sc_type_t *T);
SCOPES_LIBEXPORT bool sc_type_compatible(const sc_type_t *have, const sc_type_t *need);
SCOPES_LIBEXPORT bool sc_type_is_default_suffix(const sc_type_t *T);
SCOPES_LIBEXPORT const sc_string_t *sc_type_string(const sc_type_t *T);
SCOPES_LIBEXPORT sc_symbol_valueref_tuple_t sc_type_next(const sc_type_t *type, sc_symbol_t key);
SCOPES_LIBEXPORT void sc_type_set_symbol(const sc_type_t *T, sc_symbol_t sym, sc_valueref_t value);
SCOPES_LIBEXPORT void sc_type_del_symbol(const sc_type_t *T, sc_symbol_t sym);

// pointer types

SCOPES_LIBEXPORT const sc_type_t *sc_pointer_type(const sc_type_t *T, uint64_t flags, sc_symbol_t storage_class);
SCOPES_LIBEXPORT uint64_t sc_pointer_type_get_flags(const sc_type_t *T);
SCOPES_LIBEXPORT const sc_type_t *sc_pointer_type_set_flags(const sc_type_t *T, uint64_t flags);
SCOPES_LIBEXPORT sc_symbol_t sc_pointer_type_get_storage_class(const sc_type_t *T);
SCOPES_LIBEXPORT const sc_type_t *sc_pointer_type_set_storage_class(const sc_type_t *T, sc_symbol_t storage_class);
SCOPES_LIBEXPORT const sc_type_t *sc_pointer_type_set_element_type(const sc_type_t *T, const sc_type_t *ET);

// numerical types

SCOPES_LIBEXPORT int32_t sc_type_bitcountof(const sc_type_t *T);

// integer types

SCOPES_LIBEXPORT const sc_type_t *sc_integer_type(int width, bool issigned);
SCOPES_LIBEXPORT bool sc_integer_type_is_signed(const sc_type_t *T);

// typename types

SCOPES_LIBEXPORT sc_type_raises_t sc_typename_type(const sc_string_t *str, const sc_type_t *supertype);
SCOPES_LIBEXPORT const sc_type_t *sc_typename_type_get_super(const sc_type_t *T);
SCOPES_LIBEXPORT sc_void_raises_t sc_typename_type_set_storage(const sc_type_t *T, const sc_type_t *T2, uint32_t flags);
SCOPES_LIBEXPORT sc_void_raises_t sc_typename_type_set_opaque(const sc_type_t *T);

// array types

SCOPES_LIBEXPORT sc_type_raises_t sc_array_type(const sc_type_t *element_type, size_t count);
SCOPES_LIBEXPORT const sc_type_t *sc_array_type_set_zterm(const sc_type_t *T, bool zterm);
SCOPES_LIBEXPORT bool sc_array_type_is_zterm(const sc_type_t *T);
SCOPES_LIBEXPORT const sc_type_t *sc_array_type_set_count(const sc_type_t *T, size_t count);

// vector types

SCOPES_LIBEXPORT sc_type_raises_t sc_vector_type(const sc_type_t *element_type, size_t count);

// matrix types

SCOPES_LIBEXPORT sc_type_raises_t sc_matrix_type(const sc_type_t *element_type, size_t count);

// tuple types

SCOPES_LIBEXPORT sc_type_raises_t sc_tuple_type(int numtypes, const sc_type_t **types);
SCOPES_LIBEXPORT sc_type_raises_t sc_packed_tuple_type(int numtypes, const sc_type_t **types);
SCOPES_LIBEXPORT sc_type_raises_t sc_union_storage_type(int numtypes, const sc_type_t **types);

// argument types

SCOPES_LIBEXPORT const sc_type_t *sc_arguments_type(int numtypes, const sc_type_t **types);
SCOPES_LIBEXPORT const sc_type_t *sc_arguments_type_join(const sc_type_t *T1, const sc_type_t *T2);
SCOPES_LIBEXPORT int sc_arguments_type_argcount(sc_type_t *T);
SCOPES_LIBEXPORT const sc_type_t *sc_arguments_type_getarg(sc_type_t *T, int index);

// qualifiers

SCOPES_LIBEXPORT const sc_type_t *sc_key_type(sc_symbol_t name, const sc_type_t *T);
SCOPES_LIBEXPORT sc_symbol_type_tuple_t sc_type_key(const sc_type_t *T);
SCOPES_LIBEXPORT bool sc_type_is_refer(const sc_type_t *T);
SCOPES_LIBEXPORT bool sc_type_is_view(const sc_type_t *T);
SCOPES_LIBEXPORT const sc_type_t *sc_view_type(const sc_type_t *type, int id);
SCOPES_LIBEXPORT const sc_type_t *sc_unique_type(const sc_type_t *type, int id);
SCOPES_LIBEXPORT const sc_type_t *sc_mutate_type(const sc_type_t *type);
SCOPES_LIBEXPORT const sc_type_t *sc_refer_type(const sc_type_t *type, uint64_t flags, sc_symbol_t storage_class);
SCOPES_LIBEXPORT uint64_t sc_refer_flags(const sc_type_t *type);
SCOPES_LIBEXPORT sc_symbol_t sc_refer_storage_class(const sc_type_t *type);
SCOPES_LIBEXPORT const sc_type_t *sc_strip_qualifiers(const sc_type_t *type);

// function types

SCOPES_LIBEXPORT bool sc_function_type_is_variadic(const sc_type_t *T);
SCOPES_LIBEXPORT const sc_type_t *sc_function_type(const sc_type_t *return_type,
    int numtypes, const sc_type_t **typeargs);
SCOPES_LIBEXPORT const sc_type_t *sc_function_type_raising(const sc_type_t *T,
    const sc_type_t *except_type);
SCOPES_LIBEXPORT sc_type_type_tuple_t sc_function_type_return_type(const sc_type_t *T);

// image types

SCOPES_LIBEXPORT const sc_type_t *sc_image_type(const sc_type_t *_type, sc_symbol_t _dim,
    int _depth, int _arrayed, int _multisampled, int _sampled,
    sc_symbol_t _format, sc_symbol_t _access);

// sampled image types

SCOPES_LIBEXPORT const sc_type_t *sc_sampled_image_type(const sc_type_t *_type);

#if defined __cplusplus
}
#endif

#endif // SCOPES_H
