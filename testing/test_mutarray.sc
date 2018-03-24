
import MutableArray

let TESTSIZE = (1:usize << 16:usize)

fn autodelete (x)
    fn (...)
        'delete x
        ...

do
    # mutable array with fixed upper capacity
    let i32Arrayx65536 = (MutableArray i32 TESTSIZE)
    var a = (i32Arrayx65536)
    defer (autodelete a)
    for i in (range TESTSIZE)
        assert ((countof a) == i)
        'append a (i32 i)
    for i in (range TESTSIZE)
        assert ((a @ i) == (i32 i))
    # generator support
    for i k in (enumerate a)
        assert ((a @ i) == i)

do
    # mutable array with dynamic capacity
    let i32Array = (MutableArray i32)
    var a = (i32Array 12)
    defer (autodelete a)
    assert (a.capacity == 12:usize)
    for i in (range TESTSIZE)
        assert ((countof a) == i)
        'append a (i32 i)
    for i in (range TESTSIZE)
        assert ((a @ i) == (i32 i))
    # generator support
    for i k in (enumerate a)
        assert ((a @ i) == i)

do
    # array of array
    let i32Array = (MutableArray i32 16)
    let i32ArrayArray = (MutableArray i32Array)
    var a = (i32ArrayArray)
    # will also delete the nested array
    defer (autodelete a)
    for x in (range 16)
        let b =
            'emplace-append a
        for y in (range 16)
            'append b (x * 16 + y)
    #
    print a
    for x b in (enumerate a)
        print b
        for y n in (enumerate b)
            assert ((x * 16 + y) == n)
