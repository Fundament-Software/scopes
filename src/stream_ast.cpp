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
#include "anchor.hpp"
#include "symbol.hpp"
#include "list.hpp"
#include "prover.hpp"
#include "builtin.hpp"

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
    dependent_functions(false)
{}

StreamASTFormat StreamASTFormat::traceback() {
    auto fmt = StreamASTFormat();
    fmt.anchors = None;
    return fmt;
}

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

StreamASTFormat StreamASTFormat::content() {
    auto fmt = StreamASTFormat();
    fmt.anchors = None;
    fmt.newlines = false;
    fmt.content_only = true;
    return fmt;
}

//------------------------------------------------------------------------------

struct StreamAST : StreamAnchors {
    StreamASTFormat fmt;
    bool line_anchors;
    bool atom_anchors;
    bool newlines;
    bool dependent_functions;
    bool content_only;
    int nextid;

    std::unordered_map<Value *, int> visited;
    std::vector<ValueRef> todo;

    StreamAST(StyledStream &_ss, const StreamASTFormat &_fmt)
        : StreamAnchors(_ss), fmt(_fmt), nextid(0) {
        line_anchors = (fmt.anchors == StreamASTFormat::Line);
        atom_anchors = (fmt.anchors == StreamASTFormat::All);
        newlines = fmt.newlines;
        dependent_functions = fmt.dependent_functions;
        content_only = fmt.content_only;
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

    template<typename T>
    void write_arguments(const std::vector< TValueRef<T> > &args, int depth, int maxdepth) {
        for (int i = 0; i < args.size(); ++i) {
            auto &&arg = args[i];
            walk_same_or_newline(arg, depth+1, maxdepth);
        }
    }

    template<typename T>
    void write_arguments(const std::vector<T *> &args, int depth, int maxdepth) {
        for (int i = 0; i < args.size(); ++i) {
            auto &&arg = args[i];
            walk_same_or_newline(arg, depth+1, maxdepth);
        }
    }

    void stream_type_suffix(const Type *T) {
        if (content_only)
            return;
        ss << Style_Operator << ":" << Style_None << T;
    }

    void stream_type_suffix(const ValueRef &node) {
        if (node.isa<TypedValue>()) {
            stream_type_suffix(node.cast<TypedValue>()->get_type());
        }
    }

    void stream_block(const Block &block, int depth, int maxdepth) {
        stream_newline();
        stream_indent(depth);
        if (block.empty()) {
            ss << Style_Error << "<empty>" << Style_None;
            return;
        }
        ss << Style_Keyword << "body" << Style_None;
        ss << " " << block.depth;
        for (int i = 0; i < block.body.size(); ++i) {
            walk_newline(block.body[i], depth, maxdepth);
        }
        if (block.terminator) {
            walk_newline(block.terminator, depth, maxdepth);
        } else {
            stream_newline();
            stream_indent(depth);
            ss << Style_Error << "<terminator missing>" << Style_None;
        }
    }

    void stream_block_result(const Block &block, const ValueRef &value, int depth, int maxdepth) {
        if (!block.empty()) {
            stream_newline();
            stream_indent(depth);
            ss << Style_Keyword << "body" << Style_None;
            ss << " " << block.depth;
            for (int i = 0; i < block.body.size(); ++i) {
                walk_newline(block.body[i], depth, maxdepth);
            }
            if (block.terminator) {
                stream_newline();
                stream_indent(depth);
                walk_newline(block.terminator, depth, maxdepth);
                return;
            } else {
                stream_newline();
                stream_indent(depth);
                ss << Style_Error << "<terminator missing>" << Style_None;
            }
        }
        stream_newline();
        stream_indent(depth);
        ss << Style_Keyword << "_ " << Style_None;
        walk(value, depth, maxdepth);
    }

    bool sameline(const ValueRef &node) {
        if (!node) return true;
        switch(node->kind()) {
        case VK_ConstInt:
        case VK_ConstReal:
        case VK_ConstPointer:
        case VK_Parameter:
        case VK_Global:
        case VK_Template:
        case VK_Function:
            return true;
        default: break;
        }
        if (visited.count(node.unref()))
            return true;
        return false;
    }

    void walk_newline(const ValueRef &node, int depth, int maxdepth) {
        assert(node);
        stream_newline();
        stream_indent(depth);
        walk(node, depth, maxdepth);
    }

    void walk_same_or_newline(const ValueRef &node, int depth, int maxdepth, bool skip_space = false) {
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
            visited.erase(node.unref());
            walk(node, depth, maxdepth);
            if (newlines)
                stream_newline();
        }
    }

    void stream_illegal_value_type(const std::string &name, const Type *T) {
        ss << Style_Error << "<illegal type for " << name << ">" << Style_None;
        stream_type_suffix(T);
    }

    static bool is_annotation(const ValueRef &value) {
        return value.isa<ConstInt>()
            && value.cast<ConstInt>()->get_type() == TYPE_Builtin
            && value.cast<ConstInt>()->value == FN_Annotate;
    }

    static bool can_assign_id(const ValueRef &node) {
        return node.isa<Instruction>()
            || node.isa<LoopLabelArguments>()
            || node.isa<LoopArguments>();
     }

    // old Any printing reference:
    // https://bitbucket.org/duangle/scopes/src/dfb69b02546e859b702176c58e92a63de3461d77/src/any.cpp

    void walk(const ValueRef &node, int depth, int maxdepth) {
        if (!node) {
            ss << Style_Error << "<null>" << Style_None;
            return;
        }

        const Anchor *anchor = node.anchor();
        if (line_anchors && !anchor->is_boring()) {
            stream_anchor(anchor);
        }

        auto it = visited.find(node.unref());
        int id = -1;
        bool is_new = true;
        if (it == visited.end()) {
            if (can_assign_id(node)) {
                id = nextid++;
            }
            visited.insert({node.unref(), id});
        } else {
            is_new = false;
            id = it->second;
        }

        if (newlines && can_assign_id(node)) {
            if (is_new) {
                if (!node.isa<TypedValue>() || is_returning_value(node.cast<TypedValue>()->get_type())) {
                    ss << Style_Operator << "%" << Style_None;
                    ss << Style_Number;
                    stream_address(ss, node.unref());
                    ss << Style_None;
                    ss << " " << Style_Operator << "=" << Style_None << " ";
                }
            } else {
                ss << Style_Operator << "%" << Style_None;
                ss << Style_Number;
                stream_address(ss, node.unref());
                ss << Style_None;
                stream_type_suffix(node);
                return;
            }
        }

        switch(node->kind()) {
        case VK_Template: {
            auto val = node.cast<Template>();
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
            ss << Style_Symbol << val->name.name()->data << "λ";
            stream_address(ss, val.unref());
            ss << Style_None;
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
            auto val = node.cast<Function>();
            if (newlines && is_new && (depth == 0)) {
                ss << Style_Keyword << "Function" << Style_None;
                ss << " ";
            }
            ss << Style_Symbol << val->name.name()->data << "λ";
            stream_address(ss, val.unref());
            ss << Style_None;
            stream_type_suffix(node);
            if (newlines && is_new) {
                if (depth == 0) {
                    ss << Style_Operator << " (" << Style_None;
                    for (int i = 0; i < val->params.size(); ++i) {
                        walk_same_or_newline(val->params[i], depth+1, maxdepth);
                    }
                    ss << Style_Operator << " )" << Style_None;
                    stream_block(val->body, depth+1, maxdepth);
                } else if (dependent_functions) {
                    todo.push_back(node);
                }
            }
        } break;
        case VK_Expression: {
            auto val = node.cast<Expression>();
            ss << node;
            if (!val->scoped) {
                ss << " " << Style_Keyword << "inline" << Style_None;
            }
            for (int i = 0; i < val->body.size(); ++i) {
                walk_newline(val->body[i], depth+1, maxdepth);
            }
            walk_newline(val->value, depth+1, maxdepth);
        } break;
        case VK_SwitchTemplate: {
            auto val = node.cast<SwitchTemplate>();
            ss << node;
            if (newlines) {
                walk_same_or_newline(val->expr, depth+1, maxdepth);
                for (int i = 0; i < val->cases.size(); ++i) {
                    auto &&_case = val->cases[i];
                    stream_newline();
                    stream_indent(depth);
                    switch(_case.kind) {
                    case CK_Case: {
                        ss << Style_Keyword << "case" << Style_None;
                        walk_same_or_newline(_case.literal, depth+1, maxdepth);
                    } break;
                    case CK_Pass: {
                        ss << Style_Keyword << "pass" << Style_None;
                        walk_same_or_newline(_case.literal, depth+1, maxdepth);
                    } break;
                    case CK_Default: {
                        ss << Style_Keyword << "default" << Style_None;
                    } break;
                    default: {
                        ss << Style_Error << "???" << Style_None;
                    } break;
                    }
                    walk_same_or_newline(_case.value, depth+1, maxdepth);
                }
            }
        } break;
        case VK_Switch: {
            auto val = node.cast<Switch>();
            ss << node;
            if (newlines) {
                walk_same_or_newline(val->expr, depth+1, maxdepth);
                for (int i = 0; i < val->cases.size(); ++i) {
                    auto &_case = *val->cases[i];
                    stream_newline();
                    stream_indent(depth);
                    switch(_case.kind) {
                    case CK_Pass: {
                        ss << Style_Keyword << "pass" << Style_None;
                        walk_same_or_newline(_case.literal, depth+1, maxdepth);
                    } break;
                    case CK_Default: {
                        ss << Style_Keyword << "default" << Style_None;
                    } break;
                    default: {
                        ss << Style_Error << "???" << Style_None;
                    } break;
                    }
                    stream_block(_case.body, depth+1, maxdepth);
                }
            }
        } break;
        case VK_CondBr: {
            auto val = node.cast<CondBr>();
            ss << node;
            if (newlines) {
                walk_same_or_newline(val->cond, depth+1, maxdepth);
                stream_newline();
                stream_indent(depth);
                ss << Style_Keyword << "then" << Style_None;
                stream_block(val->then_body, depth+1, maxdepth);
                stream_newline();
                stream_indent(depth);
                ss << Style_Keyword << "else" << Style_None;
                stream_block(val->else_body, depth+1, maxdepth);
            }
        } break;
        case VK_If: {
            auto val = node.cast<If>();
            ss << node;
            if (newlines) {
                for (int i = 0; i < val->clauses.size(); ++i) {
                    auto &&expr = val->clauses[i];
                    if (expr.cond) {
                        stream_newline();
                        stream_indent(depth+1);
                        ss << Style_Keyword << "condition" << Style_None;
                        walk_same_or_newline(expr.cond, depth+2, maxdepth);
                        stream_newline();
                        stream_indent(depth+1);
                        ss << Style_Keyword << "then" << Style_None;
                    } else {
                        stream_newline();
                        stream_indent(depth+1);
                        ss << Style_Keyword << "else" << Style_None;
                    }
                    walk_same_or_newline(expr.value, depth+2, maxdepth);
                }
            }
        } break;
        case VK_Parameter: {
            auto val = node.cast<Parameter>();
            ss << Style_Symbol << val->name.name()->data << "$";
            stream_address(ss, val.unref());
            ss << Style_None;
            stream_type_suffix(node);
        } break;
        case VK_ParameterTemplate: {
            auto val = node.cast<ParameterTemplate>();
            ss << Style_Symbol << val->name.name()->data << "$";
            stream_address(ss, val.unref());
            ss << Style_None;
            if (val->is_variadic()) {
                ss << Style_Keyword << "…" << Style_None;
            }
        } break;
        case VK_Exception: {
            ss << node;
        } break;
        case VK_CallTemplate: {
            auto val = node.cast<CallTemplate>();
            if (newlines) {
                if (is_annotation(val->callee)) {
                    ss << Style_Comment << "#" << Style_None;
                    for (int i = 0; i < val->args.size(); ++i) {
                        auto &&arg = val->args[i];
                        if ((i == 0)
                            && arg.isa<ConstPointer>()
                            && arg.cast<ConstPointer>()->get_type() == TYPE_String) {
                            ss << Style_Comment << " "
                                << ((String *)arg.cast<ConstPointer>()->value)->data
                                << Style_None;
                        } else {
                            walk_newline(arg, depth+1, maxdepth);
                        }
                    }
                } else {
                    ss << node;
                    if (val->flags & CF_RawCall) {
                        ss << Style_Keyword << " rawcall" << Style_None;
                    }
                    walk_newline(val->callee, depth+1, maxdepth);
                    write_arguments(val->args, depth, maxdepth);
                }
            } else {
                ss << node;
            }
        } break;
        case VK_Call: {
            auto val = node.cast<Call>();
            if (newlines) {
                if (is_annotation(val->callee)) {
                    ss << Style_Comment << "#" << Style_None;
                    for (int i = 0; i < val->args.size(); ++i) {
                        auto &&arg = val->args[i];
                        if ((i == 0)
                            && arg.isa<ConstPointer>()
                            && arg->get_type() == TYPE_String) {
                            ss << Style_Comment << " "
                                << ((String *)arg.cast<ConstPointer>()->value)->data
                                << Style_None;
                        } else {
                            walk_same_or_newline(arg, depth+1, maxdepth);
                        }
                    }
                } else {
                    ss << node;
                    walk_same_or_newline(val->callee, depth+1, maxdepth);
                    write_arguments(val->args, depth, maxdepth);
                    if (!val->except_body.empty()) {
                        stream_block(val->except_body, depth+1, maxdepth);
                    }
                }
            } else {
                ss << node;
            }
        } break;
        case VK_ArgumentList: {
            auto val = node.cast<ArgumentList>();
            ss << node;
            if (newlines) {
                for (int i = 0; i < val->values.size(); ++i) {
                    walk_same_or_newline(val->values[i], depth+1, maxdepth);
                }
            }
        } break;
        case VK_ArgumentListTemplate: {
            auto val = node.cast<ArgumentListTemplate>();
            ss << node;
            if (newlines) {
                for (int i = 0; i < val->values.size(); ++i) {
                    walk_same_or_newline(val->values[i], depth+1, maxdepth);
                }
            }
        } break;
        case VK_ExtractArgument: {
            auto val = node.cast<ExtractArgument>();
            ss << node;
            if (newlines) {
                ss << " " << val->index;
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_PureCast: {
            auto val = node.cast<PureCast>();
            ss << node;
            if (newlines) {
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_ExtractArgumentTemplate: {
            auto val = node.cast<ExtractArgumentTemplate>();
            ss << node;
            if (newlines) {
                ss << " " << val->index;
                if (val->vararg) {
                    ss << Style_Keyword << "…" << Style_None;
                }
                walk_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Keyed: {
            auto val = node.cast<Keyed>();
            ss << node;
            if (newlines) {
                ss << " " << val->key;
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_KeyedTemplate: {
            auto val = node.cast<KeyedTemplate>();
            ss << node;
            if (newlines) {
                ss << " " << val->key;
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Loop: {
            auto val = node.cast<Loop>();
            ss << node;
            if (newlines) {
                walk_same_or_newline(val->args, depth+1, maxdepth);
                ss << " " << Style_Operator << "=" << Style_None;
                walk_same_or_newline(val->init, depth+1, maxdepth);
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_LoopLabel: {
            auto val = node.cast<LoopLabel>();
            ss << node;
            if (newlines) {
                walk_same_or_newline(val->args, depth+1, maxdepth);
                ss << " " << Style_Operator << "=" << Style_None;
                for (int i = 0; i < val->init.size(); ++i) {
                    walk_same_or_newline(val->init[i], depth+1, maxdepth);
                }
                stream_block(val->body, depth+1, maxdepth);
            }
        } break;
        case VK_LabelTemplate: {
            auto val = node.cast<LabelTemplate>();
            ss << node;
            ss << " ";
            ss << Style_Keyword << get_label_kind_name(val->label_kind);
            ss << Style_None << " ";
            ss << Style_Symbol << val->name.name()->data;
            ss << Style_None;
            if (newlines) {
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Label: {
            auto val = node.cast<Label>();
            ss << node;
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
            ss << Style_Symbol << val->name.name()->data;
            ss << Style_None;
            if (newlines) {
                stream_block(val->body, depth+1, maxdepth);
            }
        } break;
        case VK_Global: {
            auto val = node.cast<Global>();
            auto T = val->get_type();
            ss << val->name;
            stream_type_suffix(T);
        } break;
        case VK_ConstInt: {
            auto val = node.cast<ConstInt>();
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
            auto val = node.cast<ConstReal>();
            auto T = val->get_type();
            ss << val->value;
            if (T != TYPE_F32)
                stream_type_suffix(T);
        } break;
        case VK_ConstPointer: {
            auto val = node.cast<ConstPointer>();
            auto T = val->get_type();
            if (T == TYPE_Type) {
                ss << (const Type *)val->value;
            } else if (T == TYPE_String) {
                ss << (const String *)val->value;
                #if 0
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
                #endif
            #if 0
            } else if (T == TYPE_ValueRef) {
                stream_ast(ss, ref(val.anchor(), (Value *)val->value),
                    StreamASTFormat::singleline());
                stream_type_suffix(T);
            #endif
            } else {
                ss << "$";
                stream_address(ss, val->value);
                stream_type_suffix(T);
            }
        } break;
        case VK_ConstAggregate: {
            auto val = node.cast<ConstAggregate>();
            ss << node;
            for (int i = 0; i < val->values.size(); ++i) {
                walk_same_or_newline(
                    ref(val.anchor(), val->values[i]), depth+1, maxdepth);
            }
        } break;
        case VK_Break: {
            auto val = node.cast<Break>();
            ss << node;
            if (newlines) {
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_Repeat: {
            auto val = node.cast<Repeat>();
            ss << node;
            if (newlines) {
                for (int i = 0; i < val->values.size(); ++i) {
                    walk_same_or_newline(val->values[i], depth+1, maxdepth);
                }
            }
        } break;
        case VK_Return: {
            auto val = node.cast<Return>();
            ss << node;
            if (newlines) {
                for (int i = 0; i < val->values.size(); ++i) {
                    walk_same_or_newline(val->values[i], depth+1, maxdepth);
                }
            }
        } break;
        case VK_Merge: {
            auto val = node.cast<Merge>();
            ss << node;
            ss << " ";
            ss << Style_Symbol << val->label->name.name()->data << "@";
            stream_address(ss, val->label.unref());
            ss << Style_None;
            if (newlines) {
                for (int i = 0; i < val->values.size(); ++i) {
                    walk_same_or_newline(val->values[i], depth+1, maxdepth);
                }
            }
        } break;
        case VK_MergeTemplate: {
            auto val = node.cast<MergeTemplate>();
            ss << node;
            ss << " ";
            ss << Style_Symbol << val->label->name.name()->data << "@";
            stream_address(ss, val->label.unref());
            ss << Style_None;
            if (newlines) {
                walk_same_or_newline(val->value, depth+1, maxdepth);
            }
        } break;
        case VK_LoopLabelArguments: {
            auto val = node.cast<LoopLabelArguments>();
            ss << node;
            ss << " ";
            ss << Style_Symbol << "@";
            stream_address(ss, val->loop.unref());
            ss << Style_None;
        } break;
        case VK_LoopArguments: {
            auto val = node.cast<LoopArguments>();
            ss << node;
            ss << " ";
            ss << Style_Symbol << "@";
            stream_address(ss, val->loop.unref());
            ss << Style_None;
        } break;
        case VK_Raise: {
            auto val = node.cast<Raise>();
            ss << node;
            for (int i = 0; i < val->values.size(); ++i) {
                walk_same_or_newline(val->values[i], depth+1, maxdepth);
            }
        } break;
        case VK_Quote: {
            auto val = node.cast<Quote>();
            ss << node;
            walk_same_or_newline(val->value, depth+1, maxdepth);
        } break;
        case VK_Unquote: {
            auto val = node.cast<Unquote>();
            ss << node;
            walk_same_or_newline(val->value, depth+1, maxdepth);
        } break;
        case VK_CompileStage: {
            //auto val = node.cast<CompileStage>();
            ss << node;
        } break;
        default:
            ss << Style_Error << "<unhandled AST node type: "
                << get_value_kind_name(node->kind())
                << ">" << Style_None;
            stream_type_suffix(node);
        }
    }

    void stream(const ValueRef &node) {
        visited.clear();
        todo.push_back(node);
        while (!todo.empty()) {
            walk_todos(fmt.depth, -1);
        }
    }
};

//------------------------------------------------------------------------------

void stream_ast(
    StyledStream &_ss, const ValueRef &node, const StreamASTFormat &_fmt) {
    StreamAST streamer(_ss, _fmt);
    streamer.stream(node);
}

//------------------------------------------------------------------------------

struct Value2ListConverter {

typedef std::unordered_map<Value *, ValueRef> Map;

Map map;

static Symbol remap_unnamed(Symbol sym) {
    return (sym == SYM_Unnamed)?Symbol(String::from("#unnamed")):sym;
}

#define LIST(...) ValueRef(_anchor, ConstPointer::list_from(List::from_arglist( __VA_ARGS__ )))
#define SYMBOL(NAME) ValueRef(_anchor, ConstInt::symbol_from(remap_unnamed(NAME)))
#define STRING(NAME) ValueRef(_anchor, ConstPointer::string_from((NAME)))
#define I32(NAME) ValueRef(_anchor, ConstInt::from(TYPE_I32, (NAME)))
#define TYPE(NAME) ValueRef(_anchor, ConstPointer::type_from((NAME)))

#define HANDLER(CLASS) \
ValueRef convert_ ## CLASS(const CLASS ## Ref &node)

HANDLER(ParameterTemplate) {
    auto _anchor = node.anchor();
    if (node->name == SYM_Unnamed) {
        StyledString ss = StyledString::plain();
        stream_address(ss.out, node.unref());
        if (node->is_variadic()) {
            ss.out << "...";
        }
        return SYMBOL(Symbol(ss.str()));
    } else {
        return SYMBOL(node->name);
    }
}

HANDLER(Template) {
    auto _anchor = node.anchor();
    const List *l = EOL;
    int i = node->params.size();
    while (i-- > 0) {
        l = List::from(convert(node->params[i]), l);
    }
    if (node->value) {
        return LIST(
                    ((node->is_inline())?SYMBOL(KW_Inline):SYMBOL(KW_Fn)),
                    SYMBOL(node->name),
                    ConstPointer::list_from(l),
                    convert(node->value)
                    );
    } else {
        // forward declaration
        return LIST(
                    ((node->is_inline())?SYMBOL(KW_Inline):SYMBOL(KW_Fn)),
                    SYMBOL(node->name)
                    );
    }
}

HANDLER(Global) { return node; }

HANDLER(KeyedTemplate) {
    auto _anchor = node.anchor();
    return LIST(SYMBOL(node->key), SYMBOL(OP_Set), convert(node->value));
}

HANDLER(CallTemplate) {
    auto _anchor = node.anchor();
    const List *l = EOL;
    int i = node->args.size();
    while (i-- > 0) {
        l = List::from(convert(node->args[i]), l);
    }
    l = List::from(convert(node->callee), l);
    if (node->is_rawcall()) {
        l = List::from(SYMBOL(KW_RawCall), l);
    } else {       
        //if (node->callee.isa<Global>())
        //    goto skip;
        if (node->callee.isa<Const>()) {
            const Type *T = node->callee.cast<TypedValue>()->get_type();
            if (T == TYPE_Builtin)
                goto skip;
            if (T == TYPE_Closure)
                goto skip;
        }
        l = List::from(SYMBOL(KW_Call), l);
    }
skip:
    return ValueRef(_anchor, ConstPointer::list_from(l));
}

HANDLER(Expression) {
    auto _anchor = node.anchor();
    const List *l = EOL;
    if (node->value) {
        l = List::from(convert(node->value), l);
    }    
    int i = node->body.size();
    while (i-- > 0) {
        l = List::from(convert(node->body[i]), l);
    }
    if (node->scoped) {
        l = List::from(SYMBOL(KW_Do), l);
    } else {
        l = List::from(SYMBOL(KW_DoIn), l);
    }
    return ValueRef(_anchor, ConstPointer::list_from(l));
}

HANDLER(ExtractArgumentTemplate) {
    auto _anchor = node.anchor();
    return LIST(SYMBOL(FN_VaAt), I32(node->index), convert(node->value));
}

HANDLER(ConstAggregate) {
    auto _anchor = node.anchor();
    if (node->get_type() == TYPE_ValueRef) {
        auto val = (Value *)cast<ConstPointer>(node->values[0])->value;
        auto loc = (const Anchor *)cast<ConstPointer>(node->values[1])->value;
        return LIST(SYMBOL(KW_ASTQuote), ref(loc, val));
    }
    return node;
}

HANDLER(ConstInt) {
    auto _anchor = node.anchor();
    if (node->get_type() == TYPE_Builtin) {
        return SYMBOL(extract_builtin_constant(node).assert_ok().name());
    }
    return node;
}

HANDLER(ConstPointer) {
    auto _anchor = node.anchor();
    if (node->get_type() == TYPE_Closure) {
        auto cl = extract_closure_constant(node).assert_ok();
        return SYMBOL(cl->func->name);
        //return LIST(SYMBOL(Symbol(String::from("closure"))), convert(cl->func));
    }
    return node;
}

#undef HANDLER
#define CASE_HANDLER(CLASS) \
    case VK_ ## CLASS: return convert_ ## CLASS(node.cast<CLASS>());
ValueRef _convert(const ValueRef &node) {
    auto _anchor = node.anchor();
    switch(node->kind()) {
    CASE_HANDLER(CallTemplate)
    CASE_HANDLER(Template)
    CASE_HANDLER(Global)
    CASE_HANDLER(ParameterTemplate)
    CASE_HANDLER(Expression)
    CASE_HANDLER(KeyedTemplate)
    CASE_HANDLER(ExtractArgumentTemplate)
    CASE_HANDLER(ConstPointer)
    CASE_HANDLER(ConstInt)
    CASE_HANDLER(ConstAggregate)
    default:
        return node;
    }
}
#undef CASE_HANDLER

ValueRef convert(const ValueRef &node) {
    auto it = map.find(node.unref());
    if (it != map.end()) return ref(node.anchor(), it->second);
    auto val = _convert(node);
    map.insert({node.unref(), val});
    return val;
}

};

const List *ast_to_list(const ValueRef &node) {
    Value2ListConverter converter;
    auto val = converter.convert(node);
    if (val.isa<ConstPointer>() && (val.cast<ConstPointer>()->get_type() == TYPE_List)) {
        return extract_list_constant(val).assert_ok();
    } else {
        return List::from({val});
    }
}

//------------------------------------------------------------------------------

} // namespace scopes
