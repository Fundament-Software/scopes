/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "refer_qualifier.hpp"
#include "../type/pointer_type.hpp"
#include "../hash.hpp"
#include "../qualifier.inc"

#include <assert.h>

#include <unordered_set>

namespace scopes {

namespace ReferSet {
    struct Hash {
        std::size_t operator()(const ReferQualifier *s) const {
            size_t h = std::hash<uint64_t>{}(s->flags);
            h = hash2(h, s->storage_class.hash());
            return h;
        }
    };

    struct KeyEqual {
        bool operator()( const ReferQualifier *lhs, const ReferQualifier *rhs ) const {
            return
                lhs->flags == rhs->flags
                && lhs->storage_class == rhs->storage_class;
        }
    };
} // namespace ReferSet

static std::unordered_set<const ReferQualifier *, ReferSet::Hash, ReferSet::KeyEqual> refers;

//------------------------------------------------------------------------------
// REFER QUALIFIER
//------------------------------------------------------------------------------

void ReferQualifier::stream_prefix(StyledStream &ss) const {
    if (pointer_flags_is_readable(flags) && pointer_flags_is_writable(flags)) {
        ss << "&";
    } else if (pointer_flags_is_readable(flags)) {
        ss << "(&)";
    } else {
        ss << "((&))";
    }
    if (storage_class != SYM_Unnamed) {
        ss << "[" << storage_class.name()->data << "]";
    }
}

void ReferQualifier::stream_postfix(StyledStream &ss) const {
}

const Type *ReferQualifier::get_pointer_type(const Type *ET) const {
    return pointer_type(ET, flags, storage_class);
}

ReferQualifier::ReferQualifier(uint64_t _flags, Symbol _storage_class)
    : Qualifier((QualifierKind)Kind),
        flags(_flags),
        storage_class(_storage_class) {
}

//------------------------------------------------------------------------------

const Type *refer_type(const Type *type, uint64_t flags,
    Symbol storage_class) {
    const ReferQualifier *result = nullptr;
    ReferQualifier key(flags, storage_class);
    auto it = refers.find(&key);
    if (it != refers.end()) {
        result = *it;
    } else {
        result = new ReferQualifier(flags, storage_class);
        refers.insert(result);
    }
    return qualify(type, { result });
}

uint64_t refer_flags(const Type *T) {
    auto q = try_qualifier<ReferQualifier>(T);
    if (q) { return q->flags; }
    return 0;
}

Symbol refer_storage_class(const Type *T) {
    auto q = try_qualifier<ReferQualifier>(T);
    if (q) { return q->storage_class; }
    return SYM_Unnamed;
}

//------------------------------------------------------------------------------

} // namespace scopes
