/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_GEN_SPIRV_HPP
#define SCOPES_GEN_SPIRV_HPP

#include "symbol.hpp"
#include "result.hpp"

#include <vector>

namespace scopes {

struct Function;

SCOPES_RESULT(void) optimize_spirv(std::vector<unsigned int> &result, int opt_level);
SCOPES_RESULT(const String *) compile_spirv(Symbol target, Function *fn, uint64_t flags);
SCOPES_RESULT(const String *) compile_glsl(Symbol target, Function *fn, uint64_t flags);

} // namespace scopes

#endif // SCOPES_GEN_SPIRV_HPP