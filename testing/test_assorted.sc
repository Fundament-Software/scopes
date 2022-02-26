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

true