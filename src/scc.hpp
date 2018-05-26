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