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

#include "sized_storage.hpp"

namespace scopes {

StorageType::StorageType(TypeKind kind) : Type(kind) {}

//------------------------------------------------------------------------------

SizedStorageType::SizedStorageType(TypeKind kind, const Type *_element_type, size_t _count)
    : StorageType(kind), element_type(_element_type), count(_count) {
    stride = size_of(element_type);
    size = stride * count;
    align = align_of(element_type);
}

void *SizedStorageType::getelementptr(void *src, size_t i) const {
    verify_range(i, count);
    return (void *)((char *)src + stride * i);
}

Any SizedStorageType::unpack(void *src, size_t i) const {
    return wrap_pointer(type_at_index(i), getelementptr(src, i));
}

const Type *SizedStorageType::type_at_index(size_t i) const {
    verify_range(i, count);
    return element_type;
}

} // namespace scopes
