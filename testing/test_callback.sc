
using import testing

fn main ()
    fn testf (a b)
        + a b

    # force a translation to a C function pointer
    let f = (static-typify testf i32 i32)
    test ((f 2 3) == 5)
    true

dump-spice
    static-typify main

#fn testfunc (x y)
    x * y

vvv bind C
vvv include
""""
    typedef int (*testfunc)(int x, int y);
    int call_testfunc(testfunc f, int x, int y) {
        return f(x,y);
    }

fn testf (x y)
    + x y

let z =
    C.extern.call_testfunc testf 2 3
print z
test (z == 5)

test-error
    print
        as
            compile (typify ((fn (x) x) as Closure) i32)
            function i32 i32

do
    # calling function signatures with qualifiers
    using import Array

    let fnT =
        typeof
            static-typify
                fn (args)
                    move args
                & (Array i32)

    local arr : (Array i32)
    call
        as
            fn (args)
                move args
            fnT
        arr
;