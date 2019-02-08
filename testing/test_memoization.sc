
fn memoized ()
    typename "bang"

fn memoized-clone ()
    typename "bang"

fn memoized2 (x)
    typename x

inline not-memoized ()
    typename "T"

assert
    (not-memoized) != (not-memoized)

assert
    (memoized) == (memoized)

assert
    (memoized) != (memoized-clone)

assert
    (memoized2 "test") == (memoized2 "test")

true
