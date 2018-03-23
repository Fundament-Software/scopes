
import MutableArray

let TESTSIZE = (1 << 16)

let MAi32 = (MutableArray i32 TESTSIZE)
assert ((MutableArray i32 TESTSIZE) == MAi32)

var a = (MAi32)
for i in (range TESTSIZE)
    assert ((countof a) == (usize i))
    'append a i
for i in (range TESTSIZE)
    assert ((a @ i) == i)

