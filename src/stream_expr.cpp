/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "stream_expr.hpp"
#include "list.hpp"
#include "anchor.hpp"
#include "value.hpp"
#include "prover.hpp"
#include "types.hpp"
#include "gc.hpp"
#include "dyn_cast.inc"

#include <unordered_map>

namespace scopes {

static const char INDENT_SEP[] = "⁞";

//------------------------------------------------------------------------------
// EXPRESSION PRINTER
//------------------------------------------------------------------------------

Style default_symbol_styler(Symbol name) {
    if (!name.is_known())
        return Style_Symbol;
    auto val = name.known_value();
    if ((val >= KEYWORD_FIRST) && (val <= KEYWORD_LAST))
        return Style_Keyword;
    else if ((val >= FUNCTION_FIRST) && (val <= FUNCTION_LAST))
        return Style_Function;
    else if ((val >= SFXFUNCTION_FIRST) && (val <= SFXFUNCTION_LAST))
        return Style_SfxFunction;
    else if ((val >= OPERATOR_FIRST) && (val <= OPERATOR_LAST))
        return Style_Operator;
    return Style_Symbol;
}

//------------------------------------------------------------------------------

StreamExprFormat::StreamExprFormat() :
    naked(true),
    types(true),
    anchors(None),
    maxdepth(1<<30),
    maxlength(1<<30),
    symbol_styler(default_symbol_styler),
    depth(0)
{}

StreamExprFormat StreamExprFormat::debug() {
    auto fmt = StreamExprFormat();
    fmt.naked = true;
    fmt.anchors = All;
    return fmt;
}

StreamExprFormat StreamExprFormat::debug_digest() {
    auto fmt = StreamExprFormat();
    fmt.naked = true;
    fmt.anchors = Line;
    fmt.maxdepth = 5;
    fmt.maxlength = 5;
    return fmt;
}

StreamExprFormat StreamExprFormat::debug_singleline() {
    auto fmt = StreamExprFormat();
    fmt.naked = false;
    fmt.anchors = All;
    return fmt;
}

StreamExprFormat StreamExprFormat::singleline() {
    auto fmt = StreamExprFormat();
    fmt.naked = false;
    return fmt;
}

StreamExprFormat StreamExprFormat::digest() {
    auto fmt = StreamExprFormat();
    fmt.maxdepth = 5;
    fmt.maxlength = 5;
    return fmt;
}

StreamExprFormat StreamExprFormat::singleline_digest() {
    auto fmt = StreamExprFormat();
    fmt.maxdepth = 5;
    fmt.maxlength = 5;
    fmt.naked = false;
    return fmt;
}

StreamExprFormat StreamExprFormat::content() {
    auto fmt = StreamExprFormat();
    fmt.maxdepth = 3;
    fmt.maxlength = 10;
    fmt.naked = false;
    fmt.types = false;
    return fmt;
}

//------------------------------------------------------------------------------

struct StreamExpr : StreamAnchors {

StreamExprFormat fmt;
bool line_anchors;
bool atom_anchors;

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
        l = List::from(node->params[i], l);
    }
    if (node->value) {
        return LIST(
                    ((node->is_inline())?SYMBOL(KW_Inline):SYMBOL(KW_Fn)),
                    SYMBOL(node->name),
                    ConstPointer::list_from(l),
                    node->value
                    );
    } else {
        // forward declaration
        return LIST(
                    ((node->is_inline())?SYMBOL(KW_Inline):SYMBOL(KW_Fn)),
                    SYMBOL(node->name)
                    );
    }
}

HANDLER(Global) {
    auto _anchor = node.anchor();
    if (node->storage_class == SYM_Unnamed) {
        return LIST(
                    SYMBOL(SYM_Extern),
                    SYMBOL(node->name),
                    SYMBOL(OP_Colon),
                    TYPE(node->get_type()));
    }
    return node;
}

HANDLER(KeyedTemplate) {
    auto _anchor = node.anchor();
    return LIST(SYMBOL(node->key), SYMBOL(OP_Set), node->value);
}

HANDLER(CallTemplate) {
    auto _anchor = node.anchor();
    const List *l = EOL;
    int i = node->args.size();
    while (i-- > 0) {
        l = List::from(node->args[i], l);
    }
    l = List::from(node->callee, l);
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
        l = List::from(node->value, l);
    }
    int i = node->body.size();
    while (i-- > 0) {
        l = List::from(node->body[i], l);
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
    return LIST(SYMBOL(FN_VaAt), I32(node->index), node->value);
}

HANDLER(ConstAggregate) {
    auto _anchor = node.anchor();
    auto T = node->get_type();
    if (T->kind() == TK_Vector || T->kind() == TK_Array || T->kind() == TK_Tuple) {
        const List *l = EOL;
        int i = node->values.size();
        while (i-- > 0) {
            l = List::from(ref(node.anchor(), node->values[i]), l);
        }
        if (T->kind() == TK_Vector) {
            auto vt = cast<VectorType>(T);
            l = List::from(TYPE(vt->element_type), l);
            l = List::from(SYMBOL(FN_VectorOf), l);
        } else if (T->kind() == TK_Array) {
            auto at = cast<ArrayType>(T);
            l = List::from(TYPE(at->element_type), l);
            l = List::from(SYMBOL(FN_Arrayof), l);
        } else if (T->kind() == TK_Tuple) {
            l = List::from(SYMBOL(FN_TupleOf), l);
        }
        return ValueRef(_anchor, ConstPointer::list_from(l));
    } else if (T == TYPE_ValueRef) {
        auto val = (Value *)cast<ConstPointer>(node->values[0])->value;
        if (val) {
            auto loc = (const Anchor *)cast<ConstPointer>(node->values[1])->value;
            return LIST(SYMBOL(KW_ASTQuote), ref(loc, val));
        }
    }
    return node;
}

HANDLER(ConstPointer) {
    auto _anchor = node.anchor();
    if (!node->value) {
        const List *l = List::from(TYPE(node->get_type()), EOL);
        l = List::from(SYMBOL(FN_NullOf), l);
        return ValueRef(_anchor, ConstPointer::list_from(l));
    }
    if (node->get_type() == TYPE_Closure) {
        auto cl = extract_closure_constant(node).assert_ok();
        if (cl) {
            return SYMBOL(cl->func->name);
        }
        //return LIST(SYMBOL(Symbol(String::from("closure"))), cl->func);
    }
    return node;
}

#undef HANDLER
#define CASE_HANDLER(CLASS) \
    case VK_ ## CLASS: return convert_ ## CLASS(node.cast<CLASS>());
ValueRef _convert(const ValueRef &node) {
    //auto _anchor = node.anchor();
    switch(node->kind()) {
    CASE_HANDLER(CallTemplate)
    CASE_HANDLER(Template)
    CASE_HANDLER(Global)
    CASE_HANDLER(ParameterTemplate)
    CASE_HANDLER(Expression)
    CASE_HANDLER(KeyedTemplate)
    CASE_HANDLER(ExtractArgumentTemplate)
    CASE_HANDLER(ConstPointer)
    CASE_HANDLER(ConstAggregate)
    default:
        return node;
    }
}
#undef CASE_HANDLER
#undef LIST
#undef SYMBOL
#undef STRING
#undef I32
#undef TYPE

ValueRef convert(const ValueRef &node) {
    if (!node) return node;
    auto it = map.find(node.unref());
    if (it != map.end()) return ref(node.anchor(), it->second);
    auto val = _convert(node);
    map.insert({node.unref(), val});
    return val;
}

StreamExpr(StyledStream &_ss, const StreamExprFormat &_fmt) :
    StreamAnchors(_ss), fmt(_fmt) {
    line_anchors = (fmt.anchors == StreamExprFormat::Line);
    atom_anchors = (fmt.anchors == StreamExprFormat::All);
}

void stream_indent(int depth = 0) {
    if (depth >= 1) {
        ss << Style_Comment << "    ";
        for (int i = 2; i <= depth; ++i) {
            ss << INDENT_SEP << "   ";
        }
        ss << Style_None;
    }
}

static bool is_nested(const ValueRef &_e) {
    auto T = try_get_const_type(_e);
    if (T == TYPE_List) {
        auto it = extract_list_constant(_e.cast<TypedValue>()).assert_ok();
        while (it != EOL) {
            auto q = it->at;
            auto qT = try_get_const_type(q);
            if (qT == TYPE_List) {
                return true;
            }
            it = it->next;
        }
    }
    return false;
}

static bool is_list (const ValueRef &_value) {
    return try_get_const_type(_value) == TYPE_List;
}

void walk(const Anchor *anchor, const List *l, int depth, int maxdepth, bool naked, bool types) {
    if (naked) {
        stream_indent(depth);
    }
    if (anchor) {
        if (atom_anchors) {
            stream_anchor(anchor);
        }

        if (naked && line_anchors && !atom_anchors) {
            stream_anchor(anchor);
        }
    }

    maxdepth = maxdepth - 1;

    auto it = l;
    if (it == EOL) {
        ss << Style_Operator << "()" << Style_None;
        if (naked) { ss << std::endl; }
        return;
    }
    if (maxdepth == 0) {
        ss << Style_Operator << "("
            << Style_Comment << "..."
            << Style_Operator << ")"
            << Style_None;
        if (naked) { ss << std::endl; }
        return;
    }
    int offset = 0;
    // int numsublists = 0;
    if (naked) {
        if (is_list(it->at)) {
            ss << ";" << std::endl;
            goto print_sparse;
        }
    print_terse:
        walk(it->at, depth, maxdepth, false, true);
        it = it->next;
        offset = offset + 1;
        while (it != EOL) {
            if (is_nested(it->at) && (maxdepth > 1)) {
                break;
            }
            ss << " ";
            walk(it->at, depth, maxdepth, false, true);
            offset = offset + 1;
            it = it->next;
        }
        ss << std::endl;
    print_sparse:
        int subdepth = depth + 1;
        while (it != EOL) {
            auto value = it->at;
            if (!is_list(value) // not a list
                && (offset >= 1)) { // not first element in list
                stream_indent(subdepth);
                ss << "\\ ";
                goto print_terse;
            }
            if (offset >= fmt.maxlength) {
                stream_indent(subdepth);
                ss << Style_Comment << "..." << Style_None << std::endl;
                return;
            }
            walk(value, subdepth, maxdepth, true, true);
            offset = offset + 1;
            it = it->next;
        }
        return;
    } else {
        depth = depth + 1;
        ss << Style_Operator << "(" << Style_None;
        while (it != EOL) {
            if (offset > 0) {
                ss << " ";
            }
            if (offset >= fmt.maxlength) {
                ss << Style_Comment << "..." << Style_None;
                break;
            }
            walk(it->at, depth, maxdepth, false, true);
            offset = offset + 1;
            it = it->next;
        }
        ss << Style_Operator << ")" << Style_None;
    }
    if (naked) { ss << std::endl; }
}

void stream_illegal_value_type(const std::string &name) {
    ss << Style_Error << "<illegal type for " << name << ">" << Style_None;
}

void walk(const ValueRef &_e, int depth, int maxdepth, bool naked, bool types) {
    ValueRef e = convert(_e);

    const Type *T = nullptr;
    if (e) {
        T = try_get_const_type(e);
        if (T == TYPE_List) {
            walk(e.anchor(), extract_list_constant(e.cast<TypedValue>()).assert_ok(), depth, maxdepth, naked, types);
            return;
        }
    }

    if (naked) {
        stream_indent(depth);
    }
    if (e) {
        const Anchor *anchor = e.anchor();
        if (atom_anchors) {
            stream_anchor(anchor);
        }
    }

    if (!e) {
        ss << Style_Error << "?illegal?" << Style_None;
    } else if (T == TYPE_Symbol) {
        auto sym = extract_symbol_constant(e).assert_ok();
        if (sym == SYM_Unnamed) {
            ss << "#unnamed";
        } else {
            ss << fmt.symbol_styler(sym);
            sym.name()->stream(ss, SYMBOL_ESCAPE_CHARS);
            ss << Style_None;
        }
    } else {
        switch(e->kind()) {
        case VK_ConstInt: {
            auto val = e.cast<ConstInt>();
            auto T = val->get_type();
            if ((T == TYPE_Builtin) || (T == TYPE_Symbol)) {
                ss << Symbol::wrap(val->value);
            } else {
                auto TT = dyn_cast<IntegerType>(storage_type(T).assert_ok());
                if (!TT) {
                    stream_illegal_value_type("ConstInt");
                } else if (TT == TYPE_Bool) {
                    ss << (bool)val->value;
                } else {
                    if (TT->issigned) {
                        ss << (int64_t)val->value;
                    } else {
                        ss << val->value;
                    }
                }
            }
        } break;
        case VK_ConstReal: {
            auto val = e.cast<ConstReal>();
            ss << val->value;
        } break;
        case VK_ConstPointer: {
            auto val = e.cast<ConstPointer>();
            auto T = val->get_type();
            if (T == TYPE_Type) {
                ss << (const Type *)val->value;
            } else if (T == TYPE_String) {
                ss << (const String *)val->value;
            } else if (T == TYPE_Anchor) {
                ss << (const Anchor *)val->value;
            } else {
                ss << "$";
                stream_address(ss, val->value);
            }
        } break;
        case VK_ConstAggregate: {
            auto val = e.cast<ConstAggregate>();
            auto T = val->get_type();
            if (T == TYPE_Nothing) {
                ss << Style_Number << "none" << Style_None;
                break;
            }
            // pass through
        }
        default: {
            ss << Style_Keyword;
            ss << get_value_class_name(e->kind());
            ss << Style_None;
            ss << "$";
            stream_address(ss, e.unref());
        } break;
        }
        if (types && e.isa<TypedValue>()) {
            auto tv = e.cast<TypedValue>();
            if (!tv.isa<Const>() || !is_default_suffix(tv->get_type())) {
                ss << Style_Operator << ":" << Style_None;
                ss << tv->get_type();
            }
        }
    }
    if (naked) { ss << std::endl; }
}

void stream(const List *l) {
    walk(nullptr, l, fmt.depth, fmt.maxdepth, fmt.naked, fmt.types);
}

void stream(const ValueRef &node) {
    walk(node, fmt.depth, fmt.maxdepth, fmt.naked, fmt.types);
}

}; // struct StreamExpr

//------------------------------------------------------------------------------

static thread_local int _refcount = 0;

void stream_list(
    StyledStream &_ss, const List *l, const StreamListFormat &_fmt) {
    // prevent reentrance
    assert(!_refcount);
    _refcount++;
    StreamExpr streamer(_ss, _fmt);
    streamer.stream(l);
    _refcount--;
}

StyledStream& operator<<(StyledStream& ost, const List *list) {
    stream_list(ost, list, StreamExprFormat::singleline());
    return ost;
}

void stream_value(StyledStream &_ss, const ValueRef &value, const StreamValueFormat &_fmt) {
    // prevent reentrance
    assert(!_refcount);
    _refcount++;
    StreamExpr streamer(_ss, _fmt);
    streamer.stream(value);
    _refcount--;
}

//------------------------------------------------------------------------------

bool is_default_suffix(const Type *T) {
    switch (T->kind()) {
    case TK_Vector:
    case TK_Array:
    case TK_Tuple:
        return true;
    default: break;
    }
    if (T == TYPE_Anchor) return true;
    if (T == TYPE_I32) return true;
    if (T == TYPE_F32) return true;
    if (T == TYPE_Bool) return true;
    if (T == TYPE_List) return true;
    if (T == TYPE_Symbol) return true;
    if (T == TYPE_Type) return true;
    if (T == TYPE_String) return true;
    if (T == TYPE_Nothing) return true;
    return false;
}

} // namespace scopes
