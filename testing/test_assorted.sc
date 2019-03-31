assert ((min integer i8) == i8)
assert ((max 3 4 5) == 5)

# make sure we can load symbols from the global C namespace
let C =
    extern 'sc_write
        function void string

run-stage;

dump C
C "hello from C!\n"

unlet C
run-stage;

assert
    ==
        require-from module-dir '.module2
        require-from module-dir '.module2

do
    using import .module2
    assert
        7 == (compute 4 3)

let a = 1
let a = (a + 1)
assert (a == 2)

do
    let x y z = 0 1 2
    assert
        and
            x == 0
            y == 1
            z == 2
        "multideclaration failed"

assert
    ==
        and
            do
                print "and#1"
                true
            do
                print "and#2"
                true
            do
                print "and#3"
                true
            do
                print "and#4"
                false
            do
                error "should never see this"
                false
        false
    "'and' for more than two arguments failed"

assert
    ==
        or
            do
                print "or#1"
                false
            do
                print "or#2"
                false
            do
                print "or#3"
                false
            do
                print "or#4"
                true
            do
                error "should never see this"
                true
        true
    "'or' for more than two arguments failed"

true