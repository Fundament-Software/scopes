/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_CACHE_HPP
#define SCOPES_CACHE_HPP

#include <stddef.h>
#include <stdint.h>

namespace scopes {

struct String;

const String *get_cache_key(uint64_t hash, const char *content, size_t size);
int get_cache_misses();
const char *get_cache_dir();
const char *get_cache_file(const String *key);
const char *get_cache_key_file(const String *key);
void set_cache(const String *key,
    const char *key_content, size_t key_size,
    const char *content, size_t size);

} // namespace scopes

#endif // SCOPES_CACHE_HPP

