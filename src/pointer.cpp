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

#include "pointer.hpp"
#include "typefactory.hpp"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------
// POINTER TYPE
//------------------------------------------------------------------------------

bool PointerType::classof(const Type *T) {
    return T->kind() == TK_Pointer;
}

PointerType::PointerType(const Type *_element_type,
    uint64_t _flags, Symbol _storage_class)
    : Type(TK_Pointer),
        element_type(_element_type),
        flags(_flags),
        storage_class(_storage_class) {
    std::stringstream ss;
    ss << element_type->name()->data;
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
    _name = String::from_stdstring(ss.str());
}

void *PointerType::getelementptr(void *src, size_t i) const {
    size_t stride = size_of(element_type);
    return (void *)((char *)src + stride * i);
}

Any PointerType::unpack(void *src) const {
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
    static TypeFactory<PointerType> pointers;
    assert(element_type->kind() != TK_ReturnLabel);
    return pointers.insert(element_type, flags, storage_class);
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

} // namespace scopes
