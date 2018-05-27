/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_SCC_HPP
#define SCOPES_SCC_HPP

#include "label.hpp"

#include <stddef.h>

namespace scopes {

//------------------------------------------------------------------------------
// SCC
//------------------------------------------------------------------------------

// build strongly connected component map of label graph
// uses Dijkstra's Path-based strong component algorithm
struct SCCBuilder {
    struct Group {
        size_t index;
        Labels labels;
    };

    Labels S;
    Labels P;
    std::unordered_map<Label *, size_t> Cmap;
    std::vector<Group> groups;
    std::unordered_map<Label *, size_t> SCCmap;
    size_t C;

    SCCBuilder();

    SCCBuilder(Label *top);

    void stream_group(StyledStream &ss, const Group &group);

    bool is_recursive(Label *l);

    bool contains(Label *l);

    size_t group_id(Label *l);

    Group &group(Label *l);

    void walk(Label *obj);
};


} // namespace scopes

#endif // SCOPES_SCC_HPP