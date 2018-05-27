/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_SYNTAX_HPP
#define SCOPES_SYNTAX_HPP

#include "any.hpp"

namespace scopes {

struct Anchor;

//------------------------------------------------------------------------------
// SYNTAX OBJECTS
//------------------------------------------------------------------------------

struct Syntax {
protected:
    Syntax(const Anchor *_anchor, const Any &_datum, bool _quoted);

public:
    const Anchor *anchor;
    Any datum;
    bool quoted;

    static const Syntax *from(const Anchor *_anchor, const Any &_datum, bool quoted = false);

    static const Syntax *from_quoted(const Anchor *_anchor, const Any &_datum);
};

Any unsyntax(const Any &e);

Any maybe_unsyntax(const Any &e);

Any strip_syntax(Any e);

Any wrap_syntax(const Anchor *anchor, Any e, bool quoted = false);

StyledStream& operator<<(StyledStream& ost, const Syntax *value);

} // namespace scopes

#endif // SCOPES_SYNTAX_HPP