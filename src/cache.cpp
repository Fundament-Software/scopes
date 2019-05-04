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
#include <dirent.h>
//#include <libgen.h>

#include <algorithm>
#include <memory.h>
#include <stdio.h>

#include <zlib.h>

#define SCOPES_CACHE_WRITE_KEY 0
#define SCOPES_FILE_CACHE_EXT ".cache"
#define SCOPES_FILE_CACHE_KEY_PATTERN "%s/%s.cache.key"
#define SCOPES_FILE_CACHE_PATTERN "%s/%s.cache"

namespace scopes {

static bool cache_inited = false;
static char cache_dir[PATH_MAX+1];

// delete half of all cache files to make space
static void perform_thanos_finger_snap(size_t cache_size) {
    auto extsize = strlen(SCOPES_FILE_CACHE_EXT);
    static char cachefile[PATH_MAX+1];
    strcpy(cachefile, cache_dir);
    auto cache_dir_len = strlen(cache_dir);
    cachefile[cache_dir_len] = '/';
    char *cachefile_fname = cachefile + cache_dir_len + 1;

    struct CacheEntry {
        std::string path;
        ssize_t atime;
        ssize_t size;

        bool operator <(const CacheEntry &other) const {
            return atime < other.atime;
        }
    };
    std::vector<CacheEntry> cache_entries;
    cache_entries.reserve(1024);

    struct dirent *dir;
    DIR *d = opendir(cache_dir);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if ( dir->d_type == DT_REG ) {
                auto len = strlen(dir->d_name);
                auto offset = dir->d_name + len - extsize;
                if ((len >= extsize)
                    && !strcmp(offset, SCOPES_FILE_CACHE_EXT)) {
                    strcpy(cachefile_fname, dir->d_name);
                    struct stat s;
                    if( stat(cachefile,&s) == 0 ) {
                        cache_entries.push_back(
                            {cachefile, s.st_atime, s.st_size});
                    }
                }
            }
        }
        closedir(d);
    }

    size_t target_size = SCOPES_MAX_CACHE_SIZE / 2;
    // oldest entries first
    std::sort(cache_entries.begin(), cache_entries.end());
    for (auto &&entry : cache_entries) {
        if (cache_size < target_size)
            break;
        remove(entry.path.c_str());
        cache_size -= entry.size;
    }

}

// count cumulative size of cache files and clean up if too big
static void check_cache_size() {
    auto extsize = strlen(SCOPES_FILE_CACHE_EXT);
    static char cachefile[PATH_MAX+1];
    strcpy(cachefile, cache_dir);
    auto cache_dir_len = strlen(cache_dir);
    cachefile[cache_dir_len] = '/';
    char *cachefile_fname = cachefile + cache_dir_len + 1;

    size_t cache_size = 0;
    struct dirent *dir;
    DIR *d = opendir(cache_dir);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if ( dir->d_type == DT_REG ) {
                auto len = strlen(dir->d_name);
                auto offset = dir->d_name + len - extsize;
                if ((len >= extsize)
                    && !strcmp(offset, SCOPES_FILE_CACHE_EXT)) {
                    strcpy(cachefile_fname, dir->d_name);
                    struct stat s;
                    if( stat(cachefile,&s) == 0 ) {
                        cache_size += s.st_size;
                    }
                }
            }
        }
        closedir(d);
    }

    if (cache_size >= SCOPES_MAX_CACHE_SIZE) {
        perform_thanos_finger_snap(cache_size);
    }
}

static void init_cache() {
    if (cache_inited) return;
    cache_inited = true;

#ifdef SCOPES_WIN32
    char *ptr = getenv("LocalAppData");
    assert(ptr);
    strcpy(cache_dir, ptr);
#define SCOPES_MKDIR(PATH, MODE) mkdir((PATH))
#else
#define SCOPES_MKDIR(PATH, MODE) mkdir((PATH),(MODE))
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
#endif

    if (SCOPES_MKDIR(cache_dir, S_IRWXU)) {
        if (errno != EEXIST) {
            assert(false && "can't create cache directory");
        }
    }

    strncat(cache_dir, "/" SCOPES_CACHE_DIRNAME, PATH_MAX);
    if (SCOPES_MKDIR(cache_dir, S_IRWXU)) {
        if (errno != EEXIST) {
            assert(false && "can't create application cache directory");
        }
    }

    check_cache_size();
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
#ifdef SCOPES_WIN32
#define SCOPES_KEY16_FORMAT "%016llx"
#else
#define SCOPES_KEY16_FORMAT "%016lx"
#endif
    for (int i = 0; i < 4; ++i) {
        snprintf(key + i*16, 17, SCOPES_KEY16_FORMAT, h[i]);
    }
    return String::from(key, 64);
}

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

    //StyledStream ss;
    //ss << "generating " << filepath << std::endl;
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
