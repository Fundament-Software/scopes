/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#include "lexerparser.hpp"
#include "error.hpp"
#include "source_file.hpp"
#include "anchor.hpp"
#include "type.hpp"
#include "list.hpp"
#include "syntax.hpp"

#include "scopes.h"

#include <string.h>
#include <assert.h>

#pragma GCC diagnostic ignored "-Wvla-extension"

namespace scopes {

enum {
    RN_Invalid = 0,
    RN_Untyped = 1,
    RN_Typed = 2,
};

//------------------------------------------------------------------------------
// S-EXPR LEXER & PARSER
//------------------------------------------------------------------------------

const char *get_token_name(Token tok) {
    switch(tok) {
#define T(NAME, VALUE) case tok_ ## NAME: return #NAME;
    B_TOKENS()
#undef T
    }
}

template<unsigned N>
bool LexerParser::is_suffix(const char (&str)[N]) {
    if (string_len != (N - 1)) {
        return false;
    }
    return !strncmp(string, str, N - 1);
}

void LexerParser::verify_good_taste(char c) {
    if (c == '\t') {
        location_error(String::from("please use spaces instead of tabs."));
    }
}

LexerParser::LexerParser(SourceFile *_file, size_t offset, size_t length) :
        value(none) {
    file = _file;
    input_stream = file->strptr() + offset;
    token = tok_eof;
    base_offset = (int)offset;
    if (length) {
        eof = input_stream + length;
    } else {
        eof = file->strptr() + file->length;
    }
    cursor = next_cursor = input_stream;
    lineno = next_lineno = 1;
    line = next_line = input_stream;
}

int LexerParser::offset() {
    return base_offset + (cursor - input_stream);
}

int LexerParser::column() {
    return cursor - line + 1;
}

int LexerParser::next_column() {
    return next_cursor - next_line + 1;
}

const Anchor *LexerParser::anchor() {
    return Anchor::from(file, lineno, column(), offset());
}

char LexerParser::next() {
    char c = next_cursor[0];
    verify_good_taste(c);
    next_cursor = next_cursor + 1;
    return c;
}

size_t LexerParser::chars_left() {
    return eof - next_cursor;
}

bool LexerParser::is_eof() {
    return next_cursor == eof;
}

void LexerParser::newline() {
    next_lineno = next_lineno + 1;
    next_line = next_cursor;
}

void LexerParser::select_string() {
    string = cursor;
    string_len = next_cursor - cursor;
}

void LexerParser::read_single_symbol() {
    select_string();
}

void LexerParser::read_symbol() {
    bool escape = false;
    while (true) {
        if (is_eof()) {
            break;
        }
        char c = next();
        if (escape) {
            if (c == '\n') {
                newline();
            }
            escape = false;
        } else if (c == '\\') {
            escape = true;
        } else if (isspace(c) || strchr(TOKEN_TERMINATORS, c)) {
            next_cursor = next_cursor - 1;
            break;
        }
    }
    select_string();
}

void LexerParser::read_string(char terminator) {
    bool escape = false;
    while (true) {
        if (is_eof()) {
            location_error(String::from("unterminated sequence"));
            break;
        }
        char c = next();
        if (c == '\n') {
            // 0.10
            //newline();
            // 0.11
            location_error(String::from("unexpected line break in string"));
            break;
        }
        if (escape) {
            escape = false;
        } else if (c == '\\') {
            escape = true;
        } else if (c == terminator) {
            break;
        }
    }
    select_string();
}

void LexerParser::read_block(int indent) {
    int col = column() + indent;
    while (true) {
        if (is_eof()) {
            break;
        }
        int next_col = next_column();
        char c = next();
        if (c == '\n') {
            newline();
        } else if (!isspace(c) && (next_col <= col)) {
            next_cursor = next_cursor - 1;
            break;
        }
    }
}

void LexerParser::read_block_string() {
    next();next();next();
    read_block(3);
    select_string();
}

void LexerParser::read_comment() {
    read_block(0);
}

template<typename T>
int LexerParser::read_integer(void (*strton)(T *, const char*, char**)) {
    char *cend;
    errno = 0;
    T srcval;
    strton(&srcval, cursor, &cend);
    if ((cend == cursor)
        || (errno == ERANGE)
        || (cend > eof)) {
        return RN_Invalid;
    }
    value = Any(srcval);
    next_cursor = cend;
    if ((cend != eof)
        && (!isspace(*cend))
        && (!strchr(TOKEN_TERMINATORS, *cend))) {
        if (strchr(".e", *cend)) return false;
        // suffix
        auto _lineno = lineno; auto _line = line; auto _cursor = cursor;
        next_token();
        read_symbol();
        lineno = _lineno; line = _line; cursor = _cursor;
        return RN_Typed;
    } else {
        return RN_Untyped;
    }
}

template<typename T>
int LexerParser::read_real(void (*strton)(T *, const char*, char**, int)) {
    char *cend;
    errno = 0;
    T srcval;
    strton(&srcval, cursor, &cend, 0);
    if ((cend == cursor)
        || (errno == ERANGE)
        || (cend > eof)) {
        return RN_Invalid;
    }
    value = Any(srcval);
    next_cursor = cend;
    if ((cend != eof)
        && (!isspace(*cend))
        && (!strchr(TOKEN_TERMINATORS, *cend))) {
        // suffix
        auto _lineno = lineno; auto _line = line; auto _cursor = cursor;
        next_token();
        read_symbol();
        lineno = _lineno; line = _line; cursor = _cursor;
        return RN_Typed;
    } else {
        return RN_Untyped;
    }
}

bool LexerParser::has_suffix() const {
    return (string_len >= 1) && (string[0] == ':');
}

bool LexerParser::select_integer_suffix() {
    if (!has_suffix())
        return false;
    if (is_suffix(":i8")) { value = Any(value.i8); return true; }
    else if (is_suffix(":i16")) { value = Any(value.i16); return true; }
    else if (is_suffix(":i32")) { value = Any(value.i32); return true; }
    else if (is_suffix(":i64")) { value = Any(value.i64); return true; }
    else if (is_suffix(":u8")) { value = Any(value.u8); return true; }
    else if (is_suffix(":u16")) { value = Any(value.u16); return true; }
    else if (is_suffix(":u32")) { value = Any(value.u32); return true; }
    else if (is_suffix(":u64")) { value = Any(value.u64); return true; }
    //else if (is_suffix(":isize")) { value = Any(value.i64); return true; }
    else if (is_suffix(":usize")) { value = Any(value.u64); value.type = TYPE_USize; return true; }
    else {
        StyledString ss;
        ss.out << "invalid suffix for integer literal: "
            << String::from(string, string_len);
        location_error(ss.str());
        return false;
    }
}

bool LexerParser::select_real_suffix() {
    if (!has_suffix())
        return false;
    if (is_suffix(":f32")) { value = Any((float)value.f64); return true; }
    else if (is_suffix(":f64")) { value = Any(value.f64); return true; }
    else {
        StyledString ss;
        ss.out << "invalid suffix for floating point literal: "
            << String::from(string, string_len);
        location_error(ss.str());
        return false;
    }
}

bool LexerParser::read_int64() {
    switch(read_integer(scopes_strtoll)) {
    case RN_Invalid: return false;
    case RN_Untyped:
        if ((value.i64 >= -0x80000000ll) && (value.i64 <= 0x7fffffffll)) {
            value = Any(int32_t(value.i64));
        } else if ((value.i64 >= 0x80000000ll) && (value.i64 <= 0xffffffffll)) {
            value = Any(uint32_t(value.i64));
        }
        return true;
    case RN_Typed:
        return select_integer_suffix();
    default: assert(false); return false;
    }
}
bool LexerParser::read_uint64() {
    switch(read_integer(scopes_strtoull)) {
    case RN_Invalid: return false;
    case RN_Untyped:
        return true;
    case RN_Typed:
        return select_integer_suffix();
    default: assert(false); return false;
    }
}
bool LexerParser::read_real64() {
    switch(read_real(scopes_strtod)) {
    case RN_Invalid: return false;
    case RN_Untyped:
        value = Any(float(value.f64));
        return true;
    case RN_Typed:
        return select_real_suffix();
    default: assert(false); return false;
    }
}

void LexerParser::next_token() {
    lineno = next_lineno;
    line = next_line;
    cursor = next_cursor;
    set_active_anchor(anchor());
}

Token LexerParser::read_token() {
    char c;
skip:
    next_token();
    if (is_eof()) { token = tok_eof; goto done; }
    c = next();
    if (c == '\n') { newline(); }
    if (isspace(c)) { goto skip; }
    if (c == '#') { read_comment(); goto skip; }
    else if (c == '(') { token = tok_open; }
    else if (c == ')') { token = tok_close; }
    else if (c == '[') { token = tok_square_open; }
    else if (c == ']') { token = tok_square_close; }
    else if (c == '{') { token = tok_curly_open; }
    else if (c == '}') { token = tok_curly_close; }
    else if (c == '\\') { token = tok_escape; }
    else if (c == '"') {
        if ((chars_left() >= 3)
            && (next_cursor[0] == '"')
            && (next_cursor[1] == '"')
            && (next_cursor[2] == '"')) {
            token = tok_block_string;
            read_block_string();
        } else {
            token = tok_string;
            read_string(c);
        }
    }
    else if (c == ';') { token = tok_statement; }
    else if (c == '\'') { token = tok_quote; }
    else if (c == ',') { token = tok_symbol; read_single_symbol(); }
    else if (read_int64() || read_uint64() || read_real64()) { token = tok_number; }
    else { token = tok_symbol; read_symbol(); }
done:
    return token;
}

Any LexerParser::get_symbol() {
    char dest[string_len + 1];
    memcpy(dest, string, string_len);
    dest[string_len] = 0;
    auto size = unescape_string(dest);
    return Symbol(String::from(dest, size));
}
Any LexerParser::get_string() {
    auto len = string_len - 2;
    char dest[len + 1];
    memcpy(dest, string + 1, len);
    dest[len] = 0;
    auto size = unescape_string(dest);
    return String::from(dest, size);
}
Any LexerParser::get_block_string() {
    int strip_col = column() + 4;
    auto len = string_len - 4;
    assert(len >= 0);
    char dest[len + 1];
    const char *start = string + 4;
    const char *end = start + len;
    // strip trailing whitespace up to the first LF after content
    const char *last_lf = end;
    while (end != start) {
        char c = *(end - 1);
        if (!isspace(c)) break;
        if (c == '\n')
            last_lf = end;
        end--;
    }
    end = last_lf;
    char *p = dest;
    while (start != end) {
        char c = *start++;
        *p++ = c;
        if (c == '\n') {
            // strip leftside column
            for (int i = 1; i < strip_col; ++i) {
                if (start == end) break;
                if ((*start != ' ') && (*start != '\t')) break;
                start++;
            }
        }
    }
    return String::from(dest, p - dest);
}
Any LexerParser::get_number() {
    return value;
}
Any LexerParser::get() {
    if (token == tok_number) {
        return get_number();
    } else if (token == tok_symbol) {
        return get_symbol();
    } else if (token == tok_string) {
        return get_string();
    } else if (token == tok_block_string) {
        return get_block_string();
    } else {
        return none;
    }
}

// PARSER
//////////////////////////////

LexerParser::ListBuilder::ListBuilder(LexerParser &_lexer) :
    lexer(_lexer),
    prev(EOL),
    eol(EOL) {}

void LexerParser::ListBuilder::append(const Any &value) {
    assert(value.type == TYPE_Syntax);
    prev = List::from(value, prev);
}

bool LexerParser::ListBuilder::is_empty() const {
    return (prev == EOL);
}

bool LexerParser::ListBuilder::is_expression_empty() const {
    return (prev == EOL);
}

void LexerParser::ListBuilder::reset_start() {
    eol = prev;
}

void LexerParser::ListBuilder::split(const Anchor *anchor) {
    // reverse what we have, up to last split point and wrap result
    // in cell
    prev = List::from(
        Syntax::from(anchor,reverse_list_inplace(prev, eol)), eol);
    reset_start();
}

const List *LexerParser::ListBuilder::get_result() {
    return reverse_list_inplace(prev);
}

//////////////////////////////

// parses a list to its terminator and returns a handle to the first cell
const List *LexerParser::parse_list(Token end_token) {
    const Anchor *start_anchor = this->anchor();
    ListBuilder builder(*this);
    this->read_token();
    while (true) {
        if (this->token == end_token) {
            break;
        } else if (this->token == tok_escape) {
            int column = this->column();
            this->read_token();
            builder.append(parse_naked(column, end_token));
        } else if (this->token == tok_eof) {
            set_active_anchor(start_anchor);
            location_error(String::from("unclosed open bracket"));
        } else if (this->token == tok_statement) {
            builder.split(this->anchor());
            this->read_token();
        } else {
            builder.append(parse_any());
            this->read_token();
        }
    }
    return builder.get_result();
}

// parses the next sequence and returns it wrapped in a cell that points
// to prev
Any LexerParser::parse_any() {
    assert(this->token != tok_eof);
    const Anchor *anchor = this->anchor();
    if (this->token == tok_open) {
        return Syntax::from(anchor, parse_list(tok_close));
    } else if (this->token == tok_square_open) {
        return Syntax::from(anchor,
            List::from(Syntax::from(anchor,Symbol(SYM_SquareList)),
                parse_list(tok_square_close)));
    } else if (this->token == tok_curly_open) {
        return Syntax::from(anchor,
            List::from(Syntax::from(anchor,Symbol(SYM_CurlyList)),
                parse_list(tok_curly_close)));
    } else if ((this->token == tok_close)
        || (this->token == tok_square_close)
        || (this->token == tok_curly_close)) {
        location_error(String::from("stray closing bracket"));
    } else if (this->token == tok_string) {
        return Syntax::from(anchor, get_string());
    } else if (this->token == tok_block_string) {
        return Syntax::from(anchor, get_block_string());
    } else if (this->token == tok_symbol) {
        return Syntax::from(anchor, get_symbol());
    } else if (this->token == tok_number) {
        return Syntax::from(anchor, get_number());
    } else if (this->token == tok_quote) {
        this->read_token();
        if (this->token == tok_eof) {
            set_active_anchor(anchor);
            location_error(
                String::from("unexpected end of file after quote token"));
        }
        return Syntax::from(anchor,
            List::from({
                Any(Syntax::from(anchor, Symbol(KW_Quote))),
                parse_any() }));
    } else {
        location_error(format("unexpected token: %c (%i)",
            this->cursor[0], (int)this->cursor[0]));
    }
    return none;
}

Any LexerParser::parse_naked(int column, Token end_token) {
    int lineno = this->lineno;

    bool escape = false;
    int subcolumn = 0;

    const Anchor *anchor = this->anchor();
    ListBuilder builder(*this);

    bool unwrap_single = true;
    while (this->token != tok_eof) {
        if (this->token == end_token) {
            break;
        } else if (this->token == tok_escape) {
            escape = true;
            this->read_token();
            if (this->lineno <= lineno) {
                location_error(String::from(
                    "escape character is not at end of line"));
            }
            lineno = this->lineno;
        } else if (this->lineno > lineno) {
            if (subcolumn == 0) {
                subcolumn = this->column();
            } else if (this->column() != subcolumn) {
                location_error(String::from("indentation mismatch"));
            }
            if (column != subcolumn) {
                if ((column + 4) != subcolumn) {
                    location_error(String::from(
                        "indentations must nest by 4 spaces."));
                }
            }

            escape = false;
            lineno = this->lineno;
            // keep adding elements while we're in the same line
            while ((this->token != tok_eof)
                    && (this->token != end_token)
                    && (this->lineno == lineno)) {
                builder.append(parse_naked(subcolumn, end_token));
            }
        } else if (this->token == tok_statement) {
            this->read_token();
            unwrap_single = false;
            if (!builder.is_empty()) {
                break;
            }
        } else {
            builder.append(parse_any());
            lineno = this->next_lineno;
            this->read_token();
        }
        if ((!escape || (this->lineno > lineno))
            && (this->column() <= column)) {
            break;
        }
    }

    auto result = builder.get_result();
    if (unwrap_single && result && result->count == 1) {
        return result->at;
    } else {
        return Syntax::from(anchor, result);
    }
}

Any LexerParser::parse() {
    this->read_token();
    int lineno = 0;
    //bool escape = false;

    const Anchor *anchor = this->anchor();
    ListBuilder builder(*this);

    while (this->token != tok_eof) {
        if (this->token == tok_none) {
            break;
        } else if (this->token == tok_escape) {
            //escape = true;
            this->read_token();
            if (this->lineno <= lineno) {
                location_error(String::from(
                    "escape character is not at end of line"));
            }
            lineno = this->lineno;
        } else if (this->lineno > lineno) {
            if (this->column() != 1) {
                location_error(String::from(
                    "indentation mismatch"));
            }

            //escape = false;
            lineno = this->lineno;
            // keep adding elements while we're in the same line
            while ((this->token != tok_eof)
                    && (this->token != tok_none)
                    && (this->lineno == lineno)) {
                builder.append(parse_naked(1, tok_none));
            }
        } else if (this->token == tok_statement) {
            location_error(String::from(
                "unexpected statement token"));
        } else {
            builder.append(parse_any());
            lineno = this->next_lineno;
            this->read_token();
        }
    }
    return Syntax::from(anchor, builder.get_result());
}


} // namespace scopes
