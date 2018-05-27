/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "hash.hpp"

#include "cityhash/city.h"

namespace scopes {

uint64_t hash2(uint64_t a, uint64_t b) {
    return HashLen16(a, b);
}

// hash a string
uint64_t hash_bytes(const char *s, size_t len) {
    return CityHash64(s, len);
}

} // namespace scopes
