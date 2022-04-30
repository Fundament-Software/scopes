/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_CONFIG_H
#define SCOPES_CONFIG_H

#define SCOPES_VERSION_MAJOR 0
#define SCOPES_VERSION_MINOR 19
#define SCOPES_VERSION_PATCH 0

// trace partial evaluation and code generation
// produces a firehose of information
#define SCOPES_DEBUG_CODEGEN 0

// any location error aborts immediately and can not be caught
#define SCOPES_EARLY_ABORT 0

// print a list of cumulative timers on program exit
#define SCOPES_PRINT_TIMERS 0

// if 0, will never cache modules
#define SCOPES_ALLOW_CACHE 1

// if 1, will warn about missing C type support, such as for some union types
#define SCOPES_WARN_MISSING_CTYPE_SUPPORT 0

// maximum size in bytes of object cache. by default, this is set to 100 MB
#define SCOPES_MAX_CACHE_SIZE (100 << 20)
// maximum number of inodes in cache directory
// we keep this one friendly with FAT32, whose limit is 65534
// and some versions of ext, where the limit is 64000
#define SCOPES_MAX_CACHE_INODES 63000

// maximum number of recursions permitted during partial evaluation
// if you think you need more, ask yourself if ad-hoc compiling a pure C function
// that you can then use at compile time isn't the better choice;
// 100% of the time, the answer is yes because the performance is much better.
#define SCOPES_MAX_RECURSIONS 64

// folder name in ~/.cache in which all cache files are stored
#define SCOPES_CACHE_DIRNAME "scopes"

// compile native code with debug info if not otherwise specified
#define SCOPES_COMPILE_WITH_DEBUG_INFO 1

#ifndef SCOPES_WIN32
#   ifdef _WIN32
#   define SCOPES_WIN32
#   endif
#endif

#ifdef SCOPES_WIN32
//#define SCOPES_USE_WCHAR 1
#define SCOPES_USE_WCHAR 0
#else
#define SCOPES_USE_WCHAR 0
#endif

// maximum size of process stack
#ifdef SCOPES_WIN32
// on windows, we only get 1 MB of stack
// #define SCOPES_MAX_STACK_SIZE ((1 << 10) * 768)
// but we build with "-Wl,--stack,8388608"
#define SCOPES_MAX_STACK_SIZE ((1 << 20) * 7)
#else
// on linux, the system typically gives us 8 MB
#define SCOPES_MAX_STACK_SIZE ((1 << 20) * 7)
#endif

#endif // SCOPES_CONFIG_H

