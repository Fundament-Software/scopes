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

#include "typefactory.hpp"
#include "sampledimage.hpp"
#include "image.hpp"

#include "llvm/Support/Casting.h"

namespace scopes {

using llvm::isa;
using llvm::cast;
using llvm::dyn_cast;

//------------------------------------------------------------------------------
// SAMPLED IMAGE TYPE
//------------------------------------------------------------------------------

bool SampledImageType::classof(const Type *T) {
    return T->kind() == TK_SampledImage;
}

SampledImageType::SampledImageType(const Type *_type) :
    Type(TK_SampledImage), type(cast<ImageType>(_type)) {
    auto ss = StyledString::plain();
    ss.out << "<SampledImage " <<  _type->name()->data << ">";
    _name = ss.str();
}

const Type *SampledImage(const Type *_type) {
    static TypeFactory<SampledImageType> sampled_images;
    return sampled_images.insert(_type);
}


} // namespace scopes
