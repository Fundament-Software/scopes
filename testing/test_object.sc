
#let x = (unconst true)
#global y = x

let C =
    import-c "libc.c"
        """"
            #include <stdio.h>
        list;

global x @ 10 : i32
#setting x here won't have any effect because the code isn't executed at compile time
#x = 10
fn main (argc argv)
    x @ 0 = 6
    C.printf "hello world %i\n" (load (x @ 0))
    return 0

let main = (typify main i32 (pointer rawstring))
compile-object
    module-dir .. "/test.o"
    scopeof
        main = main
    'no-debug-info
    'dump-module

# execute with
    scopes test_object.sc && gcc -o test test.o && ./test

    output will be

    hello world 6
