/*
Scopes Compiler
Copyright (c) 2016, 2017, 2018 Leonard Ritter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "tuple.hpp"
#include "error.hpp"
#include "utils.hpp"

#include "cityhash/city.h"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------
// TUPLE TYPE
//------------------------------------------------------------------------------

bool TupleType::classof(const Type *T) {
    return T->kind() == TK_Tuple;
}

TupleType::TupleType(const Args &_values, bool _packed, size_t _alignment)
    : StorageType(TK_Tuple), values(_values), packed(_packed) {
    StyledString ss = StyledString::plain();
    if (_alignment) {
        ss.out << "[align:" << _alignment << "]";
    }
    if (packed) {
        ss.out << "<";
    }
    ss.out << "{";
    size_t tcount = values.size();
    types.reserve(tcount);
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            ss.out << " ";
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
            ss.out << "can not construct tuple type with field of opaque type "
                << T;
            location_error(ss.str());
        }
        types.push_back(T);
    }

    ss.out << "}";
    if (packed) {
        ss.out << ">";
    }
    _name = ss.str();

    offsets.resize(types.size());
    size_t sz = 0;
    if (packed) {
        for (size_t i = 0; i < types.size(); ++i) {
            const Type *ET = types[i];
            offsets[i] = sz;
            sz += size_of(ET);
        }
        size = sz;
        align = 1;
    } else {
        size_t al = 1;
        for (size_t i = 0; i < types.size(); ++i) {
            const Type *ET = types[i];
            size_t etal = align_of(ET);
            sz = scopes::align(sz, etal);
            offsets[i] = sz;
            al = std::max(al, etal);
            sz += size_of(ET);
        }
        size = scopes::align(sz, al);
        align = al;
    }
    if (_alignment) {
        align = _alignment;
        size = scopes::align(sz, align);
    }
}

void *TupleType::getelementptr(void *src, size_t i) const {
    verify_range(i, offsets.size());
    return (void *)((char *)src + offsets[i]);
}

Any TupleType::unpack(void *src, size_t i) const {
    return wrap_pointer(type_at_index(i), getelementptr(src, i));
}

const Type *TupleType::type_at_index(size_t i) const {
    verify_range(i, types.size());
    return types[i];
}

size_t TupleType::field_index(Symbol name) const {
    for (size_t i = 0; i < values.size(); ++i) {
        if (name == values[i].key)
            return i;
    }
    return (size_t)-1;
}

Symbol TupleType::field_name(size_t i) const {
    verify_range(i, values.size());
    return values[i].key;
}

//------------------------------------------------------------------------------

const Type *MixedTuple(const Args &values,
    bool packed, size_t alignment) {
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
                h = HashLen16(h, std::hash<size_t>{}(s.alignment));
                for (auto &&arg : s.args) {
                    h = HashLen16(h, arg.hash());
                }
                return h;
            }
        };
    };

    typedef std::unordered_map<TypeArgs, TupleType *, typename TypeArgs::Hash> ArgMap;

    static ArgMap map;

#ifdef SCOPES_DEBUG
    for (size_t i = 0; i < values.size(); ++i) {
        assert(values[i].value.is_const());
    }
#endif
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

const Type *Tuple(const ArgTypes &types,
    bool packed, size_t alignment) {
    Args args;
    args.reserve(types.size());
    for (size_t i = 0; i < types.size(); ++i) {
        args.push_back(unknown_of(types[i]));
    }
    return MixedTuple(args, packed, alignment);
}

} // namespace scopes
