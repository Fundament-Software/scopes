
using import testing

local x = 0
let y = (atomicrmw add (& x) 5)
test (y == 0)
let y = (atomicrmw add (& x) 10)
test (y == 5)
test (x == 15)
let z zs = (cmpxchg (& x) 5 20)
test (not zs)
test (z == 15)
let z zs = (cmpxchg (& x) 15 20)
test zs
test (z == 15)
test (x == 20)

;