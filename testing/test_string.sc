using import testing
using import String

# raw string blocks
###################

# keeps first trailing LF
test
    "hello world\n" == """"hello world

# doesn't escape anything
test
    "hello \"world\"\\n\n" == """"hello "world"\n

# sub 4-space indentation is illegal
#test
    "hello world\nwhat's up\n" == """"hello world
                                     what's up

# all indentation up to 4-spaces is trimmed
test
    "hello world\nwhat's up\n" == """"hello world
                                      what's up

# empty first line isn't trimmed
test
    ==
        """"
            hi everyone
        "\nhi everyone\n"

# nested use of string block token has no effect
test
    ==
        "\"\"\"\"\n\"\"\"\"\n    \"\"\"\"\n"
        """"""""
            """"
                """"

# multiline block with indented lines
test
    ==
        """"first line
            second line

            third "line"
                fourth line

        # all string compares are done at runtime
        "first line\nsecond line\n\nthird \"line\"\n    fourth line\n"

# global strings
###################

let str = (sc_global_string_new "te\x00st\n" 6)
run-stage;
print (sizeof str)
#let C = (include "stdio.h")
let C =
    include
        """"extern int printf (const char *fmt, ...);
C.extern.printf str
print (imply str rawstring)
for k in str
    print k

# strings objects
###################

do
    local s : String
    test ((countof s) == 0)
    test ((s @ 0) == 0:i8)
    s = "salvage"
    test ((countof s) == 7)
    test (s == "salvage")
    test (s != "solvager")
    test (s != "solvag")
    test (s != "salvate")
    test (s >= "salvage")
    test (s > "Salvage")
    test (s < "zal")
    s = ("test" as rawstring)
    test ((countof s) == 4)
    s ..= s
    test (s == "testtest")
    let t = (copy s)
    for a b in (zip s t)
        test (a == b)
    test (s == t)
    test ((hash s) == (hash t))
    local q : String = "init"
    test (q == "init")
    test ((String "test") == "test")
    'append q "tini"
    test (q == "inittini")
    'append q (String "init")
    test (q == "inittiniinit")
    ;

do
    test ((String "the" "quick" "brown" "fox") == "thequickbrownfox")
    test ((String (String "the") "quick" (String "brown") "fox") == "thequickbrownfox")

do
    local s : String "abcd"
    'emplace-append-many s 4 101:i8
    test (s == "abcdeeee")
    ;

# testing proper globalization
local s = (String "test")
run-stage;
test (s == "test")

# test zero termination
do
    let strlen = (extern 'strlen (function usize rawstring))
    fn verify-zero-terminated (s)
        viewing s
        #print (countof s) ('capacity s)
        test ((s @ (countof s)) == 0:i8) "string isn't zero terminated"
        test ((countof s) == (strlen s)) "countof != strlen"
        test ((countof s) != ('capacity s)) "countof == capacity"

    verify-zero-terminated (String "test")
    local q : String
    verify-zero-terminated q
    'append q "0123456789"
    verify-zero-terminated q
    local p = (copy q)
    test (p == "0123456789")
    verify-zero-terminated p
    'resize q 4
    verify-zero-terminated q
    'pop q
    verify-zero-terminated q
    'remove q 1
    verify-zero-terminated q
    'clear q
    verify-zero-terminated q
    ;

do
    # collecting into a mutable string
    using import itertools
    local src = (String "test")
    ->> src
        filter
            (x) -> (x > 101)
        map
            (el) -> el
        view
            local dst : String "foo"
    test (dst == "footst")


# \ followed by \n is a line continuation
test
    ==
        "the\
        quick\
        brown\
        \
        fox\
"
        "thequickbrownfox"

;