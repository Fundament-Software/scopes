#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

using import enum
using import String
using import format
using import inspect

""""cgen
    ====

    A C code generator. This module is work in progress.

fn tabs (n)
    local str : String
    'resize str (2 * n) " "
    str

@@ memo
inline separator (begin sep end)
    fn (elem)
        level expr offset := elem
        result := if (offset == 1) begin
        else ""
        if (offset >= (countof expr))
            _ none (.. result end)
        else
            item := expr @ offset
            result := if (offset > 1) sep
            else result
            _ (tuple level item 1) result

@@ memo
inline line-separator (begin sep end)
    fn (elem)
        level expr offset := elem
        spacing := .. "\n" (tabs level)
        begin := if (begin == "") begin
        else
            .. begin spacing
        sep := .. sep spacing
        end := if (end == "") end
        else
            .. spacing end
        result := if (offset == 1) begin
        else ""
        if (offset >= (countof expr))
            _ none (.. result end)
            # else
            item := expr @ offset
            result := if (offset > 1) sep
            else result
            _ (tuple level item 1) result

@@ memo
inline block-separator (begin sep end levelofs)
    fn (elem)
        level expr offset := elem
        sublevel := level + levelofs
        inner-spacing := .. "\n" (tabs sublevel)
        outer-spacing := .. "\n" (tabs level)
        begin := .. begin inner-spacing
        sep := .. sep inner-spacing
        end := .. outer-spacing end
        result := if (offset == 1) begin
        else ""
        if (offset >= (countof expr))
            _ none (.. result end)
        else
            item := expr @ offset
            result := if (offset > 1) sep
            else result
            _ (tuple sublevel item 1) result

fn int (elem)
    level expr offset := elem
    _ none (dec (expr @ 1))

fn intliteral (elem)
    level expr offset := elem
    value := expr @ 1
    none
    .. "0x"
        hex value
        switch (sizeof value)
        case 4 "u"
        case 8 "ull"
        default ""

#fn quotestr (x)
    sz := (sizeof x)
    s = loop (i s = 0 "")
        if (< i sz)
            c = extract s_char x i
            c =
                switch c
                    #default
                    if (char-printable? c) c
                        .. (.. "\\x" (u8->hex c)) "\"\""
                    \ "\n" "\\n"
                    \ "\t" "\\t"
                    \ "\r" "\\r"
                    \ "\"" "\\\""
                    \ "\\" "\\\\"
                    \ "\x00" "\\0"
            repeat (+ i 1) (.. s c)
            break s
    .. "\"" (.. s "\"")

#C.str = fn (elem)
    level expr offset = elem
    cell empty (C.quotestr (@ expr 1))

#C.include = fn (elem)
    level expr offset = elem
    cell empty (.. "#include " (C.quotestr (@ expr 1)))
#C.sysinclude = fn (elem)
    level expr offset = elem
    cell empty (.. (.. "#include <" (@ expr 1)) ">")

do
    locals;
