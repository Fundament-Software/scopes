/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_TRACKER_HPP
#define SCOPES_TRACKER_HPP

#include "result.hpp"

namespace scopes {

struct Function;
struct ASTContext;

SCOPES_RESULT(void) track(ASTContext &ctx);

}

#endif // SCOPES_TRACKER_HPP