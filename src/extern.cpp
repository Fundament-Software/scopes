/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "extern.hpp"
#include "pointer.hpp"
#include "typefactory.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// EXTERN TYPE
//------------------------------------------------------------------------------

bool ExternType::classof(const Type *T) {
    return T->kind() == TK_Extern;
}

ExternType::ExternType(const Type *_type,
    size_t _flags, Symbol _storage_class, int _location, int _binding) :
    Type(TK_Extern),
    type(_type),
    flags(_flags),
    storage_class(_storage_class),
    location(_location),
    binding(_binding) {
    std::stringstream ss;
    ss << "<extern " <<  _type->name()->data;
    if (storage_class != SYM_Unnamed)
        ss << " storage=" << storage_class.name()->data;
    if (location >= 0)
        ss << " location=" << location;
    if (binding >= 0)
        ss << " binding=" << binding;
    ss << ">";
    _name = String::from_stdstring(ss.str());
    if ((_storage_class == SYM_SPIRV_StorageClassUniform)
        && !(flags & EF_BufferBlock)) {
        flags |= EF_Block;
    }
    size_t ptrflags = 0;
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
    static TypeFactory<ExternType> externs;
    return externs.insert(type, flags, storage_class, location, binding);
}


} // namespace scopes
