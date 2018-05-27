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
#include "main.hpp"
#include "gc.hpp"
#include "compiler_flags.hpp"
#include "hash.hpp"

#include "scopes.h"

#include <limits.h>
#include <string.h>
#include <sys/stat.h>
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
// GLOBALS
//------------------------------------------------------------------------------

typedef struct { int x,y; } I2;
typedef struct { int x,y,z; } I3;

static const String *f_repr(Any value) {
    StyledString ss;
    value.stream(ss.out, false);
    return ss.str();
}

static const String *f_any_string(Any value) {
    auto ss = StyledString::plain();
    ss.out << value;
    return ss.str();
}

static void f_write(const String *value) {
    fputs(value->data, stdout);
}

static Scope *f_import_c(const String *path,
    const String *content, const List *arglist) {
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

static void f_dump_label(Label *label) {
    StyledStream ss(std::cerr);
    stream_label(ss, label, StreamLabelFormat::debug_all());
}

static void f_dump_frame(Frame *frame) {
    StyledStream ss(std::cerr);
    stream_frame(ss, frame, StreamFrameFormat::single());
}

static const List *f_dump_list(const List *l) {
    StyledStream ss(std::cerr);
    stream_expr(ss, l, StreamExprFormat());
    return l;
}

typedef struct { Any result; bool ok; } AnyBoolPair;
static AnyBoolPair f_scope_at(Scope *scope, Symbol key) {
    Any result = none;
    bool ok = scope->lookup(key, result);
    return { result, ok };
}

static AnyBoolPair f_scope_local_at(Scope *scope, Symbol key) {
    Any result = none;
    bool ok = scope->lookup_local(key, result);
    return { result, ok };
}

static AnyBoolPair f_type_at(const Type *T, Symbol key) {
    Any result = none;
    bool ok = T->lookup(key, result);
    return { result, ok };
}

static const String *f_scope_docstring(Scope *scope, Symbol key) {
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

static void f_scope_set_docstring(Scope *scope, Symbol key, const String *str) {
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

static Symbol f_symbol_new(const String *str) {
    return Symbol(str);
}

static const String *f_string_join(const String *a, const String *b) {
    return String::join(a,b);
}

static size_t f_sizeof(const Type *T) {
    return size_of(T);
}

static size_t f_alignof(const Type *T) {
    return align_of(T);
}

int f_type_countof(const Type *T) {
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

static const Type *f_elementtype(const Type *T, int i) {
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

static int f_elementindex(const Type *T, Symbol name) {
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

static Symbol f_elementname(const Type *T, int index) {
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

static const Type *f_pointertype(const Type *T, uint64_t flags, Symbol storage_class) {
    return Pointer(T, flags, storage_class);
}

static uint64_t f_pointer_type_flags(const Type *T) {
    verify_kind<TK_Pointer>(T);
    return cast<PointerType>(T)->flags;
}

static const Type *f_pointer_type_set_flags(const Type *T, uint64_t flags) {
    verify_kind<TK_Pointer>(T);
    auto pt = cast<PointerType>(T);
    return Pointer(pt->element_type, flags, pt->storage_class);
}

static const Symbol f_pointer_type_storage_class(const Type *T) {
    verify_kind<TK_Pointer>(T);
    return cast<PointerType>(T)->storage_class;
}

static int32_t f_extern_type_location(const Type *T) {
    T = storage_type(T);
    verify_kind<TK_Extern>(T);
    return cast<ExternType>(T)->location;
}

static int32_t f_extern_type_binding(const Type *T) {
    T = storage_type(T);
    verify_kind<TK_Extern>(T);
    return cast<ExternType>(T)->binding;
}

static const Type *f_pointer_type_set_storage_class(const Type *T, Symbol storage_class) {
    verify_kind<TK_Pointer>(T);
    auto pt = cast<PointerType>(T);
    return Pointer(pt->element_type, pt->flags, storage_class);
}

static const Type *f_pointer_type_set_element_type(const Type *T, const Type *ET) {
    verify_kind<TK_Pointer>(T);
    auto pt = cast<PointerType>(T);
    return Pointer(ET, pt->flags, pt->storage_class);
}

static const List *f_list_cons(Any at, const List *next) {
    return List::from(at, next);
}

static int32_t f_type_kind(const Type *T) {
    return T->kind();
}

static void f_type_debug_abi(const Type *T) {
    ABIClass classes[MAX_ABI_CLASSES];
    size_t sz = abi_classify(T, classes);
    StyledStream ss(std::cout);
    ss << T << " -> " << sz;
    for (size_t i = 0; i < sz; ++i) {
        ss << " " << abi_class_to_string(classes[i]);
    }
    ss << std::endl;
}

static int32_t f_bitcountof(const Type *T) {
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

static bool f_issigned(const Type *T) {
    T = storage_type(T);
    verify_kind<TK_Integer>(T);
    return cast<IntegerType>(T)->issigned;
}

static const Type *f_type_storage(const Type *T) {
    return storage_type(T);
}

static void f_error(const String *msg) {
    const Exception *exc = new Exception(nullptr, msg);
    error(exc);
}

static void f_anchor_error(const String *msg) {
    location_error(msg);
}

static void f_raise(Any value) {
    error(value);
}

static void f_set_anchor(const Anchor *anchor) {
    set_active_anchor(anchor);
}

static const Type *f_integer_type(int width, bool issigned) {
    return Integer(width, issigned);
}

static const Type *f_typename_type(const String *str) {
    return Typename(str);
}

static I3 f_compiler_version() {
    return {
        SCOPES_VERSION_MAJOR,
        SCOPES_VERSION_MINOR,
        SCOPES_VERSION_PATCH };
}

static const Syntax *f_syntax_new(const Anchor *anchor, Any value, bool quoted) {
    return Syntax::from(anchor, value, quoted);
}

static Parameter *f_parameter_new(const Anchor *anchor, Symbol symbol, const Type *type) {
    if (ends_with_parenthesis(symbol)) {
        return Parameter::variadic_from(anchor, symbol, type);
    } else {
        return Parameter::from(anchor, symbol, type);
    }
}

static const String *f_realpath(const String *path) {
    char buf[PATH_MAX];
    auto result = realpath(path->data, buf);
    if (!result) {
        return Symbol(SYM_Unnamed).name();
    } else {
        return String::from_cstr(result);
    }
}

static const String *f_dirname(const String *path) {
    auto pathcopy = strdup(path->data);
    auto result = String::from_cstr(dirname(pathcopy));
    free(pathcopy);
    return result;
}

static const String *f_basename(const String *path) {
    auto pathcopy = strdup(path->data);
    auto result = String::from_cstr(basename(pathcopy));
    free(pathcopy);
    return result;
}

static int f_parameter_index(const Parameter *param) {
    return param->index;
}

static Symbol f_parameter_name(const Parameter *param) {
    return param->name;
}

static const String *f_string_new(const char *ptr, size_t count) {
    return String::from(ptr, count);
}

static bool f_is_file(const String *path) {
    struct stat s;
    if( stat(path->data,&s) == 0 ) {
        if( s.st_mode & S_IFDIR ) {
        } else if ( s.st_mode & S_IFREG ) {
            return true;
        }
    }
    return false;
}

static bool f_is_directory(const String *path) {
    struct stat s;
    if( stat(path->data,&s) == 0 ) {
        if( s.st_mode & S_IFDIR ) {
            return true;
        }
    }
    return false;
}

static const Syntax *f_list_load(const String *path) {
    auto sf = SourceFile::from_file(path);
    if (!sf) {
        StyledString ss;
        ss.out << "no such file: " << path;
        location_error(ss.str());
    }
    LexerParser parser(sf);
    return parser.parse();
}

static const Syntax *f_list_parse(const String *str) {
    auto sf = SourceFile::from_string(Symbol("<string>"), str);
    assert(sf);
    LexerParser parser(sf);
    return parser.parse();
}

static Scope *f_scope_new() {
    return Scope::from();
}
static Scope *f_scope_clone(Scope *clone) {
    return Scope::from(nullptr, clone);
}
static Scope *f_scope_new_subscope(Scope *scope) {
    return Scope::from(scope);
}
static Scope *f_scope_clone_subscope(Scope *scope, Scope *clone) {
    return Scope::from(scope, clone);
}

static Scope *f_scope_parent(Scope *scope) {
    return scope->parent;
}

static Scope *f_globals() {
    return globals;
}

static void f_set_globals(Scope *s) {
    globals = s;
}

static Label *f_eval(const Syntax *expr, Scope *scope) {
    return specialize(Frame::root, expand_module(expr, scope), {});
}

static void f_set_scope_symbol(Scope *scope, Symbol sym, Any value) {
    scope->bind(sym, value);
}

static void f_del_scope_symbol(Scope *scope, Symbol sym) {
    scope->del(sym);
}

static Label *f_typify(Closure *srcl, int numtypes, const Type **typeargs) {
    if (srcl->label->is_inline()) {
        location_error(String::from("cannot typify inline function"));
    }
    ArgTypes types;
    for (int i = 0; i < numtypes; ++i) {
        types.push_back(typeargs[i]);

    }
    return specialize(srcl->frame, srcl->label, types);
}

static Any f_compile(Label *srcl, uint64_t flags) {
    return compile(srcl, flags);
}

static const String *f_compile_spirv(Symbol target, Label *srcl, uint64_t flags) {
    return compile_spirv(target, srcl, flags);
}

static const String *f_compile_glsl(Symbol target, Label *srcl, uint64_t flags) {
    return compile_glsl(target, srcl, flags);
}

void f_compile_object(const String *path, Scope *table, uint64_t flags) {
    compile_object(path, table, flags);
}

static const Type *f_array_type(const Type *element_type, size_t count) {
    return Array(element_type, count);
}

static const Type *f_vector_type(const Type *element_type, size_t count) {
    return Vector(element_type, count);
}

static const String *f_default_styler(Symbol style, const String *str) {
    StyledString ss;
    if (!style.is_known()) {
        location_error(String::from("illegal style"));
    }
    ss.out << Style(style.known_value()) << str->data << Style_None;
    return ss.str();
}

typedef struct { const String *_0; bool _1; } StringBoolPair;
static StringBoolPair f_prompt(const String *s, const String *pre) {
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

static const Scope *autocomplete_scope = nullptr;
static void f_set_autocomplete_scope(const Scope* scope) {
    autocomplete_scope = scope;
}
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

static const String *f_format_message(const Anchor *anchor, const String *message) {
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

static const String *f_symbol_to_string(Symbol sym) {
    return sym.name();
}

static void f_set_signal_abort(bool value) {
    signal_abort = value;
}

ExceptionPad *f_set_exception_pad(ExceptionPad *pad) {
    ExceptionPad *last_exc_pad = _exc_pad;
    _exc_pad = pad;
    return last_exc_pad;
}

Any f_exception_value(ExceptionPad *pad) {
    return pad->value;
}

static bool f_any_eq(Any a, Any b) {
    return a == b;
}

static const List *f_list_join(List *a, List *b) {
    return List::join(a, b);
}

typedef struct { Any _0; Any _1; } AnyAnyPair;
typedef struct { Symbol _0; Any _1; } SymbolAnyPair;
static SymbolAnyPair f_scope_next(Scope *scope, Symbol key) {
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

static SymbolAnyPair f_type_next(const Type *type, Symbol key) {
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

static std::unordered_map<const String *, regexp::Reprog *> pattern_cache;
static bool f_string_match(const String *pattern, const String *text) {
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

static void f_load_library(const String *name) {
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

static const String *f_type_name(const Type *T) {
    return T->name();
}

static bool f_function_type_is_variadic(const Type *T) {
    verify_kind<TK_Function>(T);
    auto ft = cast<FunctionType>(T);
    return ft->flags & FF_Variadic;
}

static void f_set_typename_super(const Type *T, const Type *ST) {
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

static const Anchor *f_label_anchor(Label *label) {
    return label->anchor;
}

static Symbol f_label_name(Label *label) {
    return label->name;
}

static size_t f_label_parameter_count(Label *label) {
    return label->params.size();
}

static Parameter *f_label_parameter(Label *label, size_t index) {
    verify_range(index, label->params.size());
    return label->params[index];
}

static Label *f_closure_label(const Closure *closure) {
    return closure->label;
}

static Frame *f_closure_frame(const Closure *closure) {
    return closure->frame;
}

size_t f_label_countof_reachable(Label *label) {
    std::unordered_set<Label *> labels;
    label->build_reachable(labels);
    return labels.size();
}

static void f_enter_solver_cli () {
    enable_specializer_step_debugger();
}

static void f_label_setinline (Label *label) {
    label->set_inline();
}

static const String *f_label_docstring(Label *label) {
    if (label->docstring) {
        return label->docstring;
    } else {
        return Symbol(SYM_Unnamed).name();
    }
}

static size_t f_verify_stack () {
    size_t ssz = memory_stack_size();
    if (ssz >= SCOPES_MAX_STACK_SIZE) {
        location_error(String::from("verify-stack!: stack overflow"));
    }
    return ssz;
}

static uint64_t f_hash (uint64_t data, size_t size) {
    return hash_bytes((const char *)&data, (size > 8)?8:size);
}

static uint64_t f_hash2x64(uint64_t a, uint64_t b) {
    return hash2(a, b);
}

static uint64_t f_hashbytes (const char *data, size_t size) {
    return hash_bytes(data, size);
}

void init_globals(int argc, char *argv[]) {

#define DEFINE_C_FUNCTION(SYMBOL, FUNC, RETTYPE, ...) \
    globals->bind(SYMBOL, \
        Any::from_pointer(Pointer(Function(RETTYPE, { __VA_ARGS__ }), \
            PTF_NonWritable, SYM_Unnamed), (void *)FUNC));
#define DEFINE_C_VARARG_FUNCTION(SYMBOL, FUNC, RETTYPE, ...) \
    globals->bind(SYMBOL, \
        Any::from_pointer(Pointer(Function(RETTYPE, { __VA_ARGS__ }, FF_Variadic), \
            PTF_NonWritable, SYM_Unnamed), (void *)FUNC));
#define DEFINE_PURE_C_FUNCTION(SYMBOL, FUNC, RETTYPE, ...) \
    globals->bind(SYMBOL, \
        Any::from_pointer(Pointer(Function(RETTYPE, { __VA_ARGS__ }, FF_Pure), \
            PTF_NonWritable, SYM_Unnamed), (void *)FUNC));

    //const Type *rawstring = Pointer(TYPE_I8);

    DEFINE_PURE_C_FUNCTION(FN_ImportC, f_import_c, TYPE_Scope, TYPE_String, TYPE_String, TYPE_List);
    DEFINE_PURE_C_FUNCTION(FN_ScopeAt, f_scope_at, Tuple({TYPE_Any,TYPE_Bool}), TYPE_Scope, TYPE_Symbol);
    DEFINE_PURE_C_FUNCTION(FN_ScopeLocalAt, f_scope_local_at, Tuple({TYPE_Any,TYPE_Bool}), TYPE_Scope, TYPE_Symbol);
    DEFINE_PURE_C_FUNCTION(FN_ScopeDocString, f_scope_docstring, TYPE_String, TYPE_Scope, TYPE_Symbol);
    DEFINE_PURE_C_FUNCTION(FN_RuntimeTypeAt, f_type_at, Tuple({TYPE_Any,TYPE_Bool}), TYPE_Type, TYPE_Symbol);
    DEFINE_PURE_C_FUNCTION(FN_SymbolNew, f_symbol_new, TYPE_Symbol, TYPE_String);
    DEFINE_PURE_C_FUNCTION(FN_Repr, f_repr, TYPE_String, TYPE_Any);
    DEFINE_PURE_C_FUNCTION(FN_AnyString, f_any_string, TYPE_String, TYPE_Any);
    DEFINE_PURE_C_FUNCTION(FN_StringJoin, f_string_join, TYPE_String, TYPE_String, TYPE_String);
    DEFINE_PURE_C_FUNCTION(FN_ElementType, f_elementtype, TYPE_Type, TYPE_Type, TYPE_I32);
    DEFINE_PURE_C_FUNCTION(FN_ElementIndex, f_elementindex, TYPE_I32, TYPE_Type, TYPE_Symbol);
    DEFINE_PURE_C_FUNCTION(FN_ElementName, f_elementname, TYPE_Symbol, TYPE_Type, TYPE_I32);
    DEFINE_PURE_C_FUNCTION(FN_SizeOf, f_sizeof, TYPE_USize, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_Alignof, f_alignof, TYPE_USize, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_PointerType, f_pointertype, TYPE_Type, TYPE_Type, TYPE_U64, TYPE_Symbol);
    DEFINE_PURE_C_FUNCTION(FN_PointerFlags, f_pointer_type_flags, TYPE_U64, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_PointerSetFlags, f_pointer_type_set_flags, TYPE_Type, TYPE_Type, TYPE_U64);
    DEFINE_PURE_C_FUNCTION(FN_PointerStorageClass, f_pointer_type_storage_class, TYPE_Symbol, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_PointerSetStorageClass, f_pointer_type_set_storage_class, TYPE_Type, TYPE_Type, TYPE_Symbol);
    DEFINE_PURE_C_FUNCTION(FN_PointerSetElementType, f_pointer_type_set_element_type, TYPE_Type, TYPE_Type, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_ExternLocation, f_extern_type_location, TYPE_I32, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_ExternBinding, f_extern_type_binding, TYPE_I32, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_ListCons, f_list_cons, TYPE_List, TYPE_Any, TYPE_List);
    DEFINE_PURE_C_FUNCTION(FN_TypeKind, f_type_kind, TYPE_I32, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_TypeDebugABI, f_type_debug_abi, TYPE_Void, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_BitCountOf, f_bitcountof, TYPE_I32, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_IsSigned, f_issigned, TYPE_Bool, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_TypeStorage, f_type_storage, TYPE_Type, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_IsOpaque, is_opaque, TYPE_Bool, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_IntegerType, f_integer_type, TYPE_Type, TYPE_I32, TYPE_Bool);
    DEFINE_PURE_C_FUNCTION(FN_CompilerVersion, f_compiler_version, Tuple({TYPE_I32, TYPE_I32, TYPE_I32}));
    DEFINE_PURE_C_FUNCTION(FN_TypeName, f_type_name, TYPE_String, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_TypenameType, f_typename_type, TYPE_Type, TYPE_String);
    DEFINE_PURE_C_FUNCTION(FN_SyntaxNew, f_syntax_new, TYPE_Syntax, TYPE_Anchor, TYPE_Any, TYPE_Bool);
    DEFINE_PURE_C_FUNCTION(FN_SyntaxWrap, wrap_syntax, TYPE_Any, TYPE_Anchor, TYPE_Any, TYPE_Bool);
    DEFINE_PURE_C_FUNCTION(FN_SyntaxStrip, strip_syntax, TYPE_Any, TYPE_Any);
    DEFINE_PURE_C_FUNCTION(FN_ParameterNew, f_parameter_new, TYPE_Parameter, TYPE_Anchor, TYPE_Symbol, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_ParameterIndex, f_parameter_index, TYPE_I32, TYPE_Parameter);
    DEFINE_PURE_C_FUNCTION(FN_ParameterName, f_parameter_name, TYPE_Symbol, TYPE_Parameter);
    DEFINE_PURE_C_FUNCTION(FN_StringNew, f_string_new, TYPE_String, NativeROPointer(TYPE_I8), TYPE_USize);
    DEFINE_PURE_C_FUNCTION(FN_DumpLabel, f_dump_label, TYPE_Void, TYPE_Label);
    DEFINE_PURE_C_FUNCTION(FN_DumpList, f_dump_list, TYPE_List, TYPE_List);
    DEFINE_PURE_C_FUNCTION(FN_DumpFrame, f_dump_frame, TYPE_Void, TYPE_Frame);
    DEFINE_PURE_C_FUNCTION(FN_Eval, f_eval, TYPE_Label, TYPE_Syntax, TYPE_Scope);
    DEFINE_PURE_C_FUNCTION(FN_Typify, f_typify, TYPE_Label, TYPE_Closure, TYPE_I32, NativeROPointer(TYPE_Type));
    DEFINE_PURE_C_FUNCTION(FN_ArrayType, f_array_type, TYPE_Type, TYPE_Type, TYPE_USize);
    DEFINE_PURE_C_FUNCTION(FN_ImageType, Image, TYPE_Type,
        TYPE_Type, TYPE_Symbol, TYPE_I32, TYPE_I32, TYPE_I32, TYPE_I32, TYPE_Symbol, TYPE_Symbol);
    DEFINE_PURE_C_FUNCTION(FN_SampledImageType, SampledImage, TYPE_Type, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_VectorType, f_vector_type, TYPE_Type, TYPE_Type, TYPE_USize);
    DEFINE_PURE_C_FUNCTION(FN_TypeCountOf, f_type_countof, TYPE_I32, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_SymbolToString, f_symbol_to_string, TYPE_String, TYPE_Symbol);
    DEFINE_PURE_C_FUNCTION(Symbol("Any=="), f_any_eq, TYPE_Bool, TYPE_Any, TYPE_Any);
    DEFINE_PURE_C_FUNCTION(FN_ListJoin, f_list_join, TYPE_List, TYPE_List, TYPE_List);
    DEFINE_PURE_C_FUNCTION(FN_ScopeNext, f_scope_next, Tuple({TYPE_Symbol, TYPE_Any}), TYPE_Scope, TYPE_Symbol);
    DEFINE_PURE_C_FUNCTION(FN_TypeNext, f_type_next, Tuple({TYPE_Symbol, TYPE_Any}), TYPE_Type, TYPE_Symbol);
    DEFINE_PURE_C_FUNCTION(FN_StringMatch, f_string_match, TYPE_Bool, TYPE_String, TYPE_String);
    DEFINE_PURE_C_FUNCTION(SFXFN_SetTypenameSuper, f_set_typename_super, TYPE_Void, TYPE_Type, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_SuperOf, superof, TYPE_Type, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_FunctionTypeIsVariadic, f_function_type_is_variadic, TYPE_Bool, TYPE_Type);
    DEFINE_PURE_C_FUNCTION(FN_LabelAnchor, f_label_anchor, TYPE_Anchor, TYPE_Label);
    DEFINE_PURE_C_FUNCTION(FN_LabelParameterCount, f_label_parameter_count, TYPE_USize, TYPE_Label);
    DEFINE_PURE_C_FUNCTION(FN_LabelParameter, f_label_parameter, TYPE_Parameter, TYPE_Label, TYPE_USize);
    DEFINE_PURE_C_FUNCTION(FN_LabelName, f_label_name, TYPE_Symbol, TYPE_Label);
    DEFINE_PURE_C_FUNCTION(FN_ClosureLabel, f_closure_label, TYPE_Label, TYPE_Closure);
    DEFINE_PURE_C_FUNCTION(FN_ClosureFrame, f_closure_frame, TYPE_Frame, TYPE_Closure);
    DEFINE_PURE_C_FUNCTION(FN_LabelCountOfReachable, f_label_countof_reachable, TYPE_USize, TYPE_Label);
    DEFINE_PURE_C_FUNCTION(FN_EnterSolverCLI, f_enter_solver_cli, TYPE_Void);
    DEFINE_PURE_C_FUNCTION(FN_LabelDocString, f_label_docstring, TYPE_String, TYPE_Label);
    DEFINE_PURE_C_FUNCTION(FN_LabelSetInline, f_label_setinline, TYPE_Void, TYPE_Label);

    DEFINE_PURE_C_FUNCTION(FN_DefaultStyler, f_default_styler, TYPE_String, TYPE_Symbol, TYPE_String);

    DEFINE_C_FUNCTION(FN_Compile, f_compile, TYPE_Any, TYPE_Label, TYPE_U64);
    DEFINE_PURE_C_FUNCTION(FN_CompileSPIRV, f_compile_spirv, TYPE_String, TYPE_Symbol, TYPE_Label, TYPE_U64);
    DEFINE_PURE_C_FUNCTION(FN_CompileGLSL, f_compile_glsl, TYPE_String, TYPE_Symbol, TYPE_Label, TYPE_U64);
    DEFINE_PURE_C_FUNCTION(FN_CompileObject, f_compile_object, TYPE_Void, TYPE_String, TYPE_Scope, TYPE_U64);
    DEFINE_C_FUNCTION(FN_Prompt, f_prompt, Tuple({TYPE_String, TYPE_Bool}), TYPE_String, TYPE_String);
    DEFINE_C_FUNCTION(FN_SetAutocompleteScope, f_set_autocomplete_scope, TYPE_Void, TYPE_Scope);
    DEFINE_C_FUNCTION(FN_LoadLibrary, f_load_library, TYPE_Void, TYPE_String);

    DEFINE_C_FUNCTION(FN_IsFile, f_is_file, TYPE_Bool, TYPE_String);
    DEFINE_C_FUNCTION(FN_IsDirectory, f_is_directory, TYPE_Bool, TYPE_String);
    DEFINE_C_FUNCTION(FN_ListLoad, f_list_load, TYPE_Syntax, TYPE_String);
    DEFINE_C_FUNCTION(FN_ListParse, f_list_parse, TYPE_Syntax, TYPE_String);
    DEFINE_C_FUNCTION(FN_ScopeNew, f_scope_new, TYPE_Scope);
    DEFINE_C_FUNCTION(FN_ScopeCopy, f_scope_clone, TYPE_Scope, TYPE_Scope);
    DEFINE_C_FUNCTION(FN_ScopeNewSubscope, f_scope_new_subscope, TYPE_Scope, TYPE_Scope);
    DEFINE_C_FUNCTION(FN_ScopeCopySubscope, f_scope_clone_subscope, TYPE_Scope, TYPE_Scope, TYPE_Scope);
    DEFINE_C_FUNCTION(FN_ScopeParent, f_scope_parent, TYPE_Scope, TYPE_Scope);
    DEFINE_C_FUNCTION(KW_Globals, f_globals, TYPE_Scope);
    DEFINE_C_FUNCTION(SFXFN_SetGlobals, f_set_globals, TYPE_Void, TYPE_Scope);
    DEFINE_C_FUNCTION(SFXFN_SetScopeSymbol, f_set_scope_symbol, TYPE_Void, TYPE_Scope, TYPE_Symbol, TYPE_Any);
    DEFINE_C_FUNCTION(SFXFN_DelScopeSymbol, f_del_scope_symbol, TYPE_Void, TYPE_Scope, TYPE_Symbol);
    DEFINE_C_FUNCTION(FN_SetScopeDocString, f_scope_set_docstring, TYPE_Void, TYPE_Scope, TYPE_Symbol, TYPE_String);
    DEFINE_C_FUNCTION(FN_RealPath, f_realpath, TYPE_String, TYPE_String);
    DEFINE_C_FUNCTION(FN_DirName, f_dirname, TYPE_String, TYPE_String);
    DEFINE_C_FUNCTION(FN_BaseName, f_basename, TYPE_String, TYPE_String);
    DEFINE_C_FUNCTION(FN_FormatMessage, f_format_message, TYPE_String, TYPE_Anchor, TYPE_String);
    DEFINE_C_FUNCTION(FN_ActiveAnchor, get_active_anchor, TYPE_Anchor);
    DEFINE_C_FUNCTION(FN_Write, f_write, TYPE_Void, TYPE_String);
    DEFINE_C_FUNCTION(SFXFN_SetAnchor, f_set_anchor, TYPE_Void, TYPE_Anchor);
    DEFINE_C_FUNCTION(SFXFN_Error, f_error, TYPE_Void, TYPE_String);
    DEFINE_C_FUNCTION(SFXFN_AnchorError, f_anchor_error, TYPE_Void, TYPE_String);
    DEFINE_C_FUNCTION(SFXFN_Raise, f_raise, TYPE_Void, TYPE_Any);
    DEFINE_C_FUNCTION(SFXFN_Abort, f_abort, TYPE_Void);
    DEFINE_C_FUNCTION(FN_Exit, f_exit, TYPE_Void, TYPE_I32);
    DEFINE_C_FUNCTION(FN_CheckStack, f_verify_stack, TYPE_USize);
    DEFINE_PURE_C_FUNCTION(FN_Hash, f_hash, TYPE_U64, TYPE_U64, TYPE_USize);
    DEFINE_PURE_C_FUNCTION(FN_Hash2x64, f_hash2x64, TYPE_U64, TYPE_U64, TYPE_U64);
    DEFINE_PURE_C_FUNCTION(FN_HashBytes, f_hashbytes, TYPE_U64, NativeROPointer(TYPE_I8), TYPE_USize);

    //DEFINE_C_FUNCTION(FN_Malloc, malloc, NativePointer(TYPE_I8), TYPE_USize);

    const Type *exception_pad_type = Array(TYPE_U8, sizeof(ExceptionPad));
    const Type *p_exception_pad_type = NativePointer(exception_pad_type);

    DEFINE_C_FUNCTION(Symbol("set-exception-pad"), f_set_exception_pad,
        p_exception_pad_type, p_exception_pad_type);
    #ifdef SCOPES_WIN32
    DEFINE_C_FUNCTION(Symbol("catch-exception"), _setjmpex, TYPE_I32,
        p_exception_pad_type, NativeROPointer(TYPE_I8));
    #else
    DEFINE_C_FUNCTION(Symbol("catch-exception"), setjmp, TYPE_I32,
        p_exception_pad_type);
    #endif
    DEFINE_C_FUNCTION(Symbol("exception-value"), f_exception_value,
        TYPE_Any, p_exception_pad_type);
    DEFINE_C_FUNCTION(Symbol("set-signal-abort!"), f_set_signal_abort,
        TYPE_Void, TYPE_Bool);



#undef DEFINE_C_FUNCTION

    auto stub_file = SourceFile::from_string(Symbol("<internal>"), String::from_cstr(""));
    auto stub_anchor = Anchor::from(stub_file, 1, 1);

    {
        // launch arguments
        // this is a function returning vararg constants
        Label *fn = Label::function_from(stub_anchor, FN_Args);
        fn->body.anchor = stub_anchor;
        fn->body.enter = fn->params[0];
        globals->bind(FN_Args, fn);
        if (argv && argc) {
            auto &&args = fn->body.args;
            args.push_back(none);
            for (int i = 0; i < argc; ++i) {
                char *s = argv[i];
                if (!s)
                    break;
                args.push_back(String::from_cstr(s));
            }
        }
    }

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

#define T(NAME, BNAME) \
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
