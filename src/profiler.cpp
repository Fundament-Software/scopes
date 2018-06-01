/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "profiler.hpp"
#include "styled_stream.hpp"
#include "label.hpp"
#include "anchor.hpp"
#include "scopes.h"

#include <unordered_map>
#include <vector>
#include <algorithm>

namespace scopes {

static std::unordered_map<Label *, int> spec_count;

void on_label_specialized(Label *l) {
#if SCOPES_PRINT_TIMERS
    auto it = spec_count.find(l);
    if (it == spec_count.end()) {
        spec_count.insert({l, 1});
    } else {
        it->second = it->second + 1;
    }
#endif
}

void print_profiler_info() {
    typedef std::tuple<int, Label *> pair;
    std::vector<pair> pairs;
    for (auto it = spec_count.begin(); it != spec_count.end(); ++it) {
        pairs.push_back(pair(it->second, it->first));
    }
    struct {
        bool operator()(const pair &a, const pair &b) const {
            int ua = std::get<0>(a);
            int ub = std::get<0>(b);
            return ua > ub;
        }
    } customLess;
    std::sort(pairs.begin(), pairs.end(), customLess);
    StyledStream ss;
    size_t printed = 0;
    for (auto &&k : pairs) {
        int used = std::get<0>(k);
        Label *l = std::get<1>(k);
        ss << l->anchor << "used " << used << " times" << std::endl;
        l->anchor->stream_source_line(ss);
        if (++printed == 100)
            break;
    }
}

} // namespace scopes
