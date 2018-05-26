/*
Scopes Compiler
Copyright (c) 2016, 2017, 2018 Leonard Ritter

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef SCOPES_STREAM_EXPR_HPP
#define SCOPES_STREAM_EXPR_HPP

#include "styled_stream.hpp"
#include "symbol.hpp"

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

struct StreamAnchors {
    StyledStream &ss;
    const Anchor *last_anchor;

    StreamAnchors(StyledStream &_ss);

    void stream_anchor(const Anchor *anchor, bool quoted = false);
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