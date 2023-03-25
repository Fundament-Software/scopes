#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

using import Array
using import String

""""format
    ======

    Support for agnostic string formatting through `format`.

    See the following examples:

        :::scopes
        using import format
        using import String

        # prints Hello World!
        print
            .. (format "{} {}!" "Hello" "World")

        # prints fizzbuzzbuzzfizzfizz
        print
            String (format "{0}buzz{1}fizz{0}" "fizz" "buzz")

        # prints * Joana (42)
        print
            vvv String
            format "* {name} ({age})"
                name = "Joana"
                age = 42

#-------------------------------------------------------------------------------
# hex/oct/bin conversion
#-------------------------------------------------------------------------------

fn integer->string (value base)
    let value base = (deref value) (deref base)
    let N = 65
    let T = (typeof value)
    let digits = (alloca-array char N)
    let absvalue = (abs value)
    let neg? = (value != absvalue)
    loop (i value = N absvalue)
        if (i == 0)
            break (String digits N)
        let i = (i - 1)
        let digit = ((value % base) as char)
        digits @ i =
            + digit
                ? (digit >= 10:char) (97:char - 10:char) 48:char
        let value = (value // base)
        if (value == (0 as T))
            let i =
                if ((i > 0) & neg?)
                    let i = (i - 1)
                    digits @ i = 45:char
                    i
                else i
            break (String (& (digits @ i)) ((N - i) as usize))
        repeat i value

fn bin (value)
    let value = (value as integer)
    integer->string value (2 as (typeof value))

fn oct (value)
    let value = (value as integer)
    integer->string value (8 as (typeof value))

fn dec (value)
    let value = (value as integer)
    integer->string value (10 as (typeof value))

fn hex (value)
    let value = (value as integer)
    integer->string value (16 as (typeof value))

#-------------------------------------------------------------------------------

fn string-array-ref-type? (T)
    if (icmp== ('kind T) type-kind-array)
        & ('refer? T)
            ptrcmp== ('element@ T 0) i8
    else false

spice format (str args...)
    using import UTF-8

    anchor := ('anchor str)
    str as:= string
    local block : (Array Value)
    inline scan-until (i L search-character)
        loop (i)
            if (i >= L)
                break (min L i)
            c := str @ i
            switch c
            case search-character
                break i
            case c"\\"
                repeat (i + 2)
            default
                repeat (i + 1)
    fn parse-integer (str)
        L := (countof str)
        loop (i val = 0:usize 0:usize)
            if (i >= L)
                break val i
            c := str @ i
            let z0 z9 = c"0" c"9"
            if ((c >= z0) & (c <= z9))
                repeat (i + 1) (val * 10:usize + (c - z0))
            else
                break val i
    L := (countof str)
    inline parse-error (i msg)
        hide-traceback;
        error@ anchor
            #sc_anchor_offset anchor (as i i32)
            "while parsing format string"
            msg
    loop (i nextarg = 0:usize 0)
        if (i == L)
            break;
        c := str @ i
        if (c == c"{")
            i := i + 1
            start := i
            let i = (scan-until i L c"}")
            if (i == L)
                parse-error start "unterminated string formatting expression"
            substr := (slice str start i)
            let nextarg body =
                if (substr == "")
                    _ (nextarg + 1) ('getarg args... nextarg)
                else
                    let val k = (parse-integer substr)
                    if (k == 0) # key
                        for arg in ('args args...)
                            let key arg = ('dekey arg)
                            if (key as string == substr)
                                break nextarg arg
                        else
                            parse-error (start + 1)
                                .. "no argument with key " (repr `substr)
                    elseif (k == (countof substr)) # index
                        _ nextarg ('getarg args... (val as i32))
                    else
                        parse-error (start + k + 1)
                            "invalid character in index expression"
            QT := ('qualifiersof body)
            T := ('strip-qualifiers QT)
            'append block
                if ((T == String) | (T == string) | (string-array-ref-type? QT)) body
                else `(tostring body)
            _ (i + 1) nextarg
        else
            # read string chunk up to the next variable and append to result,
            # after unescaping braces.
            start := i
            let i = (scan-until i L c"{")
            local substr : String
            loop (k = start)
                if (k == i)
                    break;
                c := str @ k
                if (c == c"\\")
                    k1 := k + 1
                    if (k1 < i)
                        d := str @ k1
                        if ((d == c"{") | (d == c"}"))
                            'append substr d
                            repeat (k1 + 1)
                'append substr c
                repeat (k + 1)
            'append block (substr as string)
            _ i nextarg
    sc_argument_list_new ((countof block) as i32) (& (block @ 0))

do
    let integer->string bin oct dec hex
    let format
    locals;
