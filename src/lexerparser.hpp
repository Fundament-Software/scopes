/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_LEXERPARSER_HPP
#define SCOPES_LEXERPARSER_HPP

#include "any.hpp"

namespace scopes {

struct SourceFile;

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
    T(quote, '\'') \
    T(symbol, 'S') \
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

        void append(const Any &value);

        bool is_empty() const;

        bool is_expression_empty() const;

        void reset_start();

        void split(const Anchor *anchor);

        const List *get_result();
    };

    template<unsigned N>
    bool is_suffix(const char (&str)[N]);

    void verify_good_taste(char c);

    LexerParser(SourceFile *_file, size_t offset = 0, size_t length = 0);

    int offset();

    int column();

    int next_column();

    const Anchor *anchor();

    char next();

    size_t chars_left();

    bool is_eof();

    void newline();

    void select_string();

    void read_single_symbol();

    void read_symbol();

    void read_string(char terminator);

    void read_block(int indent);

    void read_block_string();

    void read_comment();

    template<typename T>
    int read_integer(void (*strton)(T *, const char*, char**));

    template<typename T>
    int read_real(void (*strton)(T *, const char*, char**, int));

    bool has_suffix() const;

    bool select_integer_suffix();

    bool select_real_suffix();

    bool read_int64();
    bool read_uint64();
    bool read_real64();

    void next_token();

    Token read_token();

    Any get_symbol();
    Any get_string();
    Any get_block_string();
    Any get_number();
    Any get();

    // parses a list to its terminator and returns a handle to the first cell
    const List *parse_list(Token end_token);

    // parses the next sequence and returns it wrapped in a cell that points
    // to prev
    Any parse_any();

    Any parse_naked(int column, Token end_token);

    Any parse();

    Token token;
    int base_offset;
    SourceFile *file;
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

    Any value;
};


} // namespace scopes

#endif // SCOPES_LEXERPARSER_HPP