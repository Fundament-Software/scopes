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
#include "../platform_abi.hpp"
#include "array_type.hpp"
#include "vector_type.hpp"
#include "typename_type.hpp"

#include <assert.h>

#include <unordered_set>
#include <algorithm>

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

std::vector<Symbol> TupleType::find_closest_field_match(Symbol name) const {
    const String *s = name.name();
    std::unordered_set<Symbol, Symbol::Hash> done;
    std::vector<Symbol> best_syms;
    size_t best_dist = (size_t)-1;
    for (int i = 0; i < values.size(); ++i) {
        auto sym = type_key(values[i])._0;
        if (done.count(sym))
            continue;
        size_t dist = distance(s, sym.name());
        if (dist == best_dist) {
            best_syms.push_back(sym);
        } else if (dist < best_dist) {
            best_dist = dist;
            best_syms = { sym };
        }
        done.insert(sym);
    }
    std::sort(best_syms.begin(), best_syms.end());
    return best_syms;
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

/*  we must ensure that the storage type of unions (the payload)
    * translates to the correct C ABI type
    * has no interior padding that corrupts data because it's ignored by load/store
    * has the largest alignment encountered within the union
*/

static const Type *build_union_alignment_field(size_t sz, size_t al, ABIClass *classes, size_t classes_sz) {
    // currently fixing only a few float-related cases
    if (al == 4) {
        if (classes_sz == 1) {
            if (classes[0] == ABI_CLASS_SSESF)
                return TYPE_F32;
            else if (classes[0] == ABI_CLASS_SSE)
                return array_type(TYPE_F32, 2).assert_ok();
        } else if (classes_sz == 2) {
            if ((classes[0] == ABI_CLASS_INTEGER) && (classes[1] == ABI_CLASS_SSESF))
                return tuple_type({
                    array_type(TYPE_I32, 2).assert_ok(),
                    TYPE_F32}).assert_ok();
            else if ((classes[0] == ABI_CLASS_INTEGER) && (classes[1] == ABI_CLASS_SSE))
                return tuple_type({
                    array_type(TYPE_I32, 2).assert_ok(),
                    array_type(TYPE_F32, 2).assert_ok()}).assert_ok();
            else if ((classes[0] == ABI_CLASS_SSE) && (classes[1] == ABI_CLASS_INTEGERSI)) {
                assert(sz > 8);
                return tuple_type({
                    array_type(TYPE_F32, 2).assert_ok(),
                    array_type(TYPE_I8, sz - 8).assert_ok()}).assert_ok();
            } else if ((classes[0] == ABI_CLASS_SSE) && (classes[1] == ABI_CLASS_INTEGER))
                return tuple_type({
                    array_type(TYPE_F32, 2).assert_ok(),
                    array_type(TYPE_I32, 2).assert_ok()}).assert_ok();
            else if ((classes[0] == ABI_CLASS_SSE) && (classes[1] == ABI_CLASS_SSESF))
                return array_type(TYPE_F32, 3).assert_ok();
            else if ((classes[0] == ABI_CLASS_SSE) && (classes[1] == ABI_CLASS_SSE))
                return array_type(TYPE_F32, 4).assert_ok();
        }
    }
    return vector_type(TYPE_I8, al).assert_ok();
}

SCOPES_RESULT(const Type *) union_storage_type(const Types &types,
    bool packed, size_t alignment) {
    SCOPES_RESULT_TYPE(const Type *);
    ABIClass classes[MAX_ABI_CLASSES];
    for (size_t i = 0; i < MAX_ABI_CLASSES; i++)
        classes[i] = ABI_CLASS_NO_CLASS;
    size_t classes_sz = 0;
    size_t sz = 0;
    size_t al = 0;
    // find largest size, largest alignment and ABI classes
    StyledStream ss;
    for (auto ET : types) {
        auto newsz = SCOPES_GET_RESULT(size_of(ET));
        sz = std::max(sz, newsz);
        auto newal = SCOPES_GET_RESULT(align_of(ET));
        if (newal > al) {
            al = newal;
        }
        ABIClass subclasses[MAX_ABI_CLASSES];
        size_t clssz = abi_classify(ET, subclasses);
        for (size_t i = 0; i < clssz; ++i) {
            classes[i] = merge_abi_classes(classes[i], subclasses[i]);
        }
        if (clssz > classes_sz) {
            classes_sz = clssz;
        }
    }
    assert(al != 0);
    auto align_field = build_union_alignment_field(sz, al, classes, classes_sz);
    assert(align_of(align_field).assert_ok() == al);
    Types fields = { align_field };
    auto fieldsz = size_of(align_field).assert_ok();
    if (fieldsz < sz) {
        // pad out
        fields.push_back(array_type(TYPE_I8, sz - fieldsz).assert_ok());
    }
    return SCOPES_GET_RESULT(tuple_type(fields, packed, alignment));
}

} // namespace scopes
