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
}

extern "C" {

typedef scopes::Type sc_type_t;
typedef scopes::Scope sc_scope_t;
typedef scopes::Symbol sc_symbol_t;
typedef scopes::Any sc_any_t;
typedef scopes::String sc_string_t;

// some of the return types are technically illegal in C, but we take care
// that the alignment is correct
#pragma GCC diagnostic ignored "-Wreturn-type-c-linkage"

#else

typedef struct sc_type_ sc_type_t;
typedef struct sc_scope_ sc_scope_t;
typedef struct sc_string_ sc_string_t;

typedef uint64_t sc_symbol_t;

typedef struct sc_any_ {
    sc_type_t *type;
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

#if defined __cplusplus
}
#endif

#endif // SCOPES_H
