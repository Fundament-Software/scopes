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
    bool types;
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

    // minimal repr, no type signatures
    static StreamExprFormat content();

};

typedef StreamExprFormat StreamListFormat;
typedef StreamExprFormat StreamValueFormat;

void stream_list(
    StyledStream &_ss, const List *l, const StreamListFormat &_fmt = StreamListFormat());
void stream_value(
    StyledStream &_ss, const ValueRef &value, const StreamValueFormat &_fmt = StreamValueFormat());
bool is_default_suffix(const Type *T);

} // namespace scopes

#endif // SCOPES_STREAM_EXPR_HPP