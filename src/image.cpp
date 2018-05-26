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
