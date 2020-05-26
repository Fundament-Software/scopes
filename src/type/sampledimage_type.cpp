/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "sampledimage_type.hpp"
#include "image_type.hpp"
#include "../dyn_cast.inc"
#include "../hash.hpp"

#include <unordered_set>

namespace scopes {

namespace SampledImageSet {
    struct Hash {
        std::size_t operator()(const SampledImageType *s) const {
            return std::hash<const ImageType *>{}(s->type);
        }
    };

    struct KeyEqual {
        bool operator()( const SampledImageType *lhs, const SampledImageType *rhs ) const {
            return lhs->type == rhs->type;
        }
    };
} // namespace SampledImageSet

static std::unordered_set<const SampledImageType *, SampledImageSet::Hash, SampledImageSet::KeyEqual> sampled_images;

//------------------------------------------------------------------------------
// SAMPLED IMAGE TYPE
//------------------------------------------------------------------------------

void SampledImageType::stream_name(StyledStream &ss) const {
    ss << "<SampledImage ";
    stream_type_name(ss, type);
    ss << ">";
}

SampledImageType::SampledImageType(const ImageType *_type) :
    Type(TK_SampledImage), type(_type) {
}

const Type *sampled_image_type(const ImageType *_type) {
    SCOPES_TYPE_KEY(SampledImageType, key);
    key->type = _type;
    auto it = sampled_images.find(key);
    if (it != sampled_images.end())
        return *it;
    auto result = new SampledImageType(_type);
    sampled_images.insert(result);
    return result;
}

//------------------------------------------------------------------------------
// SAMPLER TYPE
//------------------------------------------------------------------------------

void SamplerType::stream_name(StyledStream &ss) const {
    ss << "Sampler";
}

SamplerType::SamplerType() :
    Type(TK_Sampler) {
}

static SamplerType *_sampler_type = nullptr;
const Type *sampler_type() {
    if (!_sampler_type) {
        _sampler_type = new SamplerType();
    }
    return _sampler_type;
}

} // namespace scopes
