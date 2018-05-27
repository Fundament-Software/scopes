/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "hash.hpp"

#define XXH_INLINE_ALL
#include "xxhash/xxhash.h"

namespace scopes {

// hash a string
uint64_t hash_bytes(const char *s, size_t len) {
    return XXH64(s, len, 0x2a47eba5d9afb4efull);
}

uint64_t hash2(uint64_t a, uint64_t b) {
    if (!(a & b)) {
        uint64_t val = a | b;
        return hash_bytes((const char *)&val, sizeof(uint64_t));
    } else {
        return XXH64_mergeRound(a, b);
    }
}

} // namespace scopes
