/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
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

    void stream_name(StyledStream &ss) const;

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