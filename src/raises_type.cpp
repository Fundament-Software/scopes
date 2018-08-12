/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "raises_type.hpp"
#include "hash.hpp"
#include "tuple_type.hpp"

#include <assert.h>

#include <unordered_set>

namespace scopes {

namespace RaisesSet {
    struct Hash {
        std::size_t operator()(const RaisesType *s) const {
            std::size_t h = std::hash<const Type *>{}(s->except_type);
            h = hash2(h, std::hash<const Type *>{}(s->result_type));
            return h;
        }
    };

    struct KeyEqual {
        bool operator()( const RaisesType *lhs, const RaisesType *rhs ) const {
            return
                (lhs->except_type == rhs->except_type)
                && (lhs->result_type == rhs->result_type);
        }
    };
} // namespace RaisesSet

static std::unordered_set<const RaisesType *, RaisesSet::Hash, RaisesSet::KeyEqual> raises;

//------------------------------------------------------------------------------
// RAISES TYPE
//------------------------------------------------------------------------------

void RaisesType::stream_name(StyledStream &ss) const {
    stream_type_name(ss, result_type);
    ss << "!!";
    stream_type_name(ss, except_type);
}

RaisesType::RaisesType(const Type *_except_type, const Type *_result_type)
    : Type(TK_Raises), except_type(_except_type), result_type(_result_type) {
}

//------------------------------------------------------------------------------

const Type *raises_type(
    const Type *except_type, const Type *result_type) {
    RaisesType key(except_type, result_type);
    auto it = raises.find(&key);
    if (it != raises.end())
        return *it;
    auto result = new RaisesType(except_type, result_type);
    raises.insert(result);

    return result;
}

const Type *raises_type(const Type *result_type) {
    return raises_type(TYPE_Error, result_type);
}

} // namespace scopes
