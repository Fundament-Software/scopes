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
#include "qualifier.inc"

#include <assert.h>

#include <unordered_set>

namespace scopes {

namespace KeyedSet {
    struct Hash {
        std::size_t operator()(const KeyedType *s) const {
            return s->key.hash();
        }
    };

    struct KeyEqual {
        bool operator()( const KeyedType *lhs, const KeyedType *rhs ) const {
            if (lhs->key != rhs->key) return false;
            return true;
        }
    };
} // namespace TupleSet

static std::unordered_set<const KeyedType *, KeyedSet::Hash, KeyedSet::KeyEqual> keyeds;

//------------------------------------------------------------------------------
// KEYED TYPE
//------------------------------------------------------------------------------

KeyedType::KeyedType(Symbol _key)
    : Qualifier(TK_Keyed), key(_key) {}

void KeyedType::stream_name(StyledStream &ss) const {
    ss << key.name()->data << "=";
}

//------------------------------------------------------------------------------

const Type *keyed_type(Symbol key, const Type *type) {
    if (key == SYM_Unnamed) {
        return strip_qualifier(type, TK_Keyed);
    }
    const KeyedType *result = nullptr;
    KeyedType kt(key);
    auto it = keyeds.find(&kt);
    if (it != keyeds.end()) {
        result = *it;
    } else {
        result = new KeyedType(key);
        keyeds.insert(result);
    }
    return qualify(type, { result });
}

sc_symbol_type_tuple_t key_type(const Type *type) {
    auto kt = try_qualifier<KeyedType>(type);
    if (kt) {
        return { kt->key, strip_qualifier<KeyedType>(type) };
    } else {
        return { SYM_Unnamed, type };
    }
}

} // namespace scopes
