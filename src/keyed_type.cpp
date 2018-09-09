/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "keyed_type.hpp"
#include "error.hpp"
#include "utils.hpp"
#include "hash.hpp"
#include "dyn_cast.inc"

#include <assert.h>

#include <unordered_set>

namespace scopes {

namespace KeyedSet {
    struct Hash {
        std::size_t operator()(const KeyedType *s) const {
            return hash2(s->key.hash(), std::hash<const Type *>{}(s->type));
        }
    };

    struct KeyEqual {
        bool operator()( const KeyedType *lhs, const KeyedType *rhs ) const {
            if (lhs->key != rhs->key) return false;
            if (lhs->type != rhs->type) return false;
            return true;
        }
    };
} // namespace TupleSet

static std::unordered_set<const KeyedType *, KeyedSet::Hash, KeyedSet::KeyEqual> keyeds;

//------------------------------------------------------------------------------
// KEYED TYPE
//------------------------------------------------------------------------------

KeyedType::KeyedType(Symbol _key, const Type *_type)
    : Type(TK_Keyed), key(_key), type(_type) {}

void KeyedType::stream_name(StyledStream &ss) const {
    ss << key.name()->data << "=";
    stream_type_name(ss, type);
}

//------------------------------------------------------------------------------

const Type *keyed_type(Symbol key, const Type *type) {
    if (isa<KeyedType>(type))
        type = cast<KeyedType>(type)->type;
    if (key == SYM_Unnamed)
        return type;
    KeyedType kt(key, type);
    auto it = keyeds.find(&kt);
    if (it != keyeds.end())
        return *it;
    auto result = new KeyedType(key, type);
    keyeds.insert(result);
    return result;
}

sc_symbol_type_tuple_t key_type(const Type *type) {
    auto kt = dyn_cast<KeyedType>(type);
    if (kt) {
        return { kt->key, kt->type };
    } else {
        return { SYM_Unnamed, type };
    }
}

} // namespace scopes
