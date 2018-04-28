
# `local` creates a stack variable of reference type
let x = (local 'copy 5)
assert (x == 5)
x = 10              # references support assignment operator
assert (x == 10)

let y = (local 'copy 2)
let z = (local 'copy 12)
assert ((x + y) == z) # references pass-through overloadable operators

assert ((typeof (@ y)) == i32)

# bind same reference to different name via let
let w = y
# copy by value to a new, independent reference
let z = (local 'copy y)
y = 3
assert (y == 3)
assert (z == 2)
assert (w == y)

# loop with a mutable counter
let i = (local 'copy 0)
let loop ()
if (i < 10)
    i = i + 1
    loop;
assert (i == 10)

# declare unsized mutable array on stack; the size can be a variable
let y = (local 'copy 5)
let x = (alloca-array i32 (y as immutable))
x @ 0 = 1
x @ 1 = x @ 0 + 1
x @ 2 = x @ 1 + 1
x @ 3 = x @ 2 + 1
x @ 4 = x @ 3 + 1
assert ((x @ 4) == 5)



