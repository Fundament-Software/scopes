
fn memoized ()
    typename-type "bang"

fn memoized2 (x)
    typename-type x

assert
    (memoized) == (memoized)

assert
    (memoized2 "test") == (memoized2 "test")
