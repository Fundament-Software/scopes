/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_EXPANDER_HPP
#define SCOPES_EXPANDER_HPP

#include "result.hpp"

namespace scopes {

struct Value;
struct Scope;
struct Anchor;
struct Template;
struct List;

SCOPES_RESULT(Template *) expand_module(const Anchor *anchor, const List *expr, Scope *scope = nullptr);
SCOPES_RESULT(Template *) expand_inline(const Anchor *anchor, Template *astscope, const List *expr, Scope *scope = nullptr);

} // namespace scopes

#endif // SCOPES_EXPANDER_HPP