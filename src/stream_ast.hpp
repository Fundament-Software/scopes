/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_STREAM_AST_HPP
#define SCOPES_STREAM_AST_HPP

#include "styled_stream.hpp"
#include "symbol.hpp"
#include "stream_anchors.hpp"

namespace scopes {

struct Anchor;
struct ASTNode;
struct CallLike;

//------------------------------------------------------------------------------
// AST PRINTER
//------------------------------------------------------------------------------

struct StreamASTFormat {
    enum Tagging {
        All,
        Line,
        None,
    };

    Tagging anchors;
    int depth;

    StreamASTFormat();
};

void stream_ast(
    StyledStream &_ss, ASTNode *node, const StreamASTFormat &_fmt);

} // namespace scopes

#endif // SCOPES_STREAM_AST_HPP