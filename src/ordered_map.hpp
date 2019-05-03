/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_ORDERED_MAP_HPP
#define SCOPES_ORDERED_MAP_HPP

#include <vector>
#include <unordered_map>

namespace scopes {

template<typename ValueType>
struct OrderedMap {
    void replace(Symbol key, const ValueType &value) {
        auto it = _symbol_index.find(key);
        if (it == _symbol_index.end()) {
            // new insertion
            int index = keys.size();
            keys.push_back(key);
            values.push_back(value);
            _symbol_index.insert({ key, index });
        } else {
            // update
            int index = it->second;
            values[index] = value;
        }
    }

    int find_index(Symbol key) {
        auto it = _symbol_index.find(key);
        if (it == _symbol_index.end())
            return -1;
        return it->second;
    }

    std::vector<Symbol> keys;
    std::vector<ValueType> values;
    std::unordered_map<Symbol, int, Symbol::Hash> _symbol_index;
};

} // namespace scopes

#endif // SCOPES_ORDERED_MAP_HPP