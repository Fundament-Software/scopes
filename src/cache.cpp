/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "cache.hpp"
#include "hash.hpp"
#include "string.hpp"
#include "scopes/config.h"

#include <limits.h>
//#include <string.h>
#include <sys/stat.h>
#ifdef SCOPES_WIN32
#include "stdlib_ex.h"
#else
#include <wordexp.h>
#endif
//#include <libgen.h>

#include <memory.h>
#include <stdio.h>

#include <zlib.h>

#define SCOPES_CACHE_WRITE_KEY 0

namespace scopes {

static bool cache_inited = false;
static char cache_dir[PATH_MAX];

static void init_cache() {
    if (cache_inited) return;
    cache_inited = true;

    wordexp_t p;
    char** w;
    wordexp( "~/.cache", &p, 0 );
    w = p.we_wordv;
    int offset = 0;
    for (size_t i=0; i<p.we_wordc;i++ ) {
        strncpy(cache_dir + offset, w[i], PATH_MAX - offset);
        offset += strlen(w[i]);
    }
    wordfree( &p );

    if (mkdir(cache_dir, S_IRWXU)) {
        if (errno != EEXIST) {
            assert(false && "can't create cache directory");
        }
    }

    strncat(cache_dir, "/" SCOPES_CACHE_DIRNAME, PATH_MAX);
    if (mkdir(cache_dir, S_IRWXU)) {
        if (errno != EEXIST) {
            assert(false && "can't create application cache directory");
        }
    }
}

const String *get_cache_key(const char *content, size_t size) {
    // split into four parts, hash each part -> 256 bits
    uint64_t h[4];
    memset(h, 0, sizeof(h));
    if (size < 4) {
        h[0] = hash_bytes(content, size);
    } else {
        size_t part = size / 4;
        h[0] = hash_bytes(content, part);
        h[1] = hash2(h[0], hash_bytes(content + part, part));
        h[2] = hash2(h[1], hash_bytes(content + part * 2, part));
        h[3] = hash2(h[2], hash_bytes(content + part * 3, size - 3 * part));
    }
    char key[65];
    memset(key, 0, sizeof(key));
    snprintf(key, 17, "%016lx", h[0]);
    snprintf(key + 16, 17, "%016lx", h[1]);
    snprintf(key + 32, 17, "%016lx", h[2]);
    snprintf(key + 48, 17, "%016lx", h[3]);
    return String::from(key, 64);
}

#define SCOPES_FILE_CACHE_KEY_PATTERN "%s/%s.cache.key"
#define SCOPES_FILE_CACHE_PATTERN "%s/%s.cache"

const char *get_cache_key_file(const String *key) {
#if SCOPES_CACHE_WRITE_KEY
    init_cache();

    static char filepath[PATH_MAX];
    snprintf(filepath, PATH_MAX, SCOPES_FILE_CACHE_KEY_PATTERN, cache_dir, key->data);

    struct stat s;
    if( stat(filepath, &s) == 0 ) {
        if( s.st_mode & S_IFDIR ) {
        } else if ( s.st_mode & S_IFREG ) {
            // exists
            //StyledStream ss;
            //ss << "reusing " << filepath << std::endl;
            return filepath;
        }
    }

    //StyledStream ss;
    //ss << "missing " << filepath << std::endl;
#endif
    return nullptr;
}

const char *get_cache_file(const String *key) {
    init_cache();

    static char filepath[PATH_MAX];
    snprintf(filepath, PATH_MAX, SCOPES_FILE_CACHE_PATTERN, cache_dir, key->data);

    struct stat s;
    if( stat(filepath, &s) == 0 ) {
        if( s.st_mode & S_IFDIR ) {
        } else if ( s.st_mode & S_IFREG ) {
            // exists
            //StyledStream ss;
            //ss << "reusing " << filepath << std::endl;
            return filepath;
        }
    }

    StyledStream ss;
    ss << "generating " << filepath << std::endl;
    return nullptr;
}

void set_cache(const String *key,
    const char *key_content, size_t key_size,
    const char *content, size_t size) {

    char filepath[PATH_MAX];
#if SCOPES_CACHE_WRITE_KEY
    {
        snprintf(filepath, PATH_MAX, SCOPES_FILE_CACHE_KEY_PATTERN, cache_dir, key->data);
        FILE *f = fopen(filepath, "wb");
        fwrite(key_content, key_size, 1, f);
        fclose(f);
    }
#endif

    snprintf(filepath, PATH_MAX, SCOPES_FILE_CACHE_PATTERN, cache_dir, key->data);

    auto f = gzopen(filepath, "wb9");
    if (!f) {
        StyledStream ss;
        ss << "unable to open " << filepath << " for writing" << std::endl;
        return;
    }

    bool failed = (gzfwrite(content, size, 1, f) != 1);

    gzclose(f);

    if (failed) {
        StyledStream ss;
        ss << "unable to write cache to " << filepath << std::endl;
        remove(filepath);
    }
}

} // namespace scopes
