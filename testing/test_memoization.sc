
fn memoized ()
    typename "bang" (fn ())

fn memoized-clone ()
    typename "bang" (fn ())

fn memoized2 (x)
    typename x (fn ())

assert
    (memoized) == (memoized)
assert
    (memoized) != (memoized-clone)

assert
    (memoized2 "test") == (memoized2 "test")
