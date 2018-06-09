/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_EXPANDER_HPP
#define SCOPES_EXPANDER_HPP

#include "any.hpp"

namespace scopes {

struct Scope;
struct Label;

Label *expand_module(Any expr, Scope *scope = nullptr);
Label *expand_inline(Any expr, Scope *scope = nullptr);

} // namespace scopes

#endif // SCOPES_EXPANDER_HPP