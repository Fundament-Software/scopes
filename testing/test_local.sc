
using import testing

# define stack variable with type signature and init value
local a : i32 = 3:i8
# define default initialized stack variable
local b : i32
assert (b == 0)
b = 2
# infer type from initialization argument; both `=` and `:=` are accepted
local k := (a + b)
assert (k == 5)

# local ... = auto-wraps the right hand side, accepts multiple left hand arguments
local x = + 1 2
test (x == 3)
local x2 : i32 = + 1 2
test (x2 == 3)
test ((local : i32 = + 1 2) == 3)

# local ... = also supports block sugars like `if`
# right hand side is evaluated in same scope
local x := if ((local z := 10) == 10)
    20
else
    30
test (and (x == 20) (z == 10))

true
