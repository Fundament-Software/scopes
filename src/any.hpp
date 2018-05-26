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

#ifndef SCOPES_ANY_HPP
#define SCOPES_ANY_HPP

#include "symbol.hpp"
#include "builtin.hpp"
#include "none.hpp"

#include <stdint.h>

#include <cstddef>

namespace scopes {

//------------------------------------------------------------------------------
// ANY
//------------------------------------------------------------------------------

struct Type;
struct Syntax;
struct List;
struct Label;
struct Parameter;
struct Scope;
struct Exception;
struct Frame;
struct Closure;
struct String;
struct Anchor;

struct Any {
    struct Hash {
        std::size_t operator()(const Any & s) const;
    };

    const Type *type;
    union {
        char content[8];
        bool i1;
        int8_t i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        size_t sizeval;
        float f32;
        double f64;
        const Type *typeref;
        const String *string;
        Symbol symbol;
        const Syntax *syntax;
        const Anchor *anchor;
        const List *list;
        Label *label;
        Parameter *parameter;
        Builtin builtin;
        Scope *scope;
        Any *ref;
        void *pointer;
        const Exception *exception;
        Frame *frame;
        const Closure *closure;
    };

    Any(Nothing x);
    Any(const Type *x);
    Any(bool x);
    Any(int8_t x);
    Any(int16_t x);
    Any(int32_t x);
    Any(int64_t x);
    Any(uint8_t x);
    Any(uint16_t x);
    Any(uint32_t x);
    Any(uint64_t x);
#ifdef SCOPES_MACOS
    Any(unsigned long x);
#endif
    Any(float x);
    Any(double x);
    Any(const String *x);
    Any(Symbol x);
    Any(const Syntax *x);
    Any(const Anchor *x);
    Any(const List *x);
    Any(const Exception *x);
    Any(Label *x);
    Any(Parameter *x);
    Any(Builtin x);
    Any(Scope *x);
    Any(Frame *x);
    Any(const Closure *x);
#if 0
    template<unsigned N>
    Any(const char (&str)[N]) : type(TYPE_String), string(String::from(str)) {}
#endif
    // a catch-all for unsupported types
    template<typename T>
    Any(const T &x);

    Any toref();

    static Any from_opaque(const Type *type);

    static Any from_pointer(const Type *type, void *ptr);

    void verify(const Type *T) const;
    void verify_indirect(const Type *T) const;
    const Type *indirect_type() const;
    bool is_const() const;

    operator const Type *() const;
    operator const List *() const;
    operator const Syntax *() const;
    operator const Anchor *() const;
    operator const String *() const;
    operator const Exception *() const;
    operator Label *() const;
    operator Scope *() const;
    operator Parameter *() const;
    operator const Closure *() const;
    operator Frame *() const;

    struct AnyStreamer {
        StyledStream& ost;
        const Type *type;
        bool annotate_type;

        AnyStreamer(StyledStream& _ost, const Type *_type, bool _annotate_type);

        void stream_type_suffix() const;

        template<typename T>
        void naked(const T &x) const {
            ost << x;
        }
        template<typename T>
        void typed(const T &x) const {
            ost << x;
            stream_type_suffix();
        }
    };

    StyledStream& stream(StyledStream& ost, bool annotate_type = true) const;

    bool operator ==(const Any &other) const;

    bool operator !=(const Any &other) const;

    size_t hash() const;
};

StyledStream& operator<<(StyledStream& ost, Any value);

bool is_unknown(const Any &value);

bool is_typed(const Any &value);

Any unknown_of(const Type *T);

Any untyped();

} // namespace scopes

#endif // SCOPES_ANY_HPP