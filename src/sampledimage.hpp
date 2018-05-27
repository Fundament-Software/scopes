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

    SampledImageType(const Type *_type);

    const ImageType *type; // image type
};

const Type *SampledImage(const Type *_type);

} // namespace scopes

#endif // SCOPES_SAMPLEDIMAGE_HPP