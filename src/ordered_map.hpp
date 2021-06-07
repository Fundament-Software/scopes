/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_ORDERED_MAP_HPP
#define SCOPES_ORDERED_MAP_HPP

#include <vector>
#include <unordered_map>
#include <algorithm>

namespace scopes {

template<typename KeyType, typename ValueType, typename KeyHash = std::hash<KeyType> >
struct OrderedMap {
    // fails if value has already been inserted
    bool insert(const KeyType &key, const ValueType &value) {
        auto it = _key_index.find(key);
        if (it == _key_index.end()) {
            // new insertion
            int index = entries.size();
            entries.push_back({ key, value});
            _key_index.insert({ key, index });
            return true;
        }
        return false;
    }

    void replace(const KeyType &key, const ValueType &value) {
        auto it = _key_index.find(key);
        if (it == _key_index.end()) {
            // new insertion
            int index = entries.size();
            entries.push_back({ key, value});
            _key_index.insert({ key, index });
        } else {
            // update
            int index = it->second;
            entries[index] = { key, value };
        }
    }

    void discard(const KeyType &key) {
        auto it = _key_index.find(key);
        if (it != _key_index.end()) {
            // update
            int index = it->second;
            _key_index.erase(it);
            entries.erase(entries.begin() + index);
            for (auto &&it : _key_index) {
                if (it.second > index)
                    it.second -= 1;
            }
        }
    }

    int find_index(const KeyType &key) const {
        auto it = _key_index.find(key);
        if (it == _key_index.end())
            return -1;
        return it->second;
    }

    // reverse order in-place
    void flip() {
        std::reverse(entries.begin(), entries.end());
        int lastindex = entries.size() - 1;
        for (auto &&it : _key_index) {
            it.second = lastindex - it.second;
        }
    }

    std::vector< std::pair<KeyType, ValueType> > entries;
    std::unordered_map<KeyType, int, KeyHash> _key_index;
};

} // namespace scopes

#endif // SCOPES_ORDERED_MAP_HPP