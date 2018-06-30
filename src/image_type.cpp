/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "image_type.hpp"
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

void ImageType::stream_name(StyledStream &ss) const {
    ss << "<Image ";
    stream_type_name(ss, type);
    ss << " " << dim;
    if (depth == 1)
        ss << " depth";
    else if (depth == 2)
        ss << " ?depth?";
    if (arrayed)
        ss << " array";
    if (multisampled)
        ss << " ms";
    if (sampled == 0)
        ss << " ?sampled?";
    else if (sampled == 1)
        ss << " sampled";
    ss << " " << format;
    if (access != SYM_Unnamed)
        ss << " " << access;
    ss << ">";
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
