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

const Type *ReturnType::to_trycall() const {
    if (!is_raising())
        return this;
    return KeyedReturn(values, flags & ~(RTF_Raising | RTF_NoReturn));
}

const Type *ReturnType::to_raising() const {
    if (is_raising())
        return this;
    return KeyedReturn(values, flags | RTF_Raising);
}

const Type *ReturnType::type_at_index(size_t i) const {
    if (!is_returning())
        return this;
    if (i < values.size())
        return values[i].type;
    return TYPE_Nothing;
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
            assert(values.size() != 1);
            return_type = KeyedTuple(values).assert_ok();
        }
    } else {
        return_type = TYPE_Void;
    }
}

//------------------------------------------------------------------------------

const Type *KeyedReturn(const KeyedTypes &values, uint64_t flags) {
    if (!flags) {
        if (values.size() == 0)
            return TYPE_Void;
        else if ((values.size() == 1)
                && (values[0].key == SYM_Unnamed))
            return values[0].type;
    }
    ReturnType key(values, flags);
    auto it = returns.find(&key);
    if (it != returns.end())
        return *it;
    if (flags & RTF_NoReturn) {
        assert(values.size() == 0);
    }
    auto result = new ReturnType(values, flags);
    returns.insert(result);
    return result;
}

const Type *Return(const ArgTypes &values, uint64_t flags) {
    KeyedTypes types;
    for (auto &&val : values) {
        types.push_back(val);
    }
    return KeyedReturn(types, flags);
}

const Type *NoReturn(uint64_t flags) {
    return KeyedReturn({}, flags | RTF_NoReturn);
}

bool is_raising(const Type *T) {
    auto rt = dyn_cast<ReturnType>(T);
    if (rt)
        return rt->is_raising();
    return true;
}

bool is_returning(const Type *T) {
    auto rt = dyn_cast<ReturnType>(T);
    if (rt)
        return rt->is_returning();
    return true;
}

} // namespace scopes
