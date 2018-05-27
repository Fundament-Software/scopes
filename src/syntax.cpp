/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "syntax.hpp"
#include "type.hpp"
#include "list.hpp"

#include <assert.h>

namespace scopes {

//------------------------------------------------------------------------------
// SYNTAX OBJECTS
//------------------------------------------------------------------------------

Syntax::Syntax(const Anchor *_anchor, const Any &_datum, bool _quoted) :
    anchor(_anchor),
    datum(_datum),
    quoted(_quoted) {}

const Syntax *Syntax::from(const Anchor *_anchor, const Any &_datum, bool quoted) {
    assert(_anchor);
    return new Syntax(_anchor, _datum, quoted);
}

const Syntax *Syntax::from_quoted(const Anchor *_anchor, const Any &_datum) {
    assert(_anchor);
    return new Syntax(_anchor, _datum, true);
}

//------------------------------------------------------------------------------

Any unsyntax(const Any &e) {
    e.verify(TYPE_Syntax);
    return e.syntax->datum;
}

Any maybe_unsyntax(const Any &e) {
    if (e.type == TYPE_Syntax) {
        return e.syntax->datum;
    } else {
        return e;
    }
}

Any strip_syntax(Any e) {
    e = maybe_unsyntax(e);
    if (e.type == TYPE_List) {
        auto src = e.list;
        auto l = src;
        bool needs_unwrap = false;
        while (l != EOL) {
            if (l->at.type == TYPE_Syntax) {
                needs_unwrap = true;
                break;
            }
            l = l->next;
        }
        if (needs_unwrap) {
            l = src;
            const List *dst = EOL;
            while (l != EOL) {
                dst = List::from(strip_syntax(l->at), dst);
                l = l->next;
            }
            return reverse_list_inplace(dst);
        }
    }
    return e;
}

Any wrap_syntax(const Anchor *anchor, Any e, bool quoted) {
    if (e.type == TYPE_List) {
        auto src = e.list;
        auto l = src;
        bool needs_wrap = false;
        while (l != EOL) {
            if (l->at.type != TYPE_Syntax) {
                needs_wrap = true;
                break;
            }
            l = l->next;
        }
        l = src;
        if (needs_wrap) {
            const List *dst = EOL;
            while (l != EOL) {
                dst = List::from(wrap_syntax(anchor, l->at, quoted), dst);
                l = l->next;
            }
            l = reverse_list_inplace(dst);
        }
        return Syntax::from(anchor, l, quoted);
    } else if (e.type != TYPE_Syntax) {
        return Syntax::from(anchor, e, quoted);
    }
    return e;
}

StyledStream& operator<<(StyledStream& ost, const Syntax *value) {
    ost << value->anchor << value->datum;
    return ost;
}


} // namespace scopes
