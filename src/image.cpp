/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "image.hpp"
#include "typefactory.hpp"

namespace scopes {

//------------------------------------------------------------------------------
// IMAGE TYPE
//------------------------------------------------------------------------------

bool ImageType::classof(const Type *T) {
    return T->kind() == TK_Image;
}

ImageType::ImageType(
    const Type *_type,
    Symbol _dim,
    int _depth,
    int _arrayed,
    int _multisampled,
    int _sampled,
    Symbol _format,
    Symbol _access) :
    Type(TK_Image),
    type(_type), dim(_dim), depth(_depth), arrayed(_arrayed),
    multisampled(_multisampled), sampled(_sampled),
    format(_format), access(_access) {
    auto ss = StyledString::plain();
    ss.out << "<Image " <<  _type->name()->data
        << " " << _dim;
    if (_depth == 1)
        ss.out << " depth";
    else if (_depth == 2)
        ss.out << " ?depth?";
    if (_arrayed)
        ss.out << " array";
    if (_multisampled)
        ss.out << " ms";
    if (_sampled == 0)
        ss.out << " ?sampled?";
    else if (_sampled == 1)
        ss.out << " sampled";
    ss.out << " " << _format;
    if (access != SYM_Unnamed)
        ss.out << " " << _access;
    ss.out << ">";
    _name = ss.str();
}

const Type *Image(
    const Type *_type,
    Symbol _dim,
    int _depth,
    int _arrayed,
    int _multisampled,
    int _sampled,
    Symbol _format,
    Symbol _access) {
    static TypeFactory<ImageType> images;
    return images.insert(_type, _dim, _depth, _arrayed,
        _multisampled, _sampled, _format, _access);
}


} // namespace scopes
