/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "stream_ast.hpp"
#include "value.hpp"
#include "type.hpp"
#include "dyn_cast.inc"
#include "types.hpp"

#include <unordered_map>
#include <queue>

namespace scopes {

static const char INDENT_SEP[] = "⁞";

//------------------------------------------------------------------------------
// AST PRINTER
//------------------------------------------------------------------------------

StreamASTFormat::StreamASTFormat() :
    anchors(None),
    depth(0),
    newlines(true)
{}

StreamASTFormat StreamASTFormat::debug() {
    auto fmt = StreamASTFormat();
    fmt.anchors = Line;
    return fmt;
}

StreamASTFormat StreamASTFormat::singleline() {
    auto fmt = StreamASTFormat();
    fmt.anchors = None;
    fmt.newlines = false;
    return fmt;
}

//------------------------------------------------------------------------------

struct StreamAST : StreamAnchors {
    StreamASTFormat fmt;
    bool line_anchors;
    bool atom_anchors;
    bool newlines;
    int nextid;

    std::unordered_map<const Value *, int> visited;
    std::vector<const Value *> todo;

    StreamAST(StyledStream &_ss, const StreamASTFormat &_fmt)
        : StreamAnchors(_ss), fmt(_fmt), nextid(0) {
        line_anchors = (fmt.anchors == StreamASTFormat::Line);
        atom_anchors = (fmt.anchors == StreamASTFormat::All);
        newlines = fmt.newlines;
    }

    void stream_newline() {
        if (newlines)
            ss << std::endl;
        else
            ss << " ";
    }

    void stream_indent(int depth = 0) {
        if (!newlines)
            return;
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
            auto &&arg = args[i];
            walk_same_or_newline(arg, depth+1, maxdepth);
        }
    }

    void stream_type_suffix(const Type *T) {
        ss << Style_Operator << ":" << Style_None << T;
    }

    void stream_type_prefix(const Value *node) {
        if (node->is_typed()) {
            ss << (const Type *)node->get_type() << Style_Comment << " ◀ " << Style_None;
        }
    }

    void stream_block(const Block &block, int depth, int maxdepth) {
        if (block.empty())
            return;
        stream_newline();
        stream_indent(depth);
        ss << Style_Keyword << "body" << Style_None;
        for (int i = 0; i < block.body.size(); ++i) {
            walk_newline(block.body[i], depth+1, maxdepth);
        }
    }

    void stream_block_result(const Block &block, const Value *value, int depth, int maxdepth) {
        if (!block.empty()) {
            for (int i = 0; i < block.body.size(); ++i) {
                walk_newline(block.body[i], depth, maxdepth);
            }
        }
        stream_newline();
        stream_indent(depth);
        ss << Style_Keyword << "_ " << Style_None;
        walk(value, depth, maxdepth);
    }

    bool sameline(const Value *node) {
        switch(node->kind()) {
        case VK_ConstInt:
        case VK_ConstReal:
        case VK_ConstPointer:
        case VK_Parameter:
        case VK_Extern:
        case VK_Template:
        case VK_Function:
            return true;
        default: break;
        }
        if (visited.count(node))
            return true;
        return false;
    }

    void walk_newline(const Value *node, int depth, int maxdepth) {
        stream_newline();
        stream_indent(depth);
        walk(node, depth, maxdepth);
    }

    void walk_same_or_newline(const Value *node, int depth, int maxdepth) {
        if (sameline(node)) {
            ss << " ";
        } else {
            stream_newline();
            stream_indent(depth);
        }
        walk(node, depth, maxdepth);
    }

    void walk_todos(int depth, int maxdepth) {
        auto oldtodos = todo;
        todo.clear();
        for (auto node : oldtodos) {
            visited.erase(node);
            walk(node, depth, maxdepth);
            if (newlines)
                stream_newline();
        }
    }

    void stream_illegal_value_type(const std::string &name, const Type *T) {
        ss << Style_Error << "<illegal type for " << name << ">" << Style_None;
        stream_type_suffix(T);
    }

    // old Any printing reference:
    // https://bitbucket.org/duangle/scopes/src/dfb69b02546e859b702176c58e92a63de3461d77/src/any.cpp

    void walk(const Value *node, int depth, int maxdepth) {
        if (!node) {
            ss << Style_Error << "<null>" << Style_None;
            return;
        }

        const Anchor *anchor = node->anchor();
        if (line_anchors) {
            stream_anchor(anchor);
        }

        auto it = visited.find(node);
        int id = -1;
        bool is_new = true;
        if (it == visited.end()) {
            if (!node->is_pure()) {
                id = nextid++;
            }
            visited.insert({node, id});
        } else {
            is_new = false;
            id = it->second;
        }

        if (newlines && !node->is_pure()) {
            ss << Style_Operator << "%" << id << Style_None;
            if (is_new) {
                ss << " " << Style_Operator << "=" << " " << Style_None;
            } else {
                if (node->is_typed()) {
                    stream_type_suffix(node->get_type());
                }
                return;
            }
        }

        switch(node->kind()) {
        case VK_Template: {
            stream_type_prefix(node);
            auto val = cast<Template>(node);
            if (newlines && is_new && (depth == 0)) {
                ss << Style_Keyword << "Template" << Style_None;
                if (val->is_inline()) {
                    ss << " " << Style_Keyword << "inline" << Style_None;
                }
                if (!val->value) {
                    ss << " " << Style_Keyword << "forward-decl" << Style_None;
                }
                ss << " ";
            }
            ss << Style_Symbol << val->name.name()->data
                << "λ" << (void *)val << Style_None;
            if (newlines && is_new) {
                if (depth == 0) {
                    ss << Style_Operator << " (" << Style_None;
                    for (int i = 0; i < val->params.size(); ++i) {
                        walk_same_or_newline(val->params[i], depth+1, maxdepth);
                    }
                    ss << Style_Operator << " )" << Style_None;
                    if (val->scope) {
                        stream_newline();
                        stream_indent(depth+1);
                        ss << "scope = ";
                        ss << Style_Symbol << val->scope->name.name()->data
                            << "λ" << (void *)val->scope << Style_None;
                    }
                    if (val->value) {
                        walk_newline(val->value, depth+1, maxdepth);
                    }
                } else {
                    todo.push_back(node);
                }
            }
        } break;
        case VK_Function: {
            auto val = cast<Function>(node);
            if (newlines && is_new && (depth == 0)) {
                ss << Style_Keyword << "Function" << Style_None;
                ss << " ";
            }
            ss << Style_Symbol << val->name.name()->data
                << "λ" << (void *)val << Style_None;
            if (node->is_typed()) {
                stream_type_suffix(node->get_type());
            }
            if (newlines && is_new) {
                if (depth == 0) {
                    ss << Style_Operator << " (" << Style_None;
                    for (int i = 0; i < val->params.size(); ++i) {
                        walk_same_or_newline(val->params[i], depth+1, maxdepth);
                    }
                    ss << Style_Operator << " )" << Style_None;
                    for (int i = 0; i < val->body.body.size(); ++i) {
                        walk_newline(val->body.body[i], depth+1, maxdepth);
                    }
                } else {
                    todo.push_back(node);
                }
            }
        } break;
        case VK_Expression: {
            stream_type_prefix(node);
            auto val = cast<Expression>(node);
            ss << Style_Keyword << "Expression" << Style_None;
            if (!val->scoped) {
                ss << " " << Style_Keyword << "inline" << Style_None;
            }
            for (int i = 0; i < val->body.size(); ++i) {
                walk_newline(val->body[i], depth+1, maxdepth);
            }
            walk_newline(val->value, depth+1, maxdepth);
        } break;
        case VK_Try: {
            stream_type_prefix(node);
            auto val = cast<Try>(node);
            ss << Style_Keyword << "Try" << Style_None;
            stream_block_result(val->try_body, val->try_value, depth+2, maxdepth);
            stream_newline();
            stream_indent(depth+1);
            ss << Style_Keyword << "except" << Style_None;
            walk_same_or_newline(val->except_param, depth+2, maxdepth);
            stream_block_result(val->except_body, val->except_value, depth+2, maxdepth);
        } break;
        case VK_If: {
            stream_type_prefix(node);
            auto val = cast<If>(node);
            ss << Style_Keyword << "If" << Style_None;
            if (newlines) {
                for (int i = 0; i < val->clauses.size(); ++i) {
                    auto &&expr = val->clauses[i];
                    if (expr.cond) {
                        stream_newline();
                        stream_indent(depth+2);
                        ss << Style_Keyword << "condition" << Style_None;
                        if (expr.cond_body.empty()) {
                            walk_same_or_newline(expr.cond, depth+3, maxdepth);
                        } else {
                            stream_block_result(expr.cond_body, expr.cond, depth+3, maxdepth);
                        }
                    }
                    stream_block_result(expr.body, expr.value, depth+2, maxdepth);
                }
                stream_newline();
                stream_indent(depth+1);
                ss << Style_Keyword << "else" << Style_None;
                stream_block_result(val->else_clause.body, val->else_clause.value, depth+2, maxdepth);
            }
        } break;
        case VK_Parameter: {
            auto val = cast<Parameter>(node);
            ss << Style_Symbol << val->name.name()->data
                << "$" << (void *)val << Style_None;
            if (val->is_variadic()) {
                ss << Style_Keyword << "…" << Style_None;
            }
            if (node->is_typed())
                stream_type_suffix(node->get_type());
        } break;
        case VK_Call: {
            stream_type_prefix(node);
            auto val = cast<Call>(node);
            ss << Style_Keyword << "Call" << Style_None;
            if (newlines) {
                if (val->flags & CF_RawCall) {
                    ss << Style_Keyword << " rawcall" << Style_None;
                }
                if (val->flags & CF_TryCall) {
                    ss << Style_Keyword << " trycall" << Style_None;
                }
                walk_same_or_newline(val->callee, depth+1, maxdepth);
                write_arguments(val->args, depth, maxdepth);
            }
        } break;
        case VK_ArgumentList: {
            stream_type_prefix(node);
            auto val = cast<ArgumentList>(node);
            ss << Style_Keyword << "ArgumentList" << Style_None;
            if (newlines) {
                for (int i = 0; i < val->values.size(); ++i) {
                    walk_same_or_newline(val->values[i], depth+1, maxdepth);
                }
            }
        } break;
        case VK_ExtractArgument: {
            stream_type_prefix(node);
            auto val = cast<ExtractArgument>(node);
            ss << Style_Keyword << "ExtractArgument" << Style_None;
            if (newlines) {
                ss << " " << val->index;
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Keyed: {
            stream_type_prefix(node);
            auto val = cast<Keyed>(node);
            ss << Style_Keyword << "Keyed" << Style_None;
            if (newlines) {
                ss << " " << val->key;
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Loop: {
            stream_type_prefix(node);
            auto val = cast<Loop>(node);
            ss << Style_Keyword << "Loop" << Style_None;
            if (newlines) {
                for (int i = 0; i < val->params.size(); ++i) {
                    walk_same_or_newline(val->params[i], depth+1, maxdepth);
                }
                ss << " " << Style_Operator << "=" << Style_None;
                for (int i = 0; i < val->args.size(); ++i) {
                    walk_same_or_newline(val->args[i], depth+1, maxdepth);
                }
                stream_block_result(val->body, val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Extern: {
            auto val = cast<Extern>(node);
            auto T = val->get_type();
            ss << val->name;
            stream_type_suffix(T);
        } break;
        case VK_ConstInt: {
            auto val = cast<ConstInt>(node);
            auto T = val->get_type();
            if ((T == TYPE_Builtin) || (T == TYPE_Symbol)) {
                ss << Symbol::wrap(val->value);
                if (T != TYPE_Symbol)
                    stream_type_suffix(T);
            } else {
                auto TT = dyn_cast<IntegerType>(storage_type(T).assert_ok());
                if (!TT) {
                    stream_illegal_value_type("ConstInt", T);
                } else if (TT == TYPE_Bool) {
                    ss << (bool)val->value;
                } else {
                    if (TT->issigned) {
                        ss << (int64_t)val->value;
                    } else {
                        ss << val->value;
                    }
                    if (T != TYPE_I32)
                        stream_type_suffix(T);
                }
            }
        } break;
        case VK_ConstReal: {
            auto val = cast<ConstReal>(node);
            auto T = val->get_type();
            ss << val->value;
            if (T != TYPE_F32)
                stream_type_suffix(T);
        } break;
        case VK_ConstPointer: {
            auto val = cast<ConstPointer>(node);
            auto T = val->get_type();
            if (T == TYPE_Type) {
                ss << (const Type *)val->value;
            } else if (T == TYPE_String) {
                ss << (const String *)val->value;
            } else if (T == TYPE_Closure) {
                ss << (const Closure *)val->value;
                stream_type_suffix(T);
            } else if (T == TYPE_List) {
                ss << (const List *)val->value;
            } else {
                ss << val->value;
                stream_type_suffix(T);
            }
        } break;
        case VK_ConstAggregate: {
            stream_type_prefix(node);
            auto val = cast<ConstAggregate>(node);
            ss << Style_Keyword << "ConstAggregate" << Style_None;
            for (int i = 0; i < val->values.size(); ++i) {
                walk_same_or_newline(val->values[i], depth+1, maxdepth);
            }
        } break;
        case VK_Break: {
            stream_type_prefix(node);
            auto val = cast<Break>(node);
            ss << Style_Keyword << "Break" << Style_None;
            if (newlines) {
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Repeat: {
            stream_type_prefix(node);
            auto val = cast<Repeat>(node);
            ss << Style_Keyword << "Repeat" << Style_None;
            if (newlines) {
                write_arguments(val->args, depth, maxdepth);
            }
        } break;
        case VK_Return: {
            stream_type_prefix(node);
            auto val = cast<Return>(node);
            ss << Style_Keyword << "Return" << Style_None;
            if (newlines) {
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Raise: {
            stream_type_prefix(node);
            auto val = cast<Raise>(node);
            ss << Style_Keyword << "Raise" << Style_None;
            walk_same_or_newline(val->value, depth+1, maxdepth);
        } break;
        case VK_CompileStage: {
            stream_type_prefix(node);
            auto val = cast<CompileStage>(node);
            ss << Style_Keyword << "CompileStage" << Style_None;
        } break;
        default:
            stream_type_prefix(node);
            ss << Style_Error << "<unhandled AST node type: "
                << get_value_kind_name(node->kind())
                << ">" << Style_None;
        }
    }

    void stream(const Value *node) {
        visited.clear();
        todo.push_back(node);
        while (!todo.empty()) {
            walk_todos(fmt.depth, -1);
        }
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
