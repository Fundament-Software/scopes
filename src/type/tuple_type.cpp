/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "tuple_type.hpp"
#include "../error.hpp"
#include "../utils.hpp"
#include "../hash.hpp"
#include "../qualifier/key_qualifier.hpp"
#include "array_type.hpp"
#include "typename_type.hpp"

#include <assert.h>

#include <unordered_set>

namespace scopes {

namespace TupleSet {
    struct Hash {
        std::size_t operator()(const TupleType *s) const {
            std::size_t h = std::hash<bool>{}(s->packed);
            h = hash2(h, std::hash<size_t>{}(s->align));
            for (auto &&arg : s->values) {
                h = hash2(h, std::hash<const Type *>{}(arg));
            }
            return h;
        }
    };

    struct KeyEqual {
        bool operator()( const TupleType *lhs, const TupleType *rhs ) const {
            if (lhs->packed != rhs->packed) return false;
            if (lhs->align != rhs->align) return false;
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
} // namespace TupleSet

static std::unordered_set<const TupleType *, TupleSet::Hash, TupleSet::KeyEqual> tuples;

//------------------------------------------------------------------------------
// TUPLE TYPE
//------------------------------------------------------------------------------

void TupleType::stream_name(StyledStream &ss) const {
    if (explicit_alignment) {
        ss << "[align:" << align << "]";
    }
    if (packed) {
        ss << "<";
    }
    ss << "{";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            ss << " ";
        }
        stream_type_name(ss, values[i]);
    }
    ss << "}";
    if (packed) {
        ss << ">";
    }
}

TupleType::TupleType(const Types &_values, bool _packed, size_t _alignment)
    : TupleLikeType(TK_Tuple, _values), packed(_packed) {
    offsets.resize(values.size());
    size_t sz = 0;
    if (packed) {
        for (size_t i = 0; i < values.size(); ++i) {
            const Type *ET = values[i];
            offsets[i] = sz;
            sz += size_of(ET).assert_ok();
        }
        size = sz;
        align = 1;
    } else {
        size_t al = 1;
        for (size_t i = 0; i < values.size(); ++i) {
            const Type *ET = values[i];
            size_t etal = align_of(ET).assert_ok();
            sz = scopes::align(sz, etal);
            offsets[i] = sz;
            al = std::max(al, etal);
            sz += size_of(ET).assert_ok();
        }
        size = scopes::align(sz, al);
        align = al;
    }
    if (_alignment) {
        explicit_alignment = true;
        align = _alignment;
        size = scopes::align(sz, align);
    } else {
        explicit_alignment = false;
    }
}

SCOPES_RESULT(void *) TupleType::getelementptr(void *src, size_t i) const {
    SCOPES_RESULT_TYPE(void *);
    SCOPES_CHECK_RESULT(verify_range(i, offsets.size()));
    return (void *)((char *)src + offsets[i]);
}

SCOPES_RESULT(const Type *) TupleType::type_at_index(size_t i) const {
    SCOPES_RESULT_TYPE(const Type *);
    SCOPES_CHECK_RESULT(verify_range(i, values.size()));
    return values[i];
}

const Type *TupleType::type_at_index_or_nothing(size_t i) const {
    if (i < values.size())
        return values[i];
    return TYPE_Nothing;
}

size_t TupleType::field_index(Symbol name) const {
    for (size_t i = 0; i < values.size(); ++i) {
        if (name == type_key(values[i])._0)
            return i;
    }
    return (size_t)-1;
}

SCOPES_RESULT(Symbol) TupleType::field_name(size_t i) const {
    SCOPES_RESULT_TYPE(Symbol);
    SCOPES_CHECK_RESULT(verify_range(i, values.size()));
    return type_key(values[i])._0;
}

//------------------------------------------------------------------------------

SCOPES_RESULT(const Type *) tuple_type(const Types &values,
    bool packed, size_t alignment) {
    SCOPES_RESULT_TYPE(const Type *);
    for (size_t i = 0; i < values.size(); ++i) {
        const Type *T = values[i];
        if (is_opaque(T)) {
            SCOPES_ERROR(OpaqueType, T);
        }
    }
    TupleType key(values, packed, alignment);
    auto it = tuples.find(&key);
    if (it != tuples.end())
        return *it;
    auto result = new TupleType(values, packed, alignment);
    tuples.insert(result);
    return result;
}

static SCOPES_RESULT(void) find_most_aligned(const Type *ET, const Type *&most_aligned_field, size_t &al) {
    SCOPES_RESULT_TYPE(void);
    auto newal = SCOPES_GET_RESULT(align_of(ET));
    if (newal > al) {
        if (!is_opaque(ET)) {
            StyledStream ss;
            auto ST = SCOPES_GET_RESULT(storage_type(ET));
            switch (ST->kind()) {
            case TK_Tuple: {
                auto TT = cast<TupleType>(ST);
                for (auto ET : TT->values) {
                    SCOPES_CHECK_RESULT(find_most_aligned(ET, most_aligned_field, al));
                }
                return {};
            } break;
            case TK_Array: {
                auto AT = cast<ArrayType>(ST);
                SCOPES_CHECK_RESULT(find_most_aligned(AT->element_type, most_aligned_field, al));
                return {};
            } break;
            default: break;
            }
        }
        most_aligned_field = ET;
        al = newal;
    }
    return {};
}

SCOPES_RESULT(const Type *) union_storage_type(const Types &types,
    bool packed, size_t alignment) {
    SCOPES_RESULT_TYPE(const Type *);
    size_t sz = 0;
    // find largest size
    for (auto ET : types) {
        auto newsz = SCOPES_GET_RESULT(size_of(ET));
        sz = std::max(sz, newsz);
    }
    const Type *most_aligned_field = nullptr;
    // recursively find field with largest alignment
    size_t al = 0;
    for (auto ET : types) {
        SCOPES_CHECK_RESULT(find_most_aligned(ET, most_aligned_field, al));
    }
    assert(most_aligned_field);
    Types fields = { most_aligned_field };
    auto fieldsz = size_of(most_aligned_field).assert_ok();
    if (fieldsz < sz) {
        // pad out
        fields.push_back(array_type(TYPE_I8, sz - fieldsz).assert_ok());
    }
    return tuple_type(fields, packed, alignment);
}

} // namespace scopes
