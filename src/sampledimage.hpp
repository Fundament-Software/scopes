/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_SAMPLEDIMAGE_HPP
#define SCOPES_SAMPLEDIMAGE_HPP

#include "type.hpp"

namespace scopes {

struct ImageType;

//------------------------------------------------------------------------------
// SAMPLED IMAGE TYPE
//------------------------------------------------------------------------------

struct SampledImageType : Type {
    static bool classof(const Type *T);

    SampledImageType(const ImageType *_type);

    const ImageType *type; // image type
};

const Type *SampledImage(const ImageType *_type);

} // namespace scopes

#endif // SCOPES_SAMPLEDIMAGE_HPP