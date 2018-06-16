/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "tuple.hpp"
#include "error.hpp"
#include "utils.hpp"
#include "hash.hpp"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------
// TUPLE TYPE
//------------------------------------------------------------------------------

bool TupleType::classof(const Type *T) {
    return T->kind() == TK_Tuple;
}

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
        if (values[i].key != SYM_Unnamed) {
            ss << values[i].key.name()->data << "=";
        }
        if (is_unknown(values[i].value)) {
            stream_type_name(ss, values[i].value.typeref);
        } else {
            ss << "!";
            stream_type_name(ss, values[i].value.type);
        }
    }
    ss << "}";
    if (packed) {
        ss << ">";
    }
}

TupleType::TupleType(const Args &_values, bool _packed, size_t _alignment)
    : StorageType(TK_Tuple), values(_values), packed(_packed) {
    size_t tcount = values.size();
    types.reserve(tcount);
    for (size_t i = 0; i < values.size(); ++i) {
        const Type *T = nullptr;
        if (is_unknown(values[i].value)) {
            T = values[i].value.typeref;
        } else {
            T = values[i].value.type;
        }
        types.push_back(T);
    }

    offsets.resize(types.size());
    size_t sz = 0;
    if (packed) {
        for (size_t i = 0; i < types.size(); ++i) {
            const Type *ET = types[i];
            offsets[i] = sz;
            sz += size_of(ET).assert_ok();
        }
        size = sz;
        align = 1;
    } else {
        size_t al = 1;
        for (size_t i = 0; i < types.size(); ++i) {
            const Type *ET = types[i];
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

SCOPES_RESULT(Any) TupleType::unpack(void *src, size_t i) const {
    SCOPES_RESULT_TYPE(Any);
    return wrap_pointer(SCOPES_GET_RESULT(type_at_index(i)), SCOPES_GET_RESULT(getelementptr(src, i)));
}

SCOPES_RESULT(const Type *) TupleType::type_at_index(size_t i) const {
    SCOPES_RESULT_TYPE(const Type *);
    SCOPES_CHECK_RESULT(verify_range(i, types.size()));
    return types[i];
}

size_t TupleType::field_index(Symbol name) const {
    for (size_t i = 0; i < values.size(); ++i) {
        if (name == values[i].key)
            return i;
    }
    return (size_t)-1;
}

SCOPES_RESULT(Symbol) TupleType::field_name(size_t i) const {
    SCOPES_RESULT_TYPE(Symbol);
    SCOPES_CHECK_RESULT(verify_range(i, values.size()));
    return values[i].key;
}

//------------------------------------------------------------------------------

SCOPES_RESULT(const Type *) MixedTuple(const Args &values,
    bool packed, size_t alignment) {
    SCOPES_RESULT_TYPE(const Type *);
    struct TypeArgs {
        Args args;
        bool packed;
        size_t alignment;

        TypeArgs() {}
        TypeArgs(const Args &_args, bool _packed, size_t _alignment)
            : args(_args), packed(_packed), alignment(_alignment) {}

        bool operator==(const TypeArgs &other) const {
            if (packed != other.packed) return false;
            if (alignment != other.alignment) return false;
            if (args.size() != other.args.size()) return false;
            for (size_t i = 0; i < args.size(); ++i) {
                auto &&a = args[i];
                auto &&b = other.args[i];
                if (a != b)
                    return false;
            }
            return true;
        }

        struct Hash {
            std::size_t operator()(const TypeArgs& s) const {
                std::size_t h = std::hash<bool>{}(s.packed);
                h = hash2(h, std::hash<size_t>{}(s.alignment));
                for (auto &&arg : s.args) {
                    h = hash2(h, arg.hash());
                }
                return h;
            }
        };
    };

    typedef std::unordered_map<TypeArgs, TupleType *, typename TypeArgs::Hash> ArgMap;

    static ArgMap map;

    for (size_t i = 0; i < values.size(); ++i) {
        assert(values[i].value.is_const());
        const Type *T = nullptr;
        if (is_unknown(values[i].value)) {
            T = values[i].value.typeref;
        } else {
            T = values[i].value.type;
        }
        if (is_opaque(T)) {
            StyledString ss;
            ss.out << "can not construct tuple type with field of opaque type "
                << T;
            SCOPES_LOCATION_ERROR(ss.str());
        }
    }

    TypeArgs ta(values, packed, alignment);
    typename ArgMap::iterator it = map.find(ta);
    if (it == map.end()) {
        TupleType *t = new TupleType(values, packed, alignment);
        map.insert({ta, t});
        return t;
    } else {
        return it->second;
    }
}

SCOPES_RESULT(const Type *) Tuple(const ArgTypes &types,
    bool packed, size_t alignment) {
    //SCOPES_RESULT_TYPE(const Type *);
    Args args;
    args.reserve(types.size());
    for (size_t i = 0; i < types.size(); ++i) {
        args.push_back(unknown_of(types[i]));
    }
    return MixedTuple(args, packed, alignment);
}

} // namespace scopes
