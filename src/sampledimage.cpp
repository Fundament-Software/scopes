/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
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
