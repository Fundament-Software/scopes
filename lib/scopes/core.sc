#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""globals
    =======

    These names are bound in every fresh module and main program by default.
    Essential symbols are created by the compiler, and subsequent utility
    functions, macros and types are defined and documented in `core.sc`.

    The core module implements the remaining standard functions and macros,
    parses the command-line and optionally enters the REPL.

let newline = "\n"

fn print-string (s)
    if true
        sc_write s
        sc_write newline
    elseif false
        sc_write s
        sc_write newline
    else
        _ 1 2 3
    return true

let result =
    do
        loop (i) = 0
        if (icmp== i 5)
            sc_write "break!\n"
            break i
        sc_write "loop!\n"
        repeat (add i 1)
    #
        if (icmp<s i 5)
            sc_write "loop!\n"
            repeat (add i 1)
        sc_write "break!\n"
        i

print-string "hello world"
print-string "hello world, again"
