
import MutableArray

let TESTSIZE = (1 << 16)

fn autodelete (x)
    fn (...)
        'delete x
        ...

do
    # mutable array with fixed upper capacity
    var a = ((MutableArray i32 TESTSIZE))
    defer (autodelete a)
    for i in (range TESTSIZE)
        assert ((countof a) == (usize i))
        'append a i
    for i in (range TESTSIZE)
        assert ((a @ i) == i)

do
    # mutable array with dynamic capacity
    var a = ((MutableArray i32) 12)
    defer (autodelete a)
    assert (a.capacity == 12:usize)
    for i in (range TESTSIZE)
        assert ((countof a) == (usize i))
        'append a i
    for i in (range TESTSIZE)
        assert ((a @ i) == i)

