/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_SAMPLEDIMAGE_HPP
#define SCOPES_SAMPLEDIMAGE_HPP

#include "../type.hpp"

namespace scopes {

struct ImageType;

//------------------------------------------------------------------------------
// SAMPLED IMAGE TYPE
//------------------------------------------------------------------------------

struct SampledImageType : Type {
    static bool classof(const Type *T);

    void stream_name(StyledStream &ss) const;
    SampledImageType(const ImageType *_type);

    const ImageType *type; // image type
};

const Type *sampled_image_type(const ImageType *_type);

} // namespace scopes

#endif // SCOPES_SAMPLEDIMAGE_HPP