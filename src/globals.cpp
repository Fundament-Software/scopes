/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "globals.hpp"
#include "string.hpp"
#include "any.hpp"
#include "list.hpp"
#include "types.hpp"
#include "c_import.hpp"
#include "stream_label.hpp"
#include "stream_frame.hpp"
#include "stream_expr.hpp"
#include "scope.hpp"
#include "error.hpp"
#include "globals.hpp"
#include "platform_abi.hpp"
#include "syntax.hpp"
#include "parameter.hpp"
#include "source_file.hpp"
#include "lexerparser.hpp"
#include "frame.hpp"
#include "expander.hpp"
#include "specializer.hpp"
#include "closure.hpp"
#include "label.hpp"
#include "gen_llvm.hpp"
#include "gen_spirv.hpp"
#include "anchor.hpp"
#include "boot.hpp"
#include "gc.hpp"
#include "compiler_flags.hpp"
#include "hash.hpp"

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

const sc_list_t *sc_launch_args() {
    using namespace scopes;
    auto argc = scopes_argc;
    auto argv = scopes_argv;
    if (!argc || !argv) return EOL;
    const sc_list_t *result = EOL;
    while (argc > 0) {
        argc--;
        const char *s = argv[argc];
        if (!s) {
            result = List::from(Symbol(SYM_Unnamed).name(), result);
        } else {
            result = List::from(String::from_cstr(s), result);
        }
    }
    return result;
}

sc_label_t *sc_eval(const sc_syntax_t *expr, sc_scope_t *scope) {
    using namespace scopes;
    return specialize(Frame::root, expand_module(expr, scope), {});
}

sc_label_t *sc_eval_inline(const sc_list_t *expr, sc_scope_t *scope) {
    using namespace scopes;
    const Syntax *sxexpr = wrap_syntax(get_active_anchor(), expr, false);
    return expand_inline(sxexpr, scope);
}

sc_label_t *sc_typify(sc_closure_t *srcl, int numtypes, const sc_type_t **typeargs) {
    using namespace scopes;
    if (srcl->label->is_inline()) {
        location_error(String::from("cannot typify inline function"));
    }
    ArgTypes types;
    for (int i = 0; i < numtypes; ++i) {
        types.push_back(typeargs[i]);

    }
    return specialize(srcl->frame, srcl->label, types);
}

sc_any_t sc_compile(sc_label_t *srcl, uint64_t flags) {
    using namespace scopes;
    return compile(srcl, flags);
}

const sc_string_t *sc_compile_spirv(sc_symbol_t target, sc_label_t *srcl, uint64_t flags) {
    using namespace scopes;
    return compile_spirv(target, srcl, flags);
}

const sc_string_t *sc_compile_glsl(sc_symbol_t target, sc_label_t *srcl, uint64_t flags) {
    using namespace scopes;
    return compile_glsl(target, srcl, flags);
}

void sc_compile_object(const sc_string_t *path, sc_scope_t *table, uint64_t flags) {
    using namespace scopes;
    compile_object(path, table, flags);
}

void sc_enter_solver_cli () {
    using namespace scopes;
    enable_specializer_step_debugger();
}

size_t sc_verify_stack () {
    using namespace scopes;
    size_t ssz = memory_stack_size();
    if (ssz >= SCOPES_MAX_STACK_SIZE) {
        location_error(String::from("verify-stack!: stack overflow"));
    }
    return ssz;
}

// stdin/out
////////////////////////////////////////////////////////////////////////////////

const sc_string_t *sc_default_styler(sc_symbol_t style, const sc_string_t *str) {
    using namespace scopes;
    StyledString ss;
    if (!style.is_known()) {
        location_error(String::from("illegal style"));
    }
    ss.out << Style(style.known_value()) << str->data << Style_None;
    return ss.str();
}

sc_string_bool_tuple_t sc_prompt(const sc_string_t *s, const sc_string_t *pre) {
    using namespace scopes;
    if (pre->count) {
        linenoisePreloadBuffer(pre->data);
    }
    char *r = linenoise(s->data);
    if (!r) {
        return { Symbol(SYM_Unnamed).name(), false };
    }
    linenoiseHistoryAdd(r);
    return { String::from_cstr(r), true };
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

void sc_set_globals(sc_scope_t *s) {
    using namespace scopes;
    globals = s;
}

// Error Handling
////////////////////////////////////////////////////////////////////////////////

void sc_error(const sc_string_t *msg) {
    using namespace scopes;
    const Exception *exc = new Exception(nullptr, msg);
    error(exc);
}

void sc_anchor_error(const sc_string_t *msg) {
    using namespace scopes;
    location_error(msg);
}

void sc_raise(sc_any_t value) {
    using namespace scopes;
    error(value);
}

void sc_set_signal_abort(bool value) {
    using namespace scopes;
    signal_abort = value;
}

sc_exception_pad_t *sc_set_exception_pad(sc_exception_pad_t *pad) {
    using namespace scopes;
    ExceptionPad *last_exc_pad = _exc_pad;
    _exc_pad = pad;
    return last_exc_pad;
}

sc_any_t sc_exception_value(sc_exception_pad_t *pad) {
    using namespace scopes;
    return pad->value;
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

typedef std::unordered_map<const List *, const List *> MemoMap;
static MemoMap memo_map;

}

sc_list_bool_tuple_t sc_map_load(const sc_list_t *key) {
    using namespace scopes;
    auto it = memo_map.find(key);
    if (it != memo_map.end()) {
        return {it->second, true };
    } else {
        return {EOL, false };
    }
}

void sc_map_store(const sc_list_t *key, const sc_list_t *value) {
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

sc_scope_t *sc_import_c(const sc_string_t *path,
    const sc_string_t *content, const sc_list_t *arglist) {
    using namespace scopes;
    std::vector<std::string> args;
    while (arglist) {
        auto &&at = arglist->at;
        if (at.type == TYPE_String) {
            args.push_back(at.string->data);
        }
        arglist = arglist->next;
    }
    return import_c_module(path->data, args, content->data);
}

void sc_load_library(const sc_string_t *name) {
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
        location_error(ss.str());
    }
#endif
    if (LLVMLoadLibraryPermanently(name->data)) {
        StyledString ss;
        ss.out << "error loading library " << name;
        location_error(ss.str());
    }
}

// Anchor
////////////////////////////////////////////////////////////////////////////////

void sc_set_active_anchor(const sc_anchor_t *anchor) {
    using namespace scopes;
    set_active_anchor(anchor);
}

const sc_anchor_t *sc_get_active_anchor() {
    using namespace scopes;
    return get_active_anchor();
}

// Scope
////////////////////////////////////////////////////////////////////////////////

void sc_scope_set_symbol(sc_scope_t *scope, sc_symbol_t sym, sc_any_t value) {
    scope->bind(sym, value);
}

sc_any_bool_tuple_t sc_scope_at(sc_scope_t *scope, sc_symbol_t key) {
    using namespace scopes;
    Any result = none;
    bool ok = scope->lookup(key, result);
    return { result, ok };
}

sc_any_bool_tuple_t sc_scope_local_at(sc_scope_t *scope, sc_symbol_t key) {
    using namespace scopes;
    Any result = none;
    bool ok = scope->lookup_local(key, result);
    return { result, ok };
}

const sc_string_t *sc_scope_get_docstring(sc_scope_t *scope, sc_symbol_t key) {
    using namespace scopes;
    if (key == SYM_Unnamed) {
        if (scope->doc) return scope->doc;
    } else {
        AnyDoc entry = { none, nullptr };
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
        AnyDoc entry = { none, nullptr };
        if (!scope->lookup_local(key, entry)) {
            location_error(
                String::from("attempting to set a docstring for a non-local name"));
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

sc_symbol_any_tuple_t sc_scope_next(sc_scope_t *scope, sc_symbol_t key) {
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
        if (is_typed(it->second.value)) {
            return { it->first, it->second.value };
        }
        it++;
    }
    return { SYM_Unnamed, none };
}

// Symbol
////////////////////////////////////////////////////////////////////////////////

sc_symbol_t sc_symbol_new(const sc_string_t *str) {
    using namespace scopes;
    return Symbol(str);
}

const sc_string_t *sc_symbol_to_string(sc_symbol_t sym) {
    using namespace scopes;
    return sym.name();
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
bool sc_string_match(const sc_string_t *pattern, const sc_string_t *text) {
    using namespace scopes;
    auto it = pattern_cache.find(pattern);
    regexp::Reprog *m = nullptr;
    if (it == pattern_cache.end()) {
        const char *error = nullptr;
        m = regexp::regcomp(pattern->data, 0, &error);
        if (error) {
            const String *err = String::from_cstr(error);
            regexp::regfree(m);
            location_error(err);
        }
        pattern_cache.insert({ pattern, m });
    } else {
        m = it->second;
    }
    return (regexp::regexec(m, text->data, nullptr, 0) == 0);
}

size_t sc_string_count(sc_string_t *str) {
    using namespace scopes;
    return str->count;
}

sc_rawstring_size_t_tuple_t sc_string_buffer(sc_string_t *str) {
    using namespace scopes;
    return {str->data, str->count};
}

const sc_string_t *sc_string_lslice(sc_string_t *str, size_t offset) {
    using namespace scopes;
    if (!offset) return str;
    if (offset >= str->count)
        return Symbol(SYM_Unnamed).name();
    return String::from(str->data + offset, str->count - offset);
}

const sc_string_t *sc_string_rslice(sc_string_t *str, size_t offset) {
    using namespace scopes;
    if (!offset) return Symbol(SYM_Unnamed).name();
    if (offset >= str->count) return str;
    return String::from(str->data, offset);
}

// Any
////////////////////////////////////////////////////////////////////////////////

const sc_string_t *sc_any_repr(sc_any_t value) {
    using namespace scopes;
    StyledString ss;
    value.stream(ss.out, false);
    return ss.str();
}

const sc_string_t *sc_any_string(sc_any_t value) {
    using namespace scopes;
    auto ss = StyledString::plain();
    ss.out << value;
    return ss.str();
}

bool sc_any_eq(sc_any_t a, sc_any_t b) {
    using namespace scopes;
    return a == b;
}

// List
////////////////////////////////////////////////////////////////////////////////

const sc_list_t *sc_list_cons(sc_any_t at, const sc_list_t *next) {
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

sc_any_list_tuple_t sc_list_decons(const sc_list_t *l) {
    using namespace scopes;
    if (l)
        return { l->at, l->next };
    else
        return { none, nullptr };
}

size_t sc_list_count(const sc_list_t *l) {
    using namespace scopes;
    return l?l->count:0;
}

sc_any_t sc_list_at(const sc_list_t *l) {
    using namespace scopes;
    return l?l->at:none;
}

const sc_list_t *sc_list_next(const sc_list_t *l) {
    using namespace scopes;
    return l?l->next:EOL;
}

const sc_list_t *sc_list_reverse(const sc_list_t *l) {
    return reverse_list(l);
}

// Syntax
////////////////////////////////////////////////////////////////////////////////

const sc_syntax_t *sc_syntax_from_path(const sc_string_t *path) {
    using namespace scopes;
    auto sf = SourceFile::from_file(path);
    if (!sf) {
        StyledString ss;
        ss.out << "no such file: " << path;
        location_error(ss.str());
    }
    LexerParser parser(sf);
    return parser.parse();
}

const sc_syntax_t *sc_syntax_from_string(const sc_string_t *str) {
    using namespace scopes;
    auto sf = SourceFile::from_string(Symbol("<string>"), str);
    assert(sf);
    LexerParser parser(sf);
    return parser.parse();
}

const sc_syntax_t *sc_syntax_new(const sc_anchor_t *anchor, sc_any_t value, bool quoted) {
    using namespace scopes;
    return Syntax::from(anchor, value, quoted);
}

sc_any_t sc_syntax_wrap(const sc_anchor_t *anchor, sc_any_t e, bool quoted) {
    using namespace scopes;
    return wrap_syntax(anchor, e, quoted);
}

sc_any_t sc_syntax_strip(sc_any_t e) {
    using namespace scopes;
    return strip_syntax(e);
}

// Types
////////////////////////////////////////////////////////////////////////////////

sc_any_bool_tuple_t sc_type_at(const sc_type_t *T, sc_symbol_t key) {
    using namespace scopes;
    Any result = none;
    bool ok = T->lookup(key, result);
    return { result, ok };
}

size_t sc_type_sizeof(const sc_type_t *T) {
    using namespace scopes;
    return size_of(T);
}

size_t sc_type_alignof(const sc_type_t *T) {
    using namespace scopes;
    return align_of(T);
}

int sc_type_countof(const sc_type_t *T) {
    using namespace scopes;
    T = storage_type(T);
    switch(T->kind()) {
    case TK_Pointer:
    case TK_Extern:
    case TK_Image:
    case TK_SampledImage:
        return 1;
    case TK_Array: return cast<ArrayType>(T)->count;
    case TK_Vector: return cast<VectorType>(T)->count;
    case TK_Tuple: return cast<TupleType>(T)->types.size();
    case TK_Union: return cast<UnionType>(T)->types.size();
    case TK_Function:  return cast<FunctionType>(T)->argument_types.size() + 1;
    default:  break;
    }
    return 0;
}

const sc_type_t *sc_type_element_at(const sc_type_t *T, int i) {
    using namespace scopes;
    T = storage_type(T);
    switch(T->kind()) {
    case TK_Pointer: return cast<PointerType>(T)->element_type;
    case TK_Array: return cast<ArrayType>(T)->element_type;
    case TK_Vector: return cast<VectorType>(T)->element_type;
    case TK_Tuple: return cast<TupleType>(T)->type_at_index(i);
    case TK_Union: return cast<UnionType>(T)->type_at_index(i);
    case TK_Function: return cast<FunctionType>(T)->type_at_index(i);
    case TK_Extern: return cast<ExternType>(T)->pointer_type;
    case TK_Image: return cast<ImageType>(T)->type;
    case TK_SampledImage: return cast<SampledImageType>(T)->type;
    default: {
        StyledString ss;
        ss.out << "storage type " << T << " has no elements" << std::endl;
        location_error(ss.str());
    } break;
    }
    return nullptr;
}

int sc_type_field_index(const sc_type_t *T, sc_symbol_t name) {
    using namespace scopes;
    T = storage_type(T);
    switch(T->kind()) {
    case TK_Tuple: return cast<TupleType>(T)->field_index(name);
    case TK_Union: return cast<UnionType>(T)->field_index(name);
    default: {
        StyledString ss;
        ss.out << "storage type " << T << " has no named elements" << std::endl;
        location_error(ss.str());
    } break;
    }
    return -1;
}

sc_symbol_t sc_type_field_name(const sc_type_t *T, int index) {
    using namespace scopes;
    T = storage_type(T);
    switch(T->kind()) {
    case TK_Tuple: return cast<TupleType>(T)->field_name(index);
    case TK_Union: return cast<UnionType>(T)->field_name(index);
    default: {
        StyledString ss;
        ss.out << "storage type " << T << " has no named elements" << std::endl;
        location_error(ss.str());
    } break;
    }
    return SYM_Unnamed;
}

int32_t sc_type_kind(const sc_type_t *T) {
    using namespace scopes;
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

const sc_type_t *sc_type_storage(const sc_type_t *T) {
    using namespace scopes;
    return storage_type(T);
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

sc_symbol_any_tuple_t sc_type_next(const sc_type_t *type, sc_symbol_t key) {
    using namespace scopes;
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
    return { SYM_Unnamed, none };
}

void sc_type_set_symbol(sc_type_t *T, sc_symbol_t sym, sc_any_t value) {
    using namespace scopes;
    const_cast<Type *>(T)->bind(sym, value);
}

// Pointer Type
////////////////////////////////////////////////////////////////////////////////

const sc_type_t *sc_pointer_type(const sc_type_t *T, uint64_t flags, sc_symbol_t storage_class) {
    using namespace scopes;
    return Pointer(T, flags, storage_class);
}

uint64_t sc_pointer_type_get_flags(const sc_type_t *T) {
    using namespace scopes;
    verify_kind<TK_Pointer>(T);
    return cast<PointerType>(T)->flags;
}

const sc_type_t *sc_pointer_type_set_flags(const sc_type_t *T, uint64_t flags) {
    using namespace scopes;
    verify_kind<TK_Pointer>(T);
    auto pt = cast<PointerType>(T);
    return Pointer(pt->element_type, flags, pt->storage_class);
}

sc_symbol_t sc_pointer_type_get_storage_class(const sc_type_t *T) {
    using namespace scopes;
    verify_kind<TK_Pointer>(T);
    return cast<PointerType>(T)->storage_class;
}

const sc_type_t *sc_pointer_type_set_storage_class(const sc_type_t *T, sc_symbol_t storage_class) {
    using namespace scopes;
    verify_kind<TK_Pointer>(T);
    auto pt = cast<PointerType>(T);
    return Pointer(pt->element_type, pt->flags, storage_class);
}

const sc_type_t *sc_pointer_type_set_element_type(const sc_type_t *T, const sc_type_t *ET) {
    using namespace scopes;
    verify_kind<TK_Pointer>(T);
    auto pt = cast<PointerType>(T);
    return Pointer(ET, pt->flags, pt->storage_class);
}

// Extern Type
////////////////////////////////////////////////////////////////////////////////

int32_t sc_extern_type_location(const sc_type_t *T) {
    using namespace scopes;
    T = storage_type(T);
    verify_kind<TK_Extern>(T);
    return cast<ExternType>(T)->location;
}

int32_t sc_extern_type_binding(const sc_type_t *T) {
    using namespace scopes;
    T = storage_type(T);
    verify_kind<TK_Extern>(T);
    return cast<ExternType>(T)->binding;
}

// numerical types
////////////////////////////////////////////////////////////////////////////////

int32_t sc_type_bitcountof(const sc_type_t *T) {
    using namespace scopes;
    T = storage_type(T);
    switch(T->kind()) {
    case TK_Integer:
        return cast<IntegerType>(T)->width;
    case TK_Real:
        return cast<RealType>(T)->width;
    default: {
        StyledString ss;
        ss.out << "type " << T << " has no bitcount" << std::endl;
        location_error(ss.str());
    } break;
    }
    return 0;
}

// Integer Type
////////////////////////////////////////////////////////////////////////////////

const sc_type_t *sc_integer_type(int width, bool issigned) {
    using namespace scopes;
    return Integer(width, issigned);
}

bool sc_integer_type_is_signed(const sc_type_t *T) {
    using namespace scopes;
    T = storage_type(T);
    verify_kind<TK_Integer>(T);
    return cast<IntegerType>(T)->issigned;
}

// Typename Type
////////////////////////////////////////////////////////////////////////////////

const sc_type_t *sc_typename_type(const sc_string_t *str) {
    using namespace scopes;
    return Typename(str);
}

void sc_typename_type_set_super(const sc_type_t *T, const sc_type_t *ST) {
    using namespace scopes;
    verify_kind<TK_Typename>(T);
    verify_kind<TK_Typename>(ST);
    // if T <=: ST, the operation is illegal
    const Type *S = ST;
    while (S) {
        if (S == T) {
            StyledString ss;
            ss.out << "typename " << ST << " can not be a supertype of " << T;
            location_error(ss.str());
        }
        if (S == TYPE_Typename)
            break;
        S = superof(S);
    }
    auto tn = cast<TypenameType>(T);
    const_cast<TypenameType *>(tn)->super_type = ST;
}

const sc_type_t *sc_typename_type_get_super(const sc_type_t *T) {
    using namespace scopes;
    return superof(T);
}

void sc_typename_type_set_storage(const sc_type_t *T, const sc_type_t *T2) {
    using namespace scopes;
    verify_kind<TK_Typename>(T);
    cast<TypenameType>(const_cast<Type *>(T))->finalize(T2);
}

// Array Type
////////////////////////////////////////////////////////////////////////////////

const sc_type_t *sc_array_type(const sc_type_t *element_type, size_t count) {
    using namespace scopes;
    return Array(element_type, count);
}

// Vector Type
////////////////////////////////////////////////////////////////////////////////

const sc_type_t *sc_vector_type(const sc_type_t *element_type, size_t count) {
    using namespace scopes;
    return Vector(element_type, count);
}

// Tuple Type
////////////////////////////////////////////////////////////////////////////////

const sc_type_t *sc_tuple_type(int numtypes, const sc_type_t **typeargs) {
    using namespace scopes;
    ArgTypes types;
    types.reserve(numtypes);
    for (int i = 0; i < numtypes; ++i) {
        types.push_back(typeargs[i]);
    }
    return Tuple(types);
}

// Function Type
////////////////////////////////////////////////////////////////////////////////

bool sc_function_type_is_variadic(const sc_type_t *T) {
    using namespace scopes;
    verify_kind<TK_Function>(T);
    auto ft = cast<FunctionType>(T);
    return ft->flags & FF_Variadic;
}

const sc_type_t *sc_function_type(const sc_type_t *return_type,
    int numtypes, const sc_type_t **typeargs) {
    using namespace scopes;
    ArgTypes types;
    types.reserve(numtypes);
    for (int i = 0; i < numtypes; ++i) {
        types.push_back(typeargs[i]);
    }
    return Function(return_type, types);
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
    return Image(_type, _dim, _depth, _arrayed, _multisampled, _sampled, _format, _access);
}

// Sampled Image Type
////////////////////////////////////////////////////////////////////////////////

const sc_type_t *sc_sampled_image_type(const sc_type_t *_type) {
    using namespace scopes;
    return SampledImage(cast<ImageType>(_type));
}

// Parameter
////////////////////////////////////////////////////////////////////////////////

sc_parameter_t *sc_parameter_new(const sc_anchor_t *anchor, sc_symbol_t symbol, const sc_type_t *type) {
    using namespace scopes;
    if (ends_with_parenthesis(symbol)) {
        return Parameter::variadic_from(anchor, symbol, type);
    } else {
        return Parameter::from(anchor, symbol, type);
    }
}

int sc_parameter_index(const sc_parameter_t *param) {
    using namespace scopes;
    return param->index;
}

sc_symbol_t sc_parameter_name(const sc_parameter_t *param) {
    using namespace scopes;
    return param->name;
}

const sc_type_t *sc_parameter_type(const sc_parameter_t *param) {
    using namespace scopes;
    return param->type;
}

// Label
////////////////////////////////////////////////////////////////////////////////

void sc_label_dump(sc_label_t *label) {
    using namespace scopes;
    StyledStream ss(SCOPES_CERR);
    stream_label(ss, label, StreamLabelFormat::debug_all());
}

void sc_label_set_inline (sc_label_t *label) {
    using namespace scopes;
    label->set_inline();
}

const sc_anchor_t *sc_label_anchor(sc_label_t *label) {
    using namespace scopes;
    return label->anchor;
}

const sc_anchor_t *sc_label_body_anchor(sc_label_t *label) {
    using namespace scopes;
    return label->body.anchor;
}

sc_symbol_t sc_label_name(sc_label_t *label) {
    using namespace scopes;
    return label->name;
}

int32_t sc_label_parameter_count(sc_label_t *label) {
    using namespace scopes;
    return label->params.size();
}

sc_parameter_t *sc_label_parameter(sc_label_t *label, int32_t index) {
    using namespace scopes;
    verify_range(index, label->params.size());
    return label->params[index];
}

size_t sc_label_countof_reachable(sc_label_t *label) {
    using namespace scopes;
    std::unordered_set<Label *> labels;
    label->build_reachable(labels);
    return labels.size();
}

const sc_string_t *sc_label_docstring(sc_label_t *label) {
    using namespace scopes;
    if (label->docstring) {
        return label->docstring;
    } else {
        return Symbol(SYM_Unnamed).name();
    }
}

sc_any_t sc_label_get_enter(sc_label_t *label) {
    using namespace scopes;
    return label->body.enter;
}

void sc_label_set_enter(sc_label_t *label, sc_any_t value) {
    using namespace scopes;
    label->body.enter = value;
}

const sc_list_t *sc_label_get_arguments(sc_label_t *label) {
    using namespace scopes;
    const List *result = EOL;
    size_t i = label->body.args.size();
    while (i) {
        i--;
        auto &&arg = label->body.args[i];
        result = List::from(arg.value, result);
    }
    return result;
}

void sc_label_set_arguments(sc_label_t *label, const sc_list_t *list) {
    using namespace scopes;
    label->body.args.clear();
    if (!list)
        return;
    label->body.args.reserve(list->count);
    while (list) {
        label->body.args.push_back({SYM_Unnamed, list->at});
        list = list->next;
    }
}

const sc_list_t *sc_label_get_keyed(sc_label_t *label) {
    using namespace scopes;
    const List *result = EOL;
    size_t i = label->body.args.size();
    while (i) {
        i--;
        auto &&arg = label->body.args[i];
        result = List::from(List::from({Any(arg.key), arg.value}), result);
    }
    return result;
}

void sc_label_set_keyed(sc_label_t *label, const sc_list_t *list) {
    using namespace scopes;
    label->body.args.clear();
    if (!list)
        return;
    label->body.args.reserve(list->count);
    while (list) {
        const List *pair = list->at;
        Symbol key = SYM_Unnamed;
        Any value = none;
        if (pair->count != 2) {
            location_error(String::from("each argument must be a key/value pair"));
        }
        pair->at.verify(TYPE_Symbol);
        key = pair->at.symbol;
        value = pair->next->at;
        label->body.args.push_back({key, value});
        list = list->next;
    }
}

const sc_list_t *sc_label_get_parameters(sc_label_t *label) {
    using namespace scopes;
    const List *result = EOL;
    size_t i = label->params.size();
    while (i) {
        i--;
        auto &&arg = label->params[i];
        result = List::from(arg, result);
    }
    return result;
}

sc_label_t *sc_label_new_cont() {
    using namespace scopes;
    Label *label = Label::continuation_from(get_active_anchor(), SYM_Unnamed);
    label->unset_template();
    label->set_inline();
    label->body.anchor = label->anchor;
    return label;
}

sc_label_t *sc_label_new_cont_template() {
    using namespace scopes;
    Label *label = Label::continuation_from(get_active_anchor(), SYM_Unnamed);
    label->set_inline();
    label->body.anchor = label->anchor;
    return label;
}

sc_label_t *sc_label_new_function_template() {
    using namespace scopes;
    Label *label = Label::function_from(get_active_anchor(), SYM_Unnamed);
    label->body.anchor = label->anchor;
    return label;
}

sc_label_t *sc_label_new_inline_template() {
    using namespace scopes;
    Label *label = Label::function_from(get_active_anchor(), SYM_Unnamed);
    label->set_inline();
    label->body.anchor = label->anchor;
    return label;
}

void sc_label_set_complete(sc_label_t *label) {
    label->body.set_complete();
}

void sc_label_append_parameter(sc_label_t *label, sc_parameter_t *param) {
    using namespace scopes;
    if (param->label) {
        location_error(String::from("attempting to append parameter that's already owned by another label"));
    }
    label->append(param);
}

const sc_type_t *sc_label_function_type(sc_label_t *label) {
    using namespace scopes;
    return label->get_function_type();
}

void sc_label_set_rawcall(sc_label_t *label) {
    using namespace scopes;
    label->body.set_rawcall();
}

sc_frame_t *sc_label_frame(sc_label_t *label) {
    using namespace scopes;
    auto frame = label->frame;
    return frame?frame:Frame::root;
}

// Label
////////////////////////////////////////////////////////////////////////////////

void sc_frame_dump(sc_frame_t *frame) {
    using namespace scopes;
    StyledStream ss(SCOPES_CERR);
    stream_frame(ss, frame, StreamFrameFormat::single());
}

sc_frame_t *sc_frame_root() {
    using namespace scopes;
    return Frame::root;
}

// Closure
////////////////////////////////////////////////////////////////////////////////

const sc_closure_t *sc_closure_new(sc_label_t *label, sc_frame_t *frame) {
    using namespace scopes;
    return Closure::from(label, frame);
}

sc_label_t *sc_closure_label(const sc_closure_t *closure) {
    using namespace scopes;
    return closure->label;
}

sc_frame_t *sc_closure_frame(const sc_closure_t *closure) {
    using namespace scopes;
    return closure->frame;
}



} // extern "C"

namespace scopes {

//------------------------------------------------------------------------------
// GLOBALS
//------------------------------------------------------------------------------

static void bind_extern(Symbol globalsym, Symbol externsym, const Type *T) {
    Any value(externsym);
    value.type = T;
    globals->bind(globalsym, value);
}

static void bind_extern(Symbol sym, const Type *T) {
    bind_extern(sym, sym, T);
}

void init_globals(int argc, char *argv[]) {
    scopes_argc = argc;
    scopes_argv = argv;

#define DEFINE_C_FUNCTION(SYMBOL, FUNC, RETTYPE, ...) \
    globals->bind(SYMBOL, \
        Any::from_pointer(Pointer(Function(RETTYPE, { __VA_ARGS__ }), \
            PTF_NonWritable, SYM_Unnamed), (void *)FUNC));
#define DEFINE_C_VARARG_FUNCTION(SYMBOL, FUNC, RETTYPE, ...) \
    globals->bind(SYMBOL, \
        Any::from_pointer(Pointer(Function(RETTYPE, { __VA_ARGS__ }, FF_Variadic), \
            PTF_NonWritable, SYM_Unnamed), (void *)FUNC));
#define DEFINE_RENAME_EXTERN_C_FUNCTION(NAME, FUNC, RETTYPE, ...) \
    (void)FUNC; /* ensure that the symbol is there */ \
    bind_extern(Symbol(#NAME), Symbol(#FUNC), \
        Extern(Function(RETTYPE, { __VA_ARGS__ }), EF_NonWritable));
#define DEFINE_EXTERN_C_FUNCTION(FUNC, RETTYPE, ...) \
    (void)FUNC; /* ensure that the symbol is there */ \
    bind_extern(Symbol(#FUNC), \
        Extern(Function(RETTYPE, { __VA_ARGS__ }), EF_NonWritable));

    const Type *rawstring = NativeROPointer(TYPE_I8);

    DEFINE_EXTERN_C_FUNCTION(sc_compiler_version, Tuple({TYPE_I32, TYPE_I32, TYPE_I32}));
    DEFINE_EXTERN_C_FUNCTION(sc_eval, TYPE_Label, TYPE_Syntax, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_eval_inline, TYPE_Label, TYPE_List, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_typify, TYPE_Label, TYPE_Closure, TYPE_I32, NativeROPointer(TYPE_Type));
    DEFINE_EXTERN_C_FUNCTION(sc_compile, TYPE_Any, TYPE_Label, TYPE_U64);
    DEFINE_EXTERN_C_FUNCTION(sc_compile_spirv, TYPE_String, TYPE_Symbol, TYPE_Label, TYPE_U64);
    DEFINE_EXTERN_C_FUNCTION(sc_compile_glsl, TYPE_String, TYPE_Symbol, TYPE_Label, TYPE_U64);
    DEFINE_EXTERN_C_FUNCTION(sc_compile_object, TYPE_Void, TYPE_String, TYPE_Scope, TYPE_U64);
    DEFINE_EXTERN_C_FUNCTION(sc_enter_solver_cli, TYPE_Void);
    DEFINE_EXTERN_C_FUNCTION(sc_verify_stack, TYPE_USize);
    DEFINE_EXTERN_C_FUNCTION(sc_launch_args, TYPE_List);

    DEFINE_EXTERN_C_FUNCTION(sc_prompt, Tuple({TYPE_String, TYPE_Bool}), TYPE_String, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_set_autocomplete_scope, TYPE_Void, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_default_styler, TYPE_String, TYPE_Symbol, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_format_message, TYPE_String, TYPE_Anchor, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_write, TYPE_Void, TYPE_String);

    DEFINE_EXTERN_C_FUNCTION(sc_is_file, TYPE_Bool, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_is_directory, TYPE_Bool, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_realpath, TYPE_String, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_dirname, TYPE_String, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_basename, TYPE_String, TYPE_String);

    DEFINE_EXTERN_C_FUNCTION(sc_get_globals, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_set_globals, TYPE_Void, TYPE_Scope);

    DEFINE_EXTERN_C_FUNCTION(sc_error, TYPE_Void, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_anchor_error, TYPE_Void, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_raise, TYPE_Void, TYPE_Any);
    DEFINE_EXTERN_C_FUNCTION(sc_abort, TYPE_Void);
    DEFINE_EXTERN_C_FUNCTION(sc_exit, TYPE_Void, TYPE_I32);

    const Type *exception_pad_type = Array(TYPE_U8, sizeof(ExceptionPad));
    const Type *p_exception_pad_type = NativePointer(exception_pad_type);

    DEFINE_EXTERN_C_FUNCTION(sc_set_exception_pad,
        p_exception_pad_type, p_exception_pad_type);
    #ifdef SCOPES_WIN32
    DEFINE_RENAME_EXTERN_C_FUNCTION(sc_setjmp, _setjmpex, TYPE_I32,
        p_exception_pad_type, NativeROPointer(TYPE_I8));
    #else
    DEFINE_RENAME_EXTERN_C_FUNCTION(sc_setjmp, setjmp, TYPE_I32,
        p_exception_pad_type);
    #endif
    DEFINE_EXTERN_C_FUNCTION(sc_exception_value,
        TYPE_Any, p_exception_pad_type);
    DEFINE_EXTERN_C_FUNCTION(sc_set_signal_abort,
        TYPE_Void, TYPE_Bool);

    DEFINE_EXTERN_C_FUNCTION(sc_map_load, Tuple({TYPE_List, TYPE_Bool}), TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_map_store, TYPE_Void, TYPE_List, TYPE_List);

    DEFINE_EXTERN_C_FUNCTION(sc_hash, TYPE_U64, TYPE_U64, TYPE_USize);
    DEFINE_EXTERN_C_FUNCTION(sc_hash2x64, TYPE_U64, TYPE_U64, TYPE_U64);
    DEFINE_EXTERN_C_FUNCTION(sc_hashbytes, TYPE_U64, NativeROPointer(TYPE_I8), TYPE_USize);

    DEFINE_EXTERN_C_FUNCTION(sc_import_c, TYPE_Scope, TYPE_String, TYPE_String, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_load_library, TYPE_Void, TYPE_String);

    DEFINE_EXTERN_C_FUNCTION(sc_get_active_anchor, TYPE_Anchor);
    DEFINE_EXTERN_C_FUNCTION(sc_set_active_anchor, TYPE_Void, TYPE_Anchor);

    DEFINE_EXTERN_C_FUNCTION(sc_scope_at, Tuple({TYPE_Any,TYPE_Bool}), TYPE_Scope, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_local_at, Tuple({TYPE_Any,TYPE_Bool}), TYPE_Scope, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_get_docstring, TYPE_String, TYPE_Scope, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_set_docstring, TYPE_Void, TYPE_Scope, TYPE_Symbol, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_set_symbol, TYPE_Void, TYPE_Scope, TYPE_Symbol, TYPE_Any);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_new, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_clone, TYPE_Scope, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_new_subscope, TYPE_Scope, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_clone_subscope, TYPE_Scope, TYPE_Scope, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_get_parent, TYPE_Scope, TYPE_Scope);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_del_symbol, TYPE_Void, TYPE_Scope, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_scope_next, Tuple({TYPE_Symbol, TYPE_Any}), TYPE_Scope, TYPE_Symbol);

    DEFINE_EXTERN_C_FUNCTION(sc_symbol_new, TYPE_Symbol, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_symbol_to_string, TYPE_String, TYPE_Symbol);

    DEFINE_EXTERN_C_FUNCTION(sc_string_new, TYPE_String, NativeROPointer(TYPE_I8), TYPE_USize);
    DEFINE_EXTERN_C_FUNCTION(sc_string_new_from_cstr, TYPE_String, NativeROPointer(TYPE_I8));
    DEFINE_EXTERN_C_FUNCTION(sc_string_join, TYPE_String, TYPE_String, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_string_match, TYPE_Bool, TYPE_String, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_string_count, TYPE_USize, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_string_buffer, Tuple({rawstring, TYPE_USize}), TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_string_lslice, TYPE_String, TYPE_String, TYPE_USize);
    DEFINE_EXTERN_C_FUNCTION(sc_string_rslice, TYPE_String, TYPE_String, TYPE_USize);

    DEFINE_EXTERN_C_FUNCTION(sc_any_repr, TYPE_String, TYPE_Any);
    DEFINE_EXTERN_C_FUNCTION(sc_any_string, TYPE_String, TYPE_Any);
    DEFINE_EXTERN_C_FUNCTION(sc_any_eq, TYPE_Bool, TYPE_Any, TYPE_Any);

    DEFINE_EXTERN_C_FUNCTION(sc_type_at, Tuple({TYPE_Any,TYPE_Bool}), TYPE_Type, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_type_element_at, TYPE_Type, TYPE_Type, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_type_field_index, TYPE_I32, TYPE_Type, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_type_field_name, TYPE_Symbol, TYPE_Type, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_type_sizeof, TYPE_USize, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_alignof, TYPE_USize, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_countof, TYPE_I32, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_kind, TYPE_I32, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_debug_abi, TYPE_Void, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_storage, TYPE_Type, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_is_opaque, TYPE_Bool, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_string, TYPE_String, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_type_next, Tuple({TYPE_Symbol, TYPE_Any}), TYPE_Type, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_type_set_symbol, TYPE_Void, TYPE_Type, TYPE_Symbol, TYPE_Any);

    DEFINE_EXTERN_C_FUNCTION(sc_pointer_type, TYPE_Type, TYPE_Type, TYPE_U64, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_pointer_type_get_flags, TYPE_U64, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_pointer_type_set_flags, TYPE_Type, TYPE_Type, TYPE_U64);
    DEFINE_EXTERN_C_FUNCTION(sc_pointer_type_get_storage_class, TYPE_Symbol, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_pointer_type_set_storage_class, TYPE_Type, TYPE_Type, TYPE_Symbol);
    DEFINE_EXTERN_C_FUNCTION(sc_pointer_type_set_element_type, TYPE_Type, TYPE_Type, TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_extern_type_location, TYPE_I32, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_extern_type_binding, TYPE_I32, TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_type_bitcountof, TYPE_I32, TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_integer_type, TYPE_Type, TYPE_I32, TYPE_Bool);
    DEFINE_EXTERN_C_FUNCTION(sc_integer_type_is_signed, TYPE_Bool, TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_typename_type, TYPE_Type, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_typename_type_set_super, TYPE_Void, TYPE_Type, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_typename_type_get_super, TYPE_Type, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_typename_type_set_storage, TYPE_Void, TYPE_Type, TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_array_type, TYPE_Type, TYPE_Type, TYPE_USize);

    DEFINE_EXTERN_C_FUNCTION(sc_vector_type, TYPE_Type, TYPE_Type, TYPE_USize);

    DEFINE_EXTERN_C_FUNCTION(sc_tuple_type, TYPE_Type, TYPE_I32, NativeROPointer(TYPE_Type));

    DEFINE_EXTERN_C_FUNCTION(sc_image_type, TYPE_Type,
        TYPE_Type, TYPE_Symbol, TYPE_I32, TYPE_I32, TYPE_I32, TYPE_I32, TYPE_Symbol, TYPE_Symbol);

    DEFINE_EXTERN_C_FUNCTION(sc_sampled_image_type, TYPE_Type, TYPE_Type);

    DEFINE_EXTERN_C_FUNCTION(sc_function_type_is_variadic, TYPE_Bool, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_function_type, TYPE_Type, TYPE_Type, TYPE_I32, NativeROPointer(TYPE_Type));

    DEFINE_EXTERN_C_FUNCTION(sc_list_cons, TYPE_List, TYPE_Any, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_dump, TYPE_List, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_join, TYPE_List, TYPE_List, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_decons, Tuple({TYPE_Any, TYPE_List}), TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_count, TYPE_USize, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_at, TYPE_Any, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_next, TYPE_List, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_list_reverse, TYPE_List, TYPE_List);

    DEFINE_EXTERN_C_FUNCTION(sc_syntax_from_path, TYPE_Syntax, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_syntax_from_string, TYPE_Syntax, TYPE_String);
    DEFINE_EXTERN_C_FUNCTION(sc_syntax_new, TYPE_Syntax, TYPE_Anchor, TYPE_Any, TYPE_Bool);
    DEFINE_EXTERN_C_FUNCTION(sc_syntax_wrap, TYPE_Any, TYPE_Anchor, TYPE_Any, TYPE_Bool);
    DEFINE_EXTERN_C_FUNCTION(sc_syntax_strip, TYPE_Any, TYPE_Any);

    DEFINE_EXTERN_C_FUNCTION(sc_parameter_new, TYPE_Parameter, TYPE_Anchor, TYPE_Symbol, TYPE_Type);
    DEFINE_EXTERN_C_FUNCTION(sc_parameter_index, TYPE_I32, TYPE_Parameter);
    DEFINE_EXTERN_C_FUNCTION(sc_parameter_name, TYPE_Symbol, TYPE_Parameter);
    DEFINE_EXTERN_C_FUNCTION(sc_parameter_type, TYPE_Type, TYPE_Parameter);

    DEFINE_EXTERN_C_FUNCTION(sc_label_dump, TYPE_Void, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_anchor, TYPE_Anchor, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_body_anchor, TYPE_Anchor, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_parameter_count, TYPE_I32, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_parameter, TYPE_Parameter, TYPE_Label, TYPE_I32);
    DEFINE_EXTERN_C_FUNCTION(sc_label_name, TYPE_Symbol, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_docstring, TYPE_String, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_set_inline, TYPE_Void, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_countof_reachable, TYPE_USize, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_get_enter, TYPE_Any, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_set_enter, TYPE_Void, TYPE_Label, TYPE_Any);
    DEFINE_EXTERN_C_FUNCTION(sc_label_get_arguments, TYPE_List, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_set_arguments, TYPE_Void, TYPE_Label, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_label_get_keyed, TYPE_List, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_set_keyed, TYPE_Void, TYPE_Label, TYPE_List);
    DEFINE_EXTERN_C_FUNCTION(sc_label_get_parameters, TYPE_List, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_new_cont, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_new_cont_template, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_new_function_template, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_new_inline_template, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_set_complete, TYPE_Void, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_append_parameter, TYPE_Void, TYPE_Label, TYPE_Parameter);
    DEFINE_EXTERN_C_FUNCTION(sc_label_function_type, TYPE_Type, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_set_rawcall, TYPE_Void, TYPE_Label);
    DEFINE_EXTERN_C_FUNCTION(sc_label_frame, TYPE_Frame, TYPE_Label);

    DEFINE_EXTERN_C_FUNCTION(sc_frame_dump, TYPE_Void, TYPE_Frame);
    DEFINE_EXTERN_C_FUNCTION(sc_frame_root, TYPE_Frame);

    DEFINE_EXTERN_C_FUNCTION(sc_closure_new, TYPE_Closure, TYPE_Label, TYPE_Frame);
    DEFINE_EXTERN_C_FUNCTION(sc_closure_label, TYPE_Label, TYPE_Closure);
    DEFINE_EXTERN_C_FUNCTION(sc_closure_frame, TYPE_Frame, TYPE_Closure);

#undef DEFINE_EXTERN_C_FUNCTION

    auto stub_file = SourceFile::from_string(Symbol("<internal>"), String::from_cstr(""));
    auto stub_anchor = Anchor::from(stub_file, 1, 1);

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
    globals->bind(Symbol("operating-system"), Symbol(SCOPES_SYM_OS));
#undef SCOPES_SYM_OS

    globals->bind(Symbol("unroll-limit"), SCOPES_MAX_RECURSIONS);
    globals->bind(KW_True, true);
    globals->bind(KW_False, false);
    globals->bind(Symbol("noreturn"), NoReturnLabel());
    globals->bind(KW_ListEmpty, EOL);
    globals->bind(KW_None, none);
    globals->bind(Symbol("unnamed"), Symbol(SYM_Unnamed));
    globals->bind(SYM_CompilerDir,
        String::from(scopes_compiler_dir, strlen(scopes_compiler_dir)));
    globals->bind(SYM_CompilerPath,
        String::from(scopes_compiler_path, strlen(scopes_compiler_path)));
    globals->bind(SYM_DebugBuild, scopes_is_debug());
    globals->bind(SYM_CompilerTimestamp,
        String::from_cstr(scopes_compile_time_date()));

    for (uint64_t i = STYLE_FIRST; i <= STYLE_LAST; ++i) {
        Symbol sym = Symbol((KnownSymbol)i);
        globals->bind(sym, sym);
    }

    globals->bind(Symbol("exception-pad-type"), exception_pad_type);

#define T(TYPE, NAME) \
    globals->bind(Symbol(NAME), TYPE);
B_TYPES()
#undef T

#define T(NAME, BNAME, CLASS) \
    globals->bind(Symbol(BNAME), (int32_t)NAME);
    B_TYPE_KIND()
#undef T

    globals->bind(Symbol("pointer-flag-non-readable"), (uint64_t)PTF_NonReadable);
    globals->bind(Symbol("pointer-flag-non-writable"), (uint64_t)PTF_NonWritable);

    globals->bind(Symbol(SYM_DumpDisassembly), (uint64_t)CF_DumpDisassembly);
    globals->bind(Symbol(SYM_DumpModule), (uint64_t)CF_DumpModule);
    globals->bind(Symbol(SYM_DumpFunction), (uint64_t)CF_DumpFunction);
    globals->bind(Symbol(SYM_DumpTime), (uint64_t)CF_DumpTime);
    globals->bind(Symbol(SYM_NoDebugInfo), (uint64_t)CF_NoDebugInfo);
    globals->bind(Symbol(SYM_O1), (uint64_t)CF_O1);
    globals->bind(Symbol(SYM_O2), (uint64_t)CF_O2);
    globals->bind(Symbol(SYM_O3), (uint64_t)CF_O3);

#define T(NAME) globals->bind(NAME, Builtin(NAME));
#define T0(NAME, STR) globals->bind(NAME, Builtin(NAME));
#define T1 T2
#define T2T T2
#define T2(UNAME, LNAME, PFIX, OP) \
    globals->bind(FN_ ## UNAME ## PFIX, Builtin(FN_ ## UNAME ## PFIX));
    B_GLOBALS()
#undef T
#undef T0
#undef T1
#undef T2
#undef T2T

    linenoiseSetCompletionCallback(prompt_completion_cb);

}

} // namespace scopes
