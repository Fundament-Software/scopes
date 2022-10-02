/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_LEXERPARSER_HPP
#define SCOPES_LEXERPARSER_HPP

#include "result.hpp"
#include "valueref.inc"
#include "symbol.hpp"

#include <stddef.h>
#include "absl/container/flat_hash_map.h"

namespace scopes {

struct String;
struct Symbol;
struct List;
struct Anchor;
struct SourceFile;
struct Const;
struct Value;
struct ConstInt;
struct ConstPointer;
struct Type;

//------------------------------------------------------------------------------
// S-EXPR LEXER & PARSER
//------------------------------------------------------------------------------

#define B_TOKENS() \
    T(none, -1) \
    T(eof, 0) \
    T(open, '(') \
    T(close, ')') \
    T(square_open, '[') \
    T(square_close, ']') \
    T(curly_open, '{') \
    T(curly_close, '}') \
    T(string, '"') \
    T(block_string, 'B') \
    T(syntax_quote, '\'') \
    T(ast_quote, '`') \
    T(symbol, 'S') \
    T(string_prefix, 'p') \
    T(escape, '\\') \
    T(statement, ';') \
    T(number, 'N')

enum Token {
#define T(NAME, VALUE) tok_ ## NAME = VALUE,
    B_TOKENS()
#undef T
};

const char *get_token_name(Token tok);

const char TOKEN_TERMINATORS[] = "()[]{}\"';#,";

struct LexerParser {
    struct ListBuilder {
        LexerParser &lexer;
        const List *prev;
        const List *eol;

        ListBuilder(LexerParser &_lexer);

        void append(ValueRef value);

        bool is_empty() const;

        bool is_expression_empty() const;

        void reset_start();

        void split(const Anchor *anchor);

        const List *get_result();
    };

    template<unsigned N>
    bool is_suffix(const char (&str)[N]);

    SCOPES_RESULT(void) verify_good_taste(char c);

    LexerParser(std::unique_ptr<SourceFile> _file, size_t offset = 0, size_t length = 0);

    int offset();

    int column();

    int next_column();

    const Anchor *anchor();

    SCOPES_RESULT(char) next();

    size_t chars_left();

    bool is_eof();

    void newline();

    void select_string();

    void read_single_symbol();

    SCOPES_RESULT(void) read_symbol();
    SCOPES_RESULT(void) read_symbol_or_prefix();

    SCOPES_RESULT(void) read_string(char terminator);

    SCOPES_RESULT(void) read_block(int indent);

    SCOPES_RESULT(void) read_block_string();

    SCOPES_RESULT(void) read_comment();

    bool has_suffix() const;

    SCOPES_RESULT(bool) select_integer_suffix();

    SCOPES_RESULT(bool) select_real_suffix();

    SCOPES_RESULT(bool) read_number();

    void next_token();

    SCOPES_RESULT(Token) read_token();

    Symbol get_symbol();
    const String *get_string();
    const String *get_unescaped_string();
    const String *get_block_string();
    ValueRef get_number();
    //Const *get();

    // parses a list to its terminator and returns a handle to the first cell
    SCOPES_RESULT(const List *) parse_list(Token end_token);

    // parses the next sequence and returns it wrapped in a cell that points
    // to prev
    SCOPES_RESULT(ValueRef) parse_any();
    SCOPES_RESULT(ValueRef) parse_prefixed_string();

    SCOPES_RESULT(ValueRef) parse_naked(int column, Token end_token);

    SCOPES_RESULT(ValueRef) parse();

    Token token;
    int base_offset;
    std::unique_ptr<SourceFile> file;
    const char *input_stream;
    const char *eof;
    const char *cursor;
    const char *next_cursor;
    int lineno;
    int next_lineno;
    const char *line;
    const char *next_line;

    const char *string;
    int string_len;

    ValueRef value;
    absl::flat_hash_map<Symbol, ConstIntRef, Symbol::Hash> prefix_symbol_map;
};


} // namespace scopes

#endif // SCOPES_LEXERPARSER_HPP