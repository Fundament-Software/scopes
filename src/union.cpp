/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "union.hpp"
#include "error.hpp"
#include "utils.hpp"
#include "tuple.hpp"

#include "cityhash/city.h"

namespace scopes {

//------------------------------------------------------------------------------
// UNION TYPE
//------------------------------------------------------------------------------

bool UnionType::classof(const Type *T) {
    return T->kind() == TK_Union;
}

UnionType::UnionType(const Args &_values)
    : StorageType(TK_Union), values(_values) {
    StyledString ss = StyledString::plain();
    ss.out << "{";
    size_t tcount = values.size();
    types.reserve(tcount);
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            ss.out << " | ";
        }
        if (values[i].key != SYM_Unnamed) {
            ss.out << values[i].key.name()->data << "=";
        }
        const Type *T = nullptr;
        if (is_unknown(values[i].value)) {
            ss.out << values[i].value.typeref->name()->data;
            T = values[i].value.typeref;
        } else {
            ss.out << "!" << values[i].value.type;
            T = values[i].value.type;
        }
        if (is_opaque(T)) {
            StyledString ss;
            ss.out << "can not construct union type with field of opaque type "
                << T;
            location_error(ss.str());
        }
        types.push_back(T);
    }
    ss.out << "}";
    _name = ss.str();

    size_t sz = 0;
    size_t al = 1;
    largest_field = 0;
    for (size_t i = 0; i < types.size(); ++i) {
        const Type *ET = types[i];
        auto newsz = size_of(ET);
        if (newsz > sz) {
            largest_field = i;
            sz = newsz;
        }
        al = std::max(al, align_of(ET));
    }
    size = scopes::align(sz, al);
    align = al;
    tuple_type = Tuple({types[largest_field]});
}

Any UnionType::unpack(void *src, size_t i) const {
    return wrap_pointer(type_at_index(i), src);
}

const Type *UnionType::type_at_index(size_t i) const {
    verify_range(i, types.size());
    return types[i];
}

size_t UnionType::field_index(Symbol name) const {
    for (size_t i = 0; i < values.size(); ++i) {
        if (name == values[i].key)
            return i;
    }
    return (size_t)-1;
}

Symbol UnionType::field_name(size_t i) const {
    verify_range(i, values.size());
    return values[i].key;
}

//------------------------------------------------------------------------------

const Type *MixedUnion(const Args &values) {
    struct TypeArgs {
        Args args;

        TypeArgs() {}
        TypeArgs(const Args &_args)
            : args(_args) {}

        bool operator==(const TypeArgs &other) const {
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
                std::size_t h = 0;
                for (auto &&arg : s.args) {
                    h = HashLen16(h, arg.hash());
                }
                return h;
            }
        };
    };

    typedef std::unordered_map<TypeArgs, UnionType *, typename TypeArgs::Hash> ArgMap;

    static ArgMap map;

    TypeArgs ta(values);
    typename ArgMap::iterator it = map.find(ta);
    if (it == map.end()) {
        UnionType *t = new UnionType(values);
        map.insert({ta, t});
        return t;
    } else {
        return it->second;
    }
}

const Type *Union(const ArgTypes &types) {
    Args args;
    args.reserve(types.size());
    for (size_t i = 0; i < types.size(); ++i) {
        args.push_back(unknown_of(types[i]));
    }
    return MixedUnion(args);
}

} // namespace scopes
