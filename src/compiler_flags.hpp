/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_COMPILER_FLAGS_HPP
#define SCOPES_COMPILER_FLAGS_HPP

namespace scopes {

#define SCOPES_COMPILER_FLAGS() \
    T(CF_DumpDisassembly, (1 << 0), "compile-flag-dump-disassembly") \
    T(CF_DumpModule, (1 << 1), "compile-flag-dump-module") \
    T(CF_DumpFunction, (1 << 2), "compile-flag-dump-function") \
    T(CF_DumpTime, (1 << 3), "compile-flag-dump-time") \
    T(CF_NoDebugInfo, (1 << 4), "compile-flag-no-debug-info") \
    T(CF_O0, (1 << 4), "compile-flag-O0") \
    T(CF_O1, (CF_O0 | (1 << 5)), "compile-flag-O1") \
    T(CF_O2, (CF_O0 | (1 << 6)), "compile-flag-O2") \
    T(CF_O3, (CF_O1 | CF_O2), "compile-flag-O3") \
    T(CF_Cache, (1 << 7), "compile-flag-cache") \


enum {
#define T(NAME, VALUE, SNAME) \
    NAME = VALUE,
SCOPES_COMPILER_FLAGS()
#undef T
};

} // namespace scopes

#endif // SCOPES_COMPILER_FLAGS_HPP
