/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_UNIQUE_TYPE_HPP
#define SCOPES_UNIQUE_TYPE_HPP

#include "../type.hpp"
#include "../type/qualify_type.hpp"

#include <vector>
#include <unordered_set>

namespace scopes {

enum {
    UnknownUnique = 0,
    FirstUniqueInput = 1,
    LastUniqueInput = 256,
    FirstUniqueOutput = -1,
    LastUniqueOutput = -256,
    FirstUniqueError = -257,
    LastUniqueError = -512,

    FirstUniquePrivate = 1000,
};

//------------------------------------------------------------------------------

struct MutateQualifier : Qualifier {
    enum { Kind = QK_Mutate };
    static bool classof(const Qualifier *T);

    void stream_prefix(StyledStream &ss) const;
    void stream_postfix(StyledStream &ss) const;

    MutateQualifier();
};

//------------------------------------------------------------------------------

typedef std::unordered_set<int> IDSet;
typedef std::vector<int> IDs;
typedef std::unordered_map<int, IDSet > ID2SetMap;

struct ViewQualifier : Qualifier {
    enum { Kind = QK_View };
    static bool classof(const Qualifier *T);

    void stream_prefix(StyledStream &ss) const;
    void stream_postfix(StyledStream &ss) const;

    ViewQualifier(const IDSet &ids);

    IDSet ids;
    IDs sorted_ids;
    std::size_t prehash;
};

//------------------------------------------------------------------------------

struct UniqueQualifier : Qualifier {
    enum { Kind = QK_Unique };
    static bool classof(const Qualifier *T);

    void stream_prefix(StyledStream &ss) const;
    void stream_postfix(StyledStream &ss) const;

    UniqueQualifier(int id);

    int id;
};

//------------------------------------------------------------------------------

const Type *mutate_type(const Type *type);
const Type *view_type(const Type *type, IDSet ids);
const Type *unique_type(const Type *type, int id);

bool is_view(const Type *T);
const Type *strip_view(const Type *T);
const ViewQualifier *get_view(const Type *T);
const ViewQualifier *try_view(const Type *T);

bool is_unique(const Type *T);
const Type *strip_unique(const Type *T);
const UniqueQualifier *get_unique(const Type *T);
const UniqueQualifier *try_unique(const Type *T);

} // namespace scopes

#endif // SCOPES_UNIQUE_TYPE_HPP