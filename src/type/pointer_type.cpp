/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "pointer_type.hpp"
#include "typename_type.hpp"
#include "../hash.hpp"
#include "../error.hpp"

#include <assert.h>

#include "absl/container/flat_hash_set.h"

namespace scopes {

namespace PointerSet {
    struct Hash {
        std::size_t operator()(const PointerType *s) const {
            size_t h = std::hash<const Type *>{}(s->element_type);
            h = hash2(h, std::hash<uint64_t>{}(s->flags));
            h = hash2(h, s->storage_class.hash());
            return h;
        }
    };

    struct KeyEqual {
        bool operator()( const PointerType *lhs, const PointerType *rhs ) const {
            return
                lhs->element_type == rhs->element_type
                && lhs->flags == rhs->flags
                && lhs->storage_class == rhs->storage_class;
        }
    };
} // namespace PointerSet

static absl::flat_hash_set<const PointerType *, PointerSet::Hash, PointerSet::KeyEqual> pointers;

//------------------------------------------------------------------------------
// POINTER TYPE
//------------------------------------------------------------------------------

void PointerType::stream_name(StyledStream &ss) const {
    ss << "(";
    if (is_writable() && is_readable()) {
        ss << "mutable@ ";
    } else if (is_readable()) {
        ss << "@ ";
    } else if (is_writable()) {
        ss << "writeonly@ ";
    } else {
        ss << "opaque@ ";
    }
    if (storage_class != SYM_Unnamed) {
        ss << "(storage = " << storage_class << ") ";
    }
    stream_type_name(ss, element_type);
    ss << ")";
}

PointerType::PointerType(const Type *_element_type,
    uint64_t _flags, Symbol _storage_class)
    : Type(TK_Pointer),
        element_type(_element_type),
        flags(_flags),
        storage_class(_storage_class) {
}

SCOPES_RESULT(void *) PointerType::getelementptr(void *src, size_t i) const {
    SCOPES_RESULT_TYPE(void *);
    size_t stride = SCOPES_GET_RESULT(size_of(element_type));
    return (void *)((char *)src + stride * i);
}

size_t PointerType::size() {
    return sizeof(uint64_t);
}

bool PointerType::is_readable() const {
    return pointer_flags_is_readable(flags);
}

bool PointerType::is_writable() const {
    return pointer_flags_is_writable(flags);
}

//------------------------------------------------------------------------------

uint64_t required_flags_for_element_type(const Type *element_type) {
    element_type = strip_qualifiers(element_type);
    if (isa<TypenameType>(element_type)
        && !cast<TypenameType>(element_type)->is_complete())
        return 0;
    if (is_opaque(element_type)) {
        return PTF_NonReadable | PTF_NonWritable;
    }
    return 0;
}

uint64_t required_flags_for_storage_class(Symbol storage_class) {
    switch (storage_class.value()) {
    case SYM_Unnamed: return 0;
    case SYM_SPIRV_StorageClassUniformConstant: return PTF_NonWritable;
    case SYM_SPIRV_StorageClassInput: return PTF_NonWritable;
    case SYM_SPIRV_StorageClassUniform: return 0;
    case SYM_SPIRV_StorageClassOutput: return PTF_NonReadable;
    case SYM_SPIRV_StorageClassWorkgroup: return 0;
    case SYM_SPIRV_StorageClassCrossWorkgroup: return 0;
    case SYM_SPIRV_StorageClassPrivate: return 0;
    case SYM_SPIRV_StorageClassFunction: return 0;
    case SYM_SPIRV_StorageClassGeneric: return 0;
    case SYM_SPIRV_StorageClassPushConstant: return PTF_NonWritable;
    case SYM_SPIRV_StorageClassAtomicCounter: return 0;
    case SYM_SPIRV_StorageClassImage: return 0;
    case SYM_SPIRV_StorageClassStorageBuffer: return 0; // ??
    default: break;
    }
    return PTF_NonWritable | PTF_NonReadable;
}

const Type *pointer_type(const Type *element_type, uint64_t flags,
    Symbol storage_class) {
    flags |= required_flags_for_storage_class(storage_class);
    flags |= required_flags_for_element_type(element_type);
    SCOPES_TYPE_KEY(PointerType, key);
    key->element_type = element_type;
    key->flags = flags;
    key->storage_class = storage_class;
    auto it = pointers.find(key);
    if (it != pointers.end())
        return *it;
    auto result = new PointerType(element_type, flags, storage_class);
    pointers.insert(result);
    return result;
}

const Type *native_opaque_pointer_type(const Type *element_type) {
    return pointer_type(element_type, PTF_NonWritable|PTF_NonReadable, SYM_Unnamed);
}

const Type *native_ro_pointer_type(const Type *element_type) {
    return pointer_type(element_type, PTF_NonWritable, SYM_Unnamed);
}

const Type *native_pointer_type(const Type *element_type) {
    return pointer_type(element_type, 0, SYM_Unnamed);
}

const Type *local_ro_pointer_type(const Type *element_type) {
    return pointer_type(element_type, PTF_NonWritable, SYM_SPIRV_StorageClassFunction);
}

const Type *local_pointer_type(const Type *element_type) {
    return pointer_type(element_type, 0, SYM_SPIRV_StorageClassFunction);
}

const Type *static_pointer_type(const Type *element_type) {
    return pointer_type(element_type, 0, SYM_SPIRV_StorageClassPrivate);
}

//------------------------------------------------------------------------------

bool pointer_flags_is_readable(uint64_t flags) {
    return !(flags & PTF_NonReadable);
}

bool pointer_flags_is_writable(uint64_t flags) {
    return !(flags & PTF_NonWritable);
}

bool pointer_storage_classes_compatible(Symbol need, Symbol got) {
    if (need == SYM_Unnamed) return true;
    return (need == got);
}

bool pointer_flags_compatible(uint64_t need, uint64_t got) {
/*
    need     got-> | 0 | nowrite | noread | nowrite-noread |
    0              | Y |    N    |    N   |        N       |
    nowrite        | Y |    Y    |    N   |        N       |
    noread         | Y |    N    |    Y   |        N       |
    nowrite-noread | Y |    Y    |    Y   |        Y       |
*/
    if (!got) return true;
    if (need == (PTF_NonWritable|PTF_NonReadable)) return true;
    return got == need;
}


//------------------------------------------------------------------------------

} // namespace scopes
