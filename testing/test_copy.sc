
using import struct
using import testing

struct K
    x : i32
    y : i32

local k : K
#
let q = k.x
let q2 = (copy q)
drop k
# copy of q still accessible
test (q2 == 0)

test-compiler-error
    do
        let x = (K)
        copy x

# One can be copied now
test-compiler-error
    do
        local x = (K)
        copy &x

test (constant? (copy none))


;