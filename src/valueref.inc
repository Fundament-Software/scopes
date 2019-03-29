/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_VALUEREF_INC
#define SCOPES_VALUEREF_INC

#include "value_kind.hpp"
#include "dyn_cast.inc"

namespace scopes {

struct Anchor;
struct List;
struct Value;
struct TypedValue;
struct Instruction;
struct Const;
struct Pure;

//------------------------------------------------------------------------------

template<typename T>
struct TValueRef {
#if 0
    struct Hash {
        std::size_t operator()(const TValueRef<T> & s) const {
            return s.hash();
        }
    };
#endif

    TValueRef () : _value(nullptr), _anchor(nullptr) {}
    TValueRef (const Anchor *anchor, T *value) : _value(value), _anchor(anchor) {
        assert(value); assert(anchor); }

    template<typename S>
    TValueRef (const TValueRef<S> &value)
        : _value(value.unref()), _anchor(value.unsafe_anchor()) {}

    template<typename S>
    TValueRef<S> cast() const {
        return TValueRef<S>(_anchor, llvm::cast<S>(_value));
    }
    template<typename S>
    TValueRef<S> dyn_cast() const {
        auto ptr = llvm::dyn_cast<S>(_value);
        if (ptr) {
            return TValueRef<S>(_anchor, ptr);
        } else {
            return TValueRef<S>();
        }
    }
    template<typename S>
    bool isa() const { return llvm::isa<S>(_value); }

    template<typename S>
    bool operator ==(const TValueRef<S> &other) const {
        return _value == other.unref(); }
    template<typename S>
    bool operator !=(const TValueRef<S> &other) const {
        return _value != other.unref(); }

    /*
    template<typename S>
    operator TValueRef<S>() const { return TValueRef<S>(_anchor, _value); }
    */

    operator bool() const { return _value != nullptr; }

    T *operator ->() const { assert (_value); return _value; }
    const Anchor *anchor() const { assert(_anchor); return _anchor; }
    const Anchor *unsafe_anchor() const { return _anchor; }
    std::size_t hash() const { return std::hash<T *>{}(_value); }

    T *unref() const { return _value; }

protected:
    T *_value;
    const Anchor *_anchor;
};

template<typename T>
struct TConstValueRef {
#if 0
    struct Hash {
        std::size_t operator()(const TConstValueRef<T> & s) const {
            return s.hash();
        }
    };
#endif

    TConstValueRef () : _value(nullptr), _anchor(nullptr) {}
    TConstValueRef (const Anchor *anchor, const T *value)
        : _value(value), _anchor(anchor) {
        assert(anchor); }

    template<typename S>
    TConstValueRef (const TValueRef<S> &value)
        : _value(value.unref()), _anchor(value.unsafe_anchor()) {}

    template<typename S>
    TConstValueRef<S> cast() const {
        return TConstValueRef<S>(_anchor, llvm::cast<S>(_value));
    }
    template<typename S>
    TConstValueRef<S> dyn_cast() const {
        return TConstValueRef<S>(_anchor, llvm::dyn_cast<S>(_value));
    }
    template<typename S>
    bool isa() const { return llvm::isa<S>(_value); }

    /*
    template<typename S>
    operator TConstValueRef<S>() const {
        return TConstValueRef<S>(_anchor, _value); }
    */

    operator bool() const { return _value != nullptr; }

    const T *operator ->() const { assert (_value); return _value; }
    const Anchor *anchor() const { assert(_anchor); return _anchor; }
    std::size_t hash() const { return std::hash<const T *>{}(_value); }

    const T *unref() const { return _value; }

protected:
    const T *_value;
    const Anchor *_anchor;
};

typedef TConstValueRef<List> ListRef;

typedef TValueRef<Value> ValueRef;
typedef TValueRef<TypedValue> TypedValueRef;
typedef TValueRef<Instruction> InstructionRef;
typedef TValueRef<Const> ConstRef;
typedef TValueRef<Pure> PureRef;
// template instances
#define T(NAME, BNAME, CLASS) typedef TValueRef<CLASS> CLASS ## Ref;
    SCOPES_VALUE_KIND()
#undef T

template<typename T>
inline TValueRef<T> ref(const Anchor *anchor, T *value) {
    return TValueRef<T>(anchor, value);
}

template<typename T>
inline TValueRef<T> ref(const Anchor *anchor, const TValueRef<T> &value) {
    return TValueRef<T>(anchor, value.unref());
}

template<typename T>
inline TConstValueRef<T> ref(const Anchor *anchor, const T *value) {
    return TConstValueRef<T>(anchor, value);
}

template<typename T>
inline TConstValueRef<T> ref(const Anchor *anchor, const TConstValueRef<T> &value) {
    return TConstValueRef<T>(anchor, value.unref());
}

//------------------------------------------------------------------------------

} // namespace scopes

#endif // SCOPES_VALUEREF_INC