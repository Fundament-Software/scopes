/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_STREAM_EXPR_HPP
#define SCOPES_STREAM_EXPR_HPP

#include "styled_stream.hpp"
#include "symbol.hpp"
#include "stream_anchors.hpp"
#include "valueref.inc"

namespace scopes {

struct Anchor;
struct List;
struct Value;

//------------------------------------------------------------------------------
// EXPRESSION PRINTER
//------------------------------------------------------------------------------

Style default_symbol_styler(Symbol name);

struct StreamExprFormat {
    enum Tagging {
        All,
        Line,
        None,
    };

    bool naked;
    Tagging anchors;
    int maxdepth;
    int maxlength;
    Style (*symbol_styler)(Symbol);
    int depth;

    StreamExprFormat();

    static StreamExprFormat debug();

    static StreamExprFormat debug_digest();

    static StreamExprFormat debug_singleline();

    static StreamExprFormat singleline();

    static StreamExprFormat digest();

    static StreamExprFormat singleline_digest();

};

struct StreamExpr : StreamAnchors {
    StreamExprFormat fmt;
    bool line_anchors;
    bool atom_anchors;

    StreamExpr(StyledStream &_ss, const StreamExprFormat &_fmt);

    void stream_indent(int depth = 0);

    static bool is_nested(const ValueRef &_e);

    static bool is_list (const ValueRef &_value);

    void walk(const Anchor *anchor, const List *l, int depth, int maxdepth, bool naked);
    void walk(const ValueRef &e, int depth, int maxdepth, bool naked);

    void stream(const List *l);
};

void stream_expr(
    StyledStream &_ss, const List *l, const StreamExprFormat &_fmt);

} // namespace scopes

#endif // SCOPES_STREAM_EXPR_HPP