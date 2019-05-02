/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_CACHE_HPP
#define SCOPES_CACHE_HPP

#include <stddef.h>

namespace scopes {

struct String;

const String *get_cache_key(const char *content, size_t size);

const char *get_cache_file(const String *key);
void set_cache(const String *key,
    const char *key_content, size_t key_size,
    const char *content, size_t size);

} // namespace scopes

#endif // SCOPES_CACHE_HPP

