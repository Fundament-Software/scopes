/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_BODY_HPP
#define SCOPES_BODY_HPP

#include "argument.hpp"
#include "any.hpp"

namespace scopes {

struct Anchor;
struct Label;

enum LabelBodyFlags {
    LBF_RawCall = (1 << 0),
    LBF_Complete = (1 << 1)
};

struct Body {
    const Anchor *anchor;
    Any enter;
    Args args;
    uint64_t flags;

    // if there's a scope label, the current frame will be truncated to the
    // parent frame that maps the scope label.
    Label *scope_label;

    Body();

    bool is_complete() const;
    void set_complete();
    void unset_complete();

    bool is_rawcall();

    void set_rawcall(bool enable = true);

    void copy_traits_from(const Body &other);
};


} // namespace scopes

#endif // SCOPES_BODY_HPP