
fn memoized ()
    typename "bang" (fn ())

fn memoized-clone ()
    typename "bang" (fn ())

fn memoized2 (x)
    typename x (fn ())

fn! not-memoized ()
    typename-type "T"

assert
    (not-memoized) != (not-memoized)

assert
    (memoized) == (memoized)
assert
    (memoized) != (memoized-clone)

assert
    (memoized2 "test") == (memoized2 "test")

