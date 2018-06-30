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
#include "type.hpp"

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

//------------------------------------------------------------------------------

StreamExpr::StreamExpr(StyledStream &_ss, const StreamExprFormat &_fmt) :
    StreamAnchors(_ss), fmt(_fmt) {
    line_anchors = (fmt.anchors == StreamExprFormat::Line);
    atom_anchors = (fmt.anchors == StreamExprFormat::All);
}

void StreamExpr::stream_indent(int depth) {
    if (depth >= 1) {
        ss << Style_Comment << "    ";
        for (int i = 2; i <= depth; ++i) {
            ss << INDENT_SEP << "   ";
        }
        ss << Style_None;
    }
}

bool StreamExpr::is_nested(Value *_e) {
    auto T = try_get_const_type(_e);
    if (T == TYPE_List) {
        auto it = extract_list_constant(_e).assert_ok();
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

bool StreamExpr::is_list (Value *_value) {
    return try_get_const_type(_value) == TYPE_List;
}

void StreamExpr::walk(const Anchor *anchor, const List *l, int depth, int maxdepth, bool naked) {
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
            << Style_Comment << "<...>"
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
        walk(it->at, depth, maxdepth, false);
        it = it->next;
        offset = offset + 1;
        while (it != EOL) {
            if (is_nested(it->at)) {
                break;
            }
            ss << " ";
            walk(it->at, depth, maxdepth, false);
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
                ss << "<...>" << std::endl;
                return;
            }
            walk(value, subdepth, maxdepth, true);
            offset = offset + 1;
            it = it->next;
        }
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
            walk(it->at, depth, maxdepth, false);
            offset = offset + 1;
            it = it->next;
        }
        ss << Style_Operator << ")" << Style_None;
    }
    if (naked) { ss << std::endl; }
}

void StreamExpr::walk(Value *e, int depth, int maxdepth, bool naked) {
    auto T = try_get_const_type(e);
    if (T == TYPE_List) {
        walk(e->anchor(), extract_list_constant(e).assert_ok(), depth, maxdepth, naked);
        return;
    }

    if (naked) {
        stream_indent(depth);
    }
    const Anchor *anchor = e->anchor();
    if (atom_anchors) {
        stream_anchor(anchor);
    }

    if (T == TYPE_Symbol) {
        auto sym = extract_symbol_constant(e).assert_ok();
        ss << fmt.symbol_styler(sym);
        sym.name()->stream(ss, SYMBOL_ESCAPE_CHARS);
        ss << Style_None;
    } else {
        ss << e;
    }
    if (naked) { ss << std::endl; }
}

void StreamExpr::stream(const List *l) {
    walk(nullptr, l, fmt.depth, fmt.maxdepth, fmt.naked);
}

//------------------------------------------------------------------------------

void stream_expr(
    StyledStream &_ss, const List *l, const StreamExprFormat &_fmt) {
    StreamExpr streamer(_ss, _fmt);
    streamer.stream(l);
}

StyledStream& operator<<(StyledStream& ost, const List *list) {
    stream_expr(ost, list, StreamExprFormat::singleline());
    return ost;
}

//------------------------------------------------------------------------------

} // namespace scopes
