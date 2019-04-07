
inline unpack-type (f)
    fn (...)
        ((f ...) as type)

@@ unpack-type
@@ memoize
fn memoized ()
    typename.type "bang" typename

@@ type-factory
fn memoized-clone ()
    typename.type "bang" typename

fn not-memoized ()
    typename.type "T" typename

assert
    (not-memoized) != (not-memoized)
assert
    (memoized) == (memoized)

assert
    (memoized) != (memoized-clone)

@@ unpack-type
@@ memoize
fn memoized2 (x)
    sc_typename_type x typename

assert
    (memoized2 "test") == (memoized2 "test")
assert
    (memoized2 "test") != (memoized2 "test2")

true
