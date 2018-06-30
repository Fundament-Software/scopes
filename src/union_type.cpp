/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "union_type.hpp"
#include "error.hpp"
#include "utils.hpp"
#include "tuple_type.hpp"
#include "hash.hpp"

#include <unordered_set>

namespace scopes {

namespace UnionSet {
    struct Hash {
        std::size_t operator()(const UnionType *s) const {
            std::size_t h = 0;
            for (auto &&arg : s->values) {
                h = hash2(h, arg.hash());
            }
            return h;
        }
    };

    struct KeyEqual {
        bool operator()( const UnionType *lhs, const UnionType *rhs ) const {
            if (lhs->values.size() != rhs->values.size()) return false;
            for (size_t i = 0; i < lhs->values.size(); ++i) {
                auto &&a = lhs->values[i];
                auto &&b = rhs->values[i];
                if (a != b)
                    return false;
            }
            return true;
        }
    };
} // namespace UnionSet

static std::unordered_set<const UnionType *, UnionSet::Hash, UnionSet::KeyEqual> unions;

//------------------------------------------------------------------------------
// UNION TYPE
//------------------------------------------------------------------------------

bool UnionType::classof(const Type *T) {
    return T->kind() == TK_Union;
}


void UnionType::stream_name(StyledStream &ss) const {
    ss << "{";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            ss << " | ";
        }
        if (values[i].key != SYM_Unnamed) {
            ss << values[i].key.name()->data << "=";
        }
        stream_type_name(ss, values[i].type);
    }
    ss << "}";
}

UnionType::UnionType(const KeyedTypes &_values)
    : StorageType(TK_Union), values(_values) {

    size_t sz = 0;
    size_t al = 1;
    largest_field = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        const Type *ET = values[i].type;
        auto newsz = size_of(ET).assert_ok();
        if (newsz > sz) {
            largest_field = i;
            sz = newsz;
        }
        al = std::max(al, align_of(ET).assert_ok());
    }
    size = scopes::align(sz, al);
    align = al;
    this->tuple_type = scopes::tuple_type({values[largest_field].type}).assert_ok();
}

SCOPES_RESULT(const Type *) UnionType::type_at_index(size_t i) const {
    SCOPES_RESULT_TYPE(const Type *);
    SCOPES_CHECK_RESULT(verify_range(i, values.size()));
    return values[i].type;
}

size_t UnionType::field_index(Symbol name) const {
    for (size_t i = 0; i < values.size(); ++i) {
        if (name == values[i].key)
            return i;
    }
    return (size_t)-1;
}

SCOPES_RESULT(Symbol) UnionType::field_name(size_t i) const {
    SCOPES_RESULT_TYPE(Symbol);
    SCOPES_CHECK_RESULT(verify_range(i, values.size()));
    return values[i].key;
}

//------------------------------------------------------------------------------

SCOPES_RESULT(const Type *) keyed_union_type(const KeyedTypes &values) {
    SCOPES_RESULT_TYPE(const Type *);
    UnionType key(values);
    auto it = unions.find(&key);
    if (it != unions.end())
        return *it;
    for (size_t i = 0; i < values.size(); ++i) {
        const Type *T = values[i].type;
        if (is_opaque(T)) {
            StyledString ss;
            ss.out << "can not construct union type with field of opaque type " << T;
            SCOPES_LOCATION_ERROR(ss.str());
        }
    }
    auto result = new UnionType(values);
    unions.insert(result);
    return result;
}

SCOPES_RESULT(const Type *) union_type(const ArgTypes &types) {
    KeyedTypes args;
    args.reserve(types.size());
    for (size_t i = 0; i < types.size(); ++i) {
        args.push_back(types[i]);
    }
    return keyed_union_type(args);
}

} // namespace scopes
