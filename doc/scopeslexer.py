# -*- coding: utf-8 -*-
"""
    pygments.lexers.scopes
    ~~~~~~~~~~~~~~~~~~~~~~

    A Lexer for Scopes.

    :copyright: Copyright 2018 by the Pygments team, see AUTHORS.
    :license: BSD, see LICENSE for details.
"""

from pygments.lexer import Lexer
from pygments.token import *

__all__ = ['ScopesLexer']

class TOK:
    none = -1
    eof = 0
    open = '('
    close = ')'
    square_open = '['
    square_close = ']'
    curly_open = '{'
    curly_close = '}'
    string = '"'
    block_string = 'B'
    quote = '\''
    symbol = 'S'
    escape = '\\'
    statement = ';'
    number = 'N'

TOKEN_TERMINATORS = "()[]{}\"';#,"

types = set("""bool i8 i16 i32 i64 u8 u16 u32 u64 f32 f64 Scope Label Parameter
string list Symbol Syntax Nothing type Any usize vector real integer Generator
array typename tuple reference extern CUnion CStruct CEnum union pointer
Closure function NullType voidstar void Macro hash Builtin Frame
""".strip().split())
keywords = set("""fn let if elseif else label return syntax-extend loop repeat
while for in del break continue call using import define-infix> define-infix<
define-macro define-scope-macro define-block-scope-macro define struct match
quote assert fn... defer define-doc
""".strip().split())
builtins = set("""print import-from branch icmp== icmp!= icmp<s icmp<=s icmp>s
icmp>=s icmp<u icmp<=u icmp>u icmp>=u
fcmp==o fcmp!=o fcmp<o fcmp<=o fcmp>o fcmp>=o
fcmp==u fcmp!=u fcmp<u fcmp<=u fcmp>u fcmp>=u
extractvalue insertvalue compiler-error! io-write abort! unreachable! undef
ptrtoint inttoptr bitcast typeof string-join va-countof va@ type@ Scope@
set-type-symbol! set-scope-symbol!
band bor bxor add sub mul udiv sdiv urem srem zext sext trunc shl ashr lshr
fadd fsub fmul fdiv frem fpext fptrunc
fptosi fptoui sitofp uitofp signed? bitcountof getelementptr
Any-repr Any-wrap integer-type unconst purify compile typify
Any-extract-constant string->Symbol dump dump-label constant? load store
countof globals load-module exit xpcall io-write! set-scope-symbol!
set-type-symbol! set-type-symbol!& empty? slice prompt repr string-repr
set-globals! shufflevector insertelement extractelement nullof itrunc trunc
element-type any? all? vector-type vector-reduce type-countof tie-const
list-parse zip fold enumerate unroll-range tupleof typefn typefn& typefn!
typefn!& set-typename-storage! arrayof cons unpack set-typename-super! tupleof
decons va-keys syntax-error! element-index opaque? unknownof deref require-from
error! docstring delete new static local imply type@& unconst-all pow
compile-object compile-spirv compile-glsl min max clamp char sizeof storageof
getattr typeattr abs sign sin cos tan log log2 length normalize va-join va-types
va-empty? none? list? Symbol? function-pointer? extern? tuple? vector? array?
pointer? real? integer? typename? function-pointer-type? extern-type?
vector-type? array-type? tuple-type? function-type? pointer-type? real-type?
integer-type? typename-type? todo! _ vectorof map element-name va-key
""".strip().split())
builtin_constants = set("""true false none syntax-scope null main-module?
pi pi:f32 pi:f64 e e:f32 e:f64 unnamed package
""".strip().split())
operators = set("""+ - * / << >> & | ^ ~ % . < > <= >= != == = .. @ += -= *= /=
//= %= >>= <<= &= |= ^= ** //
""".strip().split())
word_operators = set("""not as and or
""".strip().split())

integer_literal_suffixes = set("i8 i16 i32 i64 u8 u16 u32 u64 usize".strip().split())
real_literal_suffixes = set("f32 f64".strip().split())

def isspace(c):
    return c in ' \t\n\r'

class ScopesLexer(Lexer):
    """
    Lexer for the Scopes programming language (version 0.13).
    """
    name = 'Scopes'
    filenames = ['*.sc']
    aliases = ['scopes']
    mimetypes = ['text/scopes']

    def __init__(self, **options):
        Lexer.__init__(self, **options)

    def get_tokens_unprocessed(self, text):
        state = type('state', (), {})
        state.cursor = 0
        state.next_cursor = 0
        state.lineno = 1
        state.next_lineno = 1
        state.line = 0
        state.next_line = 0

        def column():
            return state.cursor - state.line + 1

        def next_column():
            return state.next_cursor - state.next_line + 1

        def location_error(msg):
            print text
            raise Exception("%i:%i: error: %s" % (state.lineno,column(),msg))

        def is_eof():
            return state.next_cursor == len(text)

        def chars_left():
            return len(text) - state.next_cursor

        def next():
            x = text[state.next_cursor]
            state.next_cursor = state.next_cursor + 1
            return x

        def next_token():
            state.lineno = state.next_lineno
            state.line = state.next_line
            state.cursor = state.next_cursor

        def newline():
            state.next_lineno = state.next_lineno + 1
            state.next_line = state.next_cursor

        def select_string():
            state.value = text[state.cursor:state.next_cursor]

        def reset_cursor():
            state.next_cursor = state.cursor

        def try_fmt_split(s):
            l = s.split(':')
            if len(l) == 2:
                return l
            else:
                return s,None

        def is_integer(s):
            tail = None
            if ':' in s:
                s,tail = try_fmt_split(s)
            if tail and not (tail in integer_literal_suffixes):
                return False
            if not s:
                return False
            if s[0] in '+-':
                s = s[1:]
            nums = '0123456789'
            if s.startswith('0x'):
                nums = nums + 'ABCDEFabcdef'
                s = s[2:]
            elif s.startswith('0b'):
                nums = '01'
                s = s[2:]
            elif len(s) > 1 and s[0] == '0':
                return False
            if len(s) == 0:
                return False
            for k,c in enumerate(s):
                if not c in nums:
                    return False
            return True

        def is_real(s):
            tail = None
            if ':' in s:
                s,tail = try_fmt_split(s)
            if tail and not (tail in real_literal_suffixes):
                return False
            if not s: return False
            if s[0] in '+-':
                s = s[1:]
            if s == 'inf' or s == 'nan':
                return True
            nums = '0123456789'
            if s.startswith('0x'):
                nums = nums + 'ABCDEFabcdef'
                s = s[2:]
            if len(s) == 0:
                return False
            for k,c in enumerate(s):
                if c == 'e':
                    return is_integer(s[k + 1:])
                if c == '.':
                    s = s[k + 1:]
                    for k,c in enumerate(s):
                        if c == 'e':
                            return is_integer(s[k + 1:])
                        if not c in nums:
                            return False
                    break
                if not c in nums:
                    return False
            return True

        def read_symbol():
            escape = False
            while True:
                if is_eof():
                    break
                c = next()
                if escape:
                    if c == '\n':
                        newline()
                    escape = False
                elif c == '\\':
                    escape = True
                elif isspace(c) or (c in TOKEN_TERMINATORS):
                    state.next_cursor = state.next_cursor - 1
                    break
            select_string()

        def read_string(terminator):
            escape = False
            while True:
                if is_eof():
                    location_error("unterminated sequence")
                    break
                c = next()
                if c == '\n':
                    # 0.10
                    # newline()
                    # 0.11
                    location_error("unexpected line break in string")
                    break
                if escape:
                    escape = False
                elif c == '\\':
                    escape = True
                elif c == terminator:
                    break
            select_string()

        def read_block(indent):
            col = column() + indent
            while True:
                if is_eof():
                    break
                next_col = next_column()
                c = next()
                if c == '\n':
                    newline()
                elif not isspace(c) and (next_col <= col):
                    state.next_cursor = state.next_cursor - 1
                    break
            select_string()

        def read_block_string():
            next()
            next()
            next()
            read_block(3)

        def read_comment():
            read_block(0)

        def read_whitespace():
            while True:
                if is_eof():
                    break
                c = next()
                if c == '\n':
                    newline()
                elif not isspace(c):
                    state.next_cursor = state.next_cursor - 1
                    break
            select_string()

        while True:
            next_token()
            if is_eof():
                return
            c = next()
            cur = state.cursor
            if c == '\n':
                newline()
            if isspace(c):
                token = Token.Whitespace
                read_whitespace()
            elif c == '#':
                token = Token.Comment
                read_comment()
            elif c == '(':
                token = Token.Punctuation.Open
                select_string()
            elif c == ')':
                token = Token.Punctuation.Close
                select_string()
            elif c == '[':
                token = Token.Punctuation.Square.Open
                select_string()
            elif c == ']':
                token = Token.Punctuation.Square.Close
                select_string()
            elif c == '{':
                token = Token.Punctuation.Curly.Open
                select_string()
            elif c == '}':
                token = Token.Punctuation.Curly.Close
                select_string()
            elif c == '\\':
                token = Token.Punctuation.Escape
                select_string()
            elif c == '"':
                token = Token.String
                if ((chars_left() >= 3)
                    and (text[state.next_cursor+0] == '"')
                    and (text[state.next_cursor+1] == '"')
                    and (text[state.next_cursor+2] == '"')):
                    read_block_string()
                else:
                    read_string(c)
            elif c == ';':
                token = Token.Punctuation.Statement.Separator
                select_string()
            elif c == '\'':
                token = Token.String.Symbol
                read_symbol()
            elif c == ',':
                token = Token.Punctuation.Comma
                select_string()
            else:
                read_symbol()
                if state.value in keywords:
                    token = Token.Keyword.Declaration
                elif state.value in builtins:
                    token = Token.Name.Builtin
                elif state.value in types:
                    token = Token.Keyword.Type
                elif state.value in builtin_constants:
                    token = Token.Keyword.Constant
                elif state.value in operators:
                    token = Token.Operator
                elif state.value in word_operators:
                    token = Token.Operator.Word
                elif is_integer(state.value):
                    token = Token.Number.Integer
                elif is_real(state.value):
                    token = Token.Number.Float
                else:
                    token = Token.Name
            yield state.cursor, token, state.value

def setup(app):
    app.add_lexer("scopes", ScopesLexer())
