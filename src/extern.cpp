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
