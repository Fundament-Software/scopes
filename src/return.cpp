/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "return.hpp"
#include "tuple.hpp"
#include "hash.hpp"
#include "dyn_cast.inc"

#include <assert.h>

#include <unordered_set>

namespace scopes {

namespace ReturnSet {
    struct Hash {
        std::size_t operator()(const ReturnType *s) const {
            std::size_t h = std::hash<uint64_t>{}(s->flags);
            for (auto &&arg : s->values) {
                h = hash2(h, arg.hash());
            }
            return h;
        }
    };

    struct KeyEqual {
        bool operator()( const ReturnType *lhs, const ReturnType *rhs ) const {
            return (lhs->flags == rhs->flags)
                && (lhs->values == rhs->values);
        }
    };
} // namespace ReturnSet

static std::unordered_set<const ReturnType *, ReturnSet::Hash, ReturnSet::KeyEqual> returns;

//------------------------------------------------------------------------------
// RETURN TYPE
//------------------------------------------------------------------------------

bool ReturnType::classof(const Type *T) {
    return T->kind() == TK_Return;
}

bool ReturnType::is_raising() const {
    return flags & RTF_Raising;
}

bool ReturnType::is_returning() const {
    return !(flags & RTF_NoReturn);
}

const ReturnType *ReturnType::to_trycall() const {
    if (!is_raising())
        return this;
    return KeyedReturn(values, flags & ~(RTF_Raising | RTF_NoReturn));
}

const ReturnType *ReturnType::to_raising() const {
    if (is_raising())
        return this;
    return KeyedReturn(values, flags | RTF_Raising);
}

const ReturnType *ReturnType::to_single(Symbol key) const {
    if (!is_returning())
        return this;
    int starti = (is_raising()?1:0);
    if (values.size() == starti)
        return KeyedReturn({KeyedType(key, TYPE_Nothing)}, flags);
    else
        return KeyedReturn({KeyedType(key, values[starti].type)}, flags);
}

void ReturnType::stream_name(StyledStream &ss) const {
    if (is_returning()) {
        ss << "λ(";
        size_t starti = is_raising()?1:0;
        for (size_t i = starti; i < values.size(); ++i) {
            if (i > starti) {
                ss << " ";
            }
            if (values[i].key != SYM_Unnamed) {
                ss << values[i].key.name()->data << "=";
            }
            stream_type_name(ss, values[i].type);
        }
        ss << ")";
    } else {
        ss << "λ<noreturn>";
    }
    if (is_raising()) {
        ss << "!";
    }
}

ReturnType::ReturnType(const KeyedTypes &_values, uint64_t _flags)
    : Type(TK_Return) {
    flags = _flags;
    if (is_returning()) {
        int count = (int)_values.size();
        if (is_raising()) {
            values.push_back(TYPE_Bool);
        }
        for (int i = 0; i < count; ++i) {
            auto &&value = _values[i];
            assert(!isa<ReturnType>(value.type));
            values.push_back(value);
        }
        if (values.empty()) {
            return_type = TYPE_Void;
        } else if (values.size() == 1) {
            return_type = values[0].type;
        } else {
            return_type = KeyedTuple(values).assert_ok();
        }
    } else {
        return_type = TYPE_Void;
    }
}

//------------------------------------------------------------------------------

const ReturnType *KeyedReturn(const KeyedTypes &values, uint64_t flags) {
    ReturnType key(values, flags);
    auto it = returns.find(&key);
    if (it != returns.end())
        return *it;
    auto result = new ReturnType(values, flags);
    returns.insert(result);
    return result;
}

const ReturnType *Return(const ArgTypes &values, uint64_t flags) {
    KeyedTypes types;
    for (auto &&val : values) {
        types.push_back(val);
    }
    return KeyedReturn(types, flags);
}

const ReturnType *NoReturn(uint64_t flags) {
    return KeyedReturn({}, flags | RTF_NoReturn);
}

} // namespace scopes
