/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_COMPILER_FLAGS_HPP
#define SCOPES_COMPILER_FLAGS_HPP

namespace scopes {

enum {
    CF_DumpDisassembly  = (1 << 0),
    CF_DumpModule       = (1 << 1),
    CF_DumpFunction     = (1 << 2),
    CF_DumpTime         = (1 << 3),
    CF_NoDebugInfo      = (1 << 4),
    CF_O1               = (1 << 5),
    CF_O2               = (1 << 6),
    CF_O3               = CF_O1 | CF_O2,
};

} // namespace scopes

#endif // SCOPES_COMPILER_FLAGS_HPP
