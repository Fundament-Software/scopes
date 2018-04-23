
# various ways to create arrays

# an array initialization template
fn init-array (vals)
    """"initializes the mutable array-like `vals` with four integer elements
        0 1 2 and 3
    for k in (range 4)
        vals @ k = k

# an array checking template
fn check-array (vals)
    """"checks whether the array-like `vals` is carrying four integer elements
        defined as 0 1 2 and 3
    for k in (range 4)
        assert ((vals @ k) == k)

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
    fn vals ()
        return 0 1 2 3
    # bind to vararg name
    let vals... = (vals)
    for k in (range 4)
        assert ((va@ vals... k) == k)
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
        let a b c d = (unpack (unconst vals))
        check-vals a b c d

do
    # uninitialized mutable array on stack with constant length
    let vals = (alloca-array i32 4)
    # init the array
    init-array vals
    # then check the values
    check-array vals

do
    # uninitialized mutable array on stack with dynamic length
    let count = (unconst 4) # simulate a runtime value
    let vals = (alloca-array i32 count)
    init-array vals
    check-array vals

do
    # uninitialized mutable array on heap with dynamic length
    let count = (unconst 4) # simulate a runtime value
    let vals = (malloc-array i32 count)
    init-array vals
    check-array vals
    # don't forget to free
    free vals



none
