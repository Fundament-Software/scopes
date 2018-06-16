/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_POINTER_HPP
#define SCOPES_POINTER_HPP

#include "type.hpp"
#include "result.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// POINTER TYPE
//------------------------------------------------------------------------------

enum PointerTypeFlags {
    PTF_NonWritable = (1 << 1),
    PTF_NonReadable = (1 << 2),
};

struct PointerType : Type {
    static bool classof(const Type *T);

    PointerType(const Type *_element_type,
        uint64_t _flags, Symbol _storage_class);

    void stream_name(StyledStream &ss) const;
    SCOPES_RESULT(void *) getelementptr(void *src, size_t i) const;

    SCOPES_RESULT(Any) unpack(void *src) const;
    static size_t size();

    bool is_readable() const;

    bool is_writable() const;

    const Type *element_type;
    uint64_t flags;
    Symbol storage_class;
};

const Type *Pointer(const Type *element_type, uint64_t flags,
    Symbol storage_class);

const Type *NativeROPointer(const Type *element_type);

const Type *NativePointer(const Type *element_type);

const Type *LocalROPointer(const Type *element_type);

const Type *LocalPointer(const Type *element_type);

const Type *StaticPointer(const Type *element_type);

uint64_t required_flags_for_storage_class(Symbol storage_class);

} // namespace scopes

#endif // SCOPES_POINTER_HPP