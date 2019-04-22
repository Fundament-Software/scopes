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
#include "value.hpp"
#include "dyn_cast.inc"
#include "globals.hpp"

#include "scopes/scopes.h"

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
    return "?";
}

template<unsigned N>
bool LexerParser::is_suffix(const char (&str)[N]) {
    if (string_len != (N - 1)) {
        return false;
    }
    return !strncmp(string, str, N - 1);
}

SCOPES_RESULT(void) LexerParser::verify_good_taste(char c) {
    SCOPES_RESULT_TYPE(void);
    if (c == '\t') {
        next_token();
        SCOPES_TRACE_PARSER(this->anchor());
        SCOPES_ERROR(ParserBadTaste);
    }
    return {};
}

LexerParser::LexerParser(SourceFile *_file, size_t offset, size_t length) {
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

SCOPES_RESULT(char) LexerParser::next() {
    SCOPES_RESULT_TYPE(char);
    char c = next_cursor[0];
    SCOPES_CHECK_RESULT(verify_good_taste(c));
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

SCOPES_RESULT(void) LexerParser::read_symbol() {
    SCOPES_RESULT_TYPE(void);
    bool escape = false;
    while (true) {
        if (is_eof()) {
            break;
        }
        char c = SCOPES_GET_RESULT(next());
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
    return {};
}

SCOPES_RESULT(void) LexerParser::read_string(char terminator) {
    SCOPES_RESULT_TYPE(void);
    bool escape = false;
    while (true) {
        if (is_eof()) {
            SCOPES_TRACE_PARSER(this->anchor());
            SCOPES_ERROR(ParserUnterminatedSequence);
        }
        char c = SCOPES_GET_RESULT(next());
        if (c == '\n') {
            // 0.10
            //newline();
            // 0.11
            SCOPES_TRACE_PARSER(this->anchor());
            SCOPES_ERROR(ParserUnexpectedLineBreak);
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
    return {};
}

SCOPES_RESULT(void) LexerParser::read_block(int indent) {
    SCOPES_RESULT_TYPE(void);
    int col = column() + indent;
    while (true) {
        if (is_eof()) {
            break;
        }
        int next_col = next_column();
        char c = SCOPES_GET_RESULT(next());
        if (c == '\n') {
            newline();
        } else if (!isspace(c) && (next_col <= col)) {
            next_cursor = next_cursor - 1;
            break;
        }
    }
    return {};
}

SCOPES_RESULT(void) LexerParser::read_block_string() {
    SCOPES_RESULT_TYPE(void);
    SCOPES_CHECK_RESULT(next());
    SCOPES_CHECK_RESULT(next());
    SCOPES_CHECK_RESULT(next());
    SCOPES_CHECK_RESULT(read_block(3));
    select_string();
    return {};
}

SCOPES_RESULT(void) LexerParser::read_comment() {
    SCOPES_RESULT_TYPE(void);
    SCOPES_CHECK_RESULT(read_block(0));
    return {};
}

template<typename T>
SCOPES_RESULT(int) LexerParser::read_integer(const Type *TT, void (*strton)(T *, const char*, char**)) {
    SCOPES_RESULT_TYPE(int);
    char *cend;
    errno = 0;
    T srcval;
    strton(&srcval, cursor, &cend);
    if ((cend == cursor)
        || (errno == ERANGE)
        || (cend > eof)) {
        return RN_Invalid;
    }
    value = ref(anchor(), ConstInt::from(TT, srcval));
    next_cursor = cend;
    if ((cend != eof)
        && (!isspace(*cend))
        && (!strchr(TOKEN_TERMINATORS, *cend))) {
        if (strchr(".e", *cend)) return false;
        // suffix
        auto _lineno = lineno; auto _line = line; auto _cursor = cursor;
        next_token();
        SCOPES_CHECK_RESULT(read_symbol());
        lineno = _lineno; line = _line; cursor = _cursor;
        return RN_Typed;
    } else {
        return RN_Untyped;
    }
}

template<typename T>
SCOPES_RESULT(int) LexerParser::read_real(const Type *TT, void (*strton)(T *, const char*, char**, int)) {
    SCOPES_RESULT_TYPE(int);
    char *cend;
    errno = 0;
    T srcval;
    strton(&srcval, cursor, &cend, 0);
    if ((cend == cursor)
        || (errno == ERANGE)
        || (cend > eof)) {
        return RN_Invalid;
    }
    value = ref(anchor(), ConstReal::from(TT, srcval));
    next_cursor = cend;
    if ((cend != eof)
        && (!isspace(*cend))
        && (!strchr(TOKEN_TERMINATORS, *cend))) {
        // suffix
        auto _lineno = lineno; auto _line = line; auto _cursor = cursor;
        next_token();
        SCOPES_CHECK_RESULT(read_symbol());
        lineno = _lineno; line = _line; cursor = _cursor;
        return RN_Typed;
    } else {
        return RN_Untyped;
    }
}

bool LexerParser::has_suffix() const {
    return (string_len >= 1) && (string[0] == ':');
}

SCOPES_RESULT(bool) LexerParser::select_integer_suffix() {
    SCOPES_RESULT_TYPE(bool);
    if (!has_suffix())
        return false;
    assert(value.isa<ConstInt>());
    const Type *newtype = nullptr;
    if (is_suffix(":i8")) { newtype = TYPE_I8; }
    else if (is_suffix(":i16")) { newtype = TYPE_I16; }
    else if (is_suffix(":i32")) { newtype = TYPE_I32; }
    else if (is_suffix(":i64")) { newtype = TYPE_I64; }
    else if (is_suffix(":u8")) { newtype = TYPE_U8; }
    else if (is_suffix(":u16")) { newtype = TYPE_U16; }
    else if (is_suffix(":u32")) { newtype = TYPE_U32; }
    else if (is_suffix(":u64")) { newtype = TYPE_U64; }
    //else if (is_suffix(":isize")) { newtype = TYPE_ISize; }
    else if (is_suffix(":usize")) { newtype = TYPE_USize; }
    else {
        SCOPES_TRACE_PARSER(this->anchor());
        SCOPES_ERROR(ParserInvalidIntegerSuffix,
            String::from(string, string_len));
    }
    value = ref(value.anchor(),
        ConstInt::from(newtype, value.cast<ConstInt>()->value));
    return true;
}

SCOPES_RESULT(bool) LexerParser::select_real_suffix() {
    SCOPES_RESULT_TYPE(bool);
    if (!has_suffix())
        return false;
    assert(value.isa<ConstReal>());
    const Type *newtype = nullptr;
    if (is_suffix(":f32")) { newtype = TYPE_F32; }
    else if (is_suffix(":f64")) { newtype = TYPE_F64; }
    else {
        SCOPES_TRACE_PARSER(this->anchor());
        SCOPES_ERROR(ParserInvalidRealSuffix,
            String::from(string, string_len));
    }
    value = ref(value.anchor(),
        ConstReal::from(newtype, value.cast<ConstReal>()->value));
    return true;
}

SCOPES_RESULT(bool) LexerParser::read_int64() {
    SCOPES_RESULT_TYPE(bool);
    switch(SCOPES_GET_RESULT(read_integer(TYPE_I64, scopes_strtoll))) {
    case RN_Invalid: return false;
    case RN_Untyped: {
        int64_t val = value.cast<ConstInt>()->value;
        if ((val >= -0x80000000ll) && (val <= 0x7fffffffll)) {
            value = ref(value.anchor(), ConstInt::from(TYPE_I32, val));
        } else if ((val >= 0x80000000ll) && (val <= 0xffffffffll)) {
            value = ref(value.anchor(), ConstInt::from(TYPE_U32, val));
        }
        return true;
    } break;
    case RN_Typed:
        return select_integer_suffix();
    default: assert(false); return false;
    }
}
SCOPES_RESULT(bool) LexerParser::read_uint64() {
    SCOPES_RESULT_TYPE(bool);
    switch(SCOPES_GET_RESULT(read_integer(TYPE_U64, scopes_strtoull))) {
    case RN_Invalid: return false;
    case RN_Untyped:
        return true;
    case RN_Typed:
        return select_integer_suffix();
    default: assert(false); return false;
    }
}
SCOPES_RESULT(bool) LexerParser::read_real64() {
    SCOPES_RESULT_TYPE(bool);
    switch(SCOPES_GET_RESULT(read_real(TYPE_F64, scopes_strtod))) {
    case RN_Invalid: return false;
    case RN_Untyped:
        value = ref(value.anchor(), ConstReal::from(TYPE_F32,
            value.cast<ConstReal>()->value));
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
}

SCOPES_RESULT(Token) LexerParser::read_token() {
    SCOPES_RESULT_TYPE(Token);
    char c;
skip:
    next_token();
    if (is_eof()) { token = tok_eof; goto done; }
    c = SCOPES_GET_RESULT(next());
    if (c == '\n') { newline(); }
    if (isspace(c)) { goto skip; }
    if (c == '#') { SCOPES_CHECK_RESULT(read_comment()); goto skip; }
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
            SCOPES_CHECK_RESULT(read_block_string());
        } else {
            token = tok_string;
            SCOPES_CHECK_RESULT(read_string(c));
        }
    }
    else if (c == ';') { token = tok_statement; }
    else if (c == '\'') { token = tok_syntax_quote; }
    else if (c == '`') { token = tok_ast_quote; }
    else if (c == ',') { token = tok_symbol; read_single_symbol(); }
    else if (SCOPES_GET_RESULT(read_int64())
        || SCOPES_GET_RESULT(read_uint64())
        || SCOPES_GET_RESULT(read_real64())) { token = tok_number; }
    else { token = tok_symbol; SCOPES_CHECK_RESULT(read_symbol()); }
done:
    return token;
}

Symbol LexerParser::get_symbol() {
    char dest[string_len + 1];
    memcpy(dest, string, string_len);
    dest[string_len] = 0;
    auto size = unescape_string(dest);
    return Symbol(String::from(dest, size));
}
const String *LexerParser::get_string() {
    auto len = string_len - 2;
    char dest[len + 1];
    memcpy(dest, string + 1, len);
    dest[len] = 0;
    auto size = unescape_string(dest);
    return String::from(dest, size);
}
const String *LexerParser::get_block_string() {
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
ValueRef LexerParser::get_number() {
    return value;
}
#if 0
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
#endif

// PARSER
//////////////////////////////

LexerParser::ListBuilder::ListBuilder(LexerParser &_lexer) :
    lexer(_lexer),
    prev(EOL),
    eol(EOL) {}

void LexerParser::ListBuilder::append(ValueRef value) {
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
    prev = List::from(ref(anchor,
        ConstPointer::list_from(reverse_list(prev, eol))), eol);
    reset_start();
}

const List *LexerParser::ListBuilder::get_result() {
    return reverse_list(prev);
}

//////////////////////////////

// parses a list to its terminator and returns a handle to the first cell
SCOPES_RESULT(const List *) LexerParser::parse_list(Token end_token) {
    SCOPES_RESULT_TYPE(const List *);
    const Anchor *start_anchor = this->anchor();
    ListBuilder builder(*this);
    SCOPES_CHECK_RESULT(this->read_token());
    while (true) {
        if (this->token == end_token) {
            break;
        } else if (this->token == tok_escape) {
            int column = this->column();
            SCOPES_CHECK_RESULT(this->read_token());
            builder.append(SCOPES_GET_RESULT(parse_naked(column, end_token)));
        } else if (this->token == tok_eof) {
            SCOPES_TRACE_PARSER(this->anchor());
            SCOPES_ERROR(ParserUnclosedOpenBracket, start_anchor);
        } else if (this->token == tok_statement) {
            builder.split(this->anchor());
            SCOPES_CHECK_RESULT(this->read_token());
        } else {
            builder.append(SCOPES_GET_RESULT(parse_any()));
            SCOPES_CHECK_RESULT(this->read_token());
        }
    }
    return builder.get_result();
}

// parses the next sequence and returns it wrapped in a cell that points
// to prev
SCOPES_RESULT(ValueRef) LexerParser::parse_any() {
    SCOPES_RESULT_TYPE(ValueRef);
    assert(this->token != tok_eof);
    const Anchor *anchor = this->anchor();
    if (this->token == tok_open) {
        return ValueRef(anchor, ConstPointer::list_from(
            SCOPES_GET_RESULT(parse_list(tok_close))));
    } else if (this->token == tok_square_open) {
        return ValueRef(anchor, ConstPointer::list_from(
            List::from(
                ref(anchor, ConstInt::symbol_from(Symbol(SYM_SquareList))),
                SCOPES_GET_RESULT(parse_list(tok_square_close)))));
    } else if (this->token == tok_curly_open) {
        return ValueRef(anchor, ConstPointer::list_from(
            List::from(
                ref(anchor, ConstInt::symbol_from(Symbol(SYM_CurlyList))),
                SCOPES_GET_RESULT(parse_list(tok_curly_close)))));
    } else if ((this->token == tok_close)
        || (this->token == tok_square_close)
        || (this->token == tok_curly_close)) {
        SCOPES_TRACE_PARSER(this->anchor());
        SCOPES_ERROR(ParserStrayClosingBracket);
    } else if (this->token == tok_string) {
        return ValueRef(anchor, ConstPointer::string_from(get_string()));
    } else if (this->token == tok_block_string) {
        return ValueRef(anchor, ConstPointer::string_from(get_block_string()));
    } else if (this->token == tok_symbol) {
        return ValueRef(anchor, ConstInt::symbol_from(get_symbol()));
    } else if (this->token == tok_number) {
        return get_number();
    } else if (this->token == tok_syntax_quote) {
        SCOPES_CHECK_RESULT(this->read_token());
        if (this->token == tok_eof) {
            SCOPES_TRACE_PARSER(this->anchor());
            SCOPES_ERROR(ParserUnterminatedQuote);
        }
        return ValueRef(anchor, ConstPointer::list_from(
            List::from(
                ref(anchor, ConstInt::symbol_from(Symbol(KW_SyntaxQuote))),
                SCOPES_GET_RESULT(parse_any())
                )));
    } else if (this->token == tok_ast_quote) {
        SCOPES_CHECK_RESULT(this->read_token());
        if (this->token == tok_eof) {
            SCOPES_TRACE_PARSER(this->anchor());
            SCOPES_ERROR(ParserUnterminatedQuote);
        }
        return ValueRef(anchor, ConstPointer::list_from(
            List::from(
                ref(anchor, ConstInt::symbol_from(Symbol(KW_ASTQuote))),
                SCOPES_GET_RESULT(parse_any())
                )));
    } else {
        SCOPES_TRACE_PARSER(this->anchor());
        SCOPES_ERROR(ParserUnexpectedToken,
            this->cursor[0], (int)this->cursor[0]);
    }
    return ValueRef(anchor, ConstAggregate::none_from());
}

SCOPES_RESULT(ValueRef) LexerParser::parse_naked(int column, Token end_token) {
    SCOPES_RESULT_TYPE(ValueRef);
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
            SCOPES_CHECK_RESULT(this->read_token());
            if (this->lineno <= lineno) {
                SCOPES_TRACE_PARSER(this->anchor());
                SCOPES_ERROR(ParserStrayEscapeToken);
            }
            lineno = this->lineno;
        } else if (this->lineno > lineno) {
            if (subcolumn == 0) {
                subcolumn = this->column();
            } else if (this->column() != subcolumn) {
                SCOPES_TRACE_PARSER(this->anchor());
                SCOPES_ERROR(ParserIndentationMismatch);
            }
            if (column != subcolumn) {
                if ((column + 4) != subcolumn) {
                    SCOPES_TRACE_PARSER(this->anchor());
                    SCOPES_ERROR(ParserBadIndentationLevel);
                }
            }

            escape = false;
            lineno = this->lineno;
            // keep adding elements while we're in the same line
            while ((this->token != tok_eof)
                    && (this->token != end_token)
                    && (this->lineno == lineno)) {
                builder.append(SCOPES_GET_RESULT(parse_naked(subcolumn, end_token)));
            }
        } else if (this->token == tok_statement) {
            SCOPES_CHECK_RESULT(this->read_token());
            unwrap_single = false;
            if (!builder.is_empty()) {
                break;
            }
        } else {
            builder.append(SCOPES_GET_RESULT(parse_any()));
            lineno = this->next_lineno;
            SCOPES_CHECK_RESULT(this->read_token());
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
        return ValueRef(anchor, ConstPointer::list_from(result));
    }
}

SCOPES_RESULT(ValueRef) LexerParser::parse() {
    SCOPES_RESULT_TYPE(ValueRef);
    SCOPES_CHECK_RESULT(this->read_token());
    int lineno = 0;
    //bool escape = false;

    const Anchor *anchor = this->anchor();
    ListBuilder builder(*this);

    while (this->token != tok_eof) {
        if (this->token == tok_none) {
            break;
        } else if (this->token == tok_escape) {
            //escape = true;
            SCOPES_CHECK_RESULT(this->read_token());
            if (this->lineno <= lineno) {
                SCOPES_TRACE_PARSER(this->anchor());
                SCOPES_ERROR(ParserStrayEscapeToken);
            }
            lineno = this->lineno;
        } else if (this->lineno > lineno) {
            if (this->column() != 1) {
                SCOPES_TRACE_PARSER(this->anchor());
                SCOPES_ERROR(ParserIndentationMismatch);
            }

            //escape = false;
            lineno = this->lineno;
            // keep adding elements while we're in the same line
            while ((this->token != tok_eof)
                    && (this->token != tok_none)
                    && (this->lineno == lineno)) {
                builder.append(SCOPES_GET_RESULT(parse_naked(1, tok_none)));
            }
        } else if (this->token == tok_statement) {
            SCOPES_TRACE_PARSER(this->anchor());
            SCOPES_ERROR(ParserStrayStatementToken);
        } else {
            builder.append(SCOPES_GET_RESULT(parse_any()));
            lineno = this->next_lineno;
            SCOPES_CHECK_RESULT(this->read_token());
        }
    }
    return ValueRef(anchor, ConstPointer::list_from(builder.get_result()));
}


} // namespace scopes
