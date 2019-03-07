
# define stack variable with type signature and init value
local a : i32 = 3:i8
# define default initialized stack variable
local b : i32
assert (b == 0)
b = 2
# infer type from initialization argument
local k = (a + b)
assert (k == 5)

true
