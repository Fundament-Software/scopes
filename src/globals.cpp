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
#include "c_import.hpp"
#include "stream_expr.hpp"
#include "stream_ast.hpp"
#include "scope.hpp"
#include "error.hpp"
#include "globals.hpp"
#include "platform_abi.hpp"
#include "source_file.hpp"
#include "lexerparser.hpp"
#include "expander.hpp"
#include "closure.hpp"
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
    Value *NAME = nullptr;
SCOPES_REIMPORT_SYMBOLS()
#undef T

static void import_symbols() {
    auto globs = sc_get_original_globals();
#define T(NAME, STR) \
    NAME = sc_scope_at(globs, Symbol(STR))._0; \
    assert(NAME);
SCOPES_REIMPORT_SYMBOLS()
#undef T
}

//------------------------------------------------------------------------------

#if 0
static void init_values_array(scopes::Values &dest, int numvalues, sc_value_t **values) {
    dest.reserve(numvalues);
    for (int i = 0; i < numvalues; ++i) {
        assert(values[i]);
        //assert(values[i] > (Value *)0x1000);
        dest.push_back(values[i]);
    }
}
#endif

template<typename T>
static void init_values_arrayT(std::vector<T *> &dest, int numvalues, sc_value_t **values) {
    dest.reserve(numvalues);
    for (int i = 0; i < numvalues; ++i) {
        assert(values[i]);
        dest.push_back(cast<T>(values[i]));
    }
}

//------------------------------------------------------------------------------

static Scope *globals = nullptr;
static Scope *original_globals = Scope::from();

//------------------------------------------------------------------------------

} // namespace scopes

extern "C" {

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

#define RETURN_RESULT(X) { auto _result = (X); \
    return {_result.ok(), (_result.ok()?nullptr:_result.unsafe_error()), _result.unsafe_extract()}; }
#define RETURN_VOID(X) { auto _result = (X); \
    return {_result.ok(), (_result.ok()?nullptr:_result.unsafe_error())}; }

sc_value_list_raises_t sc_expand(sc_value_t *expr, const sc_list_t *next, sc_scope_t *scope) {
    using namespace scopes;
    RETURN_RESULT(expand(expr, next, scope));
}

sc_value_raises_t sc_eval(const sc_anchor_t *anchor, const sc_list_t *expr, sc_scope_t *scope) {
    using namespace scopes;
    auto module_result = expand_module(anchor, expr, scope);
    if (!module_result.ok()) return { false, module_result.assert_error(), nullptr };
    RETURN_RESULT(prove(nullptr, module_result.assert_ok(), {}));
}

sc_value_raises_t sc_eval_inline(const sc_anchor_t *anchor, const sc_list_t *expr, sc_scope_t *scope) {
    using namespace scopes;
    //const Anchor *anchor = expr->anchor();
    //auto list = SCOPES_GET_RESULT(extract_list_constant(expr));

    RETURN_RESULT(expand_inline(anchor, nullptr, expr, scope));
}

sc_value_raises_t sc_typify_template(sc_value_t *f, int numtypes, const sc_type_t **typeargs) {
    using namespace scopes;
    auto tf = cast<Template>(f);
    if (tf->is_inline()) {
        return { false,
            make_location_error(String::from("cannot typify inline function")),
            nullptr };
    }
    Types types;
    for (int i = 0; i < numtypes; ++i) {
        types.push_back(typeargs[i]);
    }
    RETURN_RESULT(prove(nullptr, tf, types));
}

sc_value_raises_t sc_typify(sc_closure_t *srcl, int numtypes, const sc_type_t **typeargs) {
    using namespace scopes;
    if (srcl->func->is_inline()) {
        return { false,
            make_location_error(String::from("cannot typify function")),
            nullptr };
    }
    Types types;
    for (int i = 0; i < numtypes; ++i) {
        types.push_back(typeargs[i]);
    }
#if 0 //SCOPES_DEBUG_CODEGEN
    StyledStream ss(std::cout);
    std::cout << "sc_typify non-normalized:" << std::endl;
    stream_ast(ss, srcl->func, StreamASTFormat());
    std::cout << std::endl;
#endif
    RETURN_RESULT(prove(srcl->frame, srcl->func, types));
}

sc_value_raises_t sc_compile(sc_value_t *srcl, uint64_t flags) {
    using namespace scopes;
    auto result = extract_function_constant(srcl);
    if (!result.ok()) {
        return { false, result.assert_error(), nullptr };
    }
    RETURN_RESULT(compile(result.assert_ok(), flags));
}

sc_string_raises_t sc_compile_spirv(sc_symbol_t target, sc_value_t *srcl, uint64_t flags) {
    using namespace scopes;
    //RETURN_RESULT(compile_spirv(target, srcl, flags));
    return {true,nullptr,String::from("")};
}

sc_string_raises_t sc_compile_glsl(sc_symbol_t target, sc_value_t *srcl, uint64_t flags) {
    using namespace scopes;
    //RETURN_RESULT(compile_glsl(target, srcl, flags));
    return {true,nullptr,String::from("")};
}

sc_void_raises_t sc_compile_object(const sc_string_t *path, sc_scope_t *table, uint64_t flags) {
    using namespace scopes;
    RETURN_VOID(compile_object(path, table, flags));
}

void sc_enter_solver_cli () {
    using namespace scopes;
    //enable_specializer_step_debugger();
}

sc_size_raises_t sc_verify_stack () {
    using namespace scopes;
    RETURN_RESULT(verify_stack());
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

sc_error_t *sc_location_error_new(const sc_anchor_t *anchor, const sc_string_t *msg) {
    using namespace scopes;
    return make_location_error(anchor, msg);
}
sc_error_t *sc_runtime_error_new(const sc_string_t *msg) {
    using namespace scopes;
    return make_runtime_error(msg);
}
const sc_string_t *sc_format_error(const sc_error_t *err) {
    using namespace scopes;
    StyledString ss;
    stream_error_string(ss.out, err);
    return ss.str();
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

typedef std::unordered_map<const List *, Value *> MemoMap;
static MemoMap memo_map;

}

sc_bool_value_tuple_t sc_map_load(const sc_list_t *key) {
    using namespace scopes;
    auto it = memo_map.find(key);
    if (it != memo_map.end()) {
        return { true, it->second };
    } else {
        return { false, nullptr };
    }
}

void sc_map_store(sc_value_t *value, const sc_list_t *key) {
    using namespace scopes;
    auto ret = memo_map.insert({key, value});
    if (!ret.second) {
        ret.first->second = value;
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
    RETURN_RESULT(import_c_module(path->data, args, content->data));
}

sc_void_raises_t sc_load_library(const sc_string_t *name) {
    using namespace scopes;
#ifdef SCOPES_WIN32
    // try to load library through regular interface first
    dlerror();
    void *handle = dlopen(name->data, RTLD_LAZY);
    if (!handle) {
        StyledString ss;
        ss.out << "error loading library " << name;
        char *err = dlerror();
        if (err) {
            ss.out << ": " << err;
        }
        return {false, make_location_error(ss.str()) };
    }
#endif
    if (LLVMLoadLibraryPermanently(name->data)) {
        StyledString ss;
        ss.out << "error loading library " << name;
        return {false, make_location_error(ss.str()) };
    }
    return {true,nullptr};
}

// Anchor
////////////////////////////////////////////////////////////////////////////////

void sc_set_active_anchor(const sc_anchor_t *anchor) {
    using namespace scopes;
    _set_active_anchor(anchor);
}

const sc_anchor_t *sc_get_active_anchor() {
    using namespace scopes;
    return get_active_anchor();
}

// Scope
////////////////////////////////////////////////////////////////////////////////

void sc_scope_set_symbol(sc_scope_t *scope, sc_symbol_t sym, sc_value_t *value) {
    using namespace scopes;
    scope->bind(sym, value);
}

sc_value_raises_t sc_scope_at(sc_scope_t *scope, sc_symbol_t key) {
    using namespace scopes;
    Value *result = nullptr;
    bool ok = scope->lookup(key, result);
    if (!ok) {
        StyledString ss;
        ss.out << "no attribute " << key << " in scope";
        return { false, make_location_error(ss.str()), nullptr };
    }
    return { true, nullptr, result };
}

sc_value_raises_t sc_scope_local_at(sc_scope_t *scope, sc_symbol_t key) {
    using namespace scopes;
    Value *result = nullptr;
    bool ok = scope->lookup_local(key, result);
    if (!ok) {
        StyledString ss;
        ss.out << "no local attribute " << key << " in scope";
        return { false, make_location_error(ss.str()), nullptr };
    }
    return { true, nullptr, result };
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

sc_symbol_value_tuple_t sc_scope_next(sc_scope_t *scope, sc_symbol_t key) {
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
    return { SYM_Unnamed, nullptr };
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
    auto it = pattern_cache.find(pattern);
    regexp::Reprog *m = nullptr;
    if (it == pattern_cache.end()) {
        const char *error = nullptr;
        m = regexp::regcomp(pattern->data, 0, &error);
        if (error) {
            const String *err = String::from_cstr(error);
            regexp::regfree(m);
            return { false, make_location_error(err), false };
        }
        pattern_cache.insert({ pattern, m });
    } else {
        m = it->second;
    }
    return { true, nullptr, (regexp::regexec(m, text->data, nullptr, 0) == 0) };
}

size_t sc_string_count(const sc_string_t *str) {
    using namespace scopes;
    return str->count;
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

const sc_list_t *sc_list_cons(sc_value_t *at, const sc_list_t *next) {
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
    stream_expr(ss, l, StreamExprFormat());
    return l;
}

sc_value_list_tuple_t sc_list_decons(const sc_list_t *l) {
    using namespace scopes;
    if (l)
        return { l->at, l->next };
    else
        return { ConstAggregate::none_from(get_active_anchor()), nullptr };
}

int sc_list_count(const sc_list_t *l) {
    using namespace scopes;
    return l?l->count:0;
}

sc_value_t *sc_list_at(const sc_list_t *l) {
    using namespace scopes;
    return l?l->at:ConstAggregate::none_from(get_active_anchor());
}

const sc_list_t *sc_list_next(const sc_list_t *l) {
    using namespace scopes;
    return l?l->next:EOL;
}

const sc_list_t *sc_list_reverse(const sc_list_t *l) {
    return reverse_list(l);
}

// Value
////////////////////////////////////////////////////////////////////////////////

const sc_string_t *sc_value_repr (sc_value_t *value) {
    using namespace scopes;
    StyledString ss;
    stream_ast(ss.out, value, StreamASTFormat::singleline());
    return ss.str();
}

const sc_string_t *sc_value_ast_repr (sc_value_t *value) {
    using namespace scopes;
    StyledString ss;
    stream_ast(ss.out, value, StreamASTFormat());
    return ss.str();
}

const sc_string_t *sc_value_tostring (sc_value_t *value) {
    using namespace scopes;
    StyledString ss = StyledString::plain();
    stream_ast(ss.out, value, StreamASTFormat::singleline());
    return ss.str();
}

const sc_type_t *sc_value_type (sc_value_t *value) {
    using namespace scopes;
    if (!value->is_typed())
        return TYPE_Unknown;
    return strip_qualifiers(value->get_type());
}

const sc_type_t *sc_value_qualified_type (sc_value_t *value) {
    using namespace scopes;
    if (!value->is_typed())
        return TYPE_Unknown;
    return value->get_type();
}

const sc_anchor_t *sc_value_anchor (sc_value_t *value) {
    using namespace scopes;
    return value->anchor();
}

bool sc_value_is_constant(sc_value_t *value) {
    using namespace scopes;
    return isa<Const>(value);
}

bool sc_value_is_pure (sc_value_t *value) {
    using namespace scopes;
    return isa<Pure>(value);
}

int sc_value_kind (sc_value_t *value) {
    using namespace scopes;
    return value->kind();
}

sc_value_t *sc_value_wrap(const sc_type_t *type, sc_value_t *value) {
    using namespace scopes;
    if (isa<Const>(value)) {
        return value;
    }
    auto result = wrap_value(type, value);
    assert(result);
    return result;
}

sc_value_t *sc_value_unwrap(const sc_type_t *type, sc_value_t *value) {
    using namespace scopes;
    auto result = unwrap_value(type, value);
    assert(result);
    return result;
}

sc_value_t *sc_keyed_new(sc_symbol_t key, sc_value_t *value) {
    using namespace scopes;
    if (value->is_typed()) {
        return rekey(get_active_anchor(), key, value);
    } else {
        return Keyed::from(get_active_anchor(), key, value);
    }
}

sc_value_t *sc_argument_list_new() {
    using namespace scopes;
    return ArgumentList::from(get_active_anchor());
}

void sc_argument_list_append(sc_value_t *alist, sc_value_t *value) {
    using namespace scopes;
    return cast<ArgumentList>(alist)->append(value);
}

int sc_argcount(sc_value_t *value) {
    using namespace scopes;
    if (isa<ArgumentList>(value)) {
        return (int)cast<ArgumentList>(value)->values.size();
    } else {
        return 1;
    }
}

sc_value_t *sc_getarg(sc_value_t *value, int index) {
    using namespace scopes;
    if (isa<ArgumentList>(value)) {
        auto al = cast<ArgumentList>(value);
        if (index < al->values.size()) {
            return al->values[index];
        }
    } else if (index == 0) {
        return value;
    }
    return ConstAggregate::none_from(get_active_anchor());
}

sc_value_t *sc_getarglist(sc_value_t *value, int index) {
    using namespace scopes;
    if (isa<ArgumentList>(value)) {
        auto al = cast<ArgumentList>(value);
        Values values;
        for (int i = index; i < al->values.size(); ++i) {
            values.push_back(al->values[i]);
        }
        return build_argument_list(get_active_anchor(), values);
    } else if (index == 0) {
        return value;
    } else {
        return build_argument_list(get_active_anchor(), {});
    }
}

sc_value_t *sc_extract_argument_new(sc_value_t *value, int index) {
    using namespace scopes;
    return ExtractArgument::from(get_active_anchor(), value, index);
}

sc_value_t *sc_extract_argument_list_new(sc_value_t *value, int index) {
    using namespace scopes;
    return ExtractArgument::from(get_active_anchor(), value, index, true);
}

sc_value_t *sc_template_new(sc_symbol_t name) {
    using namespace scopes;
    // todo: set scope
    return Template::from(get_active_anchor(), name);
}
void sc_template_set_name(sc_value_t *fn, sc_symbol_t name) {
    using namespace scopes;
    cast<Template>(fn)->name = name;
}
sc_symbol_t sc_template_get_name(sc_value_t *fn) {
    using namespace scopes;
    return cast<Template>(fn)->name;
}
void sc_template_append_parameter(sc_value_t *fn, sc_value_t *symbol) {
    using namespace scopes;
    cast<Template>(fn)->append_param(cast<Parameter>(symbol));
}
void sc_template_set_body(sc_value_t *fn, sc_value_t *value) {
    using namespace scopes;
    cast<Template>(fn)->value = value;
}

void sc_template_set_inline(sc_value_t *fn) {
    using namespace scopes;
    cast<Template>(fn)->set_inline();
}

sc_value_t *sc_expression_new() {
    using namespace scopes;
    auto block = Expression::from(get_active_anchor());
    block->scoped = false;
    return block;
}

void sc_expression_set_scoped(sc_value_t *expr) {
    using namespace scopes;
    cast<Expression>(expr)->scoped = true;
}

void sc_expression_append(sc_value_t *expr, sc_value_t *value) {
    using namespace scopes;
    cast<Expression>(expr)->append(value);
}

sc_value_t *sc_extern_new(sc_symbol_t name, const sc_type_t *type) {
    using namespace scopes;
    return Extern::from(get_active_anchor(), type, name);
}
void sc_extern_set_flags(sc_value_t *value, uint32_t flags) {
    using namespace scopes;
    cast<Extern>(value)->flags = flags;
}
uint32_t sc_extern_get_flags(sc_value_t *value) {
    using namespace scopes;
    return cast<Extern>(value)->flags;
}
void sc_extern_set_storage_class(sc_value_t *value, sc_symbol_t storage_class) {
    using namespace scopes;
    cast<Extern>(value)->storage_class = storage_class;
}
sc_symbol_t sc_extern_get_storage_class(sc_value_t *value) {
    using namespace scopes;
    return cast<Extern>(value)->storage_class;
}
void sc_extern_set_location(sc_value_t *value, int32_t location) {
    using namespace scopes;
    cast<Extern>(value)->location = location;
}
int32_t sc_extern_get_location(sc_value_t *value) {
    using namespace scopes;
    return cast<Extern>(value)->location;
}

void sc_extern_set_binding(sc_value_t *value, int32_t binding) {
    using namespace scopes;
    cast<Extern>(value)->binding = binding;
}
int32_t sc_extern_get_binding(sc_value_t *value) {
    using namespace scopes;
    return cast<Extern>(value)->binding;
}

sc_value_t *sc_if_new() {
    using namespace scopes;
    return If::from(get_active_anchor());
}
void sc_if_append_then_clause(sc_value_t *value, sc_value_t *cond, sc_value_t *body) {
    using namespace scopes;
    cast<If>(value)->append_then(get_active_anchor(), cond, body);
}
void sc_if_append_else_clause(sc_value_t *value, sc_value_t *body) {
    using namespace scopes;
    cast<If>(value)->append_else(get_active_anchor(), body);
}

sc_value_t *sc_switch_new(sc_value_t *expr) {
    using namespace scopes;
    return SwitchTemplate::from(get_active_anchor(), expr);
}
void sc_switch_append_case(sc_value_t *value, sc_value_t *literal, sc_value_t *body) {
    using namespace scopes;
    cast<SwitchTemplate>(value)->append_case(get_active_anchor(), literal, body);
}
void sc_switch_append_pass(sc_value_t *value, sc_value_t *literal, sc_value_t *body) {
    using namespace scopes;
    cast<SwitchTemplate>(value)->append_pass(get_active_anchor(), literal, body);
}
void sc_switch_append_default(sc_value_t *value, sc_value_t *body) {
    using namespace scopes;
    cast<SwitchTemplate>(value)->append_default(get_active_anchor(), body);
}

sc_value_t *sc_parameter_new(sc_symbol_t name) {
    using namespace scopes;
    auto param = Parameter::from(get_active_anchor(), name, nullptr);
    if (ends_with_parenthesis(name))
        param->variadic = true;
    return param;
}

bool sc_parameter_is_variadic(sc_value_t *param) {
    using namespace scopes;
    return cast<Parameter>(param)->is_variadic();
}

sc_value_t *sc_call_new(sc_value_t *callee) {
    using namespace scopes;
    return Call::from(get_active_anchor(), callee);
}

void sc_call_append_argument(sc_value_t *call, sc_value_t *value) {
    using namespace scopes;
    cast<Call>(call)->args.push_back(value);
}

bool sc_call_is_rawcall(sc_value_t *value) {
    using namespace scopes;
    return cast<Call>(value)->is_rawcall();
}

void sc_call_set_rawcall(sc_value_t *value, bool enable) {
    using namespace scopes;
    cast<Call>(value)->set_rawcall();
}

sc_value_t *sc_loop_new(sc_value_t *init) {
    using namespace scopes;
    return Loop::from(get_active_anchor(), init);
}

sc_value_t *sc_loop_arguments(sc_value_t *loop) {
    using namespace scopes;
    return cast<Loop>(loop)->args;
}

void sc_loop_set_body(sc_value_t *loop, sc_value_t *body) {
    using namespace scopes;
    cast<Loop>(loop)->value = body;
}

sc_value_t *sc_const_int_new(const sc_type_t *type, uint64_t value) {
    using namespace scopes;
    return ConstInt::from(get_active_anchor(), type, value);
}
sc_value_t *sc_const_real_new(const sc_type_t *type, double value) {
    using namespace scopes;
    return ConstReal::from(get_active_anchor(), type, value);
}
sc_value_t *sc_const_aggregate_new(const sc_type_t *type, int numconsts, sc_value_t **consts) {
    using namespace scopes;
    Constants vals;
    init_values_arrayT(vals, numconsts, consts);
    return ConstAggregate::from(get_active_anchor(), type, vals);
}
sc_value_t *sc_const_pointer_new(const sc_type_t *type, const void *pointer) {
    using namespace scopes;
    return ConstPointer::from(get_active_anchor(), type, pointer);
}
uint64_t sc_const_int_extract(const sc_value_t *value) {
    using namespace scopes;
    return cast<ConstInt>(value)->value;
}
double sc_const_real_extract(const sc_value_t *value) {
    using namespace scopes;
    return cast<ConstReal>(value)->value;
}
sc_value_t *sc_const_extract_at(const sc_value_t *value, int index) {
    using namespace scopes;
    auto val = cast<ConstAggregate>(value);
    assert(index < val->values.size());
    return val->values[index];
}
const void *sc_const_pointer_extract(const sc_value_t *value) {
    using namespace scopes;
    return cast<ConstPointer>(value)->value;
}

sc_value_t *sc_break_new(sc_value_t *value) {
    using namespace scopes;
    return Break::from(get_active_anchor(), value);
}
sc_value_t *sc_repeat_new(sc_value_t *value) {
    using namespace scopes;
    return Repeat::from(get_active_anchor(), value);
}
sc_value_t *sc_return_new(sc_value_t *value) {
    using namespace scopes;
    return Return::from(get_active_anchor(), value);
}
sc_value_t *sc_raise_new(sc_value_t *value) {
    using namespace scopes;
    return Raise::from(get_active_anchor(), value);
}

sc_value_t *sc_quote_new(sc_value_t *value) {
    using namespace scopes;
    return Quote::from(get_active_anchor(), value);
}

sc_value_t *sc_unquote_new(sc_value_t *value) {
    using namespace scopes;
    return Unquote::from(get_active_anchor(), value);
}

sc_value_t *sc_label_new(int kind, sc_symbol_t name) {
    using namespace scopes;
    return LabelTemplate::from(get_active_anchor(), (LabelKind)kind, name);
}
void sc_label_set_body(sc_value_t *label, sc_value_t *body) {
    using namespace scopes;
    cast<LabelTemplate>(label)->value = body;
}
sc_value_t *sc_merge_new(sc_value_t *label, sc_value_t *value) {
    using namespace scopes;
    return MergeTemplate::from(get_active_anchor(), cast<LabelTemplate>(label), value);
}

// Parser
////////////////////////////////////////////////////////////////////////////////

sc_value_raises_t sc_parse_from_path(const sc_string_t *path) {
    using namespace scopes;
    auto sf = SourceFile::from_file(path);
    if (!sf) {
        StyledString ss;
        ss.out << "no such file: " << path;
        return { false, make_location_error(ss.str()), nullptr };
    }
    LexerParser parser(sf);
    RETURN_RESULT(parser.parse());
}

sc_value_raises_t sc_parse_from_string(const sc_string_t *str) {
    using namespace scopes;
    auto sf = SourceFile::from_string(Symbol("<string>"), str);
    assert(sf);
    LexerParser parser(sf);
    RETURN_RESULT(parser.parse());
}

// Types
////////////////////////////////////////////////////////////////////////////////

sc_value_raises_t sc_type_at(const sc_type_t *T, sc_symbol_t key) {
    using namespace scopes;
    T = strip_qualifiers(T);
    Value *result = nullptr;
    bool ok = T->lookup(key, result);
    if (!ok) {
        StyledString ss;
        ss.out << "no attribute " << key << " in type " << T;
        return { false, make_location_error(ss.str()), nullptr };
    }
    return { true, nullptr, result };
}

sc_value_raises_t sc_type_local_at(const sc_type_t *T, sc_symbol_t key) {
    using namespace scopes;
    T = strip_qualifiers(T);
    Value *result = nullptr;
    bool ok = T->lookup_local(key, result);
    if (!ok) {
        StyledString ss;
        ss.out << "no local attribute " << key << " in type " << T;
        return { false, make_location_error(ss.str()), nullptr };
    }
    return { true, nullptr, result };
}

sc_size_raises_t sc_type_sizeof(const sc_type_t *T) {
    using namespace scopes;
    RETURN_RESULT(size_of(T));
}

sc_size_raises_t sc_type_alignof(const sc_type_t *T) {
    using namespace scopes;
    RETURN_RESULT(align_of(T));
}

sc_int_raises_t sc_type_countof(const sc_type_t *T) {
    using namespace scopes;
    auto rT = storage_type(T);
    if (!rT.ok()) return { false, rT.assert_error(), 0 };
    T = rT.assert_ok();
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
    StyledString ss;
    ss.out << "storage type " << T << " has no count";
    return { false, make_location_error(ss.str()), 0 };
}

sc_type_raises_t sc_type_element_at(const sc_type_t *T, int i) {
    using namespace scopes;
    auto rT = storage_type(T);
    if (!rT.ok()) return { false, rT.assert_error(), nullptr };
    T = rT.assert_ok();
    switch(T->kind()) {
    case TK_Pointer: return { true, nullptr, cast<PointerType>(T)->element_type };
    case TK_Array: return { true, nullptr, cast<ArrayType>(T)->element_type };
    case TK_Vector: return { true, nullptr, cast<VectorType>(T)->element_type };
    case TK_Tuple: RETURN_RESULT( cast<TupleType>(T)->type_at_index(i) );
    case TK_Union: RETURN_RESULT( cast<UnionType>(T)->type_at_index(i) );
    case TK_Function: RETURN_RESULT(cast<FunctionType>(T)->type_at_index(i));
    case TK_Image: return { true, nullptr, cast<ImageType>(T)->type };
    case TK_SampledImage: return { true, nullptr, cast<SampledImageType>(T)->type };
    default: break;
    }
    StyledString ss;
    ss.out << "storage type " << T << " has no elements" << std::endl;
    return { false, make_location_error(ss.str()), nullptr };
}

sc_int_raises_t sc_type_field_index(const sc_type_t *T, sc_symbol_t name) {
    using namespace scopes;
    auto rT = storage_type(T);
    if (!rT.ok()) return { false, rT.assert_error(), -1 };
    T = rT.assert_ok();
    switch(T->kind()) {
    case TK_Tuple: return { true, nullptr, (int)cast<TupleType>(T)->field_index(name) };
    case TK_Union: return { true, nullptr, (int)cast<UnionType>(T)->field_index(name) };
    default: break;
    }
    StyledString ss;
    ss.out << "storage type " << T << " has no elements" << std::endl;
    return { false, make_location_error(ss.str()), -1 };
}

sc_symbol_raises_t sc_type_field_name(const sc_type_t *T, int index) {
    using namespace scopes;
    auto rT = storage_type(T);
    if (!rT.ok()) return { false, rT.assert_error(), SYM_Unnamed };
    T = rT.assert_ok();
    switch(T->kind()) {
    case TK_Tuple: RETURN_RESULT(cast<TupleType>(T)->field_name(index));
    case TK_Union: RETURN_RESULT(cast<UnionType>(T)->field_name(index));
    default: break;
    }
    StyledString ss;
    ss.out << "storage type " << T << " has no elements" << std::endl;
    return { false, make_location_error(ss.str()), SYM_Unnamed };
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
    RETURN_RESULT(storage_type(T));
}

bool sc_type_is_opaque(const sc_type_t *T) {
    using namespace scopes;
    return is_opaque(T);
}

const sc_string_t *sc_type_string(const sc_type_t *T) {
    using namespace scopes;
    StyledString ss = StyledString::plain();
    stream_type_name(ss.out, T);
    return ss.str();
}

sc_symbol_value_tuple_t sc_type_next(const sc_type_t *type, sc_symbol_t key) {
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
    return { SYM_Unnamed, nullptr };
}

void sc_type_set_symbol(const sc_type_t *T, sc_symbol_t sym, sc_value_t *value) {
    using namespace scopes;
    T = strip_qualifiers(T);
    const_cast<Type *>(T)->bind(sym, value);
}

// Pointer Type
////////////////////////////////////////////////////////////////////////////////

sc_symbol_type_tuple_t sc_type_key(const sc_type_t *T) {
    return type_key(T);
}

const sc_type_t *sc_key_type(sc_symbol_t name, const sc_type_t *T) {
    return key_type(name, T);
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
    auto r1 = verify_kind<TK_Typename>(T);
    if (!r1.ok()) return { false, r1.assert_error() };
    auto r2 = verify_kind<TK_Typename>(ST);
    if (!r2.ok()) return { false, r2.assert_error() };
    // if T <=: ST, the operation is illegal
    const Type *S = ST;
    while (S) {
        if (S == T) {
            StyledString ss;
            ss.out << "typename " << ST << " can not be a supertype of " << T;
            return { false, make_location_error(ss.str()) };
        }
        if (S == TYPE_Typename)
            break;
        S = superof(S);
    }
    auto tn = cast<TypenameType>(T);
    const_cast<TypenameType *>(tn)->super_type = ST;
    return { true, nullptr };
}

const sc_type_t *sc_typename_type_get_super(const sc_type_t *T) {
    using namespace scopes;
    return superof(T);
}

sc_void_raises_t sc_typename_type_set_storage(const sc_type_t *T, const sc_type_t *T2, uint32_t flags) {
    using namespace scopes;
    auto r1 = verify_kind<TK_Typename>(T);
    if (!r1.ok()) return { false, r1.assert_error() };
    RETURN_VOID(cast<TypenameType>(const_cast<Type *>(T))->finalize(T2, flags));
}

// Array Type
////////////////////////////////////////////////////////////////////////////////

sc_type_raises_t sc_array_type(const sc_type_t *element_type, size_t count) {
    using namespace scopes;
    RETURN_RESULT(array_type(element_type, count));
}

// Vector Type
////////////////////////////////////////////////////////////////////////////////

sc_type_raises_t sc_vector_type(const sc_type_t *element_type, size_t count) {
    using namespace scopes;
    RETURN_RESULT(vector_type(element_type, count));
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
    RETURN_RESULT(tuple_type(types));
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
    return view_type(type, { id });
}

const sc_type_t *sc_mutate_type(const sc_type_t *type) {
    using namespace scopes;
    return mutate_type(type);
}

const sc_type_t *sc_refer_type(const sc_type_t *type, uint64_t flags, sc_symbol_t storage_class) {
    using namespace scopes;
    return refer_type(type, flags, storage_class);
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

static void bind_extern(const Anchor *anchor, Symbol globalsym, Symbol externsym, const Type *T) {
    globals->bind(globalsym, Extern::from(anchor, T, externsym, EF_NonWritable));
}

static void bind_symbol(const Anchor *anchor, Symbol sym, Symbol value) {
    globals->bind(sym, ConstInt::symbol_from(anchor, value));
}

static void bind_extern(const Anchor *anchor, Symbol sym, const Type *T) {
    bind_extern(anchor, sym, sym, T);
}

void init_globals(int argc, char *argv[]) {
    globals = original_globals;
    scopes_argc = argc;
    scopes_argv = argv;

#define LINE_ANCHOR Anchor::from(stub_file, __LINE__, 1)

#define DEFINE_EXTERN_C_FUNCTION(FUNC, RETTYPE, ...) \
    (void)FUNC; /* ensure that the symbol is there */ \
    bind_extern(LINE_ANCHOR, Symbol(#FUNC), function_type(RETTYPE, { __VA_ARGS__ }));
#define DEFINE_RAISING_EXTERN_C_FUNCTION(FUNC, RETTYPE, ...) \
    (void)FUNC; /* ensure that the symbol is there */ \
    bind_extern(LINE_ANCHOR, Symbol(#FUNC), raising_function_type(RETTYPE, { __VA_ARGS__ }));

    auto stub_file = SourceFile::from_string(Symbol(__FILE__), String::from_cstr(""));
    auto stub_anchor = Anchor::from(stub_file, 1, 1);
    _set_active_anchor(stub_anchor);

    const Type *rawstring = native_ro_pointer_type(TYPE_I8);
    const Type *TYPE_ValuePP = native_ro_pointer_type(TYPE_Value);
    const Type *_void = empty_arguments_type();
    const Type *voidstar = native_ro_pointer_type(_void);

    DEFINE_EXTERN_C_FUNCTION(sc_compiler_version, arguments_type({TYPE_I32, TYPE_I32, TYPE_I32}));
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_expand, arguments_type({TYPE_Value, TYPE_List}), TYPE_Value, TYPE_List, TYPE_Scope);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_eval, TYPE_Value, TYPE_Anchor, TYPE_List, TYPE_Scope);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_eval_inline, TYPE_Anchor, TYPE_Value, TYPE_List, TYPE_Scope);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_typify_template, TYPE_Value, TYPE_Value, TYPE_I32, native_ro_pointer_type(TYPE_Type));
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_typify, TYPE_Value, TYPE_Closure, TYPE_I32, native_ro_pointer_type(TYPE_Type));
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_compile, TYPE_Value, TYPE_Value, TYPE_U64);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_compile_spirv, TYPE_String, TYPE_Symbol, TYPE_Value, TYPE_U64);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_compile_glsl, TYPE_String, TYPE_Symbol, TYPE_Value, TYPE_U64);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_compile_object, _void, TYPE_String, TYPE_Scope, TYPE_U64);
    DEFINE_EXTERN_C_FUNCTION(sc_enter_solver_cli, _void);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_verify_stack, TYPE_USize);
    DEFINE_EXTERN_C_FUNCTION(sc_launch_args, arguments_type({TYPE_I32,native_ro_pointer_type(rawstring)}));

    DEFINE_EXTERN_C_FUNCTION(sc_prompt, arguments_type({TYPE_Bool, TYPE_String}), TYPE_String, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_set_autocomplete_scope, _void, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_default_styler, TYPE_String, TYPE_Symbol, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_format_message, TYPE_String, TYPE_Anchor, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_write, _void, TYPE_String);

    DEFINE_EXTERN_C_FUNCTION(sc_value_repr, TYPE_String, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_value_ast_repr, TYPE_String, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_value_tostring, TYPE_String, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_value_type, TYPE_Type, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_value_qualified_type, TYPE_Type, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_value_anchor, TYPE_Anchor, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_value_is_constant, TYPE_Bool, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_value_is_pure, TYPE_Bool, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_value_kind, TYPE_I32, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_value_wrap, TYPE_Value, TYPE_Type, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_value_unwrap, TYPE_Value, TYPE_Type, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_keyed_new, TYPE_Value, TYPE_Symbol, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_argument_list_new, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_argument_list_append, _void, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_extract_argument_new, TYPE_Value, TYPE_Value, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_extract_argument_list_new, TYPE_Value, TYPE_Value, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_argcount, TYPE_I32, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_getarg, TYPE_Value, TYPE_Value, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_getarglist, TYPE_Value, TYPE_Value, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_template_new, TYPE_Value, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_template_set_name, _void, TYPE_Value, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_template_get_name, TYPE_Symbol, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_template_append_parameter, _void, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_template_set_body, _void, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_template_set_inline, _void, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_expression_new, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_expression_set_scoped, _void, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_expression_append, _void, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_extern_new, TYPE_Value, TYPE_Symbol, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_extern_set_flags, _void, TYPE_Value, TYPE_U32);
    DEFINE_EXTERN_C_FUNCTION(sc_extern_get_flags, TYPE_U32, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_extern_set_storage_class, _void, TYPE_Value, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_extern_get_storage_class, TYPE_Symbol, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_extern_set_location, _void, TYPE_Value, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_extern_get_location, TYPE_I32, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_extern_set_binding, _void, TYPE_Value, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_extern_get_binding, TYPE_I32, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_if_new, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_if_append_then_clause, _void, TYPE_Value, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_if_append_else_clause, _void, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_switch_new, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_switch_append_case, _void, TYPE_Value, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_switch_append_pass, _void, TYPE_Value, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_switch_append_default, _void, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_parameter_new, TYPE_Value, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_parameter_is_variadic, TYPE_Bool, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_call_new, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_call_append_argument, _void, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_call_is_rawcall, TYPE_Bool, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_call_set_rawcall, _void, TYPE_Value, TYPE_Bool);
    DEFINE_EXTERN_C_FUNCTION(sc_loop_new, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_loop_arguments, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_loop_set_body, _void, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_const_int_new, TYPE_Value, TYPE_Type, TYPE_U64);
    DEFINE_EXTERN_C_FUNCTION(sc_const_real_new, TYPE_Value, TYPE_Type, TYPE_F64);
    DEFINE_EXTERN_C_FUNCTION(sc_const_aggregate_new, TYPE_Value, TYPE_Type, TYPE_I32, TYPE_ValuePP);
    DEFINE_EXTERN_C_FUNCTION(sc_const_pointer_new, TYPE_Value, TYPE_Type, voidstar);
    DEFINE_EXTERN_C_FUNCTION(sc_const_int_extract, TYPE_U64, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_const_real_extract, TYPE_F64, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_const_extract_at, TYPE_Value, TYPE_Value, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_const_pointer_extract, voidstar, TYPE_Value);

    DEFINE_EXTERN_C_FUNCTION(sc_break_new, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_repeat_new, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_return_new, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_raise_new, TYPE_Value, TYPE_Value);

    DEFINE_EXTERN_C_FUNCTION(sc_quote_new, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_unquote_new, TYPE_Value, TYPE_Value);

    DEFINE_EXTERN_C_FUNCTION(sc_label_new, TYPE_Value, TYPE_I32, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_label_set_body, _void, TYPE_Value, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_merge_new, TYPE_Value, TYPE_Value, TYPE_Value);

    DEFINE_EXTERN_C_FUNCTION(sc_is_file, TYPE_Bool, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_is_directory, TYPE_Bool, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_realpath, TYPE_String, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_dirname, TYPE_String, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_basename, TYPE_String, TYPE_String);

    DEFINE_EXTERN_C_FUNCTION(sc_get_globals, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_get_original_globals, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_set_globals, _void, TYPE_Scope);

    DEFINE_EXTERN_C_FUNCTION(sc_location_error_new, TYPE_Error, TYPE_Anchor, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_runtime_error_new, TYPE_Error, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_format_error, TYPE_String, TYPE_Error);

    DEFINE_EXTERN_C_FUNCTION(sc_abort, TYPE_NoReturn);
    DEFINE_EXTERN_C_FUNCTION(sc_exit, TYPE_NoReturn, TYPE_I32);

    DEFINE_EXTERN_C_FUNCTION(sc_set_signal_abort,
        _void, TYPE_Bool);

    DEFINE_EXTERN_C_FUNCTION(sc_map_load, arguments_type({TYPE_Bool, TYPE_Value}), TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_map_store, _void, TYPE_Value, TYPE_List);

    DEFINE_EXTERN_C_FUNCTION(sc_hash, TYPE_U64, TYPE_U64, TYPE_USize);
    DEFINE_EXTERN_C_FUNCTION(sc_hash2x64, TYPE_U64, TYPE_U64, TYPE_U64);
    DEFINE_EXTERN_C_FUNCTION(sc_hashbytes, TYPE_U64, native_ro_pointer_type(TYPE_I8), TYPE_USize);

    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_import_c, TYPE_Scope, TYPE_String, TYPE_String, TYPE_List);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_load_library, _void, TYPE_String);

    DEFINE_EXTERN_C_FUNCTION(sc_get_active_anchor, TYPE_Anchor);
    DEFINE_EXTERN_C_FUNCTION(sc_set_active_anchor, _void, TYPE_Anchor);

    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_scope_at, TYPE_Value, TYPE_Scope, TYPE_Symbol);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_scope_local_at, TYPE_Value, TYPE_Scope, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_get_docstring, TYPE_String, TYPE_Scope, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_set_docstring, _void, TYPE_Scope, TYPE_Symbol, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_set_symbol, _void, TYPE_Scope, TYPE_Symbol, TYPE_Value);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_new, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_clone, TYPE_Scope, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_new_subscope, TYPE_Scope, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_clone_subscope, TYPE_Scope, TYPE_Scope, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_get_parent, TYPE_Scope, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_del_symbol, _void, TYPE_Scope, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_next, arguments_type({TYPE_Symbol, TYPE_Value}), TYPE_Scope, TYPE_Symbol);

    DEFINE_EXTERN_C_FUNCTION(sc_symbol_new, TYPE_Symbol, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_symbol_new_unique, TYPE_Symbol, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_symbol_to_string, TYPE_String, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_symbol_is_variadic, TYPE_Bool, TYPE_Symbol);

    DEFINE_EXTERN_C_FUNCTION(sc_string_new, TYPE_String, native_ro_pointer_type(TYPE_I8), TYPE_USize);
    DEFINE_EXTERN_C_FUNCTION(sc_string_new_from_cstr, TYPE_String, native_ro_pointer_type(TYPE_I8));
    DEFINE_EXTERN_C_FUNCTION(sc_string_join, TYPE_String, TYPE_String, TYPE_String);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_string_match, TYPE_Bool, TYPE_String, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_string_count, TYPE_USize, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_string_buffer, arguments_type({rawstring, TYPE_USize}), TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_string_lslice, TYPE_String, TYPE_String, TYPE_USize);
    DEFINE_EXTERN_C_FUNCTION(sc_string_rslice, TYPE_String, TYPE_String, TYPE_USize);

    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_type_at, TYPE_Value, TYPE_Type, TYPE_Symbol);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_type_local_at, TYPE_Value, TYPE_Type, TYPE_Symbol);
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
    DEFINE_EXTERN_C_FUNCTION(sc_type_string, TYPE_String, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_next, arguments_type({TYPE_Symbol, TYPE_Value}), TYPE_Type, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_type_set_symbol, _void, TYPE_Type, TYPE_Symbol, TYPE_Value);

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

    DEFINE_EXTERN_C_FUNCTION(sc_arguments_type, TYPE_Type, TYPE_I32, native_ro_pointer_type(TYPE_Type));
    DEFINE_EXTERN_C_FUNCTION(sc_arguments_type_join, TYPE_Type, TYPE_Type, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_arguments_type_argcount, TYPE_I32, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_arguments_type_getarg, TYPE_Type, TYPE_Type, TYPE_I32);

    DEFINE_EXTERN_C_FUNCTION(sc_view_type, TYPE_Type, TYPE_Type, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_mutate_type, TYPE_Type, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_refer_type, TYPE_Type, TYPE_Type, TYPE_U64, TYPE_Symbol);

    DEFINE_EXTERN_C_FUNCTION(sc_image_type, TYPE_Type,
        TYPE_Type, TYPE_Symbol, TYPE_I32, TYPE_I32, TYPE_I32, TYPE_I32, TYPE_Symbol, TYPE_Symbol);

    DEFINE_EXTERN_C_FUNCTION(sc_sampled_image_type, TYPE_Type, TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_function_type_is_variadic, TYPE_Bool, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_function_type, TYPE_Type, TYPE_Type, TYPE_I32, native_ro_pointer_type(TYPE_Type));
    DEFINE_EXTERN_C_FUNCTION(sc_function_type_raising, TYPE_Type, TYPE_Type, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_function_type_return_type, arguments_type({TYPE_Type, TYPE_Type}), TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_list_cons, TYPE_List, TYPE_Value, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_dump, TYPE_List, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_join, TYPE_List, TYPE_List, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_decons, arguments_type({TYPE_Value, TYPE_List}), TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_count, TYPE_I32, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_at, TYPE_Value, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_next, TYPE_List, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_reverse, TYPE_List, TYPE_List);

    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_parse_from_path, TYPE_Value, TYPE_String);
    DEFINE_RAISING_EXTERN_C_FUNCTION(sc_parse_from_string, TYPE_Value, TYPE_String);

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
    bind_symbol(LINE_ANCHOR, Symbol("operating-system"), Symbol(SCOPES_SYM_OS));
#undef SCOPES_SYM_OS

    globals->bind(Symbol("unroll-limit"),
        ConstInt::from(LINE_ANCHOR, TYPE_I32, SCOPES_MAX_RECURSIONS));
    globals->bind(KW_True, ConstInt::from(LINE_ANCHOR, TYPE_Bool, true));
    globals->bind(KW_False, ConstInt::from(LINE_ANCHOR, TYPE_Bool, false));
    globals->bind(Symbol("noreturn"),
        ConstPointer::type_from(LINE_ANCHOR, TYPE_NoReturn));
    globals->bind(KW_None, ConstAggregate::none_from(LINE_ANCHOR));
    bind_symbol(LINE_ANCHOR, Symbol("unnamed"), Symbol(SYM_Unnamed));
    globals->bind(SYM_CompilerDir,
        ConstPointer::string_from(LINE_ANCHOR,
            String::from(scopes_compiler_dir, strlen(scopes_compiler_dir))));
    globals->bind(SYM_CompilerPath,
        ConstPointer::string_from(LINE_ANCHOR,
            String::from(scopes_compiler_path, strlen(scopes_compiler_path))));
    globals->bind(SYM_DebugBuild,
        ConstInt::from(LINE_ANCHOR, TYPE_Bool, scopes_is_debug()));
    globals->bind(SYM_CompilerTimestamp,
        ConstPointer::string_from(LINE_ANCHOR,
            String::from_cstr(scopes_compile_time_date())));

    for (uint64_t i = STYLE_FIRST; i <= STYLE_LAST; ++i) {
        Symbol sym = Symbol((KnownSymbol)i);
        bind_symbol(LINE_ANCHOR, sym, sym);
    }

    globals->bind(Symbol("voidstar"),
        ConstPointer::type_from(LINE_ANCHOR, voidstar));
#define T(TYPE, NAME) \
    globals->bind(Symbol(NAME), ConstPointer::type_from(LINE_ANCHOR, TYPE));
B_TYPES()
#undef T

#define T(NAME, BNAME, CLASS) \
    globals->bind(Symbol(BNAME), ConstInt::from(LINE_ANCHOR, TYPE_I32, (int32_t)NAME));
    B_TYPE_KIND()
#undef T

#define T(NAME, BNAME, CLASS) \
    globals->bind(Symbol(BNAME), ConstInt::from(LINE_ANCHOR, TYPE_I32, (int32_t)NAME));
    SCOPES_VALUE_KIND()
#undef T

    globals->bind(Symbol("pointer-flag-non-readable"),
        ConstInt::from(LINE_ANCHOR, TYPE_U64, (uint64_t)PTF_NonReadable));
    globals->bind(Symbol("pointer-flag-non-writable"),
        ConstInt::from(LINE_ANCHOR, TYPE_U64, (uint64_t)PTF_NonWritable));

    globals->bind(Symbol("typename-flag-plain"),
        ConstInt::from(LINE_ANCHOR, TYPE_U32, (uint32_t)TNF_Plain));

    globals->bind(Symbol(SYM_DumpDisassembly),
        ConstInt::from(LINE_ANCHOR, TYPE_U64, (uint64_t)CF_DumpDisassembly));
    globals->bind(Symbol(SYM_DumpModule),
        ConstInt::from(LINE_ANCHOR, TYPE_U64, (uint64_t)CF_DumpModule));
    globals->bind(Symbol(SYM_DumpFunction),
        ConstInt::from(LINE_ANCHOR, TYPE_U64, (uint64_t)CF_DumpFunction));
    globals->bind(Symbol(SYM_DumpTime),
        ConstInt::from(LINE_ANCHOR, TYPE_U64, (uint64_t)CF_DumpTime));
    globals->bind(Symbol(SYM_NoDebugInfo),
        ConstInt::from(LINE_ANCHOR, TYPE_U64, (uint64_t)CF_NoDebugInfo));
    globals->bind(Symbol(SYM_O1),
        ConstInt::from(LINE_ANCHOR, TYPE_U64, (uint64_t)CF_O1));
    globals->bind(Symbol(SYM_O2),
        ConstInt::from(LINE_ANCHOR, TYPE_U64, (uint64_t)CF_O2));
    globals->bind(Symbol(SYM_O3),
        ConstInt::from(LINE_ANCHOR, TYPE_U64, (uint64_t)CF_O3));

#define T(NAME) globals->bind(NAME, ConstInt::builtin_from(LINE_ANCHOR, Builtin(NAME)));
#define T0(NAME, STR) globals->bind(NAME, ConstInt::builtin_from(LINE_ANCHOR, Builtin(NAME)));
#define T1 T2
#define T2T T2
#define T2(UNAME, LNAME, PFIX, OP) \
    globals->bind(FN_ ## UNAME ## PFIX, ConstInt::builtin_from(LINE_ANCHOR, Builtin(FN_ ## UNAME ## PFIX)));
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
