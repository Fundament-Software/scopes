/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "stream_ast.hpp"
#include "ast.hpp"
#include "type.hpp"
#include "dyn_cast.inc"

namespace scopes {

static const char INDENT_SEP[] = "⁞";

//------------------------------------------------------------------------------
// AST PRINTER
//------------------------------------------------------------------------------

StreamASTFormat::StreamASTFormat() :
    anchors(All),
    depth(0)
{}

//------------------------------------------------------------------------------

StreamAST::StreamAST(StyledStream &_ss, const StreamASTFormat &_fmt) :
    StreamAnchors(_ss), fmt(_fmt) {
    line_anchors = (fmt.anchors == StreamASTFormat::Line);
    atom_anchors = (fmt.anchors == StreamASTFormat::All);
}

void StreamAST::stream_indent(int depth) {
    if (depth >= 1) {
        ss << Style_Comment << "    ";
        for (int i = 2; i <= depth; ++i) {
            ss << INDENT_SEP << "   ";
        }
        ss << Style_None;
    }
}

void StreamAST::walk(ASTNode *node, int depth, int maxdepth, bool naked) {
    const Anchor *anchor = node->get_anchor();

    if (naked) {
        stream_indent(depth);
    }
    if (atom_anchors) {
        stream_anchor(anchor);
    }

    switch(node->kind()) {
    case ASTK_Function: {
        auto val = cast<ASTFunction>(node);
        ss << Style_Keyword << "ASTFunction" << Style_None;
        if (val->is_inline()) {
            ss << " " << Style_Keyword << "inline" << Style_None;
        }
        if (!val->block) {
            ss << " " << Style_Keyword << "forward-decl" << Style_None;
        }
        ss << " " << val->name << " "
            << Style_Operator << "(" << Style_None;
        for (int i = 0; i < val->params.size(); ++i) {
            if (i > 0) ss << " ";
            walk(val->params[i], depth+1, maxdepth, false);
        }
        ss << Style_Operator << ")" << Style_None;
        if (val->block) {
            if (naked) {
                ss << std::endl;
                walk(val->block, depth+1, maxdepth, true);
            } else {
                ss << " …";
            }
        } else {
            if (naked) ss << std::endl;
        }
    } break;
    case ASTK_Block: {
        auto val = cast<Block>(node);
        ss << Style_Keyword << "Block" << Style_None;
        if (naked && !val->body.empty()) {
            ss << std::endl;
            for (int i = 0; i < val->body.size(); ++i) {
                walk(val->body[i], depth+1, maxdepth, true);
            }
        } else {
            if (naked) ss << std::endl;
        }
    } break;
    case ASTK_If: {
        auto val = cast<If>(node);
        ss << Style_Keyword << "If" << Style_None;
        if (naked) ss << std::endl;
    } break;
    case ASTK_Symbol: {
        auto val = cast<ASTSymbol>(node);
        ss << Style_Symbol << val->name << Style_None;
        if (val->is_variadic()) {
            ss << "…";
        }
        if (val->type != TYPE_Unknown) {
            ss << Style_Operator << ":" << Style_None;
            ss << val->type;
        }
        if (naked) ss << std::endl;
    } break;
    case ASTK_Call: {
        auto val = cast<Call>(node);
        ss << Style_Keyword << "Call" << Style_None;
        if (val->flags & CF_RawCall) {
            ss << Style_Keyword << " rawcall" << Style_None;
        }
        if (val->flags & CF_TryCall) {
            ss << Style_Keyword << " trycall" << Style_None;
        }
        ss << " ";
        walk(val->callee, depth+1, maxdepth, false);
        for (int i = 0; i < val->args.size(); ++i) {
            ss << " ";
            walk(val->args[i].expr, depth+1, maxdepth, false);
        }
        if (naked) ss << std::endl;
    } break;
    case ASTK_Let: {
        auto val = cast<Let>(node);
        ss << Style_Keyword << "Let" << Style_None;
        for (int i = 0; i < val->symbols.size(); ++i) {
            ss << " ";
            walk(val->symbols[i], depth+1, maxdepth, false);
        }
        if (naked && !val->exprs.empty()) {
            ss << Style_Operator << " = " << Style_None;
            ss << std::endl;
            for (int i = 0; i < val->exprs.size(); ++i) {
                walk(val->exprs[i], depth+1, maxdepth, true);
            }
        } else {
            if (naked) ss << std::endl;
        }
    } break;
    case ASTK_Const: {
        auto val = cast<Const>(node);
        ss << val->value;
        if (naked) ss << std::endl;
    } break;
    case ASTK_Break: {
        auto val = cast<Break>(node);
        ss << Style_Keyword << "Break" << Style_None;
        if (naked) ss << std::endl;
    } break;
    case ASTK_Return: {
        auto val = cast<Return>(node);
        ss << Style_Keyword << "Return" << Style_None;
        if (naked) ss << std::endl;
    } break;
    default:
        ss << Style_Error << "<unknown AST node type>" << Style_None;
        if (naked) ss << std::endl;
    }
}

void StreamAST::stream(ASTNode *node) {
    walk(node, fmt.depth, -1, true);
}

//------------------------------------------------------------------------------

void stream_ast(
    StyledStream &_ss, ASTNode *node, const StreamASTFormat &_fmt) {
    StreamAST streamer(_ss, _fmt);
    streamer.stream(node);
}

//------------------------------------------------------------------------------

} // namespace scopes
