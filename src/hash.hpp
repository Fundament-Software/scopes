/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_HASH_HPP
#define SCOPES_HASH_HPP

#include <stddef.h>
#include <stdint.h>

namespace scopes {

// hash two 64-bit integers (which can also be hashes) into one
uint64_t hash2(uint64_t a, uint64_t b);

// hash a string
uint64_t hash_bytes(const char *s, size_t len);

} // namespace scopes

#endif // SCOPES_HASH_HPP