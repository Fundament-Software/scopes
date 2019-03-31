/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_EXPANDER_HPP
#define SCOPES_EXPANDER_HPP

#include "result.hpp"
#include "symbol.hpp"
#include "valueref.inc"
#include "scopes/scopes.h"

namespace scopes {

struct Value;
struct Scope;
struct Anchor;
struct Template;
struct List;

SCOPES_RESULT(sc_valueref_list_scope_tuple_t) expand(const ValueRef &expr, const List *next, Scope *scope = nullptr);
SCOPES_RESULT(TemplateRef) expand_module(const Anchor *anchor, const List *expr, Scope *scope = nullptr);
SCOPES_RESULT(TemplateRef) expand_inline(const Anchor *anchor, const TemplateRef &astscope, const List *expr, Scope *scope = nullptr);

} // namespace scopes

#endif // SCOPES_EXPANDER_HPP