/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "arguments_type.hpp"
#include "tuple_type.hpp"
#include "hash.hpp"
#include "dyn_cast.inc"

#include <assert.h>

#include <unordered_set>

namespace scopes {

namespace ArgumentsSet {
    struct Hash {
        std::size_t operator()(const ArgumentsType *s) const {
            std::size_t h = 0;
            for (auto &&arg : s->values) {
                h = hash2(h, arg.hash());
            }
            return h;
        }
    };

    struct KeyEqual {
        bool operator()( const ArgumentsType *lhs, const ArgumentsType *rhs ) const {
            return (lhs->values == rhs->values);
        }
    };
} // namespace ArgumentsSet

static std::unordered_set<const ArgumentsType *, ArgumentsSet::Hash, ArgumentsSet::KeyEqual> arguments;

//------------------------------------------------------------------------------
// ARGUMENTS TYPE
//------------------------------------------------------------------------------

const Type *ArgumentsType::type_at_index(size_t i) const {
    if (i < values.size())
        return values[i].type;
    return TYPE_Nothing;
}

void ArgumentsType::stream_name(StyledStream &ss) const {
    ss << "λ(";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            ss << " ";
        }
        if (values[i].key != SYM_Unnamed) {
            ss << values[i].key.name()->data << "=";
        }
        stream_type_name(ss, values[i].type);
    }
    ss << ")";
}

ArgumentsType::ArgumentsType(const KeyedTypes &_values)
    : Type(TK_Arguments) {
    int count = (int)_values.size();
    for (int i = 0; i < count; ++i) {
        auto &&value = _values[i];
        assert(!isa<ArgumentsType>(value.type));
        values.push_back(value);
    }
}

//------------------------------------------------------------------------------

const Type *keyed_arguments_type(const KeyedTypes &values) {
    if (values.size() == 0)
        return TYPE_Void;
    else if ((values.size() == 1)
            && (values[0].key == SYM_Unnamed))
        return values[0].type;
    ArgumentsType key(values);
    auto it = arguments.find(&key);
    if (it != arguments.end())
        return *it;
    auto result = new ArgumentsType(values);
    arguments.insert(result);
    return result;
}

const Type *arguments_type(const ArgTypes &values) {
    KeyedTypes types;
    for (auto &&val : values) {
        types.push_back(val);
    }
    return keyed_arguments_type(types);
}

} // namespace scopes
