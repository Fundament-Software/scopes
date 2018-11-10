/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_POINTER_HPP
#define SCOPES_POINTER_HPP

#include "../type.hpp"
#include "../result.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// POINTER TYPE
//------------------------------------------------------------------------------

enum PointerTypeFlags {
    PTF_NonWritable = (1 << 1),
    PTF_NonReadable = (1 << 2),
    PTF_Unique = (1 << 3),
};

struct PointerType : Type {
    static bool classof(const Type *T);

    PointerType(const Type *_element_type,
        uint64_t _flags, Symbol _storage_class);

    void stream_name(StyledStream &ss) const;
    SCOPES_RESULT(void *) getelementptr(void *src, size_t i) const;

    static size_t size();

    bool is_readable() const;
    bool is_writable() const;
    bool is_unique() const;

    const Type *element_type;
    uint64_t flags;
    Symbol storage_class;
};

bool pointer_flags_is_readable(uint64_t flags);
bool pointer_flags_is_writable(uint64_t flags);

const Type *pointer_type(const Type *element_type, uint64_t flags,
    Symbol storage_class);

const Type *native_ro_pointer_type(const Type *element_type);

const Type *native_pointer_type(const Type *element_type);

const Type *local_ro_pointer_type(const Type *element_type);

const Type *local_pointer_type(const Type *element_type);

const Type *static_pointer_type(const Type *element_type);

uint64_t required_flags_for_storage_class(Symbol storage_class);

bool pointer_storage_classes_compatible(Symbol need, Symbol got);
bool pointer_flags_compatible(uint64_t need, uint64_t got);

} // namespace scopes

#endif // SCOPES_POINTER_HPP