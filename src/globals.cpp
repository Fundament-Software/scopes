/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "globals.hpp"
#include "string.hpp"
#include "list.hpp"
#include "types.hpp"
#include "qualifiers.hpp"
#include "qualifier.inc"
#include "c_import.hpp"
#include "stream_expr.hpp"
#include "scope.hpp"
#include "error.hpp"
#include "globals.hpp"
#include "platform_abi.hpp"
#include "source_file.hpp"
#include "lexerparser.hpp"
#include "expander.hpp"
#include "gen_llvm.hpp"
#include "gen_spirv.hpp"
#include "anchor.hpp"
#include "boot.hpp"
#include "gc.hpp"
#include "compiler_flags.hpp"
#include "hash.hpp"
#include "value.hpp"
#include "prover.hpp"
#include "quote.hpp"
#include "boot.hpp"

#include "scopes/scopes.h"

#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#ifdef SCOPES_WIN32
#include "stdlib_ex.h"
#include "dlfcn.h"
#endif
#include <libgen.h>

#include <vector>

#include "linenoise-ng/include/linenoise.h"
#include "minilibs/regexp.cpp"

#include <llvm-c/Support.h>

#include "dyn_cast.inc"
#include "verify_tools.inc"

#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

namespace scopes {

//------------------------------------------------------------------------------

#define T(NAME, STR) \
    TypedValueRef NAME;
SCOPES_REIMPORT_SYMBOLS()
#undef T

static void import_symbols() {
    auto globs = sc_get_original_globals();
#define T(NAME, STR) \
    {   auto _tmp = sc_scope_at(globs, Symbol(STR))._0; \
        assert(_tmp && STR); \
        NAME = _tmp.cast<TypedValue>(); }
SCOPES_REIMPORT_SYMBOLS()
#undef T
}

//------------------------------------------------------------------------------

#if 0
static void init_values_array(scopes::Values &dest, int numvalues, sc_valueref_t *values) {
    dest.reserve(numvalues);
    for (int i = 0; i < numvalues; ++i) {
        assert(values[i]);
        //assert(values[i] > (Value *)0x1000);
        dest.push_back(values[i]);
    }
}
#endif

template<typename T>
static void init_values_arrayT(std::vector< T * > &dest, int numvalues, sc_valueref_t *values) {
    dest.reserve(numvalues);
    for (int i = 0; i < numvalues; ++i) {
        assert(values[i]);
        dest.push_back(values[i].cast<T>().unref());
    }
}

//------------------------------------------------------------------------------

static Scope *globals = nullptr;
static Scope *original_globals = Scope::from();

//------------------------------------------------------------------------------

#define CRESULT { return {_result.ok(), (_result.ok()?nullptr:_result.unsafe_error()), _result.unsafe_extract()}; }
#define VOID_CRESULT { return {_result.ok(), (_result.ok()?nullptr:_result.unsafe_error())}; }

sc_void_raises_t convert_result(const Result<void> &_result) VOID_CRESULT;

sc_valueref_list_scope_raises_t convert_result(const Result<sc_valueref_list_scope_tuple_t> &_result) CRESULT;

sc_valueref_raises_t convert_result(const Result<ValueRef> &_result) CRESULT;
sc_valueref_raises_t convert_result(const Result<TypedValueRef> &_result) CRESULT;
sc_valueref_raises_t convert_result(const Result<FunctionRef> &_result) CRESULT;
sc_valueref_raises_t convert_result(const Result<TemplateRef> &_result) CRESULT;
sc_valueref_raises_t convert_result(const Result<ConstPointerRef> &_result) CRESULT;

sc_type_raises_t convert_result(const Result<const Type *> &_result) CRESULT;
sc_string_raises_t convert_result(const Result<const String *> &_result) CRESULT;

sc_scope_raises_t convert_result(const Result<Scope *> &_result) CRESULT;

sc_bool_raises_t convert_result(const Result<bool> &_result) CRESULT;
sc_int_raises_t convert_result(const Result<int> &_result) CRESULT;
sc_size_raises_t convert_result(const Result<size_t> &_result) CRESULT;

sc_symbol_raises_t convert_result(const Result<Symbol> &_result) CRESULT;

#undef CRESULT
#undef VOID_CRESULT

#define SCOPES_C_RETURN(EXPR) \
    return convert_result(Result<_result_type>((EXPR)));

#define SCOPES_C_ERROR(CLASS, ...) \
    return convert_result(Result<_result_type>::raise(Error ## CLASS::from(__VA_ARGS__)));

#define SCOPES_C_RETURN_ERROR(ERR) return convert_result(Result<_result_type>::raise(ERR));
// if ok fails, return
#define SCOPES_C_CHECK_OK(OK, ERR) if (!OK) { SCOPES_C_RETURN_ERROR(ERR); }
// if an expression returning a result fails, return
#define SCOPES_C_CHECK_RESULT(EXPR) { \
    auto _result = (EXPR); \
    SCOPES_C_CHECK_OK(_result.ok(), _result.unsafe_error()); \
}
// execute expression and return an error
#define SCOPES_C_EXPECT_ERROR(EXPR) {\
    auto _tmp = (EXPR); \
    assert(!_tmp.ok()); \
    SCOPES_C_RETURN_ERROR(_tmp.unsafe_error()); \
}
// try to extract a value from a result or return
#define SCOPES_C_GET_RESULT(EXPR) ({ \
        auto _result = (EXPR); \
        SCOPES_C_CHECK_OK(_result.ok(), _result.unsafe_error()); \
        _result.unsafe_extract(); \
    })

} // namespace scopes

extern "C" {

sc_valueref_raises_t sc_load_from_executable(const char *path) {
    using namespace scopes;
    return convert_result(load_custom_core(path));
}

SCOPES_LIBEXPORT int sc_main(const char *exepath, int argc, char *argv[]) {
    using namespace scopes;
    return run_main(exepath, argc, argv);
}

// Compiler
////////////////////////////////////////////////////////////////////////////////

sc_i32_i32_i32_tuple_t sc_compiler_version() {
    using namespace scopes;
    return {
        SCOPES_VERSION_MAJOR,
        SCOPES_VERSION_MINOR,
        SCOPES_VERSION_PATCH };
}

sc_rawstring_i32_array_tuple_t sc_launch_args() {
    using namespace scopes;
    return {(int)scopes_argc, scopes_argv};
}

sc_valueref_list_scope_raises_t sc_expand(sc_valueref_t expr, const sc_list_t *next, sc_scope_t *scope) {
    using namespace scopes;
    return convert_result(expand(expr, next, scope));
}

sc_valueref_raises_t sc_eval(const sc_anchor_t *anchor, const sc_list_t *expr, sc_scope_t *scope) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(TypedValueRef);
    auto module_result = SCOPES_C_GET_RESULT(expand_module(anchor, expr, scope));
    return convert_result(prove(FunctionRef(), module_result, {}));
}

sc_valueref_raises_t sc_prove(sc_valueref_t expr) {
    using namespace scopes;
    //SCOPES_RESULT_TYPE(TypedValue *);
    return convert_result(prove(expr));
}

sc_valueref_raises_t sc_eval_inline(const sc_anchor_t *anchor, const sc_list_t *expr, sc_scope_t *scope) {
    using namespace scopes;
    //const Anchor *anchor = expr->anchor();
    //auto list = SCOPES_GET_RESULT(extract_list_constant(expr));
    return convert_result(expand_inline(anchor, TemplateRef(), expr, scope));
}

sc_valueref_raises_t sc_typify_template(sc_valueref_t f, int numtypes, const sc_type_t **typeargs) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(ValueRef);
    auto tf = SCOPES_C_GET_RESULT(extract_template_constant(f));
    if (tf->is_inline()) {
        SCOPES_C_ERROR(CannotTypeInline);
    }
    Types types;
    for (int i = 0; i < numtypes; ++i) {
        types.push_back(typeargs[i]);
    }
    return convert_result(prove(FunctionRef(), tf, types));
}

sc_valueref_raises_t sc_typify(const sc_closure_t *srcl, int numtypes, const sc_type_t **typeargs) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(ValueRef);
    if (srcl->func->is_inline()) {
        SCOPES_C_ERROR(CannotTypeInline);
    }
    Types types;
    for (int i = 0; i < numtypes; ++i) {
        types.push_back(typeargs[i]);
    }
    auto anchor = srcl->func.anchor();
    #if 0 //SCOPES_DEBUG_CODEGEN
        StyledStream ss(std::cout);
        std::cout << "sc_typify non-normalized:" << std::endl;
        stream_ast(ss, srcl->func, StreamASTFormat());
        std::cout << std::endl;
    #endif
    return convert_result(prove(
        ref(anchor, srcl->frame),
        ref(anchor, srcl->func), types));
}

sc_valueref_raises_t sc_compile(sc_valueref_t srcl, uint64_t flags) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(ConstPointerRef);
    auto result = SCOPES_C_GET_RESULT(extract_function_constant(srcl));
    return convert_result(compile(result, flags));
}

sc_string_raises_t sc_compile_spirv(sc_symbol_t target, sc_valueref_t srcl, uint64_t flags) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(const String *);
    auto result = SCOPES_C_GET_RESULT(extract_function_constant(srcl));
    return convert_result(compile_spirv(target, result, flags));
}

sc_string_raises_t sc_compile_glsl(sc_symbol_t target, sc_valueref_t srcl, uint64_t flags) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(const String *);
    auto result = SCOPES_C_GET_RESULT(extract_function_constant(srcl));
    return convert_result(compile_glsl(target, result, flags));
}

sc_void_raises_t sc_compile_object(const sc_string_t *path, sc_scope_t *table, uint64_t flags) {
    using namespace scopes;
    return convert_result(compile_object(path, table, flags));
}

void sc_enter_solver_cli () {
    using namespace scopes;
    //enable_specializer_step_debugger();
}

sc_size_raises_t sc_verify_stack () {
    using namespace scopes;
    SCOPES_RESULT_TYPE(size_t);
    SCOPES_C_RETURN(verify_stack());
}

// stdin/out
////////////////////////////////////////////////////////////////////////////////

const sc_string_t *sc_default_styler(sc_symbol_t style, const sc_string_t *str) {
    using namespace scopes;
    StyledString ss;
    if (!style.is_known()) {
        ss.out << str->data;
    } else {
        ss.out << Style(style.known_value()) << str->data << Style_None;
    }
    return ss.str();
}

sc_bool_string_tuple_t sc_prompt(const sc_string_t *s, const sc_string_t *pre) {
    using namespace scopes;
    if (pre->count) {
        linenoisePreloadBuffer(pre->data);
    }
    char *r = linenoise(s->data);
    if (!r) {
        return { false, Symbol(SYM_Unnamed).name() };
    }
    linenoiseHistoryAdd(r);
#ifdef SCOPES_WIN32
    StyledStream ss;
    ss << std::endl;
#endif
    return { true, String::from_cstr(r) };
}

namespace scopes {

static const Scope *autocomplete_scope = nullptr;

static void prompt_completion_cb(const char *buf, linenoiseCompletions *lc) {
    // Tab on an empty string gives an indentation
    if (*buf == 0) {
        linenoiseAddCompletion(lc, "    ");
        return;
    }

    const String* name = String::from_cstr(buf);
    Symbol sym(name);
    const Scope *scope = autocomplete_scope ? autocomplete_scope : globals;
    for (const auto& m : scope->find_elongations(sym))
        linenoiseAddCompletion(lc, m.name()->data);
}

}

void sc_set_autocomplete_scope(const sc_scope_t* scope) {
    using namespace scopes;
    autocomplete_scope = scope;
}

const sc_string_t *sc_format_message(const sc_anchor_t *anchor, const sc_string_t *message) {
    using namespace scopes;
    StyledString ss;
    if (anchor) {
        ss.out << anchor << " ";
    }
    ss.out << message->data << std::endl;
    if (anchor) {
        anchor->stream_source_line(ss.out);
    }
    return ss.str();
}

void sc_write(const sc_string_t *value) {
    using namespace scopes;
#if SCOPES_USE_WCHAR
    StyledStream ss(SCOPES_COUT);
    ss << value->data;
#else
    fputs(value->data, stdout);
#endif
}

// file i/o
////////////////////////////////////////////////////////////////////////////////

const sc_string_t *sc_realpath(const sc_string_t *path) {
    using namespace scopes;
    char buf[PATH_MAX];
    auto result = realpath(path->data, buf);
    if (!result) {
        return Symbol(SYM_Unnamed).name();
    } else {
        return String::from_cstr(result);
    }
}

const sc_string_t *sc_dirname(const sc_string_t *path) {
    using namespace scopes;
    auto pathcopy = strdup(path->data);
    auto result = String::from_cstr(dirname(pathcopy));
    free(pathcopy);
    return result;
}

const sc_string_t *sc_basename(const sc_string_t *path) {
    using namespace scopes;
    auto pathcopy = strdup(path->data);
    auto result = String::from_cstr(basename(pathcopy));
    free(pathcopy);
    return result;
}

bool sc_is_file(const sc_string_t *path) {
    using namespace scopes;
    struct stat s;
    if( stat(path->data,&s) == 0 ) {
        if( s.st_mode & S_IFDIR ) {
        } else if ( s.st_mode & S_IFREG ) {
            return true;
        }
    }
    return false;
}

bool sc_is_directory(const sc_string_t *path) {
    using namespace scopes;
    struct stat s;
    if( stat(path->data,&s) == 0 ) {
        if( s.st_mode & S_IFDIR ) {
            return true;
        }
    }
    return false;
}

// globals
////////////////////////////////////////////////////////////////////////////////

sc_scope_t *sc_get_globals() {
    using namespace scopes;
    return globals;
}

sc_scope_t *sc_get_original_globals() {
    using namespace scopes;
    return original_globals;
}

void sc_set_globals(sc_scope_t *s) {
    using namespace scopes;
    globals = s;
}

// Error Handling
////////////////////////////////////////////////////////////////////////////////

void sc_error_append_calltrace(sc_error_t *err, sc_valueref_t callexpr) {
    using namespace scopes;
    SCOPES_TRACE(User, callexpr);
    err->trace(_backtrace);
}

sc_error_t *sc_error_new(const sc_string_t *msg) {
    using namespace scopes;
    return ErrorUser::from(msg);
}
const sc_string_t *sc_format_error(const sc_error_t *err) {
    using namespace scopes;
    StyledString ss;
    stream_error_message(ss.out, err);
    return ss.str();
}

void sc_dump_error(const sc_error_t *err) {
    using namespace scopes;
    print_error(err);
}

void sc_set_signal_abort(bool value) {
    using namespace scopes;
    signal_abort = value;
}

void sc_abort() {
    using namespace scopes;
    f_abort();
}

void sc_exit(int c) {
    using namespace scopes;
    f_exit(c);
}

// Memoization
////////////////////////////////////////////////////////////////////////////////

namespace scopes {

struct MemoKeyEqual {
    bool operator()( Value *lhs, Value *rhs ) const {
        if (lhs == rhs) return true;
        if (lhs->kind() != rhs->kind())
            return false;
        if (isa<ArgumentList>(lhs)) {
            auto a = cast<ArgumentList>(lhs);
            auto b = cast<ArgumentList>(rhs);
            if (a->get_type() != b->get_type())
                return false;
            for (int i = 0; i < a->values.size(); ++i) {
                auto u = a->values[i];
                auto v = b->values[i];
                if (u == v) continue;
                if (u->kind() != v->kind())
                    return false;
                if (u.isa<Pure>()) {
                    if (!u.cast<Pure>()->key_equal(v.cast<Pure>().unref()))
                        return false;
                } else {
                    if (u != v)
                        return false;
                }
            }
            return true;
        } else if (isa<ArgumentListTemplate>(lhs)) {
            auto a = cast<ArgumentListTemplate>(lhs);
            auto b = cast<ArgumentListTemplate>(rhs);
            for (int i = 0; i < a->values.size(); ++i) {
                auto u = a->values[i];
                auto v = b->values[i];
                if (u == v) continue;
                if (u->kind() != v->kind())
                    return false;
                if (u.isa<Pure>()) {
                    if (!u.cast<Pure>()->key_equal(v.cast<Pure>().unref()))
                        return false;
                } else {
                    if (u != v)
                        return false;
                }
            }
            return true;
        } else if (isa<Pure>(lhs)) {
            if (isa<ConstPointer>(lhs)
                && cast<ConstPointer>(lhs)->get_type() == TYPE_List
                && cast<ConstPointer>(rhs)->get_type() == TYPE_List) {
                return sc_list_compare(
                    (const List *)cast<ConstPointer>(lhs)->value,
                    (const List *)cast<ConstPointer>(rhs)->value);
            }
            return cast<Pure>(lhs)->key_equal(cast<Pure>(rhs));
        } else {
            return false;
        }
    }
};

struct MemoHash {
    std::size_t operator()(Value *l) const {
        if (isa<ArgumentList>(l)) {
            auto alist = cast<ArgumentList>(l);
            uint64_t h = std::hash<const Type *>{}(alist->get_type());
            for (int i = 0; i < alist->values.size(); ++i) {
                auto x = alist->values[i];
                if (x.isa<Pure>()) {
                    h = hash2(h, x.cast<Pure>()->hash());
                } else {
                    h = hash2(h, std::hash<const TypedValue *>{}(x.unref()));
                }
            }
            return h;
        } else if (isa<ArgumentListTemplate>(l)) {
            auto alist = cast<ArgumentListTemplate>(l);
            uint64_t h = 0;
            for (int i = 0; i < alist->values.size(); ++i) {
                auto x = alist->values[i];
                if (x.isa<Pure>()) {
                    h = hash2(h, x.cast<Pure>()->hash());
                } else {
                    h = hash2(h, std::hash<const Value *>{}(x.unref()));
                }
            }
            return h;
        } else if (isa<Pure>(l)) {
            return cast<Pure>(l)->hash();
        } else {
            return std::hash<const Value *>{}(l);
        }
    }
};

typedef std::unordered_map<Value *, ValueRef, MemoHash, MemoKeyEqual> MemoMap;
static MemoMap memo_map;

}

sc_valueref_raises_t sc_map_get(sc_valueref_t key) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(ValueRef);
    auto it = memo_map.find(key.unref());
    if (it != memo_map.end()) {
        return convert_result(it->second);
    } else {
        SCOPES_C_ERROR(RTMissingKey);
    }
}

void sc_map_set(sc_valueref_t key, sc_valueref_t value) {
    using namespace scopes;
    if (!value) {
        auto it = memo_map.find(key.unref());
        if (it != memo_map.end()) {
            memo_map.erase(it);
        }
    } else {
        auto ret = memo_map.insert({key.unref(), value});
        if (!ret.second) {
            ret.first->second = value;
        }
    }
}

// Hashing
////////////////////////////////////////////////////////////////////////////////

uint64_t sc_hash (uint64_t data, size_t size) {
    using namespace scopes;
    return hash_bytes((const char *)&data, (size > 8)?8:size);
}

uint64_t sc_hash2x64(uint64_t a, uint64_t b) {
    using namespace scopes;
    return hash2(a, b);
}

uint64_t sc_hashbytes (const char *data, size_t size) {
    using namespace scopes;
    return hash_bytes(data, size);
}

// C Bridge
////////////////////////////////////////////////////////////////////////////////

sc_scope_raises_t sc_import_c(const sc_string_t *path,
    const sc_string_t *content, const sc_list_t *arglist) {
    using namespace scopes;
    std::vector<std::string> args;
    while (arglist) {
        auto value = extract_string_constant(arglist->at);
        if (!value.ok())
            return {false, nullptr};
        args.push_back(value.assert_ok()->data);
        arglist = arglist->next;
    }
    return convert_result(import_c_module(path->data, args, content->data));
}

sc_void_raises_t sc_load_library(const sc_string_t *name) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(void);
#ifdef SCOPES_WIN32
    // try to load library through regular interface first
    dlerror();
    void *handle = dlopen(name->data, RTLD_LAZY);
    if (!handle) {
        char *err = dlerror();
        SCOPES_C_ERROR(RTLoadLibraryFailed, name, strdup(err));
    }
#endif
    if (LLVMLoadLibraryPermanently(name->data)) {
        SCOPES_C_ERROR(RTLoadLibraryFailed, name, "reason unknown");
    }
    return convert_result({});
}

// Scope
////////////////////////////////////////////////////////////////////////////////

void sc_scope_set_symbol(sc_scope_t *scope, sc_symbol_t sym, sc_valueref_t value) {
    using namespace scopes;
    // ignore null values; this can happen when such a value is explicitly
    // created on the console
    if (!value) return;
    scope->bind(sym, value);
}

sc_valueref_raises_t sc_scope_at(sc_scope_t *scope, sc_symbol_t key) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(ValueRef);
    ValueRef result;
    bool ok = scope->lookup(key, result);
    if (!ok) {
        SCOPES_C_ERROR(RTMissingScopeAttribute, key);
    }
    return convert_result(result);
}

sc_valueref_raises_t sc_scope_local_at(sc_scope_t *scope, sc_symbol_t key) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(ValueRef);
    ValueRef result;
    bool ok = scope->lookup_local(key, result);
    if (!ok) {
        SCOPES_C_ERROR(RTMissingLocalScopeAttribute, key);
    }
    return convert_result(result);
}

const sc_string_t *sc_scope_get_docstring(sc_scope_t *scope, sc_symbol_t key) {
    using namespace scopes;
    if (key == SYM_Unnamed) {
        if (scope->doc) return scope->doc;
    } else {
        ScopeEntry entry;
        if (scope->lookup(key, entry) && entry.doc) {
            return entry.doc;
        }
    }
    return Symbol(SYM_Unnamed).name();
}

void sc_scope_set_docstring(sc_scope_t *scope, sc_symbol_t key, const sc_string_t *str) {
    using namespace scopes;
    if (key == SYM_Unnamed) {
        if (str && !str->count)
            str = nullptr;
        scope->doc = str;
    } else {
        ScopeEntry entry;
        if (!scope->lookup_local(key, entry)) {
            return;
            /*
            location_error(
                String::from("attempting to set a docstring for a non-local name"));
            */
        }
        entry.doc = str;
        scope->bind_with_doc(key, entry);
    }
}

sc_scope_t *sc_scope_new() {
    using namespace scopes;
    return Scope::from();
}

sc_scope_t *sc_scope_clone(sc_scope_t *clone) {
    using namespace scopes;
    return Scope::from(nullptr, clone);
}

sc_scope_t *sc_scope_new_subscope(sc_scope_t *scope) {
    using namespace scopes;
    return Scope::from(scope);
}

sc_scope_t *sc_scope_clone_subscope(sc_scope_t *scope, sc_scope_t *clone) {
    using namespace scopes;
    return Scope::from(scope, clone);
}

sc_scope_t *sc_scope_get_parent(sc_scope_t *scope) {
    using namespace scopes;
    return scope->parent;
}

void sc_scope_del_symbol(sc_scope_t *scope, sc_symbol_t sym) {
    using namespace scopes;
    scope->del(sym);
}

sc_symbol_valueref_tuple_t sc_scope_next(sc_scope_t *scope, sc_symbol_t key) {
    using namespace scopes;
    auto &&map = *scope->map;
    Scope::Map::const_iterator it;
    if (key == SYM_Unnamed) {
        it = map.begin();
    } else {
        it = map.find(key);
        if (it != map.end()) it++;
    }
    while (it != map.end()) {
        if (it->second.expr) {
            return { it->first, it->second.expr };
        }
        it++;
    }
    return { SYM_Unnamed, ValueRef() };
}

// Symbol
////////////////////////////////////////////////////////////////////////////////

sc_symbol_t sc_symbol_new(const sc_string_t *str) {
    using namespace scopes;
    return Symbol(str);
}

bool sc_symbol_is_variadic(sc_symbol_t sym) {
    using namespace scopes;
    return ends_with_parenthesis(sym);
}

const sc_string_t *sc_symbol_to_string(sc_symbol_t sym) {
    using namespace scopes;
    return sym.name();
}

size_t counter = 1;
sc_symbol_t sc_symbol_new_unique(const sc_string_t *str) {
    using namespace scopes;
    std::stringstream ss;
    ss << "#" << counter++ << "#" << str->data;
    return Symbol(String::from_stdstring(ss.str()));
}

// String
////////////////////////////////////////////////////////////////////////////////

const sc_string_t *sc_string_new(const char *ptr, size_t count) {
    using namespace scopes;
    return String::from(ptr, count);
}

const sc_string_t *sc_string_new_from_cstr(const char *ptr) {
    using namespace scopes;
    return String::from(ptr, strlen(ptr));
}

const sc_string_t *sc_string_join(const sc_string_t *a, const sc_string_t *b) {
    using namespace scopes;
    return String::join(a,b);
}

namespace scopes {
    static std::unordered_map<const String *, regexp::Reprog *> pattern_cache;
}
sc_bool_raises_t sc_string_match(const sc_string_t *pattern, const sc_string_t *text) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(bool);
    auto it = pattern_cache.find(pattern);
    regexp::Reprog *m = nullptr;
    if (it == pattern_cache.end()) {
        const char *error = nullptr;
        m = regexp::regcomp(pattern->data, 0, &error);
        if (error) {
            const String *err = String::from_cstr(error);
            regexp::regfree(m);
            SCOPES_C_ERROR(RTRegExError, err);
        }
        pattern_cache.insert({ pattern, m });
    } else {
        m = it->second;
    }
    SCOPES_C_RETURN(regexp::regexec(m, text->data, nullptr, 0) == 0);
}

size_t sc_string_count(const sc_string_t *str) {
    using namespace scopes;
    return str->count;
}

int sc_string_compare(const sc_string_t *a, const sc_string_t *b) {
    using namespace scopes;
    auto c = memcmp(a->data, b->data, std::min(a->count, b->count));
    if (c) return c;
    if (a->count < b->count) return -1;
    else if (a->count > b->count) return 1;
    return 0;
}

sc_rawstring_size_t_tuple_t sc_string_buffer(const sc_string_t *str) {
    using namespace scopes;
    return {str->data, str->count};
}

const sc_string_t *sc_string_rslice(const sc_string_t *str, size_t offset) {
    using namespace scopes;
    if (!offset) return str;
    if (offset >= str->count)
        return Symbol(SYM_Unnamed).name();
    return String::from(str->data + offset, str->count - offset);
}

const sc_string_t *sc_string_lslice(const sc_string_t *str, size_t offset) {
    using namespace scopes;
    if (!offset) return Symbol(SYM_Unnamed).name();
    if (offset >= str->count) return str;
    return String::from(str->data, offset);
}

// List
////////////////////////////////////////////////////////////////////////////////

const sc_list_t *sc_list_cons(sc_valueref_t at, const sc_list_t *next) {
    using namespace scopes;
    return List::from(at, next);
}

const sc_list_t *sc_list_join(const sc_list_t *a, const sc_list_t *b) {
    using namespace scopes;
    return List::join(a, b);
}

const sc_list_t *sc_list_dump(const sc_list_t *l) {
    using namespace scopes;
    StyledStream ss(SCOPES_CERR);
    stream_list(ss, l);
    return l;
}

const sc_string_t *sc_list_repr(const sc_list_t *l) {
    using namespace scopes;
    StyledString ss;
    stream_list(ss.out, l, StreamListFormat::singleline());
    return ss.str();
}

sc_valueref_list_tuple_t sc_list_decons(const sc_list_t *l) {
    using namespace scopes;
    if (l)
        return { l->at, l->next };
    else
        return { g_none, nullptr };
}

int sc_list_count(const sc_list_t *l) {
    using namespace scopes;
    return l?l->count:0;
}

sc_valueref_t sc_list_at(const sc_list_t *l) {
    using namespace scopes;
    return l?l->at:ValueRef(g_none);
}

const sc_list_t *sc_list_next(const sc_list_t *l) {
    using namespace scopes;
    return l?l->next:EOL;
}

const sc_list_t *sc_list_reverse(const sc_list_t *l) {
    using namespace scopes;
    return reverse_list(l);
}

bool sc_list_compare(const sc_list_t *a, const sc_list_t *b) {
    using namespace scopes;
    if (a == b)
        return true;
    if (a->count != b->count)
        return false;
    while (a) {
        assert(a); assert(b);
        if (!sc_value_compare(a->at, b->at))
            return false;
        a = a->next;
        b = b->next;
    }
    return true;
}

// Closure
////////////////////////////////////////////////////////////////////////////////

const sc_string_t *sc_closure_get_docstring(const sc_closure_t *func) {
    using namespace scopes;
    assert(func);
    return func->func->docstring;
}

sc_valueref_t sc_closure_get_template(const sc_closure_t *func) {
    using namespace scopes;
    assert(func);
    return func->func;
}

sc_valueref_t sc_closure_get_context(const sc_closure_t *func) {
    using namespace scopes;
    assert(func);
    return func->frame;
}

// Value
////////////////////////////////////////////////////////////////////////////////

const sc_string_t *sc_value_repr (sc_valueref_t value) {
    using namespace scopes;
    StyledString ss;
    stream_value(ss.out, value, StreamValueFormat::singleline());
    return ss.str();
}

const sc_string_t *sc_value_content_repr (sc_valueref_t value) {
    using namespace scopes;
    StyledString ss;
    stream_value(ss.out, value, StreamValueFormat::content());
    return ss.str();
}

const sc_string_t *sc_value_ast_repr (sc_valueref_t value) {
    using namespace scopes;
    StyledString ss;
    stream_value(ss.out, value);
    return ss.str();
}

const sc_string_t *sc_value_tostring (sc_valueref_t value) {
    using namespace scopes;
    StyledString ss = StyledString::plain();
    stream_value(ss.out, value, StreamValueFormat::content());
    return ss.str();
}

const sc_type_t *sc_value_type (sc_valueref_t value) {
    using namespace scopes;
    if (!value.isa<TypedValue>())
        return TYPE_Unknown;
    return strip_qualifiers(value.cast<TypedValue>()->get_type());
}

const sc_type_t *sc_value_qualified_type (sc_valueref_t value) {
    using namespace scopes;
    if (!value.isa<TypedValue>())
        return TYPE_Unknown;
    return value.cast<TypedValue>()->get_type();
}

const sc_anchor_t *sc_value_anchor (sc_valueref_t value) {
    using namespace scopes;
    return value.anchor();
}

sc_valueref_t sc_valueref_tag(sc_anchor_t *anchor, sc_valueref_t value) {
    using namespace scopes;
    //set_best_anchor(value, anchor);
    return ref(anchor, value);
}

bool sc_value_is_constant(sc_valueref_t value) {
    using namespace scopes;
    return value.isa<Const>();
}

bool sc_value_is_pure (sc_valueref_t value) {
    using namespace scopes;
    return value.isa<Pure>();
}

bool sc_value_compare (sc_valueref_t a, sc_valueref_t b) {
    using namespace scopes;
    if (a == b) return true;
    if (!a || !b) return false;
    return MemoKeyEqual{}(a.unref(),b.unref());
}

int sc_value_kind (sc_valueref_t value) {
    using namespace scopes;
    return value->kind();
}

sc_valueref_t sc_value_wrap(const sc_type_t *type, sc_valueref_t value) {
    using namespace scopes;
    if (value.isa<Const>()) {
        return value;
    }
    auto result = wrap_value(type, value);
    assert(result);
    return result;
}

sc_valueref_t sc_value_unwrap(const sc_type_t *type, sc_valueref_t value) {
    using namespace scopes;
    auto result = unwrap_value(type, value);
    if (!result) {
        result = ref(value.anchor(), ArgumentList::from({}));
    }
    return result;
}

sc_valueref_t sc_keyed_new(sc_symbol_t key, sc_valueref_t value) {
    using namespace scopes;
    return KeyedTemplate::from(key, value);
}

sc_valueref_t sc_empty_argument_list() {
    using namespace scopes;
    return ArgumentList::from({});
}

sc_valueref_t sc_argument_list_new() {
    using namespace scopes;
    return ArgumentListTemplate::empty_from();
}

void sc_argument_list_append(sc_valueref_t alist, sc_valueref_t value) {
    using namespace scopes;
    //SCOPES_RESULT_TYPE(void);
    //SCOPES_C_CHECK_RESULT(verify_kind<VK_ArgumentListTemplate>(alist));
    alist.cast<ArgumentListTemplate>()->append(value);
    //return convert_result({});
}

int sc_argcount(sc_valueref_t value) {
    using namespace scopes;
    switch(value->kind()) {
    case VK_ArgumentList: {
        return (int)value.cast<ArgumentList>()->values.size();
    } break;
    case VK_ArgumentListTemplate: {
        return (int)value.cast<ArgumentListTemplate>()->values.size();
    } break;
    default: return 1;
    }
}

sc_valueref_t sc_getarg(sc_valueref_t value, int index) {
    using namespace scopes;
    switch(value->kind()) {
    case VK_ArgumentList: {
        auto al = value.cast<ArgumentList>();
        if (index < al->values.size()) {
            return al->values[index];
        }
    } break;
    case VK_ArgumentListTemplate: {
        auto al = value.cast<ArgumentListTemplate>();
        if (index < al->values.size()) {
            return al->values[index];
        }
    } break;
    default: {
        if (index == 0) return value;
    } break;
    }
    return ref(value.anchor(), ConstAggregate::none_from());
}

sc_valueref_t sc_getarglist(sc_valueref_t value, int index) {
    using namespace scopes;
    switch(value->kind()) {
    case VK_ArgumentList: {
        auto al = value.cast<ArgumentList>();
        TypedValues values;
        for (int i = index; i < al->values.size(); ++i) {
            values.push_back(al->values[i]);
        }
        return ref(value.anchor(), ArgumentList::from(values));
    } break;
    case VK_ArgumentListTemplate: {
        auto al = value.cast<ArgumentListTemplate>();
        Values values;
        for (int i = index; i < al->values.size(); ++i) {
            values.push_back(al->values[i]);
        }
        return ref(value.anchor(), ArgumentListTemplate::from(values));
    } break;
    default: {
        if (index == 0) {
            return value;
        } else {
            return ref(value.anchor(), ArgumentList::from({}));
        }
    } break;
    }
}

sc_valueref_t sc_extract_argument_new(sc_valueref_t value, int index) {
    using namespace scopes;
    return ExtractArgumentTemplate::from(value, index);
}

sc_valueref_t sc_extract_argument_list_new(sc_valueref_t value, int index) {
    using namespace scopes;
    return ExtractArgumentTemplate::from(value, index, true);
}

sc_valueref_t sc_template_new(sc_symbol_t name) {
    using namespace scopes;
    return Template::from(name);
}
void sc_template_set_name(sc_valueref_t fn, sc_symbol_t name) {
    using namespace scopes;
    fn.cast<Template>()->name = name;
}
sc_symbol_t sc_template_get_name(sc_valueref_t fn) {
    using namespace scopes;
    return fn.cast<Template>()->name;
}
void sc_template_append_parameter(sc_valueref_t fn, sc_valueref_t symbol) {
    using namespace scopes;
    fn.cast<Template>()->append_param(symbol.cast<ParameterTemplate>());
}
void sc_template_set_body(sc_valueref_t fn, sc_valueref_t value) {
    using namespace scopes;
    fn.cast<Template>()->value = value;
}

void sc_template_set_inline(sc_valueref_t fn) {
    using namespace scopes;
    fn.cast<Template>()->set_inline();
}

sc_valueref_t sc_expression_new() {
    using namespace scopes;
    return Expression::unscoped_from();
}

void sc_expression_set_scoped(sc_valueref_t expr) {
    using namespace scopes;
    expr.cast<Expression>()->scoped = true;
}

void sc_expression_append(sc_valueref_t expr, sc_valueref_t value) {
    using namespace scopes;
    expr.cast<Expression>()->append(value);
}

sc_valueref_t sc_global_new(sc_symbol_t name, const sc_type_t *type,
    uint32_t flags, sc_symbol_t storage_class, int location, int binding) {
    using namespace scopes;
    return Global::from(type, name, flags, storage_class, location, binding);
}

sc_valueref_t sc_if_new() {
    using namespace scopes;
    return If::from();
}
void sc_if_append_then_clause(sc_valueref_t value, sc_valueref_t cond, sc_valueref_t body) {
    using namespace scopes;
    value.cast<If>()->append_then(cond, body);
}
void sc_if_append_else_clause(sc_valueref_t value, sc_valueref_t body) {
    using namespace scopes;
    value.cast<If>()->append_else(body);
}

sc_valueref_t sc_switch_new(sc_valueref_t expr) {
    using namespace scopes;
    return SwitchTemplate::from(expr);
}
void sc_switch_append_case(sc_valueref_t value, sc_valueref_t literal, sc_valueref_t body) {
    using namespace scopes;
    value.cast<SwitchTemplate>()->append_case(literal, body);
}
void sc_switch_append_pass(sc_valueref_t value, sc_valueref_t literal, sc_valueref_t body) {
    using namespace scopes;
    value.cast<SwitchTemplate>()->append_pass(literal, body);
}
void sc_switch_append_default(sc_valueref_t value, sc_valueref_t body) {
    using namespace scopes;
    value.cast<SwitchTemplate>()->append_default(body);
}

sc_valueref_t sc_parameter_new(sc_symbol_t name) {
    using namespace scopes;
    if (ends_with_parenthesis(name)) {
        return ParameterTemplate::variadic_from(name);
    } else {
        return ParameterTemplate::from(name);
    }
}

bool sc_parameter_is_variadic(sc_valueref_t param) {
    using namespace scopes;
    return param.cast<ParameterTemplate>()->is_variadic();
}

sc_valueref_t sc_call_new(sc_valueref_t callee) {
    using namespace scopes;
    return CallTemplate::from(callee);
}

void sc_call_append_argument(sc_valueref_t call, sc_valueref_t value) {
    using namespace scopes;
    call.cast<CallTemplate>()->args.push_back(value);
}

bool sc_call_is_rawcall(sc_valueref_t value) {
    using namespace scopes;
    return value.cast<CallTemplate>()->is_rawcall();
}

void sc_call_set_rawcall(sc_valueref_t value, bool enable) {
    using namespace scopes;
    value.cast<CallTemplate>()->set_rawcall();
}

sc_valueref_t sc_loop_new(sc_valueref_t init) {
    using namespace scopes;
    return Loop::from(init);
}

sc_valueref_t sc_loop_arguments(sc_valueref_t loop) {
    using namespace scopes;
    return loop.cast<Loop>()->args;
}

void sc_loop_set_body(sc_valueref_t loop, sc_valueref_t body) {
    using namespace scopes;
    loop.cast<Loop>()->value = body;
}

sc_valueref_t sc_const_int_new(const sc_type_t *type, uint64_t value) {
    using namespace scopes;
    return ConstInt::from(type, value);
}
sc_valueref_t sc_const_real_new(const sc_type_t *type, double value) {
    using namespace scopes;
    return ConstReal::from(type, value);
}
sc_valueref_t sc_const_aggregate_new(const sc_type_t *type, int numconsts, sc_valueref_t *consts) {
    using namespace scopes;
    ConstantPtrs vals;
    init_values_arrayT(vals, numconsts, consts);
    return ConstAggregate::from(type, vals);
}
sc_valueref_t sc_const_pointer_new(const sc_type_t *type, const void *pointer) {
    using namespace scopes;
    return ConstPointer::from(type, pointer);
}
uint64_t sc_const_int_extract(const sc_valueref_t value) {
    using namespace scopes;
    return value.cast<ConstInt>()->value;
}
double sc_const_real_extract(const sc_valueref_t value) {
    using namespace scopes;
    return value.cast<ConstReal>()->value;
}
sc_valueref_t sc_const_extract_at(const sc_valueref_t value, int index) {
    using namespace scopes;
    auto val = value.cast<ConstAggregate>();
    assert(index < val->values.size());
    return get_field(val, index);
}
const void *sc_const_pointer_extract(const sc_valueref_t value) {
    using namespace scopes;
    return value.cast<ConstPointer>()->value;
}

sc_valueref_t sc_break_new(sc_valueref_t value) {
    using namespace scopes;
    return Break::from(value);
}
sc_valueref_t sc_repeat_new(sc_valueref_t value) {
    using namespace scopes;
    return RepeatTemplate::from(value);
}
sc_valueref_t sc_return_new(sc_valueref_t value) {
    using namespace scopes;
    return ReturnTemplate::from(value);
}
sc_valueref_t sc_raise_new(sc_valueref_t value) {
    using namespace scopes;
    return RaiseTemplate::from(value);
}

sc_valueref_t sc_quote_new(sc_valueref_t value) {
    using namespace scopes;
    return Quote::from(value);
}

sc_valueref_t sc_unquote_new(sc_valueref_t value) {
    using namespace scopes;
    return Unquote::from(value);
}

sc_valueref_t sc_label_new(int kind, sc_symbol_t name) {
    using namespace scopes;
    return LabelTemplate::from((LabelKind)kind, name);
}
void sc_label_set_body(sc_valueref_t label, sc_valueref_t body) {
    using namespace scopes;
    label.cast<LabelTemplate>()->value = body;
}
sc_valueref_t sc_merge_new(sc_valueref_t label, sc_valueref_t value) {
    using namespace scopes;
    return MergeTemplate::from(label.cast<LabelTemplate>(), value);
}

// Parser
////////////////////////////////////////////////////////////////////////////////

sc_valueref_raises_t sc_parse_from_path(const sc_string_t *path) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(ValueRef);
    auto sf = SourceFile::from_file(path);
    if (!sf) {
        SCOPES_C_ERROR(RTUnableToOpenFile, path);
    }
    LexerParser parser(sf);
    return convert_result(parser.parse());
}

sc_valueref_raises_t sc_parse_from_string(const sc_string_t *str) {
    using namespace scopes;
    auto sf = SourceFile::from_string(Symbol("<string>"), str);
    assert(sf);
    LexerParser parser(sf);
    return convert_result(parser.parse());
}

// Types
////////////////////////////////////////////////////////////////////////////////

sc_valueref_raises_t sc_type_at(const sc_type_t *T, sc_symbol_t key) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(ValueRef);
    T = strip_qualifiers(T);
    ValueRef result;
    bool ok = T->lookup(key, result);
    if (!ok) {
        SCOPES_C_ERROR(RTMissingTypeAttribute, key, T);
    }
    return convert_result(result);
}

sc_valueref_raises_t sc_type_local_at(const sc_type_t *T, sc_symbol_t key) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(ValueRef);
    T = strip_qualifiers(T);
    ValueRef result;
    bool ok = T->lookup_local(key, result);
    if (!ok) {
        SCOPES_C_ERROR(RTMissingLocalTypeAttribute, key);
    }
    return convert_result(result);
}

sc_size_raises_t sc_type_sizeof(const sc_type_t *T) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(size_t);
    SCOPES_C_RETURN(size_of(T));
}

sc_size_raises_t sc_type_alignof(const sc_type_t *T) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(size_t);
    SCOPES_C_RETURN(align_of(T));
}

sc_int_raises_t sc_type_countof(const sc_type_t *T) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(int);
    T = SCOPES_C_GET_RESULT(storage_type(T));
    switch(T->kind()) {
    case TK_Pointer:
    case TK_Image:
    case TK_SampledImage:
        return { true, nullptr, 1 };
    case TK_Array: return { true, nullptr, (int)cast<ArrayType>(T)->count };
    case TK_Vector: return { true, nullptr, (int)cast<VectorType>(T)->count };
    case TK_Tuple: return { true, nullptr, (int)cast<TupleType>(T)->values.size() };
    case TK_Union: return { true, nullptr, (int)cast<UnionType>(T)->values.size() };
    case TK_Function:  return { true, nullptr, (int)(cast<FunctionType>(T)->argument_types.size()) };
    default: break;
    }
    SCOPES_C_ERROR(RTUncountableStorageType, T);
}

sc_type_raises_t sc_type_element_at(const sc_type_t *T, int i) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(const Type *);
    T = SCOPES_C_GET_RESULT(storage_type(T));
    const Type *result = nullptr;
    switch(T->kind()) {
    case TK_Pointer: result = cast<PointerType>(T)->element_type; break;
    case TK_Array: result = cast<ArrayType>(T)->element_type; break;
    case TK_Vector: result = cast<VectorType>(T)->element_type; break;
    case TK_Tuple: result = SCOPES_C_GET_RESULT(cast<TupleType>(T)->type_at_index(i)); break;
    case TK_Union: result = SCOPES_C_GET_RESULT(cast<UnionType>(T)->type_at_index(i)); break;
    case TK_Function: result = SCOPES_C_GET_RESULT(cast<FunctionType>(T)->type_at_index(i)); break;
    case TK_Image: result = cast<ImageType>(T)->type; break;
    case TK_SampledImage: result = cast<SampledImageType>(T)->type; break;
    default: {
        SCOPES_C_ERROR(RTNoElementsInStorageType, T);
    } break;
    }
    SCOPES_C_RETURN(result);
}

sc_int_raises_t sc_type_field_index(const sc_type_t *T, sc_symbol_t name) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(int);
    T = SCOPES_C_GET_RESULT(storage_type(T));
    switch(T->kind()) {
    case TK_Tuple: return { true, nullptr, (int)cast<TupleType>(T)->field_index(name) };
    case TK_Union: return { true, nullptr, (int)cast<UnionType>(T)->field_index(name) };
    default: break;
    }
    SCOPES_C_ERROR(RTNoNamedElementsInStorageType, T);
}

sc_symbol_raises_t sc_type_field_name(const sc_type_t *T, int index) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(Symbol);
    T = SCOPES_C_GET_RESULT(storage_type(T));
    Symbol symbol;
    switch(T->kind()) {
    case TK_Tuple: return convert_result(cast<TupleType>(T)->field_name(index));
    case TK_Union: return convert_result(cast<UnionType>(T)->field_name(index));
    default: break;
    }
    SCOPES_C_ERROR(RTNoNamedElementsInStorageType, T);
}

int32_t sc_type_kind(const sc_type_t *T) {
    using namespace scopes;
    T = strip_qualifiers(T);
    return T->kind();
}

void sc_type_debug_abi(const sc_type_t *T) {
    using namespace scopes;
    ABIClass classes[MAX_ABI_CLASSES];
    size_t sz = abi_classify(T, classes);
    StyledStream ss(SCOPES_COUT);
    ss << T << " -> " << sz;
    for (size_t i = 0; i < sz; ++i) {
        ss << " " << abi_class_to_string(classes[i]);
    }
    ss << std::endl;
}

sc_type_raises_t sc_type_storage(const sc_type_t *T) {
    using namespace scopes;
    return convert_result(storage_type(T));
}

bool sc_type_is_opaque(const sc_type_t *T) {
    using namespace scopes;
    return is_opaque(T);
}

bool sc_type_is_superof(const sc_type_t *super, const sc_type_t *T) {
    using namespace scopes;
    for (int i = 0; i < SCOPES_MAX_RECURSIONS; ++i) {
        T = sc_typename_type_get_super(T);
        if (T == super) return true;
        if (T == TYPE_Typename) return false;
    }
    return false;
}

bool sc_type_is_default_suffix(const sc_type_t *T) {
    using namespace scopes;
    return is_default_suffix(T);
}

const sc_string_t *sc_type_string(const sc_type_t *T) {
    using namespace scopes;
    StyledString ss = StyledString::plain();
    stream_type_name(ss.out, T);
    return ss.str();
}

sc_symbol_valueref_tuple_t sc_type_next(const sc_type_t *type, sc_symbol_t key) {
    using namespace scopes;
    type = strip_qualifiers(type);
    auto &&map = type->get_symbols();
    Type::Map::const_iterator it;
    if (key == SYM_Unnamed) {
        it = map.begin();
    } else {
        it = map.find(key);
        if (it != map.end()) it++;
    }
    if (it != map.end()) {
        return { it->first, it->second };
    }
    return { SYM_Unnamed, ValueRef() };
}

void sc_type_set_symbol(const sc_type_t *T, sc_symbol_t sym, sc_valueref_t value) {
    using namespace scopes;
    T = strip_qualifiers(T);
    const_cast<Type *>(T)->bind(sym, value);
}

// Qualifier
////////////////////////////////////////////////////////////////////////////////

sc_symbol_type_tuple_t sc_type_key(const sc_type_t *T) {
    using namespace scopes;
    return type_key(T);
}

const sc_type_t *sc_key_type(sc_symbol_t name, const sc_type_t *T) {
    using namespace scopes;
    return key_type(name, T);
}

bool sc_type_is_refer(const sc_type_t *T) {
    using namespace scopes;
    return has_qualifier<ReferQualifier>(T);
}

// Pointer Type
////////////////////////////////////////////////////////////////////////////////

const sc_type_t *sc_pointer_type(const sc_type_t *T, uint64_t flags, sc_symbol_t storage_class) {
    using namespace scopes;
    return pointer_type(T, flags, storage_class);
}

uint64_t sc_pointer_type_get_flags(const sc_type_t *T) {
    using namespace scopes;
    if (is_kind<TK_Pointer>(T)) {
        return cast<PointerType>(T)->flags;
    }
    return 0;
}

const sc_type_t *sc_pointer_type_set_flags(const sc_type_t *T, uint64_t flags) {
    using namespace scopes;
    if (is_kind<TK_Pointer>(T)) {
        auto pt = cast<PointerType>(T);
        return pointer_type(pt->element_type, flags, pt->storage_class);
    }
    return T;
}

sc_symbol_t sc_pointer_type_get_storage_class(const sc_type_t *T) {
    using namespace scopes;
    if (is_kind<TK_Pointer>(T)) {
        return cast<PointerType>(T)->storage_class;
    }
    return SYM_Unnamed;
}

const sc_type_t *sc_pointer_type_set_storage_class(const sc_type_t *T, sc_symbol_t storage_class) {
    using namespace scopes;
    if (is_kind<TK_Pointer>(T)) {
        auto pt = cast<PointerType>(T);
        return pointer_type(pt->element_type, pt->flags, storage_class);
    }
    return T;
}

const sc_type_t *sc_pointer_type_set_element_type(const sc_type_t *T, const sc_type_t *ET) {
    using namespace scopes;
    if (is_kind<TK_Pointer>(T)) {
        auto pt = cast<PointerType>(T);
        return pointer_type(ET, pt->flags, pt->storage_class);
    }
    return T;
}

// numerical types
////////////////////////////////////////////////////////////////////////////////

int32_t sc_type_bitcountof(const sc_type_t *T) {
    using namespace scopes;
    auto rT = storage_type(T);
    if (!rT.ok()) return 0;
    T = rT.assert_ok();
    switch(T->kind()) {
    case TK_Integer:
        return cast<IntegerType>(T)->width;
    case TK_Real:
        return cast<RealType>(T)->width;
    default: break;
    }
    return 0;
}

// Integer Type
////////////////////////////////////////////////////////////////////////////////

const sc_type_t *sc_integer_type(int width, bool issigned) {
    using namespace scopes;
    return integer_type(width, issigned);
}

bool sc_integer_type_is_signed(const sc_type_t *T) {
    using namespace scopes;
    auto rT = storage_type(T);
    if (!rT.ok()) return false;
    T = rT.assert_ok();
    if (is_kind<TK_Integer>(T)) {
        return cast<IntegerType>(T)->issigned;
    }
    return false;
}

// Typename Type
////////////////////////////////////////////////////////////////////////////////

const sc_type_t *sc_typename_type(const sc_string_t *str) {
    using namespace scopes;
    return typename_type(str);
}

sc_void_raises_t sc_typename_type_set_super(const sc_type_t *T, const sc_type_t *ST) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(void);
    SCOPES_C_CHECK_RESULT(verify_kind<TK_Typename>(T));
    SCOPES_C_CHECK_RESULT(verify_kind<TK_Typename>(ST));
    // if T <=: ST, the operation is illegal
    const Type *S = ST;
    while (S) {
        if (S == T) {
            SCOPES_C_ERROR(RTIllegalSupertype, ST, T);
        }
        if (S == TYPE_Typename)
            break;
        S = superof(S);
    }
    auto tn = cast<TypenameType>(T);
    const_cast<TypenameType *>(tn)->super_type = ST;
    return convert_result({});
}

const sc_type_t *sc_typename_type_get_super(const sc_type_t *T) {
    using namespace scopes;
    return superof(T);
}

sc_void_raises_t sc_typename_type_set_storage(const sc_type_t *T, const sc_type_t *T2, uint32_t flags) {
    using namespace scopes;
    SCOPES_RESULT_TYPE(void);
    SCOPES_C_CHECK_RESULT(verify_kind<TK_Typename>(T));
    return convert_result(cast<TypenameType>(const_cast<Type *>(T))->finalize(T2, flags));
}

// Array Type
////////////////////////////////////////////////////////////////////////////////

sc_type_raises_t sc_array_type(const sc_type_t *element_type, size_t count) {
    using namespace scopes;
    return convert_result(array_type(element_type, count));
}

// Vector Type
////////////////////////////////////////////////////////////////////////////////

sc_type_raises_t sc_vector_type(const sc_type_t *element_type, size_t count) {
    using namespace scopes;
    return convert_result(vector_type(element_type, count));
}

// Tuple Type
////////////////////////////////////////////////////////////////////////////////

sc_type_raises_t sc_tuple_type(int numtypes, const sc_type_t **typeargs) {
    using namespace scopes;
    Types types;
    types.reserve(numtypes);
    for (int i = 0; i < numtypes; ++i) {
        types.push_back(typeargs[i]);
    }
    return convert_result(tuple_type(types));
}

// Union Type
////////////////////////////////////////////////////////////////////////////////

sc_type_raises_t sc_union_type(int numtypes, const sc_type_t **typeargs) {
    using namespace scopes;
    Types types;
    types.reserve(numtypes);
    for (int i = 0; i < numtypes; ++i) {
        types.push_back(typeargs[i]);
    }
    return convert_result(union_type(types));
}

// Arguments Type
////////////////////////////////////////////////////////////////////////////////

const sc_type_t *sc_arguments_type(int numtypes, const sc_type_t **typeargs) {
    using namespace scopes;
    Types types;
    types.reserve(numtypes);
    for (int i = 0; i < numtypes; ++i) {
        types.push_back(typeargs[i]);
    }
    return arguments_type(types);
}

const sc_type_t *sc_arguments_type_join(const sc_type_t *T1, const sc_type_t *T2) {
    using namespace scopes;
    Types types;
    if (isa<ArgumentsType>(T1)) {
        for (auto &&value : cast<ArgumentsType>(T1)->values) {
            types.push_back(value);
        }
    } else {
        types.push_back(T1);
    }
    if (isa<ArgumentsType>(T2)) {
        for (auto &&value : cast<ArgumentsType>(T2)->values) {
            types.push_back(value);
        }
    } else {
        types.push_back(T2);
    }
    return arguments_type(types);
}

int sc_arguments_type_argcount(sc_type_t *T) {
    using namespace scopes;
    return get_argument_count(T);
}

const sc_type_t *sc_arguments_type_getarg(sc_type_t *T, int index) {
    using namespace scopes;
    return get_argument(T, index);
}

// Unique Type
////////////////////////////////////////////////////////////////////////////////

const sc_type_t *sc_view_type(const sc_type_t *type, int id) {
    using namespace scopes;
    if (id < 0)
        return view_type(type, {});
    else
        return view_type(type, { id });
}

const sc_type_t *sc_unique_type(const sc_type_t *type, int id) {
    using namespace scopes;
    return unique_type(type, id);
}

const sc_type_t *sc_mutate_type(const sc_type_t *type) {
    using namespace scopes;
    return mutate_type(type);
}

const sc_type_t *sc_refer_type(const sc_type_t *type, uint64_t flags, sc_symbol_t storage_class) {
    using namespace scopes;
    return refer_type(type, flags, storage_class);
}

const sc_type_t *sc_strip_qualifiers(const sc_type_t *type) {
    using namespace scopes;
    return strip_qualifiers(type);
}

// Function Type
////////////////////////////////////////////////////////////////////////////////

bool sc_function_type_is_variadic(const sc_type_t *T) {
    using namespace scopes;
    if (is_kind<TK_Function>(T)) {
        auto ft = cast<FunctionType>(T);
        return ft->flags & FF_Variadic;
    }
    return false;
}

const sc_type_t *sc_function_type(const sc_type_t *return_type,
    int numtypes, const sc_type_t **typeargs) {
    using namespace scopes;
    Types types;
    types.reserve(numtypes);
    for (int i = 0; i < numtypes; ++i) {
        types.push_back(typeargs[i]);
    }
    return function_type(return_type, types);
}

const sc_type_t *sc_function_type_raising(const sc_type_t *T,
    const sc_type_t *except_type) {
    using namespace scopes;
    auto ft = cast<FunctionType>(T);
    return raising_function_type(except_type, ft->return_type,
        ft->argument_types, ft->flags);
}

sc_type_type_tuple_t sc_function_type_return_type(const sc_type_t *T) {
    using namespace scopes;
    auto val = dyn_cast<FunctionType>(T);
    if (val) {
        return { val->return_type, val->except_type };
    } else {
        return { TYPE_Unknown, TYPE_Unknown };
    }
}

// Image Type
////////////////////////////////////////////////////////////////////////////////

const sc_type_t *sc_image_type(
    const sc_type_t *_type,
    sc_symbol_t _dim,
    int _depth,
    int _arrayed,
    int _multisampled,
    int _sampled,
    sc_symbol_t _format,
    sc_symbol_t _access) {
    using namespace scopes;
    return image_type(_type, _dim, _depth, _arrayed, _multisampled, _sampled, _format, _access);
}

// Sampled Image Type
////////////////////////////////////////////////////////////////////////////////

const sc_type_t *sc_sampled_image_type(const sc_type_t *_type) {
    using namespace scopes;
    return sampled_image_type(cast<ImageType>(_type));
}


} // extern "C"

namespace scopes {

//------------------------------------------------------------------------------
// GLOBALS
//------------------------------------------------------------------------------

static void bind_extern(Symbol globalsym, Symbol externsym, const Type *T) {
    globals->bind(globalsym, ref(builtin_anchor(), Global::from(T, externsym, GF_NonWritable)));
}

static void bind_new_value(Symbol sym, const ValueRef &value) {
    globals->bind(sym, ref(builtin_anchor(), value));
}

static void bind_symbol(Symbol sym, Symbol value) {
    bind_new_value(sym, ConstInt::symbol_from(value));
}

static void bind_extern(Symbol sym, const Type *T) {
    bind_extern(sym, sym, T);
}

void init_globals(int argc, char *argv[]) {
    globals = original_globals;
    scopes_argc = argc;
    scopes_argv = argv;

#define DEFINE_EXTERN_C_FUNCTION(FUNC, RETTYPE, ...) \
    (void)FUNC; /* ensure that the symbol is there */ \
    bind_extern(Symbol(#FUNC), function_type(RETTYPE, { __VA_ARGS__ }));
#define DEFINE_RAISING_EXTERN_C_FUNCTION(FUNC, RETTYPE, ...) \
    (void)FUNC; /* ensure that the symbol is there */ \
    bind_extern(Symbol(#FUNC), raising_function_type(RETTYPE, { __VA_ARGS__ }));

    const Type *rawstring = native_ro_pointer_type(TYPE_I8);
    const Type *TYPE_ValuePP = native_ro_pointer_type(TYPE_ValueRef);
    const Type *_void = empty_arguments_type();
    const Type *voidstar = native_ro_pointer_type(_void);

    DEFINE_EXTERN_C_FUNCTION(sc_compiler_version, arguments_type({TYPE_I32, TYPE_I32, TYPE_I32}));
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_expand, arguments_type({TYPE_ValueRef, TYPE_List, TYPE_Scope}), TYPE_ValueRef, TYPE_List, TYPE_Scope);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_eval, TYPE_ValueRef, TYPE_Anchor, TYPE_List, TYPE_Scope);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_prove, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_eval_inline, TYPE_Anchor, TYPE_ValueRef, TYPE_List, TYPE_Scope);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_typify_template, TYPE_ValueRef, TYPE_ValueRef, TYPE_I32, native_ro_pointer_type(TYPE_Type));
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_typify, TYPE_ValueRef, TYPE_Closure, TYPE_I32, native_ro_pointer_type(TYPE_Type));
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_compile, TYPE_ValueRef, TYPE_ValueRef, TYPE_U64);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_compile_spirv, TYPE_String, TYPE_Symbol, TYPE_ValueRef, TYPE_U64);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_compile_glsl, TYPE_String, TYPE_Symbol, TYPE_ValueRef, TYPE_U64);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_compile_object, _void, TYPE_String, TYPE_Scope, TYPE_U64);
    DEFINE_EXTERN_C_FUNCTION(sc_enter_solver_cli, _void);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_verify_stack, TYPE_USize);
    DEFINE_EXTERN_C_FUNCTION(sc_launch_args, arguments_type({TYPE_I32,native_ro_pointer_type(rawstring)}));

    DEFINE_EXTERN_C_FUNCTION(sc_prompt, arguments_type({TYPE_Bool, TYPE_String}), TYPE_String, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_set_autocomplete_scope, _void, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_default_styler, TYPE_String, TYPE_Symbol, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_format_message, TYPE_String, TYPE_Anchor, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_write, _void, TYPE_String);

    DEFINE_EXTERN_C_FUNCTION(sc_value_repr, TYPE_String, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_value_content_repr, TYPE_String, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_value_ast_repr, TYPE_String, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_value_tostring, TYPE_String, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_value_type, TYPE_Type, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_value_qualified_type, TYPE_Type, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_value_anchor, TYPE_Anchor, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_valueref_tag, TYPE_ValueRef, TYPE_Anchor, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_value_is_constant, TYPE_Bool, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_value_is_pure, TYPE_Bool, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_value_compare, TYPE_Bool, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_value_kind, TYPE_I32, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_value_wrap, TYPE_ValueRef, TYPE_Type, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_value_unwrap, TYPE_ValueRef, TYPE_Type, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_keyed_new, TYPE_ValueRef, TYPE_Symbol, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_empty_argument_list, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_argument_list_new, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_argument_list_append, _void, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_extract_argument_new, TYPE_ValueRef, TYPE_ValueRef, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_extract_argument_list_new, TYPE_ValueRef, TYPE_ValueRef, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_argcount, TYPE_I32, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_getarg, TYPE_ValueRef, TYPE_ValueRef, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_getarglist, TYPE_ValueRef, TYPE_ValueRef, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_template_new, TYPE_ValueRef, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_template_set_name, _void, TYPE_ValueRef, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_template_get_name, TYPE_Symbol, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_template_append_parameter, _void, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_template_set_body, _void, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_template_set_inline, _void, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_expression_new, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_expression_set_scoped, _void, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_expression_append, _void, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_global_new, TYPE_ValueRef, TYPE_Symbol, TYPE_Type,
        TYPE_U32, TYPE_Symbol, TYPE_I32, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_if_new, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_if_append_then_clause, _void, TYPE_ValueRef, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_if_append_else_clause, _void, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_switch_new, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_switch_append_case, _void, TYPE_ValueRef, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_switch_append_pass, _void, TYPE_ValueRef, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_switch_append_default, _void, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_parameter_new, TYPE_ValueRef, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_parameter_is_variadic, TYPE_Bool, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_call_new, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_call_append_argument, _void, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_call_is_rawcall, TYPE_Bool, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_call_set_rawcall, _void, TYPE_ValueRef, TYPE_Bool);
    DEFINE_EXTERN_C_FUNCTION(sc_loop_new, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_loop_arguments, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_loop_set_body, _void, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_const_int_new, TYPE_ValueRef, TYPE_Type, TYPE_U64);
    DEFINE_EXTERN_C_FUNCTION(sc_const_real_new, TYPE_ValueRef, TYPE_Type, TYPE_F64);
    DEFINE_EXTERN_C_FUNCTION(sc_const_aggregate_new, TYPE_ValueRef, TYPE_Type, TYPE_I32, TYPE_ValuePP);
    DEFINE_EXTERN_C_FUNCTION(sc_const_pointer_new, TYPE_ValueRef, TYPE_Type, voidstar);
    DEFINE_EXTERN_C_FUNCTION(sc_const_int_extract, TYPE_U64, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_const_real_extract, TYPE_F64, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_const_extract_at, TYPE_ValueRef, TYPE_ValueRef, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_const_pointer_extract, voidstar, TYPE_ValueRef);

    DEFINE_EXTERN_C_FUNCTION(sc_break_new, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_repeat_new, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_return_new, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_raise_new, TYPE_ValueRef, TYPE_ValueRef);

    DEFINE_EXTERN_C_FUNCTION(sc_quote_new, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_unquote_new, TYPE_ValueRef, TYPE_ValueRef);

    DEFINE_EXTERN_C_FUNCTION(sc_label_new, TYPE_ValueRef, TYPE_I32, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_label_set_body, _void, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_merge_new, TYPE_ValueRef, TYPE_ValueRef, TYPE_ValueRef);

    DEFINE_EXTERN_C_FUNCTION(sc_is_file, TYPE_Bool, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_is_directory, TYPE_Bool, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_realpath, TYPE_String, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_dirname, TYPE_String, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_basename, TYPE_String, TYPE_String);

    DEFINE_EXTERN_C_FUNCTION(sc_get_globals, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_get_original_globals, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_set_globals, _void, TYPE_Scope);

    DEFINE_EXTERN_C_FUNCTION(sc_error_append_calltrace, _void, TYPE_Error, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_error_new, TYPE_Error, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_format_error, TYPE_String, TYPE_Error);
    DEFINE_EXTERN_C_FUNCTION(sc_dump_error, _void, TYPE_Error);

    DEFINE_EXTERN_C_FUNCTION(sc_abort, TYPE_NoReturn);
    DEFINE_EXTERN_C_FUNCTION(sc_exit, TYPE_NoReturn, TYPE_I32);

    DEFINE_EXTERN_C_FUNCTION(sc_set_signal_abort,
        _void, TYPE_Bool);

    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_map_get, TYPE_ValueRef, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_map_set, _void, TYPE_ValueRef, TYPE_ValueRef);

    DEFINE_EXTERN_C_FUNCTION(sc_hash, TYPE_U64, TYPE_U64, TYPE_USize);
    DEFINE_EXTERN_C_FUNCTION(sc_hash2x64, TYPE_U64, TYPE_U64, TYPE_U64);
    DEFINE_EXTERN_C_FUNCTION(sc_hashbytes, TYPE_U64, native_ro_pointer_type(TYPE_I8), TYPE_USize);

    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_import_c, TYPE_Scope, TYPE_String, TYPE_String, TYPE_List);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_load_library, _void, TYPE_String);

    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_scope_at, TYPE_ValueRef, TYPE_Scope, TYPE_Symbol);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_scope_local_at, TYPE_ValueRef, TYPE_Scope, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_get_docstring, TYPE_String, TYPE_Scope, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_set_docstring, _void, TYPE_Scope, TYPE_Symbol, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_set_symbol, _void, TYPE_Scope, TYPE_Symbol, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_new, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_clone, TYPE_Scope, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_new_subscope, TYPE_Scope, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_clone_subscope, TYPE_Scope, TYPE_Scope, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_get_parent, TYPE_Scope, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_del_symbol, _void, TYPE_Scope, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_next, arguments_type({TYPE_Symbol, TYPE_ValueRef}), TYPE_Scope, TYPE_Symbol);

    DEFINE_EXTERN_C_FUNCTION(sc_symbol_new, TYPE_Symbol, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_symbol_new_unique, TYPE_Symbol, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_symbol_to_string, TYPE_String, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_symbol_is_variadic, TYPE_Bool, TYPE_Symbol);

    DEFINE_EXTERN_C_FUNCTION(sc_string_new, TYPE_String, native_ro_pointer_type(TYPE_I8), TYPE_USize);
    DEFINE_EXTERN_C_FUNCTION(sc_string_new_from_cstr, TYPE_String, native_ro_pointer_type(TYPE_I8));
    DEFINE_EXTERN_C_FUNCTION(sc_string_join, TYPE_String, TYPE_String, TYPE_String);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_string_match, TYPE_Bool, TYPE_String, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_string_count, TYPE_USize, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_string_compare, TYPE_I32, TYPE_String, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_string_buffer, arguments_type({rawstring, TYPE_USize}), TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_string_lslice, TYPE_String, TYPE_String, TYPE_USize);
    DEFINE_EXTERN_C_FUNCTION(sc_string_rslice, TYPE_String, TYPE_String, TYPE_USize);

    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_type_at, TYPE_ValueRef, TYPE_Type, TYPE_Symbol);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_type_local_at, TYPE_ValueRef, TYPE_Type, TYPE_Symbol);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_type_element_at, TYPE_Type, TYPE_Type, TYPE_I32);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_type_field_index, TYPE_I32, TYPE_Type, TYPE_Symbol);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_type_field_name, TYPE_Symbol, TYPE_Type, TYPE_I32);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_type_sizeof, TYPE_USize, TYPE_Type);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_type_alignof, TYPE_USize, TYPE_Type);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_type_countof, TYPE_I32, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_kind, TYPE_I32, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_debug_abi, _void, TYPE_Type);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_type_storage, TYPE_Type, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_is_opaque, TYPE_Bool, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_is_superof, TYPE_Bool, TYPE_Type, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_is_default_suffix, TYPE_Bool, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_string, TYPE_String, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_next, arguments_type({TYPE_Symbol, TYPE_ValueRef}), TYPE_Type, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_type_set_symbol, _void, TYPE_Type, TYPE_Symbol, TYPE_ValueRef);
    DEFINE_EXTERN_C_FUNCTION(sc_type_is_refer, TYPE_Bool, TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_type_key, arguments_type({TYPE_Symbol, TYPE_Type}), TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_key_type, TYPE_Type, TYPE_Symbol, TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_pointer_type, TYPE_Type, TYPE_Type, TYPE_U64, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_pointer_type_get_flags, TYPE_U64, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_pointer_type_set_flags, TYPE_Type, TYPE_Type, TYPE_U64);
    DEFINE_EXTERN_C_FUNCTION(sc_pointer_type_get_storage_class, TYPE_Symbol, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_pointer_type_set_storage_class, TYPE_Type, TYPE_Type, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_pointer_type_set_element_type, TYPE_Type, TYPE_Type, TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_type_bitcountof, TYPE_I32, TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_integer_type, TYPE_Type, TYPE_I32, TYPE_Bool);
    DEFINE_EXTERN_C_FUNCTION(sc_integer_type_is_signed, TYPE_Bool, TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_typename_type, TYPE_Type, TYPE_String);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_typename_type_set_super, _void, TYPE_Type, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_typename_type_get_super, TYPE_Type, TYPE_Type);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_typename_type_set_storage, _void, TYPE_Type, TYPE_Type, TYPE_U32);

    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_array_type, TYPE_Type, TYPE_Type, TYPE_USize);

    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_vector_type, TYPE_Type, TYPE_Type, TYPE_USize);

    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_tuple_type, TYPE_Type, TYPE_I32, native_ro_pointer_type(TYPE_Type));

    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_union_type, TYPE_Type, TYPE_I32, native_ro_pointer_type(TYPE_Type));

    DEFINE_EXTERN_C_FUNCTION(sc_arguments_type, TYPE_Type, TYPE_I32, native_ro_pointer_type(TYPE_Type));
    DEFINE_EXTERN_C_FUNCTION(sc_arguments_type_join, TYPE_Type, TYPE_Type, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_arguments_type_argcount, TYPE_I32, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_arguments_type_getarg, TYPE_Type, TYPE_Type, TYPE_I32);

    DEFINE_EXTERN_C_FUNCTION(sc_view_type, TYPE_Type, TYPE_Type, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_unique_type, TYPE_Type, TYPE_Type, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_mutate_type, TYPE_Type, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_refer_type, TYPE_Type, TYPE_Type, TYPE_U64, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_strip_qualifiers, TYPE_Type, TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_image_type, TYPE_Type,
        TYPE_Type, TYPE_Symbol, TYPE_I32, TYPE_I32, TYPE_I32, TYPE_I32, TYPE_Symbol, TYPE_Symbol);

    DEFINE_EXTERN_C_FUNCTION(sc_sampled_image_type, TYPE_Type, TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_function_type_is_variadic, TYPE_Bool, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_function_type, TYPE_Type, TYPE_Type, TYPE_I32, native_ro_pointer_type(TYPE_Type));
    DEFINE_EXTERN_C_FUNCTION(sc_function_type_raising, TYPE_Type, TYPE_Type, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_function_type_return_type, arguments_type({TYPE_Type, TYPE_Type}), TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_list_cons, TYPE_List, TYPE_ValueRef, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_dump, TYPE_List, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_repr, TYPE_String, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_join, TYPE_List, TYPE_List, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_decons, arguments_type({TYPE_ValueRef, TYPE_List}), TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_count, TYPE_I32, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_at, TYPE_ValueRef, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_next, TYPE_List, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_reverse, TYPE_List, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_compare, TYPE_Bool, TYPE_List, TYPE_List);

    DEFINE_EXTERN_C_FUNCTION(sc_closure_get_docstring, TYPE_String, TYPE_Closure);
    DEFINE_EXTERN_C_FUNCTION(sc_closure_get_template, TYPE_ValueRef, TYPE_Closure);
    DEFINE_EXTERN_C_FUNCTION(sc_closure_get_context, TYPE_ValueRef, TYPE_Closure);

    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_parse_from_path, TYPE_ValueRef, TYPE_String);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_parse_from_string, TYPE_ValueRef, TYPE_String);

#undef DEFINE_EXTERN_C_FUNCTION

#ifdef SCOPES_WIN32
#define SCOPES_SYM_OS "windows"
#else
#ifdef SCOPES_MACOS
#define SCOPES_SYM_OS "macos"
#else
#ifdef SCOPES_LINUX
#define SCOPES_SYM_OS "linux"
#else
#define SCOPES_SYM_OS "unknown"
#endif
#endif
#endif
    bind_symbol(Symbol("operating-system"), Symbol(SCOPES_SYM_OS));
#undef SCOPES_SYM_OS

    bind_new_value(Symbol("unroll-limit"),
        ConstInt::from(TYPE_I32, SCOPES_MAX_RECURSIONS));
    bind_new_value(KW_True, ConstInt::from(TYPE_Bool, true));
    bind_new_value(KW_False, ConstInt::from(TYPE_Bool, false));
    bind_new_value(Symbol("noreturn"),
        ConstPointer::type_from(TYPE_NoReturn));
    bind_new_value(KW_None, ConstAggregate::none_from());
    bind_symbol(Symbol("unnamed"), Symbol(SYM_Unnamed));
    bind_new_value(SYM_CompilerDir,
        ConstPointer::string_from(
            String::from(scopes_compiler_dir, strlen(scopes_compiler_dir))));
    bind_new_value(SYM_CompilerPath,
        ConstPointer::string_from(
            String::from(scopes_compiler_path, strlen(scopes_compiler_path))));
    bind_new_value(SYM_DebugBuild,
        ConstInt::from(TYPE_Bool, scopes_is_debug()));
    bind_new_value(SYM_CompilerTimestamp,
        ConstPointer::string_from(
            String::from_cstr(scopes_compile_time_date())));

    for (uint64_t i = STYLE_FIRST; i <= STYLE_LAST; ++i) {
        Symbol sym = Symbol((KnownSymbol)i);
        bind_symbol(sym, sym);
    }

    bind_new_value(Symbol("voidstar"),
        ConstPointer::type_from(voidstar));
#define T(TYPE, NAME) \
    bind_new_value(Symbol(NAME), ConstPointer::type_from(TYPE));
B_TYPES()
#undef T

#define T(NAME, BNAME, CLASS) \
    bind_new_value(Symbol(BNAME), ConstInt::from(TYPE_I32, (int32_t)NAME));
    B_TYPE_KIND()
#undef T

#define T(NAME, BNAME, CLASS) \
    bind_new_value(Symbol(BNAME), ConstInt::from(TYPE_I32, (int32_t)NAME));
    SCOPES_VALUE_KIND()
#undef T

    bind_new_value(Symbol("global-flag-buffer-block"),
        ConstInt::from(TYPE_U32, GF_BufferBlock));
    bind_new_value(Symbol("global-flag-non-writable"),
        ConstInt::from(TYPE_U32, GF_NonWritable));
    bind_new_value(Symbol("global-flag-non-readable"),
        ConstInt::from(TYPE_U32, GF_NonReadable));
    bind_new_value(Symbol("global-flag-volatile"),
        ConstInt::from(TYPE_U32, GF_Volatile));
    bind_new_value(Symbol("global-flag-coherent"),
        ConstInt::from(TYPE_U32, GF_Coherent));
    bind_new_value(Symbol("global-flag-restrict"),
        ConstInt::from(TYPE_U32, GF_Restrict));
    bind_new_value(Symbol("global-flag-block"),
        ConstInt::from(TYPE_U32, GF_Block));

    bind_new_value(Symbol("pointer-flag-non-readable"),
        ConstInt::from(TYPE_U64, (uint64_t)PTF_NonReadable));
    bind_new_value(Symbol("pointer-flag-non-writable"),
        ConstInt::from(TYPE_U64, (uint64_t)PTF_NonWritable));

    bind_new_value(Symbol("typename-flag-plain"),
        ConstInt::from(TYPE_U32, (uint32_t)TNF_Plain));

    bind_new_value(Symbol(SYM_DumpDisassembly),
        ConstInt::from(TYPE_U64, (uint64_t)CF_DumpDisassembly));
    bind_new_value(Symbol(SYM_DumpModule),
        ConstInt::from(TYPE_U64, (uint64_t)CF_DumpModule));
    bind_new_value(Symbol(SYM_DumpFunction),
        ConstInt::from(TYPE_U64, (uint64_t)CF_DumpFunction));
    bind_new_value(Symbol(SYM_DumpTime),
        ConstInt::from(TYPE_U64, (uint64_t)CF_DumpTime));
    bind_new_value(Symbol(SYM_NoDebugInfo),
        ConstInt::from(TYPE_U64, (uint64_t)CF_NoDebugInfo));
    bind_new_value(Symbol(SYM_O1),
        ConstInt::from(TYPE_U64, (uint64_t)CF_O1));
    bind_new_value(Symbol(SYM_O2),
        ConstInt::from(TYPE_U64, (uint64_t)CF_O2));
    bind_new_value(Symbol(SYM_O3),
        ConstInt::from(TYPE_U64, (uint64_t)CF_O3));

#define T(NAME) bind_new_value(NAME, ConstInt::builtin_from(Builtin(NAME)));
#define T0(NAME, STR) bind_new_value(NAME, ConstInt::builtin_from(Builtin(NAME)));
#define T1 T2
#define T2T T2
#define T2(UNAME, LNAME, PFIX, OP) \
    bind_new_value(FN_ ## UNAME ## PFIX, ConstInt::builtin_from(Builtin(FN_ ## UNAME ## PFIX)));
    B_GLOBALS()
#undef T
#undef T0
#undef T1
#undef T2
#undef T2T

    linenoiseSetCompletionCallback(prompt_completion_cb);

    // reimport wrapped symbols
    import_symbols();
}

} // namespace scopes