/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_ANY_HPP
#define SCOPES_ANY_HPP

#include "symbol.hpp"
#include "builtin.hpp"
#include "none.hpp"
#include "result.hpp"

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
struct Error;
struct Frame;
struct Closure;
struct String;
struct Anchor;

#define SCOPES_RESULT_CAST_OPERATOR(T) \
    operator Result<T>() const; \
    operator T() const

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
        const Error *error;
        Frame *frame;
        const Closure *closure;
    };

    Any();
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
    Any(const Error *x);
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
    Any(const T &x) __attribute__((deprecated("illegal constructor")));


    Any toref();

    static Any from_opaque(const Type *type);

    static Any from_pointer(const Type *type, void *ptr);

    SCOPES_RESULT(void) verify(const Type *T) const;
    SCOPES_RESULT(void) verify_indirect(const Type *T) const;
    const Type *indirect_type() const;
    bool is_const() const;

    SCOPES_RESULT_CAST_OPERATOR(const Type *);
    SCOPES_RESULT_CAST_OPERATOR(const List *);
    SCOPES_RESULT_CAST_OPERATOR(const Syntax *);
    SCOPES_RESULT_CAST_OPERATOR(const Anchor *);
    SCOPES_RESULT_CAST_OPERATOR(const String *);
    SCOPES_RESULT_CAST_OPERATOR(const Error *);
    SCOPES_RESULT_CAST_OPERATOR(Label *);
    SCOPES_RESULT_CAST_OPERATOR(Scope *);
    SCOPES_RESULT_CAST_OPERATOR(Parameter *);
    SCOPES_RESULT_CAST_OPERATOR(const Closure *);
    SCOPES_RESULT_CAST_OPERATOR(Frame *);

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

bool is_unknown(const Any &value);

bool is_typed(const Any &value);

Any unknown_of(const Type *T);

Any untyped();

} // namespace scopes

#endif // SCOPES_ANY_HPP