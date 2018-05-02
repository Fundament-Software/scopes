
using import Array

let TESTSIZE = (1:usize << 16:usize)
let fullrange = (range (unconst TESTSIZE))

fn autodelete (x)
    fn (...)
        delete x
        ...

do
    # mutable array with fixed upper capacity
    let i32Arrayx65536 = (Array i32 TESTSIZE)
    let a = (local i32Arrayx65536)
    defer (autodelete a)
    for i in fullrange
        assert ((countof a) == i)
        'append a (i32 i)
    for i in fullrange
        assert ((a @ i) == (i32 i))
    # generator support
    for i k in (enumerate a)
        assert ((a @ i) == i)

do
    # mutable array with dynamic capacity
    let i32Array = (Array i32)
    let a = (local i32Array (capacity = 12))
    defer (autodelete a)
    assert (a.capacity == 12:usize)
    for i in fullrange
        assert ((countof a) == i)
        'append a (i32 i)
    for i in fullrange
        assert ((a @ i) == (i32 i))
    # generator support
    for i k in (enumerate a)
        assert ((a @ i) == i)

do
    # array of array
    let i32Array = (Array i32 16)
    let i32ArrayArray = (Array i32Array)
    let a = (local i32ArrayArray)
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

do
    # sorting a mutable array
    let T = (Array i32 32)
    let a = (local T)
    defer (autodelete a)
    for k in (va-each 3 1 9 5 0 7 12 3 99 -20)
        'append a k
    'sort a
    for i k in (enumerate (va-each -20 0 1 3 3 5 7 9 12 99))
        assert ((a @ i) == k)
    # custom sorting key
    'sort a
        fn (x)
            - x
    for i k in (enumerate (va-each 99 12 9 7 5 3 3 1 0 -20))
        assert ((a @ i) == k)

do
    let a = (local (Array string))
    defer (autodelete a)
    for k in (va-each "yes" "this" "is" "dog" "")
        'append a k
    'sort a
    for i k in (enumerate (va-each "" "dog" "is" "this" "yes"))
        assert ((a @ i) == k)
