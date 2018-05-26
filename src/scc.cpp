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

#include "scc.hpp"
#include "stream_label.hpp"
#include "type.hpp"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------
// SCC
//------------------------------------------------------------------------------

SCCBuilder::SCCBuilder() : C(0) {}

SCCBuilder::SCCBuilder(Label *top) :
    C(0) {
    walk(top);
}

void SCCBuilder::stream_group(StyledStream &ss, const Group &group) {
    ss << "group #" << group.index << " (" << group.labels.size() << " labels):" << std::endl;
    for (size_t k = 0; k < group.labels.size(); ++k) {
        stream_label(ss, group.labels[k], StreamLabelFormat::single());
    }
}

bool SCCBuilder::is_recursive(Label *l) {
    return group(l).labels.size() > 1;
}

bool SCCBuilder::contains(Label *l) {
    auto it = SCCmap.find(l);
    return it != SCCmap.end();
}

size_t SCCBuilder::group_id(Label *l) {
    auto it = SCCmap.find(l);
    assert(it != SCCmap.end());
    return it->second;
}

SCCBuilder::Group &SCCBuilder::group(Label *l) {
    return groups[group_id(l)];
}

void SCCBuilder::walk(Label *obj) {
    Cmap[obj] = C++;
    S.push_back(obj);
    P.push_back(obj);

    int size = (int)obj->body.args.size();
    for (int i = -1; i < size; ++i) {
        Any arg = none;
        if (i == -1) {
            arg = obj->body.enter;
        } else {
            arg = obj->body.args[i].value;
        }

        if (arg.type == TYPE_Label) {
            Label *label = arg.label;

            auto it = Cmap.find(label);
            if (it == Cmap.end()) {
                walk(label);
            } else if (!SCCmap.count(label)) {
                size_t Cw = it->second;
                while (true) {
                    assert(!P.empty());
                    auto it = Cmap.find(P.back());
                    assert(it != Cmap.end());
                    if (it->second <= Cw) break;
                    P.pop_back();
                }
            }
        }
    }

    assert(!P.empty());
    if (P.back() == obj) {
        groups.emplace_back();
        Group &scc = groups.back();
        scc.index = groups.size() - 1;
        while (true) {
            assert(!S.empty());
            Label *q = S.back();
            scc.labels.push_back(q);
            SCCmap[q] = groups.size() - 1;
            S.pop_back();
            if (q == obj) {
                break;
            }
        }
        P.pop_back();
    }
}

} // namespace scopes
