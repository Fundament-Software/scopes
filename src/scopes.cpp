/*
Scopes Compiler
Copyright (c) 2016, 2017, 2018 Leonard Ritter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
BEWARE: If you build this with anything else but a recent enough clang,
        you will have a bad time.

        an exception is windows where with mingw64, only gcc will work.
*/

#include <sys/types.h>
#ifdef SCOPES_WIN32
#include "mman.h"
#include "stdlib_ex.h"
#else
#include <sys/mman.h>
#include <unistd.h>
#endif
#include "linenoise-ng/include/linenoise.h"
#include <ctype.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#define STB_SPRINTF_DECORATE(name) stb_##name
#define STB_SPRINTF_NOUNALIGNED
#include "stb_sprintf.h"
#include "cityhash/city.h"

#include <ffi.h>

#include "scopes.h"

//#define SCOPES_DEBUG_IL

#undef NDEBUG
#ifdef SCOPES_WIN32
#include <windows.h>
#include "stdlib_ex.h"
#include "dlfcn.h"
#else
// for backtrace
#include <execinfo.h>
#include <dlfcn.h>
#endif
#include <assert.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdlib.h>
#include <libgen.h>

#include <cstdlib>
//#include <string>
#include <sstream>
#include <iostream>
#include <unordered_set>
#include <deque>
#include <csignal>
#include <utility>
#include <algorithm>
#include <functional>

#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Transforms/PassManagerBuilder.h>
#include <llvm-c/Disassembler.h>
#include <llvm-c/Support.h>

#include "llvm/IR/Module.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/Object/SymbolSize.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_os_ostream.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/MultiplexConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/RecordLayout.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/LiteralSupport.h"

#include "glslang/SpvBuilder.h"
#include "glslang/disassemble.h"
#include "glslang/GLSL.std.450.h"
#include "SPIRV-Cross/spirv_glsl.hpp"
#include "spirv-tools/libspirv.hpp"
#include "spirv-tools/optimizer.hpp"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#pragma GCC diagnostic ignored "-Wvla-extension"
#pragma GCC diagnostic ignored "-Wzero-length-array"
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
// #pragma GCC diagnostic ignored "-Wembedded-directive"
// #pragma GCC diagnostic ignored "-Wgnu-statement-expression"
#pragma GCC diagnostic ignored "-Wc99-extensions"
// #pragma GCC diagnostic ignored "-Wmissing-braces"
// this one is only enabled for code cleanup
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#pragma GCC diagnostic ignored "-Wdate-time"
#pragma GCC diagnostic ignored "-Wabsolute-value"

#ifdef SCOPES_WIN32
#include <setjmpex.h>
#else
#include <setjmp.h>
#endif
#include "minilibs/regexp.cpp"

#include "utils.hpp"
#include "gc.hpp"
#include "symbol_enum.hpp"
#include "styled_stream.hpp"
#include "none.hpp"
#include "string.hpp"
#include "symbol.hpp"
#include "timer.hpp"
#include "source_file.hpp"
#include "anchor.hpp"
#include "type.hpp"
#include "builtin.hpp"
#include "any.hpp"
#include "error.hpp"
#include "typefactory.hpp"
#include "integer.hpp"
#include "real.hpp"
#include "pointer.hpp"
#include "sized_storage.hpp"
#include "array.hpp"
#include "vector.hpp"
#include "typename.hpp"
#include "argument.hpp"
#include "tuple.hpp"
#include "union.hpp"
#include "extern.hpp"
#include "return.hpp"
#include "function.hpp"

namespace scopes {

using llvm::isa;
using llvm::cast;
using llvm::dyn_cast;

typedef std::vector<Label *> Labels;
typedef std::deque<Label *> LabelQueue;
typedef std::vector<Parameter *> Parameters;

//------------------------------------------------------------------------------
// IMAGE TYPE
//------------------------------------------------------------------------------

struct ImageType : Type {
    static bool classof(const Type *T) {
        return T->kind() == TK_Image;
    }

    ImageType(
        const Type *_type,
        Symbol _dim,
        int _depth,
        int _arrayed,
        int _multisampled,
        int _sampled,
        Symbol _format,
        Symbol _access) :
        Type(TK_Image),
        type(_type), dim(_dim), depth(_depth), arrayed(_arrayed),
        multisampled(_multisampled), sampled(_sampled),
        format(_format), access(_access) {
        auto ss = StyledString::plain();
        ss.out << "<Image " <<  _type->name()->data
            << " " << _dim;
        if (_depth == 1)
            ss.out << " depth";
        else if (_depth == 2)
            ss.out << " ?depth?";
        if (_arrayed)
            ss.out << " array";
        if (_multisampled)
            ss.out << " ms";
        if (_sampled == 0)
            ss.out << " ?sampled?";
        else if (_sampled == 1)
            ss.out << " sampled";
        ss.out << " " << _format;
        if (access != SYM_Unnamed)
            ss.out << " " << _access;
        ss.out << ">";
        _name = ss.str();
    }

    const Type *type; // sampled type
    Symbol dim; // resolved to spv::Dim
    int depth; // 0 = not a depth image, 1 = depth image, 2 = undefined
    int arrayed; // 1 = array image
    int multisampled; // 1 = multisampled content
    int sampled; // 0 = runtime dependent, 1 = sampled, 2 = storage image
    Symbol format; // resolved to spv::ImageFormat
    Symbol access; // resolved to spv::AccessQualifier
};

static const Type *Image(
    const Type *_type,
    Symbol _dim,
    int _depth,
    int _arrayed,
    int _multisampled,
    int _sampled,
    Symbol _format,
    Symbol _access) {
    static TypeFactory<ImageType> images;
    return images.insert(_type, _dim, _depth, _arrayed,
        _multisampled, _sampled, _format, _access);
}

//------------------------------------------------------------------------------
// IMAGE TYPE
//------------------------------------------------------------------------------

struct SampledImageType : Type {
    static bool classof(const Type *T) {
        return T->kind() == TK_SampledImage;
    }

    SampledImageType(const Type *_type) :
        Type(TK_SampledImage), type(cast<ImageType>(_type)) {
        auto ss = StyledString::plain();
        ss.out << "<SampledImage " <<  _type->name()->data << ">";
        _name = ss.str();
    }

    const ImageType *type; // image type
};

static const Type *SampledImage(const Type *_type) {
    static TypeFactory<SampledImageType> sampled_images;
    return sampled_images.insert(_type);
}

//------------------------------------------------------------------------------
// TYPE INQUIRIES
//------------------------------------------------------------------------------

template<TypeKind tk>
static void verify_kind(const Type *T) {
    if (T->kind() != tk) {
        StyledString ss;
        ss.out << "value of ";
        switch(tk) {
        case TK_Integer: ss.out << "integer"; break;
        case TK_Real: ss.out << "real"; break;
        case TK_Pointer: ss.out << "pointer"; break;
        case TK_Array: ss.out << "array"; break;
        case TK_Vector: ss.out << "vector"; break;
        case TK_Tuple: ss.out << "tuple"; break;
        case TK_Union: ss.out << "union"; break;
        case TK_Typename: ss.out << "typename"; break;
        case TK_ReturnLabel: ss.out << "return label"; break;
        case TK_Function: ss.out << "function"; break;
        case TK_Extern: ss.out << "extern"; break;
        case TK_Image: ss.out << "image"; break;
        case TK_SampledImage: ss.out << "sampled image"; break;
        }
        ss.out << " kind expected, got " << T;
        location_error(ss.str());
    }
}

static void verify_function_pointer(const Type *type) {
    if (!is_function_pointer(type)) {
        StyledString ss;
        ss.out << "function pointer expected, got " << type;
        location_error(ss.str());
    }
}

//------------------------------------------------------------------------------
// ANY METHODS
//------------------------------------------------------------------------------

StyledStream& Any::stream(StyledStream& ost, bool annotate_type) const {
    AnyStreamer as(ost, type, annotate_type);
    if (type == TYPE_Nothing) { as.naked(none); }
    else if (type == TYPE_Type) { as.naked(typeref); }
    else if (type == TYPE_Bool) { as.naked(i1); }
    else if (type == TYPE_I8) { as.typed(i8); }
    else if (type == TYPE_I16) { as.typed(i16); }
    else if (type == TYPE_I32) { as.naked(i32); }
    else if (type == TYPE_I64) { as.typed(i64); }
    else if (type == TYPE_U8) { as.typed(u8); }
    else if (type == TYPE_U16) { as.typed(u16); }
    else if (type == TYPE_U32) { as.typed(u32); }
    else if (type == TYPE_U64) { as.typed(u64); }
    else if (type == TYPE_USize) { as.typed(u64); }
    else if (type == TYPE_F32) { as.naked(f32); }
    else if (type == TYPE_F64) { as.typed(f64); }
    else if (type == TYPE_String) { as.naked(string); }
    else if (type == TYPE_Symbol) { as.naked(symbol); }
    else if (type == TYPE_Syntax) { as.naked(syntax); }
    else if (type == TYPE_Anchor) { as.typed(anchor); }
    else if (type == TYPE_List) { as.naked(list); }
    else if (type == TYPE_Builtin) { as.typed(builtin); }
    else if (type == TYPE_Label) { as.typed(label); }
    else if (type == TYPE_Parameter) { as.typed(parameter); }
    else if (type == TYPE_Scope) { as.typed(scope); }
    else if (type == TYPE_Frame) { as.typed(frame); }
    else if (type == TYPE_Closure) { as.typed(closure); }
    else if (type == TYPE_Any) {
        ost << Style_Operator << "[" << Style_None;
        ((Any *)pointer)->stream(ost);
        ost << Style_Operator << "]" << Style_None;
        as.stream_type_suffix();
    } else if (type->kind() == TK_Extern) {
        ost << symbol;
        as.stream_type_suffix();
    } else if (type->kind() == TK_Vector) {
        auto vt = cast<VectorType>(type);
        ost << Style_Operator << "<" << Style_None;
        for (size_t i = 0; i < vt->count; ++i) {
            if (i != 0) {
                ost << " ";
            }
            vt->unpack(pointer, i).stream(ost, false);
        }
        ost << Style_Operator << ">" << Style_None;
        auto ET = vt->element_type;
        if (!((ET == TYPE_Bool)
            || (ET == TYPE_I32)
            || (ET == TYPE_F32)
            ))
            as.stream_type_suffix();
    } else { as.typed(pointer); }
    return ost;
}

size_t Any::hash() const {
    if (type == TYPE_String) {
        if (!string) return 0; // can happen with nullof
        return CityHash64(string->data, string->count);
    }
    if (is_opaque(type))
        return 0;
    const Type *T = storage_type(type);
    switch(T->kind()) {
    case TK_Integer: {
        switch(cast<IntegerType>(T)->width) {
        case 1: return std::hash<bool>{}(i1);
        case 8: return std::hash<uint8_t>{}(u8);
        case 16: return std::hash<uint16_t>{}(u16);
        case 32: return std::hash<uint32_t>{}(u32);
        case 64: return std::hash<uint64_t>{}(u64);
        default: break;
        }
    } break;
    case TK_Real: {
        switch(cast<RealType>(T)->width) {
        case 32: return std::hash<float>{}(f32);
        case 64: return std::hash<double>{}(f64);
        default: break;
        }
    } break;
    case TK_Extern: {
        return std::hash<uint64_t>{}(u64);
    } break;
    case TK_Pointer: return std::hash<void *>{}(pointer);
    case TK_Array: {
        auto ai = cast<ArrayType>(T);
        size_t h = 0;
        for (size_t i = 0; i < ai->count; ++i) {
            h = HashLen16(h, ai->unpack(pointer, i).hash());
        }
        return h;
    } break;
    case TK_Vector: {
        auto vi = cast<VectorType>(T);
        size_t h = 0;
        for (size_t i = 0; i < vi->count; ++i) {
            h = HashLen16(h, vi->unpack(pointer, i).hash());
        }
        return h;
    } break;
    case TK_Tuple: {
        auto ti = cast<TupleType>(T);
        size_t h = 0;
        for (size_t i = 0; i < ti->types.size(); ++i) {
            h = HashLen16(h, ti->unpack(pointer, i).hash());
        }
        return h;
    } break;
    case TK_Union:
        return CityHash64((const char *)pointer, size_of(T));
    default: break;
    }

    StyledStream ss(std::cout);
    ss << "unhashable value: " << T << std::endl;
    assert(false && "unhashable value");
    return 0;
}

bool Any::operator ==(const Any &other) const {
    if (type != other.type) return false;
    if (type == TYPE_String) {
        if (string == other.string) return true;
        if (!string || !other.string) return false;
        if (string->count != other.string->count)
            return false;
        return !memcmp(string->data, other.string->data, string->count);
    }
    if (is_opaque(type))
        return true;
    const Type *T = storage_type(type);
    switch(T->kind()) {
    case TK_Integer: {
        switch(cast<IntegerType>(T)->width) {
        case 1: return (i1 == other.i1);
        case 8: return (u8 == other.u8);
        case 16: return (u16 == other.u16);
        case 32: return (u32 == other.u32);
        case 64: return (u64 == other.u64);
        default: break;
        }
    } break;
    case TK_Real: {
        switch(cast<RealType>(T)->width) {
        case 32: return (f32 == other.f32);
        case 64: return (f64 == other.f64);
        default: break;
        }
    } break;
    case TK_Extern: return symbol == other.symbol;
    case TK_Pointer: return pointer == other.pointer;
    case TK_Array: {
        auto ai = cast<ArrayType>(T);
        for (size_t i = 0; i < ai->count; ++i) {
            if (ai->unpack(pointer, i) != ai->unpack(other.pointer, i))
                return false;
        }
        return true;
    } break;
    case TK_Vector: {
        auto vi = cast<VectorType>(T);
        for (size_t i = 0; i < vi->count; ++i) {
            if (vi->unpack(pointer, i) != vi->unpack(other.pointer, i))
                return false;
        }
        return true;
    } break;
    case TK_Tuple: {
        auto ti = cast<TupleType>(T);
        for (size_t i = 0; i < ti->types.size(); ++i) {
            if (ti->unpack(pointer, i) != ti->unpack(other.pointer, i))
                return false;
        }
        return true;
    } break;
    case TK_Union:
        return !memcmp(pointer, other.pointer, size_of(T));
    default: break;
    }

    StyledStream ss(std::cout);
    ss << "incomparable value: " << T << std::endl;
    assert(false && "incomparable value");
    return false;
}

//------------------------------------------------------------------------------
// ERROR HANDLING
//------------------------------------------------------------------------------

static const Anchor *_active_anchor = nullptr;

static void set_active_anchor(const Anchor *anchor) {
    assert(anchor);
    _active_anchor = anchor;
}

static const Anchor *get_active_anchor() {
    return _active_anchor;
}

struct Exception {
    const Anchor *anchor;
    const String *msg;

    Exception() :
        anchor(nullptr),
        msg(nullptr) {}

    Exception(const Anchor *_anchor, const String *_msg) :
        anchor(_anchor),
        msg(_msg) {}
};

struct ExceptionPad {
    jmp_buf retaddr;
    Any value;

    ExceptionPad() : value(none) {
    }

    void invoke(const Any &value) {
        this->value = value;
        longjmp(retaddr, 1);
    }
};

#ifdef SCOPES_WIN32
#define SCOPES_TRY() \
    ExceptionPad exc_pad; \
    ExceptionPad *_last_exc_pad = _exc_pad; \
    _exc_pad = &exc_pad; \
    if (!_setjmpex(exc_pad.retaddr, nullptr)) {
#else
#define SCOPES_TRY() \
    ExceptionPad exc_pad; \
    ExceptionPad *_last_exc_pad = _exc_pad; \
    _exc_pad = &exc_pad; \
    if (!setjmp(exc_pad.retaddr)) {
#endif

#define SCOPES_CATCH(EXCNAME) \
        _exc_pad = _last_exc_pad; \
    } else { \
        _exc_pad = _last_exc_pad; \
        auto &&EXCNAME = exc_pad.value;

#define SCOPES_TRY_END() \
    }

static ExceptionPad *_exc_pad = nullptr;

static void default_exception_handler(const Any &value);

static void error(const Any &value) {
#if SCOPES_EARLY_ABORT
    default_exception_handler(value);
#else
    if (!_exc_pad) {
        default_exception_handler(value);
    } else {
        _exc_pad->invoke(value);
    }
#endif
}

void location_error(const String *msg) {
    const Exception *exc = new Exception(_active_anchor, msg);
    error(exc);
}

//------------------------------------------------------------------------------
// SCOPE
//------------------------------------------------------------------------------

struct AnyDoc {
    Any value;
    const String *doc;
};

struct Scope {
public:
    typedef std::unordered_map<Symbol, AnyDoc, Symbol::Hash> Map;
protected:
    Scope(Scope *_parent = nullptr, Map *_map = nullptr) :
        parent(_parent),
        map(_map?_map:(new Map())),
        borrowed(_map?true:false),
        doc(nullptr),
        next_doc(nullptr) {
        if (_parent)
            doc = _parent->doc;
    }

public:
    Scope *parent;
    Map *map;
    bool borrowed;
    const String *doc;
    const String *next_doc;

    void set_doc(const String *str) {
        if (!doc) {
            doc = str;
            next_doc = nullptr;
        } else {
            next_doc = str;
        }
    }

    size_t count() const {
#if 0
        return map->size();
#else
        size_t count = 0;
        auto &&_map = *map;
        for (auto &&k : _map) {
            if (!is_typed(k.second.value))
                continue;
            count++;
        }
        return count;
#endif
    }

    size_t totalcount() const {
        const Scope *self = this;
        size_t count = 0;
        while (self) {
            count += self->count();
            self = self->parent;
        }
        return count;
    }

    size_t levelcount() const {
        const Scope *self = this;
        size_t count = 0;
        while (self) {
            count += 1;
            self = self->parent;
        }
        return count;
    }

    void ensure_not_borrowed() {
        if (!borrowed) return;
        parent = Scope::from(parent, this);
        map = new Map();
        borrowed = false;
    }

    void bind_with_doc(Symbol name, const AnyDoc &entry) {
        ensure_not_borrowed();
        auto ret = map->insert({name, entry});
        if (!ret.second) {
            ret.first->second = entry;
        }
    }

    void bind(Symbol name, const Any &value) {
        AnyDoc entry = { value, next_doc };
        bind_with_doc(name, entry);
        next_doc = nullptr;
    }

    void bind(KnownSymbol name, const Any &value) {
        AnyDoc entry = { value, nullptr };
        bind_with_doc(Symbol(name), entry);
    }

    void del(Symbol name) {
        ensure_not_borrowed();
        auto it = map->find(name);
        if (it != map->end()) {
            // if in local map, we can delete it directly
            map->erase(it);
        } else {
            // otherwise check if it's contained at all
            Any dest = none;
            if (lookup(name, dest)) {
                AnyDoc entry = { untyped(), nullptr };
                // if yes, bind to unknown unknown to mark it as deleted
                bind_with_doc(name, entry);
            }
        }
    }

    std::vector<Symbol> find_closest_match(Symbol name) const {
        const String *s = name.name();
        std::unordered_set<Symbol, Symbol::Hash> done;
        std::vector<Symbol> best_syms;
        size_t best_dist = (size_t)-1;
        const Scope *self = this;
        do {
            auto &&map = *self->map;
            for (auto &&k : map) {
                Symbol sym = k.first;
                if (done.count(sym))
                    continue;
                if (is_typed(k.second.value)) {
                    size_t dist = distance(s, sym.name());
                    if (dist == best_dist) {
                        best_syms.push_back(sym);
                    } else if (dist < best_dist) {
                        best_dist = dist;
                        best_syms = { sym };
                    }
                }
                done.insert(sym);
            }
            self = self->parent;
        } while (self);
        std::sort(best_syms.begin(), best_syms.end());
        return best_syms;
    }

    std::vector<Symbol> find_elongations(Symbol name) const {
        const String *s = name.name();

        std::unordered_set<Symbol, Symbol::Hash> done;
        std::vector<Symbol> found;
        const Scope *self = this;
        do {
            auto &&map = *self->map;
            for (auto &&k : map) {
                Symbol sym = k.first;
                if (done.count(sym))
                    continue;
                if (is_typed(k.second.value)) {
                    if (sym.name()->count >= s->count &&
                            *sym.name()->substr(0, s->count) == *s)
                        found.push_back(sym);
                }
                done.insert(sym);
            }
            self = self->parent;
        } while (self);
        std::sort(found.begin(), found.end(), [](Symbol a, Symbol b){
                  return a.name()->count < b.name()->count; });
        return found;
    }

    bool lookup(Symbol name, AnyDoc &dest, size_t depth = -1) const {
        const Scope *self = this;
        do {
            auto it = self->map->find(name);
            if (it != self->map->end()) {
                if (is_typed(it->second.value)) {
                    dest = it->second;
                    return true;
                } else {
                    return false;
                }
            }
            if (!depth)
                break;
            depth = depth - 1;
            self = self->parent;
        } while (self);
        return false;
    }

    bool lookup(Symbol name, Any &dest, size_t depth = -1) const {
        AnyDoc entry = { none, nullptr };
        if (lookup(name, entry, depth)) {
            dest = entry.value;
            return true;
        }
        return false;
    }

    bool lookup_local(Symbol name, AnyDoc &dest) const {
        return lookup(name, dest, 0);
    }

    bool lookup_local(Symbol name, Any &dest) const {
        return lookup(name, dest, 0);
    }

    StyledStream &stream(StyledStream &ss) {
        size_t totalcount = this->totalcount();
        size_t count = this->count();
        size_t levelcount = this->levelcount();
        ss << Style_Keyword << "Scope" << Style_Comment << "<" << Style_None
            << format("L:%i T:%i in %i levels", count, totalcount, levelcount)->data
            << Style_Comment << ">" << Style_None;
        return ss;
    }

    static Scope *from(Scope *_parent = nullptr, Scope *_borrow = nullptr) {
        return new Scope(_parent, _borrow?(_borrow->map):nullptr);
    }
};

static Scope *globals = Scope::from();

static StyledStream& operator<<(StyledStream& ost, Scope *scope) {
    scope->stream(ost);
    return ost;
}

//------------------------------------------------------------------------------
// LIST
//------------------------------------------------------------------------------

static const List *EOL = nullptr;


#define LIST_POOLSIZE 0x10000
struct List {
protected:
    List(const Any &_at, const List *_next, size_t _count) :
        at(_at),
        next(_next),
        count(_count) {}

public:
    Any at;
    const List *next;
    size_t count;

    Any first() const {
        if (this == EOL) {
            return none;
        } else {
            return at;
        }
    }

    static const List *from(const Any &_at, const List *_next) {
        return new List(_at, _next, (_next != EOL)?(_next->count + 1):1);
    }

    static const List *from(const Any *values, int N) {
        const List *list = EOL;
        for (int i = N - 1; i >= 0; --i) {
            list = from(values[i], list);
        }
        return list;
    }

    template<unsigned N>
    static const List *from(const Any (&values)[N]) {
        return from(values, N);
    }

    static const List *join(const List *a, const List *b);
};

// (a . (b . (c . (d . NIL)))) -> (d . (c . (b . (a . NIL))))
// this is the mutating version; input lists are modified, direction is inverted
const List *reverse_list_inplace(
    const List *l, const List *eol = EOL, const List *cat_to = EOL) {
    const List *next = cat_to;
    size_t count = 0;
    if (cat_to != EOL) {
        count = cat_to->count;
    }
    while (l != eol) {
        count = count + 1;
        const List *iternext = l->next;
        const_cast<List *>(l)->next = next;
        const_cast<List *>(l)->count = count;
        next = l;
        l = iternext;
    }
    return next;
}

const List *List::join(const List *la, const List *lb) {
    const List *l = lb;
    while (la != EOL) {
        l = List::from(la->at, l);
        la = la->next;
    }
    return reverse_list_inplace(l, lb, lb);
}

static StyledStream& operator<<(StyledStream& ost, const List *list);

//------------------------------------------------------------------------------
// SYNTAX OBJECTS
//------------------------------------------------------------------------------

struct Syntax {
protected:
    Syntax(const Anchor *_anchor, const Any &_datum, bool _quoted) :
        anchor(_anchor),
        datum(_datum),
        quoted(_quoted) {}

public:
    const Anchor *anchor;
    Any datum;
    bool quoted;

    static const Syntax *from(const Anchor *_anchor, const Any &_datum, bool quoted = false) {
        assert(_anchor);
        return new Syntax(_anchor, _datum, quoted);
    }

    static const Syntax *from_quoted(const Anchor *_anchor, const Any &_datum) {
        assert(_anchor);
        return new Syntax(_anchor, _datum, true);
    }
};

static Any unsyntax(const Any &e) {
    e.verify(TYPE_Syntax);
    return e.syntax->datum;
}

static Any maybe_unsyntax(const Any &e) {
    if (e.type == TYPE_Syntax) {
        return e.syntax->datum;
    } else {
        return e;
    }
}

static Any strip_syntax(Any e) {
    e = maybe_unsyntax(e);
    if (e.type == TYPE_List) {
        auto src = e.list;
        auto l = src;
        bool needs_unwrap = false;
        while (l != EOL) {
            if (l->at.type == TYPE_Syntax) {
                needs_unwrap = true;
                break;
            }
            l = l->next;
        }
        if (needs_unwrap) {
            l = src;
            const List *dst = EOL;
            while (l != EOL) {
                dst = List::from(strip_syntax(l->at), dst);
                l = l->next;
            }
            return reverse_list_inplace(dst);
        }
    }
    return e;
}

static Any wrap_syntax(const Anchor *anchor, Any e, bool quoted = false) {
    if (e.type == TYPE_List) {
        auto src = e.list;
        auto l = src;
        bool needs_wrap = false;
        while (l != EOL) {
            if (l->at.type != TYPE_Syntax) {
                needs_wrap = true;
                break;
            }
            l = l->next;
        }
        l = src;
        if (needs_wrap) {
            const List *dst = EOL;
            while (l != EOL) {
                dst = List::from(wrap_syntax(anchor, l->at, quoted), dst);
                l = l->next;
            }
            l = reverse_list_inplace(dst);
        }
        return Syntax::from(anchor, l, quoted);
    } else if (e.type != TYPE_Syntax) {
        return Syntax::from(anchor, e, quoted);
    }
    return e;
}

static StyledStream& operator<<(StyledStream& ost, const Syntax *value) {
    ost << value->anchor << value->datum;
    return ost;
}

//------------------------------------------------------------------------------
// S-EXPR LEXER & PARSER
//------------------------------------------------------------------------------

#define B_TOKENS() \
    T(none, -1) \
    T(eof, 0) \
    T(open, '(') \
    T(close, ')') \
    T(square_open, '[') \
    T(square_close, ']') \
    T(curly_open, '{') \
    T(curly_close, '}') \
    T(string, '"') \
    T(block_string, 'B') \
    T(quote, '\'') \
    T(symbol, 'S') \
    T(escape, '\\') \
    T(statement, ';') \
    T(number, 'N')

enum Token {
#define T(NAME, VALUE) tok_ ## NAME = VALUE,
    B_TOKENS()
#undef T
};

static const char *get_token_name(Token tok) {
    switch(tok) {
#define T(NAME, VALUE) case tok_ ## NAME: return #NAME;
    B_TOKENS()
#undef T
    }
}

static const char TOKEN_TERMINATORS[] = "()[]{}\"';#,";

struct LexerParser {
    // LEXER
    //////////////////////////////

    void verify_good_taste(char c) {
        if (c == '\t') {
            location_error(String::from("please use spaces instead of tabs."));
        }
    }

    Token token;
    int base_offset;
    SourceFile *file;
    const char *input_stream;
    const char *eof;
    const char *cursor;
    const char *next_cursor;
    int lineno;
    int next_lineno;
    const char *line;
    const char *next_line;

    const char *string;
    int string_len;

    Any value;

    LexerParser(SourceFile *_file, size_t offset = 0, size_t length = 0) :
            value(none) {
        file = _file;
        input_stream = file->strptr() + offset;
        token = tok_eof;
        base_offset = (int)offset;
        if (length) {
            eof = input_stream + length;
        } else {
            eof = file->strptr() + file->length;
        }
        cursor = next_cursor = input_stream;
        lineno = next_lineno = 1;
        line = next_line = input_stream;
    }

    int offset() {
        return base_offset + (cursor - input_stream);
    }

    int column() {
        return cursor - line + 1;
    }

    int next_column() {
        return next_cursor - next_line + 1;
    }

    const Anchor *anchor() {
        return Anchor::from(file, lineno, column(), offset());
    }

    char next() {
        char c = next_cursor[0];
        verify_good_taste(c);
        next_cursor = next_cursor + 1;
        return c;
    }

    size_t chars_left() {
        return eof - next_cursor;
    }

    bool is_eof() {
        return next_cursor == eof;
    }

    void newline() {
        next_lineno = next_lineno + 1;
        next_line = next_cursor;
    }

    void select_string() {
        string = cursor;
        string_len = next_cursor - cursor;
    }

    void read_single_symbol() {
        select_string();
    }

    void read_symbol() {
        bool escape = false;
        while (true) {
            if (is_eof()) {
                break;
            }
            char c = next();
            if (escape) {
                if (c == '\n') {
                    newline();
                }
                escape = false;
            } else if (c == '\\') {
                escape = true;
            } else if (isspace(c) || strchr(TOKEN_TERMINATORS, c)) {
                next_cursor = next_cursor - 1;
                break;
            }
        }
        select_string();
    }

    void read_string(char terminator) {
        bool escape = false;
        while (true) {
            if (is_eof()) {
                location_error(String::from("unterminated sequence"));
                break;
            }
            char c = next();
            if (c == '\n') {
                // 0.10
                //newline();
                // 0.11
                location_error(String::from("unexpected line break in string"));
                break;
            }
            if (escape) {
                escape = false;
            } else if (c == '\\') {
                escape = true;
            } else if (c == terminator) {
                break;
            }
        }
        select_string();
    }

    void read_block(int indent) {
        int col = column() + indent;
        while (true) {
            if (is_eof()) {
                break;
            }
            int next_col = next_column();
            char c = next();
            if (c == '\n') {
                newline();
            } else if (!isspace(c) && (next_col <= col)) {
                next_cursor = next_cursor - 1;
                break;
            }
        }
    }

    void read_block_string() {
        next();next();next();
        read_block(3);
        select_string();
    }

    void read_comment() {
        read_block(0);
    }

    template<unsigned N>
    bool is_suffix(const char (&str)[N]) {
        if (string_len != (N - 1)) {
            return false;
        }
        return !strncmp(string, str, N - 1);
    }

    enum {
        RN_Invalid = 0,
        RN_Untyped = 1,
        RN_Typed = 2,
    };

    template<typename T>
    int read_integer(void (*strton)(T *, const char*, char**)) {
        char *cend;
        errno = 0;
        T srcval;
        strton(&srcval, cursor, &cend);
        if ((cend == cursor)
            || (errno == ERANGE)
            || (cend > eof)) {
            return RN_Invalid;
        }
        value = Any(srcval);
        next_cursor = cend;
        if ((cend != eof)
            && (!isspace(*cend))
            && (!strchr(TOKEN_TERMINATORS, *cend))) {
            if (strchr(".e", *cend)) return false;
            // suffix
            auto _lineno = lineno; auto _line = line; auto _cursor = cursor;
            next_token();
            read_symbol();
            lineno = _lineno; line = _line; cursor = _cursor;
            return RN_Typed;
        } else {
            return RN_Untyped;
        }
    }

    template<typename T>
    int read_real(void (*strton)(T *, const char*, char**, int)) {
        char *cend;
        errno = 0;
        T srcval;
        strton(&srcval, cursor, &cend, 0);
        if ((cend == cursor)
            || (errno == ERANGE)
            || (cend > eof)) {
            return RN_Invalid;
        }
        value = Any(srcval);
        next_cursor = cend;
        if ((cend != eof)
            && (!isspace(*cend))
            && (!strchr(TOKEN_TERMINATORS, *cend))) {
            // suffix
            auto _lineno = lineno; auto _line = line; auto _cursor = cursor;
            next_token();
            read_symbol();
            lineno = _lineno; line = _line; cursor = _cursor;
            return RN_Typed;
        } else {
            return RN_Untyped;
        }
    }

    bool has_suffix() const {
        return (string_len >= 1) && (string[0] == ':');
    }

    bool select_integer_suffix() {
        if (!has_suffix())
            return false;
        if (is_suffix(":i8")) { value = Any(value.i8); return true; }
        else if (is_suffix(":i16")) { value = Any(value.i16); return true; }
        else if (is_suffix(":i32")) { value = Any(value.i32); return true; }
        else if (is_suffix(":i64")) { value = Any(value.i64); return true; }
        else if (is_suffix(":u8")) { value = Any(value.u8); return true; }
        else if (is_suffix(":u16")) { value = Any(value.u16); return true; }
        else if (is_suffix(":u32")) { value = Any(value.u32); return true; }
        else if (is_suffix(":u64")) { value = Any(value.u64); return true; }
        //else if (is_suffix(":isize")) { value = Any(value.i64); return true; }
        else if (is_suffix(":usize")) { value = Any(value.u64); value.type = TYPE_USize; return true; }
        else {
            StyledString ss;
            ss.out << "invalid suffix for integer literal: "
                << String::from(string, string_len);
            location_error(ss.str());
            return false;
        }
    }

    bool select_real_suffix() {
        if (!has_suffix())
            return false;
        if (is_suffix(":f32")) { value = Any((float)value.f64); return true; }
        else if (is_suffix(":f64")) { value = Any(value.f64); return true; }
        else {
            StyledString ss;
            ss.out << "invalid suffix for floating point literal: "
                << String::from(string, string_len);
            location_error(ss.str());
            return false;
        }
    }

    bool read_int64() {
        switch(read_integer(scopes_strtoll)) {
        case RN_Invalid: return false;
        case RN_Untyped:
            if ((value.i64 >= -0x80000000ll) && (value.i64 <= 0x7fffffffll)) {
                value = Any(int32_t(value.i64));
            } else if ((value.i64 >= 0x80000000ll) && (value.i64 <= 0xffffffffll)) {
                value = Any(uint32_t(value.i64));
            }
            return true;
        case RN_Typed:
            return select_integer_suffix();
        default: assert(false); return false;
        }
    }
    bool read_uint64() {
        switch(read_integer(scopes_strtoull)) {
        case RN_Invalid: return false;
        case RN_Untyped:
            return true;
        case RN_Typed:
            return select_integer_suffix();
        default: assert(false); return false;
        }
    }
    bool read_real64() {
        switch(read_real(scopes_strtod)) {
        case RN_Invalid: return false;
        case RN_Untyped:
            value = Any(float(value.f64));
            return true;
        case RN_Typed:
            return select_real_suffix();
        default: assert(false); return false;
        }
    }

    void next_token() {
        lineno = next_lineno;
        line = next_line;
        cursor = next_cursor;
        set_active_anchor(anchor());
    }

    Token read_token() {
        char c;
    skip:
        next_token();
        if (is_eof()) { token = tok_eof; goto done; }
        c = next();
        if (c == '\n') { newline(); }
        if (isspace(c)) { goto skip; }
        if (c == '#') { read_comment(); goto skip; }
        else if (c == '(') { token = tok_open; }
        else if (c == ')') { token = tok_close; }
        else if (c == '[') { token = tok_square_open; }
        else if (c == ']') { token = tok_square_close; }
        else if (c == '{') { token = tok_curly_open; }
        else if (c == '}') { token = tok_curly_close; }
        else if (c == '\\') { token = tok_escape; }
        else if (c == '"') {
            if ((chars_left() >= 3)
                && (next_cursor[0] == '"')
                && (next_cursor[1] == '"')
                && (next_cursor[2] == '"')) {
                token = tok_block_string;
                read_block_string();
            } else {
                token = tok_string;
                read_string(c);
            }
        }
        else if (c == ';') { token = tok_statement; }
        else if (c == '\'') { token = tok_quote; }
        else if (c == ',') { token = tok_symbol; read_single_symbol(); }
        else if (read_int64() || read_uint64() || read_real64()) { token = tok_number; }
        else { token = tok_symbol; read_symbol(); }
    done:
        return token;
    }

    Any get_symbol() {
        char dest[string_len + 1];
        memcpy(dest, string, string_len);
        dest[string_len] = 0;
        auto size = unescape_string(dest);
        return Symbol(String::from(dest, size));
    }
    Any get_string() {
        auto len = string_len - 2;
        char dest[len + 1];
        memcpy(dest, string + 1, len);
        dest[len] = 0;
        auto size = unescape_string(dest);
        return String::from(dest, size);
    }
    Any get_block_string() {
        int strip_col = column() + 4;
        auto len = string_len - 4;
        assert(len >= 0);
        char dest[len + 1];
        const char *start = string + 4;
        const char *end = start + len;
        // strip trailing whitespace up to the first LF after content
        const char *last_lf = end;
        while (end != start) {
            char c = *(end - 1);
            if (!isspace(c)) break;
            if (c == '\n')
                last_lf = end;
            end--;
        }
        end = last_lf;
        char *p = dest;
        while (start != end) {
            char c = *start++;
            *p++ = c;
            if (c == '\n') {
                // strip leftside column
                for (int i = 1; i < strip_col; ++i) {
                    if (start == end) break;
                    if ((*start != ' ') && (*start != '\t')) break;
                    start++;
                }
            }
        }
        return String::from(dest, p - dest);
    }
    Any get_number() {
        return value;
    }
    Any get() {
        if (token == tok_number) {
            return get_number();
        } else if (token == tok_symbol) {
            return get_symbol();
        } else if (token == tok_string) {
            return get_string();
        } else if (token == tok_block_string) {
            return get_block_string();
        } else {
            return none;
        }
    }

    // PARSER
    //////////////////////////////

    struct ListBuilder {
        LexerParser &lexer;
        const List *prev;
        const List *eol;

        ListBuilder(LexerParser &_lexer) :
            lexer(_lexer),
            prev(EOL),
            eol(EOL) {}

        void append(const Any &value) {
            assert(value.type == TYPE_Syntax);
            prev = List::from(value, prev);
        }

        bool is_empty() const {
            return (prev == EOL);
        }

        bool is_expression_empty() const {
            return (prev == EOL);
        }

        void reset_start() {
            eol = prev;
        }

        void split(const Anchor *anchor) {
            // reverse what we have, up to last split point and wrap result
            // in cell
            prev = List::from(
                Syntax::from(anchor,reverse_list_inplace(prev, eol)), eol);
            reset_start();
        }

        const List *get_result() {
            return reverse_list_inplace(prev);
        }
    };

    // parses a list to its terminator and returns a handle to the first cell
    const List *parse_list(Token end_token) {
        const Anchor *start_anchor = this->anchor();
        ListBuilder builder(*this);
        this->read_token();
        while (true) {
            if (this->token == end_token) {
                break;
            } else if (this->token == tok_escape) {
                int column = this->column();
                this->read_token();
                builder.append(parse_naked(column, end_token));
            } else if (this->token == tok_eof) {
                set_active_anchor(start_anchor);
                location_error(String::from("unclosed open bracket"));
            } else if (this->token == tok_statement) {
                builder.split(this->anchor());
                this->read_token();
            } else {
                builder.append(parse_any());
                this->read_token();
            }
        }
        return builder.get_result();
    }

    // parses the next sequence and returns it wrapped in a cell that points
    // to prev
    Any parse_any() {
        assert(this->token != tok_eof);
        const Anchor *anchor = this->anchor();
        if (this->token == tok_open) {
            return Syntax::from(anchor, parse_list(tok_close));
        } else if (this->token == tok_square_open) {
            return Syntax::from(anchor,
                List::from(Syntax::from(anchor,Symbol(SYM_SquareList)),
                    parse_list(tok_square_close)));
        } else if (this->token == tok_curly_open) {
            return Syntax::from(anchor,
                List::from(Syntax::from(anchor,Symbol(SYM_CurlyList)),
                    parse_list(tok_curly_close)));
        } else if ((this->token == tok_close)
            || (this->token == tok_square_close)
            || (this->token == tok_curly_close)) {
            location_error(String::from("stray closing bracket"));
        } else if (this->token == tok_string) {
            return Syntax::from(anchor, get_string());
        } else if (this->token == tok_block_string) {
            return Syntax::from(anchor, get_block_string());
        } else if (this->token == tok_symbol) {
            return Syntax::from(anchor, get_symbol());
        } else if (this->token == tok_number) {
            return Syntax::from(anchor, get_number());
        } else if (this->token == tok_quote) {
            this->read_token();
            if (this->token == tok_eof) {
                set_active_anchor(anchor);
                location_error(
                    String::from("unexpected end of file after quote token"));
            }
            return Syntax::from(anchor,
                List::from({
                    Any(Syntax::from(anchor, Symbol(KW_Quote))),
                    parse_any() }));
        } else {
            location_error(format("unexpected token: %c (%i)",
                this->cursor[0], (int)this->cursor[0]));
        }
        return none;
    }

    Any parse_naked(int column, Token end_token) {
        int lineno = this->lineno;

        bool escape = false;
        int subcolumn = 0;

        const Anchor *anchor = this->anchor();
        ListBuilder builder(*this);

        bool unwrap_single = true;
        while (this->token != tok_eof) {
            if (this->token == end_token) {
                break;
            } else if (this->token == tok_escape) {
                escape = true;
                this->read_token();
                if (this->lineno <= lineno) {
                    location_error(String::from(
                        "escape character is not at end of line"));
                }
                lineno = this->lineno;
            } else if (this->lineno > lineno) {
                if (subcolumn == 0) {
                    subcolumn = this->column();
                } else if (this->column() != subcolumn) {
                    location_error(String::from("indentation mismatch"));
                }
                if (column != subcolumn) {
                    if ((column + 4) != subcolumn) {
                        location_error(String::from(
                            "indentations must nest by 4 spaces."));
                    }
                }

                escape = false;
                lineno = this->lineno;
                // keep adding elements while we're in the same line
                while ((this->token != tok_eof)
                        && (this->token != end_token)
                        && (this->lineno == lineno)) {
                    builder.append(parse_naked(subcolumn, end_token));
                }
            } else if (this->token == tok_statement) {
                this->read_token();
                unwrap_single = false;
                if (!builder.is_empty()) {
                    break;
                }
            } else {
                builder.append(parse_any());
                lineno = this->next_lineno;
                this->read_token();
            }
            if ((!escape || (this->lineno > lineno))
                && (this->column() <= column)) {
                break;
            }
        }

        auto result = builder.get_result();
        if (unwrap_single && result && result->count == 1) {
            return result->at;
        } else {
            return Syntax::from(anchor, result);
        }
    }

    Any parse() {
        this->read_token();
        int lineno = 0;
        //bool escape = false;

        const Anchor *anchor = this->anchor();
        ListBuilder builder(*this);

        while (this->token != tok_eof) {
            if (this->token == tok_none) {
                break;
            } else if (this->token == tok_escape) {
                //escape = true;
                this->read_token();
                if (this->lineno <= lineno) {
                    location_error(String::from(
                        "escape character is not at end of line"));
                }
                lineno = this->lineno;
            } else if (this->lineno > lineno) {
                if (this->column() != 1) {
                    location_error(String::from(
                        "indentation mismatch"));
                }

                //escape = false;
                lineno = this->lineno;
                // keep adding elements while we're in the same line
                while ((this->token != tok_eof)
                        && (this->token != tok_none)
                        && (this->lineno == lineno)) {
                    builder.append(parse_naked(1, tok_none));
                }
            } else if (this->token == tok_statement) {
                location_error(String::from(
                    "unexpected statement token"));
            } else {
                builder.append(parse_any());
                lineno = this->next_lineno;
                this->read_token();
            }
        }
        return Syntax::from(anchor, builder.get_result());
    }

};

//------------------------------------------------------------------------------
// EXPRESSION PRINTER
//------------------------------------------------------------------------------

static const char INDENT_SEP[] = "⁞";

static Style default_symbol_styler(Symbol name) {
    if (!name.is_known())
        return Style_Symbol;
    auto val = name.known_value();
    if ((val >= KEYWORD_FIRST) && (val <= KEYWORD_LAST))
        return Style_Keyword;
    else if ((val >= FUNCTION_FIRST) && (val <= FUNCTION_LAST))
        return Style_Function;
    else if ((val >= SFXFUNCTION_FIRST) && (val <= SFXFUNCTION_LAST))
        return Style_SfxFunction;
    else if ((val >= OPERATOR_FIRST) && (val <= OPERATOR_LAST))
        return Style_Operator;
    return Style_Symbol;
}

struct StreamExprFormat {
    enum Tagging {
        All,
        Line,
        None,
    };

    bool naked;
    Tagging anchors;
    int maxdepth;
    int maxlength;
    Style (*symbol_styler)(Symbol);
    int depth;

    StreamExprFormat() :
        naked(true),
        anchors(None),
        maxdepth(1<<30),
        maxlength(1<<30),
        symbol_styler(default_symbol_styler),
        depth(0)
    {}

    static StreamExprFormat debug() {
        auto fmt = StreamExprFormat();
        fmt.naked = true;
        fmt.anchors = All;
        return fmt;
    }

    static StreamExprFormat debug_digest() {
        auto fmt = StreamExprFormat();
        fmt.naked = true;
        fmt.anchors = Line;
        fmt.maxdepth = 5;
        fmt.maxlength = 5;
        return fmt;
    }

    static StreamExprFormat debug_singleline() {
        auto fmt = StreamExprFormat();
        fmt.naked = false;
        fmt.anchors = All;
        return fmt;
    }

    static StreamExprFormat singleline() {
        auto fmt = StreamExprFormat();
        fmt.naked = false;
        return fmt;
    }

    static StreamExprFormat digest() {
        auto fmt = StreamExprFormat();
        fmt.maxdepth = 5;
        fmt.maxlength = 5;
        return fmt;
    }

    static StreamExprFormat singleline_digest() {
        auto fmt = StreamExprFormat();
        fmt.maxdepth = 5;
        fmt.maxlength = 5;
        fmt.naked = false;
        return fmt;
    }


};

struct StreamAnchors {
    StyledStream &ss;
    const Anchor *last_anchor;

    StreamAnchors(StyledStream &_ss) :
        ss(_ss), last_anchor(nullptr) {
    }

    void stream_anchor(const Anchor *anchor, bool quoted = false) {
        if (anchor) {
            ss << Style_Location;
            auto rss = StyledStream::plain(ss);
            // ss << path.name()->data << ":" << lineno << ":" << column << ":";
            if (!last_anchor || (last_anchor->path() != anchor->path())) {
                rss << anchor->path().name()->data
                    << ":" << anchor->lineno
                    << ":" << anchor->column
                    << ":";
            } else if (!last_anchor || (last_anchor->lineno != anchor->lineno)) {
                rss << ":" << anchor->lineno
                    << ":" << anchor->column
                    << ":";
            } else if (!last_anchor || (last_anchor->column != anchor->column)) {
                rss << "::" << anchor->column
                    << ":";
            } else {
                rss << ":::";
            }
            if (quoted) { rss << "'"; }
            ss << Style_None;
            last_anchor = anchor;
        }
    }
};

struct StreamExpr : StreamAnchors {
    StreamExprFormat fmt;
    bool line_anchors;
    bool atom_anchors;

    StreamExpr(StyledStream &_ss, const StreamExprFormat &_fmt) :
        StreamAnchors(_ss), fmt(_fmt) {
        line_anchors = (fmt.anchors == StreamExprFormat::Line);
        atom_anchors = (fmt.anchors == StreamExprFormat::All);
    }

    void stream_indent(int depth = 0) {
        if (depth >= 1) {
            ss << Style_Comment << "    ";
            for (int i = 2; i <= depth; ++i) {
                ss << INDENT_SEP << "   ";
            }
            ss << Style_None;
        }
    }

    static bool is_nested(const Any &_e) {
        auto e = maybe_unsyntax(_e);
        if (e.type == TYPE_List) {
            auto it = e.list;
            while (it != EOL) {
                auto q = maybe_unsyntax(it->at);
                if (q.type == TYPE_List) {
                    return true;
                }
                it = it->next;
            }
        }
        return false;
    }

    static bool is_list (const Any &_value) {
        auto value = maybe_unsyntax(_value);
        return value.type == TYPE_List;
    }

    void walk(Any e, int depth, int maxdepth, bool naked) {
        bool quoted = false;

        const Anchor *anchor = nullptr;
        if (e.type == TYPE_Syntax) {
            anchor = e.syntax->anchor;
            quoted = e.syntax->quoted;
            e = e.syntax->datum;
        }

        if (naked) {
            stream_indent(depth);
        }
        if (atom_anchors) {
            stream_anchor(anchor, quoted);
        }

        if (e.type == TYPE_List) {
            if (naked && line_anchors && !atom_anchors) {
                stream_anchor(anchor, quoted);
            }

            maxdepth = maxdepth - 1;

            auto it = e.list;
            if (it == EOL) {
                ss << Style_Operator << "()" << Style_None;
                if (naked) { ss << std::endl; }
                return;
            }
            if (maxdepth == 0) {
                ss << Style_Operator << "("
                   << Style_Comment << "<...>"
                   << Style_Operator << ")"
                   << Style_None;
                if (naked) { ss << std::endl; }
                return;
            }
            int offset = 0;
            // int numsublists = 0;
            if (naked) {
                if (is_list(it->at)) {
                    ss << ";" << std::endl;
                    goto print_sparse;
                }
            print_terse:
                walk(it->at, depth, maxdepth, false);
                it = it->next;
                offset = offset + 1;
                while (it != EOL) {
                    if (is_nested(it->at)) {
                        break;
                    }
                    ss << " ";
                    walk(it->at, depth, maxdepth, false);
                    offset = offset + 1;
                    it = it->next;
                }
                ss << std::endl;
            print_sparse:
                int subdepth = depth + 1;
                while (it != EOL) {
                    auto value = it->at;
                    if (!is_list(value) // not a list
                        && (offset >= 1)) { // not first element in list
                        stream_indent(subdepth);
                        ss << "\\ ";
                        goto print_terse;
                    }
                    if (offset >= fmt.maxlength) {
                        stream_indent(subdepth);
                        ss << "<...>" << std::endl;
                        return;
                    }
                    walk(value, subdepth, maxdepth, true);
                    offset = offset + 1;
                    it = it->next;
                }
            } else {
                depth = depth + 1;
                ss << Style_Operator << "(" << Style_None;
                while (it != EOL) {
                    if (offset > 0) {
                        ss << " ";
                    }
                    if (offset >= fmt.maxlength) {
                        ss << Style_Comment << "..." << Style_None;
                        break;
                    }
                    walk(it->at, depth, maxdepth, false);
                    offset = offset + 1;
                    it = it->next;
                }
                ss << Style_Operator << ")" << Style_None;
            }
        } else {
            if (e.type == TYPE_Symbol) {
                ss << fmt.symbol_styler(e.symbol);
                e.symbol.name()->stream(ss, SYMBOL_ESCAPE_CHARS);
                ss << Style_None;
            } else {
                ss << e;
            }
            if (naked) { ss << std::endl; }
        }
    }

    void stream(const Any &e) {
        walk(e, fmt.depth, fmt.maxdepth, fmt.naked);
    }
};

static void stream_expr(
    StyledStream &_ss, const Any &e, const StreamExprFormat &_fmt) {
    StreamExpr streamer(_ss, _fmt);
    streamer.stream(e);
}

static StyledStream& operator<<(StyledStream& ost, const List *list) {
    stream_expr(ost, list, StreamExprFormat::singleline());
    return ost;
}

//------------------------------------------------------------------------------
// IL OBJECTS
//------------------------------------------------------------------------------

// IL form inspired by
// Leissa et al., Graph-Based Higher-Order Intermediate Representation
// http://compilers.cs.uni-saarland.de/papers/lkh15_cgo.pdf


enum {
    ARG_Cont = 0,
    ARG_Arg0 = 1,
    PARAM_Cont = 0,
    PARAM_Arg0 = 1,
};

enum ParameterKind {
    PK_Regular = 0,
    PK_Variadic = 1,
};

struct Parameter {
protected:
    Parameter(const Anchor *_anchor, Symbol _name, const Type *_type, ParameterKind _kind) :
        anchor(_anchor), name(_name), type(_type), label(nullptr), index(-1),
        kind(_kind) {}

public:
    const Anchor *anchor;
    Symbol name;
    const Type *type;
    Label *label;
    int index;
    ParameterKind kind;

    bool is_vararg() const {
        return (kind == PK_Variadic);
    }

    bool is_typed() const {
        return type != TYPE_Unknown;
    }

    bool is_none() const {
        return type == TYPE_Nothing;
    }

    StyledStream &stream_local(StyledStream &ss) const {
        if ((name != SYM_Unnamed) || !label) {
            ss << Style_Symbol;
            name.name()->stream(ss, SYMBOL_ESCAPE_CHARS);
            ss << Style_None;
        } else {
            ss << Style_Operator << "@" << Style_None << index;
        }
        if (is_vararg()) {
            ss << Style_Keyword << "…" << Style_None;
        }
        if (is_typed()) {
            ss << Style_Operator << ":" << Style_None << type;
        }
        return ss;
    }
    StyledStream &stream(StyledStream &ss) const;

    static Parameter *from(const Parameter *_param) {
        return new Parameter(
            _param->anchor, _param->name, _param->type, _param->kind);
    }

    static Parameter *from(const Anchor *_anchor, Symbol _name, const Type *_type) {
        return new Parameter(_anchor, _name, _type, PK_Regular);
    }

    static Parameter *variadic_from(const Anchor *_anchor, Symbol _name, const Type *_type) {
        return new Parameter(_anchor, _name, _type, PK_Variadic);
    }
};

void Any::verify_indirect(const Type *T) const {
    scopes::verify(T, indirect_type());
}

bool Any::is_const() const {
    return !((type == TYPE_Parameter) && parameter->label);
}

const Type *Any::indirect_type() const {
    if (!is_const()) {
        return parameter->type;
    } else {
        return type;
    }
}

static StyledStream& operator<<(StyledStream& ss, Parameter *param) {
    param->stream(ss);
    return ss;
}

//------------------------------------------------------------------------------

enum LabelBodyFlags {
    LBF_RawCall = (1 << 0),
    LBF_Complete = (1 << 1)
};

struct Body {
    const Anchor *anchor;
    Any enter;
    Args args;
    uint64_t flags;

    // if there's a scope label, the current frame will be truncated to the
    // parent frame that maps the scope label.
    Label *scope_label;

    Body() :
        anchor(nullptr), enter(none), flags(0), scope_label(nullptr) {}

    bool is_complete() const {
        return flags & LBF_Complete;
    }
    void set_complete() {
        flags |= LBF_Complete;
    }
    void unset_complete() {
        flags &= ~LBF_Complete;
    }

    bool is_rawcall() {
        return (flags & LBF_RawCall) == LBF_RawCall;
    }

    void set_rawcall(bool enable = true) {
        if (enable) {
            flags |= LBF_RawCall;
        } else {
            flags &= ~LBF_RawCall;
        }
    }

    void copy_traits_from(const Body &other) {
        flags = other.flags;
        anchor = other.anchor;
        scope_label = other.scope_label;
    }
};

static const char CONT_SEP[] = "▶";

template<typename T>
struct Tag {
    static uint64_t active_gen;
    uint64_t gen;

    Tag() :
        gen(active_gen) {}

    static void clear() {
        active_gen++;
    }
    bool visited() const {
        return gen == active_gen;
    }
    void visit() {
        gen = active_gen;
    }
};

template<typename T>
uint64_t Tag<T>::active_gen = 0;

typedef Tag<Label> LabelTag;

enum LabelFlags {
    LF_Template = (1 << 0),
    // label is reentrant
    LF_Reentrant = (1 << 1),
    // this label is an inline
    LF_Inline = (1 << 2),
    // this label can not be instantiated more than once per frame and forces
    // all callers to use the same arguments
    LF_Merge = (1 << 3),
    // when this label is processed, output some information to screen
    LF_Debug = (1 << 4),
};

struct Label {
protected:
    static uint64_t next_uid;

    Label(const Anchor *_anchor, Symbol _name, uint64_t _flags) :
        original(nullptr), docstring(nullptr),
        uid(0), next_instanceid(1), anchor(_anchor), name(_name),
        paired(nullptr), flags(_flags)
        {}

public:
    Label *original;
    const String *docstring;
    size_t uid;
    size_t next_instanceid;
    const Anchor *anchor;
    Symbol name;
    Parameters params;
    Body body;
    LabelTag tag;
    Label *paired;
    uint64_t flags;

    void set_reentrant() {
        flags |= LF_Reentrant;
    }

    bool is_reentrant() const {
        return flags & LF_Reentrant;
    }

    bool is_debug() const {
        return flags & LF_Debug;
    }

    void set_debug() {
        flags |= LF_Debug;
    }

    void set_merge() {
        flags |= LF_Merge;
    }

    void unset_merge() {
        flags &= ~LF_Merge;
    }

    void set_inline() {
        flags |= LF_Inline;
    }

    void unset_inline() {
        flags &= ~LF_Inline;
    }

    bool is_merge() const {
        return flags & LF_Merge;
    }

    bool is_inline() const {
        return flags & LF_Inline;
    }

    bool is_important() const {
        return flags & (LF_Merge | LF_Reentrant);
    }

    bool is_template() const {
        return flags & LF_Template;
    }

    Parameter *get_param_by_name(Symbol name) {
        size_t count = params.size();
        for (size_t i = 1; i < count; ++i) {
            if (params[i]->name == name) {
                return params[i];
            }
        }
        return nullptr;
    }

    bool is_jumping() const {
        auto &&args = body.args;
        assert(!args.empty());
        return args[0].value.type == TYPE_Nothing;
    }

    bool is_calling(Label *callee) const {
        auto &&enter = body.enter;
        return (enter.type == TYPE_Label) && (enter.label == callee);
    }

    bool is_continuing_to(Label *callee) const {
        auto &&args = body.args;
        assert(!args.empty());
        return (args[0].value.type == TYPE_Label) && (args[0].value.label == callee);
    }

    bool is_basic_block_like() const {
        if (params.empty())
            return true;
        if (params[0]->type == TYPE_Nothing)
            return true;
        return false;
    }

    bool is_return_param_typed() const {
        assert(!params.empty());
        return params[0]->is_typed();
    }

    bool has_params() const {
        return params.size() > 1;
    }

    bool is_variadic() const {
        return (!params.empty() && params.back()->is_vararg());
    }

    bool is_valid() const {
        return !params.empty() && body.anchor && !body.args.empty();
    }

    void verify_valid () {
        const String *msg = nullptr;
        if (params.empty()) {
            msg = String::from("label corrupt: parameters are missing");
        } else if (!body.anchor) {
            msg = String::from("label corrupt: body anchor is missing");
        } else if (body.args.empty()) {
            msg = String::from("label corrupt: body arguments are missing");
        }
        if (msg) {
            set_active_anchor(anchor);
            location_error(msg);
        }
    }

    struct UserMap {
        std::unordered_map<Label *, std::unordered_set<Label *> > label_map;
        std::unordered_map<Parameter *, std::unordered_set<Label *> > param_map;

        void clear() {
            label_map.clear();
            param_map.clear();
        }

        void insert(Label *source, Label *dest) {
            label_map[dest].insert(source);
        }

        void insert(Label *source, Parameter *dest) {
            param_map[dest].insert(source);
        }

        void remove(Label *source, Label *dest) {
            auto it = label_map.find(dest);
            if (it != label_map.end()) {
                it->second.erase(source);
            }
        }

        void remove(Label *source, Parameter *dest) {
            auto it = param_map.find(dest);
            if (it != param_map.end()) {
                it->second.erase(source);
            }
        }

        void stream_users(const std::unordered_set<Label *> &users,
            StyledStream &ss) const {
            ss << Style_Comment << "{" << Style_None;
            size_t i = 0;
            for (auto &&kv : users) {
                if (i > 0) {
                    ss << " ";
                }
                Label *label = kv;
                label->stream_short(ss);
                i++;
            }
            ss << Style_Comment << "}" << Style_None;
        }

        void stream_users(Label *node, StyledStream &ss) const {
            auto it = label_map.find(node);
            if (it != label_map.end()) stream_users(it->second, ss);
        }

        void stream_users(Parameter *node, StyledStream &ss) const {
            auto it = param_map.find(node);
            if (it != param_map.end()) stream_users(it->second, ss);
        }
    };

    Label *get_label_enter() const {
        assert(body.enter.type == TYPE_Label);
        return body.enter.label;
    }

    const Closure *get_closure_enter() const {
        assert(body.enter.type == TYPE_Closure);
        return body.enter.closure;
    }

    Builtin get_builtin_enter() const {
        assert(body.enter.type == TYPE_Builtin);
        return body.enter.builtin;
    }

    Label *get_label_cont() const {
        assert(!body.args.empty());
        assert(body.args[0].value.type == TYPE_Label);
        return body.args[0].value.label;
    }

    const ReturnLabelType *verify_return_label();

    const Type *get_return_type() const {
        assert(params.size());
        assert(!is_basic_block_like());
        if (!params[0]->is_typed())
            return TYPE_Void;
        // verify that the return type is the one we expect
        cast<ReturnLabelType>(params[0]->type);
        return params[0]->type;
    }

    void verify_compilable() const {
        if (params[0]->is_typed()
            && !params[0]->is_none()) {
            auto tl = dyn_cast<ReturnLabelType>(params[0]->type);
            if (!tl) {
                set_active_anchor(anchor);
                StyledString ss;
                ss.out << "cannot compile function with return type "
                    << params[0]->type;
                location_error(ss.str());
            }
            for (size_t i = 0; i < tl->values.size(); ++i) {
                auto &&val = tl->values[i].value;
                if (is_unknown(val)) {
                    auto T = val.typeref;
                    if (is_opaque(T)) {
                        set_active_anchor(anchor);
                        StyledString ss;
                        ss.out << "cannot compile function with opaque return argument of type "
                            << T;
                        location_error(ss.str());
                    }
                }
            }
        }

        ArgTypes argtypes;
        for (size_t i = 1; i < params.size(); ++i) {
            auto T = params[i]->type;
            if (T == TYPE_Unknown) {
                set_active_anchor(anchor);
                location_error(String::from("cannot compile function with untyped argument"));
            } else if (is_invalid_argument_type(T)) {
                set_active_anchor(anchor);
                StyledString ss;
                ss.out << "cannot compile function with opaque argument of type "
                    << T;
                location_error(ss.str());
            }
        }
    }

    const Type *get_params_as_return_label_type() const {
        scopes::Args values;
        for (size_t i = 1; i < params.size(); ++i) {
            values.push_back(unknown_of(params[i]->type));
        }
        return ReturnLabel(values);
    }

    const Type *get_function_type() const {

        ArgTypes argtypes;
        for (size_t i = 1; i < params.size(); ++i) {
            argtypes.push_back(params[i]->type);
        }
        uint64_t flags = 0;
        assert(params.size());
        if (!params[0]->is_typed()) {
            flags |= FF_Divergent;
        }
        return Function(get_return_type(), argtypes, flags);
    }

    void use(UserMap &um, const Any &arg, int i) {
        if (arg.type == TYPE_Parameter && (arg.parameter->label != this)) {
            um.insert(this, arg.parameter /*, i*/);
        } else if (arg.type == TYPE_Label && (arg.label != this)) {
            um.insert(this, arg.label /*, i*/);
        }
    }

    void unuse(UserMap &um, const Any &arg, int i) {
        if (arg.type == TYPE_Parameter && (arg.parameter->label != this)) {
            um.remove(this, arg.parameter /*, i*/);
        } else if (arg.type == TYPE_Label && (arg.label != this)) {
            um.remove(this, arg.label /*, i*/);
        }
    }

    void insert_into_usermap(UserMap &um) {
        use(um, body.enter, -1);
        size_t count = body.args.size();
        for (size_t i = 0; i < count; ++i) {
            use(um, body.args[i].value, i);
        }
    }

    void remove_from_usermap(UserMap &um) {
        unuse(um, body.enter, -1);
        size_t count = body.args.size();
        for (size_t i = 0; i < count; ++i) {
            unuse(um, body.args[i].value, i);
        }
    }

    void append(Parameter *param) {
        assert(!param->label);
        param->label = this;
        param->index = (int)params.size();
        params.push_back(param);
    }

    void set_parameters(const Parameters &_params) {
        assert(params.empty());
        params = _params;
        for (size_t i = 0; i < params.size(); ++i) {
            Parameter *param = params[i];
            assert(!param->label);
            param->label = this;
            param->index = (int)i;
        }
    }

    void build_reachable(std::unordered_set<Label *> &labels,
        Labels *ordered_labels = nullptr) {
        labels.clear();
        labels.insert(this);
        if (ordered_labels)
            ordered_labels->push_back(this);
        Labels foreign_stack = { this };
        Labels stack = {};
        while (true) {
            Label *parent = nullptr;
            if (!stack.empty()) {
                parent = stack.back();
                stack.pop_back();
            } else if (!foreign_stack.empty()) {
                parent = foreign_stack.back();
                foreign_stack.pop_back();
                if (ordered_labels)
                    ordered_labels->push_back(parent);
            } else {
                 break;
            }

            int size = (int)parent->body.args.size();
            for (int i = -1; i < size; ++i) {
                Any arg = none;
                if (i == -1) {
                    arg = parent->body.enter;
                } else {
                    arg = parent->body.args[i].value;
                }

                if (arg.type == TYPE_Label) {
                    Label *label = arg.label;
                    if (!labels.count(label)) {
                        labels.insert(label);
                        if (label->is_basic_block_like()) {
                            if (ordered_labels)
                                ordered_labels->push_back(label);
                            stack.push_back(label);
                        } else {
                            foreign_stack.push_back(label);
                        }
                    }
                }
            }
        }
    }

    void build_scope(UserMap &um, Labels &tempscope) {
        tempscope.clear();

        std::unordered_set<Label *> visited;
        visited.clear();
        visited.insert(this);

        for (auto &&param : params) {
            auto it = um.param_map.find(param);
            if (it != um.param_map.end()) {
                auto &&users = it->second;
                // every label using one of our parameters is live in scope
                for (auto &&kv : users) {
                    Label *live_label = kv;
                    if (!visited.count(live_label)) {
                        visited.insert(live_label);
                        tempscope.push_back(live_label);
                    }
                }
            }
        }

        size_t index = 0;
        while (index < tempscope.size()) {
            Label *scope_label = tempscope[index++];

            auto it = um.label_map.find(scope_label);
            if (it != um.label_map.end()) {
                auto &&users = it->second;
                // users of scope_label are indirectly live in scope
                for (auto &&kv : users) {
                    Label *live_label = kv;
                    if (!visited.count(live_label)) {
                        visited.insert(live_label);
                        tempscope.push_back(live_label);
                    }
                }
            }

            for (auto &&param : scope_label->params) {
                auto it = um.param_map.find(param);
                if (it != um.param_map.end()) {
                    auto &&users = it->second;
                    // every label using scope_label's parameters is live in scope
                    for (auto &&kv : users) {
                        Label *live_label = kv;
                        if (!visited.count(live_label)) {
                            visited.insert(live_label);
                            tempscope.push_back(live_label);
                        }
                    }
                }
            }
        }
    }

    void build_scope(Labels &tempscope) {
        std::unordered_set<Label *> visited;
        Labels reachable;
        build_reachable(visited, &reachable);
        UserMap um;
        for (auto it = reachable.begin(); it != reachable.end(); ++it) {
            (*it)->insert_into_usermap(um);
        }

        build_scope(um, tempscope);
    }

    Label *get_original() {
        Label *l = this;
        while (l->original)
            l = l->original;
        return l;
    }

    StyledStream &stream_id(StyledStream &ss) const {
        if (original) {
            original->stream_id(ss);
        }
        ss << uid;
        if (uid >= 10) {
            ss << "x";
        }
        return ss;
    }

    StyledStream &stream_short(StyledStream &ss) const {
#if SCOPES_DEBUG_CODEGEN
        if (is_template()) {
            ss << Style_Keyword << "T:" << Style_None;
        }
#endif
        if (name != SYM_Unnamed) {
            ss << Style_Symbol;
            name.name()->stream(ss, SYMBOL_ESCAPE_CHARS);
        }
        ss << Style_Keyword << "λ" << Style_Symbol;
        {
            StyledStream ps = StyledStream::plain(ss);
            if (!original) {
                ps << uid;
            } else {
                stream_id(ps);
            }
        }
        ss << Style_None;
        return ss;
    }

    StyledStream &stream(StyledStream &ss, bool users = false) const {
        stream_short(ss);
        ss << Style_Operator << "(" << Style_None;
        size_t count = params.size();
        for (size_t i = 1; i < count; ++i) {
            if (i > 1) {
                ss << " ";
            }
            params[i]->stream_local(ss);
        }
        ss << Style_Operator << ")" << Style_None;
        if (count) {
            const Type *rtype = params[0]->type;
            if (rtype != TYPE_Nothing) {
                ss << Style_Comment << CONT_SEP << Style_None;
                if (rtype == TYPE_Unknown) {
                    ss << Style_Comment << "?" << Style_None;
                } else {
                    params[0]->stream_local(ss);
                }
            }
        }
        return ss;
    }

    static Label *from(const Anchor *_anchor, Symbol _name) {
        assert(_anchor);
        Label *result = new Label(_anchor, _name, LF_Template);
        result->uid = next_uid++;
        return result;
    }
    // only inherits name and anchor
    static Label *from(Label *label) {
        Label *result = new Label(label->anchor, label->name, 0);
        result->original = label;
        result->uid = label->next_instanceid++;
        result->flags |= label->flags & (LF_Merge | LF_Debug);
        return result;
    }

    // a continuation that never returns
    static Label *continuation_from(const Anchor *_anchor, Symbol _name) {
        Label *value = from(_anchor, _name);
        // first argument is present, but unused
        value->append(Parameter::from(_anchor, _name, TYPE_Nothing));
        return value;
    }

    // an inline function that eventually returns
    static Label *inline_from(const Anchor *_anchor, Symbol _name) {
        Label *value = from(_anchor, _name);
        // continuation is always first argument
        // this argument will be eventually inlined
        value->append(
            Parameter::from(_anchor,
                Symbol(SYM_Unnamed),
                TYPE_Unknown));
        value->set_inline();
        return value;
    }

    // a function that eventually returns
    static Label *function_from(const Anchor *_anchor, Symbol _name) {
        Label *value = from(_anchor, _name);
        // continuation is always first argument
        // this argument will be called when the function is done
        value->append(
            Parameter::from(_anchor,
                Symbol(SYM_Unnamed),
                TYPE_Unknown));
        return value;
    }

};

uint64_t Label::next_uid = 0;

static StyledStream& operator<<(StyledStream& ss, Label *label) {
    label->stream(ss);
    return ss;
}

static StyledStream& operator<<(StyledStream& ss, const Label *label) {
    label->stream(ss);
    return ss;
}

StyledStream &Parameter::stream(StyledStream &ss) const {
    if (label) {
        label->stream_short(ss);
    } else {
        ss << Style_Comment << "<unbound>" << Style_None;
    }
    ss << Style_Comment << "." << Style_None;
    stream_local(ss);
    return ss;
}

//------------------------------------------------------------------------------

struct Closure {
protected:

    Closure(Label *_label, Frame *_frame) :
        label(_label), frame(_frame) {}

public:

    struct Hash {
        std::size_t operator()(const Closure &k) const {
            return HashLen16(
                std::hash<Label *>{}(k.label),
                std::hash<Frame *>{}(k.frame));
        }
    };

    bool operator ==(const Closure &k) const {
        return (label == k.label)
            && (frame == k.frame);
    }

    static std::unordered_map<Closure, const Closure *, Closure::Hash> map;

    Label *label;
    Frame *frame;

    static const Closure *from(Label *label, Frame *frame) {
        assert (label->is_template());
        Closure cl(label, frame);
        auto it = map.find(cl);
        if (it != map.end()) {
            return it->second;
        }
        const Closure *result = new Closure(label, frame);
        map.insert({cl, result});
        return result;
    }

    StyledStream &stream(StyledStream &ost) const {
        ost << Style_Comment << "<" << Style_None
            << frame
            << Style_Comment << "::" << Style_None;
        label->stream_short(ost);
        ost << Style_Comment << ">" << Style_None;
        return ost;
    }
};

std::unordered_map<Closure, const Closure *, Closure::Hash> Closure::map;

static StyledStream& operator<<(StyledStream& ss, const Closure *closure) {
    closure->stream(ss);
    return ss;
}

//------------------------------------------------------------------------------

struct Frame {
    Frame() :
        parent(nullptr),
        label(nullptr),
        loop_count(0),
        inline_merge(false),
        instance(nullptr)
    {}
    Frame(Frame *_parent, Label *_label, Label *_instance = nullptr, size_t _loop_count = 0) :
        parent(_parent),
        label(_label),
        loop_count(_loop_count),
        inline_merge(false),
        instance(_instance) {
        assert(parent);
        assert(label);
        args.reserve(_label->params.size());
    }

    Args args;
    Frame *parent;
    Label *label;
    size_t loop_count;
    bool inline_merge;

    Frame *find_parent_frame(Label *label) {
        Frame *top = this;
        while (top) {
            if (top->label == label) {
                return top;
            }
            top = top->parent;
        }
        return nullptr;
    }

    static Frame *from(Frame *parent, Label *label, Label *instance, size_t loop_count) {
        return new Frame(parent, label, instance, loop_count);
    }

    bool all_args_constant() const {
        for (size_t i = 1; i < args.size(); ++i) {
            if (is_unknown(args[i].value))
                return false;
            if (!args[i].value.is_const())
                return false;
        }
        return true;
    }

    struct ArgsKey {
        Label *label;
        scopes::Args args;

        ArgsKey() : label(nullptr) {}

        bool operator==(const ArgsKey &other) const {
            if (label != other.label) return false;
            if (args.size() != other.args.size()) return false;
            for (size_t i = 0; i < args.size(); ++i) {
                auto &&a = args[i];
                auto &&b = other.args[i];
                if (a != b)
                    return false;
            }
            return true;
        }

        struct Hash {
            std::size_t operator()(const ArgsKey& s) const {
                std::size_t h = std::hash<Label *>{}(s.label);
                for (auto &&arg : s.args) {
                    h = HashLen16(h, arg.hash());
                }
                return h;
            }
        };

    };

    Frame *find_frame(const ArgsKey &key) const {
        auto it = frames.find(key);
        if (it != frames.end())
            return it->second;
        return nullptr;
    }

    Frame *find_any_frame(Label *label, ArgsKey &key) const {
        for (auto &&it : frames) {
            if (it.first.label == label) {
                key = it.first;
                return it.second;
            }
        }
        return nullptr;
    }

    void insert_frame(const ArgsKey &key, Frame *frame) {
        frames.insert({key, frame});
    }

    Label *get_instance() const {
        return instance;
    }

    static Frame *root;
protected:
    std::unordered_map<ArgsKey, Frame *, ArgsKey::Hash> frames;
    Label *instance;
};

Frame *Frame::root = nullptr;

//------------------------------------------------------------------------------
// IL PRINTER
//------------------------------------------------------------------------------

struct StreamLabelFormat {
    enum Tagging {
        All,
        Line,
        Scope,
        None,
    };

    Tagging anchors;
    Tagging follow;
    bool show_users;
    bool show_scope;

    StreamLabelFormat() :
        anchors(None),
        follow(All),
        show_users(false),
        show_scope(false)
        {}

    static StreamLabelFormat debug_all() {
        StreamLabelFormat fmt;
        fmt.follow = All;
        fmt.show_users = true;
        fmt.show_scope = true;
        return fmt;
    }

    static StreamLabelFormat debug_scope() {
        StreamLabelFormat fmt;
        fmt.follow = Scope;
        fmt.show_users = true;
        return fmt;
    }

    static StreamLabelFormat debug_single() {
        StreamLabelFormat fmt;
        fmt.follow = None;
        fmt.show_users = true;
        fmt.anchors = Line;
        return fmt;
    }

    static StreamLabelFormat single() {
        StreamLabelFormat fmt;
        fmt.follow = None;
        return fmt;
    }

    static StreamLabelFormat scope() {
        StreamLabelFormat fmt;
        fmt.follow = Scope;
        return fmt;
    }

};

struct StreamLabel : StreamAnchors {
    StreamLabelFormat fmt;
    bool line_anchors;
    bool atom_anchors;
    bool follow_labels;
    bool follow_scope;
    std::unordered_set<Label *> visited;

    StreamLabel(StyledStream &_ss, const StreamLabelFormat &_fmt) :
        StreamAnchors(_ss), fmt(_fmt) {
        line_anchors = (fmt.anchors == StreamLabelFormat::Line);
        atom_anchors = (fmt.anchors == StreamLabelFormat::All);
        follow_labels = (fmt.follow == StreamLabelFormat::All);
        follow_scope = (fmt.follow == StreamLabelFormat::Scope);
    }

    void stream_label_label(Label *alabel) {
        alabel->stream_short(ss);
    }

    void stream_label_label_user(Label *alabel) {
        alabel->stream_short(ss);
    }

    void stream_param_label(Parameter *param, Label *alabel) {
        if (param->label == alabel) {
            param->stream_local(ss);
        } else {
            param->stream(ss);
        }
    }

    void stream_argument(Argument arg, Label *alabel) {
        if (arg.key != SYM_Unnamed) {
            ss << arg.key << Style_Operator << "=" << Style_None;
        }
        if (arg.value.type == TYPE_Parameter) {
            stream_param_label(arg.value.parameter, alabel);
        } else if (arg.value.type == TYPE_Label) {
            stream_label_label(arg.value.label);
        } else if (arg.value.type == TYPE_List) {
            stream_expr(ss, arg.value, StreamExprFormat::singleline_digest());
        } else {
            ss << arg.value;
        }
    }

    void stream_label (Label *alabel) {
        if (visited.count(alabel)) {
            return;
        }
        visited.insert(alabel);
        if (line_anchors) {
            stream_anchor(alabel->anchor);
        }
        if (alabel->is_inline()) {
            ss << Style_Keyword << "inline" << Style_None;
            ss << " ";
        }
        if (alabel->is_reentrant()) {
            ss << Style_Keyword << "reentrant" << Style_None;
            ss << " ";
        }
        if (alabel->is_merge()) {
            ss << Style_Keyword << "merge" << Style_None;
            ss << " ";
        }
        alabel->stream(ss, fmt.show_users);
        ss << Style_Operator << ":" << Style_None;
        if (fmt.show_scope && alabel->body.scope_label) {
            ss << " " << Style_Operator << "[" << Style_None;
            alabel->body.scope_label->stream_short(ss);
            ss << Style_Operator << "]" << Style_None;
        }
        //stream_scope(scopes[alabel])
        ss << std::endl;
        ss << "    ";
        if (line_anchors && alabel->body.anchor) {
            stream_anchor(alabel->body.anchor);
            ss << " ";
        }
        if (!alabel->body.is_complete()) {
            ss << Style_Keyword << "T " << Style_None;
        }
        if (alabel->body.is_rawcall()) {
            ss << Style_Keyword << "rawcall " << Style_None;
        }
        stream_argument(alabel->body.enter, alabel);
        for (size_t i=1; i < alabel->body.args.size(); ++i) {
            ss << " ";
            stream_argument(alabel->body.args[i], alabel);
        }
        if (!alabel->body.args.empty()) {
            auto &&cont = alabel->body.args[0];
            if (cont.value.type != TYPE_Nothing) {
                ss << " " << Style_Comment << CONT_SEP << Style_None << " ";
                stream_argument(cont.value, alabel);
            }
        }
        ss << std::endl;

        if (follow_labels) {
            for (size_t i=0; i < alabel->body.args.size(); ++i) {
                stream_any(alabel->body.args[i].value);
            }
            stream_any(alabel->body.enter);
        }
    }

    void stream_any(const Any &afunc) {
        if (afunc.type == TYPE_Label) {
            stream_label(afunc.label);
        }
    }

    void stream(Label *label) {
        stream_label(label);
        if (follow_scope) {
            Labels scope;
            label->build_scope(scope);
            size_t i = scope.size();
            while (i > 0) {
                --i;
                stream_label(scope[i]);
            }
        }
    }

};

static void stream_label(
    StyledStream &_ss, Label *label, const StreamLabelFormat &_fmt) {
    StreamLabel streamer(_ss, _fmt);
    streamer.stream(label);
}

const ReturnLabelType *Label::verify_return_label() {
    if (!params.empty()) {
        const ReturnLabelType *rt = dyn_cast<ReturnLabelType>(params[0]->type);
        if (rt)
            return rt;
    }
#if SCOPES_DEBUG_CODEGEN
    {
        StyledStream ss;
        stream_label(ss, this, StreamLabelFormat::debug_all());
    }
#endif
    set_active_anchor(anchor);
    location_error(String::from("label is not a function"));
    return nullptr;
}

//------------------------------------------------------------------------------
// FRAME PRINTER
//------------------------------------------------------------------------------

struct StreamFrameFormat {
    enum Tagging {
        All,
        None,
    };

    Tagging follow;

    StreamFrameFormat()
        : follow(All)
    {}

    static StreamFrameFormat single() {
        StreamFrameFormat fmt;
        fmt.follow = None;
        return fmt;
    }
};

struct StreamFrame : StreamAnchors {
    bool follow_all;
    StreamFrameFormat fmt;

    StreamFrame(StyledStream &_ss, const StreamFrameFormat &_fmt) :
        StreamAnchors(_ss), fmt(_fmt) {
        follow_all = (fmt.follow == StreamFrameFormat::All);
    }

    void stream_frame(const Frame *frame) {
        if (follow_all) {
            if (frame->parent != Frame::root)
                stream_frame(frame->parent);
        }
        ss << frame;
        if (frame->loop_count) {
            ss << " [loop=" << frame->loop_count << "]" << std::endl;
        }
        ss << std::endl;
        //ss << "    instance = " << frame->instance << std::endl;
        ss << "    original label = " << frame->label << std::endl;
        auto &&args = frame->args;
        for (size_t i = 0; i < args.size(); ++i) {
            ss << "    " << i << " = " << args[i] << std::endl;
        }
    }

    void stream(const Frame *frame) {
        stream_frame(frame);
    }

};

static void stream_frame(
    StyledStream &_ss, const Frame *frame, const StreamFrameFormat &_fmt) {
    StreamFrame streamer(_ss, _fmt);
    streamer.stream(frame);
}

//------------------------------------------------------------------------------
// IL EVAL/APPLY
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// C BRIDGE (CLANG)
//------------------------------------------------------------------------------

class CVisitor : public clang::RecursiveASTVisitor<CVisitor> {
public:

    typedef std::unordered_map<Symbol, const Type *, Symbol::Hash> NamespaceMap;

    Scope *dest;
    clang::ASTContext *Context;
    std::unordered_map<clang::RecordDecl *, bool> record_defined;
    std::unordered_map<clang::EnumDecl *, bool> enum_defined;
    NamespaceMap named_structs;
    NamespaceMap named_classes;
    NamespaceMap named_unions;
    NamespaceMap named_enums;
    NamespaceMap typedefs;

    CVisitor() : dest(nullptr), Context(NULL) {
        const Type *T = Typename(String::from("__builtin_va_list"));
        auto tnt = cast<TypenameType>(const_cast<Type*>(T));
        tnt->finalize(Array(TYPE_I8, sizeof(va_list)));
        typedefs.insert({Symbol("__builtin_va_list"), T });
    }

    const Anchor *anchorFromLocation(clang::SourceLocation loc) {
        auto &SM = Context->getSourceManager();

        auto PLoc = SM.getPresumedLoc(loc);

        if (PLoc.isValid()) {
            auto fname = PLoc.getFilename();
            const String *strpath = String::from_cstr(fname);
            Symbol key(strpath);
            SourceFile *sf = SourceFile::from_file(key);
            if (!sf) {
                sf = SourceFile::from_string(key, Symbol(SYM_Unnamed).name());
            }
            return Anchor::from(sf, PLoc.getLine(), PLoc.getColumn(),
                SM.getFileOffset(loc));
        }

        return get_active_anchor();
    }

    void SetContext(clang::ASTContext * ctx, Scope *_dest) {
        Context = ctx;
        dest = _dest;
    }

    void GetFields(TypenameType *tni, clang::RecordDecl * rd) {
        auto &rl = Context->getASTRecordLayout(rd);

        bool is_union = rd->isUnion();

        if (is_union) {
            tni->super_type = TYPE_CUnion;
        } else {
            tni->super_type = TYPE_CStruct;
        }

        std::vector<Argument> args;
        //auto anchors = new std::vector<Anchor>();
        //StyledStream ss;
        const Type *ST = tni;

        size_t sz = 0;
        size_t al = 1;
        bool packed = false;
        bool has_bitfield = false;
        for(clang::RecordDecl::field_iterator it = rd->field_begin(), end = rd->field_end(); it != end; ++it) {
            clang::DeclarationName declname = it->getDeclName();

            if (!it->isAnonymousStructOrUnion() && !declname) {
                continue;
            }

            clang::QualType FT = it->getType();
            const Type *fieldtype = TranslateType(FT);

            if(it->isBitField()) {
                has_bitfield = true;
                break;
            }

            //unsigned width = it->getBitWidthValue(*Context);

            Symbol name = it->isAnonymousStructOrUnion() ?
                Symbol("") : Symbol(String::from_stdstring(declname.getAsString()));

            if (!is_union) {
                //ss << "type " << ST << " field " << name << " : " << fieldtype << std::endl;

                unsigned idx = it->getFieldIndex();
                auto offset = rl.getFieldOffset(idx) / 8;
                size_t newsz = sz;
                if (!packed) {
                    size_t etal = align_of(fieldtype);
                    newsz = scopes::align(sz, etal);
                    al = std::max(al, etal);
                }
                if (newsz != offset) {
                    //ss << "offset mismatch " << newsz << " != " << offset << std::endl;
                    if (newsz < offset) {
                        size_t pad = offset - newsz;
                        args.push_back(Argument(SYM_Unnamed,
                            Array(TYPE_U8, pad)));
                    } else {
                        // our computed offset is later than the real one
                        // structure is likely packed
                        packed = true;
                    }
                }
                sz = offset + size_of(fieldtype);
            } else {
                sz = std::max(sz, size_of(fieldtype));
                al = std::max(al, align_of(fieldtype));
            }

            args.push_back(Argument(name, unknown_of(fieldtype)));
        }
        if (packed) {
            al = 1;
        }
        bool explicit_alignment = false;
        if (has_bitfield) {
            // ignore for now and hope that an underlying union fixes the problem
        } else {
            size_t needalign = rl.getAlignment().getQuantity();
            size_t needsize = rl.getSize().getQuantity();
            #if 0
            if (!is_union && (needalign != al)) {
                al = needalign;
                explicit_alignment = true;
            }
            #endif
            sz = scopes::align(sz, al);
            bool align_ok = (al == needalign);
            bool size_ok = (sz == needsize);
            if (!(align_ok && size_ok)) {
#ifdef SCOPES_DEBUG
                StyledStream ss;
                auto anchor = anchorFromLocation(rd->getSourceRange().getBegin());
                if (al != needalign) {
                    ss << anchor << " type " << ST << " alignment mismatch: " << al << " != " << needalign << std::endl;
                }
                if (sz != needsize) {
                    ss << anchor << " type " << ST << " size mismatch: " << sz << " != " << needsize << std::endl;
                }
                #if 0
                set_active_anchor(anchor);
                location_error(String::from("clang-bridge: imported record doesn't fit"));
                #endif
#endif
            }
        }

        tni->finalize(is_union?MixedUnion(args):MixedTuple(args, packed, explicit_alignment?al:0));
    }

    const Type *get_typename(Symbol name, NamespaceMap &map) {
        if (name != SYM_Unnamed) {
            auto it = map.find(name);
            if (it != map.end()) {
                return it->second;
            }
            const Type *T = Typename(name.name());
            auto ok = map.insert({name, T});
            assert(ok.second);
            return T;
        }
        return Typename(name.name());
    }

    const Type *TranslateRecord(clang::RecordDecl *rd) {
        Symbol name = SYM_Unnamed;
        if (rd->isAnonymousStructOrUnion()) {
            auto tdn = rd->getTypedefNameForAnonDecl();
            if (tdn) {
                name = Symbol(String::from_stdstring(tdn->getName().data()));
            }
        } else {
            name = Symbol(String::from_stdstring(rd->getName().data()));
        }

        const Type *struct_type = nullptr;
        if (rd->isUnion()) {
            struct_type = get_typename(name, named_unions);
        } else if (rd->isStruct()) {
            struct_type = get_typename(name, named_structs);
        } else if (rd->isClass()) {
            struct_type = get_typename(name, named_classes);
        } else {
            set_active_anchor(anchorFromLocation(rd->getSourceRange().getBegin()));
            StyledString ss;
            ss.out << "clang-bridge: can't translate record of unuspported type " << name;
            location_error(ss.str());
        }

        clang::RecordDecl * defn = rd->getDefinition();
        if (defn && !record_defined[rd]) {
            record_defined[rd] = true;

            auto tni = cast<TypenameType>(const_cast<Type *>(struct_type));
            if (tni->finalized()) {
                set_active_anchor(anchorFromLocation(rd->getSourceRange().getBegin()));
                StyledString ss;
                ss.out << "clang-bridge: duplicate body defined for type " << struct_type;
                location_error(ss.str());
            }

            GetFields(tni, defn);

            if (name != SYM_Unnamed) {
                Any target = none;
                // don't overwrite names already bound
                if (!dest->lookup(name, target)) {
                    dest->bind(name, struct_type);
                }
            }
        }

        return struct_type;
    }

    Any make_integer(const Type *T, int64_t v) {
        auto it = cast<IntegerType>(T);
        if (it->issigned) {
            switch(it->width) {
            case 8: return Any((int8_t)v);
            case 16: return Any((int16_t)v);
            case 32: return Any((int32_t)v);
            case 64: return Any((int64_t)v);
            default: assert(false); return none;
            }
        } else {
            switch(it->width) {
            case 8: return Any((uint8_t)v);
            case 16: return Any((uint16_t)v);
            case 32: return Any((uint32_t)v);
            case 64: return Any((uint64_t)v);
            default: assert(false); return none;
            }
        }
    }

    const Type *TranslateEnum(clang::EnumDecl *ed) {

        Symbol name(String::from_stdstring(ed->getName()));

        const Type *enum_type = get_typename(name, named_enums);

        //const Anchor *anchor = anchorFromLocation(ed->getIntegerTypeRange().getBegin());

        clang::EnumDecl * defn = ed->getDefinition();
        if (defn && !enum_defined[ed]) {
            enum_defined[ed] = true;

            const Type *tag_type = TranslateType(ed->getIntegerType());

            auto tni = cast<TypenameType>(const_cast<Type *>(enum_type));
            tni->super_type = TYPE_CEnum;
            tni->finalize(tag_type);

            for (auto it : ed->enumerators()) {
                //const Anchor *anchor = anchorFromLocation(it->getSourceRange().getBegin());
                auto &val = it->getInitVal();

                auto name = Symbol(String::from_stdstring(it->getName().data()));
                auto value = make_integer(tag_type, val.getExtValue());
                value.type = enum_type;

                tni->bind(name, value);
                dest->bind(name, value);
            }
        }

        return enum_type;
    }

    bool always_immutable(clang::QualType T) {
        using namespace clang;
        const clang::Type *Ty = T.getTypePtr();
        assert(Ty);
        switch (Ty->getTypeClass()) {
        case clang::Type::Elaborated: {
            const ElaboratedType *et = dyn_cast<ElaboratedType>(Ty);
            return always_immutable(et->getNamedType());
        } break;
        case clang::Type::Paren: {
            const ParenType *pt = dyn_cast<ParenType>(Ty);
            return always_immutable(pt->getInnerType());
        } break;
        case clang::Type::Typedef:
        case clang::Type::Record:
        case clang::Type::Enum:
            break;
        case clang::Type::Builtin:
            switch (cast<BuiltinType>(Ty)->getKind()) {
            case clang::BuiltinType::Void:
            case clang::BuiltinType::Bool:
            case clang::BuiltinType::Char_S:
            case clang::BuiltinType::SChar:
            case clang::BuiltinType::Char_U:
            case clang::BuiltinType::UChar:
            case clang::BuiltinType::Short:
            case clang::BuiltinType::UShort:
            case clang::BuiltinType::Int:
            case clang::BuiltinType::UInt:
            case clang::BuiltinType::Long:
            case clang::BuiltinType::ULong:
            case clang::BuiltinType::LongLong:
            case clang::BuiltinType::ULongLong:
            case clang::BuiltinType::WChar_S:
            case clang::BuiltinType::WChar_U:
            case clang::BuiltinType::Char16:
            case clang::BuiltinType::Char32:
            case clang::BuiltinType::Half:
            case clang::BuiltinType::Float:
            case clang::BuiltinType::Double:
            case clang::BuiltinType::LongDouble:
            case clang::BuiltinType::NullPtr:
            case clang::BuiltinType::UInt128:
            default:
                break;
            }
        case clang::Type::Complex:
        case clang::Type::LValueReference:
        case clang::Type::RValueReference:
            break;
        case clang::Type::Decayed: {
            const clang::DecayedType *DTy = cast<clang::DecayedType>(Ty);
            return always_immutable(DTy->getDecayedType());
        } break;
        case clang::Type::Pointer:
        case clang::Type::VariableArray:
        case clang::Type::IncompleteArray:
        case clang::Type::ConstantArray:
            break;
        case clang::Type::ExtVector:
        case clang::Type::Vector: return true;
        case clang::Type::FunctionNoProto:
        case clang::Type::FunctionProto: return true;
        case clang::Type::ObjCObject: break;
        case clang::Type::ObjCInterface: break;
        case clang::Type::ObjCObjectPointer: break;
        case clang::Type::BlockPointer:
        case clang::Type::MemberPointer:
        case clang::Type::Atomic:
        default:
            break;
        }
        if (T.isLocalConstQualified())
            return true;
        return false;
    }

    uint64_t PointerFlags(clang::QualType T) {
        uint64_t flags = 0;
        if (always_immutable(T))
            flags |= PTF_NonWritable;
        return flags;
    }

    // generate a storage type that matches alignment and size of the
    // original type; used for types that we can't translate
    const Type *TranslateStorage(clang::QualType T) {
        // retype as a tuple of aligned integer and padding byte array
        size_t sz = Context->getTypeSize(T);
        size_t al = Context->getTypeAlign(T);
        assert (sz % 8 == 0);
        assert (al % 8 == 0);
        sz = (sz + 7) / 8;
        al = (al + 7) / 8;
        assert (sz > al);
        ArgTypes fields;
        const Type *TB = Integer(al * 8, false);
        fields.push_back(TB);
        size_t pad = sz - al;
        if (pad)
            fields.push_back(Array(TYPE_U8, pad));
        return Tuple(fields);
    }

    const Type *TranslateType(clang::QualType T) {
        using namespace clang;

        const clang::Type *Ty = T.getTypePtr();
        assert(Ty);

        switch (Ty->getTypeClass()) {
        case clang::Type::Attributed: {
            const AttributedType *at = dyn_cast<AttributedType>(Ty);
            // we probably want to eventually handle some of the attributes
            // but for now, ignore any attribute
            return TranslateType(at->getEquivalentType());
        } break;
        case clang::Type::Elaborated: {
            const ElaboratedType *et = dyn_cast<ElaboratedType>(Ty);
            return TranslateType(et->getNamedType());
        } break;
        case clang::Type::Paren: {
            const ParenType *pt = dyn_cast<ParenType>(Ty);
            return TranslateType(pt->getInnerType());
        } break;
        case clang::Type::Typedef: {
            const TypedefType *tt = dyn_cast<TypedefType>(Ty);
            TypedefNameDecl * td = tt->getDecl();
            auto it = typedefs.find(
                Symbol(String::from_stdstring(td->getName().data())));
            if (it == typedefs.end()) {
                return TYPE_Void;
            }
            return it->second;
        } break;
        case clang::Type::Record: {
            const RecordType *RT = dyn_cast<RecordType>(Ty);
            RecordDecl * rd = RT->getDecl();
            return TranslateRecord(rd);
        }  break;
        case clang::Type::Enum: {
            const clang::EnumType *ET = dyn_cast<clang::EnumType>(Ty);
            EnumDecl * ed = ET->getDecl();
            return TranslateEnum(ed);
        } break;
        case clang::Type::SubstTemplateTypeParm: {
            return TranslateType(T.getCanonicalType());
        } break;
        case clang::Type::TemplateSpecialization: {
            return TranslateStorage(T);
        } break;
        case clang::Type::Builtin:
            switch (cast<BuiltinType>(Ty)->getKind()) {
            case clang::BuiltinType::Void:
                return TYPE_Void;
            case clang::BuiltinType::Bool:
                return TYPE_Bool;
            case clang::BuiltinType::Char_S:
            case clang::BuiltinType::SChar:
            case clang::BuiltinType::Char_U:
            case clang::BuiltinType::UChar:
            case clang::BuiltinType::Short:
            case clang::BuiltinType::UShort:
            case clang::BuiltinType::Int:
            case clang::BuiltinType::UInt:
            case clang::BuiltinType::Long:
            case clang::BuiltinType::ULong:
            case clang::BuiltinType::LongLong:
            case clang::BuiltinType::ULongLong:
            case clang::BuiltinType::WChar_S:
            case clang::BuiltinType::WChar_U:
            case clang::BuiltinType::Char16:
            case clang::BuiltinType::Char32: {
                int sz = Context->getTypeSize(T);
                return Integer(sz, !Ty->isUnsignedIntegerType());
            } break;
            case clang::BuiltinType::Half: return TYPE_F16;
            case clang::BuiltinType::Float:
                return TYPE_F32;
            case clang::BuiltinType::Double:
                return TYPE_F64;
            case clang::BuiltinType::LongDouble: return TYPE_F80;
            case clang::BuiltinType::NullPtr:
            case clang::BuiltinType::UInt128:
            default:
                break;
            }
        case clang::Type::Complex:
        case clang::Type::LValueReference: {
            const clang::LValueReferenceType *PTy =
                cast<clang::LValueReferenceType>(Ty);
            QualType ETy = PTy->getPointeeType();
            return Pointer(TranslateType(ETy), PointerFlags(ETy), SYM_Unnamed);
        } break;
        case clang::Type::RValueReference:
            break;
        case clang::Type::Decayed: {
            const clang::DecayedType *DTy = cast<clang::DecayedType>(Ty);
            return TranslateType(DTy->getDecayedType());
        } break;
        case clang::Type::Pointer: {
            const clang::PointerType *PTy = cast<clang::PointerType>(Ty);
            QualType ETy = PTy->getPointeeType();
            return Pointer(TranslateType(ETy), PointerFlags(ETy), SYM_Unnamed);
        } break;
        case clang::Type::VariableArray:
            break;
        case clang::Type::IncompleteArray: {
            const IncompleteArrayType *ATy = cast<IncompleteArrayType>(Ty);
            QualType ETy = ATy->getElementType();
            return Pointer(TranslateType(ETy), PointerFlags(ETy), SYM_Unnamed);
        } break;
        case clang::Type::ConstantArray: {
            const ConstantArrayType *ATy = cast<ConstantArrayType>(Ty);
            const Type *at = TranslateType(ATy->getElementType());
            uint64_t sz = ATy->getSize().getZExtValue();
            return Array(at, sz);
        } break;
        case clang::Type::ExtVector:
        case clang::Type::Vector: {
            const clang::VectorType *VT = cast<clang::VectorType>(T);
            const Type *at = TranslateType(VT->getElementType());
            uint64_t n = VT->getNumElements();
            return Vector(at, n);
        } break;
        case clang::Type::FunctionNoProto:
        case clang::Type::FunctionProto: {
            const clang::FunctionType *FT = cast<clang::FunctionType>(Ty);
            return TranslateFuncType(FT);
        } break;
        case clang::Type::ObjCObject: break;
        case clang::Type::ObjCInterface: break;
        case clang::Type::ObjCObjectPointer: break;
        case clang::Type::BlockPointer:
        case clang::Type::MemberPointer:
        case clang::Type::Atomic:
        default:
            break;
        }
        location_error(format("clang-bridge: cannot convert type: %s (%s)",
            T.getAsString().c_str(),
            Ty->getTypeClassName()));
        return TYPE_Void;
    }

    const Type *TranslateFuncType(const clang::FunctionType * f) {

        clang::QualType RT = f->getReturnType();

        const Type *returntype = TranslateType(RT);

        uint64_t flags = 0;

        ArgTypes argtypes;

        const clang::FunctionProtoType * proto = f->getAs<clang::FunctionProtoType>();
        if(proto) {
            if (proto->isVariadic()) {
                flags |= FF_Variadic;
            }
            for(size_t i = 0; i < proto->getNumParams(); i++) {
                clang::QualType PT = proto->getParamType(i);
                argtypes.push_back(TranslateType(PT));
            }
        }

        return Function(returntype, argtypes, flags);
    }

    void exportType(Symbol name, const Type *type) {
        dest->bind(name, type);
    }

    void exportExtern(Symbol name, const Type *type,
        const Anchor *anchor) {
        Any value(name);
        value.type = Extern(type);
        dest->bind(name, value);
    }

    bool TraverseRecordDecl(clang::RecordDecl *rd) {
        if (rd->isFreeStanding()) {
            TranslateRecord(rd);
        }
        return true;
    }

    bool TraverseEnumDecl(clang::EnumDecl *ed) {
        if (ed->isFreeStanding()) {
            TranslateEnum(ed);
        }
        return true;
    }

    bool TraverseVarDecl(clang::VarDecl *vd) {
        if (vd->isExternC()) {
            const Anchor *anchor = anchorFromLocation(vd->getSourceRange().getBegin());

            exportExtern(
                String::from_stdstring(vd->getName().data()),
                TranslateType(vd->getType()),
                anchor);
        }

        return true;
    }

    bool TraverseTypedefDecl(clang::TypedefDecl *td) {

        //const Anchor *anchor = anchorFromLocation(td->getSourceRange().getBegin());

        const Type *type = TranslateType(td->getUnderlyingType());

        Symbol name = Symbol(String::from_stdstring(td->getName().data()));

        typedefs.insert({name, type});
        exportType(name, type);

        return true;
    }

    bool TraverseLinkageSpecDecl(clang::LinkageSpecDecl *ct) {
        if (ct->getLanguage() == clang::LinkageSpecDecl::lang_c) {
            return clang::RecursiveASTVisitor<CVisitor>::TraverseLinkageSpecDecl(ct);
        }
        return false;
    }

    bool TraverseClassTemplateDecl(clang::ClassTemplateDecl *ct) {
        return false;
    }

    bool TraverseFunctionDecl(clang::FunctionDecl *f) {
        clang::DeclarationName DeclName = f->getNameInfo().getName();
        std::string FuncName = DeclName.getAsString();
        const clang::FunctionType * fntyp = f->getType()->getAs<clang::FunctionType>();

        if(!fntyp)
            return true;

        if(f->getStorageClass() == clang::SC_Static) {
            return true;
        }

        const Type *functype = TranslateFuncType(fntyp);

        std::string InternalName = FuncName;
        clang::AsmLabelAttr * asmlabel = f->getAttr<clang::AsmLabelAttr>();
        if(asmlabel) {
            InternalName = asmlabel->getLabel();
            #ifndef __linux__
                //In OSX and Windows LLVM mangles assembler labels by adding a '\01' prefix
                InternalName.insert(InternalName.begin(), '\01');
            #endif
        }
        const Anchor *anchor = anchorFromLocation(f->getSourceRange().getBegin());

        exportExtern(Symbol(String::from_stdstring(FuncName)),
            functype, anchor);

        return true;
    }

};

class CodeGenProxy : public clang::ASTConsumer {
public:
    Scope *dest;

    CVisitor visitor;

    CodeGenProxy(Scope *dest_) : dest(dest_) {}
    virtual ~CodeGenProxy() {}

    virtual void Initialize(clang::ASTContext &Context) {
        visitor.SetContext(&Context, dest);
    }

    virtual bool HandleTopLevelDecl(clang::DeclGroupRef D) {
        for (clang::DeclGroupRef::iterator b = D.begin(), e = D.end(); b != e; ++b)
            visitor.TraverseDecl(*b);
        return true;
    }
};

// see ASTConsumers.h for more utilities
class BangEmitLLVMOnlyAction : public clang::EmitLLVMOnlyAction {
public:
    Scope *dest;

    BangEmitLLVMOnlyAction(Scope *dest_) :
        EmitLLVMOnlyAction((llvm::LLVMContext *)LLVMGetGlobalContext()),
        dest(dest_)
    {
    }

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI,
                                                 clang::StringRef InFile) override {

        std::vector< std::unique_ptr<clang::ASTConsumer> > consumers;
        consumers.push_back(clang::EmitLLVMOnlyAction::CreateASTConsumer(CI, InFile));
        consumers.push_back(llvm::make_unique<CodeGenProxy>(dest));
        return llvm::make_unique<clang::MultiplexConsumer>(std::move(consumers));
    }
};

static LLVMExecutionEngineRef ee = nullptr;

static void init_llvm() {
    LLVMEnablePrettyStackTrace();
    LLVMLinkInMCJIT();
    //LLVMLinkInInterpreter();
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmParser();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeDisassembler();
}

static std::vector<LLVMModuleRef> llvm_c_modules;

static void add_c_macro(clang::Preprocessor & PP,
    const clang::IdentifierInfo * II,
    clang::MacroDirective * MD, Scope *scope, std::list< std::pair<Symbol, Symbol> > &aliases) {
    if(!II->hasMacroDefinition())
        return;
    clang::MacroInfo * MI = MD->getMacroInfo();
    if(MI->isFunctionLike())
        return;
    bool negate = false;
    const clang::Token * Tok;
    auto numtokens = MI->getNumTokens();
    if(numtokens == 2 && MI->getReplacementToken(0).is(clang::tok::minus)) {
        negate = true;
        Tok = &MI->getReplacementToken(1);
    } else if(numtokens == 1) {
        Tok = &MI->getReplacementToken(0);
    } else {
        return;
    }

    if ((numtokens == 1) && Tok->is(clang::tok::identifier)) {
        // aliases need to be resolved once the whole namespace is known
        const String *name = String::from_cstr(II->getName().str().c_str());
        const String *value = String::from_cstr(Tok->getIdentifierInfo()->getName().str().c_str());
        aliases.push_back({ Symbol(name), Symbol(value) });
        return;
    }

    if ((numtokens == 1) && Tok->is(clang::tok::string_literal)) {
        clang::Token tokens[] = { *Tok };
        clang::StringLiteralParser Literal(tokens, PP, false);
        const String *name = String::from_cstr(II->getName().str().c_str());
        std::string svalue = Literal.GetString();
        const String *value = String::from(svalue.c_str(), svalue.size());
        scope->bind(Symbol(name), value);
        return;
    }

    if(Tok->isNot(clang::tok::numeric_constant))
        return;

    clang::SmallString<64> IntegerBuffer;
    bool NumberInvalid = false;
    clang::StringRef Spelling = PP.getSpelling(*Tok, IntegerBuffer, &NumberInvalid);
    clang::NumericLiteralParser Literal(Spelling, Tok->getLocation(), PP);
    if(Literal.hadError)
        return;
    const String *name = String::from_cstr(II->getName().str().c_str());
    std::string suffix;
    if (Literal.hasUDSuffix()) {
        suffix = Literal.getUDSuffix();
        std::cout << "TODO: macro literal suffix: " << suffix << std::endl;
    }
    if(Literal.isFloatingLiteral()) {
        llvm::APFloat Result(0.0);
        Literal.GetFloatValue(Result);
        double V = Result.convertToDouble();
        if (negate)
            V = -V;
        scope->bind(Symbol(name), V);
    } else {
        llvm::APInt Result(64,0);
        Literal.GetIntegerValue(Result);
        int64_t i = Result.getSExtValue();
        if (negate)
            i = -i;
        scope->bind(Symbol(name), i);
    }
}

static Scope *import_c_module (
    const std::string &path, const std::vector<std::string> &args,
    const char *buffer = nullptr) {
    using namespace clang;

    std::vector<const char *> aargs;
    aargs.push_back("clang");
    aargs.push_back(path.c_str());
    aargs.push_back("-I");
    aargs.push_back(scopes_clang_include_dir);
    aargs.push_back("-I");
    aargs.push_back(scopes_include_dir);
    for (size_t i = 0; i < args.size(); ++i) {
        aargs.push_back(args[i].c_str());
    }

    CompilerInstance compiler;
    compiler.setInvocation(createInvocationFromCommandLine(aargs));

    if (buffer) {
        auto &opts = compiler.getPreprocessorOpts();

        llvm::MemoryBuffer * membuffer =
            llvm::MemoryBuffer::getMemBuffer(buffer, "<buffer>").release();

        opts.addRemappedFile(path, membuffer);
    }

    // Create the compilers actual diagnostics engine.
    compiler.createDiagnostics();

    // Infer the builtin include path if unspecified.
    //~ if (compiler.getHeaderSearchOpts().UseBuiltinIncludes &&
        //~ compiler.getHeaderSearchOpts().ResourceDir.empty())
        //~ compiler.getHeaderSearchOpts().ResourceDir =
            //~ CompilerInvocation::GetResourcesPath(scopes_argv[0], MainAddr);

    LLVMModuleRef M = NULL;


    Scope *result = Scope::from();

    // Create and execute the frontend to generate an LLVM bitcode module.
    std::unique_ptr<CodeGenAction> Act(new BangEmitLLVMOnlyAction(result));
    if (compiler.ExecuteAction(*Act)) {

        clang::Preprocessor & PP = compiler.getPreprocessor();
        PP.getDiagnostics().setClient(new IgnoringDiagConsumer(), true);

        std::list< std::pair<Symbol, Symbol> > todo;
        for(Preprocessor::macro_iterator it = PP.macro_begin(false),end = PP.macro_end(false);
            it != end; ++it) {
            const IdentifierInfo * II = it->first;
            MacroDirective * MD = it->second.getLatest();

            add_c_macro(PP, II, MD, result, todo);
        }

        while (!todo.empty()) {
            auto sz = todo.size();
            for (auto it = todo.begin(); it != todo.end();) {
                Any value = none;
                if (result->lookup(it->second, value)) {
                    result->bind(it->first, value);
                    auto oldit = it++;
                    todo.erase(oldit);
                } else {
                    it++;
                }
            }
            // couldn't resolve any more keys, abort
            if (todo.size() == sz) break;
        }

        M = (LLVMModuleRef)Act->takeModule().release();
        assert(M);
        llvm_c_modules.push_back(M);
        assert(ee);
        LLVMAddModule(ee, M);
        return result;
    } else {
        location_error(String::from("compilation failed"));
    }

    return nullptr;
}

//------------------------------------------------------------------------------
// INTERPRETER
//------------------------------------------------------------------------------

static void apply_type_error(const Any &enter) {
    StyledString ss;
    ss.out << "don't know how to apply value of type " << enter.type;
    location_error(ss.str());
}

template<int mincount, int maxcount>
inline int checkargs(size_t argsize, bool allow_overshoot = false) {
    int count = (int)argsize - 1;
    if ((mincount <= 0) && (maxcount == -1)) {
        return count;
    }

    if ((maxcount >= 0) && (count > maxcount)) {
        if (allow_overshoot) {
            count = maxcount;
        } else {
            location_error(
                format("at most %i argument(s) expected, got %i", maxcount, count));
        }
    }
    if ((mincount >= 0) && (count < mincount)) {
        location_error(
            format("at least %i argument(s) expected, got %i", mincount, count));
    }
    return count;
}

static void *global_c_namespace = nullptr;

static void on_shutdown();

static bool signal_abort = false;
void f_abort() {
    on_shutdown();
    if (SCOPES_EARLY_ABORT || signal_abort) {
        std::abort();
    } else {
        exit(1);
    }
}


void f_exit(int c) {
    on_shutdown();
    exit(c);
}

void location_message(const Anchor *anchor, const String* str) {
    assert(anchor);
    auto cerr = StyledStream(std::cerr);
    cerr << anchor << str->data << std::endl;
    anchor->stream_source_line(cerr);
}

static void print_exception(const Any &value) {
    auto cerr = StyledStream(std::cerr);
    if (value.type == TYPE_Exception) {
        const Exception *exc = value;
        if (exc->anchor) {
            cerr << exc->anchor << " ";
        }
        cerr << Style_Error << "error:" << Style_None << " "
            << exc->msg->data << std::endl;
        if (exc->anchor) {
            exc->anchor->stream_source_line(cerr);
        }
    } else {
        cerr << "exception raised: " << value << std::endl;
    }
}

static void default_exception_handler(const Any &value) {
    print_exception(value);
    f_abort();
}

static int integer_type_bit_size(const Type *T) {
    return (int)cast<IntegerType>(T)->width;
}

template<typename T>
static T cast_number(const Any &value) {
    auto ST = storage_type(value.type);
    auto it = dyn_cast<IntegerType>(ST);
    if (it) {
        if (it->issigned) {
            switch(it->width) {
            case 8: return (T)value.i8;
            case 16: return (T)value.i16;
            case 32: return (T)value.i32;
            case 64: return (T)value.i64;
            default: break;
            }
        } else {
            switch(it->width) {
            case 1: return (T)value.i1;
            case 8: return (T)value.u8;
            case 16: return (T)value.u16;
            case 32: return (T)value.u32;
            case 64: return (T)value.u64;
            default: break;
            }
        }
    }
    auto ft = dyn_cast<RealType>(ST);
    if (ft) {
        switch(ft->width) {
        case 32: return (T)value.f32;
        case 64: return (T)value.f64;
        default: break;
        }
    }
    StyledString ss;
    ss.out << "can not extract constant from ";
    if (value.is_const()) {
        ss.out << "value of type " << value.type;
    } else {
        ss.out << "variable of type " << value.indirect_type();
    }
    location_error(ss.str());
    return 0;
}

//------------------------------------------------------------------------------
// PLATFORM ABI
//------------------------------------------------------------------------------

// life is unfair, which is why we need to implement the remaining platform ABI
// support in the front-end, particularly whether an argument is passed by
// value or not.

// based on x86-64 PS ABI (SystemV)
#define DEF_ABI_CLASS_NAMES \
    /* This class consists of integral types that fit into one of the general */ \
    /* purpose registers. */ \
    T(INTEGER) \
    T(INTEGERSI) \
    /* special types for windows, not used anywhere else */ \
    T(INTEGERSI16) \
    T(INTEGERSI8) \
    /* The class consists of types that fit into a vector register. */ \
    T(SSE) \
    T(SSESF) \
    T(SSEDF) \
    /* The class consists of types that fit into a vector register and can be */ \
    /* passed and returned in the upper bytes of it. */ \
    T(SSEUP) \
    /* These classes consists of types that will be returned via the x87 FPU */ \
    T(X87) \
    T(X87UP) \
    /* This class consists of types that will be returned via the x87 FPU */ \
    T(COMPLEX_X87) \
    /* This class is used as initializer in the algorithms. It will be used for */ \
    /* padding and empty structures and unions. */ \
    T(NO_CLASS) \
    /* This class consists of types that will be passed and returned in memory */ \
    /* via the stack. */ \
    T(MEMORY)

enum ABIClass {
#define T(X) ABI_CLASS_ ## X,
    DEF_ABI_CLASS_NAMES
#undef T
};

static const char *abi_class_to_string(ABIClass class_) {
    switch(class_) {
    #define T(X) case ABI_CLASS_ ## X: return #X;
    DEF_ABI_CLASS_NAMES
    #undef T
    default: return "?";
    }
}

#undef DEF_ABI_CLASS_NAMES

const size_t MAX_ABI_CLASSES = 4;

#ifdef SCOPES_WIN32
#else
// x86-64 PS ABI based on https://www.uclibc.org/docs/psABI-x86_64.pdf

static ABIClass merge_abi_classes(ABIClass class1, ABIClass class2) {
    if (class1 == class2)
        return class1;

    if (class1 == ABI_CLASS_NO_CLASS)
        return class2;
    if (class2 == ABI_CLASS_NO_CLASS)
        return class1;

    if (class1 == ABI_CLASS_MEMORY || class2 == ABI_CLASS_MEMORY)
        return ABI_CLASS_MEMORY;

    if ((class1 == ABI_CLASS_INTEGERSI && class2 == ABI_CLASS_SSESF)
        || (class2 == ABI_CLASS_INTEGERSI && class1 == ABI_CLASS_SSESF))
        return ABI_CLASS_INTEGERSI;
    if (class1 == ABI_CLASS_INTEGER || class1 == ABI_CLASS_INTEGERSI
        || class2 == ABI_CLASS_INTEGER || class2 == ABI_CLASS_INTEGERSI)
        return ABI_CLASS_INTEGER;

    if (class1 == ABI_CLASS_X87
        || class1 == ABI_CLASS_X87UP
        || class1 == ABI_CLASS_COMPLEX_X87
        || class2 == ABI_CLASS_X87
        || class2 == ABI_CLASS_X87UP
        || class2 == ABI_CLASS_COMPLEX_X87)
        return ABI_CLASS_MEMORY;

    return ABI_CLASS_SSE;
}

static size_t classify(const Type *T, ABIClass *classes, size_t offset);

static size_t classify_array_like(size_t size,
    const Type *element_type, size_t count,
    ABIClass *classes, size_t offset) {
    const size_t UNITS_PER_WORD = 8;
    size_t words = (size + UNITS_PER_WORD - 1) / UNITS_PER_WORD;
    if (size > 32)
        return 0;
    for (size_t i = 0; i < MAX_ABI_CLASSES; i++)
        classes[i] = ABI_CLASS_NO_CLASS;
    if (!words) {
        classes[0] = ABI_CLASS_NO_CLASS;
        return 1;
    }
    auto ET = element_type;
    ABIClass subclasses[MAX_ABI_CLASSES];
    size_t alignment = align_of(ET);
    size_t esize = size_of(ET);
    for (size_t i = 0; i < count; ++i) {
        offset = align(offset, alignment);
        size_t num = classify(ET, subclasses, offset % 8);
        if (!num) return 0;
        for (size_t k = 0; k < num; ++k) {
            size_t pos = offset / 8;
            classes[k + pos] =
                merge_abi_classes (subclasses[k], classes[k + pos]);
        }
        offset += esize;
    }
    if (words > 2) {
        if (classes[0] != ABI_CLASS_SSE)
            return 0;
        for (size_t i = 1; i < words; ++i) {
            if (classes[i] != ABI_CLASS_SSEUP)
                return 0;
        }
    }
    for (size_t i = 0; i < words; i++) {
        if (classes[i] == ABI_CLASS_MEMORY)
            return 0;

        if (classes[i] == ABI_CLASS_SSEUP) {
            assert(i > 0);
            if (classes[i - 1] != ABI_CLASS_SSE
                && classes[i - 1] != ABI_CLASS_SSEUP) {
                classes[i] = ABI_CLASS_SSE;
            }
        }

        if (classes[i] == ABI_CLASS_X87UP) {
            assert(i > 0);
            if(classes[i - 1] != ABI_CLASS_X87) {
                return 0;
            }
        }
    }
    return words;
}

static size_t classify(const Type *T, ABIClass *classes, size_t offset) {
    switch(T->kind()) {
    case TK_Integer:
    case TK_Extern:
    case TK_Pointer: {
        size_t size = size_of(T) + offset;
        if (size <= 4) {
            classes[0] = ABI_CLASS_INTEGERSI;
            return 1;
        } else if (size <= 8) {
            classes[0] = ABI_CLASS_INTEGER;
            return 1;
        } else if (size <= 12) {
            classes[0] = ABI_CLASS_INTEGER;
            classes[1] = ABI_CLASS_INTEGERSI;
            return 2;
        } else if (size <= 16) {
            classes[0] = ABI_CLASS_INTEGER;
            classes[1] = ABI_CLASS_INTEGER;
            return 2;
        } else {
            assert(false && "illegal type");
        }
    } break;
    case TK_Real: {
        size_t size = size_of(T);
        if (size == 4) {
            if (!(offset % 8))
                classes[0] = ABI_CLASS_SSESF;
            else
                classes[0] = ABI_CLASS_SSE;
            return 1;
        } else if (size == 8) {
            classes[0] = ABI_CLASS_SSEDF;
            return 1;
        } else {
            assert(false && "illegal type");
        }
    } break;
    case TK_ReturnLabel:
    case TK_Typename: {
        if (is_opaque(T)) {
            classes[0] = ABI_CLASS_NO_CLASS;
            return 1;
        } else {
            return classify(storage_type(T), classes, offset);
        }
    } break;
    case TK_Vector: {
        auto tt = cast<VectorType>(T);
        return classify_array_like(size_of(T),
            tt->element_type, tt->count, classes, offset);
    } break;
    case TK_Array: {
        auto tt = cast<ArrayType>(T);
        return classify_array_like(size_of(T),
            tt->element_type, tt->count, classes, offset);
    } break;
    case TK_Union: {
        auto ut = cast<UnionType>(T);
        return classify(ut->types[ut->largest_field], classes, offset);
    } break;
    case TK_Tuple: {
        const size_t UNITS_PER_WORD = 8;
        size_t size = size_of(T);
	    size_t words = (size + UNITS_PER_WORD - 1) / UNITS_PER_WORD;
        if (size > 32)
            return 0;
        for (size_t i = 0; i < MAX_ABI_CLASSES; i++)
	        classes[i] = ABI_CLASS_NO_CLASS;
        if (!words) {
            classes[0] = ABI_CLASS_NO_CLASS;
            return 1;
        }
        auto tt = cast<TupleType>(T);
        ABIClass subclasses[MAX_ABI_CLASSES];
        for (size_t i = 0; i < tt->types.size(); ++i) {
            auto ET = tt->types[i];
            if (!tt->packed)
                offset = align(offset, align_of(ET));
            size_t num = classify (ET, subclasses, offset % 8);
            if (!num) return 0;
            for (size_t k = 0; k < num; ++k) {
                size_t pos = offset / 8;
		        classes[k + pos] =
		            merge_abi_classes (subclasses[k], classes[k + pos]);
            }
            offset += size_of(ET);
        }
        if (words > 2) {
            if (classes[0] != ABI_CLASS_SSE)
                return 0;
            for (size_t i = 1; i < words; ++i) {
                if (classes[i] != ABI_CLASS_SSEUP)
                    return 0;
            }
        }
        for (size_t i = 0; i < words; i++) {
            if (classes[i] == ABI_CLASS_MEMORY)
                return 0;

            if (classes[i] == ABI_CLASS_SSEUP) {
                assert(i > 0);
                if (classes[i - 1] != ABI_CLASS_SSE
                    && classes[i - 1] != ABI_CLASS_SSEUP) {
                    classes[i] = ABI_CLASS_SSE;
                }
            }

            if (classes[i] == ABI_CLASS_X87UP) {
                assert(i > 0);
                if(classes[i - 1] != ABI_CLASS_X87) {
                    return 0;
                }
            }
        }
        return words;
    } break;
    default: {
        assert(false && "not supported in ABI");
        return 0;
    } break;
    }
    return 0;
}
#endif // SCOPES_WIN32

static size_t abi_classify(const Type *T, ABIClass *classes) {
#ifdef SCOPES_WIN32
    if (T->kind() == TK_ReturnLabel) {
        T = cast<ReturnLabelType>(T)->return_type;
    }
    classes[0] = ABI_CLASS_NO_CLASS;
    if (is_opaque(T))
        return 1;
    T = storage_type(T);
    size_t sz = size_of(T);
    if (sz > 8)
        return 0;
    switch(T->kind()) {
    case TK_Array:
    case TK_Union:
    case TK_Tuple:
        if (sz <= 1)
            classes[0] = ABI_CLASS_INTEGERSI8;
        else if (sz <= 2)
            classes[0] = ABI_CLASS_INTEGERSI16;
        else if (sz <= 4)
            classes[0] = ABI_CLASS_INTEGERSI;
        else
            classes[0] = ABI_CLASS_INTEGER;
        return 1;
    case TK_Integer:
    case TK_Extern:
    case TK_Pointer:
    case TK_Real:
    case TK_ReturnLabel:
    case TK_Typename:
    case TK_Vector:
    default:
        return 1;
    }
#else
    size_t sz = classify(T, classes, 0);
#if 0
    if (sz) {
        StyledStream ss(std::cout);
        ss << T << " -> " << sz;
        for (int i = 0; i < sz; ++i) {
            ss << " " << abi_class_to_string(classes[i]);
        }
        ss << std::endl;
    }
#endif
    return sz;
#endif
}

static bool is_memory_class(const Type *T) {
    ABIClass classes[MAX_ABI_CLASSES];
    return !abi_classify(T, classes);
}

//------------------------------------------------------------------------------
// SCC
//------------------------------------------------------------------------------

// build strongly connected component map of label graph
// uses Dijkstra's Path-based strong component algorithm
struct SCCBuilder {
    struct Group {
        size_t index;
        Labels labels;
    };

    Labels S;
    Labels P;
    std::unordered_map<Label *, size_t> Cmap;
    std::vector<Group> groups;
    std::unordered_map<Label *, size_t> SCCmap;
    size_t C;

    SCCBuilder() : C(0) {}

    SCCBuilder(Label *top) :
        C(0) {
        walk(top);
    }

    void stream_group(StyledStream &ss, const Group &group) {
        ss << "group #" << group.index << " (" << group.labels.size() << " labels):" << std::endl;
        for (size_t k = 0; k < group.labels.size(); ++k) {
            stream_label(ss, group.labels[k], StreamLabelFormat::single());
        }
    }

    bool is_recursive(Label *l) {
        return group(l).labels.size() > 1;
    }

    bool contains(Label *l) {
        auto it = SCCmap.find(l);
        return it != SCCmap.end();
    }

    size_t group_id(Label *l) {
        auto it = SCCmap.find(l);
        assert(it != SCCmap.end());
        return it->second;
    }

    Group &group(Label *l) {
        return groups[group_id(l)];
    }

    void walk(Label *obj) {
        Cmap[obj] = C++;
        S.push_back(obj);
        P.push_back(obj);

        int size = (int)obj->body.args.size();
        for (int i = -1; i < size; ++i) {
            Any arg = none;
            if (i == -1) {
                arg = obj->body.enter;
            } else {
                arg = obj->body.args[i].value;
            }

            if (arg.type == TYPE_Label) {
                Label *label = arg.label;

                auto it = Cmap.find(label);
                if (it == Cmap.end()) {
                    walk(label);
                } else if (!SCCmap.count(label)) {
                    size_t Cw = it->second;
                    while (true) {
                        assert(!P.empty());
                        auto it = Cmap.find(P.back());
                        assert(it != Cmap.end());
                        if (it->second <= Cw) break;
                        P.pop_back();
                    }
                }
            }
        }

        assert(!P.empty());
        if (P.back() == obj) {
            groups.emplace_back();
            Group &scc = groups.back();
            scc.index = groups.size() - 1;
            while (true) {
                assert(!S.empty());
                Label *q = S.back();
                scc.labels.push_back(q);
                SCCmap[q] = groups.size() - 1;
                S.pop_back();
                if (q == obj) {
                    break;
                }
            }
            P.pop_back();
        }
    }
};

//------------------------------------------------------------------------------
// IL->SPIR-V GENERATOR
//------------------------------------------------------------------------------

static void disassemble_spirv(std::vector<unsigned int> &contents) {
    spv_context context = spvContextCreate(SPV_ENV_UNIVERSAL_1_2);
    uint32_t options = SPV_BINARY_TO_TEXT_OPTION_PRINT;
    options |= SPV_BINARY_TO_TEXT_OPTION_INDENT;
    options |= SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES;
    //options |= SPV_BINARY_TO_TEXT_OPTION_SHOW_BYTE_OFFSET;
    if (stream_default_style == stream_ansi_style) {
        options |= SPV_BINARY_TO_TEXT_OPTION_COLOR;
    }
    spv_diagnostic diagnostic = nullptr;
    spv_result_t error =
        spvBinaryToText(context, contents.data(), contents.size(), options,
                        nullptr, &diagnostic);
    spvContextDestroy(context);
    if (error) {
        spvDiagnosticPrint(diagnostic);
        spvDiagnosticDestroy(diagnostic);
        std::cerr << "error while pretty-printing disassembly, falling back to"
            " failsafe disassembly" << std::endl;
        spv::Disassemble(std::cerr, contents);
    }
}

static void format_spv_location(StyledStream &ss, const char* source,
    const spv_position_t& position) {
    ss << Style_Location;
    StyledStream ps = StyledStream::plain(ss);
    if (source) {
        ps << source << ":";
    }
    ps << position.line << ":" << position.column;
    ss << ":" << Style_None << " ";
}

static void verify_spirv(std::vector<unsigned int> &contents) {
    spv_target_env target_env = SPV_ENV_UNIVERSAL_1_2;
    //spvtools::ValidatorOptions options;

    StyledString ss;
    spvtools::SpirvTools tools(target_env);
    tools.SetMessageConsumer([&ss](spv_message_level_t level, const char* source,
                                const spv_position_t& position,
                                const char* message) {
        switch (level) {
        case SPV_MSG_FATAL:
        case SPV_MSG_INTERNAL_ERROR:
        case SPV_MSG_ERROR:
            format_spv_location(ss.out, source, position);
            ss.out << Style_Error << "error: " << Style_None
                << message << std::endl;
            break;
        case SPV_MSG_WARNING:
            format_spv_location(ss.out, source, position);
            ss.out << Style_Warning << "warning: " << Style_None
                << message << std::endl;
            break;
        case SPV_MSG_INFO:
            format_spv_location(ss.out, source, position);
            ss.out << Style_Comment << "info: " << Style_None
                << message << std::endl;
            break;
        default:
            break;
        }
    });

    bool succeed = tools.Validate(contents);
    if (!succeed) {
        disassemble_spirv(contents);
        std::cerr << ss._ss.str();
        location_error(String::from("SPIR-V validation found errors"));
    }
}


struct SPIRVGenerator {
    struct HashFuncLabelPair {
        size_t operator ()(const std::pair<spv::Function *, Label *> &value) const {
            return
                HashLen16(std::hash<spv::Function *>()(value.first),
                    std::hash<Label *>()(value.second));
        }
    };

    typedef std::pair<spv::Function *, Parameter *> ParamKey;
    struct HashFuncParamPair {
        size_t operator ()(const ParamKey &value) const {
            return
                HashLen16(std::hash<spv::Function *>()(value.first),
                    std::hash<Parameter *>()(value.second));
        }
    };

    typedef std::pair<const Type *, uint64_t> TypeKey;
    struct HashTypeFlagsPair {
        size_t operator ()(const TypeKey &value) const {
            return
                HashLen16(std::hash<const Type *>{}(value.first),
                    std::hash<uint64_t>{}(value.second));
        }
    };

    spv::SpvBuildLogger logger;
    spv::Builder builder;
    SCCBuilder scc;

    Label *active_function;
    spv::Function *active_function_value;
    spv::Id glsl_ext_inst;

    bool use_debug_info;

    std::unordered_map<Label *, spv::Function *> label2func;
    std::unordered_map< std::pair<spv::Function *, Label *>,
        spv::Block *, HashFuncLabelPair> label2bb;
    std::vector< std::pair<Label *, Label *> > bb_label_todo;

    //std::unordered_map<Label *, LLVMValueRef> label2md;
    //std::unordered_map<SourceFile *, LLVMValueRef> file2value;
    std::unordered_map< ParamKey, spv::Id, HashFuncParamPair> param2value;

    std::unordered_map<std::pair<const Type *, uint64_t>,
        spv::Id, HashTypeFlagsPair> type_cache;

    std::unordered_map<Any, spv::Id, Any::Hash> const_cache;

    Label::UserMap user_map;

    SPIRVGenerator() :
        builder('S' << 24 | 'C' << 16 | 'O' << 8 | 'P', &logger),
        active_function(nullptr),
        active_function_value(nullptr),
        glsl_ext_inst(0),
        use_debug_info(true) {

    }

    static spv::Dim dim_from_symbol(Symbol sym) {
        switch(sym.value()) {
        #define T(NAME) \
            case SYM_SPIRV_Dim ## NAME: return spv::Dim ## NAME;
            B_SPIRV_DIM()
        #undef T
            default:
                location_error(
                    String::from(
                        "IL->SPIR: unsupported dimensionality"));
                break;
        }
        return spv::DimMax;
    }

    spv::ImageFormat image_format_from_symbol(Symbol sym) {
        switch(sym.value()) {
        #define T(NAME) \
            case SYM_SPIRV_ImageFormat ## NAME: return spv::ImageFormat ## NAME;
            B_SPIRV_IMAGE_FORMAT()
        #undef T
            default:
                location_error(
                    String::from(
                        "IL->SPIR: unsupported image format"));
                break;
        }
        return spv::ImageFormatMax;
    }

    static spv::ExecutionMode execution_mode_from_symbol(Symbol sym) {
        switch(sym.value()) {
        #define T(NAME) \
            case SYM_SPIRV_ExecutionMode ## NAME: return spv::ExecutionMode ## NAME;
            B_SPIRV_EXECUTION_MODE()
        #undef T
            default:
                location_error(
                    String::from(
                        "IL->SPIR: unsupported execution mode"));
                break;
        }
        return spv::ExecutionModeMax;
    }

    spv::StorageClass storage_class_from_extern_class(Symbol sym) {
        switch(sym.value()) {
        #define T(NAME) \
            case SYM_SPIRV_StorageClass ## NAME: return spv::StorageClass ## NAME;
            B_SPIRV_STORAGE_CLASS()
        #undef T
            case SYM_Unnamed:
                location_error(
                    String::from(
                        "IL->SPIR: pointers with C storage class"
                        " are unsupported"));
                break;
            default:
                location_error(
                    String::from(
                        "IL->SPIR: unsupported storage class for pointer"));
                break;
        }
        return spv::StorageClassMax;
    }

    bool parameter_is_bound(Parameter *param) {
        return param2value.find({active_function_value, param}) != param2value.end();
    }

    spv::Id argument_to_value(Any value) {
        if (value.type == TYPE_Parameter) {
            auto it = param2value.find({active_function_value, value.parameter});
            if (it == param2value.end()) {
                assert(active_function_value);
                if (value.parameter->label) {
                    location_message(value.parameter->label->anchor, String::from("declared here"));
                }
                StyledString ss;
                ss.out << "IL->SPIR: can't access free variable " << value.parameter;
                location_error(ss.str());
            }
            return it->second;
        }
        if (value.type != TYPE_String) {
            switch(value.type->kind()) {
            case TK_Integer: {
                auto it = cast<IntegerType>(value.type);
                if (it->issigned) {
                    switch(it->width) {
                    case 8: return builder.makeIntConstant(
                        builder.makeIntegerType(8, true), value.i8);
                    case 16: return builder.makeIntConstant(
                        builder.makeIntegerType(16, true), value.i16);
                    case 32: return builder.makeIntConstant(value.i32);
                    case 64: return builder.makeInt64Constant(value.i64);
                    default: break;
                    }
                } else {
                    switch(it->width) {
                    case 1: return builder.makeBoolConstant(value.i1);
                    case 8: return builder.makeIntConstant(
                        builder.makeIntegerType(8, false), value.i8);
                    case 16: return builder.makeIntConstant(
                        builder.makeIntegerType(16, false), value.i16);
                    case 32: return builder.makeUintConstant(value.u32);
                    case 64: return builder.makeUint64Constant(value.u64);
                    default: break;
                    }
                }
                StyledString ss;
                ss.out << "IL->SPIR: unsupported integer constant type " << value.type;
                location_error(ss.str());
            } break;
            case TK_Real: {
                auto rt = cast<RealType>(value.type);
                switch(rt->width) {
                case 32: return builder.makeFloatConstant(value.f32);
                case 64: return builder.makeDoubleConstant(value.f64);
                default: break;
                }
                StyledString ss;
                ss.out << "IL->SPIR: unsupported real constant type " << value.type;
                location_error(ss.str());
            } break;
            case TK_Pointer: {
                if (is_function_pointer(value.type)) {
                    StyledString ss;
                    ss.out << "IL->SPIR: function pointer constants are unsupported";
                    location_error(ss.str());
                }
                auto pt = cast<PointerType>(value.type);
                auto val = argument_to_value(pt->unpack(value.pointer));
                auto id = builder.createVariable(spv::StorageClassFunction,
                    builder.getTypeId(val), nullptr);
                builder.getInstruction(id)->addIdOperand(val);
                return id;
            } break;
            case TK_Typename: {
                auto tn = cast<TypenameType>(value.type);
                assert(tn->finalized());
                Any storage_value = value;
                storage_value.type = tn->storage_type;
                return argument_to_value(storage_value);
            } break;
            case TK_Array: {
                auto ai = cast<ArrayType>(value.type);
                size_t count = ai->count;
                std::vector<spv::Id> values;
                for (size_t i = 0; i < count; ++i) {
                    values.push_back(argument_to_value(ai->unpack(value.pointer, i)));
                }
                return builder.makeCompositeConstant(
                    type_to_spirv_type(value.type), values);
            } break;
            case TK_Vector: {
                auto vi = cast<VectorType>(value.type);
                size_t count = vi->count;
                std::vector<spv::Id> values;
                for (size_t i = 0; i < count; ++i) {
                    values.push_back(argument_to_value(vi->unpack(value.pointer, i)));
                }
                return builder.makeCompositeConstant(
                    type_to_spirv_type(value.type), values);
            } break;
            case TK_Tuple: {
                auto ti = cast<TupleType>(value.type);
                size_t count = ti->types.size();
                std::vector<spv::Id> values;
                for (size_t i = 0; i < count; ++i) {
                    values.push_back(argument_to_value(ti->unpack(value.pointer, i)));
                }
                return builder.makeCompositeConstant(
                    type_to_spirv_type(value.type), values);
            } break;
            case TK_Union: {
                auto ui = cast<UnionType>(value.type);
                value.type = ui->tuple_type;
                return argument_to_value(value);
            } break;
            default: {
            } break;
            }
        }
        auto it = const_cache.find(value);
        if (it != const_cache.end()) {
            return it->second;
        }
        auto id = create_spirv_value(value);
        const_cache.insert({ value, id });
        return id;
    }

    spv::Id create_spirv_value(Any value) {
        if (value.type == TYPE_String) {
            return builder.createString(value.string->data);
        }
        switch(value.type->kind()) {
        case TK_Extern: {
            auto et = cast<ExternType>(value.type);
            spv::StorageClass sc = storage_class_from_extern_class(
                et->storage_class);
            const char *name = nullptr;
            spv::BuiltIn builtin = spv::BuiltInMax;
            switch(value.symbol.value()) {
            #define T(NAME) \
            case SYM_SPIRV_BuiltIn ## NAME: \
                builtin = spv::BuiltIn ## NAME; break;
                B_SPIRV_BUILTINS()
            #undef T
                default:
                    name = value.symbol.name()->data;
                    break;
            }
            auto ty = type_to_spirv_type(et->type, et->flags);
            auto id = builder.createVariable(sc, ty, name);
            if (builtin != spv::BuiltInMax) {
                builder.addDecoration(id, spv::DecorationBuiltIn, builtin);
            }
            switch(sc) {
            case spv::StorageClassUniformConstant:
            case spv::StorageClassUniform: {
                //builder.addDecoration(id, spv::DecorationDescriptorSet, 0);
                if (et->binding >= 0) {
                    builder.addDecoration(id, spv::DecorationBinding, et->binding);
                }
                if (et->location >= 0) {
                    builder.addDecoration(id, spv::DecorationLocation, et->location);
                }
                if (builder.isImageType(ty)) {
                    auto flags = et->flags;
                    if (flags & EF_Volatile) {
                        builder.addDecoration(id, spv::DecorationVolatile);
                    }
                    if (flags & EF_Coherent) {
                        builder.addDecoration(id, spv::DecorationCoherent);
                    }
                    if (flags & EF_Restrict) {
                        builder.addDecoration(id, spv::DecorationRestrict);
                    }
                    if (flags & EF_NonWritable) {
                        builder.addDecoration(id, spv::DecorationNonWritable);
                    }
                    if (flags & EF_NonReadable) {
                        builder.addDecoration(id, spv::DecorationNonReadable);
                    }
                }
            } break;
            default: {
                if (et->location >= 0) {
                    builder.addDecoration(id, spv::DecorationLocation, et->location);
                }
            } break;
            }
            return id;
        } break;
        default: break;
        };

        StyledString ss;
        ss.out << "IL->SPIR: cannot convert argument of type " << value.type;
        location_error(ss.str());
        return 0;
    }

    bool is_bool(spv::Id value) {
        auto T = builder.getTypeId(value);
        return
            (builder.isVectorType(T)
             && builder.isBoolType(builder.getContainedTypeId(T)))
            || builder.isBoolType(T);
    }

    void write_anchor(const Anchor *anchor) {
        assert(anchor);
        assert(anchor->file);
        if (use_debug_info) {
            builder.addLine(
                argument_to_value(anchor->path().name()),
                anchor->lineno, anchor->column);
        }
    }

    // set of processed SCC groups
    std::unordered_set<size_t> handled_loops;

    Label *lowest_common_ancestor(Label *a, Label *b, bool verbose = false) {
        // walk both labels simultaneously and stop as soon as we find a
        // collision with the other label's set
        std::unordered_set<Label *> done[2];
        Label *labels[2] = { a, b };
        while (labels[0] || labels[1]) {
            for (int i = 0; i < 2; ++i) {
                Label *l = labels[i];
                if (!l)
                    continue;
                if (!done[i].count(l)) {
                    if (verbose) {
                        StyledStream ss;
                        ss << "argument " << i << std::endl;
                        StreamLabelFormat fmt = StreamLabelFormat::single();
                        fmt.anchors = StreamLabelFormat::Line;
                        stream_label(ss, l, fmt);
                    }
                    if (done[(i+1)&1].count(l)) {
                        return l;
                    }
                    done[i].insert(l);
                    assert (l->is_basic_block_like());
                    auto &&enter = l->body.enter;
                    auto &&args = l->body.args;
                    if ((enter.type == TYPE_Builtin)
                        && (enter.builtin.value() == FN_Branch)) {
                        assert(args.size() >= 4);
                        Label *then_label = args[2].value;
                        //Label *else_label = args[3].value;
                        labels[i] = then_label;
                        continue;
                    } else if (!l->is_jumping()) {
                        Any arg = l->body.args[0].value;
                        if (arg.type == TYPE_Label) {
                            labels[i] = arg;
                            continue;
                        }
                    } else if (enter.type == TYPE_Label) {
                        labels[i] = enter;
                        continue;
                    }
                }
                labels[i] = nullptr;
            }
        }

        return nullptr;
    }

    bool handle_loop_label (Label *label,
        Label *&continue_label,
        Label *&break_label) {
        auto &&group = scc.group(label);
        if (group.labels.size() <= 1)
            return false;
        if (handled_loops.count(group.index))
            return false;
        handled_loops.insert(group.index);
        auto &&labels = group.labels;
        Label *header_label = label;
        continue_label = nullptr;
        break_label = nullptr;
        std::unordered_set<Label *> break_labels;
        size_t count = labels.size();
        for (size_t i = 0; i < count; ++i) {
            Label *l = labels[i];
            if (l->is_calling(header_label)
                || l->is_continuing_to(header_label)) {
                if (continue_label) {
                    StyledStream ss;
                    ss << header_label->anchor << " for this loop" << std::endl;
                    ss << continue_label->body.anchor << " previous continue is here" << std::endl;
                    //stream_label(ss, continue_label, StreamLabelFormat::debug_single());
                    //stream_label(ss, l, StreamLabelFormat::debug_single());
                    set_active_anchor(l->body.anchor);
                    location_error(String::from(
                        "IL->SPIR: duplicate continue label found. only one continue label is permitted per loop."));
                }
                continue_label = l;
            }
            auto &&enter = l->body.enter;
            if ((enter.type == TYPE_Builtin)
                && (enter.builtin.value() == FN_Branch)) {
                auto &&args = l->body.args;
                assert(args.size() >= 4);
                Label *then_label = args[2].value;
                Label *else_label = args[3].value;
                if (scc.group_id(then_label) != group.index) {
                    break_labels.insert(then_label);
                } else if (scc.group_id(else_label) != group.index) {
                    break_labels.insert(else_label);
                }
            }
        }
        assert(continue_label);
        assert(continue_label->is_basic_block_like());

        if (break_labels.empty()) {
            location_error(String::from(
                "IL->SPIR: loop is infinite"));
        }

        // as long as the continue label is dominated by a single predecessor, the
        // predecessor becomes the continue label; that way we move as much
        // unconditional code as possible into the continue section
        while (true) {
            Label *label = get_single_caller(continue_label);
            if (!label)
                break;
            continue_label = label;
        }

        if (break_labels.size() > 1) {
            Label *lca = nullptr;
            for (auto label : break_labels) {
                if (!lca) {
                    lca = label;
                } else {
                    lca = lowest_common_ancestor(lca, label);
                    if (!lca)
                        break;
                }
            }
            if (!lca) {
                StyledStream ss;
                ss << header_label->anchor << " for this loop" << std::endl;
                for (auto label : break_labels) {
                    ss << label->anchor << " found exit point" << std::endl;
                    label->anchor->stream_source_line(ss);
                    if (!lca) {
                        lca = label;
                    } else {
                        Label *old_lca = lca;
                        lca = lowest_common_ancestor(lca, label);
                        if (lca) {
                            ss << lca->anchor << " found merge candidate" << std::endl;
                            lca->anchor->stream_source_line(ss);
                        } else {
                            ss << label->anchor << " but exits function" << std::endl;
                            lca = old_lca;
                        }
                    }
                }
                location_error(String::from(
                    "IL->SPIR: cannot merge multiple loop exit points."));
            }
            break_label = lca;
        } else {
            break_label = *break_labels.begin();
        }

        assert(break_label->is_basic_block_like());
        #if 0
        StyledStream ss;
        ss << "loop found:" << std::endl;
        ss << "    labels in group:";
        for (size_t i = 0; i < count; ++i) {
            ss << " " << labels[i];
        }
        ss << std::endl;
        ss << "    entry: " << label << std::endl;
        if (continue_label) {
            ss << "    continue: " << continue_label << std::endl;
        }
        if (break_label) {
            ss << "    break: " << break_label << std::endl;
        }
        #endif
        return true;
    }

    spv::Id build_extract_image(spv::Id value) {
        spv::Id typeId = builder.getTypeId(value);
        if (builder.isSampledImageType(typeId)) {
            spv::Id imgtypeId = builder.getImageType(value);
            return builder.createUnaryOp(spv::OpImage, imgtypeId, value);
        }
        return value;
    }

    void bind_parameter(Parameter *param, spv::Id value) {
        assert(value);
        param2value[{active_function_value, param}] = value;
    }

    void write_label_body(Label *label) {
    repeat:
        assert(label->body.is_complete());

        auto &&body = label->body;
        auto &&enter = body.enter;
        auto &&args = body.args;

        set_active_anchor(label->body.anchor);

        write_anchor(label->body.anchor);

        Label *continue_label = nullptr;
        Label *break_label = nullptr;
        if (handle_loop_label(label, continue_label, break_label)) {
            spv::Block *bb_continue = nullptr;
            spv::Block *bb_merge = nullptr;
            unsigned int control = spv::LoopControlMaskNone;
            bb_continue = label_to_basic_block(continue_label, true);
            bb_merge = label_to_basic_block(break_label, true);
            builder.createLoopMerge(bb_merge, bb_continue, control);
            auto bb = &builder.makeNewBlock();
            builder.createBranch(bb);
            builder.setBuildPoint(bb);
        }

        assert(!args.empty());
        size_t argcount = args.size() - 1;
        size_t argn = 1;
#define READ_ANY(NAME) \
        assert(argn <= argcount); \
        Any &NAME = args[argn++].value;
#define READ_VALUE(NAME) \
        assert(argn <= argcount); \
        auto && _arg_ ## NAME = args[argn++]; \
        auto && _ ## NAME = _arg_ ## NAME .value; \
        spv::Id NAME = argument_to_value(_ ## NAME);
#define READ_LABEL_BLOCK(NAME) \
        assert(argn <= argcount); \
        spv::Block *NAME = label_to_basic_block(args[argn++].value); \
        assert(NAME);
#define READ_TYPE(NAME) \
        assert(argn <= argcount); \
        assert(args[argn].value.type == TYPE_Type); \
        spv::Id NAME = type_to_spirv_type(args[argn++].value.typeref);

        spv::Id retvalue = 0;
        ReturnTraits rtraits;
        if (enter.type == TYPE_Builtin) {
            switch(enter.builtin.value()) {
            case FN_Sample: {
                READ_VALUE(sampler);
                READ_VALUE(coords);
                spv::Builder::TextureParameters params;
                memset(&params, 0, sizeof(params));
                params.sampler = sampler;
                params.coords = coords;
                auto ST = storage_type(_sampler.indirect_type());
                if (ST->kind() == TK_SampledImage) {
                    ST = storage_type(cast<SampledImageType>(ST)->type);
                }
                auto resultType = type_to_spirv_type(cast<ImageType>(ST)->type);
                bool sparse = false;
                bool fetch = false;
                bool proj = false;
                bool gather = false;
                bool explicitLod = false;
                while (argn <= argcount) {
                    READ_VALUE(value);
                    switch (_arg_value.key.value()) {
                        case SYM_SPIRV_ImageOperandLod: params.lod = value; break;
                        case SYM_SPIRV_ImageOperandBias: params.bias = value; break;
                        case SYM_SPIRV_ImageOperandDref: params.Dref = value; break;
                        case SYM_SPIRV_ImageOperandProj: proj = true; break;
                        case SYM_SPIRV_ImageOperandFetch: fetch = true; break;
                        case SYM_SPIRV_ImageOperandGradX: params.gradX = value; break;
                        case SYM_SPIRV_ImageOperandGradY: params.gradY = value; break;
                        case SYM_SPIRV_ImageOperandOffset: params.offset = value; break;
                        case SYM_SPIRV_ImageOperandConstOffsets: params.offsets = value; break;
                        case SYM_SPIRV_ImageOperandMinLod: params.lodClamp = value; break;
                        case SYM_SPIRV_ImageOperandSample: params.sample = value; break;
                        case SYM_SPIRV_ImageOperandGather: {
                            params.component = value;
                            gather = true;
                        } break;
                        case SYM_SPIRV_ImageOperandSparse: {
                            params.texelOut = value;
                            sparse = true;
                        } break;
                        default: break;
                    }
                }
                if (fetch) {
                    params.sampler = build_extract_image(sampler);
                }
                retvalue = builder.createTextureCall(
                    spv::NoPrecision, resultType, sparse, fetch, proj, gather,
                    explicitLod, params);
            } break;
            case FN_ImageQuerySize: {
                READ_VALUE(sampler);
                spv::Builder::TextureParameters params;
                memset(&params, 0, sizeof(params));
                params.sampler = build_extract_image(sampler);
                spv::Op op = spv::OpImageQuerySize;
                while (argn <= argcount) {
                    READ_VALUE(value);
                    switch (_arg_value.key.value()) {
                        case SYM_SPIRV_ImageOperandLod:
                            op = spv::OpImageQuerySizeLod;
                            params.lod = value; break;
                        default: break;
                    }
                }
                retvalue = builder.createTextureQueryCall(op, params, false);
            } break;
            case FN_ImageQueryLod: {
                READ_VALUE(sampler);
                READ_VALUE(coords);
                spv::Builder::TextureParameters params;
                memset(&params, 0, sizeof(params));
                params.sampler = sampler;
                params.coords = coords;
                retvalue = builder.createTextureQueryCall(
                    spv::OpImageQueryLod, params, false);
            } break;
            case FN_ImageQueryLevels: {
                READ_VALUE(sampler);
                spv::Builder::TextureParameters params;
                memset(&params, 0, sizeof(params));
                params.sampler = build_extract_image(sampler);
                retvalue = builder.createTextureQueryCall(
                    spv::OpImageQueryLevels, params, false);
            } break;
            case FN_ImageQuerySamples: {
                READ_VALUE(sampler);
                spv::Builder::TextureParameters params;
                memset(&params, 0, sizeof(params));
                params.sampler = build_extract_image(sampler);
                retvalue = builder.createTextureQueryCall(
                    spv::OpImageQuerySamples, params, false);
            } break;
            case FN_ImageRead: {
                READ_VALUE(image);
                READ_VALUE(coords);
                auto ST = _image.indirect_type();
                auto resultType = type_to_spirv_type(cast<ImageType>(ST)->type);
                retvalue = builder.createBinOp(spv::OpImageRead,
                    resultType, image, coords);
            } break;
            case FN_ImageWrite: {
                READ_VALUE(image);
                READ_VALUE(coords);
                READ_VALUE(texel);
                builder.createNoResultOp(spv::OpImageWrite, { image, coords, texel });
            } break;
            case FN_Branch: {
                READ_VALUE(cond);
                READ_LABEL_BLOCK(then_block);
                READ_LABEL_BLOCK(else_block);
                builder.createConditionalBranch(cond, then_block, else_block);
                rtraits.terminated = true;
            } break;
            case OP_Tertiary: {
                READ_VALUE(cond);
                READ_VALUE(then_value);
                READ_VALUE(else_value);
                if (builder.isScalar(cond) && builder.isVector(then_value)) {
                    // emulate LLVM's behavior for selecting full vectors with single booleans
                    spv::Id elementT = builder.getTypeId(cond);
                    spv::Id vectorT = builder.getTypeId(then_value);

                    cond = builder.smearScalar(spv::NoPrecision, cond,
                        builder.makeVectorType(elementT, builder.getNumTypeComponents(vectorT)));
                }
                retvalue = builder.createTriOp(spv::OpSelect,
                    builder.getTypeId(then_value), cond,
                    then_value, else_value);
            } break;
            case OP_FMix: {
                READ_VALUE(a);
                READ_VALUE(b);
                READ_VALUE(x);
                retvalue = builder.createBuiltinCall(
                    builder.getTypeId(a),
                    glsl_ext_inst, GLSLstd450FMix, { a, b, x });
            } break;
            case FN_Length:
            case FN_Normalize:
            case OP_Sin:
            case OP_Cos:
            case OP_Tan:
            case OP_Asin:
            case OP_Acos:
            case OP_Atan:
            case OP_Trunc:
            case OP_Floor:
            case OP_FAbs:
            case OP_FSign:
            case OP_Log:
            case OP_Log2:
            case OP_Exp:
            case OP_Exp2:
            case OP_Sqrt: {
                READ_VALUE(val);
                GLSLstd450 builtin = GLSLstd450Bad;
                auto rtype = builder.getTypeId(val);
                switch (enter.symbol.value()) {
                case FN_Length:
                    rtype = builder.getContainedTypeId(rtype);
                    builtin = GLSLstd450Length; break;
                case FN_Normalize: builtin = GLSLstd450Normalize; break;
                case OP_Sin: builtin = GLSLstd450Sin; break;
                case OP_Cos: builtin = GLSLstd450Cos; break;
                case OP_Tan: builtin = GLSLstd450Tan; break;
                case OP_Asin: builtin = GLSLstd450Asin; break;
                case OP_Acos: builtin = GLSLstd450Acos; break;
                case OP_Atan: builtin = GLSLstd450Atan; break;
                case OP_Trunc: builtin = GLSLstd450Trunc; break;
                case OP_Floor: builtin = GLSLstd450Floor; break;
                case OP_FAbs: builtin = GLSLstd450FAbs; break;
                case OP_FSign: builtin = GLSLstd450FSign; break;
                case OP_Sqrt: builtin = GLSLstd450Sqrt; break;
                case OP_Log: builtin = GLSLstd450Log; break;
                case OP_Log2: builtin = GLSLstd450Log2; break;
                case OP_Exp: builtin = GLSLstd450Exp; break;
                case OP_Exp2: builtin = GLSLstd450Exp2; break;
                default: {
                    StyledString ss;
                    ss.out << "IL->SPIR: unsupported unary intrinsic " << enter << " encountered";
                    location_error(ss.str());
                } break;
                }
                retvalue = builder.createBuiltinCall(rtype, glsl_ext_inst, builtin, { val });
            } break;
            case FN_Cross:
            case OP_Step:
            case OP_Pow: {
                READ_VALUE(a);
                READ_VALUE(b);
                GLSLstd450 builtin = GLSLstd450Bad;
                auto rtype = builder.getTypeId(a);
                switch (enter.symbol.value()) {
                case OP_Step: builtin = GLSLstd450Step; break;
                case OP_Pow: builtin = GLSLstd450Pow; break;
                case FN_Cross: builtin = GLSLstd450Cross; break;
                default: {
                    StyledString ss;
                    ss.out << "IL->SPIR: unsupported binary intrinsic " << enter << " encountered";
                    location_error(ss.str());
                } break;
                }
                retvalue = builder.createBuiltinCall(rtype, glsl_ext_inst, builtin, { a, b });
            } break;
            case FN_Unconst: {
                READ_VALUE(val);
                retvalue = val;
            } break;
            case FN_ExtractValue: {
                READ_VALUE(val);
                READ_ANY(index);
                int i = cast_number<unsigned>(index);
                retvalue = builder.createCompositeExtract(val,
                    builder.getContainedTypeId(builder.getTypeId(val), i),
                    i);
            } break;
            case FN_InsertValue: {
                READ_VALUE(val);
                READ_VALUE(eltval);
                READ_ANY(index);
                retvalue = builder.createCompositeInsert(eltval, val,
                    builder.getTypeId(val),
                    cast_number<unsigned>(index));
            } break;
            case FN_ExtractElement: {
                READ_VALUE(val);
                READ_VALUE(index);
                if (_index.is_const()) {
                    int i = cast_number<unsigned>(_index);
                    retvalue = builder.createCompositeExtract(val,
                        builder.getContainedTypeId(builder.getTypeId(val), i),
                        i);
                } else {
                    retvalue = builder.createVectorExtractDynamic(val,
                        builder.getContainedTypeId(builder.getTypeId(val)),
                        index);
                }
            } break;
            case FN_InsertElement: {
                READ_VALUE(val);
                READ_VALUE(eltval);
                READ_VALUE(index);
                if (_index.is_const()) {
                    retvalue = builder.createCompositeInsert(eltval, val,
                        builder.getTypeId(val),
                        cast_number<unsigned>(_index));
                } else {
                    retvalue = builder.createVectorInsertDynamic(val,
                        builder.getTypeId(val), eltval, index);
                }
            } break;
            case FN_ShuffleVector: {
                READ_VALUE(v1);
                READ_VALUE(v2);
                READ_VALUE(mask);
                auto ET = builder.getContainedTypeId(builder.getTypeId(v1));
                auto sz = builder.getNumTypeComponents(builder.getTypeId(mask));
                auto op = new spv::Instruction(
                    builder.getUniqueId(),
                    builder.makeVectorType(ET, sz),
                    spv::OpVectorShuffle);
                op->addIdOperand(v1);
                op->addIdOperand(v2);
                auto vt = cast<VectorType>(storage_type(_mask.type));
                for (int i = 0; i < sz; ++i) {
                    op->addImmediateOperand(
                        cast_number<unsigned int>(vt->unpack(_mask.pointer, i)));
                }
                retvalue = op->getResultId();
                builder.getBuildPoint()->addInstruction(
                    std::unique_ptr<spv::Instruction>(op));
            } break;
            case FN_Undef: { READ_TYPE(ty);
                retvalue = builder.createUndefined(ty); } break;
            case FN_Alloca: { READ_TYPE(ty);
                retvalue = builder.createVariable(
                    spv::StorageClassFunction, ty); } break;
            /*
            case FN_AllocaArray: { READ_TYPE(ty); READ_VALUE(val);
                retvalue = LLVMBuildArrayAlloca(builder, ty, val, ""); } break;
            */
            case FN_AllocaOf: {
                READ_VALUE(val);
                retvalue = builder.createVariable(spv::StorageClassFunction,
                    builder.getTypeId(val));
                builder.createStore(val, retvalue);
            } break;
            /*
            case FN_Malloc: { READ_TYPE(ty);
                retvalue = LLVMBuildMalloc(builder, ty, ""); } break;
            case FN_MallocArray: { READ_TYPE(ty); READ_VALUE(val);
                retvalue = LLVMBuildArrayMalloc(builder, ty, val, ""); } break;
            case FN_Free: { READ_VALUE(val);
                retvalue = LLVMBuildFree(builder, val); } break;
            */
            case SFXFN_ExecutionMode: {
                assert(active_function_value);
                READ_ANY(mode);
                auto em = execution_mode_from_symbol(mode.symbol);
                int values[3] = { -1, -1, -1 };
                int c = 0;
                while ((c < 3) && (argn <= argcount)) {
                    READ_ANY(val);
                    values[c] = cast_number<int>(val);
                    c++;
                }
                builder.addExecutionMode(active_function_value, em,
                    values[0], values[1], values[2]);
            } break;
            case FN_GetElementPtr: {
                READ_VALUE(pointer);
                assert(argcount > 1);
                size_t count = argcount - 1;
                std::vector<spv::Id> indices;
                for (size_t i = 1; i < count; ++i) {
                    indices.push_back(argument_to_value(args[argn + i].value));
                }

                retvalue = builder.createAccessChain(
                    builder.getTypeStorageClass(builder.getTypeId(pointer)),
                    pointer, indices);
            } break;
            case FN_Bitcast:
            case FN_IntToPtr:
            case FN_PtrToInt:
            case FN_ITrunc:
            case FN_SExt:
            case FN_ZExt:
            case FN_FPTrunc:
            case FN_FPExt:
            case FN_FPToUI:
            case FN_FPToSI:
            case FN_UIToFP:
            case FN_SIToFP:
            {
                READ_VALUE(val); READ_TYPE(ty);
                spv::Op op = spv::OpMax;
                switch(enter.builtin.value()) {
                case FN_Bitcast:
                    if (builder.getTypeId(val) == ty) {
                        // do nothing
                        retvalue = val;
                    } else {
                        op = spv::OpBitcast;
                    }
                    break;
                case FN_IntToPtr: op = spv::OpConvertUToPtr; break;
                case FN_PtrToInt: op = spv::OpConvertPtrToU; break;
                case FN_SExt: op = spv::OpSConvert; break;
                case FN_ZExt: op = spv::OpUConvert; break;
                case FN_ITrunc: op = spv::OpSConvert; break;
                case FN_FPTrunc: op = spv::OpFConvert; break;
                case FN_FPExt: op = spv::OpFConvert; break;
                case FN_FPToUI: op = spv::OpConvertFToU; break;
                case FN_FPToSI: op = spv::OpConvertFToS; break;
                case FN_UIToFP: op = spv::OpConvertUToF; break;
                case FN_SIToFP: op = spv::OpConvertSToF; break;
                default: break;
                }
                if (op != spv::OpMax) {
                    retvalue = builder.createUnaryOp(op, ty, val);
                }
            } break;
            case FN_VolatileLoad:
            case FN_Load: {
                READ_VALUE(ptr);
                retvalue = builder.createLoad(ptr);
                if (enter.builtin == FN_VolatileLoad) {
                    builder.getInstruction(retvalue)->addImmediateOperand(
                        1<<spv::MemoryAccessVolatileShift);
                }
            } break;
            case FN_VolatileStore:
            case FN_Store: {
                READ_VALUE(val); READ_VALUE(ptr);
                builder.createStore(val, ptr);
                if (enter.builtin == FN_VolatileStore) {
                    builder.getInstruction(retvalue)->addImmediateOperand(
                        1<<spv::MemoryAccessVolatileShift);
                }
            } break;
            case OP_ICmpEQ:
            case OP_ICmpNE:
            case OP_ICmpUGT:
            case OP_ICmpUGE:
            case OP_ICmpULT:
            case OP_ICmpULE:
            case OP_ICmpSGT:
            case OP_ICmpSGE:
            case OP_ICmpSLT:
            case OP_ICmpSLE:
            case OP_FCmpOEQ:
            case OP_FCmpONE:
            case OP_FCmpORD:
            case OP_FCmpOGT:
            case OP_FCmpOGE:
            case OP_FCmpOLT:
            case OP_FCmpOLE:
            case OP_FCmpUEQ:
            case OP_FCmpUNE:
            case OP_FCmpUNO:
            case OP_FCmpUGT:
            case OP_FCmpUGE:
            case OP_FCmpULT:
            case OP_FCmpULE: { READ_VALUE(a); READ_VALUE(b);
                spv::Op op = spv::OpMax;
#define BOOL_OR_INT_OP(BOOL_OP, INT_OP) \
    (is_bool(a)?(BOOL_OP):(INT_OP))
                switch(enter.builtin.value()) {
                case OP_ICmpEQ: op = BOOL_OR_INT_OP(spv::OpLogicalEqual, spv::OpIEqual); break;
                case OP_ICmpNE: op = BOOL_OR_INT_OP(spv::OpLogicalNotEqual, spv::OpINotEqual); break;
                case OP_ICmpUGT: op = spv::OpUGreaterThan; break;
                case OP_ICmpUGE: op = spv::OpUGreaterThanEqual; break;
                case OP_ICmpULT: op = spv::OpULessThan; break;
                case OP_ICmpULE: op = spv::OpULessThanEqual; break;
                case OP_ICmpSGT: op = spv::OpSGreaterThan; break;
                case OP_ICmpSGE: op = spv::OpSGreaterThanEqual; break;
                case OP_ICmpSLT: op = spv::OpSLessThan; break;
                case OP_ICmpSLE: op = spv::OpSLessThanEqual; break;
                case OP_FCmpOEQ: op = spv::OpFOrdEqual; break;
                case OP_FCmpONE: op = spv::OpFOrdNotEqual; break;
                case OP_FCmpORD: op = spv::OpOrdered; break;
                case OP_FCmpOGT: op = spv::OpFOrdGreaterThan; break;
                case OP_FCmpOGE: op = spv::OpFOrdGreaterThanEqual; break;
                case OP_FCmpOLT: op = spv::OpFOrdLessThan; break;
                case OP_FCmpOLE: op = spv::OpFOrdLessThanEqual; break;
                case OP_FCmpUEQ: op = spv::OpFUnordEqual; break;
                case OP_FCmpUNE: op = spv::OpFUnordNotEqual; break;
                case OP_FCmpUNO: op = spv::OpUnordered; break;
                case OP_FCmpUGT: op = spv::OpFUnordGreaterThan; break;
                case OP_FCmpUGE: op = spv::OpFUnordGreaterThanEqual; break;
                case OP_FCmpULT: op = spv::OpFUnordLessThan; break;
                case OP_FCmpULE: op = spv::OpFUnordLessThanEqual; break;
                default: break;
                }
#undef BOOL_OR_INT_OP
                auto T = builder.getTypeId(a);
                if (builder.isVectorType(T)) {
                    T = builder.makeVectorType(builder.makeBoolType(),
                        builder.getNumTypeComponents(T));
                } else {
                    T = builder.makeBoolType();
                }
                retvalue = builder.createBinOp(op, T, a, b); } break;
            case OP_Add:
            case OP_AddNUW:
            case OP_AddNSW:
            case OP_Sub:
            case OP_SubNUW:
            case OP_SubNSW:
            case OP_Mul:
            case OP_MulNUW:
            case OP_MulNSW:
            case OP_SDiv:
            case OP_UDiv:
            case OP_SRem:
            case OP_URem:
            case OP_Shl:
            case OP_LShr:
            case OP_AShr:
            case OP_BAnd:
            case OP_BOr:
            case OP_BXor:
            case OP_FAdd:
            case OP_FSub:
            case OP_FMul:
            case OP_FDiv:
            case OP_FRem: { READ_VALUE(a); READ_VALUE(b);
                spv::Op op = spv::OpMax;
                switch(enter.builtin.value()) {
#define BOOL_OR_INT_OP(BOOL_OP, INT_OP) \
    (is_bool(a)?(BOOL_OP):(INT_OP))
                case OP_Add:
                case OP_AddNUW:
                case OP_AddNSW: op = spv::OpIAdd; break;
                case OP_Sub:
                case OP_SubNUW:
                case OP_SubNSW: op = spv::OpISub; break;
                case OP_Mul:
                case OP_MulNUW:
                case OP_MulNSW: op = spv::OpIMul; break;
                case OP_SDiv: op = spv::OpSDiv; break;
                case OP_UDiv: op = spv::OpUDiv; break;
                case OP_SRem: op = spv::OpSRem; break;
                case OP_URem: op = spv::OpUMod; break;
                case OP_Shl: op = spv::OpShiftLeftLogical; break;
                case OP_LShr: op = spv::OpShiftRightLogical; break;
                case OP_AShr: op = spv::OpShiftRightArithmetic; break;
                case OP_BAnd: op = BOOL_OR_INT_OP(spv::OpLogicalAnd, spv::OpBitwiseAnd); break;
                case OP_BOr: op = BOOL_OR_INT_OP(spv::OpLogicalOr, spv::OpBitwiseOr); break;
                case OP_BXor: op = BOOL_OR_INT_OP(spv::OpLogicalNotEqual, spv::OpBitwiseXor); break;
                case OP_FAdd: op = spv::OpFAdd; break;
                case OP_FSub: op = spv::OpFSub; break;
                case OP_FMul: op = spv::OpFMul; break;
                case OP_FDiv: op = spv::OpFDiv; break;
                case OP_FRem: op = spv::OpFRem; break;
                default: break;
                }
#undef BOOL_OR_INT_OP
                retvalue = builder.createBinOp(op,
                    builder.getTypeId(a), a, b); } break;
            case SFXFN_Unreachable:
                builder.makeUnreachable();
                rtraits.terminated = true; break;
            case SFXFN_Discard:
                builder.makeDiscard();
                rtraits.terminated = true; break;
            default: {
                StyledString ss;
                ss.out << "IL->SPIR: unsupported builtin " << enter.builtin << " encountered";
                location_error(ss.str());
            } break;
            }
        } else if (enter.type == TYPE_Label) {
            if (enter.label->is_basic_block_like()) {
                auto block = label_to_basic_block(enter.label);
                if (!block) {
                    // no basic block was generated - just generate assignments
                    auto &&params = enter.label->params;
                    for (size_t i = 1; i < params.size(); ++i) {
                        bind_parameter(params[i], argument_to_value(args[i].value));
                    }
                    label = enter.label;
                    goto repeat;
                } else {
                    auto bbfrom = builder.getBuildPoint();
                    // assign phi nodes
                    auto &&params = enter.label->params;
                    bool single_caller = has_single_caller(enter.label);
                    for (size_t i = 1; i < params.size(); ++i) {
                        Parameter *param = params[i];
                        auto value = argument_to_value(args[i].value);
                        if (single_caller) {
                            assert(!parameter_is_bound(param));
                            bind_parameter(param, value);
                        } else {
                            auto phinode = argument_to_value(param);
                            auto op = builder.getInstruction(phinode);
                            assert(op);
                            op->addIdOperand(value);
                            op->addIdOperand(bbfrom->getId());
                        }
                    }
                    builder.createBranch(block);
                    rtraits.terminated = true;
                }
            } else {
                /*if (use_debug_info) {
                    LLVMSetCurrentDebugLocation(builder, diloc);
                }*/
                auto func = label_to_function(enter.label);
                retvalue = build_call(
                    enter.label->get_function_type(),
                    func, args, rtraits);
            }
        } else if (enter.type == TYPE_Closure) {
            StyledString ss;
            ss.out << "IL->SPIR: invalid call of compile time closure at runtime";
            location_error(ss.str());
        } else if (enter.type == TYPE_Parameter) {
            assert (enter.parameter->type != TYPE_Nothing);
            assert(enter.parameter->type != TYPE_Unknown);
            std::vector<spv::Id> values;
            for (size_t i = 0; i < argcount; ++i) {
                values.push_back(argument_to_value(args[i + 1].value));
            }
            // must be a return
            assert(enter.parameter->index == 0);
            // must be returning from this function
            assert(enter.parameter->label == active_function);

            //Label *label = enter.parameter->label;
            if (argcount > 1) {
                auto ilfunctype = cast<FunctionType>(active_function->get_function_type());
                auto rettype = type_to_spirv_type(ilfunctype->return_type);
                auto id = builder.createUndefined(rettype);
                for (size_t i = 0; i < values.size(); ++i) {
                    id = builder.createCompositeInsert(
                        values[i], id, rettype, i);
                }
                builder.makeReturn(true, id);
            } else if (argcount == 1) {
                builder.makeReturn(true, values[0]);
            } else {
                builder.makeReturn(true, 0);
            }
            rtraits.terminated = true;
        } else {
            StyledString ss;
            ss.out << "IL->SPIR: cannot translate call to " << enter;
            location_error(ss.str());
        }

        Any contarg = args[0].value;
        if (rtraits.terminated) {
            // write nothing
        } else if ((contarg.type == TYPE_Parameter)
            && (contarg.parameter->type != TYPE_Nothing)) {
            assert(contarg.parameter->type != TYPE_Unknown);
            assert(contarg.parameter->index == 0);
            assert(contarg.parameter->label == active_function);
            //Label *label = contarg.parameter->label;
            if (retvalue) {
                builder.makeReturn(true, retvalue);
            } else {
                builder.makeReturn(true, 0);
            }
        } else if (contarg.type == TYPE_Label) {
            auto bb = label_to_basic_block(contarg.label);
#define UNPACK_RET_ARGS() \
    if (rtraits.multiple_return_values) { \
        assert(rtraits.rtype); \
        auto &&values = rtraits.rtype->values; \
        auto &&params = contarg.label->params; \
        size_t pi = 1; \
        for (size_t i = 0; i < values.size(); ++i) { \
            if (pi >= params.size()) \
                break; \
            Parameter *param = params[pi]; \
            auto &&arg = values[i]; \
            if (is_unknown(arg.value)) { \
                spv::Id incoval = builder.createCompositeExtract( \
                    retvalue, \
                    builder.getContainedTypeId(rtype, i), i); \
                T(param, incoval); \
                pi++; \
            } \
        } \
    } else { \
        auto &&params = contarg.label->params; \
        if (params.size() > 1) { \
            assert(params.size() == 2); \
            Parameter *param = params[1]; \
            T(param, retvalue); \
        } \
    }
            if (bb) {
                if (retvalue) {
                    auto bbfrom = builder.getBuildPoint();
                    // assign phi nodes
                    auto rtype = builder.getTypeId(retvalue);

                    bool single_caller = has_single_caller(contarg.label);
                    #define T(PARAM, VALUE) \
                        if (single_caller) { \
                            assert(!parameter_is_bound(PARAM)); \
                            bind_parameter(PARAM, VALUE); \
                        } else { \
                            auto phinode = argument_to_value(PARAM); \
                            auto op = builder.getInstruction(phinode); \
                            assert(op); \
                            op->addIdOperand(VALUE); \
                            op->addIdOperand(bbfrom->getId()); \
                        }
                    UNPACK_RET_ARGS()
                    #undef T
                }
                builder.createBranch(bb);
            } else {
                if (retvalue) {
                    // no basic block - just add assignments and continue
                    auto rtype = builder.getTypeId(retvalue);
                    #define T(PARAM, VALUE) \
                        bind_parameter(PARAM, VALUE);
                    UNPACK_RET_ARGS()
                    #undef T
                }
                label = contarg.label;
                goto repeat;
            }
#undef UNPACK_RET_ARGS
        } else if (contarg.type == TYPE_Nothing) {
            StyledStream ss(std::cerr);
            stream_label(ss, label, StreamLabelFormat::debug_single());
            location_error(String::from("IL->SPIR: unexpected end of function"));
        } else {
            StyledStream ss(std::cerr);
            stream_label(ss, label, StreamLabelFormat::debug_single());
            location_error(String::from("IL->SPIR: continuation is of invalid type"));
        }

        //LLVMSetCurrentDebugLocation(builder, nullptr);
    }
    #undef READ_ANY
    #undef READ_VALUE
    #undef READ_TYPE
    #undef READ_LABEL_BLOCK

    struct ReturnTraits {
        bool multiple_return_values;
        bool terminated;
        const ReturnLabelType *rtype;

        ReturnTraits() :
            multiple_return_values(false),
            terminated(false),
            rtype(nullptr) {}
    };

    spv::Id build_call(const Type *functype, spv::Function* func, Args &args,
        ReturnTraits &traits) {
        size_t argcount = args.size() - 1;

        auto fi = cast<FunctionType>(functype);

        std::vector<spv::Id> values;
        for (size_t i = 0; i < argcount; ++i) {
            auto &&arg = args[i + 1];
            values.push_back(argument_to_value(arg.value));
        }

        size_t fargcount = fi->argument_types.size();
        assert(argcount >= fargcount);
        if (fi->flags & FF_Variadic) {
            location_error(String::from("IL->SPIR: variadic calls not supported"));
        }

        auto ret = builder.createFunctionCall(func, values);
        auto rlt = cast<ReturnLabelType>(fi->return_type);
        traits.rtype = rlt;
        traits.multiple_return_values = rlt->has_multiple_return_values();
        if (rlt->return_type == TYPE_Void) {
            return 0;
        } else if (!rlt->is_returning()) {
            builder.makeUnreachable();
            traits.terminated = true;
            return 0;
        } else {
            return ret;
        }
    }

    void set_active_function(Label *l) {
        if (active_function == l) return;
        active_function = l;
        if (l) {
            auto it = label2func.find(l);
            assert(it != label2func.end());
            active_function_value = it->second;
        } else {
            active_function_value = nullptr;
        }
    }

    void process_labels() {
        while (!bb_label_todo.empty()) {
            auto it = bb_label_todo.back();
            set_active_function(it.first);
            Label *label = it.second;
            bb_label_todo.pop_back();

            auto it2 = label2bb.find({active_function_value, label});
            assert(it2 != label2bb.end());
            spv::Block *bb = it2->second;
            builder.setBuildPoint(bb);

            write_label_body(label);
        }
    }

    Label *get_single_caller(Label *l) {
        auto it = user_map.label_map.find(l);
        assert(it != user_map.label_map.end());
        auto &&users = it->second;
        if (users.size() != 1)
            return nullptr;
        Label *userl = *users.begin();
        if (userl->body.enter == Any(l))
            return userl;
        if (userl->body.args[0] == Any(l))
            return userl;
        return nullptr;
    }

    bool has_single_caller(Label *l) {
        Label *userl = get_single_caller(l);
        return (userl != nullptr);
    }

    spv::Id create_struct_type(const Type *type, uint64_t flags,
        const TypenameType *tname = nullptr) {
        // todo: packed tuples
        auto ti = cast<TupleType>(type);
        size_t count = ti->types.size();
        std::vector<spv::Id> members;
        for (size_t i = 0; i < count; ++i) {
            members.push_back(type_to_spirv_type(ti->types[i]));
        }
        const char *name = "tuple";
        if (tname) {
            name = tname->name()->data;
        }
        auto id = builder.makeStructType(members, name);
        if (flags & EF_BufferBlock) {
            builder.addDecoration(id, spv::DecorationBufferBlock);
        } else if (flags & EF_Block) {
            builder.addDecoration(id, spv::DecorationBlock);
        }
        for (size_t i = 0; i < count; ++i) {
            builder.addMemberName(id, i, ti->values[i].key.name()->data);
            if (flags & EF_Volatile) {
                builder.addMemberDecoration(id, i, spv::DecorationVolatile);
            }
            if (flags & EF_Coherent) {
                builder.addMemberDecoration(id, i, spv::DecorationCoherent);
            }
            if (flags & EF_Restrict) {
                builder.addMemberDecoration(id, i, spv::DecorationRestrict);
            }
            if (flags & EF_NonWritable) {
                builder.addMemberDecoration(id, i, spv::DecorationNonWritable);
            }
            if (flags & EF_NonReadable) {
                builder.addMemberDecoration(id, i, spv::DecorationNonReadable);
            }
            builder.addMemberDecoration(id, i, spv::DecorationOffset, ti->offsets[i]);
        }
        return id;
    }

    spv::Id create_spirv_type(const Type *type, uint64_t flags) {
        switch(type->kind()) {
        case TK_Integer: {
            if (type == TYPE_Bool)
                return builder.makeBoolType();
            auto it = cast<IntegerType>(type);
            return builder.makeIntegerType(it->width, it->issigned);
        } break;
        case TK_Real: {
            auto rt = cast<RealType>(type);
            return builder.makeFloatType(rt->width);
        } break;
        case TK_Pointer: {
            auto pt = cast<PointerType>(type);
            return builder.makePointer(
                storage_class_from_extern_class(pt->storage_class),
                type_to_spirv_type(pt->element_type));
        } break;
        case TK_Array: {
            auto ai = cast<ArrayType>(type);
            auto etype = type_to_spirv_type(ai->element_type);
            spv::Id ty;
            if (!ai->count) {
                ty = builder.makeRuntimeArray(etype);
            } else {
                ty = builder.makeArrayType(etype,
                    builder.makeUintConstant(ai->count), 0);
            }
            builder.addDecoration(ty,
                spv::DecorationArrayStride,
                size_of(ai->element_type));
            return ty;
        } break;
        case TK_Vector: {
            auto vi = cast<VectorType>(type);
            return builder.makeVectorType(
                type_to_spirv_type(vi->element_type),
                vi->count);
        } break;
        case TK_Tuple: {
            return create_struct_type(type, flags);
        } break;
        case TK_Union: {
            auto ui = cast<UnionType>(type);
            return type_to_spirv_type(ui->tuple_type);
        } break;
        case TK_Extern: {
            auto et = cast<ExternType>(type);
            spv::StorageClass sc = storage_class_from_extern_class(
                et->storage_class);
            auto ty = type_to_spirv_type(et->type, et->flags);
            return builder.makePointer(sc, ty);
        } break;
        case TK_Image: {
            auto it = cast<ImageType>(type);
            auto ty = type_to_spirv_type(it->type);
            if (builder.isVectorType(ty)) {
                ty = builder.getContainedTypeId(ty);
            }
            return builder.makeImageType(ty,
                dim_from_symbol(it->dim),
                (it->depth == 1),
                (it->arrayed == 1),
                (it->multisampled == 1),
                it->sampled,
                image_format_from_symbol(it->format));
        } break;
        case TK_SampledImage: {
            auto sit = cast<SampledImageType>(type);
            return builder.makeSampledImageType(type_to_spirv_type(sit->type));
        } break;
        case TK_Typename: {
            if (type == TYPE_Void)
                return builder.makeVoidType();
            else if (type == TYPE_Sampler)
                return builder.makeSamplerType();
            auto tn = cast<TypenameType>(type);
            if (tn->finalized()) {
                if (tn->storage_type->kind() == TK_Tuple) {
                    return create_struct_type(tn->storage_type, flags, tn);
                } else {
                    return type_to_spirv_type(tn->storage_type, flags);
                }
            } else {
                location_error(String::from("IL->SPIR: opaque types are not supported"));
                return 0;
            }
        } break;
        case TK_ReturnLabel: {
            auto rlt = cast<ReturnLabelType>(type);
            return type_to_spirv_type(rlt->return_type);
        } break;
        case TK_Function: {
            auto fi = cast<FunctionType>(type);
            if (fi->vararg()) {
                location_error(String::from("IL->SPIR: vararg functions are not supported"));
            }
            size_t count = fi->argument_types.size();
            spv::Id rettype = type_to_spirv_type(fi->return_type);
            std::vector<spv::Id> elements;
            for (size_t i = 0; i < count; ++i) {
                auto AT = fi->argument_types[i];
                elements.push_back(type_to_spirv_type(AT));
            }
            return builder.makeFunctionType(rettype, elements);
        } break;
        };

        StyledString ss;
        ss.out << "IL->SPIR: cannot convert type " << type;
        location_error(ss.str());
        return 0;
    }

    spv::Id type_to_spirv_type(const Type *type, uint64_t flags = 0) {
        auto it = type_cache.find({type, flags});
        if (it == type_cache.end()) {
            spv::Id result = create_spirv_type(type, flags);
            type_cache.insert({ { type, flags }, result});
            return result;
        } else {
            return it->second;
        }
    }

    spv::Block *label_to_basic_block(Label *label, bool force = false) {
        auto old_bb = builder.getBuildPoint();
        auto func = &old_bb->getParent();
        auto it = label2bb.find({func, label});
        if (it == label2bb.end()) {
            bool single_caller = has_single_caller(label);
            if (single_caller && !force) {
                // not generating basic blocks for single user labels
                return nullptr;
            }
            //const char *name = label->name.name()->data;
            auto bb = &builder.makeNewBlock();
            label2bb.insert({{func, label}, bb});
            bb_label_todo.push_back({active_function, label});
            builder.setBuildPoint(bb);

            auto &&params = label->params;
            if (!params.empty()) {
                size_t paramcount = label->params.size() - 1;
                for (size_t i = 0; i < paramcount; ++i) {
                    Parameter *param = params[i + 1];
                    auto ptype = type_to_spirv_type(param->type);
                    if (!single_caller) {
                        auto op = new spv::Instruction(
                            builder.getUniqueId(), ptype, spv::OpPhi);
                        builder.addName(op->getResultId(), param->name.name()->data);
                        bind_parameter(param, op->getResultId());
                        bb->addInstruction(std::unique_ptr<spv::Instruction>(op));
                    }
                }
            }

            builder.setBuildPoint(old_bb);
            return bb;
        } else {
            return it->second;
        }
    }

    spv::Function *label_to_function(Label *label,
        bool root_function = false,
        Symbol funcname = SYM_Unnamed) {
        auto it = label2func.find(label);
        if (it == label2func.end()) {

            const Anchor *old_anchor = get_active_anchor();
            set_active_anchor(label->anchor);
            Label *last_function = active_function;

            auto old_bb = builder.getBuildPoint();

            if (funcname == SYM_Unnamed) {
                funcname = label->name;
            }

            const char *name;
            if (root_function && (funcname == SYM_Unnamed)) {
                name = "unnamed";
            } else {
                name = funcname.name()->data;
            }

            label->verify_compilable();
            auto ilfunctype = cast<FunctionType>(label->get_function_type());
            //auto fi = cast<FunctionType>(ilfunctype);

            auto rettype = type_to_spirv_type(ilfunctype->return_type);

            spv::Block* bb;
            std::vector<spv::Id> paramtypes;

            auto &&argtypes = ilfunctype->argument_types;
            for (auto it = argtypes.begin(); it != argtypes.end(); ++it) {
                paramtypes.push_back(type_to_spirv_type(*it));
            }

            std::vector<std::vector<spv::Decoration>> decorations;

            auto func = builder.makeFunctionEntry(
                spv::NoPrecision, rettype, name,
                paramtypes, decorations, &bb);
            //LLVMSetLinkage(func, LLVMPrivateLinkage);

            label2func[label] = func;
            set_active_function(label);

            if (use_debug_info) {
                // LLVMSetFunctionSubprogram(func, label_to_subprogram(label));
            }

            builder.setBuildPoint(bb);
            write_anchor(label->anchor);

            auto &&params = label->params;
            size_t paramcount = params.size() - 1;
            for (size_t i = 0; i < paramcount; ++i) {
                Parameter *param = params[i + 1];
                auto val = func->getParamId(i);
                bind_parameter(param, val);
            }

            write_label_body(label);

            builder.setBuildPoint(old_bb);

            set_active_function(last_function);
            set_active_anchor(old_anchor);
            return func;
        } else {
            return it->second;
        }
    }

    void generate(std::vector<unsigned int> &result, Symbol target, Label *entry) {
        //assert(all_parameters_lowered(entry));
        assert(!entry->is_basic_block_like());

        builder.setSource(spv::SourceLanguageGLSL, 450);
        glsl_ext_inst = builder.import("GLSL.std.450");

        auto needfi = Function(TYPE_Void, {}, 0);
        auto hasfi = entry->get_function_type();
        if (hasfi != needfi) {
            set_active_anchor(entry->anchor);
            StyledString ss;
            ss.out << "Entry function must have type " << needfi
                << " but has type " << hasfi;
            location_error(ss.str());
        }

        {
            std::unordered_set<Label *> visited;
            Labels labels;
            entry->build_reachable(visited, &labels);
            for (auto it = labels.begin(); it != labels.end(); ++it) {
                (*it)->insert_into_usermap(user_map);
            }
        }

        scc.walk(entry);

        //const char *name = entry->name.name()->data;
        //module = LLVMModuleCreateWithName(name);

        if (use_debug_info) {
            /*
            const char *DebugStr = "Debug Info Version";
            LLVMValueRef DbgVer[3];
            DbgVer[0] = LLVMConstInt(i32T, 1, 0);
            DbgVer[1] = LLVMMDString(DebugStr, strlen(DebugStr));
            DbgVer[2] = LLVMConstInt(i32T, 3, 0);
            LLVMAddNamedMetadataOperand(module, "llvm.module.flags",
                LLVMMDNode(DbgVer, 3));

            LLVMDIBuilderCreateCompileUnit(di_builder,
                llvm::dwarf::DW_LANG_C99, "file", "directory", "scopes",
                false, "", 0, "", 0);*/
        }

        auto func = label_to_function(entry, true);

        switch(target.value()) {
        case SYM_TargetVertex: {
            builder.addCapability(spv::CapabilityShader);
            builder.addEntryPoint(spv::ExecutionModelVertex, func, "main");
        } break;
        case SYM_TargetFragment: {
            builder.addCapability(spv::CapabilityShader);
            builder.addEntryPoint(spv::ExecutionModelFragment, func, "main");
        } break;
        case SYM_TargetGeometry: {
            builder.addCapability(spv::CapabilityShader);
            builder.addEntryPoint(spv::ExecutionModelGeometry, func, "main");
        } break;
        case SYM_TargetCompute: {
            builder.addCapability(spv::CapabilityShader);
            builder.addEntryPoint(spv::ExecutionModelGLCompute, func, "main");
        } break;
        default: {
            StyledString ss;
            ss.out << "IL->SPIR: unsupported target: " << target << ", try one of "
                << Symbol(SYM_TargetVertex) << " "
                << Symbol(SYM_TargetFragment) << " "
                << Symbol(SYM_TargetGeometry) << " "
                << Symbol(SYM_TargetCompute);
            location_error(ss.str());
        } break;
        }

        process_labels();

        //size_t k = finalize_types();
        //assert(!k);

        builder.dump(result);

        verify_spirv(result);
    }
};

//------------------------------------------------------------------------------
// IL->LLVM IR GENERATOR
//------------------------------------------------------------------------------

static void build_and_run_opt_passes(LLVMModuleRef module, int opt_level) {
    LLVMPassManagerBuilderRef passBuilder;

    passBuilder = LLVMPassManagerBuilderCreate();
    LLVMPassManagerBuilderSetOptLevel(passBuilder, opt_level);
    LLVMPassManagerBuilderSetSizeLevel(passBuilder, 0);
    if (opt_level >= 2) {
        LLVMPassManagerBuilderUseInlinerWithThreshold(passBuilder, 225);
    }

    LLVMPassManagerRef functionPasses =
      LLVMCreateFunctionPassManagerForModule(module);
    LLVMPassManagerRef modulePasses =
      LLVMCreatePassManager();
    //LLVMAddAnalysisPasses(LLVMGetExecutionEngineTargetMachine(ee), functionPasses);

    LLVMPassManagerBuilderPopulateFunctionPassManager(passBuilder,
                                                      functionPasses);
    LLVMPassManagerBuilderPopulateModulePassManager(passBuilder, modulePasses);

    LLVMPassManagerBuilderDispose(passBuilder);

    LLVMInitializeFunctionPassManager(functionPasses);
    for (LLVMValueRef value = LLVMGetFirstFunction(module);
         value; value = LLVMGetNextFunction(value))
      LLVMRunFunctionPassManager(functionPasses, value);
    LLVMFinalizeFunctionPassManager(functionPasses);

    LLVMRunPassManager(modulePasses, module);

    LLVMDisposePassManager(functionPasses);
    LLVMDisposePassManager(modulePasses);
}

typedef llvm::DIBuilder *LLVMDIBuilderRef;

static LLVMDIBuilderRef LLVMCreateDIBuilder(LLVMModuleRef M) {
  return new llvm::DIBuilder(*llvm::unwrap(M));
}

static void LLVMDisposeDIBuilder(LLVMDIBuilderRef Builder) {
  Builder->finalize();
  delete Builder;
}

static llvm::MDNode *value_to_mdnode(LLVMValueRef value) {
    return value ? cast<llvm::MDNode>(
        llvm::unwrap<llvm::MetadataAsValue>(value)->getMetadata()) : nullptr;
}

template<typename T>
static T *value_to_DI(LLVMValueRef value) {
    return value ? cast<T>(
        llvm::unwrap<llvm::MetadataAsValue>(value)->getMetadata()) : nullptr;
}

static LLVMValueRef mdnode_to_value(llvm::MDNode *node) {
  return llvm::wrap(
    llvm::MetadataAsValue::get(*llvm::unwrap(LLVMGetGlobalContext()), node));
}

typedef llvm::DINode::DIFlags LLVMDIFlags;

static LLVMValueRef LLVMDIBuilderCreateSubroutineType(
    LLVMDIBuilderRef Builder, LLVMValueRef ParameterTypes) {
    return mdnode_to_value(
        Builder->createSubroutineType(value_to_DI<llvm::MDTuple>(ParameterTypes)));
}

static LLVMValueRef LLVMDIBuilderCreateCompileUnit(LLVMDIBuilderRef Builder,
    unsigned Lang,
    const char *File, const char *Dir, const char *Producer, bool isOptimized,
    const char *Flags, unsigned RV, const char *SplitName,
    //DICompileUnit::DebugEmissionKind Kind,
    uint64_t DWOId) {
    auto ctx = (llvm::LLVMContext *)LLVMGetGlobalContext();
    auto file = llvm::DIFile::get(*ctx, File, Dir);
    return mdnode_to_value(
        Builder->createCompileUnit(Lang, file,
                      Producer, isOptimized, Flags,
                      RV, SplitName,
                      llvm::DICompileUnit::DebugEmissionKind::FullDebug,
                      //llvm::DICompileUnit::DebugEmissionKind::LineTablesOnly,
                      DWOId));
}

static LLVMValueRef LLVMDIBuilderCreateFunction(
    LLVMDIBuilderRef Builder, LLVMValueRef Scope, const char *Name,
    const char *LinkageName, LLVMValueRef File, unsigned LineNo,
    LLVMValueRef Ty, bool IsLocalToUnit, bool IsDefinition,
    unsigned ScopeLine) {
  return mdnode_to_value(Builder->createFunction(
        cast<llvm::DIScope>(value_to_mdnode(Scope)), Name, LinkageName,
        cast<llvm::DIFile>(value_to_mdnode(File)),
        LineNo, cast<llvm::DISubroutineType>(value_to_mdnode(Ty)),
        IsLocalToUnit, IsDefinition, ScopeLine));
}

static LLVMValueRef LLVMGetFunctionSubprogram(LLVMValueRef func) {
    return mdnode_to_value(
        llvm::cast<llvm::Function>(llvm::unwrap(func))->getSubprogram());
}

static void LLVMSetFunctionSubprogram(LLVMValueRef func, LLVMValueRef subprogram) {
    llvm::cast<llvm::Function>(llvm::unwrap(func))->setSubprogram(
        value_to_DI<llvm::DISubprogram>(subprogram));
}

static LLVMValueRef LLVMDIBuilderCreateLexicalBlock(LLVMDIBuilderRef Builder,
    LLVMValueRef Scope, LLVMValueRef File, unsigned Line, unsigned Col) {
    return mdnode_to_value(Builder->createLexicalBlock(
        value_to_DI<llvm::DIScope>(Scope),
        value_to_DI<llvm::DIFile>(File), Line, Col));
}

static LLVMValueRef LLVMCreateDebugLocation(unsigned Line,
                                     unsigned Col, const LLVMValueRef Scope,
                                     const LLVMValueRef InlinedAt) {
  llvm::MDNode *SNode = value_to_mdnode(Scope);
  llvm::MDNode *INode = value_to_mdnode(InlinedAt);
  return mdnode_to_value(llvm::DebugLoc::get(Line, Col, SNode, INode).get());
}

static LLVMValueRef LLVMDIBuilderCreateFile(
    LLVMDIBuilderRef Builder, const char *Filename,
                            const char *Directory) {
  return mdnode_to_value(Builder->createFile(Filename, Directory));
}

static std::vector<void *> loaded_libs;
static void *local_aware_dlsym(const char *name) {
#if 1
    return LLVMSearchForAddressOfSymbol(name);
#else
    size_t i = loaded_libs.size();
    while (i--) {
        void *ptr = dlsym(loaded_libs[i], name);
        if (ptr) {
            LLVMAddSymbol(name, ptr);
            return ptr;
        }
    }
    return dlsym(global_c_namespace, name);
#endif
}

struct LLVMIRGenerator {
    enum Intrinsic {
        llvm_sin_f32,
        llvm_sin_f64,
        llvm_cos_f32,
        llvm_cos_f64,
        llvm_sqrt_f32,
        llvm_sqrt_f64,
        llvm_fabs_f32,
        llvm_fabs_f64,
        llvm_trunc_f32,
        llvm_trunc_f64,
        llvm_floor_f32,
        llvm_floor_f64,
        llvm_pow_f32,
        llvm_pow_f64,
        llvm_exp_f32,
        llvm_exp_f64,
        llvm_log_f32,
        llvm_log_f64,
        llvm_exp2_f32,
        llvm_exp2_f64,
        llvm_log2_f32,
        llvm_log2_f64,
        custom_fsign_f32,
        custom_fsign_f64,
        NumIntrinsics,
    };

    struct HashFuncLabelPair {
        size_t operator ()(const std::pair<LLVMValueRef, Label *> &value) const {
            return
                HashLen16(std::hash<LLVMValueRef>()(value.first),
                    std::hash<Label *>()(value.second));
        }
    };


    typedef std::pair<LLVMValueRef, Parameter *> ParamKey;
    struct HashFuncParamPair {
        size_t operator ()(const ParamKey &value) const {
            return
                HashLen16(std::hash<LLVMValueRef>()(value.first),
                    std::hash<Parameter *>()(value.second));
        }
    };

    std::unordered_map<Label *, LLVMValueRef> label2func;
    std::unordered_map< std::pair<LLVMValueRef, Label *>,
        LLVMBasicBlockRef, HashFuncLabelPair> label2bb;
    std::vector< std::pair<Label *, Label *> > bb_label_todo;

    std::unordered_map<Label *, LLVMValueRef> label2md;
    std::unordered_map<SourceFile *, LLVMValueRef> file2value;
    std::unordered_map< ParamKey, LLVMValueRef, HashFuncParamPair> param2value;
    static std::unordered_map<const Type *, LLVMTypeRef> type_cache;
    static ArgTypes type_todo;

    std::unordered_map<Any, LLVMValueRef, Any::Hash> extern2global;
    std::unordered_map<void *, LLVMValueRef> ptr2global;

    Label::UserMap user_map;

    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMDIBuilderRef di_builder;

    static LLVMTypeRef voidT;
    static LLVMTypeRef i1T;
    static LLVMTypeRef i8T;
    static LLVMTypeRef i16T;
    static LLVMTypeRef i32T;
    static LLVMTypeRef i64T;
    static LLVMTypeRef f32T;
    static LLVMTypeRef f32x2T;
    static LLVMTypeRef f64T;
    static LLVMTypeRef rawstringT;
    static LLVMTypeRef noneT;
    static LLVMValueRef noneV;
    static LLVMAttributeRef attr_byval;
    static LLVMAttributeRef attr_sret;
    static LLVMAttributeRef attr_nonnull;
    LLVMValueRef intrinsics[NumIntrinsics];

    Label *active_function;
    LLVMValueRef active_function_value;

    bool use_debug_info;
    bool inline_pointers;

    template<unsigned N>
    static LLVMAttributeRef get_attribute(const char (&s)[N]) {
        unsigned kind = LLVMGetEnumAttributeKindForName(s, N - 1);
        assert(kind);
        return LLVMCreateEnumAttribute(LLVMGetGlobalContext(), kind, 0);
    }

    LLVMIRGenerator() :
        active_function(nullptr),
        active_function_value(nullptr),
        use_debug_info(true),
        inline_pointers(true) {
        static_init();
        for (int i = 0; i < NumIntrinsics; ++i) {
            intrinsics[i] = nullptr;
        }
    }

    LLVMValueRef source_file_to_scope(SourceFile *sf) {
        assert(use_debug_info);

        auto it = file2value.find(sf);
        if (it != file2value.end())
            return it->second;

        char *dn = strdup(sf->path.name()->data);
        char *bn = strdup(dn);

        LLVMValueRef result = LLVMDIBuilderCreateFile(di_builder,
            basename(bn), dirname(dn));
        free(dn);
        free(bn);

        file2value.insert({ sf, result });

        return result;
    }

    LLVMValueRef label_to_subprogram(Label *l) {
        assert(use_debug_info);

        auto it = label2md.find(l);
        if (it != label2md.end())
            return it->second;

        const Anchor *anchor = l->anchor;

        LLVMValueRef difile = source_file_to_scope(anchor->file);

        LLVMValueRef subroutinevalues[] = {
            nullptr
        };
        LLVMValueRef disrt = LLVMDIBuilderCreateSubroutineType(di_builder,
            LLVMMDNode(subroutinevalues, 1));

        LLVMValueRef difunc = LLVMDIBuilderCreateFunction(
            di_builder, difile, l->name.name()->data, l->name.name()->data,
            difile, anchor->lineno, disrt, false, true,
            anchor->lineno);

        label2md.insert({ l, difunc });
        return difunc;
    }

    LLVMValueRef anchor_to_location(const Anchor *anchor) {
        assert(use_debug_info);

        //auto old_bb = LLVMGetInsertBlock(builder);
        //LLVMValueRef func = LLVMGetBasicBlockParent(old_bb);
        LLVMValueRef disp = LLVMGetFunctionSubprogram(active_function_value);

        LLVMValueRef result = LLVMCreateDebugLocation(
            anchor->lineno, anchor->column, disp, nullptr);

        return result;
    }

    static void diag_handler(LLVMDiagnosticInfoRef info, void *) {
        const char *severity = "Message";
        switch(LLVMGetDiagInfoSeverity(info)) {
        case LLVMDSError: severity = "Error"; break;
        case LLVMDSWarning: severity = "Warning"; break;
        case LLVMDSRemark: return;// severity = "Remark"; break;
        case LLVMDSNote: return;//severity = "Note"; break;
        default: break;
        }

        char *str = LLVMGetDiagInfoDescription(info);
        fprintf(stderr, "LLVM %s: %s\n", severity, str);
        LLVMDisposeMessage(str);
        //LLVMDiagnosticSeverity LLVMGetDiagInfoSeverity(LLVMDiagnosticInfoRef DI);
    }

    LLVMValueRef get_intrinsic(Intrinsic op) {
        if (!intrinsics[op]) {
            LLVMValueRef result = nullptr;
            switch(op) {
#define LLVM_INTRINSIC_IMPL(ENUMVAL, RETTYPE, STRNAME, ...) \
    case ENUMVAL: { \
        LLVMTypeRef argtypes[] = {__VA_ARGS__}; \
        result = LLVMAddFunction(module, STRNAME, LLVMFunctionType(RETTYPE, argtypes, sizeof(argtypes) / sizeof(LLVMTypeRef), false)); \
    } break;
#define LLVM_INTRINSIC_IMPL_BEGIN(ENUMVAL, RETTYPE, STRNAME, ...) \
    case ENUMVAL: { \
        LLVMTypeRef argtypes[] = { __VA_ARGS__ }; \
        result = LLVMAddFunction(module, STRNAME, \
            LLVMFunctionType(f32T, argtypes, sizeof(argtypes) / sizeof(LLVMTypeRef), false)); \
        LLVMSetLinkage(result, LLVMPrivateLinkage); \
        auto bb = LLVMAppendBasicBlock(result, ""); \
        auto oldbb = LLVMGetInsertBlock(builder); \
        LLVMPositionBuilderAtEnd(builder, bb);
#define LLVM_INTRINSIC_IMPL_END() \
        LLVMPositionBuilderAtEnd(builder, oldbb); \
    } break;
            LLVM_INTRINSIC_IMPL(llvm_sin_f32, f32T, "llvm.sin.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_sin_f64, f64T, "llvm.sin.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_cos_f32, f32T, "llvm.cos.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_cos_f64, f64T, "llvm.cos.f64", f64T)

            LLVM_INTRINSIC_IMPL(llvm_sqrt_f32, f32T, "llvm.sqrt.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_sqrt_f64, f64T, "llvm.sqrt.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_fabs_f32, f32T, "llvm.fabs.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_fabs_f64, f64T, "llvm.fabs.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_trunc_f32, f32T, "llvm.trunc.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_trunc_f64, f64T, "llvm.trunc.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_floor_f32, f32T, "llvm.floor.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_floor_f64, f64T, "llvm.floor.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_pow_f32, f32T, "llvm.pow.f32", f32T, f32T)
            LLVM_INTRINSIC_IMPL(llvm_pow_f64, f64T, "llvm.pow.f64", f64T, f64T)
            LLVM_INTRINSIC_IMPL(llvm_exp_f32, f32T, "llvm.exp.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_exp_f64, f64T, "llvm.exp.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_log_f32, f32T, "llvm.log.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_log_f64, f64T, "llvm.log.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_exp2_f32, f32T, "llvm.exp2.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_exp2_f64, f64T, "llvm.exp2.f64", f64T)
            LLVM_INTRINSIC_IMPL(llvm_log2_f32, f32T, "llvm.log2.f32", f32T)
            LLVM_INTRINSIC_IMPL(llvm_log2_f64, f64T, "llvm.log2.f64", f64T)
            LLVM_INTRINSIC_IMPL_BEGIN(custom_fsign_f32, f32T, "custom.fsign.f32", f32T)
                // (0 < val) - (val < 0)
                LLVMValueRef val = LLVMGetParam(result, 0);
                LLVMValueRef zero = LLVMConstReal(f32T, 0.0);
                LLVMValueRef a = LLVMBuildZExt(builder, LLVMBuildFCmp(builder, LLVMRealOLT, zero, val, ""), i8T, "");
                LLVMValueRef b = LLVMBuildZExt(builder, LLVMBuildFCmp(builder, LLVMRealOLT, val, zero, ""), i8T, "");
                val = LLVMBuildSub(builder, a, b, "");
                val = LLVMBuildSIToFP(builder, val, f32T, "");
                LLVMBuildRet(builder, val);
            LLVM_INTRINSIC_IMPL_END()
            LLVM_INTRINSIC_IMPL_BEGIN(custom_fsign_f64, f64T, "custom.fsign.f64", f64T)
                // (0 < val) - (val < 0)
                LLVMValueRef val = LLVMGetParam(result, 0);
                LLVMValueRef zero = LLVMConstReal(f64T, 0.0);
                LLVMValueRef a = LLVMBuildZExt(builder, LLVMBuildFCmp(builder, LLVMRealOLT, zero, val, ""), i8T, "");
                LLVMValueRef b = LLVMBuildZExt(builder, LLVMBuildFCmp(builder, LLVMRealOLT, val, zero, ""), i8T, "");
                val = LLVMBuildSub(builder, a, b, "");
                val = LLVMBuildSIToFP(builder, val, f64T, "");
                LLVMBuildRet(builder, val);
            LLVM_INTRINSIC_IMPL_END()
#undef LLVM_INTRINSIC_IMPL
#undef LLVM_INTRINSIC_IMPL_BEGIN
#undef LLVM_INTRINSIC_IMPL_END
            default: assert(false); break;
            }
            intrinsics[op] = result;
        }
        return intrinsics[op];
    }

    static void static_init() {
        if (voidT) return;
        voidT = LLVMVoidType();
        i1T = LLVMInt1Type();
        i8T = LLVMInt8Type();
        i16T = LLVMInt16Type();
        i32T = LLVMInt32Type();
        i64T = LLVMInt64Type();
        f32T = LLVMFloatType();
        f32x2T = LLVMVectorType(f32T, 2);
        f64T = LLVMDoubleType();
        noneV = LLVMConstStruct(nullptr, 0, false);
        noneT = LLVMTypeOf(noneV);
        rawstringT = LLVMPointerType(LLVMInt8Type(), 0);
        attr_byval = get_attribute("byval");
        attr_sret = get_attribute("sret");
        attr_nonnull = get_attribute("nonnull");

        LLVMContextSetDiagnosticHandler(LLVMGetGlobalContext(),
            diag_handler,
            nullptr);

    }

#undef DEFINE_BUILTIN

    static bool all_parameters_lowered(Label *label) {
        for (auto &&param : label->params) {
            if (param->kind != PK_Regular)
                return false;
            if ((param->type == TYPE_Type) || (param->type == TYPE_Label))
                return false;
            if (isa<ReturnLabelType>(param->type) && (param->index != 0))
                return false;
        }
        return true;
    }

    static LLVMTypeRef abi_struct_type(const ABIClass *classes, size_t sz) {
        LLVMTypeRef types[sz];
        size_t k = 0;
        for (size_t i = 0; i < sz; ++i) {
            ABIClass cls = classes[i];
            switch(cls) {
            case ABI_CLASS_SSE: {
                types[i] = f32x2T; k++;
            } break;
            case ABI_CLASS_SSESF: {
                types[i] = f32T; k++;
            } break;
            case ABI_CLASS_SSEDF: {
                types[i] = f64T; k++;
            } break;
            case ABI_CLASS_INTEGER: {
                types[i] = i64T; k++;
            } break;
            case ABI_CLASS_INTEGERSI: {
                types[i] = i32T; k++;
            } break;
            case ABI_CLASS_INTEGERSI16: {
                types[i] = i16T; k++;
            } break;
            case ABI_CLASS_INTEGERSI8: {
                types[i] = i8T; k++;
            } break;
            default: {
                // do nothing
#if 0
                StyledStream ss;
                ss << "unhandled ABI class: " <<
                    abi_class_to_string(cls) << std::endl;
#endif
            } break;
            }
        }
        if (k != sz) return nullptr;
        return LLVMStructType(types, sz, false);
    }

    LLVMValueRef abi_import_argument(Parameter *param, LLVMValueRef func, size_t &k) {
        ABIClass classes[MAX_ABI_CLASSES];
        size_t sz = abi_classify(param->type, classes);
        if (!sz) {
            LLVMValueRef val = LLVMGetParam(func, k++);
            return LLVMBuildLoad(builder, val, "");
        }
        LLVMTypeRef T = type_to_llvm_type(param->type);
        auto tk = LLVMGetTypeKind(T);
        if (tk == LLVMStructTypeKind) {
            auto ST = abi_struct_type(classes, sz);
            if (ST) {
                // reassemble from argument-sized bits
                auto ptr = safe_alloca(ST);
                auto zero = LLVMConstInt(i32T,0,false);
                for (size_t i = 0; i < sz; ++i) {
                    LLVMValueRef indices[] = {
                        zero, LLVMConstInt(i32T,i,false),
                    };
                    auto dest = LLVMBuildGEP(builder, ptr, indices, 2, "");
                    LLVMBuildStore(builder, LLVMGetParam(func, k++), dest);
                }
                ptr = LLVMBuildBitCast(builder, ptr, LLVMPointerType(T, 0), "");
                return LLVMBuildLoad(builder, ptr, "");
            }
        }
        LLVMValueRef val = LLVMGetParam(func, k++);
        return val;
    }

    void abi_export_argument(LLVMValueRef val, const Type *AT,
        std::vector<LLVMValueRef> &values, std::vector<size_t> &memptrs) {
        ABIClass classes[MAX_ABI_CLASSES];
        size_t sz = abi_classify(AT, classes);
        if (!sz) {
            LLVMValueRef ptrval = safe_alloca(type_to_llvm_type(AT));
            LLVMBuildStore(builder, val, ptrval);
            val = ptrval;
            memptrs.push_back(values.size());
            values.push_back(val);
            return;
        }
        auto tk = LLVMGetTypeKind(LLVMTypeOf(val));
        if (tk == LLVMStructTypeKind) {
            auto ST = abi_struct_type(classes, sz);
            if (ST) {
                // break into argument-sized bits
                auto ptr = safe_alloca(LLVMTypeOf(val));
                auto zero = LLVMConstInt(i32T,0,false);
                LLVMBuildStore(builder, val, ptr);
                ptr = LLVMBuildBitCast(builder, ptr, LLVMPointerType(ST, 0), "");
                for (size_t i = 0; i < sz; ++i) {
                    LLVMValueRef indices[] = {
                        zero, LLVMConstInt(i32T,i,false),
                    };
                    auto val = LLVMBuildGEP(builder, ptr, indices, 2, "");
                    val = LLVMBuildLoad(builder, val, "");
                    values.push_back(val);
                }
                return;
            }
        }
        values.push_back(val);
    }

    static void abi_transform_parameter(const Type *AT,
        std::vector<LLVMTypeRef> &params) {
        ABIClass classes[MAX_ABI_CLASSES];
        size_t sz = abi_classify(AT, classes);
        auto T = type_to_llvm_type(AT);
        if (!sz) {
            params.push_back(LLVMPointerType(T, 0));
            return;
        }
        auto tk = LLVMGetTypeKind(T);
        if (tk == LLVMStructTypeKind) {
            auto ST = abi_struct_type(classes, sz);
            if (ST) {
                for (size_t i = 0; i < sz; ++i) {
                    params.push_back(LLVMStructGetTypeAtIndex(ST, i));
                }
                return;
            }
        }
        params.push_back(T);
    }

    static LLVMTypeRef create_llvm_type(const Type *type) {
        switch(type->kind()) {
        case TK_Integer:
            return LLVMIntType(cast<IntegerType>(type)->width);
        case TK_Real:
            switch(cast<RealType>(type)->width) {
            case 32: return f32T;
            case 64: return f64T;
            default: break;
            }
            break;
        case TK_Extern: {
            return LLVMPointerType(
                _type_to_llvm_type(cast<ExternType>(type)->type), 0);
        } break;
        case TK_Pointer:
            return LLVMPointerType(
                _type_to_llvm_type(cast<PointerType>(type)->element_type), 0);
        case TK_Array: {
            auto ai = cast<ArrayType>(type);
            return LLVMArrayType(_type_to_llvm_type(ai->element_type), ai->count);
        } break;
        case TK_Vector: {
            auto vi = cast<VectorType>(type);
            return LLVMVectorType(_type_to_llvm_type(vi->element_type), vi->count);
        } break;
        case TK_Tuple: {
            auto ti = cast<TupleType>(type);
            size_t count = ti->types.size();
            LLVMTypeRef elements[count];
            for (size_t i = 0; i < count; ++i) {
                elements[i] = _type_to_llvm_type(ti->types[i]);
            }
            return LLVMStructType(elements, count, ti->packed);
        } break;
        case TK_Union: {
            auto ui = cast<UnionType>(type);
            return _type_to_llvm_type(ui->tuple_type);
        } break;
        case TK_Typename: {
            if (type == TYPE_Void)
                return LLVMVoidType();
            else if (type == TYPE_Sampler) {
                location_error(String::from(
                    "sampler type can not be used for native target"));
            }
            auto tn = cast<TypenameType>(type);
            if (tn->finalized()) {
                switch(tn->storage_type->kind()) {
                case TK_Tuple:
                case TK_Union: {
                    type_todo.push_back(type);
                } break;
                default: {
                    return create_llvm_type(tn->storage_type);
                } break;
                }
            }
            return LLVMStructCreateNamed(
                LLVMGetGlobalContext(), type->name()->data);
        } break;
        case TK_ReturnLabel: {
            auto rlt = cast<ReturnLabelType>(type);
            return _type_to_llvm_type(rlt->return_type);
        } break;
        case TK_Function: {
            auto fi = cast<FunctionType>(type);
            size_t count = fi->argument_types.size();
            bool use_sret = is_memory_class(fi->return_type);

            std::vector<LLVMTypeRef> elements;
            elements.reserve(count);
            LLVMTypeRef rettype;
            if (use_sret) {
                elements.push_back(
                    LLVMPointerType(_type_to_llvm_type(fi->return_type), 0));
                rettype = voidT;
            } else {
                rettype = _type_to_llvm_type(fi->return_type);
            }
            for (size_t i = 0; i < count; ++i) {
                auto AT = fi->argument_types[i];
                abi_transform_parameter(AT, elements);
            }
            return LLVMFunctionType(rettype,
                &elements[0], elements.size(), fi->vararg());
        } break;
        case TK_SampledImage: {
            location_error(String::from(
                "sampled image type can not be used for native target"));
        } break;
        case TK_Image: {
            location_error(String::from(
                "image type can not be used for native target"));
        } break;
        };

        StyledString ss;
        ss.out << "IL->IR: cannot convert type " << type;
        location_error(ss.str());
        return nullptr;
    }

    static size_t finalize_types() {
        size_t result = type_todo.size();
        while (!type_todo.empty()) {
            const Type *T = type_todo.back();
            type_todo.pop_back();
            auto tn = cast<TypenameType>(T);
            if (!tn->finalized())
                continue;
            LLVMTypeRef LLT = _type_to_llvm_type(T);
            const Type *ST = tn->storage_type;
            switch(ST->kind()) {
            case TK_Tuple: {
                auto ti = cast<TupleType>(ST);
                size_t count = ti->types.size();
                LLVMTypeRef elements[count];
                for (size_t i = 0; i < count; ++i) {
                    elements[i] = _type_to_llvm_type(ti->types[i]);
                }
                LLVMStructSetBody(LLT, elements, count, false);
            } break;
            case TK_Union: {
                auto ui = cast<UnionType>(ST);
                size_t count = ui->types.size();
                size_t sz = ui->size;
                size_t al = ui->align;
                // find member with the same alignment
                for (size_t i = 0; i < count; ++i) {
                    const Type *ET = ui->types[i];
                    size_t etal = align_of(ET);
                    if (etal == al) {
                        size_t remsz = sz - size_of(ET);
                        LLVMTypeRef values[2];
                        values[0] = _type_to_llvm_type(ET);
                        if (remsz) {
                            // too small, add padding
                            values[1] = LLVMArrayType(i8T, remsz);
                            LLVMStructSetBody(LLT, values, 2, false);
                        } else {
                            LLVMStructSetBody(LLT, values, 1, false);
                        }
                        break;
                    }
                }
            } break;
            default: assert(false); break;
            }
        }
        return result;
    }

    static LLVMTypeRef _type_to_llvm_type(const Type *type) {
        auto it = type_cache.find(type);
        if (it == type_cache.end()) {
            LLVMTypeRef result = create_llvm_type(type);
            type_cache.insert({type, result});
            return result;
        } else {
            return it->second;
        }
    }

    static LLVMTypeRef type_to_llvm_type(const Type *type) {
        auto typeref = _type_to_llvm_type(type);
        finalize_types();
        return typeref;
    }

    LLVMValueRef label_to_value(Label *label) {
        if (label->is_basic_block_like()) {
            auto bb = label_to_basic_block(label);
            if (!bb) return nullptr;
            else
                return LLVMBasicBlockAsValue(bb);
        } else {
            return label_to_function(label);
        }
    }

    static void fatal_error_handler(const char *Reason) {
        location_error(String::from_cstr(Reason));
    }

    void bind_parameter(Parameter *param, LLVMValueRef value) {
        assert(value);
        param2value[{active_function_value, param}] = value;
    }

    bool parameter_is_bound(Parameter *param) {
        return param2value.find({active_function_value, param}) != param2value.end();
    }

    LLVMValueRef resolve_parameter(Parameter *param) {
        auto it = param2value.find({active_function_value, param});
        if (it == param2value.end()) {
            assert(active_function_value);
            if (param->label) {
                location_message(param->label->anchor, String::from("declared here"));
            }
            StyledString ss;
            ss.out << "IL->IR: can't access free variable " << param;
            location_error(ss.str());
        }
        assert(it->second);
        return it->second;
    }

    LLVMValueRef argument_to_value(Any value) {
        if (value.type == TYPE_Parameter) {
            return resolve_parameter(value.parameter);
        }
        switch(value.type->kind()) {
        case TK_Integer: {
            auto it = cast<IntegerType>(value.type);
            if (it->issigned) {
                switch(it->width) {
                case 8: return LLVMConstInt(i8T, value.i8, true);
                case 16: return LLVMConstInt(i16T, value.i16, true);
                case 32: return LLVMConstInt(i32T, value.i32, true);
                case 64: return LLVMConstInt(i64T, value.i64, true);
                default: break;
                }
            } else {
                switch(it->width) {
                case 1: return LLVMConstInt(i1T, value.i1, false);
                case 8: return LLVMConstInt(i8T, value.u8, false);
                case 16: return LLVMConstInt(i16T, value.u16, false);
                case 32: return LLVMConstInt(i32T, value.u32, false);
                case 64: return LLVMConstInt(i64T, value.u64, false);
                default: break;
                }
            }
        } break;
        case TK_Real: {
            auto rt = cast<RealType>(value.type);
            switch(rt->width) {
            case 32: return LLVMConstReal(f32T, value.f32);
            case 64: return LLVMConstReal(f64T, value.f64);
            default: break;
            }
        } break;
        case TK_Extern: {
            auto it = extern2global.find(value);
            if (it == extern2global.end()) {
                const String *namestr = value.symbol.name();
                const char *name = namestr->data;
                assert(name);
                auto et = cast<ExternType>(value.type);
                LLVMTypeRef LLT = type_to_llvm_type(et->type);
                LLVMValueRef result = nullptr;
                if ((namestr->count > 5) && !strncmp(name, "llvm.", 5)) {
                    result = LLVMAddFunction(module, name, LLT);
                } else {
                    void *pptr = local_aware_dlsym(name);
                    uint64_t ptr = *(uint64_t*)&pptr;
                    if (!ptr) {
                        LLVMInstallFatalErrorHandler(fatal_error_handler);
                        SCOPES_TRY()
                        ptr = LLVMGetGlobalValueAddress(ee, name);
                        SCOPES_CATCH(e)
                        (void)e; // shut up unused variable warning
                        SCOPES_TRY_END()
                        LLVMResetFatalErrorHandler();
                    }
                    if (!ptr) {
                        StyledString ss;
                        ss.out << "could not resolve " << value;
                        location_error(ss.str());
                    }
                    result = LLVMAddGlobal(module, LLT, name);
                }
                extern2global.insert({ value, result });
                return result;
            } else {
                return it->second;
            }
        } break;
        case TK_Pointer: {
            LLVMTypeRef LLT = type_to_llvm_type(value.type);
            if (!value.pointer) {
                return LLVMConstPointerNull(LLT);
            } else if (inline_pointers) {
                return LLVMConstIntToPtr(
                    LLVMConstInt(i64T, *(uint64_t*)&value.pointer, false),
                    LLT);
            } else {
                // to serialize a pointer, we serialize the allocation range
                // of the pointer as a global binary blob
                void *baseptr;
                size_t alloc_size;
                if (!find_allocation(value.pointer, baseptr, alloc_size)) {
                    StyledString ss;
                    ss.out << "IL->IR: constant pointer of type " << value.type
                        << " points to unserializable memory";
                    location_error(ss.str());
                }
                LLVMValueRef basevalue = nullptr;
                auto it = ptr2global.find(baseptr);

                auto pi = cast<PointerType>(value.type);
                bool writable = pi->is_writable();

                if (it == ptr2global.end()) {
                    auto data = LLVMConstString((const char *)baseptr, alloc_size, true);
                    basevalue = LLVMAddGlobal(module, LLVMTypeOf(data), "");
                    ptr2global.insert({ baseptr, basevalue });
                    LLVMSetInitializer(basevalue, data);
                    if (!writable) {
                        LLVMSetGlobalConstant(basevalue, true);
                    }
                } else {
                    basevalue = it->second;
                }
                size_t offset = (uint8_t*)value.pointer - (uint8_t*)baseptr;
                LLVMValueRef indices[2];
                indices[0] = LLVMConstInt(i64T, 0, false);
                indices[1] = LLVMConstInt(i64T, offset, false);
                return LLVMConstPointerCast(
                    LLVMConstGEP(basevalue, indices, 2), LLT);
            }
        } break;
        case TK_Typename: {
            LLVMTypeRef LLT = type_to_llvm_type(value.type);
            auto tn = cast<TypenameType>(value.type);
            switch(tn->storage_type->kind()) {
            case TK_Tuple: {
                auto ti = cast<TupleType>(tn->storage_type);
                size_t count = ti->types.size();
                LLVMValueRef values[count];
                for (size_t i = 0; i < count; ++i) {
                    values[i] = argument_to_value(ti->unpack(value.pointer, i));
                }
                return LLVMConstNamedStruct(LLT, values, count);
            } break;
            default: {
                Any storage_value = value;
                storage_value.type = tn->storage_type;
                LLVMValueRef val = argument_to_value(storage_value);
                return LLVMConstBitCast(val, LLT);
            } break;
            }
        } break;
        case TK_Array: {
            auto ai = cast<ArrayType>(value.type);
            size_t count = ai->count;
            LLVMValueRef values[count];
            for (size_t i = 0; i < count; ++i) {
                values[i] = argument_to_value(ai->unpack(value.pointer, i));
            }
            return LLVMConstArray(type_to_llvm_type(ai->element_type),
                values, count);
        } break;
        case TK_Vector: {
            auto vi = cast<VectorType>(value.type);
            size_t count = vi->count;
            LLVMValueRef values[count];
            for (size_t i = 0; i < count; ++i) {
                values[i] = argument_to_value(vi->unpack(value.pointer, i));
            }
            return LLVMConstVector(values, count);
        } break;
        case TK_Tuple: {
            auto ti = cast<TupleType>(value.type);
            size_t count = ti->types.size();
            LLVMValueRef values[count];
            for (size_t i = 0; i < count; ++i) {
                values[i] = argument_to_value(ti->unpack(value.pointer, i));
            }
            return LLVMConstStruct(values, count, false);
        } break;
        case TK_Union: {
            auto ui = cast<UnionType>(value.type);
            value.type = ui->tuple_type;
            return argument_to_value(value);
        } break;
        default: break;
        };

        StyledString ss;
        ss.out << "IL->IR: cannot convert argument of type " << value.type;
        location_error(ss.str());
        return nullptr;
    }

    struct ReturnTraits {
        bool multiple_return_values;
        bool terminated;
        const ReturnLabelType *rtype;

        ReturnTraits() :
            multiple_return_values(false),
            terminated(false),
            rtype(nullptr) {}
    };

    LLVMValueRef build_call(const Type *functype, LLVMValueRef func, Args &args,
        ReturnTraits &traits) {
        size_t argcount = args.size() - 1;

        auto fi = cast<FunctionType>(functype);

        bool use_sret = is_memory_class(fi->return_type);

        std::vector<LLVMValueRef> values;
        values.reserve(argcount + 1);

        if (use_sret) {
            values.push_back(safe_alloca(_type_to_llvm_type(fi->return_type)));
        }
        std::vector<size_t> memptrs;
        for (size_t i = 0; i < argcount; ++i) {
            auto &&arg = args[i + 1];
            LLVMValueRef val = argument_to_value(arg.value);
            auto AT = arg.value.indirect_type();
            abi_export_argument(val, AT, values, memptrs);
        }

        size_t fargcount = fi->argument_types.size();
        assert(argcount >= fargcount);
        // make variadic calls C compatible
        if (fi->flags & FF_Variadic) {
            for (size_t i = fargcount; i < argcount; ++i) {
                auto value = values[i];
                // floats need to be widened to doubles
                if (LLVMTypeOf(value) == f32T) {
                    values[i] = LLVMBuildFPExt(builder, value, f64T, "");
                }
            }
        }

        auto ret = LLVMBuildCall(builder, func, &values[0], values.size(), "");
        for (auto idx : memptrs) {
            auto i = idx + 1;
            LLVMAddCallSiteAttribute(ret, i, attr_nonnull);
        }
        auto rlt = cast<ReturnLabelType>(fi->return_type);
        traits.rtype = rlt;
        traits.multiple_return_values = rlt->has_multiple_return_values();
        if (use_sret) {
            LLVMAddCallSiteAttribute(ret, 1, attr_sret);
            return LLVMBuildLoad(builder, values[0], "");
        } else if (!rlt->is_returning()) {
            LLVMBuildUnreachable(builder);
            traits.terminated = true;
            return nullptr;
        } else if (rlt->return_type == TYPE_Void) {
            return nullptr;
        } else {
            return ret;
        }
    }

    LLVMValueRef set_debug_location(Label *label) {
        assert(use_debug_info);
        LLVMValueRef diloc = anchor_to_location(label->body.anchor);
        LLVMSetCurrentDebugLocation(builder, diloc);
        return diloc;
    }

    LLVMValueRef build_length_op(LLVMValueRef x) {
        auto T = LLVMTypeOf(x);
        auto ET = LLVMGetElementType(T);
        LLVMValueRef func_sqrt = get_intrinsic((ET == f64T)?llvm_sqrt_f64:llvm_sqrt_f32);
        assert(func_sqrt);
        auto count = LLVMGetVectorSize(T);
        LLVMValueRef src = LLVMBuildFMul(builder, x, x, "");
        LLVMValueRef retvalue = nullptr;
        for (unsigned i = 0; i < count; ++i) {
            LLVMValueRef idx = LLVMConstInt(i32T, i, false);
            LLVMValueRef val = LLVMBuildExtractElement(builder, src, idx, "");
            if (i == 0) {
                retvalue = val;
            } else {
                retvalue = LLVMBuildFAdd(builder, retvalue, val, "");
            }
        }
        LLVMValueRef values[] = { retvalue };
        return LLVMBuildCall(builder, func_sqrt, values, 1, "");
    }

    LLVMValueRef safe_alloca(LLVMTypeRef ty, LLVMValueRef val = nullptr) {
        if (val && !LLVMIsConstant(val)) {
            // for stack arrays with dynamic size, build the array locally
            return LLVMBuildArrayAlloca(builder, ty, val, "");
        } else {
#if 0
            // add allocas at the tail
            auto oldbb = LLVMGetInsertBlock(builder);
            auto entry = LLVMGetEntryBasicBlock(active_function_value);
            auto term = LLVMGetBasicBlockTerminator(entry);
            if (term) {
                LLVMPositionBuilderBefore(builder, term);
            } else {
                LLVMPositionBuilderAtEnd(builder, entry);
            }
            LLVMValueRef result;
            if (val) {
                result = LLVMBuildArrayAlloca(builder, ty, val, "");
            } else {
                result = LLVMBuildAlloca(builder, ty, "");
            }
            LLVMPositionBuilderAtEnd(builder, oldbb);
            return result;
#elif 1
            // add allocas to the front
            auto oldbb = LLVMGetInsertBlock(builder);
            auto entry = LLVMGetEntryBasicBlock(active_function_value);
            auto instr = LLVMGetFirstInstruction(entry);
            if (instr) {
                LLVMPositionBuilderBefore(builder, instr);
            } else {
                LLVMPositionBuilderAtEnd(builder, entry);
            }
            LLVMValueRef result;
            if (val) {
                result = LLVMBuildArrayAlloca(builder, ty, val, "");
            } else {
                result = LLVMBuildAlloca(builder, ty, "");
                //LLVMSetAlignment(result, 16);
            }
            LLVMPositionBuilderAtEnd(builder, oldbb);
            return result;
#else
            // add allocas locally
            LLVMValueRef result;
            if (val) {
                result = LLVMBuildArrayAlloca(builder, ty, val, "");
            } else {
                result = LLVMBuildAlloca(builder, ty, "");
            }
            return result;
#endif
        }
    }

    LLVMValueRef build_matching_constant_real_vector(LLVMValueRef value, double c) {
        auto T = LLVMTypeOf(value);
        if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
            unsigned count = LLVMGetVectorSize(T);
            auto ET = LLVMGetElementType(T);
            LLVMValueRef one = LLVMConstReal(ET, c);
            LLVMValueRef values[count];
            for (unsigned i = 0; i < count; ++i) {
                values[i] = one;
            }
            return LLVMConstVector(values, count);
        } else {
            return LLVMConstReal(T, c);
        }
    }

    void write_label_body(Label *label) {
    repeat:
        if (!label->body.is_complete()) {
            set_active_anchor(label->body.anchor);
            location_error(String::from("IL->IR: incomplete label body encountered"));
        }
#if SCOPES_DEBUG_CODEGEN
        {
            StyledStream ss(std::cout);
            std::cout << "generating LLVM for label:" << std::endl;
            stream_label(ss, label, StreamLabelFormat::debug_single());
            std::cout << std::endl;
        }
#endif
        auto &&body = label->body;
        auto &&enter = body.enter;
        auto &&args = body.args;

        set_active_anchor(label->body.anchor);

        LLVMValueRef diloc = nullptr;
        if (use_debug_info) {
            diloc = set_debug_location(label);
        }

        assert(!args.empty());
        size_t argcount = args.size() - 1;
        size_t argn = 1;
#define READ_ANY(NAME) \
        assert(argn <= argcount); \
        Any &NAME = args[argn++].value;
#define READ_VALUE(NAME) \
        assert(argn <= argcount); \
        LLVMValueRef NAME = argument_to_value(args[argn++].value);
#define READ_LABEL_VALUE(NAME) \
        assert(argn <= argcount); \
        LLVMValueRef NAME = label_to_value(args[argn++].value); \
        assert(NAME);
#define READ_TYPE(NAME) \
        assert(argn <= argcount); \
        assert(args[argn].value.type == TYPE_Type); \
        LLVMTypeRef NAME = type_to_llvm_type(args[argn++].value.typeref);

        LLVMValueRef retvalue = nullptr;
        ReturnTraits rtraits;
        if (enter.type == TYPE_Builtin) {
            switch(enter.builtin.value()) {
            case FN_Branch: {
                READ_VALUE(cond);
                READ_LABEL_VALUE(then_block);
                READ_LABEL_VALUE(else_block);
                assert(LLVMValueIsBasicBlock(then_block));
                assert(LLVMValueIsBasicBlock(else_block));
                LLVMBuildCondBr(builder, cond,
                    LLVMValueAsBasicBlock(then_block),
                    LLVMValueAsBasicBlock(else_block));
                rtraits.terminated = true;
            } break;
            case OP_Tertiary: {
                READ_VALUE(cond);
                READ_VALUE(then_value);
                READ_VALUE(else_value);
                retvalue = LLVMBuildSelect(
                    builder, cond, then_value, else_value, "");
            } break;
            case FN_Unconst: {
                READ_ANY(val);
                if (val.type == TYPE_Label) {
                    retvalue = label_to_function(val);
                } else {
                    retvalue = argument_to_value(val);
                }
            } break;
            case FN_ExtractValue: {
                READ_VALUE(val);
                READ_ANY(index);
                retvalue = LLVMBuildExtractValue(
                    builder, val, cast_number<int32_t>(index), "");
            } break;
            case FN_InsertValue: {
                READ_VALUE(val);
                READ_VALUE(eltval);
                READ_ANY(index);
                retvalue = LLVMBuildInsertValue(
                    builder, val, eltval, cast_number<int32_t>(index), "");
            } break;
            case FN_ExtractElement: {
                READ_VALUE(val);
                READ_VALUE(index);
                retvalue = LLVMBuildExtractElement(builder, val, index, "");
            } break;
            case FN_InsertElement: {
                READ_VALUE(val);
                READ_VALUE(eltval);
                READ_VALUE(index);
                retvalue = LLVMBuildInsertElement(builder, val, eltval, index, "");
            } break;
            case FN_ShuffleVector: {
                READ_VALUE(v1);
                READ_VALUE(v2);
                READ_VALUE(mask);
                retvalue = LLVMBuildShuffleVector(builder, v1, v2, mask, "");
            } break;
            case FN_Undef: { READ_TYPE(ty);
                retvalue = LLVMGetUndef(ty); } break;
            case FN_Alloca: { READ_TYPE(ty);
                retvalue = safe_alloca(ty);
            } break;
            case FN_AllocaExceptionPad: {
                LLVMTypeRef ty = type_to_llvm_type(Array(TYPE_U8, sizeof(ExceptionPad)));
#ifdef SCOPES_WIN32
                retvalue = LLVMBuildAlloca(builder, ty, "");
#else
                retvalue = safe_alloca(ty);
#endif
            } break;
            case FN_AllocaArray: { READ_TYPE(ty); READ_VALUE(val);
                retvalue = safe_alloca(ty, val); } break;
            case FN_AllocaOf: {
                READ_VALUE(val);
                retvalue = safe_alloca(LLVMTypeOf(val));
                LLVMBuildStore(builder, val, retvalue);
            } break;
            case FN_Malloc: { READ_TYPE(ty);
                retvalue = LLVMBuildMalloc(builder, ty, ""); } break;
            case FN_MallocArray: { READ_TYPE(ty); READ_VALUE(val);
                retvalue = LLVMBuildArrayMalloc(builder, ty, val, ""); } break;
            case FN_Free: { READ_VALUE(val);
                LLVMBuildFree(builder, val);
                retvalue = nullptr; } break;
            case FN_GetElementPtr: {
                READ_VALUE(pointer);
                assert(argcount > 1);
                size_t count = argcount - 1;
                LLVMValueRef indices[count];
                for (size_t i = 0; i < count; ++i) {
                    indices[i] = argument_to_value(args[argn + i].value);
                }
                retvalue = LLVMBuildGEP(builder, pointer, indices, count, "");
            } break;
            case FN_Bitcast: { READ_VALUE(val); READ_TYPE(ty);
                auto T = LLVMTypeOf(val);
                if (T == ty) {
                    retvalue = val;
                } else if (LLVMGetTypeKind(ty) == LLVMStructTypeKind) {
                    // completely braindead, but what can you do
                    LLVMValueRef ptr = safe_alloca(T);
                    LLVMBuildStore(builder, val, ptr);
                    ptr = LLVMBuildBitCast(builder, ptr, LLVMPointerType(ty,0), "");
                    retvalue = LLVMBuildLoad(builder, ptr, "");
                } else {
                    retvalue = LLVMBuildBitCast(builder, val, ty, "");
                }
            } break;
            case FN_IntToPtr: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildIntToPtr(builder, val, ty, ""); } break;
            case FN_PtrToInt: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildPtrToInt(builder, val, ty, ""); } break;
            case FN_ITrunc: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildTrunc(builder, val, ty, ""); } break;
            case FN_SExt: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildSExt(builder, val, ty, ""); } break;
            case FN_ZExt: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildZExt(builder, val, ty, ""); } break;
            case FN_FPTrunc: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildFPTrunc(builder, val, ty, ""); } break;
            case FN_FPExt: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildFPExt(builder, val, ty, ""); } break;
            case FN_FPToUI: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildFPToUI(builder, val, ty, ""); } break;
            case FN_FPToSI: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildFPToSI(builder, val, ty, ""); } break;
            case FN_UIToFP: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildUIToFP(builder, val, ty, ""); } break;
            case FN_SIToFP: { READ_VALUE(val); READ_TYPE(ty);
                retvalue = LLVMBuildSIToFP(builder, val, ty, ""); } break;
            case FN_VolatileLoad:
            case FN_Load: { READ_VALUE(ptr);
                retvalue = LLVMBuildLoad(builder, ptr, "");
                if (enter.builtin.value() == FN_VolatileLoad) { LLVMSetVolatile(retvalue, true); }
            } break;
            case FN_VolatileStore:
            case FN_Store: { READ_VALUE(val); READ_VALUE(ptr);
                retvalue = LLVMBuildStore(builder, val, ptr);
                if (enter.builtin.value() == FN_VolatileStore) { LLVMSetVolatile(retvalue, true); }
                retvalue = nullptr;
            } break;
            case OP_ICmpEQ:
            case OP_ICmpNE:
            case OP_ICmpUGT:
            case OP_ICmpUGE:
            case OP_ICmpULT:
            case OP_ICmpULE:
            case OP_ICmpSGT:
            case OP_ICmpSGE:
            case OP_ICmpSLT:
            case OP_ICmpSLE: {
                READ_VALUE(a); READ_VALUE(b);
                LLVMIntPredicate pred = LLVMIntEQ;
                switch(enter.builtin.value()) {
                    case OP_ICmpEQ: pred = LLVMIntEQ; break;
                    case OP_ICmpNE: pred = LLVMIntNE; break;
                    case OP_ICmpUGT: pred = LLVMIntUGT; break;
                    case OP_ICmpUGE: pred = LLVMIntUGE; break;
                    case OP_ICmpULT: pred = LLVMIntULT; break;
                    case OP_ICmpULE: pred = LLVMIntULE; break;
                    case OP_ICmpSGT: pred = LLVMIntSGT; break;
                    case OP_ICmpSGE: pred = LLVMIntSGE; break;
                    case OP_ICmpSLT: pred = LLVMIntSLT; break;
                    case OP_ICmpSLE: pred = LLVMIntSLE; break;
                    default: assert(false); break;
                }
                retvalue = LLVMBuildICmp(builder, pred, a, b, "");
            } break;
            case OP_FCmpOEQ:
            case OP_FCmpONE:
            case OP_FCmpORD:
            case OP_FCmpOGT:
            case OP_FCmpOGE:
            case OP_FCmpOLT:
            case OP_FCmpOLE:
            case OP_FCmpUEQ:
            case OP_FCmpUNE:
            case OP_FCmpUNO:
            case OP_FCmpUGT:
            case OP_FCmpUGE:
            case OP_FCmpULT:
            case OP_FCmpULE: {
                READ_VALUE(a); READ_VALUE(b);
                LLVMRealPredicate pred = LLVMRealOEQ;
                switch(enter.builtin.value()) {
                    case OP_FCmpOEQ: pred = LLVMRealOEQ; break;
                    case OP_FCmpONE: pred = LLVMRealONE; break;
                    case OP_FCmpORD: pred = LLVMRealORD; break;
                    case OP_FCmpOGT: pred = LLVMRealOGT; break;
                    case OP_FCmpOGE: pred = LLVMRealOGE; break;
                    case OP_FCmpOLT: pred = LLVMRealOLT; break;
                    case OP_FCmpOLE: pred = LLVMRealOLE; break;
                    case OP_FCmpUEQ: pred = LLVMRealUEQ; break;
                    case OP_FCmpUNE: pred = LLVMRealUNE; break;
                    case OP_FCmpUNO: pred = LLVMRealUNO; break;
                    case OP_FCmpUGT: pred = LLVMRealUGT; break;
                    case OP_FCmpUGE: pred = LLVMRealUGE; break;
                    case OP_FCmpULT: pred = LLVMRealULT; break;
                    case OP_FCmpULE: pred = LLVMRealULE; break;
                    default: assert(false); break;
                }
                retvalue = LLVMBuildFCmp(builder, pred, a, b, "");
            } break;
            case OP_Add: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildAdd(builder, a, b, ""); } break;
            case OP_AddNUW: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildNUWAdd(builder, a, b, ""); } break;
            case OP_AddNSW: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildNSWAdd(builder, a, b, ""); } break;
            case OP_Sub: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildSub(builder, a, b, ""); } break;
            case OP_SubNUW: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildNUWSub(builder, a, b, ""); } break;
            case OP_SubNSW: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildNSWSub(builder, a, b, ""); } break;
            case OP_Mul: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildMul(builder, a, b, ""); } break;
            case OP_MulNUW: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildNUWMul(builder, a, b, ""); } break;
            case OP_MulNSW: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildNSWMul(builder, a, b, ""); } break;
            case OP_SDiv: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildSDiv(builder, a, b, ""); } break;
            case OP_UDiv: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildUDiv(builder, a, b, ""); } break;
            case OP_SRem: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildSRem(builder, a, b, ""); } break;
            case OP_URem: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildURem(builder, a, b, ""); } break;
            case OP_Shl: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildShl(builder, a, b, ""); } break;
            case OP_LShr: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildLShr(builder, a, b, ""); } break;
            case OP_AShr: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildAShr(builder, a, b, ""); } break;
            case OP_BAnd: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildAnd(builder, a, b, ""); } break;
            case OP_BOr: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildOr(builder, a, b, ""); } break;
            case OP_BXor: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildXor(builder, a, b, ""); } break;
            case OP_FAdd: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildFAdd(builder, a, b, ""); } break;
            case OP_FSub: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildFSub(builder, a, b, ""); } break;
            case OP_FMul: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildFMul(builder, a, b, ""); } break;
            case OP_FDiv: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildFDiv(builder, a, b, ""); } break;
            case OP_FRem: { READ_VALUE(a); READ_VALUE(b);
                retvalue = LLVMBuildFRem(builder, a, b, ""); } break;
            case OP_FMix: {
                READ_VALUE(a);
                READ_VALUE(b);
                READ_VALUE(x);
                LLVMValueRef one = build_matching_constant_real_vector(a, 1.0);
                auto invx = LLVMBuildFSub(builder, one, x, "");
                retvalue = LLVMBuildFAdd(builder,
                    LLVMBuildFMul(builder, a, invx, ""),
                    LLVMBuildFMul(builder, b, x, ""),
                    "");
            } break;
            case FN_Length: {
                READ_VALUE(x);
                auto T = LLVMTypeOf(x);
                if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                    retvalue = build_length_op(x);
                } else {
                    LLVMValueRef func_fabs = get_intrinsic((T == f64T)?llvm_fabs_f64:llvm_fabs_f32);
                    assert(func_fabs);
                    LLVMValueRef values[] = { x };
                    retvalue = LLVMBuildCall(builder, func_fabs, values, 1, "");
                }
            } break;
            case FN_Normalize: {
                READ_VALUE(x);
                auto T = LLVMTypeOf(x);
                if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                    auto count = LLVMGetVectorSize(T);
                    auto ET = LLVMGetElementType(T);
                    LLVMValueRef l = build_length_op(x);
                    l = LLVMBuildInsertElement(builder,
                        LLVMGetUndef(LLVMVectorType(ET, 1)), l,
                        LLVMConstInt(i32T, 0, false),
                        "");
                    LLVMValueRef mask[count];
                    for (int i = 0; i < count; ++i) {
                        mask[i] = 0;
                    }
                    l = LLVMBuildShuffleVector(builder, l, l,
                        LLVMConstNull(LLVMVectorType(i32T, count)), "");
                    retvalue = LLVMBuildFDiv(builder, x, l, "");
                } else {
                    retvalue = LLVMConstReal(T, 1.0);
                }
            } break;
            case FN_Cross: {
                READ_VALUE(a);
                READ_VALUE(b);
                auto T = LLVMTypeOf(a);
                assert (LLVMGetTypeKind(T) == LLVMVectorTypeKind);
                LLVMValueRef i0 = LLVMConstInt(i32T, 0, false);
                LLVMValueRef i1 = LLVMConstInt(i32T, 1, false);
                LLVMValueRef i2 = LLVMConstInt(i32T, 2, false);
                LLVMValueRef i120[] = { i1, i2, i0 };
                LLVMValueRef v120 = LLVMConstVector(i120, 3);
                LLVMValueRef a120 = LLVMBuildShuffleVector(builder, a, a, v120, "");
                LLVMValueRef b120 = LLVMBuildShuffleVector(builder, b, b, v120, "");
                retvalue = LLVMBuildFSub(builder,
                    LLVMBuildFMul(builder, a, b120, ""),
                    LLVMBuildFMul(builder, b, a120, ""), "");
                retvalue = LLVMBuildShuffleVector(builder, retvalue, retvalue, v120, "");
            } break;
            case OP_Step: {
                // select (lhs > rhs) (T 0) (T 1)
                READ_VALUE(a);
                READ_VALUE(b);
                LLVMValueRef one = build_matching_constant_real_vector(a, 1.0);
                LLVMValueRef zero = build_matching_constant_real_vector(b, 0.0);
                retvalue = LLVMBuildSelect(
                    builder,
                    LLVMBuildFCmp(builder, LLVMRealOGT, a, b, ""),
                    zero, one, "");
            } break;
            // binops
            case OP_Pow: {
                READ_VALUE(a);
                READ_VALUE(b);
                auto T = LLVMTypeOf(a);
                auto ET = T;
                if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                    ET = LLVMGetElementType(T);
                }
                LLVMValueRef func = nullptr;
                Intrinsic op = NumIntrinsics;
                switch(enter.builtin.value()) {
                case OP_Pow: { op = (ET == f64T)?llvm_pow_f64:llvm_pow_f32; } break;
                default: break;
                }
                func = get_intrinsic(op);
                assert(func);
                if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                    auto count = LLVMGetVectorSize(T);
                    retvalue = LLVMGetUndef(T);
                    for (unsigned i = 0; i < count; ++i) {
                        LLVMValueRef idx = LLVMConstInt(i32T, i, false);
                        LLVMValueRef values[] = {
                            LLVMBuildExtractElement(builder, a, idx, ""),
                            LLVMBuildExtractElement(builder, b, idx, "")
                        };
                        LLVMValueRef eltval = LLVMBuildCall(builder, func, values, 2, "");
                        retvalue = LLVMBuildInsertElement(builder, retvalue, eltval, idx, "");
                    }
                } else {
                    LLVMValueRef values[] = { a, b };
                    retvalue = LLVMBuildCall(builder, func, values, 2, "");
                }
            } break;
            // unops
            case OP_Sin:
            case OP_Cos:
            case OP_Sqrt:
            case OP_FAbs:
            case OP_FSign:
            case OP_Trunc:
            case OP_Exp:
            case OP_Log:
            case OP_Exp2:
            case OP_Log2:
            case OP_Floor: { READ_VALUE(x);
                auto T = LLVMTypeOf(x);
                auto ET = T;
                if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                    ET = LLVMGetElementType(T);
                }
                LLVMValueRef func = nullptr;
                Intrinsic op = NumIntrinsics;
                switch(enter.builtin.value()) {
                case OP_Sin: { op = (ET == f64T)?llvm_sin_f64:llvm_sin_f32; } break;
                case OP_Cos: { op = (ET == f64T)?llvm_cos_f64:llvm_cos_f32; } break;
                case OP_Sqrt: { op = (ET == f64T)?llvm_sqrt_f64:llvm_sqrt_f32; } break;
                case OP_FAbs: { op = (ET == f64T)?llvm_fabs_f64:llvm_fabs_f32; } break;
                case OP_Trunc: { op = (ET == f64T)?llvm_trunc_f64:llvm_trunc_f32; } break;
                case OP_Floor: { op = (ET == f64T)?llvm_floor_f64:llvm_floor_f32; } break;
                case OP_Exp: { op = (ET == f64T)?llvm_exp_f64:llvm_exp_f32; } break;
                case OP_Log: { op = (ET == f64T)?llvm_log_f64:llvm_log_f32; } break;
                case OP_Exp2: { op = (ET == f64T)?llvm_exp2_f64:llvm_exp2_f32; } break;
                case OP_Log2: { op = (ET == f64T)?llvm_log2_f64:llvm_log2_f32; } break;
                case OP_FSign: { op = (ET == f64T)?custom_fsign_f64:custom_fsign_f32; } break;
                default: break;
                }
                func = get_intrinsic(op);
                assert(func);
                if (LLVMGetTypeKind(T) == LLVMVectorTypeKind) {
                    auto count = LLVMGetVectorSize(T);
                    retvalue = LLVMGetUndef(T);
                    for (unsigned i = 0; i < count; ++i) {
                        LLVMValueRef idx = LLVMConstInt(i32T, i, false);
                        LLVMValueRef values[] = { LLVMBuildExtractElement(builder, x, idx, "") };
                        LLVMValueRef eltval = LLVMBuildCall(builder, func, values, 1, "");
                        retvalue = LLVMBuildInsertElement(builder, retvalue, eltval, idx, "");
                    }
                } else {
                    LLVMValueRef values[] = { x };
                    retvalue = LLVMBuildCall(builder, func, values, 1, "");
                }
            } break;
            case SFXFN_Unreachable:
                retvalue = LLVMBuildUnreachable(builder);
                rtraits.terminated = true;
                break;
            default: {
                StyledString ss;
                ss.out << "IL->IR: unsupported builtin " << enter.builtin << " encountered";
                location_error(ss.str());
            } break;
            }
        } else if (enter.type == TYPE_Label) {
            LLVMValueRef value = label_to_value(enter);
            if (!value) {
                // no basic block was generated - just generate assignments
                auto &&params = enter.label->params;
                for (size_t i = 1; i <= argcount; ++i) {
                    if (i < params.size()) {
                        bind_parameter(params[i], argument_to_value(args[i].value));
                    }
                }
                label = enter.label;
                goto repeat;
            } else if (LLVMValueIsBasicBlock(value)) {
                LLVMValueRef values[argcount];
                for (size_t i = 0; i < argcount; ++i) {
                    values[i] = argument_to_value(args[i + 1].value);
                }
                auto bbfrom = LLVMGetInsertBlock(builder);
                // assign phi nodes
                auto &&params = enter.label->params;
                LLVMBasicBlockRef incobbs[] = { bbfrom };
                for (size_t i = 1; i < params.size(); ++i) {
                    Parameter *param = params[i];
                    LLVMValueRef phinode = argument_to_value(param);
                    LLVMValueRef incovals[] = { values[i - 1] };
                    LLVMAddIncoming(phinode, incovals, incobbs, 1);
                }
                LLVMBuildBr(builder, LLVMValueAsBasicBlock(value));
                rtraits.terminated = true;
            } else {
                if (use_debug_info) {
                    LLVMSetCurrentDebugLocation(builder, diloc);
                }
                retvalue = build_call(
                    enter.label->get_function_type(),
                    value, args, rtraits);
            }
        } else if (enter.type == TYPE_Closure) {
            StyledString ss;
            ss.out << "IL->IR: invalid call of compile time closure at runtime";
            location_error(ss.str());
        } else if (is_function_pointer(enter.indirect_type())) {
            retvalue = build_call(extract_function_type(enter.indirect_type()),
                argument_to_value(enter), args, rtraits);
        } else if (enter.type == TYPE_Parameter) {
            assert (enter.parameter->type != TYPE_Nothing);
            assert(enter.parameter->type != TYPE_Unknown);
            LLVMValueRef values[argcount];
            for (size_t i = 0; i < argcount; ++i) {
                values[i] = argument_to_value(args[i + 1].value);
            }
            // must be a return
            assert(enter.parameter->index == 0);
            // must be returning from this function
            assert(enter.parameter->label == active_function);

            Label *label = enter.parameter->label;
            bool use_sret = is_memory_class(label->get_return_type());
            if (use_sret) {
                auto pval = resolve_parameter(enter.parameter);
                if (argcount > 1) {
                    LLVMTypeRef types[argcount];
                    for (size_t i = 0; i < argcount; ++i) {
                        types[i] = LLVMTypeOf(values[i]);
                    }

                    LLVMValueRef val = LLVMGetUndef(LLVMStructType(types, argcount, false));
                    for (size_t i = 0; i < argcount; ++i) {
                        val = LLVMBuildInsertValue(builder, val, values[i], i, "");
                    }
                    LLVMBuildStore(builder, val, pval);
                } else if (argcount == 1) {
                    LLVMBuildStore(builder, values[0], pval);
                }
                LLVMBuildRetVoid(builder);
            } else {
                if (argcount > 1) {
                    LLVMBuildAggregateRet(builder, values, argcount);
                } else if (argcount == 1) {
                    LLVMBuildRet(builder, values[0]);
                } else {
                    LLVMBuildRetVoid(builder);
                }
            }
            rtraits.terminated = true;
        } else {
            StyledString ss;
            ss.out << "IL->IR: cannot translate call to " << enter;
            location_error(ss.str());
        }

        Any contarg = args[0].value;
        if (rtraits.terminated) {
            // write nothing
        } else if ((contarg.type == TYPE_Parameter)
            && (contarg.parameter->type != TYPE_Nothing)) {
            assert(contarg.parameter->type != TYPE_Unknown);
            assert(contarg.parameter->index == 0);
            assert(contarg.parameter->label == active_function);
            Label *label = contarg.parameter->label;
            bool use_sret = is_memory_class(label->get_return_type());
            if (use_sret) {
                auto pval = resolve_parameter(contarg.parameter);
                if (retvalue) {
                    LLVMBuildStore(builder, retvalue, pval);
                }
                LLVMBuildRetVoid(builder);
            } else {
                if (retvalue) {
                    LLVMBuildRet(builder, retvalue);
                } else {
                    LLVMBuildRetVoid(builder);
                }
            }
        } else if (contarg.type == TYPE_Label) {
            auto bb = label_to_basic_block(contarg.label);
            if (bb) {
                if (retvalue) {
                    auto bbfrom = LLVMGetInsertBlock(builder);
                    LLVMBasicBlockRef incobbs[] = { bbfrom };

#define UNPACK_RET_ARGS() \
    if (rtraits.multiple_return_values) { \
        assert(rtraits.rtype); \
        auto &&values = rtraits.rtype->values; \
        auto &&params = contarg.label->params; \
        size_t pi = 1; \
        for (size_t i = 0; i < values.size(); ++i) { \
            if (pi >= params.size()) \
                break; \
            Parameter *param = params[pi]; \
            auto &&arg = values[i]; \
            if (is_unknown(arg.value)) { \
                LLVMValueRef incoval = LLVMBuildExtractValue(builder, retvalue, i, ""); \
                T(param, incoval); \
                pi++; \
            } \
        } \
    } else { \
        auto &&params = contarg.label->params; \
        if (params.size() > 1) { \
            assert(params.size() == 2); \
            Parameter *param = params[1]; \
            T(param, retvalue); \
        } \
    }
                    #define T(PARAM, VALUE) \
                        LLVMAddIncoming(argument_to_value(PARAM), &VALUE, incobbs, 1);
                    UNPACK_RET_ARGS()
                    #undef T
                }

                LLVMBuildBr(builder, bb);
            } else {
                if (retvalue) {
                    #define T(PARAM, VALUE) \
                        bind_parameter(PARAM, VALUE);
                    UNPACK_RET_ARGS()
                    #undef T
                }
                label = contarg.label;
                goto repeat;
            }
#undef UNPACK_RET_ARGS
        } else if (contarg.type == TYPE_Nothing) {
            StyledStream ss(std::cerr);
            stream_label(ss, label, StreamLabelFormat::debug_single());
            location_error(String::from("IL->IR: unexpected end of function"));
        } else {
            StyledStream ss(std::cerr);
            stream_label(ss, label, StreamLabelFormat::debug_single());
            location_error(String::from("IL->IR: continuation is of invalid type"));
        }

        LLVMSetCurrentDebugLocation(builder, nullptr);

    }
#undef READ_ANY
#undef READ_VALUE
#undef READ_TYPE
#undef READ_LABEL_VALUE

    void set_active_function(Label *l) {
        if (active_function == l) return;
        active_function = l;
        if (l) {
            auto it = label2func.find(l);
            assert(it != label2func.end());
            active_function_value = it->second;
        } else {
            active_function_value = nullptr;
        }
    }

    void process_labels() {
        while (!bb_label_todo.empty()) {
            auto it = bb_label_todo.back();
            set_active_function(it.first);
            Label *label = it.second;
            bb_label_todo.pop_back();

            auto it2 = label2bb.find({active_function_value, label});
            assert(it2 != label2bb.end());
            LLVMBasicBlockRef bb = it2->second;
            LLVMPositionBuilderAtEnd(builder, bb);

            write_label_body(label);
        }
    }

    bool has_single_caller(Label *l) {
        auto it = user_map.label_map.find(l);
        assert(it != user_map.label_map.end());
        auto &&users = it->second;
        if (users.size() != 1)
            return false;
        Label *userl = *users.begin();
        if (userl->body.enter == Any(l))
            return true;
        if (userl->body.args[0] == Any(l))
            return true;
        return false;
    }

    LLVMBasicBlockRef label_to_basic_block(Label *label) {
        auto old_bb = LLVMGetInsertBlock(builder);
        LLVMValueRef func = LLVMGetBasicBlockParent(old_bb);
        auto it = label2bb.find({func, label});
        if (it == label2bb.end()) {
            if (has_single_caller(label)) {
                // not generating basic blocks for single user labels
                label2bb.insert({{func, label}, nullptr});
                return nullptr;
            }
            const char *name = label->name.name()->data;
            auto bb = LLVMAppendBasicBlock(func, name);
            label2bb.insert({{func, label}, bb});
            bb_label_todo.push_back({active_function, label});
            LLVMPositionBuilderAtEnd(builder, bb);

            auto &&params = label->params;
            if (!params.empty()) {
                size_t paramcount = label->params.size() - 1;
                for (size_t i = 0; i < paramcount; ++i) {
                    Parameter *param = params[i + 1];
                    auto pvalue = LLVMBuildPhi(builder,
                        type_to_llvm_type(param->type),
                        param->name.name()->data);
                    bind_parameter(param, pvalue);
                }
            }

            LLVMPositionBuilderAtEnd(builder, old_bb);
            return bb;
        } else {
            return it->second;
        }
    }

    LLVMValueRef label_to_function(Label *label,
        bool root_function = false,
        Symbol funcname = SYM_Unnamed) {
        auto it = label2func.find(label);
        if (it == label2func.end()) {

            const Anchor *old_anchor = get_active_anchor();
            set_active_anchor(label->anchor);
            Label *last_function = active_function;

            auto old_bb = LLVMGetInsertBlock(builder);

            if (funcname == SYM_Unnamed) {
                funcname = label->name;
            }

            const char *name;
            if (root_function && (funcname == SYM_Unnamed)) {
                name = "unnamed";
            } else {
                name = funcname.name()->data;
            }

            label->verify_compilable();
            auto ilfunctype = label->get_function_type();
            auto fi = cast<FunctionType>(ilfunctype);
            bool use_sret = is_memory_class(fi->return_type);

            auto functype = type_to_llvm_type(ilfunctype);

            auto func = LLVMAddFunction(module, name, functype);
            if (use_debug_info) {
                LLVMSetFunctionSubprogram(func, label_to_subprogram(label));
            }
            LLVMSetLinkage(func, LLVMPrivateLinkage);
            label2func[label] = func;
            set_active_function(label);

            auto bb = LLVMAppendBasicBlock(func, "");
            LLVMPositionBuilderAtEnd(builder, bb);

            auto &&params = label->params;
            size_t offset = 0;
            if (use_sret) {
                offset++;
                Parameter *param = params[0];
                bind_parameter(param, LLVMGetParam(func, 0));
            }

            size_t paramcount = params.size() - 1;

            if (use_debug_info)
                set_debug_location(label);
            size_t k = offset;
            for (size_t i = 0; i < paramcount; ++i) {
                Parameter *param = params[i + 1];
                LLVMValueRef val = abi_import_argument(param, func, k);
                bind_parameter(param, val);
            }

            write_label_body(label);

            LLVMPositionBuilderAtEnd(builder, old_bb);

            set_active_function(last_function);
            set_active_anchor(old_anchor);
            return func;
        } else {
            return it->second;
        }
    }

    void setup_generate(const char *module_name) {
        module = LLVMModuleCreateWithName(module_name);
        builder = LLVMCreateBuilder();
        di_builder = LLVMCreateDIBuilder(module);

        if (use_debug_info) {
            const char *DebugStr = "Debug Info Version";
            LLVMValueRef DbgVer[3];
            DbgVer[0] = LLVMConstInt(i32T, 1, 0);
            DbgVer[1] = LLVMMDString(DebugStr, strlen(DebugStr));
            DbgVer[2] = LLVMConstInt(i32T, 3, 0);
            LLVMAddNamedMetadataOperand(module, "llvm.module.flags",
                LLVMMDNode(DbgVer, 3));

            LLVMDIBuilderCreateCompileUnit(di_builder,
                llvm::dwarf::DW_LANG_C99, "file", "directory", "scopes",
                false, "", 0, "", 0);
            //LLVMAddNamedMetadataOperand(module, "llvm.dbg.cu", dicu);
        }
    }

    void teardown_generate(Label *entry = nullptr) {
        process_labels();

        size_t k = finalize_types();
        assert(!k);

        LLVMDisposeBuilder(builder);
        LLVMDisposeDIBuilder(di_builder);

#if SCOPES_DEBUG_CODEGEN
        LLVMDumpModule(module);
#endif
        char *errmsg = NULL;
        if (LLVMVerifyModule(module, LLVMReturnStatusAction, &errmsg)) {
            StyledStream ss(std::cerr);
            if (entry) {
                stream_label(ss, entry, StreamLabelFormat());
            }
            LLVMDumpModule(module);
            location_error(
                String::join(
                    String::from("LLVM: "),
                    String::from_cstr(errmsg)));
        }
        LLVMDisposeMessage(errmsg);
    }

    // for generating object files
    LLVMModuleRef generate(const String *name, Scope *table) {

        {
            std::unordered_set<Label *> visited;
            Labels labels;
            Scope *t = table;
            while (t) {
                for (auto it = t->map->begin(); it != t->map->end(); ++it) {
                    Label *fn = it->second.value;

                    fn->verify_compilable();
                    fn->build_reachable(visited, &labels);
                }
                t = t->parent;
            }
            for (auto it = labels.begin(); it != labels.end(); ++it) {
                (*it)->insert_into_usermap(user_map);
            }
        }

        setup_generate(name->data);

        Scope *t = table;
        while (t) {
            for (auto it = t->map->begin(); it != t->map->end(); ++it) {

                Symbol name = it->first;
                Label *fn = it->second.value;

                auto func = label_to_function(fn, true, name);
                LLVMSetLinkage(func, LLVMExternalLinkage);

            }
            t = t->parent;
        }

        teardown_generate();
        return module;
    }

    std::pair<LLVMModuleRef, LLVMValueRef> generate(Label *entry) {
        assert(all_parameters_lowered(entry));
        assert(!entry->is_basic_block_like());

        {
            std::unordered_set<Label *> visited;
            Labels labels;
            entry->build_reachable(visited, &labels);
            for (auto it = labels.begin(); it != labels.end(); ++it) {
                (*it)->insert_into_usermap(user_map);
            }
        }

        const char *name = entry->name.name()->data;
        setup_generate(name);

        auto func = label_to_function(entry, true);
        LLVMSetLinkage(func, LLVMExternalLinkage);

        teardown_generate(entry);

        return std::pair<LLVMModuleRef, LLVMValueRef>(module, func);
    }

};

std::unordered_map<const Type *, LLVMTypeRef> LLVMIRGenerator::type_cache;
ArgTypes LLVMIRGenerator::type_todo;
LLVMTypeRef LLVMIRGenerator::voidT = nullptr;
LLVMTypeRef LLVMIRGenerator::i1T = nullptr;
LLVMTypeRef LLVMIRGenerator::i8T = nullptr;
LLVMTypeRef LLVMIRGenerator::i16T = nullptr;
LLVMTypeRef LLVMIRGenerator::i32T = nullptr;
LLVMTypeRef LLVMIRGenerator::i64T = nullptr;
LLVMTypeRef LLVMIRGenerator::f32T = nullptr;
LLVMTypeRef LLVMIRGenerator::f32x2T = nullptr;
LLVMTypeRef LLVMIRGenerator::f64T = nullptr;
LLVMTypeRef LLVMIRGenerator::rawstringT = nullptr;
LLVMTypeRef LLVMIRGenerator::noneT = nullptr;
LLVMValueRef LLVMIRGenerator::noneV = nullptr;
LLVMAttributeRef LLVMIRGenerator::attr_byval = nullptr;
LLVMAttributeRef LLVMIRGenerator::attr_sret = nullptr;
LLVMAttributeRef LLVMIRGenerator::attr_nonnull = nullptr;

//------------------------------------------------------------------------------
// IL COMPILER
//------------------------------------------------------------------------------

static void pprint(int pos, unsigned char *buf, int len, const char *disasm) {
  int i;
  printf("%04x:  ", pos);
  for (i = 0; i < 8; i++) {
    if (i < len) {
      printf("%02x ", buf[i]);
    } else {
      printf("   ");
    }
  }

  printf("   %s\n", disasm);
}

static void do_disassemble(LLVMTargetMachineRef tm, void *fptr, int siz) {

    unsigned char *buf = (unsigned char *)fptr;

  LLVMDisasmContextRef D = LLVMCreateDisasmCPUFeatures(
    LLVMGetTargetMachineTriple(tm),
    LLVMGetTargetMachineCPU(tm),
    LLVMGetTargetMachineFeatureString(tm),
    NULL, 0, NULL, NULL);
    LLVMSetDisasmOptions(D,
        LLVMDisassembler_Option_PrintImmHex);
  char outline[1024];
  int pos;

  if (!D) {
    printf("ERROR: Couldn't create disassembler\n");
    return;
  }

  pos = 0;
  while (pos < siz) {
    size_t l = LLVMDisasmInstruction(D, buf + pos, siz - pos, 0, outline,
                                     sizeof(outline));
    if (!l) {
      pprint(pos, buf + pos, 1, "\t???");
      pos++;
        break;
    } else {
      pprint(pos, buf + pos, l, outline);
      pos += l;
    }
  }

  LLVMDisasmDispose(D);
}

class DisassemblyListener : public llvm::JITEventListener {
public:
    llvm::ExecutionEngine *ee;
    DisassemblyListener(llvm::ExecutionEngine *_ee) : ee(_ee) {}

    std::unordered_map<void *, size_t> sizes;

    void InitializeDebugData(
        llvm::StringRef name,
        llvm::object::SymbolRef::Type type, uint64_t sz) {
        if(type == llvm::object::SymbolRef::ST_Function) {
            #if !defined(__arm__) && !defined(__linux__)
            name = name.substr(1);
            #endif
            void * addr = (void*)ee->getFunctionAddress(name);
            if(addr) {
                assert(addr);
                sizes[addr] = sz;
            }
        }
    }

    virtual void NotifyObjectEmitted(
        const llvm::object::ObjectFile &Obj,
        const llvm::RuntimeDyld::LoadedObjectInfo &L) {
        auto size_map = llvm::object::computeSymbolSizes(Obj);
        for(auto & S : size_map) {
            llvm::object::SymbolRef sym = S.first;
            auto name = sym.getName();
            auto type = sym.getType();
            if(name && type)
                InitializeDebugData(name.get(),type.get(),S.second);
        }
    }
};

enum {
    CF_DumpDisassembly  = (1 << 0),
    CF_DumpModule       = (1 << 1),
    CF_DumpFunction     = (1 << 2),
    CF_DumpTime         = (1 << 3),
    CF_NoDebugInfo      = (1 << 4),
    CF_O1               = (1 << 5),
    CF_O2               = (1 << 6),
    CF_O3               = CF_O1 | CF_O2,
};

static void compile_object(const String *path, Scope *scope, uint64_t flags) {
    Timer sum_compile_time(TIMER_Compile);
#if SCOPES_COMPILE_WITH_DEBUG_INFO
#else
    flags |= CF_NoDebugInfo;
#endif
#if SCOPES_OPTIMIZE_ASSEMBLY
    flags |= CF_O3;
#endif

    LLVMIRGenerator ctx;
    ctx.inline_pointers = false;
    if (flags & CF_NoDebugInfo) {
        ctx.use_debug_info = false;
    }

    LLVMModuleRef module;
    {
        Timer generate_timer(TIMER_Generate);
        module = ctx.generate(path, scope);
    }

    if (flags & CF_O3) {
        Timer optimize_timer(TIMER_Optimize);
        int level = 0;
        if ((flags & CF_O3) == CF_O1)
            level = 1;
        else if ((flags & CF_O3) == CF_O2)
            level = 2;
        else if ((flags & CF_O3) == CF_O3)
            level = 3;
        build_and_run_opt_passes(module, level);
    }
    if (flags & CF_DumpModule) {
        LLVMDumpModule(module);
    }

    assert(ee);
    auto tm = LLVMGetExecutionEngineTargetMachine(ee);

    char *errormsg = nullptr;
    char *path_cstr = strdup(path->data);
    if (LLVMTargetMachineEmitToFile(tm, module, path_cstr,
        LLVMObjectFile, &errormsg)) {
        location_error(String::from_cstr(errormsg));
    }
    free(path_cstr);
}

static DisassemblyListener *disassembly_listener = nullptr;
static Any compile(Label *fn, uint64_t flags) {
    Timer sum_compile_time(TIMER_Compile);
#if SCOPES_COMPILE_WITH_DEBUG_INFO
#else
    flags |= CF_NoDebugInfo;
#endif
#if SCOPES_OPTIMIZE_ASSEMBLY
    flags |= CF_O3;
#endif

    fn->verify_compilable();
    const Type *functype = Pointer(
        fn->get_function_type(), PTF_NonWritable, SYM_Unnamed);

    LLVMIRGenerator ctx;
    if (flags & CF_NoDebugInfo) {
        ctx.use_debug_info = false;
    }

    std::pair<LLVMModuleRef, LLVMValueRef> result;
    {
        /*
        A note on debugging "LLVM ERROR:" messages that seem to give no plausible
        point of origin: you can either set a breakpoint at llvm::report_fatal_error
        or at exit if the llvm symbols are missing, and then look at the stack trace.
        */
        Timer generate_timer(TIMER_Generate);
        result = ctx.generate(fn);
    }

    auto module = result.first;
    auto func = result.second;
    assert(func);

    if (!ee) {
        char *errormsg = nullptr;

        LLVMMCJITCompilerOptions opts;
        LLVMInitializeMCJITCompilerOptions(&opts, sizeof(opts));
        opts.OptLevel = 0;
        opts.NoFramePointerElim = true;

        if (LLVMCreateMCJITCompilerForModule(&ee, module, &opts,
            sizeof(opts), &errormsg)) {
            location_error(String::from_cstr(errormsg));
        }
    } else {
        LLVMAddModule(ee, module);
    }

    if (!disassembly_listener && (flags & CF_DumpDisassembly)) {
        llvm::ExecutionEngine *pEE = reinterpret_cast<llvm::ExecutionEngine*>(ee);
        disassembly_listener = new DisassemblyListener(pEE);
        pEE->RegisterJITEventListener(disassembly_listener);
    }

    if (flags & CF_O3) {
        Timer optimize_timer(TIMER_Optimize);
        int level = 0;
        if ((flags & CF_O3) == CF_O1)
            level = 1;
        else if ((flags & CF_O3) == CF_O2)
            level = 2;
        else if ((flags & CF_O3) == CF_O3)
            level = 3;
        build_and_run_opt_passes(module, level);
    }
    if (flags & CF_DumpModule) {
        LLVMDumpModule(module);
    } else if (flags & CF_DumpFunction) {
        LLVMDumpValue(func);
    }

    void *pfunc;
    {
        Timer mcjit_timer(TIMER_MCJIT);
        pfunc = LLVMGetPointerToGlobal(ee, func);
    }
    if (flags & CF_DumpDisassembly) {
        assert(disassembly_listener);
        //auto td = LLVMGetExecutionEngineTargetData(ee);
        auto tm = LLVMGetExecutionEngineTargetMachine(ee);
        auto it = disassembly_listener->sizes.find(pfunc);
        if (it != disassembly_listener->sizes.end()) {
            std::cout << "disassembly:\n";
            do_disassemble(tm, pfunc, it->second);
        } else {
            std::cout << "no disassembly available\n";
        }
    }

    return Any::from_pointer(functype, pfunc);
}

static void optimize_spirv(std::vector<unsigned int> &result, int opt_level) {
    spvtools::Optimizer optimizer(SPV_ENV_UNIVERSAL_1_2);
    /*
    optimizer.SetMessageConsumer([](spv_message_level_t level, const char* source,
        const spv_position_t& position,
        const char* message) {
    std::cerr << StringifyMessage(level, source, position, message)
    << std::endl;
    });*/
    StyledStream ss(std::cerr);
    optimizer.SetMessageConsumer([&ss](spv_message_level_t level, const char*,
        const spv_position_t& position,
        const char* message) {
        switch (level) {
        case SPV_MSG_FATAL:
        case SPV_MSG_INTERNAL_ERROR:
        case SPV_MSG_ERROR:
            ss << Style_Error << "error: " << Style_None
                << position.index << ": " << message << std::endl;
            break;
        case SPV_MSG_WARNING:
            ss << Style_Warning << "warning: " << Style_None
                << position.index << ": " << message << std::endl;
            break;
        case SPV_MSG_INFO:
            ss << Style_Comment << "info: " << Style_None
                << position.index << ": " << message << std::endl;
            break;
        default:
            break;
        }
    });

    if (opt_level == 3) {
        optimizer.RegisterPass(spvtools::CreateStripDebugInfoPass());
        optimizer.RegisterPass(spvtools::CreateFreezeSpecConstantValuePass());
    }

    if (opt_level == 3) {
        optimizer.RegisterPass(spvtools::CreateInlineExhaustivePass());
    }
    optimizer.RegisterPass(spvtools::CreateLocalAccessChainConvertPass());
    optimizer.RegisterPass(spvtools::CreateInsertExtractElimPass());
    optimizer.RegisterPass(spvtools::CreateLocalSingleBlockLoadStoreElimPass());
    optimizer.RegisterPass(spvtools::CreateLocalSingleStoreElimPass());
    optimizer.RegisterPass(spvtools::CreateBlockMergePass());
    optimizer.RegisterPass(spvtools::CreateEliminateDeadConstantPass());
    optimizer.RegisterPass(spvtools::CreateFoldSpecConstantOpAndCompositePass());
    optimizer.RegisterPass(spvtools::CreateUnifyConstantPass());

    optimizer.RegisterPass(spvtools::CreateDeadBranchElimPass());
    optimizer.RegisterPass(spvtools::CreateLocalMultiStoreElimPass());
    if (opt_level == 3) {
        optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
    }
    optimizer.RegisterPass(spvtools::CreateCommonUniformElimPass());

    optimizer.RegisterPass(spvtools::CreateFlattenDecorationPass());
    //optimizer.RegisterPass(spvtools::CreateCompactIdsPass());

    std::vector<unsigned int> oldresult = result;
    result.clear();
    if (!optimizer.Run(oldresult.data(), oldresult.size(), &result)) {
        location_error(String::from(
            "IL->SPIR: error while running optimization passes"));
    }

    verify_spirv(result);
}

static const String *compile_spirv(Symbol target, Label *fn, uint64_t flags) {
    Timer sum_compile_time(TIMER_CompileSPIRV);

    fn->verify_compilable();

    SPIRVGenerator ctx;
    if (flags & CF_NoDebugInfo) {
        ctx.use_debug_info = false;
    }

    std::vector<unsigned int> result;
    {
        Timer generate_timer(TIMER_GenerateSPIRV);
        ctx.generate(result, target, fn);
    }

    if (flags & CF_O3) {
        int level = 0;
        if ((flags & CF_O3) == CF_O1)
            level = 1;
        else if ((flags & CF_O3) == CF_O2)
            level = 2;
        else if ((flags & CF_O3) == CF_O3)
            level = 3;
        optimize_spirv(result, level);
    }

    if (flags & CF_DumpModule) {
    } else if (flags & CF_DumpFunction) {
    }
    if (flags & CF_DumpDisassembly) {
        disassemble_spirv(result);
    }

    size_t bytesize = sizeof(unsigned int) * result.size();

    return String::from((char *)&result[0], bytesize);
}

static const String *compile_glsl(Symbol target, Label *fn, uint64_t flags) {
    Timer sum_compile_time(TIMER_CompileSPIRV);

    fn->verify_compilable();

    SPIRVGenerator ctx;
    if (flags & CF_NoDebugInfo) {
        ctx.use_debug_info = false;
    }

    std::vector<unsigned int> result;
    {
        Timer generate_timer(TIMER_GenerateSPIRV);
        ctx.generate(result, target, fn);
    }

    if (flags & CF_O3) {
        int level = 0;
        if ((flags & CF_O3) == CF_O1)
            level = 1;
        else if ((flags & CF_O3) == CF_O2)
            level = 2;
        else if ((flags & CF_O3) == CF_O3)
            level = 3;
        optimize_spirv(result, level);
    }

    if (flags & CF_DumpDisassembly) {
        disassemble_spirv(result);
    }

	spirv_cross::CompilerGLSL glsl(std::move(result));

    /*
    // The SPIR-V is now parsed, and we can perform reflection on it.
    spirv_cross::ShaderResources resources = glsl.get_shader_resources();
    // Get all sampled images in the shader.
    for (auto &resource : resources.sampled_images)
    {
        unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
        unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
        printf("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);

        // Modify the decoration to prepare it for GLSL.
        glsl.unset_decoration(resource.id, spv::DecorationDescriptorSet);

        // Some arbitrary remapping if we want.
        glsl.set_decoration(resource.id, spv::DecorationBinding, set * 16 + binding);
    }
    */

    // Set some options.
    /*
    spirv_cross::CompilerGLSL::Options options;
    options.version = 450;
    glsl.set_options(options);*/

    // Compile to GLSL, ready to give to GL driver.
    std::string source = glsl.compile();

    if (flags & (CF_DumpModule|CF_DumpFunction)) {
        std::cout << source << std::endl;
    }

    return String::from_stdstring(source);
}

//------------------------------------------------------------------------------
// COMMON ERRORS
//------------------------------------------------------------------------------

void invalid_op2_types_error(const Type *A, const Type *B) {
    StyledString ss;
    ss.out << "invalid operand types " << A << " and " << B;
    location_error(ss.str());
}

//------------------------------------------------------------------------------
// OPERATOR TEMPLATES
//------------------------------------------------------------------------------

#define OP1_TEMPLATE(NAME, RTYPE, OP) \
    template<typename T> struct op_ ## NAME { \
        typedef RTYPE rtype; \
        static bool reductive() { return false; } \
        void operator()(void **srcptrs, void *destptr, size_t count) { \
            for (size_t i = 0; i < count; ++i) { \
                ((rtype *)destptr)[i] = op(((T *)(srcptrs[0]))[i]); \
            } \
        } \
        rtype op(T x) { \
            return OP; \
        } \
    };

#define OP2_TEMPLATE(NAME, RTYPE, OP) \
    template<typename T> struct op_ ## NAME { \
        typedef RTYPE rtype; \
        static bool reductive() { return false; } \
        void operator()(void **srcptrs, void *destptr, size_t count) { \
            for (size_t i = 0; i < count; ++i) { \
                ((rtype *)destptr)[i] = op(((T *)(srcptrs[0]))[i], ((T *)(srcptrs[1]))[i]); \
            } \
        } \
        rtype op(T a, T b) { \
            return OP; \
        } \
    };

template<typename T>
inline bool isnan(T f) {
    return f != f;
}

#define BOOL_IFXOP_TEMPLATE(NAME, OP) OP2_TEMPLATE(NAME, bool, a OP b)
#define BOOL_OF_TEMPLATE(NAME) OP2_TEMPLATE(NAME, bool, !isnan(a) && !isnan(b))
#define BOOL_UF_TEMPLATE(NAME) OP2_TEMPLATE(NAME, bool, isnan(a) || isnan(b))
#define BOOL_OF_IFXOP_TEMPLATE(NAME, OP) OP2_TEMPLATE(NAME, bool, !isnan(a) && !isnan(b) && (a OP b))
#define BOOL_UF_IFXOP_TEMPLATE(NAME, OP) OP2_TEMPLATE(NAME, bool, isnan(a) || isnan(b) || (a OP b))
#define IFXOP_TEMPLATE(NAME, OP) OP2_TEMPLATE(NAME, T, a OP b)
#define PFXOP_TEMPLATE(NAME, OP) OP2_TEMPLATE(NAME, T, OP(a, b))
#define PUNOP_TEMPLATE(NAME, OP) OP1_TEMPLATE(NAME, T, OP(x))

template<typename RType>
struct select_op_return_type {
    const Type *operator ()(const Type *T) { return T; }
};

static const Type *bool_op_return_type(const Type *T) {
    T = storage_type(T);
    if (T->kind() == TK_Vector) {
        auto vi = cast<VectorType>(T);
        return Vector(TYPE_Bool, vi->count);
    } else {
        return TYPE_Bool;
    }
}

template<>
struct select_op_return_type<bool> {
    const Type *operator ()(const Type *T) {
        return bool_op_return_type(T);
    }
};

BOOL_IFXOP_TEMPLATE(Equal, ==)
BOOL_IFXOP_TEMPLATE(NotEqual, !=)
BOOL_IFXOP_TEMPLATE(Greater, >)
BOOL_IFXOP_TEMPLATE(GreaterEqual, >=)
BOOL_IFXOP_TEMPLATE(Less, <)
BOOL_IFXOP_TEMPLATE(LessEqual, <=)

BOOL_OF_IFXOP_TEMPLATE(OEqual, ==)
BOOL_OF_IFXOP_TEMPLATE(ONotEqual, !=)
BOOL_OF_IFXOP_TEMPLATE(OGreater, >)
BOOL_OF_IFXOP_TEMPLATE(OGreaterEqual, >=)
BOOL_OF_IFXOP_TEMPLATE(OLess, <)
BOOL_OF_IFXOP_TEMPLATE(OLessEqual, <=)
BOOL_OF_TEMPLATE(Ordered)

BOOL_UF_IFXOP_TEMPLATE(UEqual, ==)
BOOL_UF_IFXOP_TEMPLATE(UNotEqual, !=)
BOOL_UF_IFXOP_TEMPLATE(UGreater, >)
BOOL_UF_IFXOP_TEMPLATE(UGreaterEqual, >=)
BOOL_UF_IFXOP_TEMPLATE(ULess, <)
BOOL_UF_IFXOP_TEMPLATE(ULessEqual, <=)
BOOL_UF_TEMPLATE(Unordered)

IFXOP_TEMPLATE(Add, +)
IFXOP_TEMPLATE(Sub, -)
IFXOP_TEMPLATE(Mul, *)

IFXOP_TEMPLATE(SDiv, /)
IFXOP_TEMPLATE(UDiv, /)
IFXOP_TEMPLATE(SRem, %)
IFXOP_TEMPLATE(URem, %)

IFXOP_TEMPLATE(BAnd, &)
IFXOP_TEMPLATE(BOr, |)
IFXOP_TEMPLATE(BXor, ^)

IFXOP_TEMPLATE(Shl, <<)
IFXOP_TEMPLATE(LShr, >>)
IFXOP_TEMPLATE(AShr, >>)

IFXOP_TEMPLATE(FAdd, +)
IFXOP_TEMPLATE(FSub, -)
IFXOP_TEMPLATE(FMul, *)
IFXOP_TEMPLATE(FDiv, /)
PFXOP_TEMPLATE(FRem, std::fmod)

} namespace std {

static bool abs(bool x) {
    return x;
}

} namespace scopes {

PUNOP_TEMPLATE(FAbs, std::abs)

template <typename T>
static T sgn(T val) {
    return T((T(0) < val) - (val < T(0)));
}

template<typename T>
static T radians(T x) {
    return x * T(M_PI / 180.0);
}

template<typename T>
static T degrees(T x) {
    return x * T(180.0 / M_PI);
}

template<typename T>
static T inversesqrt(T x) {
    return T(1.0) / std::sqrt(x);
}

template<typename T>
static T step(T edge, T x) {
    return T(x >= edge);
}

PUNOP_TEMPLATE(FSign, sgn)
PUNOP_TEMPLATE(SSign, sgn)
PUNOP_TEMPLATE(Radians, radians)
PUNOP_TEMPLATE(Degrees, degrees)
PUNOP_TEMPLATE(Sin, std::sin)
PUNOP_TEMPLATE(Cos, std::cos)
PUNOP_TEMPLATE(Tan, std::tan)
PUNOP_TEMPLATE(Asin, std::asin)
PUNOP_TEMPLATE(Acos, std::acos)
PUNOP_TEMPLATE(Atan, std::atan)
PFXOP_TEMPLATE(Atan2, std::atan2)
PUNOP_TEMPLATE(Exp, std::exp)
PUNOP_TEMPLATE(Log, std::log)
PUNOP_TEMPLATE(Exp2, std::exp2)
PUNOP_TEMPLATE(Log2, std::log2)
PUNOP_TEMPLATE(Sqrt, std::sqrt)
PUNOP_TEMPLATE(Trunc, std::trunc)
PUNOP_TEMPLATE(Floor, std::floor)
PUNOP_TEMPLATE(InverseSqrt, inversesqrt)
PFXOP_TEMPLATE(Pow, std::pow)
PFXOP_TEMPLATE(Step, step)

template<typename T> struct op_Cross {
    typedef T rtype;
    static bool reductive() { return false; }
    void operator()(void **srcptrs, void *destptr, size_t count) {
        assert(count == 3);
        auto x = (T *)(srcptrs[0]);
        auto y = (T *)(srcptrs[1]);
        auto ret = (rtype *)destptr;
        ret[0] = x[1] * y[2] - y[1] * x[2];
        ret[1] = x[2] * y[0] - y[2] * x[0];
        ret[2] = x[0] * y[1] - y[0] * x[1];
    }
};

template<typename T> struct op_FMix {
    typedef T rtype;
    static bool reductive() { return false; }
    void operator()(void **srcptrs, void *destptr, size_t count) {
        auto x = (T *)(srcptrs[0]);
        auto y = (T *)(srcptrs[1]);
        auto a = (T *)(srcptrs[2]);
        auto ret = (rtype *)destptr;
        for (size_t i = 0; i < count; ++i) {
            ret[i] = x[i] * (T(1.0) - a[i]) + y[i] * a[i];
        }
    }
};

template<typename T> struct op_Normalize {
    typedef T rtype;
    static bool reductive() { return false; }
    void operator()(void **srcptrs, void *destptr, size_t count) {
        auto x = (T *)(srcptrs[0]);
        T r = T(0);
        for (size_t i = 0; i < count; ++i) {
            r += x[i] * x[i];
        }
        r = (r == T(0))?T(1):(T(1)/std::sqrt(r));
        auto ret = (rtype *)destptr;
        for (size_t i = 0; i < count; ++i) {
            ret[i] = x[i] * r;
        }
    }
};

template<typename T> struct op_Length {
    typedef T rtype;
    static bool reductive() { return true; }
    void operator()(void **srcptrs, void *destptr, size_t count) {
        auto x = (T *)(srcptrs[0]);
        T r = T(0);
        for (size_t i = 0; i < count; ++i) {
            r += x[i] * x[i];
        }
        auto ret = (rtype *)destptr;
        ret[0] = std::sqrt(r);
    }
};

template<typename T> struct op_Distance {
    typedef T rtype;
    static bool reductive() { return true; }
    void operator()(void **srcptrs, void *destptr, size_t count) {
        auto x = (T *)(srcptrs[0]);
        auto y = (T *)(srcptrs[1]);
        T r = T(0);
        for (size_t i = 0; i < count; ++i) {
            T d = x[i] - y[i];
            r += d * d;
        }
        auto ret = (rtype *)destptr;
        ret[0] = std::sqrt(r);
    }
};

#undef BOOL_IFXOP_TEMPLATE
#undef BOOL_OF_TEMPLATE
#undef BOOL_UF_TEMPLATE
#undef BOOL_OF_IFXOP_TEMPLATE
#undef BOOL_UF_IFXOP_TEMPLATE
#undef IFXOP_TEMPLATE
#undef PFXOP_TEMPLATE

static void *aligned_alloc(size_t sz, size_t al) {
    assert(sz);
    assert(al);
    return reinterpret_cast<void *>(
        scopes::align(reinterpret_cast<uintptr_t>(tracked_malloc(sz + al - 1)), al));
}

static void *alloc_storage(const Type *T) {
    size_t sz = size_of(T);
    size_t al = align_of(T);
    return aligned_alloc(sz, al);
}

static void *copy_storage(const Type *T, void *ptr) {
    size_t sz = size_of(T);
    size_t al = align_of(T);
    void *destptr = aligned_alloc(sz, al);
    memcpy(destptr, ptr, sz);
    return destptr;
}

struct IntTypes_i {
    typedef bool i1;
    typedef int8_t i8;
    typedef int16_t i16;
    typedef int32_t i32;
    typedef int64_t i64;
};
struct IntTypes_u {
    typedef bool i1;
    typedef uint8_t i8;
    typedef uint16_t i16;
    typedef uint32_t i32;
    typedef uint64_t i64;
};

template<typename IT, template<typename T> class OpT>
struct DispatchInteger {
    typedef typename OpT<int8_t>::rtype rtype;
    static bool reductive() { return OpT<int8_t>::reductive(); }
    static const Type *return_type(Any *args, size_t numargs) {
        assert(numargs >= 1);
        return select_op_return_type<rtype>{}(args[0].type);
    }
    void operator ()(const Type *ET, void **srcptrs, void *destptr, size_t count,
                        Any *args, size_t numargs) {
        size_t width = cast<IntegerType>(ET)->width;
        switch(width) {
        case 1: OpT<typename IT::i1>{}(srcptrs, destptr, count); break;
        case 8: OpT<typename IT::i8>{}(srcptrs, destptr, count); break;
        case 16: OpT<typename IT::i16>{}(srcptrs, destptr, count); break;
        case 32: OpT<typename IT::i32>{}(srcptrs, destptr, count); break;
        case 64: OpT<typename IT::i64>{}(srcptrs, destptr, count); break;
        default:
            StyledString ss;
            ss.out << "unsupported bitwidth (" << width << ") for integer operation";
            location_error(ss.str());
            break;
        };
    }
};

template<template<typename T> class OpT>
struct DispatchReal {
    typedef typename OpT<float>::rtype rtype;
    static bool reductive() { return OpT<float>::reductive(); }
    static const Type *return_type(Any *args, size_t numargs) {
        assert(numargs >= 1);
        return select_op_return_type<rtype>{}(args[0].type);
    }
    void operator ()(const Type *ET, void **srcptrs, void *destptr, size_t count,
                        Any *args, size_t numargs) {
        size_t width = cast<RealType>(ET)->width;
        switch(width) {
        case 32: OpT<float>{}(srcptrs, destptr, count); break;
        case 64: OpT<double>{}(srcptrs, destptr, count); break;
        default:
            StyledString ss;
            ss.out << "unsupported bitwidth (" << width << ") for float operation";
            location_error(ss.str());
            break;
        };
    }
};

struct DispatchSelect {
    static bool reductive() { return false; }
    static const Type *return_type(Any *args, size_t numargs) {
        assert(numargs >= 1);
        return args[1].type;
    }
    void operator ()(const Type *ET, void **srcptrs, void *destptr, size_t count,
                        Any *args, size_t numargs) {
        assert(numargs == 3);
        bool *cond = (bool *)srcptrs[0];
        void *x = srcptrs[1];
        void *y = srcptrs[2];
        const Type *Tx = storage_type(args[1].type);
        if (Tx->kind() == TK_Vector) {
            auto VT = cast<VectorType>(Tx);
            auto stride = VT->stride;
            for (size_t i = 0; i < count; ++i) {
                memcpy(VT->getelementptr(destptr, i),
                    VT->getelementptr((cond[i] ? x : y), i),
                    stride);
            }
        } else {
            assert(count == 1);
            auto sz = size_of(Tx);
            memcpy(destptr, (cond[0] ? x : y), sz);
        }
    }
};

template<typename DispatchT>
static Any apply_op(Any *args, size_t numargs) {
    auto ST = storage_type(args[0].type);
    size_t count;
    void *srcptrs[numargs];
    void *destptr;
    Any result = none;
    auto RT = DispatchT::return_type(args, numargs);
    const Type *ET = nullptr;
    if (ST->kind() == TK_Vector) {
        auto vi = cast<VectorType>(ST);
        count = vi->count;
        for (size_t i = 0; i < numargs; ++i) {
            srcptrs[i] = args[i].pointer;
        }
        if (DispatchT::reductive()) {
            result.type = vi->element_type;
            destptr = get_pointer(result.type, result);
        } else {
            destptr = alloc_storage(RT);
            result = Any::from_pointer(RT, destptr);
        }
        ET = storage_type(vi->element_type);
    } else {
        count = 1;
        for (size_t i = 0; i < numargs; ++i) {
            srcptrs[i] = get_pointer(args[i].type, args[i]);
        }
        result.type = RT;
        destptr = get_pointer(result.type, result);
        ET = ST;
    }
    DispatchT{}(ET, srcptrs, destptr, count, args, numargs);
    return result;
}

template<typename IT, template<typename T> class OpT >
static Any apply_integer_op(Any x) {
    Any args[] = { x };
    return apply_op< DispatchInteger<IT, OpT> >(args, 1);
}

template<typename IT, template<typename T> class OpT >
static Any apply_integer_op(Any a, Any b) {
    Any args[] = { a, b };
    return apply_op< DispatchInteger<IT, OpT> >(args, 2);
}

template<template<typename T> class OpT>
static Any apply_real_op(Any x) {
    Any args[] = { x };
    return apply_op< DispatchReal<OpT> >(args, 1);
}

template<template<typename T> class OpT>
static Any apply_real_op(Any a, Any b) {
    Any args[] = { a, b };
    return apply_op< DispatchReal<OpT> >(args, 2);
}

template<template<typename T> class OpT>
static Any apply_real_op(Any a, Any b, Any c) {
    Any args[] = { a, b, c };
    return apply_op< DispatchReal<OpT> >(args, 3);
}

//------------------------------------------------------------------------------
// NORMALIZE
//------------------------------------------------------------------------------

// assuming that value is an elementary type that can be put in a vector
static Any smear(Any value, size_t count) {
    size_t sz = size_of(value.type);
    void *psrc = get_pointer(value.type, value);
    auto VT = cast<VectorType>(Vector(value.type, count));
    void *pdest = alloc_storage(VT);
    for (size_t i = 0; i < count; ++i) {
        void *p = VT->getelementptr(pdest, i);
        memcpy(p, psrc, sz);
    }
    return Any::from_pointer(VT, pdest);
}

#define B_ARITH_OPS() \
        IARITH_NUW_NSW_OPS(Add) \
        IARITH_NUW_NSW_OPS(Sub) \
        IARITH_NUW_NSW_OPS(Mul) \
        \
        IARITH_OP(SDiv, i) \
        IARITH_OP(UDiv, u) \
        IARITH_OP(SRem, i) \
        IARITH_OP(URem, u) \
        \
        IARITH_OP(BAnd, u) \
        IARITH_OP(BOr, u) \
        IARITH_OP(BXor, u) \
        \
        IARITH_OP(Shl, u) \
        IARITH_OP(LShr, u) \
        IARITH_OP(AShr, i) \
        \
        FARITH_OP(FAdd) \
        FARITH_OP(FSub) \
        FARITH_OP(FMul) \
        FARITH_OP(FDiv) \
        FARITH_OP(FRem) \
        \
        FUN_OP(FAbs) \
        \
        IUN_OP(SSign, i) \
        FUN_OP(FSign) \
        \
        FUN_OP(Radians) FUN_OP(Degrees) \
        FUN_OP(Sin) FUN_OP(Cos) FUN_OP(Tan) \
        FUN_OP(Asin) FUN_OP(Acos) FUN_OP(Atan) FARITH_OP(Atan2) \
        FUN_OP(Exp) FUN_OP(Log) FUN_OP(Exp2) FUN_OP(Log2) \
        FUN_OP(Trunc) FUN_OP(Floor) FARITH_OP(Step) \
        FARITH_OP(Pow) FUN_OP(Sqrt) FUN_OP(InverseSqrt) \
        \
        FTRI_OP(FMix)

static Label *expand_module(Any expr, Scope *scope = nullptr);

struct Specializer {
    enum CLICmd {
        CmdNone,
        CmdSkip,
    };

    struct Trace {
        const Anchor *anchor;
        Symbol name;

        Trace(const Anchor *_anchor, Symbol _name) :
            anchor(_anchor), name(_name) {
            assert(anchor);
            }
    };

    typedef std::vector<Trace> Traces;

    StyledStream ss_cout;
    static Traces traceback;
    static int solve_refs;
    static bool enable_step_debugger;
    static CLICmd clicmd;
    Args tmp_args;
    Parameters tmp_params;

    /*
        the new solver works more like a specializer VM, in that it executes
        template labels and returns a residual program as specialized labels.

        the specializer nests each time the program branches, and returns when
        branches merge.

        each run operates on a single contiguous non-branching chain of commands.

        when a function is encountered that has not been specialized yet, it is
        specialized.

        when an inline is encountered, a new nesting level is entered until the
        inline has returned.

        when a branch is encountered, each branch is specialized, and the
        resulting types merged. only then does specialization continue.

        when a possible loop entry point is encountered, it is necessary to run
        the loop in a new nesting to discover whether the loop is reentrant.

        for non-tail recursive functions, it becomes difficult to type the
        function when it has not returned yet. in this case, execution needs
        to suspend until all other branches have been mapped out.

        additional care has to be taken to optimize continuation arguments of
        residual instructions and branches only when the next residual
        instruction is known, so that useless continuation calls can be
        purged before they are even created.
    */

#if 0
    Label *specialize_function(Frame *frame, Label *l, const Args &args) {
        assert(frame);
        assert(l);

        assert(l->is_template());



    }
#endif

    Specializer()
        : ss_cout(std::cout)
    {}

    static void evaluate(Frame *frame, Argument arg, Args &dest, bool last_param = false) {
        if (arg.value.type == TYPE_Label) {
            // do not wrap labels in closures that have been solved
            if (!arg.value.label->is_template()) {
                dest.push_back(Argument(arg.key, arg.value.label));
            } else {
                Label *label = arg.value.label;
                assert(frame);
                Frame *top = frame->find_parent_frame(label);
                if (top) {
                    frame = top;
                } else if (label->body.scope_label) {
                    top = frame->find_parent_frame(label->body.scope_label);
                    if (top) {
                        frame = top;
                    } else {
                        if (label->is_debug()) {
                            StyledStream ss(std::cerr);
                            ss << "frame " <<  frame <<  ": can't find scope label for closure" << std::endl;
                            stream_label(ss, label, StreamLabelFormat::debug_single());
                        }
                    }
                } else {
                    if (label->is_debug()) {
                        StyledStream ss(std::cerr);
                        ss << "frame " <<  frame <<  ": label has no scope label for closure" << std::endl;
                        stream_label(ss, label, StreamLabelFormat::debug_single());
                    }
                }
                dest.push_back(Argument(arg.key, Closure::from(label, frame)));
            }
        } else if (arg.value.type == TYPE_Parameter
            && arg.value.parameter->label
            && arg.value.parameter->label->is_template()) {
            auto param = arg.value.parameter;
            frame = frame->find_parent_frame(param->label);
            if (!frame) {
                StyledString ss;
                ss.out << "parameter " << param << " is unbound";
                location_error(ss.str());
            }
            // special situation: we're forwarding varargs, but assigning
            // it to a new argument key; since keys can only be mapped to
            // individual varargs, and must not be duplicated, we have these
            // options to resolve the conflict:
            // 1. the vararg keys override the new explicit key; this was the
            //    old behavior and made it possible for implicit vararg return
            //    values to override explicit reassignments, which was
            //    always surprising and unwanted, i.e. a bug.
            // 2. re-assign only the first vararg key, keeping remaining keys
            //    as they are.
            // 3. produce a compiler error when an explicit key is set, but
            //    the vararg set is larger than 1.
            // 4. treat a keyed argument in last place like any previous argument,
            //    causing only a single vararg result to be forwarded.
            // we use option 4, as it is most consistent with existing behavior,
            // and seems to be the least surprising choice.
            if (last_param && param->is_vararg() && !arg.is_keyed()) {
                // forward as-is, with keys
                for (size_t i = (size_t)param->index; i < frame->args.size(); ++i) {
                    dest.push_back(frame->args[i]);
                }
            } else if ((size_t)param->index < frame->args.size()) {
                auto &&srcarg = frame->args[param->index];
                // when forwarding a vararg and the arg is not re-keyed,
                // forward the vararg key as well.
                if (param->is_vararg() && !arg.is_keyed()) {
                    dest.push_back(srcarg);
                } else {
                    dest.push_back(Argument(arg.key, srcarg.value));
                }
            } else {
                if (!param->is_vararg()) {
    #if SCOPES_DEBUG_CODEGEN
                    {
                        StyledStream ss;
                        ss << frame << " " << frame->label;
                        for (size_t i = 0; i < frame->args.size(); ++i) {
                            ss << " " << frame->args[i];
                        }
                        ss << std::endl;
                    }
    #endif
                    StyledString ss;
                    ss.out << "parameter " << param << " is out of bounds ("
                        << param->index << " >= " << (int)frame->args.size() << ")";
                    location_error(ss.str());
                }
                dest.push_back(Argument(arg.key, none));
            }
        } else {
            dest.push_back(arg);
        }
    }

    void evaluate_body(Frame *frame, Body &dest, const Body &source) {
        auto &&args = source.args;
        Args &body = dest.args;
        tmp_args.clear();
        dest.copy_traits_from(source);
        evaluate(frame, source.enter, tmp_args);
        dest.enter = first(tmp_args).value;
        body.clear();

        size_t lasti = (args.size() - 1);
        for (size_t i = 0; i < args.size(); ++i) {
            evaluate(frame, args[i], body, (i == lasti));
        }
    }

    static void map_constant_arguments(Frame *frame, Label *label, const Args &args) {
        size_t lasti = label->params.size() - 1;
        size_t srci = 0;
        for (size_t i = 0; i < label->params.size(); ++i) {
            Parameter *param = label->params[i];
            if (param->is_vararg()) {
                assert(i == lasti);
                size_t ncount = args.size();
                while (srci < ncount) {
                    Argument value = args[srci];
                    assert(!is_unknown(value.value));
                    frame->args.push_back(value);
                    srci++;
                }
            } else if (srci < args.size()) {
                Argument value = args[srci];
                assert(!is_unknown(value.value));
                frame->args.push_back(value);
                srci++;
            } else {
                frame->args.push_back(none);
                srci++;
            }
        }
    }

    Label *fold_type_label_single(Frame *parent, Label *label, const Args &args) {
        return fold_type_label_single_frame(parent, label, args)->get_instance();
    }

    void stream_arg_types(StyledStream &ss, const Args &args) {
        if (args.size() <= 1) {
            ss << TYPE_Void << " ";
            return;
        }
        for (int i = 1; i < args.size(); ++i) {
            auto &&arg = args[i];
            assert(is_unknown(arg.value));
            ss << arg.value.typeref << " ";
        }
    }

    // inlining the arguments of an untyped scope (including continuation)
    // folds arguments and types parameters
    // arguments are treated as follows:
    // TYPE_Unknown = type the parameter
    //      type as TYPE_Unknown = leave the parameter as-is
    // any other = inline the argument and remove the parameter
    Frame *fold_type_label_single_frame(Frame *parent, Label *label, const Args &args) {
        assert(parent);
        assert(label);
        assert(!label->body.is_complete());
        size_t loop_count = 0;
        if ((parent != Frame::root) && (parent->label == label)) {
            Frame *top = parent;
            parent = top->parent;
            loop_count = top->loop_count + 1;
            if (loop_count > SCOPES_MAX_RECURSIONS) {
                StyledString ss;
                ss.out << "maximum number of recursions exceeded during"
                " compile time evaluation (" << SCOPES_MAX_RECURSIONS << ")."
                " Use 'unconst' to prevent constant propagation.";
                location_error(ss.str());
            }
        }

        const Anchor *caller_anchor = parent->label?parent->label->body.anchor:label->anchor;

        tmp_args.clear();

        size_t numparams = label->params.size();
        size_t numargs = args.size();

        size_t lasti = numparams - 1;
        size_t srci = 0;
        for (size_t i = 0; i < numparams; ++i) {
            Parameter *param = label->params[i];
            if (param->is_vararg()) {
                while (srci < numargs) {
                    tmp_args.push_back(args[srci]);
                    srci++;
                }
            } else {
                tmp_args.push_back((srci < numargs)?(args[srci]):none);
                srci++;
            }
        }

        if (label->is_debug()) {
            StyledStream ss(std::cerr);
            ss << "frame " << parent <<  ": instantiating label" << std::endl;
            stream_label(ss, label, StreamLabelFormat::debug_single());
            ss << "with key ";
            for (size_t i = 0; i < tmp_args.size(); ++i) {
                if (is_unknown(tmp_args[i].value)) {
                    ss << "?" << tmp_args[i].value.typeref << " ";
                } else {
                    ss << tmp_args[i] << " ";
                }
            }
            if (parent->inline_merge) {
                ss << "and merge inlined";
            }
            ss << std::endl;
        }

        Frame::ArgsKey la;
        la.label = label;
        la.args = tmp_args;
        {
            Frame *result = parent->find_frame(la);
            if (label->is_debug()) {
                StyledStream ss(std::cerr);
                if (result) {
                    ss << " and the label already exists (frame " << result << ")";
                    if (!result->get_instance()->body.is_complete()) {
                        ss << " but is incomplete:" << std::endl;
                        stream_label(ss, result->get_instance(), StreamLabelFormat::debug_single());
                    } else {
                        ss << std::endl;
                    }
                } else {
                    ss << " and the label is new" << std::endl;
                }
            }
            if (result)
                return result;
        }

        if (label->is_merge() && !parent->inline_merge) {
            for (int i = 1; i < tmp_args.size(); ++i) {
                // verify that all keys are unknown
                auto &&val = tmp_args[i].value;
                if (is_unknown(val)) {
                } else if (val.is_const()) {
                    StyledString ss;
                    ss.out << "attempting to return from branch, but returned argument #"
                        << i << " of type " << val.type << " is constant";
                    set_active_anchor(caller_anchor);
                    location_error(ss.str());
                } else {
                    // should not happen
                    assert(false);
                }
            }
            Frame::ArgsKey key;
            Frame *result_frame = parent->find_any_frame(label, key);
            if (result_frame && !(la == key)) {
                if ((key.args.size() > 1) && !is_unknown(key.args[0].value)) {
                    // ugh, label has been inlined before :C
                    location_message(key.label->anchor, String::from("internal error: first inlined here"));
                } else {
                    StyledString ss;
                    ss.out << "previously returned ";
                    stream_arg_types(ss.out, key.args);
                    location_message(key.label->anchor, ss.str());
                }
                {
                    StyledString ss;
                    ss.out << "cannot merge conditional branches returning ";
                    stream_arg_types(ss.out, key.args);
                    ss.out << "and ";
                    stream_arg_types(ss.out, la.args);
                    set_active_anchor(caller_anchor);
                    location_error(ss.str());
                }
            }
        }

        tmp_params.clear();

        srci = 0;
        for (size_t i = 0; i < label->params.size(); ++i) {
            Parameter *param = label->params[i];
            if (param->is_vararg()) {
                assert(i == lasti);
                size_t ncount = args.size();
                while (srci < ncount) {
                    auto &&value = tmp_args[srci];
                    if (is_unknown(value.value)) {
                        Parameter *newparam = Parameter::from(param);
                        tmp_params.push_back(newparam);
                        newparam->kind = PK_Regular;
                        newparam->type = value.value.typeref;
                        newparam->name = Symbol(SYM_Unnamed);
                        tmp_args[srci] = Argument(value.key, newparam);
                    }
                    srci++;
                }
            } else {
                if (srci < args.size()) {
                    auto &&value = tmp_args[srci];
                    if (is_unknown(value.value)) {
                        Parameter *newparam = Parameter::from(param);
                        tmp_params.push_back(newparam);
                        if (is_typed(value.value)) {
                            if (newparam->is_typed()
                                && (newparam->type != value.value.typeref)) {
                                StyledString ss;
                                ss.out << "attempting to retype parameter of type "
                                    << newparam->type << " as " << value.value.typeref;
                                location_error(ss.str());
                            } else {
                                newparam->type = value.value.typeref;
                            }
                        }
                        tmp_args[srci] = Argument(value.key, newparam);
                    } else if (!srci) {
                        Parameter *newparam = Parameter::from(param);
                        tmp_params.push_back(newparam);
                        newparam->type = TYPE_Nothing;
                    }
                }
                srci++;
            }
        }

        Label *newlabel = Label::from(label);
        newlabel->set_parameters(tmp_params);
        if (parent->inline_merge) {
            newlabel->unset_merge();
        }

        Frame *frame = Frame::from(parent, label, newlabel, loop_count);
        frame->args = tmp_args;

        if (label->is_debug()) {
            StyledStream ss(std::cerr);
            ss << "the label is contained in frame " << frame << std::endl;
        }

        parent->insert_frame(la, frame);

        evaluate_body(frame, newlabel->body, label->body);

        return frame;
    }

    // inlining the continuation of a branch label without arguments
    void verify_branch_continuation(const Closure *closure) {
        if (closure->label->is_inline())
            return;
        StyledString ss;
        ss.out << "branch destination must be inline" << std::endl;
        location_error(ss.str());
    }

    Any fold_type_return(Any dest, const Type *return_label) {
        //ss_cout << "type_return: " << dest << std::endl;
        if (dest.type == TYPE_Parameter) {
            Parameter *param = dest.parameter;
            if (param->is_none()) {
                location_error(String::from("attempting to type return continuation of non-returning label"));
            } else if (!param->is_typed()) {
                param->type = return_label;
                param->anchor = get_active_anchor();
            } else {
                const Type *ptype = param->type;
                if (return_label != ptype) {
                    // try to get a fit by unconsting types
                    return_label = cast<ReturnLabelType>(return_label)->to_unconst();
                    ptype = cast<ReturnLabelType>(ptype)->to_unconst();
                    if (return_label == ptype) {
                        param->type = return_label;
                        param->anchor = get_active_anchor();
                    } else {
                        {
                            StyledStream cerr(std::cerr);
                            cerr << param->anchor << " first typed here as " << ptype << std::endl;
                            param->anchor->stream_source_line(cerr);
                        }
                        {
                            StyledString ss;
                            ss.out << "attempting to retype return continuation as " << return_label;
                            location_error(ss.str());
                        }
                    }
                }
            }
        } else if (dest.type == TYPE_Closure) {
            assert(return_label->kind() == TK_ReturnLabel);
            const ReturnLabelType *rlt = cast<ReturnLabelType>(return_label);
            if (!rlt->is_returning()) {
                return none;
            }
            auto &&values = rlt->values;
            auto enter_frame = dest.closure->frame;
            auto enter_label = dest.closure->label;
            Args args;
            args.reserve(values.size() + 1);
            args = { Argument(untyped()) };
            for (size_t i = 0; i < values.size(); ++i) {
                args.push_back(values[i]);
            }
            dest = fold_type_label_single(enter_frame, enter_label, args);
        } else if (dest.type == TYPE_Label) {
            auto TR = dest.label->get_params_as_return_label_type();
            if (return_label != TR) {
                {
                    StyledStream cerr(std::cerr);
                    cerr << dest.label->anchor << " typed as " << TR << std::endl;
                    dest.label->anchor->stream_source_line(cerr);
                }
                {
                    StyledString ss;
                    ss.out << "attempting to retype label as " << return_label;
                    location_error(ss.str());
                }
            }
        } else {
            apply_type_error(dest);
        }
        return dest;
    }

    static void verify_integer_ops(Any x) {
        verify_integer_vector(storage_type(x.indirect_type()));
    }

    static void verify_real_ops(Any x) {
        verify_real_vector(storage_type(x.indirect_type()));
    }

    static void verify_integer_ops(Any a, Any b) {
        verify_integer_vector(storage_type(a.indirect_type()));
        verify(a.indirect_type(), b.indirect_type());
    }

    static void verify_real_ops(Any a, Any b) {
        verify_real_vector(storage_type(a.indirect_type()));
        verify(a.indirect_type(), b.indirect_type());
    }

    static void verify_real_ops(Any a, Any b, Any c) {
        verify_real_vector(storage_type(a.indirect_type()));
        verify(a.indirect_type(), b.indirect_type());
        verify(a.indirect_type(), c.indirect_type());
    }

    static bool has_keyed_args(Label *l) {
        auto &&args = l->body.args;
        for (size_t i = 1; i < args.size(); ++i) {
            if (args[i].key != SYM_Unnamed)
                return true;
        }
        return false;
    }

    static void verify_no_keyed_args(Label *l) {
        auto &&args = l->body.args;
        for (size_t i = 1; i < args.size(); ++i) {
            if (args[i].key != SYM_Unnamed) {
                location_error(String::from("unexpected keyed argument"));
            }
        }

    }

    static bool is_jumping(Label *l) {
        auto &&args = l->body.args;
        assert(!args.empty());
        return args[0].value.type == TYPE_Nothing;
    }

    static bool is_continuing_to_label(Label *l) {
        auto &&args = l->body.args;
        assert(!args.empty());
        return args[0].value.type == TYPE_Label;
    }

    static bool is_continuing_to_parameter(Label *l) {
        auto &&args = l->body.args;
        assert(!args.empty());
        return args[0].value.type == TYPE_Parameter;
    }

    static bool is_continuing_to_closure(Label *l) {
        auto &&args = l->body.args;
        assert(!args.empty());
        return args[0].value.type == TYPE_Closure;
    }

    static bool is_calling_closure(Label *l) {
        auto &&enter = l->body.enter;
        return enter.type == TYPE_Closure;
    }

    static bool is_calling_label(Label *l) {
        auto &&enter = l->body.enter;
        return enter.type == TYPE_Label;
    }

    static bool is_return_parameter(Any val) {
        return (val.type == TYPE_Parameter) && (val.parameter->index == 0);
    }

    static bool is_calling_continuation(Label *l) {
        auto &&enter = l->body.enter;
        return (enter.type == TYPE_Parameter) && (enter.parameter->index == 0);
    }

    static bool is_calling_builtin(Label *l) {
        auto &&enter = l->body.enter;
        return enter.type == TYPE_Builtin;
    }

    static bool is_calling_callable(Label *l) {
        if (l->body.is_rawcall())
            return false;
        auto &&enter = l->body.enter;
        const Type *T = enter.indirect_type();
        Any value = none;
        return T->lookup_call_handler(value);
    }

    static bool is_calling_function(Label *l) {
        auto &&enter = l->body.enter;
        return is_function_pointer(enter.indirect_type());
    }

    static bool is_calling_pure_function(Label *l) {
        auto &&enter = l->body.enter;
        return is_pure_function_pointer(enter.type);
    }

    static bool all_params_typed(Label *l) {
        auto &&params = l->params;
        for (size_t i = 1; i < params.size(); ++i) {
            if (!params[i]->is_typed())
                return false;
        }
        return true;
    }

    static size_t find_untyped_arg(Label *l) {
        auto &&args = l->body.args;
        for (size_t i = 1; i < args.size(); ++i) {
            if ((args[i].value.type == TYPE_Parameter)
                && (args[i].value.parameter->index != 0)
                && (!args[i].value.parameter->is_typed()))
                return i;
        }
        return 0;
    }

    static bool all_args_typed(Label *l) {
        return !find_untyped_arg(l);
    }

    static bool all_args_constant(Label *l) {
        auto &&args = l->body.args;
        for (size_t i = 1; i < args.size(); ++i) {
            if (!args[i].value.is_const())
                return false;
        }
        return true;
    }

    static bool has_foldable_args(Label *l) {
        auto &&args = l->body.args;
        for (size_t i = 1; i < args.size(); ++i) {
            if (args[i].value.is_const())
                return true;
            else if (is_return_parameter(args[i].value))
                return true;
        }
        return false;
    }

    static bool is_called_by(Label *callee, Label *caller) {
        auto &&enter = caller->body.enter;
        return (enter.type == TYPE_Label) && (enter.label == callee);
    }

    static bool is_called_by(Parameter *callee, Label *caller) {
        auto &&enter = caller->body.enter;
        return (enter.type == TYPE_Parameter) && (enter.parameter == callee);
    }

    static bool is_continuing_from(Label *callee, Label *caller) {
        auto &&args = caller->body.args;
        assert(!args.empty());
        return (args[0].value.type == TYPE_Label) && (args[0].value.label == callee);
    }

    static bool is_continuing_from(Parameter *callee, Label *caller) {
        auto &&args = caller->body.args;
        assert(!args.empty());
        return (args[0].value.type == TYPE_Parameter) && (args[0].value.parameter == callee);
    }

    void verify_function_argument_signature(const FunctionType *fi, Label *l) {
        auto &&args = l->body.args;
        verify_function_argument_count(fi, args.size() - 1);

        size_t fargcount = fi->argument_types.size();
        for (size_t i = 1; i < args.size(); ++i) {
            Argument &arg = args[i];
            size_t k = i - 1;
            const Type *argT = arg.value.indirect_type();
            if (k < fargcount) {
                const Type *ft = fi->argument_types[k];
                const Type *A = storage_type(argT);
                const Type *B = storage_type(ft);
                if (A == B)
                    continue;
                if ((A->kind() == TK_Pointer) && (B->kind() == TK_Pointer)) {
                    auto pa = cast<PointerType>(A);
                    auto pb = cast<PointerType>(B);
                    if ((pa->element_type == pb->element_type)
                        && (pa->flags == pb->flags)
                        && (pb->storage_class == SYM_Unnamed))
                        continue;
                }
                StyledString ss;
                ss.out << "argument of type " << ft << " expected, got " << argT;
                location_error(ss.str());
            }
        }
    }

    void fold_pure_function_call(Label *l) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "folding pure function call in " << l << std::endl;
#endif

        auto &&enter = l->body.enter;
        auto &&args = l->body.args;

        auto pi = cast<PointerType>(enter.type);
        auto fi = cast<FunctionType>(pi->element_type);

        verify_function_argument_signature(fi, l);

        assert(!args.empty());
        Any result = none;

        if (fi->flags & FF_Variadic) {
            // convert C types
            size_t argcount = args.size() - 1;
            Args cargs;
            cargs.reserve(argcount);
            for (size_t i = 0; i < argcount; ++i) {
                Argument &srcarg = args[i + 1];
                if (i >= fi->argument_types.size()) {
                    if (srcarg.value.type == TYPE_F32) {
                        cargs.push_back(Argument(srcarg.key, (double)srcarg.value.f32));
                        continue;
                    }
                }
                cargs.push_back(srcarg);
            }
            result = run_ffi_function(enter, &cargs[0], cargs.size());
        } else {
            result = run_ffi_function(enter, &args[1], args.size() - 1);
        }

        enter = args[0].value;
        args = { Argument() };
        auto rlt = cast<ReturnLabelType>(fi->return_type);
        if (rlt->return_type != TYPE_Void) {
            if (isa<TupleType>(rlt->return_type)) {
                // unpack
                auto ti = cast<TupleType>(rlt->return_type);
                size_t count = ti->types.size();
                for (size_t i = 0; i < count; ++i) {
                    args.push_back(Argument(ti->unpack(result.pointer, i)));
                }
            } else {
                args.push_back(Argument(result));
            }
        }
    }

    void solve_keyed_args(Label *l) {
        Label *enter = l->get_closure_enter()->label;

        auto &&args = l->body.args;
        assert(!args.empty());
        Args newargs;
        newargs.reserve(args.size());
        newargs.push_back(args[0]);
        Parameter *vaparam = nullptr;
        if (!enter->params.empty() && enter->params.back()->is_vararg()) {
            vaparam = enter->params.back();
        }
        std::vector<bool> mapped;
        mapped.reserve(args.size());
        mapped.push_back(true);
        size_t next_index = 1;
        for (size_t i = 1; i < args.size(); ++i) {
            auto &&arg = args[i];
            if (arg.key == SYM_Unnamed) {
                while ((next_index < mapped.size()) && mapped[next_index])
                    next_index++;
                while (mapped.size() <= next_index) {
                    mapped.push_back(false);
                    newargs.push_back(none);
                }
                mapped[next_index] = true;
                newargs[next_index] = arg;
                next_index++;
            } else {
                auto param = enter->get_param_by_name(arg.key);
                size_t index = -1;
                if (param && (param != vaparam)) {
                    while (mapped.size() <= (size_t)param->index) {
                        mapped.push_back(false);
                        newargs.push_back(none);
                    }
                    if (mapped[param->index]) {
                        StyledString ss;
                        ss.out << "duplicate binding to parameter " << arg.key;
                        location_error(ss.str());
                    }
                    index = param->index;
                } else if (vaparam) {
                    while (mapped.size() < (size_t)vaparam->index) {
                        mapped.push_back(false);
                        newargs.push_back(none);
                    }
                    index = newargs.size();
                    mapped.push_back(false);
                    newargs.push_back(none);
                    newargs[index].key = arg.key;
                } else {
                    // no such parameter, map like regular parameter
                    while ((next_index < mapped.size()) && mapped[next_index])
                        next_index++;
                    while (mapped.size() <= next_index) {
                        mapped.push_back(false);
                        newargs.push_back(none);
                    }
                    index = next_index;
                    newargs[index].key = SYM_Unnamed;
                    next_index++;
                }
                mapped[index] = true;
                newargs[index].value = arg.value;
            }
        }
        args = newargs;
    }

    bool is_indirect_closure_type(const Type *T) {
        if (is_opaque(T)) return false;
        if (T == TYPE_Closure) return true;
        T = storage_type(T);
        const Type *ST = storage_type(TYPE_Closure);
        if (T == ST) return true;
        // TODO: detect closures in aggregate types
        return false;
    }

    void validate_label_return_types(Label *l) {
        assert(!l->is_basic_block_like());
        assert(l->is_return_param_typed());
        const ReturnLabelType *rlt = cast<ReturnLabelType>(l->params[0]->type);
        for (size_t i = 0; i < rlt->values.size(); ++i) {
            auto &&val = rlt->values[i].value;
            bool needs_inline = false;
            const char *name = nullptr;
            const Type *displayT = nullptr;
            if (is_indirect_closure_type(val.type)) {
                needs_inline = true;
                name = "closure";
                displayT = val.type;
            } else if (is_unknown(val)) {
                auto T = val.typeref;
                if (!is_opaque(T)) {
                    T = storage_type(T);
                    if (T->kind() == TK_Pointer) {
                        auto pt = cast<PointerType>(T);
                        if (pt->storage_class != SYM_Unnamed) {
                            needs_inline = true;
                            name = "pointer";
                            displayT = val.typeref;
                        }
                    }
                }
            }
            if (needs_inline) {
                StyledString ss;
                set_active_anchor(l->anchor);
                ss.out << "return argument #" << i << " is of non-returnable " << name << " type "
                    << displayT << " but function is not being inlined" << std::endl;
                location_error(ss.str());
            }
        }
    }

    bool frame_args_match_keys(const Args &args, const Args &keys) const {
        if (args.size() != keys.size())
            return false;
        for (size_t i = 1; i < keys.size(); ++i) {
            auto &&arg = args[i].value;
            auto &&key = keys[i].value;
            if (is_unknown(key)
                && !arg.is_const()
                && (arg.parameter->type == key.typeref))
                continue;
            if (args[i].value != keys[i].value)
                return false;
        }
        return true;
    }

    const Type *fold_closure_call(Label *l, bool &recursive) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "folding & typing arguments in " << l << std::endl;
#endif

        auto &&enter = l->body.enter;
        assert(enter.type == TYPE_Closure);
        Frame *enter_frame = enter.closure->frame;
        Label *enter_label = enter.closure->label;

        bool inline_const = (!enter_label->is_merge()) || enter_frame->inline_merge;

        // inline constant arguments
        Args callargs;
        Args keys;
        auto &&args = l->body.args;
#if 0
        if (enter_label->is_inline() && inline_const) {
            callargs.push_back(none);
            keys.push_back(args[0]);
            for (size_t i = 1; i < args.size(); ++i) {
                keys.push_back(args[i]);
            }
        } else
#endif
        {
            callargs.push_back(args[0]);
            keys.push_back(Argument(untyped()));
            for (size_t i = 1; i < args.size(); ++i) {
                auto &&arg = args[i];
                if (arg.value.is_const() && inline_const) {
                    keys.push_back(arg);
                } else if (is_return_parameter(arg.value)) {
                    keys.push_back(arg);
                } else {
                    keys.push_back(Argument(arg.key,
                        unknown_of(arg.value.indirect_type())));
                    callargs.push_back(arg);
                }
            }
            #if 1
            if (enter_label->is_inline()) {
                callargs[0] = none;
                keys[0] = args[0];
            }
            #endif
        }

#if 0
        bool is_function_entry = !enter_label->is_basic_block_like();
#endif

        Frame *newf = fold_type_label_single_frame(
            enter_frame, enter_label, keys);
        Label *newl = newf->get_instance();

#if 0
        if (newf == enter_frame) {
            location_error(String::from("label or function forms an infinite but empty loop"));
        }
#endif

        // labels points do not need to be entered
        if (newl->is_basic_block_like()) {
            enter = newl;
            args = callargs;
            //clear_continuation_arg(l);
            return nullptr;
        } else {
            // newl is a function

            // we need to solve body, return type and reentrant flags for the
            // section that follows
            normalize_function(newf);

            // function is done, but not typed
            if (!newl->is_return_param_typed()) {
                // two possible reasons for this:
                // 2. all return paths are unreachable, because the function
                //    never returns.
                //    but this is not the case, because the function has not been
                //    typed yet as not returning.

                // 1. recursion - entry label has already been
                //    processed, but not exited yet, so we don't have the
                //    continuation type yet.
                // try again when all outstanding branches have been processed
                recursive = true;
                return nullptr;
            } else {
                validate_label_return_types(newl);
            }

            const Type *rtype = newl->get_return_type();
            if (is_empty_function(newl)) {
    #if 1
                if (enable_step_debugger) {
                    StyledStream ss(std::cerr);
                    ss << "folding call to empty function:" << std::endl;
                    stream_label(ss, newl, StreamLabelFormat::debug_scope());
                }
    #endif
                // function performs no work, fold
                enter = args[0].value;
                args = { none };
                const ReturnLabelType *rlt = cast<ReturnLabelType>(rtype);
                auto &&values = rlt->values;
                for (size_t i = 0; i < values.size(); ++i) {
                    auto &&arg = values[i];
                    assert(!is_unknown(arg.value));
                    args.push_back(arg);
                }
            } else {
                enter = newl;
                args = callargs;
                if (rtype == NoReturnLabel()) {
                    rtype = nullptr;
                }
            }
            return rtype;
        }
    }

    // returns true if the builtin folds regardless of whether the arguments are
    // constant
    bool builtin_always_folds(Builtin builtin) {
        switch(builtin.value()) {
        case FN_TypeOf:
        case FN_NullOf:
        case FN_IsConstant:
        case FN_VaCountOf:
        case FN_VaKeys:
        case FN_VaKey:
        case FN_VaValues:
        case FN_VaAt:
        case FN_Location:
        case FN_Dump:
        case FN_ExternNew:
        case FN_ExternSymbol:
        case FN_TupleType:
        case FN_UnionType:
        case FN_StaticAlloc:
            return true;
        default: return false;
        }
    }

    bool builtin_has_keyed_args(Builtin builtin) {
        switch(builtin.value()) {
        case FN_VaCountOf:
        case FN_VaKeys:
        case FN_VaValues:
        case FN_VaAt:
        case FN_Dump:
        case FN_ExternNew:
        case FN_ReturnLabelType:
        case FN_ScopeOf:
        case FN_TupleType:
        case FN_UnionType:
        case FN_Sample:
        case FN_ImageQuerySize:
            return true;
        default: return false;
        }
    }

    bool builtin_never_folds(Builtin builtin) {
        switch(builtin.value()) {
        case FN_Bitcast:
        case FN_Unconst:
        case FN_Undef:
        case FN_Alloca:
        case FN_AllocaExceptionPad:
        case FN_AllocaArray:
        case FN_Malloc:
        case FN_MallocArray:
        case SFXFN_Unreachable:
        case SFXFN_Discard:
        case FN_VolatileStore:
        case FN_Store:
        case FN_VolatileLoad:
        case FN_Load:
        case FN_Sample:
        case FN_ImageRead:
        case FN_ImageQuerySize:
        case FN_ImageQueryLod:
        case FN_ImageQueryLevels:
        case FN_ImageQuerySamples:
        case FN_GetElementPtr:
        case SFXFN_ExecutionMode:
        case OP_Tertiary:
            return true;
        default: return false;
        }
    }

    void verify_readable(const Type *T) {
        auto pi = cast<PointerType>(T);
        if (!pi->is_readable()) {
            StyledString ss;
            ss.out << "can not load value from address of type " << T
                << " because the target is non-readable";
            location_error(ss.str());
        }
    }

    void verify_writable(const Type *T) {
        auto pi = cast<PointerType>(T);
        if (!pi->is_writable()) {
            StyledString ss;
            ss.out << "can not store value at address of type " << T
                << " because the target is non-writable";
            location_error(ss.str());
        }
    }

    // reduce typekind to compatible
    static TypeKind canonical_typekind(TypeKind k) {
        if (k == TK_Real)
            return TK_Integer;
        return k;
    }

#define CHECKARGS(MINARGS, MAXARGS) \
    checkargs<MINARGS, MAXARGS>(args.size())

#define RETARGTYPES(...) \
    { \
        const Type *retargtypes[] = { __VA_ARGS__ }; \
        size_t _count = (sizeof(retargtypes) / sizeof(const Type *)); \
        retvalues.reserve(_count); \
        for (size_t _i = 0; _i < _count; ++_i) { \
            retvalues.push_back(unknown_of(retargtypes[_i])); \
        } \
    }
#define RETARGS(...) \
    retvalues = { __VA_ARGS__ }; \
    return true;

    // returns true if the call can be eliminated
    bool values_from_builtin_call(Label *l, Args &retvalues) {
        auto &&enter = l->body.enter;
        auto &&args = l->body.args;
        assert(enter.type == TYPE_Builtin);
        switch(enter.builtin.value()) {
        case FN_Sample: {
            CHECKARGS(2, -1);
            auto ST = storage_type(args[1].value.indirect_type());
            if (ST->kind() == TK_SampledImage) {
                auto sit = cast<SampledImageType>(ST);
                ST = storage_type(sit->type);
            }
            verify_kind<TK_Image>(ST);
            auto it = cast<ImageType>(ST);
            RETARGTYPES(it->type);
        } break;
        case FN_ImageQuerySize: {
            CHECKARGS(1, -1);
            auto ST = storage_type(args[1].value.indirect_type());
            if (ST->kind() == TK_SampledImage) {
                auto sit = cast<SampledImageType>(ST);
                ST = storage_type(sit->type);
            }
            verify_kind<TK_Image>(ST);
            auto it = cast<ImageType>(ST);
            spv::Dim dim = SPIRVGenerator::dim_from_symbol(it->dim);
            int comps = 0;
            switch(dim) {
            case spv::Dim1D:
            case spv::DimBuffer:
                comps = 1;
                break;
            case spv::Dim2D:
            case spv::DimCube:
            case spv::DimRect:
            case spv::DimSubpassData:
                comps = 2;
                break;
            case spv::Dim3D:
                comps = 3;
                break;
            default: break;
            }
            if (it->arrayed) {
                comps++;
            }
            if (comps == 1) {
                RETARGTYPES(TYPE_I32);
            } else {
                RETARGTYPES(Vector(TYPE_I32, comps));
            }
        } break;
        case FN_ImageQueryLod: {
            CHECKARGS(2, 2);
            auto ST = storage_type(args[1].value.indirect_type());
            if (ST->kind() == TK_SampledImage) {
                auto sit = cast<SampledImageType>(ST);
                ST = storage_type(sit->type);
            }
            verify_kind<TK_Image>(ST);
            RETARGTYPES(Vector(TYPE_F32, 2));
        } break;
        case FN_ImageQueryLevels:
        case FN_ImageQuerySamples: {
            CHECKARGS(1, 1);
            auto ST = storage_type(args[1].value.indirect_type());
            if (ST->kind() == TK_SampledImage) {
                auto sit = cast<SampledImageType>(ST);
                ST = storage_type(sit->type);
            }
            verify_kind<TK_Image>(ST);
            RETARGTYPES(TYPE_I32);
        } break;
        case FN_ImageRead: {
            CHECKARGS(2, 2);
            auto ST = storage_type(args[1].value.indirect_type());
            verify_kind<TK_Image>(ST);
            auto it = cast<ImageType>(ST);
            RETARGTYPES(it->type);
        } break;
        case SFXFN_ExecutionMode: {
            CHECKARGS(1, 4);
            args[1].value.verify(TYPE_Symbol);
            SPIRVGenerator::execution_mode_from_symbol(args[1].value.symbol);
            for (size_t i = 2; i < args.size(); ++i) {
                cast_number<int>(args[i].value);
            }
            RETARGTYPES();
        } break;
        case OP_Tertiary: {
            CHECKARGS(3, 3);
            auto &&cond = args[1].value;
            if (cond.is_const() &&
                ((cond.type == TYPE_Bool)
                    || (args[2].value.is_const() && args[3].value.is_const()))) {
                if (cond.type == TYPE_Bool) {
                    if (cond.i1) {
                        RETARGS(args[2].value);
                    } else {
                        RETARGS(args[3].value);
                    }
                } else {
                    auto T1 = storage_type(cond.type);
                    auto T2 = storage_type(args[2].value.type);
                    verify_bool_vector(T1);
                    verify(args[2].value.type, args[3].value.type);
                    if (T1->kind() == TK_Vector) {
                        verify_vector_sizes(T1, T2);
                    } else if (T2->kind() == TK_Vector) {
                        cond = smear(cond, cast<VectorType>(T2)->count);
                    }
                    Any fargs[] = { cond, args[2].value, args[3].value };
                    RETARGS(apply_op< DispatchSelect >(fargs, 3));
                }
            } else {
                auto T1 = storage_type(args[1].value.indirect_type());
                auto T2 = storage_type(args[2].value.indirect_type());
                auto T3 = storage_type(args[3].value.indirect_type());
                verify_bool_vector(T1);
                if (T1->kind() == TK_Vector) {
                    verify_vector_sizes(T1, T2);
                }
                verify(T2, T3);
                RETARGTYPES(args[2].value.indirect_type());
            }
        } break;
        case FN_Unconst: {
            CHECKARGS(1, 1);
            if (!args[1].value.is_const()) {
                RETARGS(args[1]);
            } else {
                auto T = args[1].value.indirect_type();
                auto et = dyn_cast<ExternType>(T);
                if (et) {
                    RETARGTYPES(et->pointer_type);
                } else if (args[1].value.type == TYPE_Label) {
                    Label *fn = args[1].value;
                    fn->verify_compilable();
                    const Type *functype = Pointer(
                        fn->get_function_type(), PTF_NonWritable, SYM_Unnamed);
                    RETARGTYPES(functype);
                } else {
                    RETARGTYPES(T);
                }
            }
        } break;
        case FN_Bitcast: {
            CHECKARGS(2, 2);
            // todo: verify source and dest type are non-aggregate
            // also, both must be of same category
            args[2].value.verify(TYPE_Type);
            const Type *SrcT = args[1].value.indirect_type();
            const Type *DestT = args[2].value.typeref;
            if (SrcT == DestT) {
                RETARGS(args[1].value);
            } else {
                const Type *SSrcT = storage_type(SrcT);
                const Type *SDestT = storage_type(DestT);

                if (canonical_typekind(SSrcT->kind())
                        != canonical_typekind(SDestT->kind())) {
                    StyledString ss;
                    ss.out << "can not bitcast value of type " << SrcT
                        << " to type " << DestT
                        << " because storage types are not of compatible category";
                    location_error(ss.str());
                }
                if (SSrcT != SDestT) {
                    switch (SDestT->kind()) {
                    case TK_Array:
                    //case TK_Vector:
                    case TK_Tuple:
                    case TK_Union: {
                        StyledString ss;
                        ss.out << "can not bitcast value of type " << SrcT
                            << " to type " << DestT
                            << " with aggregate storage type " << SDestT;
                        location_error(ss.str());
                    } break;
                    default: break;
                    }
                }
                if (args[1].value.is_const()) {
                    Any result = args[1].value;
                    result.type = DestT;
                    RETARGS(result);
                } else {
                    RETARGTYPES(DestT);
                }
            }
        } break;
        case FN_IntToPtr: {
            CHECKARGS(2, 2);
            verify_integer(storage_type(args[1].value.indirect_type()));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_kind<TK_Pointer>(storage_type(DestT));
            RETARGTYPES(DestT);
        } break;
        case FN_PtrToInt: {
            CHECKARGS(2, 2);
            verify_kind<TK_Pointer>(
                storage_type(args[1].value.indirect_type()));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(storage_type(DestT));
            RETARGTYPES(DestT);
        } break;
        case FN_ITrunc: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_integer(storage_type(T));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(storage_type(DestT));
            RETARGTYPES(DestT);
        } break;
        case FN_FPTrunc: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            if (cast<RealType>(T)->width >= cast<RealType>(DestT)->width) {
            } else { invalid_op2_types_error(T, DestT); }
            RETARGTYPES(DestT);
        } break;
        case FN_FPExt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            if (cast<RealType>(T)->width <= cast<RealType>(DestT)->width) {
            } else { invalid_op2_types_error(T, DestT); }
            RETARGTYPES(DestT);
        } break;
        case FN_FPToUI: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(DestT);
            if ((T == TYPE_F32) || (T == TYPE_F64)) {
            } else {
                invalid_op2_types_error(T, DestT);
            }
            RETARGTYPES(DestT);
        } break;
        case FN_FPToSI: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(DestT);
            if ((T == TYPE_F32) || (T == TYPE_F64)) {
            } else {
                invalid_op2_types_error(T, DestT);
            }
            RETARGTYPES(DestT);
        } break;
        case FN_UIToFP: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_integer(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            if ((DestT == TYPE_F32) || (DestT == TYPE_F64)) {
            } else {
                invalid_op2_types_error(T, DestT);
            }
            RETARGTYPES(DestT);
        } break;
        case FN_SIToFP: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_integer(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            if ((DestT == TYPE_F32) || (DestT == TYPE_F64)) {
            } else {
                invalid_op2_types_error(T, DestT);
            }
            RETARGTYPES(DestT);
        } break;
        case FN_ZExt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_integer(storage_type(T));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(storage_type(DestT));
            RETARGTYPES(DestT);
        } break;
        case FN_SExt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.indirect_type();
            verify_integer(storage_type(T));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(storage_type(DestT));
            RETARGTYPES(DestT);
        } break;
        case FN_ExtractElement: {
            CHECKARGS(2, 2);
            const Type *T = storage_type(args[1].value.indirect_type());
            verify_kind<TK_Vector>(T);
            auto vi = cast<VectorType>(T);
            verify_integer(storage_type(args[2].value.indirect_type()));
            RETARGTYPES(vi->element_type);
        } break;
        case FN_InsertElement: {
            CHECKARGS(3, 3);
            const Type *T = storage_type(args[1].value.indirect_type());
            const Type *ET = storage_type(args[2].value.indirect_type());
            verify_integer(storage_type(args[3].value.indirect_type()));
            verify_kind<TK_Vector>(T);
            auto vi = cast<VectorType>(T);
            verify(storage_type(vi->element_type), ET);
            RETARGTYPES(args[1].value.indirect_type());
        } break;
        case FN_ShuffleVector: {
            CHECKARGS(3, 3);
            const Type *TV1 = storage_type(args[1].value.indirect_type());
            const Type *TV2 = storage_type(args[2].value.indirect_type());
            const Type *TMask = storage_type(args[3].value.type);
            verify_kind<TK_Vector>(TV1);
            verify_kind<TK_Vector>(TV2);
            verify_kind<TK_Vector>(TMask);
            verify(TV1, TV2);
            auto vi = cast<VectorType>(TV1);
            auto mask_vi = cast<VectorType>(TMask);
            verify(TYPE_I32, mask_vi->element_type);
            size_t incount = vi->count * 2;
            size_t outcount = mask_vi->count;
            for (size_t i = 0; i < outcount; ++i) {
                verify_range(
                    (size_t)mask_vi->unpack(args[3].value.pointer, i).i32,
                    incount);
            }
            RETARGTYPES(Vector(vi->element_type, outcount));
        } break;
        case FN_ExtractValue: {
            CHECKARGS(2, 2);
            size_t idx = cast_number<size_t>(args[2].value);
            const Type *T = storage_type(args[1].value.indirect_type());
            switch(T->kind()) {
            case TK_Array: {
                auto ai = cast<ArrayType>(T);
                RETARGTYPES(ai->type_at_index(idx));
            } break;
            case TK_Tuple: {
                auto ti = cast<TupleType>(T);
                RETARGTYPES(ti->type_at_index(idx));
            } break;
            case TK_Union: {
                auto ui = cast<UnionType>(T);
                RETARGTYPES(ui->type_at_index(idx));
            } break;
            default: {
                StyledString ss;
                ss.out << "can not extract value from type " << T;
                location_error(ss.str());
            } break;
            }
        } break;
        case FN_InsertValue: {
            CHECKARGS(3, 3);
            const Type *T = storage_type(args[1].value.indirect_type());
            const Type *ET = storage_type(args[2].value.indirect_type());
            size_t idx = cast_number<size_t>(args[3].value);
            switch(T->kind()) {
            case TK_Array: {
                auto ai = cast<ArrayType>(T);
                verify(storage_type(ai->type_at_index(idx)), ET);
            } break;
            case TK_Tuple: {
                auto ti = cast<TupleType>(T);
                verify(storage_type(ti->type_at_index(idx)), ET);
            } break;
            case TK_Union: {
                auto ui = cast<UnionType>(T);
                verify(storage_type(ui->type_at_index(idx)), ET);
            } break;
            default: {
                StyledString ss;
                ss.out << "can not insert value into type " << T;
                location_error(ss.str());
            } break;
            }
            RETARGTYPES(args[1].value.indirect_type());
        } break;
        case FN_Undef: {
            CHECKARGS(1, 1);
            args[1].value.verify(TYPE_Type);
            RETARGTYPES(args[1].value.typeref);
        } break;
        case FN_Malloc: {
            CHECKARGS(1, 1);
            args[1].value.verify(TYPE_Type);
            RETARGTYPES(NativePointer(args[1].value.typeref));
        } break;
        case FN_Alloca: {
            CHECKARGS(1, 1);
            args[1].value.verify(TYPE_Type);
            RETARGTYPES(LocalPointer(args[1].value.typeref));
        } break;
        case FN_AllocaExceptionPad: {
            CHECKARGS(0, 0);
            RETARGTYPES(LocalPointer(Array(TYPE_U8, sizeof(ExceptionPad))));
        } break;
        case FN_AllocaOf: {
            CHECKARGS(1, 1);
            RETARGTYPES(LocalROPointer(args[1].value.indirect_type()));
        } break;
        case FN_MallocArray: {
            CHECKARGS(2, 2);
            args[1].value.verify(TYPE_Type);
            verify_integer(storage_type(args[2].value.indirect_type()));
            RETARGTYPES(NativePointer(args[1].value.typeref));
        } break;
        case FN_AllocaArray: {
            CHECKARGS(2, 2);
            args[1].value.verify(TYPE_Type);
            verify_integer(storage_type(args[2].value.indirect_type()));
            RETARGTYPES(LocalPointer(args[1].value.typeref));
        } break;
        case FN_Free: {
            CHECKARGS(1, 1);
            const Type *T = storage_type(args[1].value.indirect_type());
            verify_kind<TK_Pointer>(T);
            verify_writable(T);
            if (cast<PointerType>(T)->storage_class != SYM_Unnamed) {
                location_error(String::from(
                    "pointer is not a heap pointer"));
            }
            RETARGTYPES();
        } break;
        case FN_GetElementPtr: {
            CHECKARGS(2, -1);
            const Type *T = storage_type(args[1].value.indirect_type());
            bool is_extern = (T->kind() == TK_Extern);
            if (is_extern) {
                T = cast<ExternType>(T)->pointer_type;
            }
            verify_kind<TK_Pointer>(T);
            auto pi = cast<PointerType>(T);
            T = pi->element_type;
            bool all_const = args[1].value.is_const();
            if (all_const) {
                for (size_t i = 2; i < args.size(); ++i) {
                    if (!args[i].value.is_const()) {
                        all_const = false;
                        break;
                    }
                }
            }
            if (!is_extern && all_const) {
                void *ptr = args[1].value.pointer;
                size_t idx = cast_number<size_t>(args[2].value);
                ptr = pi->getelementptr(ptr, idx);

                for (size_t i = 3; i < args.size(); ++i) {
                    const Type *ST = storage_type(T);
                    auto &&arg = args[i].value;
                    switch(ST->kind()) {
                    case TK_Array: {
                        auto ai = cast<ArrayType>(ST);
                        T = ai->element_type;
                        size_t idx = cast_number<size_t>(arg);
                        ptr = ai->getelementptr(ptr, idx);
                    } break;
                    case TK_Tuple: {
                        auto ti = cast<TupleType>(ST);
                        size_t idx = 0;
                        if (arg.type == TYPE_Symbol) {
                            idx = ti->field_index(arg.symbol);
                            if (idx == (size_t)-1) {
                                StyledString ss;
                                ss.out << "no such field " << arg.symbol << " in storage type " << ST;
                                location_error(ss.str());
                            }
                            // rewrite field
                            arg = (int)idx;
                        } else {
                            idx = cast_number<size_t>(arg);
                        }
                        T = ti->type_at_index(idx);
                        ptr = ti->getelementptr(ptr, idx);
                    } break;
                    default: {
                        StyledString ss;
                        ss.out << "can not get element pointer from type " << T;
                        location_error(ss.str());
                    } break;
                    }
                }
                T = Pointer(T, pi->flags, pi->storage_class);
                RETARGS(Any::from_pointer(T, ptr));
            } else {
                verify_integer(storage_type(args[2].value.indirect_type()));
                for (size_t i = 3; i < args.size(); ++i) {

                    const Type *ST = storage_type(T);
                    auto &&arg = args[i];
                    switch(ST->kind()) {
                    case TK_Array: {
                        auto ai = cast<ArrayType>(ST);
                        T = ai->element_type;
                        verify_integer(storage_type(arg.value.indirect_type()));
                    } break;
                    case TK_Tuple: {
                        auto ti = cast<TupleType>(ST);
                        size_t idx = 0;
                        if (arg.value.type == TYPE_Symbol) {
                            idx = ti->field_index(arg.value.symbol);
                            if (idx == (size_t)-1) {
                                StyledString ss;
                                ss.out << "no such field " << arg.value.symbol << " in storage type " << ST;
                                location_error(ss.str());
                            }
                            // rewrite field
                            arg = Argument(arg.key, Any((int)idx));
                        } else {
                            idx = cast_number<size_t>(arg.value);
                        }
                        T = ti->type_at_index(idx);
                    } break;
                    default: {
                        StyledString ss;
                        ss.out << "can not get element pointer from type " << T;
                        location_error(ss.str());
                    } break;
                    }
                }
                T = Pointer(T, pi->flags, pi->storage_class);
                RETARGTYPES(T);
            }
        } break;
        case FN_VolatileLoad:
        case FN_Load: {
            CHECKARGS(1, 1);
            const Type *T = storage_type(args[1].value.indirect_type());
            bool is_extern = (T->kind() == TK_Extern);
            if (is_extern) {
                T = cast<ExternType>(T)->pointer_type;
            }
            verify_kind<TK_Pointer>(T);
            verify_readable(T);
            auto pi = cast<PointerType>(T);
            if (!is_extern && args[1].value.is_const()
                && !pi->is_writable()) {
                RETARGS(pi->unpack(args[1].value.pointer));
            } else {
                RETARGTYPES(pi->element_type);
            }
        } break;
        case FN_VolatileStore:
        case FN_Store: {
            CHECKARGS(2, 2);
            const Type *T = storage_type(args[2].value.indirect_type());
            bool is_extern = (T->kind() == TK_Extern);
            if (is_extern) {
                T = cast<ExternType>(T)->pointer_type;
            }
            verify_kind<TK_Pointer>(T);
            verify_writable(T);
            auto pi = cast<PointerType>(T);
            verify(storage_type(pi->element_type),
                storage_type(args[1].value.indirect_type()));
            RETARGTYPES();
        } break;
        case FN_Cross: {
            CHECKARGS(2, 2);
            const Type *Ta = storage_type(args[1].value.indirect_type());
            const Type *Tb = storage_type(args[2].value.indirect_type());
            verify_real_vector(Ta, 3);
            verify(Ta, Tb);
            RETARGTYPES(args[1].value.indirect_type());
        } break;
        case FN_Normalize: {
            CHECKARGS(1, 1);
            verify_real_ops(args[1].value);
            RETARGTYPES(args[1].value.indirect_type());
        } break;
        case FN_Length: {
            CHECKARGS(1, 1);
            const Type *T = storage_type(args[1].value.indirect_type());
            verify_real_vector(T);
            if (T->kind() == TK_Vector) {
                RETARGTYPES(cast<VectorType>(T)->element_type);
            } else {
                RETARGTYPES(args[1].value.indirect_type());
            }
        } break;
        case FN_Distance: {
            CHECKARGS(2, 2);
            verify_real_ops(args[1].value, args[2].value);
            const Type *T = storage_type(args[1].value.indirect_type());
            if (T->kind() == TK_Vector) {
                RETARGTYPES(cast<VectorType>(T)->element_type);
            } else {
                RETARGTYPES(args[1].value.indirect_type());
            }
        } break;
        case OP_ICmpEQ:
        case OP_ICmpNE:
        case OP_ICmpUGT:
        case OP_ICmpUGE:
        case OP_ICmpULT:
        case OP_ICmpULE:
        case OP_ICmpSGT:
        case OP_ICmpSGE:
        case OP_ICmpSLT:
        case OP_ICmpSLE: {
            CHECKARGS(2, 2);
            verify_integer_ops(args[1].value, args[2].value);
            RETARGTYPES(
                bool_op_return_type(args[1].value.indirect_type()));
        } break;
        case OP_FCmpOEQ:
        case OP_FCmpONE:
        case OP_FCmpORD:
        case OP_FCmpOGT:
        case OP_FCmpOGE:
        case OP_FCmpOLT:
        case OP_FCmpOLE:
        case OP_FCmpUEQ:
        case OP_FCmpUNE:
        case OP_FCmpUNO:
        case OP_FCmpUGT:
        case OP_FCmpUGE:
        case OP_FCmpULT:
        case OP_FCmpULE: {
            CHECKARGS(2, 2);
            verify_real_ops(args[1].value, args[2].value);
            RETARGTYPES(
                bool_op_return_type(args[1].value.indirect_type()));
        } break;
#define IARITH_NUW_NSW_OPS(NAME) \
    case OP_ ## NAME: \
    case OP_ ## NAME ## NUW: \
    case OP_ ## NAME ## NSW: { \
        CHECKARGS(2, 2); \
        verify_integer_ops(args[1].value, args[2].value); \
        RETARGTYPES(args[1].value.indirect_type()); \
    } break;
#define IARITH_OP(NAME, PFX) \
    case OP_ ## NAME: { \
        CHECKARGS(2, 2); \
        verify_integer_ops(args[1].value, args[2].value); \
        RETARGTYPES(args[1].value.indirect_type()); \
    } break;
#define FARITH_OP(NAME) \
    case OP_ ## NAME: { \
        CHECKARGS(2, 2); \
        verify_real_ops(args[1].value, args[2].value); \
        RETARGTYPES(args[1].value.indirect_type()); \
    } break;
#define FTRI_OP(NAME) \
    case OP_ ## NAME: { \
        CHECKARGS(3, 3); \
        verify_real_ops(args[1].value, args[2].value, args[3].value); \
        RETARGTYPES(args[1].value.indirect_type()); \
    } break;
#define IUN_OP(NAME, PFX) \
    case OP_ ## NAME: { \
        CHECKARGS(1, 1); \
        verify_integer_ops(args[1].value); \
        RETARGTYPES(args[1].value.indirect_type()); \
    } break;
#define FUN_OP(NAME) \
    case OP_ ## NAME: { \
        CHECKARGS(1, 1); \
        verify_real_ops(args[1].value); \
        RETARGTYPES(args[1].value.indirect_type()); \
    } break;
            B_ARITH_OPS()

#undef IARITH_NUW_NSW_OPS
#undef IARITH_OP
#undef FARITH_OP
#undef IUN_OP
#undef FUN_OP
#undef FTRI_OP
        default: {
            StyledString ss;
            ss.out << "can not type builtin " << enter.builtin;
            location_error(ss.str());
        } break;
        }

        return false;
    }
#undef RETARGS
#undef RETARGTYPES

#define RETARGS(...) \
    enter = args[0].value; \
    args = { none, __VA_ARGS__ };


    static void print_traceback_entry(StyledStream &ss, const Trace &trace) {
        ss << trace.anchor << " in ";
        if (trace.name == SYM_Unnamed) {
            ss << "unnamed";
        } else {
            ss << Style_Function << trace.name.name()->data << Style_None;
        }
        ss << std::endl;
        trace.anchor->stream_source_line(ss);
    }

    static void print_traceback() {
        if (traceback.empty()) return;
        StyledStream ss(std::cerr);
        ss << "Traceback (most recent call last):" << std::endl;

        size_t sz = traceback.size();
    #if 0
        size_t lasti = sz - 1;
        size_t i = 0;
        Label *l = traceback[lasti - i];
        Label *last_head = l;
        Label *last_loc = l;
        i++;
        while (i < sz) {
            l = traceback[lasti - i++];
            Label *orig = l->get_original();
            if (!orig->is_basic_block_like()) {
                print_traceback_entry(ss, last_head, last_loc);
                last_head = l;
            }
            last_loc = l;
            if (is_calling_label(orig)
                && !orig->get_label_enter()->is_basic_block_like()) {
                if (!last_head)
                    last_head = last_loc;
                print_traceback_entry(ss, last_head, last_loc);
                last_head = nullptr;
            }
        }
        print_traceback_entry(ss, last_head, last_loc);
    #else
        std::unordered_set<const Anchor *> visited;
        size_t i = sz;
        size_t capped = 0;
        while (i != 0) {
            i--;
            auto &&trace = traceback[i];
            if (!visited.count(trace.anchor)) {
                if (capped) {
                    ss << "<...>" << std::endl;
                    capped = 0;
                }
                visited.insert(trace.anchor);
                print_traceback_entry(ss, trace);
            } else {
                capped++;
            }
        }
    #endif
    }

    void fold_callable_call(Label *l) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "folding callable call in " << l << std::endl;
#endif

        auto &&enter = l->body.enter;
        auto &&args = l->body.args;
        const Type *T = enter.indirect_type();

        Any value = none;
        bool result = T->lookup_call_handler(value);
        assert(result);
        args.insert(args.begin() + 1, Argument(enter));
        enter = value;
    }

    bool fold_builtin_call(Label *l) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "folding builtin call in " << l << std::endl;
#endif

        auto &&enter = l->body.enter;
        auto &&args = l->body.args;
        assert(enter.type == TYPE_Builtin);
        switch(enter.builtin.value()) {
        case KW_SyntaxExtend: {
            CHECKARGS(3, 3);
            const Closure *cl = args[1].value;
            const Syntax *sx = args[2].value;
            Scope *env = args[3].value;
            Specializer solver;
            Label *metafunc = solver.solve_inline(cl->frame, cl->label, { untyped(), env });
            auto rlt = metafunc->verify_return_label();
            //const Type *functype = metafunc->get_function_type();
            if (rlt->values.size() != 1)
                goto failed;
            {
                Scope *scope = nullptr;
                Any compiled = compile(metafunc, 0);
                if (rlt->values[0].value.type == TYPE_Scope) {
                    // returns a constant scope
                    typedef void (*FuncType)();
                    FuncType fptr = (FuncType)compiled.pointer;
                    fptr();
                    scope = rlt->values[0].value.scope;
                } else if ((rlt->values[0].value.type == TYPE_Unknown)
                    && (rlt->values[0].value.typeref == TYPE_Scope)) {
                    // returns a variable scope
                    typedef Scope *(*FuncType)();
                    FuncType fptr = (FuncType)compiled.pointer;
                    scope = fptr();
                } else {
                    goto failed;
                }
                enter = fold_type_label_single(cl->frame,
                    expand_module(sx, scope), { args[0] });
                args = { none };
                return false;
            }
        failed:
            set_active_anchor(sx->anchor);
            StyledString ss;
            const Type *T = rlt;
            ss.out << "syntax-extend has wrong return type (expected "
                << ReturnLabel({unknown_of(TYPE_Scope)}) << ", got "
                << T << ")";
            location_error(ss.str());
        } break;
        case FN_ScopeOf: {
            CHECKARGS(0, -1);
            Scope *scope = nullptr;
            size_t start = 1;
            if ((args.size() > 1) && (args[1].key == SYM_Unnamed)) {
                start = 2;
                scope = Scope::from(args[1].value);
            } else {
                scope = Scope::from();
            }
            for (size_t i = start; i < args.size(); ++i) {
                auto &&arg = args[i];
                if (arg.key == SYM_Unnamed) {
                    scope = Scope::from(scope, arg.value);
                } else {
                    scope->bind(arg.key, arg.value);
                }
            }
            RETARGS(scope);
        } break;
        case FN_AllocaOf: {
            CHECKARGS(1, 1);
            const Type *T = args[1].value.type;
            void *src = get_pointer(T, args[1].value);
            void *dst = tracked_malloc(size_of(T));
            memcpy(dst, src, size_of(T));
            RETARGS(Any::from_pointer(NativeROPointer(T), dst));
        } break;
        case FN_StaticAlloc: {
            CHECKARGS(1, 1);
            const Type *T = args[1].value;
            void *dst = tracked_malloc(size_of(T));
            RETARGS(Any::from_pointer(StaticPointer(T), dst));
        } break;
        case FN_NullOf: {
            CHECKARGS(1, 1);
            const Type *T = args[1].value;
            Any value = none;
            value.type = T;
            if (!is_opaque(T)) {
                void *ptr = get_pointer(T, value, true);
                memset(ptr, 0, size_of(T));
            }
            RETARGS(value);
        } break;
        case FN_ExternSymbol: {
            CHECKARGS(1, 1);
            verify_kind<TK_Extern>(args[1].value);
            RETARGS(args[1].value.symbol);
        } break;
        case FN_ExternNew: {
            CHECKARGS(2, -1);
            args[1].value.verify(TYPE_Symbol);
            const Type *T = args[2].value;
            Any value(args[1].value.symbol);
            Symbol extern_storage_class = SYM_Unnamed;
            size_t flags = 0;
            int location = -1;
            int binding = -1;
            if (args.size() > 3) {
                size_t i = 3;
                while (i < args.size()) {
                    auto &&arg = args[i];
                    switch(arg.key.value()) {
                    case SYM_Location: {
                        if (location == -1) {
                            location = cast_number<int>(arg.value);
                        } else {
                            location_error(String::from("duplicate location"));
                        }
                    } break;
                    case SYM_Binding: {
                        if (binding == -1) {
                            binding = cast_number<int>(arg.value);
                        } else {
                            location_error(String::from("duplicate binding"));
                        }
                    } break;
                    case SYM_Storage: {
                        arg.value.verify(TYPE_Symbol);

                        if (extern_storage_class == SYM_Unnamed) {
                            switch(arg.value.symbol.value()) {
                            #define T(NAME) \
                                case SYM_SPIRV_StorageClass ## NAME:
                                B_SPIRV_STORAGE_CLASS()
                            #undef T
                                extern_storage_class = arg.value.symbol; break;
                            default: {
                                location_error(String::from("illegal storage class"));
                            } break;
                            }
                        } else {
                            location_error(String::from("duplicate storage class"));
                        }
                    } break;
                    case SYM_Unnamed: {
                        arg.value.verify(TYPE_Symbol);

                        switch(arg.value.symbol.value()) {
                        case SYM_Buffer: flags |= EF_BufferBlock; break;
                        case SYM_ReadOnly: flags |= EF_NonWritable; break;
                        case SYM_WriteOnly: flags |= EF_NonReadable; break;
                        case SYM_Coherent: flags |= EF_Coherent; break;
                        case SYM_Restrict: flags |= EF_Restrict; break;
                        case SYM_Volatile: flags |= EF_Volatile; break;
                        default: {
                            location_error(String::from("unknown flag"));
                        } break;
                        }
                    } break;
                    default: {
                        StyledString ss;
                        ss.out << "unexpected key: " << arg.key;
                        location_error(ss.str());
                    } break;
                    }

                    i++;
                }
            }
            value.type = Extern(T, flags, extern_storage_class, location, binding);
            RETARGS(value);
        } break;
        case FN_FunctionType: {
            CHECKARGS(1, -1);
            ArgTypes types;
            size_t k = 2;
            while (k < args.size()) {
                if (args[k].value.type != TYPE_Type)
                    break;
                types.push_back(args[k].value);
                k++;
            }
            uint32_t flags = 0;

            while (k < args.size()) {
                args[k].value.verify(TYPE_Symbol);
                Symbol sym = args[k].value.symbol;
                uint64_t flag = 0;
                switch(sym.value()) {
                case SYM_Variadic: flag = FF_Variadic; break;
                case SYM_Pure: flag = FF_Pure; break;
                default: {
                    StyledString ss;
                    ss.out << "illegal option: " << sym;
                    location_error(ss.str());
                } break;
                }
                flags |= flag;
                k++;
            }
            RETARGS(Function(args[1].value, types, flags));
        } break;
        case FN_TupleType: {
            CHECKARGS(0, -1);
            Args values;
            for (size_t i = 1; i < args.size(); ++i) {
#if 0
                if (args[i].value.is_const()) {
                    values.push_back(args[i]);
                } else {
                    values.push_back(
                        Argument(args[i].key,
                            unknown_of(args[i].value.indirect_type())));
                }
#else
                values.push_back(
                    Argument(args[i].key,
                        unknown_of(args[i].value)));
#endif
            }
            RETARGS(MixedTuple(values));
        } break;
        case FN_UnionType: {
            CHECKARGS(0, -1);
            Args values;
            for (size_t i = 1; i < args.size(); ++i) {
#if 0
                if (args[i].value.is_const()) {
                    location_error(String::from("all union type arguments must be non-constant"));
                    //values.push_back(args[i]);
                } else {
                    values.push_back(
                        Argument(args[i].key,
                            unknown_of(args[i].value.indirect_type())));
                }
#else
                values.push_back(
                    Argument(args[i].key,
                        unknown_of(args[i].value)));
#endif
            }
            RETARGS(MixedUnion(values));
        } break;
        case FN_ReturnLabelType: {
            CHECKARGS(0, -1);
            Args values;
            // can theoretically also be initialized with constants; for that
            // we need a way to quote constants.
            for (size_t i = 1; i < args.size(); ++i) {
                const Type *T = args[i].value;
                values.push_back(Argument(args[i].key, unknown_of(T)));
            }
            RETARGS(ReturnLabel(values));
        } break;
        case FN_Location: {
            CHECKARGS(0, 0);
            RETARGS(l->body.anchor);
        } break;
        case SFXFN_SetTypenameStorage: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value;
            const Type *T2 = args[2].value;
            verify_kind<TK_Typename>(T);
            cast<TypenameType>(const_cast<Type *>(T))->finalize(T2);
            RETARGS();
        } break;
        case SFXFN_SetTypeSymbol: {
            CHECKARGS(3, 3);
            const Type *T = args[1].value;
            args[2].value.verify(TYPE_Symbol);
            const_cast<Type *>(T)->bind(args[2].value.symbol, args[3].value);
            RETARGS();
        } break;
        case SFXFN_DelTypeSymbol: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value;
            args[2].value.verify(TYPE_Symbol);
            const_cast<Type *>(T)->del(args[2].value.symbol);
            RETARGS();
        } break;
        case FN_TypeAt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value;
            args[2].value.verify(TYPE_Symbol);
            Any result = none;
            if (!T->lookup(args[2].value.symbol, result)) {
                RETARGS(none, false);
            } else {
                RETARGS(result, true);
            }
        } break;
        case FN_TypeLocalAt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value;
            args[2].value.verify(TYPE_Symbol);
            Any result = none;
            if (!T->lookup_local(args[2].value.symbol, result)) {
                RETARGS(none, false);
            } else {
                RETARGS(result, true);
            }
        } break;
        case FN_IsConstant: {
            CHECKARGS(1, 1);
            RETARGS(args[1].value.is_const());
        } break;
        case FN_VaCountOf: {
            RETARGS((int)(args.size()-1));
        } break;
        case FN_VaKeys: {
            CHECKARGS(0, -1);
            Args result = { none };
            for (size_t i = 1; i < args.size(); ++i) {
                result.push_back(args[i].key);
            }
            enter = args[0].value;
            args = result;
        } break;
        case FN_VaValues: {
            CHECKARGS(0, -1);
            Args result = { none };
            for (size_t i = 1; i < args.size(); ++i) {
                result.push_back(args[i].value);
            }
            enter = args[0].value;
            args = result;
        } break;
        case FN_VaKey: {
            CHECKARGS(2, 2);
            args[1].value.verify(TYPE_Symbol);
            enter = args[0].value;
            args = { none, Argument(args[1].value.symbol, args[2].value) };
        } break;
        case FN_VaAt: {
            CHECKARGS(1, -1);
            Args result = { none };
            if (args[1].value.type == TYPE_Symbol) {
                auto key = args[1].value.symbol;
                for (size_t i = 2; i < args.size(); ++i) {
                    if (args[i].key == key) {
                        result.push_back(args[i]);
                    }
                }
            } else {
                size_t idx = cast_number<size_t>(args[1].value);
                for (size_t i = (idx + 2); i < args.size(); ++i) {
                    result.push_back(args[i]);
                }
            }
            enter = args[0].value;
            args = result;
        } break;
        case FN_OffsetOf: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value;
            auto &&arg = args[2].value;
            size_t idx = 0;
            T = storage_type(T);
            verify_kind<TK_Tuple>(T);
            auto ti = cast<TupleType>(T);
            if (arg.type == TYPE_Symbol) {
                idx = ti->field_index(arg.symbol);
                if (idx == (size_t)-1) {
                    StyledString ss;
                    ss.out << "no such field " << arg.symbol << " in storage type " << T;
                    location_error(ss.str());
                }
                // rewrite field
                arg = (int)idx;
            } else {
                idx = cast_number<size_t>(arg);
            }
            verify_range(idx, ti->offsets.size());
            RETARGS(ti->offsets[idx]);
        } break;
        case FN_Branch: {
            CHECKARGS(3, 3);
            args[1].value.verify(TYPE_Bool);
            // either branch label is typed and binds no parameters,
            // so we can directly inline it
            const Closure *newl = nullptr;
            if (args[1].value.i1) {
                newl = args[2].value;
            } else {
                newl = args[3].value;
            }
            verify_branch_continuation(newl);
            Any cont = args[0].value;
            if (cont.type == TYPE_Closure) {
                cont.closure->frame->inline_merge = true;
            }
            enter = newl;
            args = { cont };
        } break;
        case FN_IntToPtr: {
            CHECKARGS(2, 2);
            verify_integer(storage_type(args[1].value.type));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_kind<TK_Pointer>(storage_type(DestT));
            Any result = args[1].value;
            result.type = DestT;
            RETARGS(result);
        } break;
        case FN_PtrToInt: {
            CHECKARGS(2, 2);
            verify_kind<TK_Pointer>(storage_type(args[1].value.type));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(storage_type(DestT));
            Any result = args[1].value;
            result.type = DestT;
            RETARGS(result);
        } break;
        case FN_ITrunc: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            verify_integer(storage_type(T));
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(storage_type(DestT));
            Any result = args[1].value;
            result.type = DestT;
            RETARGS(result);
        } break;
        case FN_FPTrunc: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            if ((T == TYPE_F64) && (DestT == TYPE_F32)) {
                RETARGS((float)args[1].value.f64);
            } else { invalid_op2_types_error(T, DestT); }
        } break;
        case FN_FPExt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            if ((T == TYPE_F32) && (DestT == TYPE_F64)) {
                RETARGS((double)args[1].value.f32);
            } else { invalid_op2_types_error(T, DestT); }
        } break;
        case FN_FPToUI: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(DestT);
            uint64_t val = 0;
            if (T == TYPE_F32) {
                val = (uint64_t)args[1].value.f32;
            } else if (T == TYPE_F64) {
                val = (uint64_t)args[1].value.f64;
            } else {
                invalid_op2_types_error(T, DestT);
            }
            Any result = val;
            result.type = DestT;
            RETARGS(result);
        } break;
        case FN_FPToSI: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            verify_real(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_integer(DestT);
            int64_t val = 0;
            if (T == TYPE_F32) {
                val = (int64_t)args[1].value.f32;
            } else if (T == TYPE_F64) {
                val = (int64_t)args[1].value.f64;
            } else {
                invalid_op2_types_error(T, DestT);
            }
            Any result = val;
            result.type = DestT;
            RETARGS(result);
        } break;
        case FN_UIToFP: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            verify_integer(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            uint64_t src = cast_number<uint64_t>(args[1].value);
            Any result = none;
            if (DestT == TYPE_F32) {
                result = (float)src;
            } else if (DestT == TYPE_F64) {
                result = (double)src;
            } else {
                invalid_op2_types_error(T, DestT);
            }
            RETARGS(result);
        } break;
        case FN_SIToFP: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            verify_integer(T);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            verify_real(DestT);
            int64_t src = cast_number<int64_t>(args[1].value);
            Any result = none;
            if (DestT == TYPE_F32) {
                result = (float)src;
            } else if (DestT == TYPE_F64) {
                result = (double)src;
            } else {
                invalid_op2_types_error(T, DestT);
            }
            RETARGS(result);
        } break;
        case FN_ZExt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            auto ST = storage_type(T);
            verify_integer(ST);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            auto DestST = storage_type(DestT);
            verify_integer(DestST);
            Any result = args[1].value;
            result.type = DestT;
            int oldbitnum = integer_type_bit_size(ST);
            int newbitnum = integer_type_bit_size(DestST);
            for (int i = oldbitnum; i < newbitnum; ++i) {
                result.u64 &= ~(1ull << i);
            }
            RETARGS(result);
        } break;
        case FN_SExt: {
            CHECKARGS(2, 2);
            const Type *T = args[1].value.type;
            auto ST = storage_type(T);
            verify_integer(ST);
            args[2].value.verify(TYPE_Type);
            const Type *DestT = args[2].value.typeref;
            auto DestST = storage_type(DestT);
            verify_integer(DestST);
            Any result = args[1].value;
            result.type = DestT;
            int oldbitnum = integer_type_bit_size(ST);
            int newbitnum = integer_type_bit_size(DestST);
            uint64_t bit = (result.u64 >> (oldbitnum - 1)) & 1ull;
            for (int i = oldbitnum; i < newbitnum; ++i) {
                result.u64 &= ~(1ull << i);
                result.u64 |= bit << i;
            }
            RETARGS(result);
        } break;
        case FN_TypeOf: {
            CHECKARGS(1, 1);
            RETARGS(args[1].value.indirect_type());
        } break;
        case FN_ExtractElement: {
            CHECKARGS(2, 2);
            const Type *T = storage_type(args[1].value.type);
            verify_kind<TK_Vector>(T);
            auto vi = cast<VectorType>(T);
            size_t idx = cast_number<size_t>(args[2].value);
            RETARGS(vi->unpack(args[1].value.pointer, idx));
        } break;
        case FN_InsertElement: {
            CHECKARGS(3, 3);
            const Type *T = storage_type(args[1].value.type);
            const Type *ET = storage_type(args[2].value.type);
            size_t idx = cast_number<size_t>(args[3].value);
            void *destptr = args[1].value.pointer;
            void *offsetptr = nullptr;
            destptr = copy_storage(T, destptr);
            auto vi = cast<VectorType>(T);
            verify(storage_type(vi->type_at_index(idx)), ET);
            offsetptr = vi->getelementptr(destptr, idx);
            void *srcptr = get_pointer(ET, args[2].value);
            memcpy(offsetptr, srcptr, size_of(ET));
            RETARGS(Any::from_pointer(args[1].value.type, destptr));
        } break;
        case FN_ShuffleVector: {
            CHECKARGS(3, 3);
            const Type *TV1 = storage_type(args[1].value.type);
            const Type *TV2 = storage_type(args[2].value.type);
            const Type *TMask = storage_type(args[3].value.type);
            verify_kind<TK_Vector>(TV1);
            verify_kind<TK_Vector>(TV2);
            verify_kind<TK_Vector>(TMask);
            verify(TV1, TV2);
            auto vi = cast<VectorType>(TV1);
            auto mask_vi = cast<VectorType>(TMask);
            verify(TYPE_I32, storage_type(mask_vi->element_type));
            size_t halfcount = vi->count;
            size_t incount = halfcount * 2;
            size_t outcount = mask_vi->count;
            const Type *T = Vector(vi->element_type, outcount);
            void *srcptr_a = get_pointer(TV1, args[1].value);
            void *srcptr_b = get_pointer(TV1, args[2].value);
            void *destptr = alloc_storage(T);
            auto out_vi = cast<VectorType>(T);
            size_t esize = size_of(vi->element_type);
            for (size_t i = 0; i < outcount; ++i) {
                size_t idx = (size_t)mask_vi->unpack(args[3].value.pointer, i).i32;
                verify_range(idx, incount);
                void *srcptr;
                if (idx < halfcount) {
                    srcptr = srcptr_a;
                } else {
                    srcptr = srcptr_b;
                    idx -= halfcount;
                }
                void *inp = vi->getelementptr(srcptr, idx);
                void *outp = out_vi->getelementptr(destptr, i);
                memcpy(outp, inp, esize);
            }
            RETARGS(Any::from_pointer(T, destptr));
        } break;
        case FN_ExtractValue: {
            CHECKARGS(2, 2);
            size_t idx = cast_number<size_t>(args[2].value);
            const Type *T = storage_type(args[1].value.type);
            switch(T->kind()) {
            case TK_Array: {
                auto ai = cast<ArrayType>(T);
                RETARGS(ai->unpack(args[1].value.pointer, idx));
            } break;
            case TK_Tuple: {
                auto ti = cast<TupleType>(T);
                RETARGS(ti->unpack(args[1].value.pointer, idx));
            } break;
            case TK_Union: {
                auto ui = cast<UnionType>(T);
                RETARGS(ui->unpack(args[1].value.pointer, idx));
            } break;
            default: {
                StyledString ss;
                ss.out << "can not extract value from type " << T;
                location_error(ss.str());
            } break;
            }
        } break;
        case FN_InsertValue: {
            CHECKARGS(3, 3);
            const Type *T = storage_type(args[1].value.type);
            const Type *ET = storage_type(args[2].value.type);
            size_t idx = cast_number<size_t>(args[3].value);

            void *destptr = args[1].value.pointer;
            void *offsetptr = nullptr;
            switch(T->kind()) {
            case TK_Array: {
                destptr = copy_storage(T, destptr);
                auto ai = cast<ArrayType>(T);
                verify(storage_type(ai->type_at_index(idx)), ET);
                offsetptr = ai->getelementptr(destptr, idx);
            } break;
            case TK_Tuple: {
                destptr = copy_storage(T, destptr);
                auto ti = cast<TupleType>(T);
                verify(storage_type(ti->type_at_index(idx)), ET);
                offsetptr = ti->getelementptr(destptr, idx);
            } break;
            case TK_Union: {
                destptr = copy_storage(T, destptr);
                auto ui = cast<UnionType>(T);
                verify(storage_type(ui->type_at_index(idx)), ET);
                offsetptr = destptr;
            } break;
            default: {
                StyledString ss;
                ss.out << "can not extract value from type " << T;
                location_error(ss.str());
            } break;
            }
            void *srcptr = get_pointer(ET, args[2].value);
            memcpy(offsetptr, srcptr, size_of(ET));
            RETARGS(Any::from_pointer(args[1].value.type, destptr));
        } break;
        case FN_AnyExtract: {
            CHECKARGS(1, 1);
            args[1].value.verify(TYPE_Any);
            Any arg = *args[1].value.ref;
            RETARGS(arg);
        } break;
        case FN_AnyWrap: {
            CHECKARGS(1, 1);
            RETARGS(args[1].value.toref());
        } break;
        case FN_Purify: {
            CHECKARGS(1, 1);
            Any arg = args[1].value;
            verify_function_pointer(arg.type);
            auto pi = cast<PointerType>(arg.type);
            auto fi = cast<FunctionType>(pi->element_type);
            if (fi->flags & FF_Pure) {
                RETARGS(args[1]);
            } else {
                arg.type = Pointer(Function(
                    fi->return_type, fi->argument_types, fi->flags | FF_Pure),
                    pi->flags, pi->storage_class);
                RETARGS(arg);
            }
        } break;
        case SFXFN_CompilerError: {
            CHECKARGS(1, 1);
            location_error(args[1].value);
            RETARGS();
        } break;
        case FN_CompilerMessage: {
            CHECKARGS(1, 1);
            args[1].value.verify(TYPE_String);
            StyledString ss;
            ss.out << l->body.anchor << " message: " << args[1].value.string->data << std::endl;
            std::cout << ss.str()->data;
            RETARGS();
        } break;
        case FN_Dump: {
            CHECKARGS(0, -1);
            StyledStream ss(std::cerr);
            ss << l->body.anchor << " dump:";
            for (size_t i = 1; i < args.size(); ++i) {
                ss << " ";
                if (args[i].key != SYM_Unnamed) {
                    ss << args[i].key << " " << Style_Operator << "=" << Style_None << " ";
                }

                if (args[i].value.is_const()) {
                    stream_expr(ss, args[i].value, StreamExprFormat::singleline());
                } else {
                    /*
                    ss << "<unknown>"
                        << Style_Operator << ":" << Style_None
                        << args[i].value.indirect_type() << std::endl;*/
                    args[i].value.stream(ss, false);
                }
            }
            ss << std::endl;
            enter = args[0].value;
            args[0].value = none;
        } break;
        case FN_Cross: {
            CHECKARGS(2, 2);
            const Type *Ta = storage_type(args[1].value.type);
            const Type *Tb = storage_type(args[2].value.type);
            verify_real_vector(Ta, 3);
            verify(Ta, Tb);
            RETARGS(apply_real_op<op_Cross>(args[1].value, args[2].value));
        } break;
        case FN_Normalize: {
            CHECKARGS(1, 1);
            verify_real_ops(args[1].value);
            RETARGS(apply_real_op<op_Normalize>(args[1].value));
        } break;
        case FN_Length: {
            CHECKARGS(1, 1);
            verify_real_ops(args[1].value);
            RETARGS(apply_real_op<op_Length>(args[1].value));
        } break;
        case FN_Distance: {
            CHECKARGS(2, 2);
            verify_real_ops(args[1].value, args[2].value);
            RETARGS(apply_real_op<op_Distance>(args[1].value, args[2].value));
        } break;
        case OP_ICmpEQ:
        case OP_ICmpNE:
        case OP_ICmpUGT:
        case OP_ICmpUGE:
        case OP_ICmpULT:
        case OP_ICmpULE:
        case OP_ICmpSGT:
        case OP_ICmpSGE:
        case OP_ICmpSLT:
        case OP_ICmpSLE: {
            CHECKARGS(2, 2);
            verify_integer_ops(args[1].value, args[2].value);
#define B_INT_OP1(OP, N) \
    result = apply_integer_op<IntTypes_ ## N, op_ ## OP>(args[1].value);
#define B_INT_OP2(OP, N) \
    result = apply_integer_op<IntTypes_ ## N, op_ ## OP>(args[1].value, args[2].value);
            Any result = false;
            switch(enter.builtin.value()) {
            case OP_ICmpEQ: B_INT_OP2(Equal, u); break;
            case OP_ICmpNE: B_INT_OP2(NotEqual, u); break;
            case OP_ICmpUGT: B_INT_OP2(Greater, u); break;
            case OP_ICmpUGE: B_INT_OP2(GreaterEqual, u); break;
            case OP_ICmpULT: B_INT_OP2(Less, u); break;
            case OP_ICmpULE: B_INT_OP2(LessEqual, u); break;
            case OP_ICmpSGT: B_INT_OP2(Greater, i); break;
            case OP_ICmpSGE: B_INT_OP2(GreaterEqual, i); break;
            case OP_ICmpSLT: B_INT_OP2(Less, i); break;
            case OP_ICmpSLE: B_INT_OP2(LessEqual, i); break;
            default: assert(false); break;
            }
            RETARGS(result);
        } break;
        case OP_FCmpOEQ:
        case OP_FCmpONE:
        case OP_FCmpORD:
        case OP_FCmpOGT:
        case OP_FCmpOGE:
        case OP_FCmpOLT:
        case OP_FCmpOLE:
        case OP_FCmpUEQ:
        case OP_FCmpUNE:
        case OP_FCmpUNO:
        case OP_FCmpUGT:
        case OP_FCmpUGE:
        case OP_FCmpULT:
        case OP_FCmpULE: {
            CHECKARGS(2, 2);
            verify_real_ops(args[1].value, args[2].value);
#define B_FLOAT_OP1(OP) \
    result = apply_real_op<op_ ## OP>(args[1].value);
#define B_FLOAT_OP2(OP) \
    result = apply_real_op<op_ ## OP>(args[1].value, args[2].value);
#define B_FLOAT_OP3(OP) \
    result = apply_real_op<op_ ## OP>(args[1].value, args[2].value, args[3].value);
            Any result = false;
            switch(enter.builtin.value()) {
            case OP_FCmpOEQ: B_FLOAT_OP2(OEqual); break;
            case OP_FCmpONE: B_FLOAT_OP2(ONotEqual); break;
            case OP_FCmpORD: B_FLOAT_OP2(Ordered); break;
            case OP_FCmpOGT: B_FLOAT_OP2(OGreater); break;
            case OP_FCmpOGE: B_FLOAT_OP2(OGreaterEqual); break;
            case OP_FCmpOLT: B_FLOAT_OP2(OLess); break;
            case OP_FCmpOLE: B_FLOAT_OP2(OLessEqual); break;
            case OP_FCmpUEQ: B_FLOAT_OP2(UEqual); break;
            case OP_FCmpUNE: B_FLOAT_OP2(UNotEqual); break;
            case OP_FCmpUNO: B_FLOAT_OP2(Unordered); break;
            case OP_FCmpUGT: B_FLOAT_OP2(UGreater); break;
            case OP_FCmpUGE: B_FLOAT_OP2(UGreaterEqual); break;
            case OP_FCmpULT: B_FLOAT_OP2(ULess); break;
            case OP_FCmpULE: B_FLOAT_OP2(ULessEqual); break;
            default: assert(false); break;
            }
            RETARGS(result);
        } break;
#define IARITH_NUW_NSW_OPS(NAME) \
    case OP_ ## NAME: \
    case OP_ ## NAME ## NUW: \
    case OP_ ## NAME ## NSW: { \
        CHECKARGS(2, 2); \
        verify_integer_ops(args[1].value, args[2].value); \
        Any result = none; \
        switch(enter.builtin.value()) { \
        case OP_ ## NAME: B_INT_OP2(NAME, u); break; \
        case OP_ ## NAME ## NUW: B_INT_OP2(NAME, u); break; \
        case OP_ ## NAME ## NSW: B_INT_OP2(NAME, i); break; \
        default: assert(false); break; \
        } \
        result.type = args[1].value.type; \
        RETARGS(result); \
    } break;
#define IARITH_OP(NAME, PFX) \
    case OP_ ## NAME: { \
        CHECKARGS(2, 2); \
        verify_integer_ops(args[1].value, args[2].value); \
        Any result = none; \
        B_INT_OP2(NAME, PFX); \
        result.type = args[1].value.type; \
        RETARGS(result); \
    } break;
#define FARITH_OP(NAME) \
    case OP_ ## NAME: { \
        CHECKARGS(2, 2); \
        verify_real_ops(args[1].value, args[2].value); \
        Any result = none; \
        B_FLOAT_OP2(NAME); \
        RETARGS(result); \
    } break;
#define FTRI_OP(NAME) \
    case OP_ ## NAME: { \
        CHECKARGS(3, 3); \
        verify_real_ops(args[1].value, args[2].value, args[3].value); \
        Any result = none; \
        B_FLOAT_OP3(NAME); \
        RETARGS(result); \
    } break;
#define IUN_OP(NAME, PFX) \
    case OP_ ## NAME: { \
        CHECKARGS(1, 1); \
        verify_integer_ops(args[1].value); \
        Any result = none; \
        B_INT_OP1(NAME, PFX); \
        result.type = args[1].value.type; \
        RETARGS(result); \
    } break;
#define FUN_OP(NAME) \
    case OP_ ## NAME: { \
        CHECKARGS(1, 1); \
        verify_real_ops(args[1].value); \
        Any result = none; \
        B_FLOAT_OP1(NAME); \
        RETARGS(result); \
    } break;

        B_ARITH_OPS()

        default: {
            StyledString ss;
            ss.out << "can not fold constant expression using builtin " << enter.builtin;
            location_error(ss.str());
        } break;
        }
        return true;
    }

    bool requires_mergelabel(const Any &cont) {
        if (cont.type == TYPE_Closure) {
            const Closure *cl = cont.closure;
            if (cl->label->is_merge()) {
                return false;
            }
        }
        return true;
    }

    bool mergelabel_has_params(const Any &cont) {
        if (cont.type == TYPE_Closure) {
            const Closure *cl = cont.closure;
            if (!cl->label->has_params()) {
                return false;
            }
        }
        return true;
    }

    void type_branch_continuations(Label *l) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "inlining branch continuations in " << l << std::endl;
#endif
        assert(!l->body.is_complete());
        auto &&args = l->body.args;
        CHECKARGS(3, 3);

        args[1].value.verify_indirect(TYPE_Bool);
        const Closure *then_br = args[2].value;
        const Closure *else_br = args[3].value;
        verify_branch_continuation(then_br);
        verify_branch_continuation(else_br);

        Any cont = args[0].value;
        args[2].value = fold_type_label_single(then_br->frame, then_br->label, { cont });
        args[3].value = fold_type_label_single(else_br->frame, else_br->label, { cont });
        args[0].value = none;
    }

#undef IARITH_NUW_NSW_OPS
#undef IARITH_OP
#undef FARITH_OP
#undef FARITH_OPF
#undef B_INT_OP2
#undef B_INT_OP1
#undef B_FLOAT_OP3
#undef B_FLOAT_OP2
#undef B_FLOAT_OP1
#undef CHECKARGS
#undef RETARGS
#undef IUN_OP
#undef FUN_OP
#undef FTRI_OP

    static Label *skip_jumps(Label *l) {
        size_t counter = 0;
        while (jumps_immediately(l)) {
            l = l->body.enter.label;
            counter++;
            if (counter == SCOPES_MAX_SKIP_JUMPS) {
                std::cerr
                    << "internal warning: max iterations exceeded"
                        " during jump skip check" << std::endl;
                break;
            }
        }
        return l;
    }

    static bool is_exiting(Label *l) {
        return is_calling_continuation(l) || is_continuing_to_parameter(l);
    }

    // label targets count as calls, all other as ops
    static bool is_empty_function(Label *l) {
        assert(!l->params.empty());
        auto rtype = l->params[0]->type;
        if (rtype->kind() != TK_ReturnLabel)
            return false;
        const ReturnLabelType *rlt = cast<ReturnLabelType>(rtype);
        if (rlt->has_variables() || !rlt->is_returning())
            return false;
        assert(!l->params.empty());
        std::unordered_set<Label *> visited;
        while (!visited.count(l)) {
            visited.insert(l);
            if (jumps_immediately(l)) {
                l = l->body.enter.label;
                continue;
            }
            if (!is_calling_continuation(l)) {
                return false;
            }
            assert(!is_continuing_to_label(l));
            if (l->body.args[0].value.type == TYPE_Nothing) {
                // branch, unreachable, etc.
                break;
            } else {
                StyledStream ss(std::cerr);
                ss << "internal warning: unexpected continuation type "
                    << l->body.args[0].value.type
                    << " encountered while counting instructions" << std::endl;
                break;
            }
        }
        return true;
    }

    static bool matches_arg_count(Label *l, size_t inargs) {
        // works only on instantiated labels, as we're
        // assuming no parameter is variadic at this point
        auto &&params = l->params;
        return ((inargs + 1) == std::max(size_t(1), params.size()));
    }

    static bool forwards_all_args(Label *l) {
        assert(!l->params.empty());
        auto &&args = l->body.args;
        auto &&params = l->params;
        if (args.size() != params.size())
            return false;
        for (size_t i = 1; i < args.size(); ++i) {
            auto &&arg = args[i];
            if (arg.value.type != TYPE_Parameter)
                return false;
            if (arg.value.parameter != params[i])
                return false;
        }
        return true;
    }

    // a label just jumps to the next label
    static bool jumps_immediately(Label *l) {
        return is_calling_label(l)
            && l->get_label_enter()->is_basic_block_like();
    }

    void clear_continuation_arg(Label *l) {
        auto &&args = l->body.args;
        args[0] = none;
    }

    // clear continuation argument and clear it for labels that use it
    void delete_continuation(Label *owner) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "deleting continuation of " << owner << std::endl;
#endif

        assert(!owner->params.empty());
        Parameter *param = owner->params[0];
        param->type = TYPE_Nothing;

        assert(!is_called_by(param, owner));
        if (is_continuing_from(param, owner)) {
            clear_continuation_arg(owner);
        }
    }

    const Type *get_return_type_from_function_call(Label *l) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "typing continuation from function call in " << l << std::endl;
#endif
        auto &&enter = l->body.enter;

        const FunctionType *fi = extract_function_type(enter.indirect_type());
        verify_function_argument_signature(fi, l);
        return fi->return_type;
    }

    const Type *get_return_type_from_call_arguments(Label *l) {
#if SCOPES_DEBUG_CODEGEN
        ss_cout << "typing continuation call in " << l << std::endl;
#endif
        auto &&args = l->body.args;
        Args values;
        values.reserve(args.size());
        for (size_t i = 1; i < args.size(); ++i) {
            if (args[i].value.is_const()) {
                values.push_back(args[i]);
            } else {
                values.push_back(Argument(
                    args[i].key,
                    unknown_of(args[i].value.indirect_type())));
            }
        }
        clear_continuation_arg(l);
        return ReturnLabel(values);
    }

    static void inc_solve_ref() {
        solve_refs++;
    }

    static bool dec_solve_ref() {
        solve_refs--;
        assert(solve_refs >= 0);
        return solve_refs == 0;
    }

    Label *solve_inline(Frame *frame, Label *label, const Args &values) {
        Frame *entryf = fold_type_label_single_frame(frame, label, values);

        inc_solve_ref();
        SCOPES_TRY()

        normalize_function(entryf);

        SCOPES_CATCH(exc)
            if (dec_solve_ref()) {
                print_traceback();
                traceback.clear();
            }
            error(exc);
        SCOPES_TRY_END()

        if (dec_solve_ref()) {
            traceback.clear();
        }

        Label *entry = entryf->get_instance();
        validate_scope(entry);
        return entry;
    }

    Label *typify(Frame *frame, Label *label, const ArgTypes &argtypes) {
        Args args;
        args.reserve(argtypes.size() + 1);
        args = { Argument(untyped()) };
        for (size_t i = 0; i < argtypes.size(); ++i) {
            args.push_back(Argument(unknown_of(argtypes[i])));
        }
        return solve_inline(frame, label, args);
    }

    Label *solve(Frame *frame, Label *label, const Args &values) {
        assert(frame);
        assert(!label->params.empty());
        Args args;
        args.reserve(values.size() + 1);
        args = { Argument(untyped()) };
        for (size_t i = 0; i < values.size(); ++i) {
            args.push_back(values[i]);
        }
        return solve_inline(frame, label, values);
    }

    const Type *complete_existing_label_continuation (Label *l) {
        Label *enter_label = l->get_label_enter();
        if (!enter_label->is_basic_block_like()) {
            assert(enter_label->body.is_complete());
            assert(enter_label->is_return_param_typed());
            return enter_label->get_return_type();
        }
        return nullptr;
    }

    void on_label_processing(Label *l, const char *task = nullptr) {
        if (!enable_step_debugger) {
            if (clicmd == CmdSkip) {
                enable_step_debugger = true;
                clicmd = CmdNone;
            }
            return;
        }
        clicmd = CmdNone;
        auto slfmt = StreamLabelFormat::debug_single();
        slfmt.anchors = StreamLabelFormat::Line;
        if (task) {
            ss_cout << task << std::endl;
        }
        stream_label(ss_cout, l, slfmt);
        bool skip = false;
        while (!skip) {
            set_active_anchor(l->body.anchor);
            char *r = linenoise("solver> ");
            if (!r) {
                location_error(String::from("aborted"));
            }

            linenoiseHistoryAdd(r);
            SCOPES_TRY()
                auto file = SourceFile::from_string(Symbol("<string>"),
                    String::from_cstr(r));
                LexerParser parser(file);
                auto expr = parser.parse();
                //stream_expr(ss_cout, expr, StreamExprFormat());
                const List *stmts = unsyntax(expr);
                if (stmts != EOL) {
                    while (stmts != EOL) {
                        set_active_anchor(stmts->at.syntax->anchor);
                        auto cmd = unsyntax(stmts->at);
                        Symbol head = SYM_Unnamed;
                        const List *arglist = nullptr;
                        if (cmd.type == TYPE_Symbol) {
                            head = cmd.symbol;
                        } else if (cmd.type == TYPE_List) {
                            arglist = cmd.list;
                            if (arglist != EOL) {
                                cmd = unsyntax(arglist->at);
                                if (cmd.type == TYPE_Symbol) {
                                    head = cmd.symbol;
                                }
                                arglist = arglist->next;
                            }
                        }
                        if (head == SYM_Unnamed) {
                            location_error(String::from("syntax error"));
                        }
                        switch(head.value()) {
                        case SYM_C:
                        case KW_Continue: {
                            skip = true;
                            enable_step_debugger = false;
                        } break;
                        case SYM_Skip: {
                            skip = true;
                            clicmd = CmdSkip;
                            enable_step_debugger = false;
                        } break;
                        case SYM_Original: {
                            Label *o = l->original;
                            while (o) {
                                stream_label(ss_cout, o, slfmt);
                                o = o->original;
                            }
                        } break;
                        case SYM_Help: {
                            ss_cout << "Available commands:" << std::endl;
                            ss_cout << "c(ontinue) help original skip" << std::endl;
                            ss_cout << "An empty line continues to the next label." << std::endl;
                        } break;
                        default: {
                            location_error(String::from("unknown command. try 'help'."));
                        }break;
                        }
                        stmts = stmts->next;
                    }
                } else {
                    skip = true;
                }
            SCOPES_CATCH(exc)
                print_exception(exc);
            SCOPES_TRY_END()
        }
    }

    bool fold_useless_labels(Label *l) {
        Label *startl = l;
    repeat:
        if (l->body.is_complete()) {
            auto &&enter = l->body.enter;
            if (enter.type == TYPE_Label) {
                Label *nextl = enter.label;
                if (nextl->is_basic_block_like()
                    && !nextl->is_important()
                    && !nextl->has_params()) {
                    l = nextl;
                    goto repeat;
                }
            }
        }
        if (l != startl) {
            const Anchor *saveanchor = startl->body.anchor;
            startl->body = l->body;
            startl->body.anchor = saveanchor;
            return true;
        }
        return false;
    }

    void verify_stack_size() {
        size_t ssz = memory_stack_size();
        if (ssz >= SCOPES_MAX_STACK_SIZE) {
            location_error(String::from("stack overflow during partial evaluation"));
        }
    }

    // fold body of single label as far as possible
    bool fold_label_body(Label *l, LabelQueue &todo, LabelQueue &recursions, Traces &traces) {
        if (l->body.is_complete())
            return true;

        while (true) {
            on_label_processing(l);
            assert(!l->is_template());
            l->verify_valid();
            assert(all_params_typed(l));

            //traces.push_back(Trace(l->body.anchor, l->name));
            set_active_anchor(l->body.anchor);

            if (!all_args_typed(l)) {
                size_t idx = find_untyped_arg(l);
                StyledString ss;
                ss.out << "parameter " << l->body.args[idx].value.parameter
                    << " passed as argument " << idx << " has not been typed yet";
                location_error(ss.str());
            }

            const Type *rtype = nullptr;
            if (is_calling_label(l)) {
                if (!l->get_label_enter()->body.is_complete()) {
                    location_error(String::from("failed to propagate return type from untyped label"));
                }
                rtype = complete_existing_label_continuation(l);
            } else if (is_calling_callable(l)) {
                fold_callable_call(l);
                continue;
            } else if (is_calling_function(l)) {
                verify_no_keyed_args(l);
                if (is_calling_pure_function(l)
                    && all_args_constant(l)) {
                    fold_pure_function_call(l);
                    continue;
                } else {
                    rtype = get_return_type_from_function_call(l);
                }
            } else if (is_calling_builtin(l)) {
                auto builtin = l->get_builtin_enter();
                if (!builtin_has_keyed_args(builtin))
                    verify_no_keyed_args(l);
                if (builtin_always_folds(builtin)
                    || (!builtin_never_folds(builtin) && all_args_constant(l))) {
                    if (fold_builtin_call(l)) {
                        continue;
                    }
                } else if (builtin == FN_Branch) {
                    type_branch_continuations(l);
                    auto &&args = l->body.args;
                    todo.push_back(args[3].value);
                    todo.push_back(args[2].value);
                } else {
                    auto &&enter = l->body.enter;
                    auto &&args = l->body.args;
                    assert(enter.type == TYPE_Builtin);
                    if ((enter.builtin == SFXFN_Unreachable)
                        || (enter.builtin == SFXFN_Discard)) {
                        args[0] = none;
                    } else {
                        Args values;
                        bool fold = values_from_builtin_call(l, values);
                        if (fold) {
                            enter = args[0].value;
                            args = { none };
                            for (size_t i = 0; i < values.size(); ++i) {
                                args.push_back(values[i]);
                            }
                            continue;
                        } else {
                            rtype = ReturnLabel(values);
                        }
                    }
                }
            } else if (is_calling_closure(l)) {
                if (has_keyed_args(l)) {
                    solve_keyed_args(l);
                }
                bool recursive = false;
                rtype = fold_closure_call(l, recursive);
                if (recursive) {
                    // try again later
                    recursions.push_front(l);
                    return false;
                }
            } else if (is_calling_continuation(l)) {
                rtype = get_return_type_from_call_arguments(l);
            } else {
                StyledString ss;
                auto &&enter = l->body.enter;
                if (!enter.is_const()) {
                    ss.out << "unable to call variable of type " << enter.indirect_type();
                } else {
                    ss.out << "unable to call constant of type " << enter.type;
                }
                location_error(ss.str());
            }

            if (rtype) {
                if (is_jumping(l)) {
                    l->body.enter = fold_type_return(l->body.enter, rtype);
                } else {
                    assert(!l->body.args.empty());
                    l->body.args[0] = fold_type_return(l->body.args[0].value, rtype);
                }
            }

            l->body.set_complete();

#if 1
            if (fold_useless_labels(l)) {
                continue;
            }
#endif

            break;
        }

        return true;
    }

    // normalize all labels in the scope of a function with entry point l
    void normalize_function(Frame *f) {
        Label *l = f->get_instance();
        if (l->body.is_complete())
            return;
        verify_stack_size();
        assert(!l->is_basic_block_like());
        Label *entry_label = l;
        // second stack of recursive calls that can't be typed yet
        LabelQueue recursions;
        LabelQueue todo;
        todo.push_back(l);
        Traces traces;

        SCOPES_TRY()

        while (true) {
            if (todo.empty()) {
                if (recursions.empty())
                    break;
                entry_label->set_reentrant();
                // all directly reachable branches have been processed
                // check that entry_label is typed at this point
                // if it is not, we are recursing infinitely
                if (!entry_label->is_return_param_typed()) {
                    set_active_anchor(entry_label->anchor);
                    location_error(String::from("recursive function never returns"));
                }
                todo = recursions;
                recursions.clear();
            }
            l = todo.back();
            todo.pop_back();
            if(l->body.is_complete())
                continue;

            if (!fold_label_body(l, todo, recursions, traces))
                continue;

            /*
                some thoughts:

                it's better to skip labels when continuing rather than copying
                over bodies, because we overwrite the anchor information
                (not sure if that's relevant tho)

            */

            if (jumps_immediately(l)) {
                Label *enter_label = l->get_label_enter();
                clear_continuation_arg(l);
                todo.push_back(enter_label);
            } else if (is_continuing_to_label(l)) {
                Label *nextl = l->body.args[0].value.label;
                if (!all_params_typed(nextl)) {
                    StyledStream ss(std::cerr);
                    stream_label(ss, l, StreamLabelFormat::debug_single());
                    location_error(String::from("failed to type continuation"));
                }
                todo.push_back(nextl);
            }
        }

        SCOPES_CATCH(exc)
        #if 1
            traceback.push_back(Trace(l->body.anchor, l->name));
            if (entry_label != l) {
                traceback.push_back(Trace(entry_label->anchor, entry_label->name));
            }
        #else
            size_t i = traces.size();
            while (i > 0) {
                i--;
                traceback.push_back(traces[i]);
            }
        #endif
            error(exc);
        SCOPES_TRY_END()

        assert(!entry_label->params.empty());
        assert(entry_label->body.is_complete());

        // function is done, but not typed
        if (!entry_label->is_return_param_typed()) {
            // two possible reasons for this:
            // 1. recursion - entry label has already been
            //    processed, but not exited yet, so we don't have the
            //    continuation type yet.
            //    -- but in that case, we're never arriving here!
            //if (f->is_reentrant())
            // 2. all return paths are unreachable, because the function
            //    never returns.
            fold_type_return(entry_label->params[0], NoReturnLabel());
        } else {
            validate_label_return_types(entry_label);
        }
    }

    Label *validate_scope(Label *entry) {
        Timer validate_scope_timer(TIMER_ValidateScope);

        std::unordered_set<Label *> visited;
        Labels labels;
        entry->build_reachable(visited, &labels);

        Label::UserMap um;
        for (auto it = labels.begin(); it != labels.end(); ++it) {
            (*it)->insert_into_usermap(um);
        }

        for (auto it = labels.begin(); it != labels.end(); ++it) {
            Label *l = *it;
            if (l->is_basic_block_like()) {
                continue;
            }
            Labels scope;
            l->build_scope(um, scope);
            for (size_t i = 0; i < scope.size(); ++i) {
                Label *subl = scope[i];
                if (!subl->is_basic_block_like()) {
                    assert(!subl->is_inline());
                    location_message(l->anchor, String::from("depends on this scope"));
                    set_active_anchor(subl->anchor);
                    location_error(String::from("only inlines may depend on variables in exterior scopes"));
                }
            }
        }

        return entry;
    }

    std::unordered_map<const Type *, ffi_type *> ffi_types;

    ffi_type *new_type() {
        ffi_type *result = (ffi_type *)malloc(sizeof(ffi_type));
        memset(result, 0, sizeof(ffi_type));
        return result;
    }

    ffi_type *create_ffi_type(const Type *type) {
        if (type == TYPE_Void) return &ffi_type_void;
        if (type == TYPE_Nothing) return &ffi_type_void;

        switch(type->kind()) {
        case TK_Integer: {
            auto it = cast<IntegerType>(type);
            if (it->issigned) {
                switch (it->width) {
                case 8: return &ffi_type_sint8;
                case 16: return &ffi_type_sint16;
                case 32: return &ffi_type_sint32;
                case 64: return &ffi_type_sint64;
                default: break;
                }
            } else {
                switch (it->width) {
                case 1: return &ffi_type_uint8;
                case 8: return &ffi_type_uint8;
                case 16: return &ffi_type_uint16;
                case 32: return &ffi_type_uint32;
                case 64: return &ffi_type_uint64;
                default: break;
                }
            }
        } break;
        case TK_Real: {
            switch(cast<RealType>(type)->width) {
            case 32: return &ffi_type_float;
            case 64: return &ffi_type_double;
            default: break;
            }
        } break;
        case TK_Pointer: return &ffi_type_pointer;
        case TK_Typename: {
            return get_ffi_type(storage_type(type));
        } break;
        case TK_Array: {
            auto ai = cast<ArrayType>(type);
            size_t count = ai->count;
            ffi_type *ty = (ffi_type *)malloc(sizeof(ffi_type));
            ty->size = 0;
            ty->alignment = 0;
            ty->type = FFI_TYPE_STRUCT;
            ty->elements = (ffi_type **)malloc(sizeof(ffi_type*) * (count + 1));
            ffi_type *element_type = get_ffi_type(ai->element_type);
            for (size_t i = 0; i < count; ++i) {
                ty->elements[i] = element_type;
            }
            ty->elements[count] = nullptr;
            return ty;
        } break;
        case TK_Tuple: {
            auto ti = cast<TupleType>(type);
            size_t count = ti->types.size();
            ffi_type *ty = (ffi_type *)malloc(sizeof(ffi_type));
            ty->size = 0;
            ty->alignment = 0;
            ty->type = FFI_TYPE_STRUCT;
            ty->elements = (ffi_type **)malloc(sizeof(ffi_type*) * (count + 1));
            for (size_t i = 0; i < count; ++i) {
                ty->elements[i] = get_ffi_type(ti->types[i]);
            }
            ty->elements[count] = nullptr;
            return ty;
        } break;
        case TK_Union: {
            auto ui = cast<UnionType>(type);
            size_t count = ui->types.size();
            size_t sz = ui->size;
            size_t al = ui->align;
            ffi_type *ty = (ffi_type *)malloc(sizeof(ffi_type));
            ty->size = 0;
            ty->alignment = 0;
            ty->type = FFI_TYPE_STRUCT;
            // find member with the same alignment
            for (size_t i = 0; i < count; ++i) {
                const Type *ET = ui->types[i];
                size_t etal = align_of(ET);
                if (etal == al) {
                    size_t remsz = sz - size_of(ET);
                    ffi_type *tvalue = get_ffi_type(ET);
                    if (remsz) {
                        ty->elements = (ffi_type **)malloc(sizeof(ffi_type*) * 3);
                        ty->elements[0] = tvalue;
                        ty->elements[1] = get_ffi_type(Array(TYPE_I8, remsz));
                        ty->elements[2] = nullptr;
                    } else {
                        ty->elements = (ffi_type **)malloc(sizeof(ffi_type*) * 2);
                        ty->elements[0] = tvalue;
                        ty->elements[1] = nullptr;
                    }
                    return ty;
                }
            }
            // should never get here
            assert(false);
        } break;
        default: break;
        };

        StyledString ss;
        ss.out << "FFI: cannot convert argument of type " << type;
        location_error(ss.str());
        return nullptr;
    }

    ffi_type *get_ffi_type(const Type *type) {
        auto it = ffi_types.find(type);
        if (it == ffi_types.end()) {
            auto result = create_ffi_type(type);
            ffi_types[type] = result;
            return result;
        } else {
            return it->second;
        }
    }

    void verify_function_argument_count(const FunctionType *fi, size_t argcount) {

        size_t fargcount = fi->argument_types.size();
        if (fi->flags & FF_Variadic) {
            if (argcount < fargcount) {
                StyledString ss;
                ss.out << "argument count mismatch (need at least "
                    << fargcount << ", got " << argcount << ")";
                location_error(ss.str());
            }
        } else {
            if (argcount != fargcount) {
                StyledString ss;
                ss.out << "argument count mismatch (need "
                    << fargcount << ", got " << argcount << ")";
                location_error(ss.str());
            }
        }
    }

    Any run_ffi_function(Any enter, Argument *args, size_t argcount) {
        auto pi = cast<PointerType>(enter.type);
        auto fi = cast<FunctionType>(pi->element_type);

        size_t fargcount = fi->argument_types.size();

        const Type *rettype = cast<ReturnLabelType>(fi->return_type)->return_type;

        ffi_cif cif;
        ffi_type *argtypes[argcount];
        void *avalues[argcount];
        for (size_t i = 0; i < argcount; ++i) {
            Argument &arg = args[i];
            argtypes[i] = get_ffi_type(arg.value.type);
            avalues[i] = get_pointer(arg.value.type, arg.value);
        }
        ffi_status prep_result;
        if (fi->flags & FF_Variadic) {
            prep_result = ffi_prep_cif_var(
                &cif, FFI_DEFAULT_ABI, fargcount, argcount, get_ffi_type(rettype), argtypes);
        } else {
            prep_result = ffi_prep_cif(
                &cif, FFI_DEFAULT_ABI, argcount, get_ffi_type(rettype), argtypes);
        }
        assert(prep_result == FFI_OK);

        Any result = Any::from_pointer(rettype, nullptr);
        ffi_call(&cif, FFI_FN(enter.pointer),
            get_pointer(result.type, result, true), avalues);
        return result;
    }

};

Specializer::Traces Specializer::traceback;
int Specializer::solve_refs = 0;
bool Specializer::enable_step_debugger = false;
Specializer::CLICmd Specializer::clicmd = CmdNone;

//------------------------------------------------------------------------------
// MACRO EXPANDER
//------------------------------------------------------------------------------
// expands macros and generates the IL

static bool verify_list_parameter_count(const List *expr, int mincount, int maxcount) {
    assert(expr != EOL);
    if ((mincount <= 0) && (maxcount == -1)) {
        return true;
    }
    int argcount = (int)expr->count - 1;

    if ((maxcount >= 0) && (argcount > maxcount)) {
        location_error(
            format("excess argument. At most %i arguments expected", maxcount));
        return false;
    }
    if ((mincount >= 0) && (argcount < mincount)) {
        location_error(
            format("at least %i arguments expected, got %i", mincount, argcount));
        return false;
    }
    return true;
}

static void verify_at_parameter_count(const List *topit, int mincount, int maxcount) {
    assert(topit != EOL);
    verify_list_parameter_count(unsyntax(topit->at), mincount, maxcount);
}

//------------------------------------------------------------------------------

static bool ends_with_parenthesis(Symbol sym) {
    if (sym == SYM_Parenthesis)
        return true;
    const String *str = sym.name();
    if (str->count < 3)
        return false;
    const char *dot = str->data + str->count - 3;
    return !strcmp(dot, "...");
}

struct Expander {
    Label *state;
    Scope *env;
    const List *next;
    static bool verbose;

    const Type *list_expander_func_type;

    Expander(Label *_state, Scope *_env, const List *_next = EOL) :
        state(_state),
        env(_env),
        next(_next),
        list_expander_func_type(nullptr) {
        list_expander_func_type = Pointer(Function(
            ReturnLabel({unknown_of(TYPE_List), unknown_of(TYPE_Scope)}),
            {TYPE_List, TYPE_Scope}), PTF_NonWritable, SYM_Unnamed);
    }

    ~Expander() {}

    bool is_goto_label(Any enter) {
        return (enter.type == TYPE_Label)
            && (enter.label->params[0]->type == TYPE_Nothing);
    }

    // arguments must include continuation
    // enter and args must be passed with syntax object removed
    void br(Any enter, const Args &args, uint64_t flags = 0) {
        assert(!args.empty());
        const Anchor *anchor = get_active_anchor();
        assert(anchor);
        if (!state) {
            set_active_anchor(anchor);
            location_error(String::from("can not define body: continuation already exited."));
            return;
        }
        assert(!is_goto_label(enter) || (args[0].value.type == TYPE_Nothing));
        assert(state->body.enter.type == TYPE_Nothing);
        assert(state->body.args.empty());
        state->body.flags = flags;
        state->body.enter = enter;
        state->body.args = args;
        state->body.anchor = anchor;
        state = nullptr;
    }

    bool is_instanced_dest(Any val) {
        return (val.type == TYPE_Parameter)
            || (val.type == TYPE_Label)
            || (val.type == TYPE_Nothing);
    }

    void verify_dest_not_none(Any dest) {
        if (dest.type == TYPE_Nothing) {
            location_error(String::from("attempting to implicitly return from label"));
        }
    }

    Any write_dest(const Any &dest) {
        if (dest.type == TYPE_Symbol) {
            return none;
        } else if (is_instanced_dest(dest)) {
            if (last_expression()) {
                verify_dest_not_none(dest);
                br(dest, { none });
            }
            return none;
        } else {
            assert(false && "illegal dest type");
        }
        return none;
    }

    Any write_dest(const Any &dest, const Any &value) {
        if (dest.type == TYPE_Symbol) {
            return value;
        } else if (is_instanced_dest(dest)) {
            if (last_expression()) {
                verify_dest_not_none(dest);
                br(dest, { none, value });
            }
            return value;
        } else {
            assert(false && "illegal dest type");
        }
        return none;
    }

    void expand_block(const List *it, const Any &dest) {
        assert(is_instanced_dest(dest));
        if (it == EOL) {
            br(dest, { none });
        } else {
            while (it) {
                next = it->next;
                const Syntax *sx = it->at;
                Any expr = sx->datum;
                if (!last_expression() && (expr.type == TYPE_String)) {
                    env->set_doc(expr);
                }
                expand(it->at, dest);
                it = next;
            }
        }
    }

    Any expand_syntax_extend(const List *it, const Any &dest) {
        auto _anchor = get_active_anchor();

        verify_list_parameter_count(it, 1, -1);

        // skip head
        it = it->next;

        Label *func = Label::from(_anchor, Symbol(KW_SyntaxExtend));

        auto retparam = Parameter::from(_anchor, Symbol(SYM_Unnamed), TYPE_Unknown);
        auto scopeparam = Parameter::from(_anchor, Symbol(SYM_SyntaxScope), TYPE_Unknown);

        func->append(retparam);
        func->append(scopeparam);

        Scope *subenv = Scope::from(env);
        subenv->bind(Symbol(SYM_SyntaxScope), scopeparam);

        Expander subexpr(func, subenv);

        subexpr.expand_block(it, retparam);

        set_active_anchor(_anchor);

        Args args;
        args.reserve(4);
        Label *nextstate = nullptr;
        Any result = none;
        if (dest.type == TYPE_Symbol) {
            nextstate = Label::continuation_from(_anchor, Symbol(SYM_Unnamed));
            Parameter *param = Parameter::variadic_from(_anchor, Symbol(SYM_Unnamed), TYPE_Unknown);
            nextstate->append(param);
            args.push_back(nextstate);
            result = param;
        } else if (is_instanced_dest(dest)) {
            args.push_back(dest);
        } else {
            assert(false && "syntax extend: illegal dest type");
        }
        args.push_back(func);
        args.push_back(Syntax::from(_anchor, next));
        args.push_back(env);
        //state = subexp.state;
        set_active_anchor(_anchor);
        br(Builtin(KW_SyntaxExtend), args);
        state = nextstate;
        next = EOL;
        return result;
    }

    Parameter *expand_parameter(Any value) {
        const Syntax *sxvalue = value;
        const Anchor *anchor = sxvalue->anchor;
        Any _value = sxvalue->datum;
        if (_value.type == TYPE_Parameter) {
            return _value.parameter;
        } else if (_value.type == TYPE_List && _value.list == EOL) {
            return Parameter::from(anchor, Symbol(SYM_Unnamed), TYPE_Nothing);
        } else {
            _value.verify(TYPE_Symbol);
            Parameter *param = nullptr;
            if (ends_with_parenthesis(_value.symbol)) {
                param = Parameter::variadic_from(anchor, _value.symbol, TYPE_Unknown);
            } else {
                param = Parameter::from(anchor, _value.symbol, TYPE_Unknown);
            }
            env->bind(_value.symbol, param);
            return param;
        }
    }

    struct ExpandFnSetup {
        bool label;
        bool inlined;

        ExpandFnSetup() {
            label = false;
            inlined = false;
        };
    };

    Any expand_fn(const List *it, const Any &dest, const ExpandFnSetup &setup) {
        auto _anchor = get_active_anchor();

        verify_list_parameter_count(it, 1, -1);

        // skip head
        it = it->next;

        assert(it != EOL);

        bool continuing = false;
        Label *func = nullptr;
        Any tryfunc_name = unsyntax(it->at);
        if (tryfunc_name.type == TYPE_Symbol) {
            // named self-binding
            // see if we can find a forward declaration in the local scope
            Any result = none;
            if (env->lookup_local(tryfunc_name.symbol, result)
                && (result.type == TYPE_Label)
                && !result.label->is_valid()) {
                func = result.label;
                continuing = true;
            } else {
                func = Label::from(_anchor, tryfunc_name.symbol);
                env->bind(tryfunc_name.symbol, func);
            }
            it = it->next;
        } else if (tryfunc_name.type == TYPE_String) {
            // named lambda
            func = Label::from(_anchor, Symbol(tryfunc_name.string));
            it = it->next;
        } else {
            // unnamed lambda
            func = Label::from(_anchor, Symbol(SYM_Unnamed));
        }
        if (setup.inlined)
            func->set_inline();

        Parameter *retparam = nullptr;
        if (continuing) {
            assert(!func->params.empty());
            retparam = func->params[0];
        } else {
            retparam = Parameter::from(_anchor, Symbol(SYM_Unnamed), setup.label?TYPE_Nothing:TYPE_Unknown);
            func->append(retparam);
        }

        if (it == EOL) {
            // forward declaration
            if (tryfunc_name.type != TYPE_Symbol) {
                location_error(setup.label?
                    String::from("forward declared label must be named")
                    :String::from("forward declared function must be named"));
            }

            return write_dest(dest);
        }

        const Syntax *sxplist = it->at;
        const List *params = sxplist->datum;

        it = it->next;

        Scope *subenv = Scope::from(env);
        // hidden self-binding for subsequent macros
        subenv->bind(SYM_ThisFnCC, func);
        Any subdest = none;
        if (!setup.label) {
            subenv->bind(KW_Recur, func);

            subdest = retparam;
            subenv->bind(KW_Return, retparam);
        }
        // ensure the local scope does not contain special symbols
        subenv = Scope::from(subenv);

        Expander subexpr(func, subenv);

        while (params != EOL) {
            func->append(subexpr.expand_parameter(params->at));
            params = params->next;
        }

        if ((it != EOL) && (it->next != EOL)) {
            Any val = unsyntax(it->at);
            if (val.type == TYPE_String) {
                func->docstring = val.string;
                it = it->next;
            }
        }

        subexpr.expand_block(it, subdest);

        if (state) {
            func->body.scope_label = state;
        }

        set_active_anchor(_anchor);
        return write_dest(dest, func);
    }

    bool is_return_parameter(Any val) {
        return (val.type == TYPE_Parameter) && (val.parameter->index == 0);
    }

    bool last_expression() {
        return next == EOL;
    }

    Label *make_nextstate(const Any &dest, Any &result, Any &subdest) {
        auto _anchor = get_active_anchor();
        Label *nextstate = nullptr;
        subdest = dest;
        if (dest.type == TYPE_Symbol) {
            nextstate = Label::continuation_from(_anchor, Symbol(SYM_Unnamed));
            Parameter *param = Parameter::variadic_from(_anchor,
                Symbol(SYM_Unnamed), TYPE_Unknown);
            nextstate->append(param);
            nextstate->set_inline();
            if (state) {
                nextstate->body.scope_label = state;
            }
            subdest = nextstate;
            result = param;
        } else if (is_instanced_dest(dest)) {
            if (!last_expression()) {
                nextstate = Label::continuation_from(_anchor, Symbol(SYM_Unnamed));
                if (state) {
                    nextstate->body.scope_label = state;
                }
                subdest = nextstate;
            }
        } else {
            assert(false && "illegal dest type");
        }
        return nextstate;
    }

    Any expand_defer(const List *it, const Any &dest) {
        auto _anchor = get_active_anchor();

        it = it->next;
        const List *body = it;
        const List *block = next;
        next = EOL;

        Label *nextstate = Label::continuation_from(_anchor, Symbol(SYM_Unnamed));

        expand_block(block, nextstate);

        state = nextstate;
        // read parameter names
        it = unsyntax(it->at);
        while (it != EOL) {
            nextstate->append(expand_parameter(it->at));
            it = it->next;
        }
        return expand_do(body, dest, false);
    }

    Any expand_do(const List *it, const Any &dest, bool new_scope) {
        auto _anchor = get_active_anchor();

        it = it->next;

        Any result = none;
        Any subdest = none;
        Label *nextstate = make_nextstate(dest, result, subdest);

        Label *func = Label::continuation_from(_anchor, Symbol(SYM_Unnamed));
        Scope *subenv = env;
        if (new_scope) {
            subenv = Scope::from(env);
        }
        Expander subexpr(func, subenv);
        subexpr.expand_block(it, subdest);

        set_active_anchor(_anchor);
        br(func, { none });
        state = nextstate;
        return result;
    }

    bool is_equal_token(const Any &name) {
        return (name.type == TYPE_Symbol) && (name.symbol == OP_Set);
    }

    void print_name_suggestions(Symbol name, StyledStream &ss) {
        auto syms = env->find_closest_match(name);
        if (!syms.empty()) {
            ss << "Did you mean '" << syms[0].name()->data << "'";
            for (size_t i = 1; i < syms.size(); ++i) {
                if ((i + 1) == syms.size()) {
                    ss << " or ";
                } else {
                    ss << ", ";
                }
                ss << "'" << syms[i].name()->data << "'";
            }
            ss << "?";
        }
    }

    // (let x ... [= args ...])
    // (let name ([x ...]) [= args ...])
    // ...
    Any expand_let(const List *it, const Any &dest) {

        verify_list_parameter_count(it, 1, -1);
        it = it->next;

        auto _anchor = get_active_anchor();

        Symbol labelname = Symbol(SYM_Unnamed);
        const List *params = nullptr;
        const List *values = nullptr;

        if (it) {
            auto name = unsyntax(it->at);
            auto nextit = it->next;
            if ((name.type == TYPE_Symbol) && nextit) {
                auto val = unsyntax(nextit->at);
                if (val.type == TYPE_List) {
                    labelname = name.symbol;
                    params = val.list;
                    nextit = nextit->next;
                    it = params;
                    if (nextit != EOL) {
                        if (!is_equal_token(unsyntax(nextit->at))) {
                            location_error(String::from("equal sign (=) expected"));
                        }
                        values = nextit;
                    }
                }
            }
        }

        auto endit = EOL;
        if (!params) {
            endit = it;
            // read parameter names
            while (endit) {
                auto name = unsyntax(endit->at);
                if (is_equal_token(name))
                    break;
                endit = endit->next;
            }
            if (endit != EOL)
                values = endit;
        }

        Label *nextstate = nullptr;
        if (!values) {
            // no assignments, reimport parameter names into local scope
            if (labelname != SYM_Unnamed) {
                nextstate = Label::continuation_from(_anchor, labelname);
                env->bind(labelname, nextstate);
            }

            while (it != endit) {
                auto name = unsyntax(it->at);
                name.verify(TYPE_Symbol);
                AnyDoc entry = { none, nullptr };
                if (!env->lookup(name.symbol, entry)) {
                    StyledString ss;
                    ss.out << "no such name bound in parent scope: '"
                        << name.symbol.name()->data << "'. ";
                    print_name_suggestions(name.symbol, ss.out);
                    location_error(ss.str());
                }
                env->bind_with_doc(name.symbol, entry);
                it = it->next;
            }

            if (nextstate) {
                br(nextstate, { none });
                state = nextstate;
            }

            return write_dest(dest);
        }

        nextstate = Label::continuation_from(_anchor, labelname);
        if (state) {
            nextstate->body.scope_label = state;
        }
        if (labelname != SYM_Unnamed) {
            env->bind(labelname, nextstate);
        } else {
            nextstate->set_inline();
        }

        size_t numparams = 0;
        // bind to fresh env so the rhs expressions don't see the symbols yet
        Scope *orig_env = env;
        env = Scope::from();
        // read parameter names
        while (it != endit) {
            nextstate->append(expand_parameter(it->at));
            numparams++;
            it = it->next;
        }

        if (nextstate->is_variadic()) {
            // accepts maximum number of arguments
            numparams = (size_t)-1;
        }

        it = values;

        Args args;
        args.reserve(it->count);
        args.push_back(none);

        it = it->next;

        // read init values
        Expander subexp(state, orig_env);
        size_t numvalues = 0;
        while (it) {
            numvalues++;
            if (numvalues > numparams) {
                set_active_anchor(((const Syntax *)it->at)->anchor);
                StyledString ss;
                ss.out << "number of arguments exceeds number of defined names ("
                    << numvalues << " > " << numparams << ")";
                location_error(ss.str());
            }
            subexp.next = it->next;
            args.push_back(subexp.expand(it->at, Symbol(SYM_Unnamed)));
            it = subexp.next;
        }

        //
        for (auto kv = env->map->begin(); kv != env->map->end(); ++kv) {
            orig_env->bind(kv->first, kv->second.value);
        }
        env = orig_env;

        set_active_anchor(_anchor);
        state = subexp.state;
        br(nextstate, args);
        state = nextstate;

        return write_dest(dest);
    }

    // quote <value> ...
    Any expand_quote(const List *it, const Any &dest) {
        //auto _anchor = get_active_anchor();

        verify_list_parameter_count(it, 1, -1);
        it = it->next;

        Any result = none;
        if (it->count == 1) {
            result = it->at;
        } else {
            result = it;
        }
        return write_dest(dest, strip_syntax(result));
    }

    Any expand_syntax_log(const List *it, const Any &dest) {
        //auto _anchor = get_active_anchor();

        verify_list_parameter_count(it, 1, 1);
        it = it->next;

        Any val = unsyntax(it->at);
        val.verify(TYPE_Symbol);

        auto sym = val.symbol;
        if (sym == KW_True) {
            this->verbose = true;
        } else if (sym == KW_False) {
            this->verbose = false;
        } else {
            // ignore
        }

        return write_dest(dest);
    }

    // (if cond body ...)
    // [(elseif cond body ...)]
    // [(else body ...)]
    Any expand_if(const List *it, const Any &dest) {
        auto _anchor = get_active_anchor();

        std::vector<const List *> branches;

    collect_branch:
        verify_list_parameter_count(it, 1, -1);
        branches.push_back(it);

        it = next;
        if (it != EOL) {
            auto itnext = it->next;
            const Syntax *sx = it->at;
            if (sx->datum.type == TYPE_List) {
                it = sx->datum;
                if (it != EOL) {
                    auto head = unsyntax(it->at);
                    if (head == Symbol(KW_ElseIf)) {
                        next = itnext;
                        goto collect_branch;
                    } else if (head == Symbol(KW_Else)) {
                        next = itnext;
                        branches.push_back(it);
                    } else {
                        branches.push_back(EOL);
                    }
                } else {
                    branches.push_back(EOL);
                }
            } else {
                branches.push_back(EOL);
            }
        } else {
            branches.push_back(EOL);
        }

        Any result = none;
        Any subdest = none;
        Label *nextstate = make_nextstate(dest, result, subdest);

        if (subdest.type == TYPE_Label) {
            subdest.label->unset_inline();
            subdest.label->set_merge();
        }

        int lastidx = (int)branches.size() - 1;
        for (int idx = 0; idx < lastidx; ++idx) {
            it = branches[idx];
            it = it->next;

            Label *thenstate = Label::inline_from(_anchor, Symbol(SYM_Unnamed));
            Label *elsestate = Label::inline_from(_anchor, Symbol(SYM_Unnamed));
            if (state) {
                thenstate->body.scope_label = state;
                elsestate->body.scope_label = state;
            }

            Expander subexp(state, env);
            subexp.next = it->next;
            Any cond = subexp.expand(it->at, Symbol(SYM_Unnamed));
            it = subexp.next;

            set_active_anchor(_anchor);
            state = subexp.state;

            br(Builtin(FN_Branch), { subdest, cond, thenstate, elsestate });

            subexp.env = Scope::from(env);
            subexp.state = thenstate;
            subexp.expand_block(it, thenstate->params[0]);

            state = elsestate;
        }

        assert(!state->is_basic_block_like());

        it = branches[lastidx];
        if (it != EOL) {
            it = it->next;
            Expander subexp(state, Scope::from(env));
            subexp.expand_block(it, state->params[0]);
        } else {
            br(state->params[0], { none });
        }

        state = nextstate;

        return result;
    }

    static bool get_kwargs(Any it, Argument &value) {
        it = unsyntax(it);
        if (it.type != TYPE_List) return false;
        auto l = it.list;
        if (l == EOL) return false;
        if (l->count != 3) return false;
        it = unsyntax(l->at);
        if (it.type != TYPE_Symbol) return false;
        value.key = it.symbol;
        l = l->next;
        it = unsyntax(l->at);
        if (it.type != TYPE_Symbol) return false;
        if (it.symbol != OP_Set) return false;
        l = l->next;
        value.value = l->at;
        return true;
    }

    Any expand_call(const List *it, const Any &dest, bool rawcall = false) {
        if (it == EOL)
            return write_dest(dest, it);
        auto _anchor = get_active_anchor();
        Expander subexp(state, env, it->next);

        Args args;
        args.reserve(it->count);

        Any result = none;
        Any subdest = none;
        Label *nextstate = make_nextstate(dest, result, subdest);
        args.push_back(subdest);

        Any enter = subexp.expand(it->at, Symbol(SYM_Unnamed));
        if (is_return_parameter(enter)) {
            assert(enter.parameter->type != TYPE_Nothing);
            args[0] = none;
            if (!last_expression()) {
                location_error(
                    String::from("return call must be last in statement list"));
            }
        } else if (is_goto_label(enter)) {
            args[0] = none;
        }

        it = subexp.next;
        while (it) {
            subexp.next = it->next;
            Argument value;
            set_active_anchor(((const Syntax *)it->at)->anchor);
            if (get_kwargs(it->at, value)) {
                value.value = subexp.expand(
                    value.value, Symbol(SYM_Unnamed));
            } else {
                value = subexp.expand(it->at, Symbol(SYM_Unnamed));
            }
            args.push_back(value);
            it = subexp.next;
        }

        state = subexp.state;
        set_active_anchor(_anchor);
        br(enter, args, rawcall?LBF_RawCall:0);
        state = nextstate;
        return result;
    }

    Any expand(const Syntax *sx, const Any &dest) {
    expand_again:
        set_active_anchor(sx->anchor);
        if (sx->quoted) {
            if (verbose) {
                StyledStream ss(std::cerr);
                ss << "quoting ";
                stream_expr(ss, sx, StreamExprFormat::debug_digest());
            }
            // return as-is
            return write_dest(dest, sx->datum);
        }
        Any expr = sx->datum;
        if (expr.type == TYPE_List) {
            if (verbose) {
                StyledStream ss(std::cerr);
                ss << "expanding list ";
                stream_expr(ss, sx, StreamExprFormat::debug_digest());
            }

            const List *list = expr.list;
            if (list == EOL) {
                location_error(String::from("expression is empty"));
            }

            Any head = unsyntax(list->at);

            // resolve symbol
            if (head.type == TYPE_Symbol) {
                env->lookup(head.symbol, head);
            }

            if (head.type == TYPE_Builtin) {
                Builtin func = head.builtin;
                switch(func.value()) {
                case KW_SyntaxLog: return expand_syntax_log(list, dest);
                case KW_Fn: {
                    return expand_fn(list, dest, ExpandFnSetup());
                }
                case KW_Inline: {
                    ExpandFnSetup setup;
                    setup.inlined = true;
                    return expand_fn(list, dest, setup);
                }
                case KW_Label: {
                    ExpandFnSetup setup;
                    setup.label = true;
                    return expand_fn(list, dest, setup);
                }
                case KW_SyntaxExtend: return expand_syntax_extend(list, dest);
                case KW_Let: return expand_let(list, dest);
                case KW_If: return expand_if(list, dest);
                case KW_Quote: return expand_quote(list, dest);
                case KW_Defer: return expand_defer(list, dest);
                case KW_Do: return expand_do(list, dest, true);
                case KW_DoIn: return expand_do(list, dest, false);
                case KW_RawCall:
                case KW_Call: {
                    verify_list_parameter_count(list, 1, -1);
                    list = list->next;
                    assert(list != EOL);
                    return expand_call(list, dest, func.value() == KW_RawCall);
                } break;
                default: break;
                }
            }

            Any list_handler = none;
            if (env->lookup(Symbol(SYM_ListWildcard), list_handler)) {
                if (list_handler.type != list_expander_func_type) {
                    StyledString ss;
                    ss.out << "custom list expander has wrong type "
                        << list_handler.type << ", must be "
                        << list_expander_func_type;
                    location_error(ss.str());
                }
                struct ListScopePair { const List *topit; Scope *env; };
                typedef ListScopePair (*HandlerFuncType)(const List *, Scope *);
                HandlerFuncType f = (HandlerFuncType)list_handler.pointer;
                auto result = f(List::from(sx, next), env);
                const Syntax *newsx = result.topit->at;
                if (newsx != sx) {
                    sx = newsx;
                    next = result.topit->next;
                    env = result.env;
                    goto expand_again;
                } else if (verbose) {
                    StyledStream ss(std::cerr);
                    ss << "ignored by list handler" << std::endl;
                }
            }
            return expand_call(list, dest);
        } else if (expr.type == TYPE_Symbol) {
            if (verbose) {
                StyledStream ss(std::cerr);
                ss << "expanding symbol ";
                stream_expr(ss, sx, StreamExprFormat::debug_digest());
            }

            Symbol name = expr.symbol;

            Any result = none;
            if (!env->lookup(name, result)) {
                Any symbol_handler = none;
                if (env->lookup(Symbol(SYM_SymbolWildcard), symbol_handler)) {
                    if (symbol_handler.type != list_expander_func_type) {
                        StyledString ss;
                        ss.out << "custom symbol expander has wrong type "
                            << symbol_handler.type << ", must be "
                            << list_expander_func_type;
                        location_error(ss.str());
                    }
                    struct ListScopePair { const List *topit; Scope *env; };
                    typedef ListScopePair (*HandlerFuncType)(const List *, Scope *);
                    HandlerFuncType f = (HandlerFuncType)symbol_handler.pointer;
                    auto result = f(List::from(sx, next), env);
                    const Syntax *newsx = result.topit->at;
                    if (newsx != sx) {
                        sx = newsx;
                        next = result.topit->next;
                        env = result.env;
                        goto expand_again;
                    }
                }

                StyledString ss;
                ss.out << "use of undeclared identifier '" << name.name()->data << "'. ";
                print_name_suggestions(name, ss.out);
                location_error(ss.str());
            }
            return write_dest(dest, result);
        } else {
            if (verbose) {
                StyledStream ss(std::cerr);
                ss << "ignoring ";
                stream_expr(ss, sx, StreamExprFormat::debug_digest());
            }
            return write_dest(dest, expr);
        }
    }

};

bool Expander::verbose = false;

static Label *expand_module(Any expr, Scope *scope) {
    const Anchor *anchor = get_active_anchor();
    if (expr.type == TYPE_Syntax) {
        anchor = expr.syntax->anchor;
        set_active_anchor(anchor);
        expr = expr.syntax->datum;
    }
    expr.verify(TYPE_List);
    assert(anchor);
    Label *mainfunc = Label::function_from(anchor, anchor->path());

    Expander subexpr(mainfunc, scope?scope:globals);
    subexpr.expand_block(expr, mainfunc->params[0]);

    return mainfunc;
}

//------------------------------------------------------------------------------
// GLOBALS
//------------------------------------------------------------------------------

#define DEFINE_TYPENAME(NAME, T) \
    T = Typename(String::from(NAME));

#define DEFINE_BASIC_TYPE(NAME, CT, T, BODY) { \
        T = Typename(String::from(NAME)); \
        auto tn = cast<TypenameType>(const_cast<Type *>(T)); \
        tn->finalize(BODY); \
        assert(sizeof(CT) == size_of(T)); \
    }

#define DEFINE_STRUCT_TYPE(NAME, CT, T, ...) { \
        T = Typename(String::from(NAME)); \
        auto tn = cast<TypenameType>(const_cast<Type *>(T)); \
        tn->finalize(Tuple({ __VA_ARGS__ })); \
        assert(sizeof(CT) == size_of(T)); \
    }

#define DEFINE_STRUCT_HANDLE_TYPE(NAME, CT, T, ...) { \
        T = Typename(String::from(NAME)); \
        auto tn = cast<TypenameType>(const_cast<Type *>(T)); \
        auto ET = Tuple({ __VA_ARGS__ }); \
        assert(sizeof(CT) == size_of(ET)); \
        tn->finalize(NativeROPointer(ET)); \
    }

#define DEFINE_OPAQUE_HANDLE_TYPE(NAME, CT, T) { \
        T = Typename(String::from(NAME)); \
        auto tn = cast<TypenameType>(const_cast<Type *>(T)); \
        tn->finalize(NativeROPointer(Typename(String::from("_" NAME)))); \
    }

static void init_types() {
    DEFINE_TYPENAME("typename", TYPE_Typename);

    DEFINE_TYPENAME("void", TYPE_Void);
    DEFINE_TYPENAME("Nothing", TYPE_Nothing);

    DEFINE_TYPENAME("Sampler", TYPE_Sampler);

    DEFINE_TYPENAME("integer", TYPE_Integer);
    DEFINE_TYPENAME("real", TYPE_Real);
    DEFINE_TYPENAME("pointer", TYPE_Pointer);
    DEFINE_TYPENAME("array", TYPE_Array);
    DEFINE_TYPENAME("vector", TYPE_Vector);
    DEFINE_TYPENAME("tuple", TYPE_Tuple);
    DEFINE_TYPENAME("union", TYPE_Union);
    DEFINE_TYPENAME("ReturnLabel", TYPE_ReturnLabel);
    DEFINE_TYPENAME("constant", TYPE_Constant);
    DEFINE_TYPENAME("function", TYPE_Function);
    DEFINE_TYPENAME("extern", TYPE_Extern);
    DEFINE_TYPENAME("Image", TYPE_Image);
    DEFINE_TYPENAME("SampledImage", TYPE_SampledImage);
    DEFINE_TYPENAME("CStruct", TYPE_CStruct);
    DEFINE_TYPENAME("CUnion", TYPE_CUnion);
    DEFINE_TYPENAME("CEnum", TYPE_CEnum);

    TYPE_Bool = Integer(1, false);

    TYPE_I8 = Integer(8, true);
    TYPE_I16 = Integer(16, true);
    TYPE_I32 = Integer(32, true);
    TYPE_I64 = Integer(64, true);

    TYPE_U8 = Integer(8, false);
    TYPE_U16 = Integer(16, false);
    TYPE_U32 = Integer(32, false);
    TYPE_U64 = Integer(64, false);

    TYPE_F16 = Real(16);
    TYPE_F32 = Real(32);
    TYPE_F64 = Real(64);
    TYPE_F80 = Real(80);

    DEFINE_BASIC_TYPE("usize", size_t, TYPE_USize, TYPE_U64);

    TYPE_Type = Typename(String::from("type"));
    TYPE_Unknown = Typename(String::from("Unknown"));
    const Type *_TypePtr = NativeROPointer(Typename(String::from("_type")));
    cast<TypenameType>(const_cast<Type *>(TYPE_Type))->finalize(_TypePtr);
    cast<TypenameType>(const_cast<Type *>(TYPE_Unknown))->finalize(_TypePtr);

    cast<TypenameType>(const_cast<Type *>(TYPE_Nothing))->finalize(Tuple({}));

    DEFINE_BASIC_TYPE("Symbol", Symbol, TYPE_Symbol, TYPE_U64);
    DEFINE_BASIC_TYPE("Builtin", Builtin, TYPE_Builtin, TYPE_U64);

    DEFINE_STRUCT_TYPE("Any", Any, TYPE_Any,
        TYPE_Type,
        TYPE_U64
    );

    DEFINE_OPAQUE_HANDLE_TYPE("SourceFile", SourceFile, TYPE_SourceFile);
    DEFINE_OPAQUE_HANDLE_TYPE("Label", Label, TYPE_Label);
    DEFINE_OPAQUE_HANDLE_TYPE("Parameter", Parameter, TYPE_Parameter);
    DEFINE_OPAQUE_HANDLE_TYPE("Scope", Scope, TYPE_Scope);
    DEFINE_OPAQUE_HANDLE_TYPE("Frame", Frame, TYPE_Frame);
    DEFINE_OPAQUE_HANDLE_TYPE("Closure", Closure, TYPE_Closure);

    DEFINE_STRUCT_HANDLE_TYPE("Anchor", Anchor, TYPE_Anchor,
        NativeROPointer(TYPE_SourceFile),
        TYPE_I32,
        TYPE_I32,
        TYPE_I32
    );

    {
        TYPE_List = Typename(String::from("list"));

        const Type *cellT = Typename(String::from("_list"));
        auto tn = cast<TypenameType>(const_cast<Type *>(cellT));
        auto ET = Tuple({ TYPE_Any,
            NativeROPointer(cellT), TYPE_USize });
        assert(sizeof(List) == size_of(ET));
        tn->finalize(ET);

        cast<TypenameType>(const_cast<Type *>(TYPE_List))
            ->finalize(NativeROPointer(cellT));
    }

    DEFINE_STRUCT_HANDLE_TYPE("Syntax", Syntax, TYPE_Syntax,
        TYPE_Anchor,
        TYPE_Any,
        TYPE_Bool);

    DEFINE_STRUCT_HANDLE_TYPE("string", String, TYPE_String,
        TYPE_USize,
        Array(TYPE_I8, 1)
    );

    DEFINE_STRUCT_HANDLE_TYPE("Exception", Exception, TYPE_Exception,
        TYPE_Anchor,
        TYPE_String);

#define T(TYPE, TYPENAME) \
    assert(TYPE);
    B_TYPES()
#undef T
}

#undef DEFINE_TYPENAME
#undef DEFINE_BASIC_TYPE
#undef DEFINE_STRUCT_TYPE
#undef DEFINE_STRUCT_HANDLE_TYPE
#undef DEFINE_OPAQUE_HANDLE_TYPE
#undef DEFINE_STRUCT_TYPE

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
    Specializer solver;
    return solver.typify(Frame::root, expand_module(expr, scope), {});
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
    Specializer solver;
    return solver.typify(srcl->frame, srcl->label, types);
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
    Specializer::enable_step_debugger = true;
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
    return CityHash64((const char *)&data, (size > 8)?8:size);
}

static uint64_t f_hash2x64(uint64_t a, uint64_t b) {
    return HashLen16(a, b);
}

static uint64_t f_hashbytes (const char *data, size_t size) {
    return CityHash64(data, size);
}

static void init_globals(int argc, char *argv[]) {

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
}

//------------------------------------------------------------------------------
// SCOPES CORE
//------------------------------------------------------------------------------

/* this function looks for a header at the end of the compiler executable
   that indicates a scopes core.

   the header has the format (core-size <size>), where size is a i32 value
   holding the size of the core source file in bytes.

   the compiler uses this function to override the default scopes core 'core.sc'
   located in the compiler's directory.

   to later override the default core file and load your own, cat the new core
   file behind the executable and append the header, like this:

   $ cp scopes myscopes
   $ cat mycore.sc >> myscopes
   $ echo "(core-size " >> myscopes
   $ wc -c < mycore.sc >> myscopes
   $ echo ")" >> myscopes

   */

static Any load_custom_core(const char *executable_path) {
    // attempt to read bootstrap expression from end of binary
    auto file = SourceFile::from_file(
        Symbol(String::from_cstr(executable_path)));
    if (!file) {
        stb_fprintf(stderr, "could not open binary\n");
        return none;
    }
    auto ptr = file->strptr();
    auto size = file->size();
    auto cursor = ptr + size - 1;
    while ((*cursor == '\n')
        || (*cursor == '\r')
        || (*cursor == ' ')) {
        // skip the trailing text formatting garbage
        // that win32 echo produces
        cursor--;
        if (cursor < ptr) return none;
    }
    if (*cursor != ')') return none;
    cursor--;
    // seek backwards to find beginning of expression
    while ((cursor >= ptr) && (*cursor != '('))
        cursor--;

    LexerParser footerParser(file, cursor - ptr);
    auto expr = footerParser.parse();
    if (expr.type == TYPE_Nothing) {
        stb_fprintf(stderr, "could not parse footer expression\n");
        return none;
    }
    expr = strip_syntax(expr);
    if ((expr.type != TYPE_List) || (expr.list == EOL)) {
        stb_fprintf(stderr, "footer parser returned illegal structure\n");
        return none;
    }
    expr = ((const List *)expr)->at;
    if (expr.type != TYPE_List)  {
        stb_fprintf(stderr, "footer expression is not a symbolic list\n");
        return none;
    }
    auto symlist = expr.list;
    auto it = symlist;
    if (it == EOL) {
        stb_fprintf(stderr, "footer expression is empty\n");
        return none;
    }
    auto head = it->at;
    it = it->next;
    if (head.type != TYPE_Symbol)  {
        stb_fprintf(stderr, "footer expression does not begin with symbol\n");
        return none;
    }
    if (head != Any(Symbol("core-size")))  {
        stb_fprintf(stderr, "footer expression does not begin with 'core-size'\n");
        return none;
    }
    if (it == EOL) {
        stb_fprintf(stderr, "footer expression needs two arguments\n");
        return none;
    }
    auto arg = it->at;
    it = it->next;
    if (arg.type != TYPE_I32)  {
        stb_fprintf(stderr, "script-size argument is not of type i32\n");
        return none;
    }
    auto script_size = arg.i32;
    if (script_size <= 0) {
        stb_fprintf(stderr, "script-size must be larger than zero\n");
        return none;
    }
    LexerParser parser(file, cursor - script_size - ptr, script_size);
    return parser.parse();
}

//------------------------------------------------------------------------------
// MAIN
//------------------------------------------------------------------------------

static bool terminal_supports_ansi() {
#ifdef SCOPES_WIN32
    if (isatty(STDOUT_FILENO))
        return true;
    return getenv("TERM") != nullptr;
#else
    //return isatty(fileno(stdout));
    return isatty(STDOUT_FILENO);
#endif
}

static void setup_stdio() {
    if (terminal_supports_ansi()) {
        stream_default_style = stream_ansi_style;
        #ifdef SCOPES_WIN32
        #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
        #endif

        // turn on ANSI code processing
        auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        auto hStdErr = GetStdHandle(STD_ERROR_HANDLE);
        DWORD mode;
        GetConsoleMode(hStdOut, &mode);
        SetConsoleMode(hStdOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        GetConsoleMode(hStdErr, &mode);
        SetConsoleMode(hStdErr, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        setbuf(stdout, 0);
        setbuf(stderr, 0);
        SetConsoleOutputCP(65001);
        #endif
    }
}

static void on_shutdown() {
#if SCOPES_PRINT_TIMERS
    Timer::print_timers();
    std::cerr << "largest recorded stack size: " << g_largest_stack_size << std::endl;
#endif
}

} // namespace scopes

#ifndef SCOPES_WIN32
static void crash_handler(int sig) {
  void *array[20];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 20);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}
#endif

// This function isn't referenced outside its translation unit, but it
// can't use the "static" keyword because its address is used for
// GetMainExecutable (since some platforms don't support taking the
// address of main, and some platforms can't implement GetMainExecutable
// without being given the address of a function in the main executable).
std::string GetExecutablePath(const char *Argv0) {
  // This just needs to be some symbol in the binary; C++ doesn't
  // allow taking the address of ::main however.
  void *MainAddr = (void*) (intptr_t) GetExecutablePath;
  return llvm::sys::fs::getMainExecutable(Argv0, MainAddr);
}

int main(int argc, char *argv[]) {
    using namespace scopes;
    uint64_t c = 0;
    g_stack_start = (char *)&c;

    Frame::root = new Frame();

    Symbol::_init_symbols();
    init_llvm();

    setup_stdio();
    scopes_argc = argc;
    scopes_argv = argv;

    scopes::global_c_namespace = dlopen(NULL, RTLD_LAZY);

    scopes_compiler_path = nullptr;
    scopes_compiler_dir = nullptr;
    scopes_clang_include_dir = nullptr;
    scopes_include_dir = nullptr;
    if (argv) {
        if (argv[0]) {
            std::string loader = GetExecutablePath(argv[0]);
            // string must be kept resident
            scopes_compiler_path = strdup(loader.c_str());
        } else {
            scopes_compiler_path = strdup("");
        }

        char *path_copy = strdup(scopes_compiler_path);
        scopes_compiler_dir = format("%s/..", dirname(path_copy))->data;
        free(path_copy);
        scopes_clang_include_dir = format("%s/lib/clang/include", scopes_compiler_dir)->data;
        scopes_include_dir = format("%s/include", scopes_compiler_dir)->data;
    }

    init_types();
    init_globals(argc, argv);

    linenoiseSetCompletionCallback(prompt_completion_cb);

    Any expr = load_custom_core(scopes_compiler_path);
    if (expr != none) {
        goto skip_regular_load;
    }

    {
        SourceFile *sf = nullptr;
#if 0
        Symbol name = format("%s/lib/scopes/%i.%i.%i/core.sc",
            scopes_compiler_dir,
            SCOPES_VERSION_MAJOR,
            SCOPES_VERSION_MINOR,
            SCOPES_VERSION_PATCH);
#else
        Symbol name = format("%s/lib/scopes/core.sc",
            scopes_compiler_dir);
#endif
        sf = SourceFile::from_file(name);
        if (!sf) {
            location_error(String::from("core missing\n"));
        }
        LexerParser parser(sf);
        expr = parser.parse();
    }

skip_regular_load:
    Label *fn = expand_module(expr, Scope::from(globals));

#if SCOPES_DEBUG_CODEGEN
    StyledStream ss(std::cout);
    std::cout << "non-normalized:" << std::endl;
    stream_label(ss, fn, StreamLabelFormat::debug_all());
    std::cout << std::endl;
#endif

    Specializer solver;
    fn = solver.typify(Frame::root, fn, {});
#if SCOPES_DEBUG_CODEGEN
    std::cout << "normalized:" << std::endl;
    stream_label(ss, fn, StreamLabelFormat::debug_all());
    std::cout << std::endl;
#endif

    typedef void (*MainFuncType)();
    MainFuncType fptr = (MainFuncType)compile(fn, 0).pointer;
    fptr();

    return 0;
}
