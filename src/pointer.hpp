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

#ifndef SCOPES_POINTER_HPP
#define SCOPES_POINTER_HPP

#include "type.hpp"

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

    void *getelementptr(void *src, size_t i) const;

    Any unpack(void *src) const;
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

} // namespace scopes

#endif // SCOPES_POINTER_HPP