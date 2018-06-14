/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "image.hpp"
#include "hash.hpp"

#include <unordered_set>

namespace scopes {

namespace ImageSet {
    struct Hash {
        std::size_t operator()(const ImageType *s) const {
            size_t h = std::hash<const Type *>{}(s->type);
            h = hash2(h, s->dim.hash());
            h = hash2(h, std::hash<int>{}(s->depth));
            h = hash2(h, std::hash<int>{}(s->arrayed));
            h = hash2(h, std::hash<int>{}(s->multisampled));
            h = hash2(h, std::hash<int>{}(s->sampled));
            h = hash2(h, s->format.hash());
            h = hash2(h, s->access.hash());
            return h;
        }
    };

    struct KeyEqual {
        bool operator()( const ImageType *lhs, const ImageType *rhs ) const {
            return
                lhs->type == rhs->type
                && lhs->dim == rhs->dim
                && lhs->depth == rhs->depth
                && lhs->arrayed == rhs->arrayed
                && lhs->multisampled == rhs->multisampled
                && lhs->sampled == rhs->sampled
                && lhs->format == rhs->format
                && lhs->access == rhs->access;
        }
    };
} // namespace ImageSet

static std::unordered_set<const ImageType *, ImageSet::Hash, ImageSet::KeyEqual> images;

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
    SCOPES_TYPE_KEY(ImageType, key);
    key->type = _type;
    key->dim = _dim;
    key->depth = _depth;
    key->arrayed = _arrayed;
    key->multisampled = _multisampled;
    key->sampled = _sampled;
    key->format = _format;
    key->access = _access;
    auto it = images.find(key);
    if (it != images.end())
        return *it;
    auto result = new ImageType(_type, _dim, _depth, _arrayed,
        _multisampled, _sampled, _format, _access);
    images.insert(result);
    return result;
}

} // namespace scopes
