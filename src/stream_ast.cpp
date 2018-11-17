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
#include "closure.hpp"

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
    newlines(true),
    data_dependency(true),
    dependent_functions(false)
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
    bool data_dependency;
    bool dependent_functions;
    int nextid;

    std::unordered_map<const Value *, int> visited;
    std::vector<const Value *> todo;

    StreamAST(StyledStream &_ss, const StreamASTFormat &_fmt)
        : StreamAnchors(_ss), fmt(_fmt), nextid(0) {
        line_anchors = (fmt.anchors == StreamASTFormat::Line);
        atom_anchors = (fmt.anchors == StreamASTFormat::All);
        newlines = fmt.newlines;
        data_dependency = fmt.data_dependency;
        dependent_functions = fmt.dependent_functions;
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

    void stream_type_suffix(const Value *node) {
        if (node->is_typed()) {
            stream_type_suffix(node->get_type());
        }
    }

    void stream_block(const Block &block, int depth, int maxdepth) {
        if (!block.empty())
            return;
        stream_newline();
        stream_indent(depth);
        ss << Style_Keyword << "body" << Style_None;
        ss << " " << block.depth;
        for (int i = 0; i < block.body.size(); ++i) {
            walk_newline(block.body[i], depth+1, maxdepth);
        }
        stream_annotations(depth, block.annotations);
        if (block.terminator) {
            stream_newline();
            stream_indent(depth);
            ss << Style_Keyword << "---" << Style_None;
            walk_newline(block.terminator, depth+1, maxdepth);
        }
    }

    void stream_block_result(const Block &block, const Value *value, int depth, int maxdepth) {
        if (!block.empty()) {
            stream_newline();
            stream_indent(depth);
            ss << Style_Keyword << "body" << Style_None;
            ss << " " << block.depth;
            for (int i = 0; i < block.body.size(); ++i) {
                walk_newline(block.body[i], depth, maxdepth);
            }
            stream_annotations(depth, block.annotations);
            if (block.terminator) {
                stream_newline();
                stream_indent(depth);
                ss << Style_Keyword << "---" << Style_None;
                walk_newline(block.terminator, depth, maxdepth);
                return;
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

    void stream_annotations(int depth, const Strings &annotations) {
        for (auto entry : annotations) {
            stream_newline();
            stream_indent(depth);
            ss << "# " << entry->data;
        }
    }

    void walk_newline(const Value *node, int depth, int maxdepth) {
        stream_annotations(depth, node->annotations);
        stream_newline();
        stream_indent(depth);
        walk(node, depth, maxdepth);
    }

    void walk_same_or_newline(const Value *node, int depth, int maxdepth, bool skip_space = false) {
        if (sameline(node)) {
            if (!skip_space) {
                ss << " ";
            }
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

    void stream_depends(const Depends &deps) {
        auto &&args = deps.args;
        auto &&kinds = deps.kinds;
        ss << Style_Operator << "[" << Style_None;
        for (int i = 0; i < args.size(); ++i) {
            if (i > 0)
                ss << " ";
            auto &&arg = args[i];
            auto kind = kinds[i];
            bool first = true;
            switch(kind) {
            case DK_Unique: ss << Style_Operator << "†" << Style_None; break;
            case DK_Borrowed: ss << Style_Operator << "%" << Style_None; break;
            case DK_Conflicted: ss << Style_Error << "!" << Style_None; break;
            case DK_Undefined: ss << Style_Error << "?" << Style_None; break;
            }
            for (auto &&dep : arg) {
                if (!first) {
                    ss << Style_Operator << "|" << Style_None;
                }
                first = false;
                ValueIndex val = dep;
                if (isa<Parameter>(val.value)) {
                    walk_same_or_newline(val.value, 0, 0, true);
                } else if (isa<Pure>(val.value)) {
                    ss << Style_Keyword << get_value_class_name(val.value->kind()) << Style_None;
                } else {
                    auto it = visited.find(val.value);
                    if (it == visited.end()) {
                        ss << Style_Error << "?" << Style_None;
                    } else {
                        int id = it->second;
                        ss << Style_Operator << "%" << id << Style_None;
                    }
                }
                if (val.index != 0)
                    ss << Style_Operator << "@" << Style_None << val.index;
            }
        }
        ss << Style_Operator << "]" << Style_None;
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
                if (data_dependency) {
                    stream_depends(node->deps);
                    ss << " ";
                }
            } else {
                stream_type_suffix(node);
                return;
            }
        }

        switch(node->kind()) {
        case VK_Template: {
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
            stream_type_suffix(node);
            if (newlines && is_new) {
                if (depth == 0) {
                    ss << Style_Operator << " (" << Style_None;
                    for (int i = 0; i < val->params.size(); ++i) {
                        walk_same_or_newline(val->params[i], depth+1, maxdepth);
                    }
                    ss << Style_Operator << " )" << Style_None;
                    if (val->value) {
                        walk_newline(val->value, depth+1, maxdepth);
                    }
                } else if (dependent_functions) {
                    todo.push_back(node);
                }
            }
        } break;
        case VK_Function: {
            auto val = cast<Function>(node);
            if (newlines && is_new && (depth == 0)) {
                ss << Style_Keyword << "Function" << Style_None;
                ss << " ";
                if (data_dependency) {
                    stream_depends(val->deps);
                    ss << " ";
                }
            }
            ss << Style_Symbol << val->name.name()->data
                << "λ" << (void *)val << Style_None;
            stream_type_suffix(node);
            if (newlines && is_new) {
                if (depth == 0) {
                    ss << Style_Operator << " (" << Style_None;
                    for (int i = 0; i < val->params.size(); ++i) {
                        walk_same_or_newline(val->params[i], depth+1, maxdepth);
                    }
                    ss << Style_Operator << " )" << Style_None;
                    stream_annotations(depth+1, val->annotations);
                    stream_block_result(val->body, val->value, depth+1, maxdepth);
                } else if (dependent_functions) {
                    todo.push_back(node);
                }
            }
        } break;
        case VK_Expression: {
            auto val = cast<Expression>(node);
            ss << Style_Keyword << "Expression" << Style_None;
            stream_type_suffix(node);
            if (!val->scoped) {
                ss << " " << Style_Keyword << "inline" << Style_None;
            }
            for (int i = 0; i < val->body.size(); ++i) {
                walk_newline(val->body[i], depth+1, maxdepth);
            }
            walk_newline(val->value, depth+1, maxdepth);
        } break;
        case VK_Switch: {
            auto val = cast<Switch>(node);
            ss << Style_Keyword << "Switch" << Style_None;
            stream_type_suffix(node);
            if (newlines) {
                walk_same_or_newline(val->expr, depth+1, maxdepth);
                for (int i = 0; i < val->cases.size(); ++i) {
                    auto &&_case = val->cases[i];
                    stream_newline();
                    stream_indent(depth+1);
                    if (_case.literal) {
                        ss << Style_Keyword << "case" << Style_None;
                        walk_same_or_newline(_case.literal, depth+2, maxdepth);
                    } else {
                        ss << Style_Keyword << "default" << Style_None;
                    }
                    stream_block_result(_case.body, _case.value, depth+2, maxdepth);
                }
            }
        } break;
        case VK_If: {
            auto val = cast<If>(node);
            ss << Style_Keyword << "If" << Style_None;
            stream_type_suffix(node);
            if (newlines) {
                for (int i = 0; i < val->clauses.size(); ++i) {
                    auto &&expr = val->clauses[i];
                    if (expr.cond) {
                        stream_newline();
                        stream_indent(depth+1);
                        ss << Style_Keyword << "condition" << Style_None;
                        stream_block_result(expr.cond_body, expr.cond, depth+2, maxdepth);
                        stream_newline();
                        stream_indent(depth+1);
                        ss << Style_Keyword << "then" << Style_None;
                    } else {
                        stream_newline();
                        stream_indent(depth+1);
                        ss << Style_Keyword << "else" << Style_None;
                    }
                    stream_block_result(expr.body, expr.value, depth+2, maxdepth);
                }
            }
        } break;
        case VK_Parameter: {
            auto val = cast<Parameter>(node);
            ss << Style_Symbol << val->name.name()->data
                << "$" << (void *)val << Style_None;
            if (val->is_variadic()) {
                ss << Style_Keyword << "…" << Style_None;
            }
            stream_type_suffix(node);
        } break;
        case VK_Call: {
            auto val = cast<Call>(node);
            ss << Style_Keyword << "Call" << Style_None;
            stream_type_suffix(node);
            if (newlines) {
                if (val->flags & CF_RawCall) {
                    ss << Style_Keyword << " rawcall" << Style_None;
                }
                if (val->except_label) {
                    ss << Style_Keyword << " except=" << Style_None;
                    ss << Style_Symbol << val->except_label->name.name()->data
                        << "@" << (void *)val->except_label << Style_None;
                }
                walk_same_or_newline(val->callee, depth+1, maxdepth);
                write_arguments(val->args, depth, maxdepth);
            }
        } break;
        case VK_ArgumentList: {
            auto val = cast<ArgumentList>(node);
            ss << Style_Keyword << "ArgumentList" << Style_None;
            stream_type_suffix(node);
            if (newlines) {
                for (int i = 0; i < val->values.size(); ++i) {
                    walk_same_or_newline(val->values[i], depth+1, maxdepth);
                }
            }
        } break;
        case VK_ExtractArgument: {
            auto val = cast<ExtractArgument>(node);
            ss << Style_Keyword << "ExtractArgument" << Style_None;
            stream_type_suffix(node);
            if (newlines) {
                ss << " " << val->index;
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Keyed: {
            auto val = cast<Keyed>(node);
            ss << Style_Keyword << "Keyed" << Style_None;
            stream_type_suffix(node);
            if (newlines) {
                ss << " " << val->key;
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Loop: {
            auto val = cast<Loop>(node);
            ss << Style_Keyword << "Loop" << Style_None;
            stream_type_suffix(node);
            if (newlines) {
                ss << " " << Style_Operator << "=" << Style_None;
                walk_same_or_newline(val->init, depth+1, maxdepth);
                stream_block_result(val->body, val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Label: {
            auto val = cast<Label>(node);
            ss << Style_Keyword << "Label" << Style_None;
            stream_type_suffix(node);
            ss << " ";
            switch(val->label_kind) {
            #define T(NAME, BNAME) \
                case NAME: ss << Style_Keyword << BNAME; break;
            SCOPES_LABEL_KIND()
            #undef T
            default:
                ss << Style_Error << "?kind";
            }
            ss << Style_None << " ";
            ss << Style_Symbol << val->name.name()->data
                << "@" << (void *)val << Style_None;
            if (newlines) {
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
                auto cl = (const Closure *)val->value;
                ss << cl;
                stream_type_suffix(T);
                if (newlines && !visited.count(cl->func)) {
                    visited.insert({cl->func, -1});
                    if (dependent_functions)
                        todo.push_back(cl->func);
                }
            /*} else if (T == TYPE_List) {
                ss << (const List *)val->value;*/
            } else if (T == TYPE_Value) {
                stream_ast(ss, (Value *)val->value, StreamASTFormat::singleline());
                stream_type_suffix(T);
            } else {
                ss << val->value;
                stream_type_suffix(T);
            }
        } break;
        case VK_ConstAggregate: {
            auto val = cast<ConstAggregate>(node);
            ss << Style_Keyword << "ConstAggregate" << Style_None;
            stream_type_suffix(node);
            for (int i = 0; i < val->values.size(); ++i) {
                walk_same_or_newline(val->values[i], depth+1, maxdepth);
            }
        } break;
        case VK_Break: {
            auto val = cast<Break>(node);
            ss << Style_Keyword << "Break" << Style_None;
            stream_type_suffix(node);
            if (newlines) {
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Repeat: {
            auto val = cast<Repeat>(node);
            ss << Style_Keyword << "Repeat" << Style_None;
            stream_type_suffix(node);
            if (newlines) {
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Return: {
            auto val = cast<Return>(node);
            ss << Style_Keyword << "Return" << Style_None;
            stream_type_suffix(node);
            if (newlines) {
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Merge: {
            auto val = cast<Merge>(node);
            ss << Style_Keyword << "Merge" << Style_None;
            stream_type_suffix(node);
            ss << " ";
            ss << Style_Symbol << val->label->name.name()->data
                << "@" << (void *)val->label << Style_None;
            if (newlines) {
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Raise: {
            auto val = cast<Raise>(node);
            ss << Style_Keyword << "Raise" << Style_None;
            stream_type_suffix(node);
            walk_same_or_newline(val->value, depth+1, maxdepth);
        } break;
        case VK_Quote: {
            auto val = cast<Quote>(node);
            ss << Style_Keyword << "Quote" << Style_None;
            stream_type_suffix(node);
            walk_same_or_newline(val->value, depth+1, maxdepth);
        } break;
        case VK_Unquote: {
            auto val = cast<Unquote>(node);
            ss << Style_Keyword << "Unquote" << Style_None;
            stream_type_suffix(node);
            walk_same_or_newline(val->value, depth+1, maxdepth);
        } break;
        case VK_CompileStage: {
            auto val = cast<CompileStage>(node);
            ss << Style_Keyword << "CompileStage" << Style_None;
            stream_type_suffix(node);
        } break;
        default:
            ss << Style_Error << "<unhandled AST node type: "
                << get_value_kind_name(node->kind())
                << ">" << Style_None;
            stream_type_suffix(node);
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
