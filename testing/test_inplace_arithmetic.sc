
using import testing

local x = 5

x += 7
assert (x == 12)
x -= 3
assert (x == 9)

fn assignment-error ()
    local x = 3
    for i in (range 3:usize)
        x = i

# error: can't apply assignment with values of types i32 and usize
test-compiler-error (assignment-error)

