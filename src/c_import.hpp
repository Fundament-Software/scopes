/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_C_IMPORT_HPP
#define SCOPES_C_IMPORT_HPP

#include "result.hpp"

#include <string>
#include <vector>

namespace scopes {

struct Scope;

//------------------------------------------------------------------------------
// C BRIDGE (CLANG)
//------------------------------------------------------------------------------

SCOPES_RESULT(const Scope *) import_c_module (
    const std::string &path, const std::vector<std::string> &args,
    const char *buffer = nullptr,
    const Scope *scope = nullptr);

} // namespace scopes

#endif // SCOPES_C_IMPORT_HPP
