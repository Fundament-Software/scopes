
# various ways to create arrays

let u8x4 = (array u8 4)

run-stage;

# an array initialization template
fn init-array (vals)
    """"initializes the mutable array-like `vals` with four integer elements
        0 1 2 and 3
    for k in (range 4:usize)
        vals @ k = (k as i32)

# an array checking template
fn check-array (vals)
    """"checks whether the array-like `vals` is carrying four integer elements
        defined as 0 1 2 and 3
    assert (vals @ 0 == 0)
    assert (vals @ 1 == 1)
    assert (vals @ 2 == 2)
    assert (vals @ 3 == 3)

fn check-vals (a b c d)
    assert
        and
            a == 0
            b == 1
            c == 2
            d == 3

do
    # an array without an array, encoded as a function that returns
      multiple return values.
    inline vals ()
        _ 0 1 2 3
    # bind to vararg name
    let vals... = (vals)
    va-lifold none
        inline (i key value)
            assert (i == value)
        vals...
    # unpack to individual values directly
    let a b c d = (vals)
    check-vals a b c d

do
    # array from constants
    let vals = (arrayof i32 0 1 2 3)
    # check that it contains indeed 4 elements
    assert ((countof vals) == 4:usize)
    check-array vals
    # unpack array
    do
        let a b c d = (unpack vals)
        check-vals a b c d

    do
        # array using memory interface
        local vals : u8x4
        assert ((vals @ 0) == 0:u8)
        assert ((vals @ 1) == 0:u8)
        assert ((vals @ 2) == 0:u8)
        assert ((vals @ 3) == 0:u8)
        vals = (arrayof u8 0xa0 0xb0 0xc0 0xd0)
        assert ((vals @ 0) == 0xa0:u8)
        assert ((vals @ 1) == 0xb0:u8)
        assert ((vals @ 2) == 0xc0:u8)
        assert ((vals @ 3) == 0xd0:u8)

        local vals2 = vals
        assert ((vals2 @ 0) == 0xa0:u8)
        assert ((vals2 @ 1) == 0xb0:u8)
        assert ((vals2 @ 2) == 0xc0:u8)
        assert ((vals2 @ 3) == 0xd0:u8)

    do
        # uninitialized mutable array on stack with constant length
        let vals = (alloca-array i32 4)
        # init the array
        init-array vals
        # then check the values
        check-array vals

    do
        # uninitialized mutable array on stack with dynamic length
        # simulate a runtime value
        let count = ((fn (x) x) 4)
        let vals = (alloca-array i32 count)
        init-array vals
        check-array vals

    do
        # uninitialized mutable array on heap with dynamic length
        # simulate a runtime value
        let count = ((fn (x) x) 4)
        let vals = (malloc-array i32 count)
        init-array vals
        check-array vals
        # don't forget to free
        free vals


