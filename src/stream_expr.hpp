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

namespace scopes {

struct Anchor;
struct Any;
struct List;

//------------------------------------------------------------------------------
// EXPRESSION PRINTER
//------------------------------------------------------------------------------

const char INDENT_SEP[] = "⁞";

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

    static bool is_nested(const Any &_e);

    static bool is_list (const Any &_value);

    void walk(Any e, int depth, int maxdepth, bool naked);

    void stream(const Any &e);
};

void stream_expr(
    StyledStream &_ss, const Any &e, const StreamExprFormat &_fmt);

StyledStream& operator<<(StyledStream& ost, const List *list);

} // namespace scopes

#endif // SCOPES_STREAM_EXPR_HPP