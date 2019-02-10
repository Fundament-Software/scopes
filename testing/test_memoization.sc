
fn unpack-type (x) (x as type)


let memoized =
    memoize 
        fn ()
            sc_typename_type "bang"
        unpack-type

let memoized-clone =
    memoize 
        fn ()
            sc_typename_type "bang"
        unpack-type

fn not-memoized ()
    sc_typename_type "T"

assert
    (not-memoized) != (not-memoized)

assert
    (memoized) == (memoized)

assert
    (memoized) != (memoized-clone)

let memoized2 =
    memoize 
        fn (x)
            sc_typename_type x
        unpack-type

assert
    (memoized2 "test") == (memoized2 "test")
assert
    (memoized2 "test") != (memoized2 "test2")

true
