
using import testing

local x = 0
let y = (atomicrmw add (& x) 5)
test (y == 0)
let y = (atomicrmw add (& x) 10)
test (y == 5)
test (x == 15)

;