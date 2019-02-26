
inline unpack-type (f)
    fn (...)
        ((f ...) as type)

@@ unpack-type
@@ memoize
fn memoized ()
    typename "bang"

@@ unpack-type
@@ memoize
fn memoized-clone ()
    typename "bang"

fn not-memoized ()
    typename "T"

assert
    (not-memoized) != (not-memoized)

assert
    (memoized) == (memoized)

assert
    (memoized) != (memoized-clone)

@@ unpack-type
@@ memoize
fn memoized2 (x)
    sc_typename_type x

assert
    (memoized2 "test") == (memoized2 "test")
assert
    (memoized2 "test") != (memoized2 "test2")

true
