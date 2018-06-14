/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "extern.hpp"
#include "pointer.hpp"
#include "hash.hpp"

#include <unordered_set>

namespace scopes {

namespace ExternSet {
    struct Hash {
        std::size_t operator()(const ExternType *s) const {
            size_t h = std::hash<const Type *>{}(s->type);
            h = hash2(h, std::hash<size_t>{}(s->flags));
            h = hash2(h, s->storage_class.hash());
            h = hash2(h, std::hash<int>{}(s->location));
            h = hash2(h, std::hash<int>{}(s->binding));
            return h;
        }
    };

    struct KeyEqual {
        bool operator()( const ExternType *lhs, const ExternType *rhs ) const {
            return
                lhs->type == rhs->type
                && lhs->flags == rhs->flags
                && lhs->storage_class == rhs->storage_class
                && lhs->location == rhs->location
                && lhs->binding == rhs->binding;
        }
    };
} // namespace ExternSet

static std::unordered_set<const ExternType *, ExternSet::Hash, ExternSet::KeyEqual> externs;

//------------------------------------------------------------------------------
// EXTERN TYPE
//------------------------------------------------------------------------------

bool ExternType::classof(const Type *T) {
    return T->kind() == TK_Extern;
}

void ExternType::stream_name(StyledStream &ss) const {
    ss << "<extern ";
    stream_type_name(ss, type);
    if (storage_class != SYM_Unnamed)
        ss << " storage=" << storage_class.name()->data;
    if (location >= 0)
        ss << " location=" << location;
    if (binding >= 0)
        ss << " binding=" << binding;
    ss << ">";
}

ExternType::ExternType(const Type *_type,
    size_t _flags, Symbol _storage_class, int _location, int _binding) :
    Type(TK_Extern),
    type(_type),
    flags(_flags),
    storage_class(_storage_class),
    location(_location),
    binding(_binding) {
    if ((_storage_class == SYM_SPIRV_StorageClassUniform)
        && !(flags & EF_BufferBlock)) {
        flags |= EF_Block;
    }
    size_t ptrflags = required_flags_for_storage_class(storage_class);
    if (flags & EF_NonWritable)
        ptrflags |= PTF_NonWritable;
    else if (flags & EF_NonReadable)
        ptrflags |= PTF_NonReadable;
    pointer_type = Pointer(type, ptrflags, storage_class);
}

//------------------------------------------------------------------------------

const Type *Extern(const Type *type,
    size_t flags,
    Symbol storage_class,
    int location,
    int binding) {
    SCOPES_TYPE_KEY(ExternType, key);
    key->type = type;
    key->flags = flags;
    key->storage_class = storage_class;
    key->location = location;
    key->binding = binding;
    auto it = externs.find(key);
    if (it != externs.end())
        return *it;
    auto result = new ExternType(type, flags, storage_class, location, binding);
    externs.insert(result);
    return result;
}


} // namespace scopes
