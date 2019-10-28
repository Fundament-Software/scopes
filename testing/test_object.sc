
#let x = (unconst true)
#global y = x

include (import C) "stdio.h"

let i32x10 = (array i32 10)

fn static-array-init ()
    private i32x10

compile
    typify static-array-init
    'dump-module
    'no-debug-info

global x : i32x10
# global constructor syntax
global y : i32 =
    inline ()
        C.printf "y constructor called\n"
        7

print "y=" (deref y)

fn main (argc argv)
    x @ 3 = 6
    #y = 7
    C.printf ("hello world %i %i\n" as rawstring)
        deref (x @ 3)
        deref y
    #print "hello world"
        deref (x @ 3)
        deref y
    return 0

let main = (static-typify main i32 (pointer rawstring))
#compile main
    'dump-module
    #'no-debug-info
main 0 null

print "-----------------"

let scope =
    'bind-symbols (Scope)
        main = main

compile-object
    default-target-triple
    compiler-file-kind-object
    module-dir .. "/test.o"
    scope
    #'no-debug-info
    'dump-module

# execute with
    scopes test_object.sc && gcc -o test[.exe] test.o && ./test

    output will be

    y constructor called
    hello world 6 7
