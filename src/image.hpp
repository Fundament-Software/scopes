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

#ifndef SCOPES_IMAGE_HPP
#define SCOPES_IMAGE_HPP

#include "type.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// IMAGE TYPE
//------------------------------------------------------------------------------

struct ImageType : Type {
    static bool classof(const Type *T);

    ImageType(
        const Type *_type,
        Symbol _dim,
        int _depth,
        int _arrayed,
        int _multisampled,
        int _sampled,
        Symbol _format,
        Symbol _access);

    const Type *type; // sampled type
    Symbol dim; // resolved to spv::Dim
    int depth; // 0 = not a depth image, 1 = depth image, 2 = undefined
    int arrayed; // 1 = array image
    int multisampled; // 1 = multisampled content
    int sampled; // 0 = runtime dependent, 1 = sampled, 2 = storage image
    Symbol format; // resolved to spv::ImageFormat
    Symbol access; // resolved to spv::AccessQualifier
};

const Type *Image(
    const Type *_type,
    Symbol _dim,
    int _depth,
    int _arrayed,
    int _multisampled,
    int _sampled,
    Symbol _format,
    Symbol _access);

} // namespace scopes

#endif // SCOPES_IMAGE_HPP