/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "key_qualifier.hpp"
#include "../error.hpp"
#include "../utils.hpp"
#include "../hash.hpp"
#include "../dyn_cast.inc"
#include "../qualifier.inc"

#include <assert.h>

#include "absl/container/flat_hash_set.h"

namespace scopes {

namespace KeyedSet {
    struct Hash {
        std::size_t operator()(const KeyQualifier *s) const {
            return s->key.hash();
        }
    };

    struct KeyEqual {
        bool operator()( const KeyQualifier *lhs, const KeyQualifier *rhs ) const {
            if (lhs->key != rhs->key) return false;
            return true;
        }
    };
} // namespace TupleSet

static absl::flat_hash_set<const KeyQualifier *, KeyedSet::Hash, KeyedSet::KeyEqual> keyeds;

//------------------------------------------------------------------------------
// KEY QUALIFIER
//------------------------------------------------------------------------------

KeyQualifier::KeyQualifier(Symbol _key)
    : Qualifier((QualifierKind)Kind), key(_key) {}

void KeyQualifier::stream_prefix(StyledStream &ss) const {
    ss << "(";
    ss << key.name()->data;
    ss << " = ";
}

void KeyQualifier::stream_postfix(StyledStream &ss) const {
    ss << ")";
}

//------------------------------------------------------------------------------

const Type *key_type(Symbol key, const Type *type) {
    if (key == SYM_Unnamed) {
        return strip_qualifier<KeyQualifier>(type);
    }
    const KeyQualifier *result = nullptr;
    KeyQualifier kt(key);
    auto it = keyeds.find(&kt);
    if (it != keyeds.end()) {
        result = *it;
    } else {
        result = new KeyQualifier(key);
        keyeds.insert(result);
    }
    return qualify(type, { result });
}

SymbolTypeTuple type_key(const Type *type) {
    assert(type);
    auto kt = try_qualifier<KeyQualifier>(type);
    if (kt) {
        return { kt->key, strip_qualifier<KeyQualifier>(type) };
    } else {
        return { SYM_Unnamed, type };
    }
}

} // namespace scopes
