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

template<typename KeyType, typename ValueType, typename KeyHash>
struct OrderedMap {
    void replace(const KeyType &key, const ValueType &value) {
        auto it = _key_index.find(key);
        if (it == _key_index.end()) {
            // new insertion
            int index = keys.size();
            keys.push_back(key);
            values.push_back(value);
            _key_index.insert({ key, index });
        } else {
            // update
            int index = it->second;
            values[index] = value;
        }
    }

    int find_index(const KeyType &key) const {
        auto it = _key_index.find(key);
        if (it == _key_index.end())
            return -1;
        return it->second;
    }

    std::vector<KeyType> keys;
    std::vector<ValueType> values;
    std::unordered_map<KeyType, int, KeyHash> _key_index;
};

} // namespace scopes

#endif // SCOPES_ORDERED_MAP_HPP