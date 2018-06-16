/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "pointer.hpp"
#include "hash.hpp"

#include <assert.h>

#include <unordered_set>

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

static std::unordered_set<const PointerType *, PointerSet::Hash, PointerSet::KeyEqual> pointers;

//------------------------------------------------------------------------------
// POINTER TYPE
//------------------------------------------------------------------------------

bool PointerType::classof(const Type *T) {
    return T->kind() == TK_Pointer;
}

void PointerType::stream_name(StyledStream &ss) const {
    stream_type_name(ss, element_type);
    if (is_writable() && is_readable()) {
        ss << "*";
    } else if (is_readable()) {
        ss << "(*)";
    } else {
        ss << "*!";
    }
    if (storage_class != SYM_Unnamed) {
        ss << "[" << storage_class.name()->data << "]";
    }
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

SCOPES_RESULT(Any) PointerType::unpack(void *src) const {
    //SCOPES_RESULT_TYPE(Any);
    return wrap_pointer(element_type, src);
}
size_t PointerType::size() {
    return sizeof(uint64_t);
}

bool PointerType::is_readable() const {
    return !(flags & PTF_NonReadable);
}

bool PointerType::is_writable() const {
    return !(flags & PTF_NonWritable);
}

//------------------------------------------------------------------------------

const Type *Pointer(const Type *element_type, uint64_t flags,
    Symbol storage_class) {
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

const Type *NativeROPointer(const Type *element_type) {
    return Pointer(element_type, PTF_NonWritable, SYM_Unnamed);
}

const Type *NativePointer(const Type *element_type) {
    return Pointer(element_type, 0, SYM_Unnamed);
}

const Type *LocalROPointer(const Type *element_type) {
    return Pointer(element_type, PTF_NonWritable, SYM_SPIRV_StorageClassFunction);
}

const Type *LocalPointer(const Type *element_type) {
    return Pointer(element_type, 0, SYM_SPIRV_StorageClassFunction);
}

const Type *StaticPointer(const Type *element_type) {
    return Pointer(element_type, 0, SYM_SPIRV_StorageClassPrivate);
}

//------------------------------------------------------------------------------

uint64_t required_flags_for_storage_class(Symbol storage_class) {
    switch (storage_class.value()) {
    case SYM_Unnamed: return 0;
    case SYM_SPIRV_StorageClassUniformConstant: return PTF_NonWritable;
    case SYM_SPIRV_StorageClassInput: return PTF_NonWritable;
    case SYM_SPIRV_StorageClassUniform: return PTF_NonWritable;
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

//------------------------------------------------------------------------------

} // namespace scopes
