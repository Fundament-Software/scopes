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

#include "stream_expr.hpp"
#include "list.hpp"
#include "anchor.hpp"
#include "syntax.hpp"
#include "type.hpp"

namespace scopes {

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

bool StreamExpr::is_nested(const Any &_e) {
    auto e = maybe_unsyntax(_e);
    if (e.type == TYPE_List) {
        auto it = e.list;
        while (it != EOL) {
            auto q = maybe_unsyntax(it->at);
            if (q.type == TYPE_List) {
                return true;
            }
            it = it->next;
        }
    }
    return false;
}

bool StreamExpr::is_list (const Any &_value) {
    auto value = maybe_unsyntax(_value);
    return value.type == TYPE_List;
}

void StreamExpr::walk(Any e, int depth, int maxdepth, bool naked) {
    bool quoted = false;

    const Anchor *anchor = nullptr;
    if (e.type == TYPE_Syntax) {
        anchor = e.syntax->anchor;
        quoted = e.syntax->quoted;
        e = e.syntax->datum;
    }

    if (naked) {
        stream_indent(depth);
    }
    if (atom_anchors) {
        stream_anchor(anchor, quoted);
    }

    if (e.type == TYPE_List) {
        if (naked && line_anchors && !atom_anchors) {
            stream_anchor(anchor, quoted);
        }

        maxdepth = maxdepth - 1;

        auto it = e.list;
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
    } else {
        if (e.type == TYPE_Symbol) {
            ss << fmt.symbol_styler(e.symbol);
            e.symbol.name()->stream(ss, SYMBOL_ESCAPE_CHARS);
            ss << Style_None;
        } else {
            ss << e;
        }
        if (naked) { ss << std::endl; }
    }
}

void StreamExpr::stream(const Any &e) {
    walk(e, fmt.depth, fmt.maxdepth, fmt.naked);
}

//------------------------------------------------------------------------------

void stream_expr(
    StyledStream &_ss, const Any &e, const StreamExprFormat &_fmt) {
    StreamExpr streamer(_ss, _fmt);
    streamer.stream(e);
}

StyledStream& operator<<(StyledStream& ost, const List *list) {
    stream_expr(ost, list, StreamExprFormat::singleline());
    return ost;
}

//------------------------------------------------------------------------------

} // namespace scopes
