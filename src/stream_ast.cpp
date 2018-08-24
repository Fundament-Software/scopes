/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "stream_ast.hpp"
#include "value.hpp"
#include "type.hpp"
#include "dyn_cast.inc"

#include <unordered_set>

namespace scopes {

static const char INDENT_SEP[] = "⁞";

//------------------------------------------------------------------------------
// AST PRINTER
//------------------------------------------------------------------------------

StreamASTFormat::StreamASTFormat() :
    anchors(None),
    depth(0)
{}

StreamASTFormat StreamASTFormat::debug() {
    auto fmt = StreamASTFormat();
    fmt.anchors = Line;
    return fmt;
}

//------------------------------------------------------------------------------

struct StreamAST : StreamAnchors {
    StreamASTFormat fmt;
    bool line_anchors;
    bool atom_anchors;

    std::unordered_set<const Value *> visited;

    StreamAST(StyledStream &_ss, const StreamASTFormat &_fmt)
        : StreamAnchors(_ss), fmt(_fmt) {
        line_anchors = (fmt.anchors == StreamASTFormat::Line);
        atom_anchors = (fmt.anchors == StreamASTFormat::All);
    }

    void stream_indent(int depth = 0) {
        const int N = 50;
        if (depth > N) {
            int levels = depth / N;
            depth = depth % N;
            ss << "[" << (levels * N) << "]";
        }
        if (depth >= 1) {
            ss << Style_Comment << "  ";
            for (int i = 2; i <= depth; ++i) {
                ss << INDENT_SEP << " ";
            }
            ss << Style_None;
        }
    }

    void write_arguments(const Values &args, int depth, int maxdepth) {
        for (int i = 0; i < args.size(); ++i) {
            ss << std::endl;
            auto &&arg = args[i];

            //if (arg.key == SYM_Unnamed) {
                walk(arg, depth+1, maxdepth);
            #if 0
            } else {
                stream_indent(depth+1);
                ss << arg.key << " "
                    << Style_Operator << "=" << Style_None << std::endl;
                walk(args[i].expr, depth+2, maxdepth);
            }
            #endif
        }
    }

    void walk(const Value *node, int depth, int maxdepth) {
        const Anchor *anchor = node->anchor();

        stream_indent(depth);
        if (line_anchors) {
            stream_anchor(anchor);
        }

        bool is_new = !visited.count(node);
        if (is_new)
            visited.insert(node);

        if (node->is_typed()) {
            ss << (const Type *)node->get_type() << " ◀ ";
        }
        switch(node->kind()) {
        case VK_Template: {
            auto val = cast<Template>(node);
            ss << Style_Keyword << "Template" << Style_None;
            if (val->is_inline()) {
                ss << " " << Style_Keyword << "inline" << Style_None;
            }
            if (!val->value) {
                ss << " " << Style_Keyword << "forward-decl" << Style_None;
            }
            ss << " ";
            ss << Style_Symbol << val->name.name()->data
                << "λ" << (void *)val << Style_None;
            if (is_new) {
                for (int i = 0; i < val->params.size(); ++i) {
                    ss << std::endl;
                    walk(val->params[i], depth+1, maxdepth);
                }
                if (val->scope) {
                    ss << std::endl;
                    stream_indent(depth+1);
                    ss << "scope = ";
                    ss << Style_Symbol << val->scope->name.name()->data
                        << "λ" << (void *)val->scope << Style_None;
                }
                if (val->value) {
                    ss << std::endl;
                    walk(val->value, depth+1, maxdepth);
                }
            } else {
                ss << " <...>";
            }
        } break;
        case VK_Function: {
            auto val = cast<Function>(node);
            ss << Style_Keyword << "Function" << Style_None;
            ss << " ";
            ss << Style_Symbol << val->name.name()->data
                << "λ" << (void *)val << Style_None;
            if (is_new) {
                for (int i = 0; i < val->params.size(); ++i) {
                    ss << std::endl;
                    walk(val->params[i], depth+1, maxdepth);
                }
                if (val->value) {
                    ss << std::endl;
                    walk(val->value, depth+1, maxdepth);
                }
            } else {
                ss << " <...>";
            }
        } break;
        case VK_Block: {
            auto val = cast<Block>(node);
            ss << Style_Keyword << "Block" << Style_None;
            for (int i = 0; i < val->body.size(); ++i) {
                ss << std::endl;
                walk(val->body[i], depth+1, maxdepth);
            }
            ss << std::endl;
            walk(val->value, depth+1, maxdepth);
        } break;
        case VK_If: {
            auto val = cast<If>(node);
            ss << Style_Keyword << "If" << Style_None;
            for (int i = 0; i < val->clauses.size(); ++i) {
                ss << std::endl;
                stream_indent(depth+1);
                ss << "clause" << Style_None;
                ss << std::endl;
                auto &&expr = val->clauses[i];
                if (expr.cond)
                    walk(expr.cond, depth+2, maxdepth);
                ss << std::endl;
                walk(expr.value, depth+2, maxdepth);
            }
            ss << std::endl;
            stream_indent(depth+1);
            ss << "else";
            ss << std::endl;
            walk(val->else_clause.value, depth+2, maxdepth);
        } break;
        case VK_Symbol: {
            auto val = cast<SymbolValue>(node);
            ss << Style_Symbol << val->name.name()->data
                << "$" << (void *)val << Style_None;
            if (val->is_variadic()) {
                ss << "…";
            }
        } break;
        case VK_Call: {
            auto val = cast<Call>(node);
            ss << Style_Keyword << "Call" << Style_None;
            if (val->flags & CF_RawCall) {
                ss << Style_Keyword << " rawcall" << Style_None;
            }
            if (val->flags & CF_TryCall) {
                ss << Style_Keyword << " trycall" << Style_None;
            }
            ss << std::endl;
            walk(val->callee, depth+1, maxdepth);
            write_arguments(val->args, depth, maxdepth);
        } break;
        case VK_Let: {
            auto val = cast<Let>(node);
            ss << Style_Keyword << "Let" << Style_None;
            for (int i = 0; i < val->params.size(); ++i) {
                ss << std::endl;
                walk(val->params[i], depth+1, maxdepth);
            }
            for (int i = 0; i < val->args.size(); ++i) {
                ss << std::endl;
                walk(val->args[i], depth+2, maxdepth);
            }
        } break;
        case VK_ArgumentList: {
            auto val = cast<ArgumentList>(node);
            ss << Style_Keyword << "ArgumentList" << Style_None;
            for (int i = 0; i < val->values.size(); ++i) {
                ss << std::endl;
                walk(val->values[i], depth+1, maxdepth);
            }
        } break;
        case VK_ExtractArgument: {
            auto val = cast<ExtractArgument>(node);
            ss << Style_Keyword << "ExtractArgument" << Style_None;
            ss << " " << val->index;
            ss << std::endl;
            walk(val->value, depth+1, maxdepth);
        } break;
        case VK_Loop: {
            auto val = cast<Loop>(node);
            ss << Style_Keyword << "Loop" << Style_None;
            for (int i = 0; i < val->params.size(); ++i) {
                ss << std::endl;
                walk(val->params[i], depth+1, maxdepth);
            }
            for (int i = 0; i < val->args.size(); ++i) {
                ss << std::endl;
                walk(val->args[i], depth+2, maxdepth);
            }
            ss << std::endl;
            walk(val->value, depth+1, maxdepth);
        } break;
        case VK_Extern: {
            auto val = cast<Extern>(node);
            ss << Style_Keyword << "Extern" << Style_None << " " << val->name;
        } break;
        case VK_ConstInt: {
            auto val = cast<ConstInt>(node);
            ss << Style_Keyword << "ConstInt" << Style_None << " ";
            auto T = val->get_type();
            if ((T == TYPE_Builtin) || (T == TYPE_Symbol)) {
                ss << Symbol::wrap(val->value);
            } else {
                ss << val->value;
            }
        } break;
        case VK_ConstReal: {
            auto val = cast<ConstReal>(node);
            ss << Style_Keyword << "ConstReal" << Style_None << " " << val->value;
        } break;
        case VK_ConstPointer: {
            auto val = cast<ConstPointer>(node);
            ss << Style_Keyword << "ConstPointer" << Style_None << " ";
            auto T = val->get_type();
            if (T == TYPE_Type) {
                ss << (const Type *)val->value;
            } else if (T == TYPE_String) {
                ss << (const String *)val->value;
            } else if (T == TYPE_Closure) {
                ss << (const Closure *)val->value;
            } else {
                ss << val->value;
            }
        } break;
        case VK_ConstTuple: {
            auto val = cast<ConstTuple>(node);
            ss << Style_Keyword << "ConstTuple" << Style_None;
            for (int i = 0; i < val->values.size(); ++i) {
                ss << std::endl;
                walk(val->values[i], depth+1, maxdepth);
            }
        } break;
        case VK_Break: {
            auto val = cast<Break>(node);
            ss << Style_Keyword << "Break" << Style_None;
            ss << std::endl;
            walk(val->value, depth+1, maxdepth);
        } break;
        case VK_Repeat: {
            auto val = cast<Repeat>(node);
            ss << Style_Keyword << "Repeat" << Style_None;
            write_arguments(val->args, depth, maxdepth);
        } break;
        case VK_Return: {
            auto val = cast<Return>(node);
            ss << Style_Keyword << "Return" << Style_None;
            ss << std::endl;
            walk(val->value, depth+1, maxdepth);
        } break;
        case VK_Raise: {
            auto val = cast<Raise>(node);
            ss << Style_Keyword << "Raise" << Style_None;
            ss << std::endl;
            walk(val->value, depth+1, maxdepth);
        } break;
        case VK_SyntaxExtend: {
            auto val = cast<SyntaxExtend>(node);
            ss << Style_Keyword << "SyntaxExtend" << Style_None;
            ss << std::endl;
            walk(val->func, depth+1, maxdepth);
        } break;
        default:
            ss << Style_Error << "<unhandled AST node type: "
                << get_value_kind_name(node->kind())
                << ">" << Style_None;
        }
    }

    void stream(const Value *node) {
        visited.clear();
        walk(node, fmt.depth, -1);
        ss << std::endl;
    }
};

//------------------------------------------------------------------------------

void stream_ast(
    StyledStream &_ss, const Value *node, const StreamASTFormat &_fmt) {
    StreamAST streamer(_ss, _fmt);
    streamer.stream(node);
}

//------------------------------------------------------------------------------

} // namespace scopes
