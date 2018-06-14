/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_EXTERN_HPP
#define SCOPES_EXTERN_HPP

#include "type.hpp"
#include "symbol.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// EXTERN TYPE
//------------------------------------------------------------------------------

enum ExternFlags {
    // if storage class is 'Uniform, the value is a SSBO
    EF_BufferBlock = (1 << 0),
    EF_NonWritable = (1 << 1),
    EF_NonReadable = (1 << 2),
    EF_Volatile = (1 << 3),
    EF_Coherent = (1 << 4),
    EF_Restrict = (1 << 5),
    // if storage class is 'Uniform, the value is a UBO
    EF_Block = (1 << 6),
};

struct ExternType : Type {
    static bool classof(const Type *T);

    void stream_name(StyledStream &ss) const;

    ExternType(const Type *_type,
        size_t _flags, Symbol _storage_class, int _location, int _binding);

    const Type *type;
    size_t flags;
    Symbol storage_class;
    int location;
    int binding;
    const Type *pointer_type;
};

const Type *Extern(const Type *type,
    size_t flags = 0,
    Symbol storage_class = SYM_Unnamed,
    int location = -1,
    int binding = -1);

} // namespace scopes

#endif // SCOPES_EXTERN_HPP