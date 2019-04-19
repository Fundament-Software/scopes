
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

inline g (x y)
    dump x y
    _ 1 2 (typename "Q")

@@ memo
inline f (q x y)
    q x y

let a b t1 = (f g 2 3)
let c d t2 = (f g 2 3)
static-assert (t1 == t2)

# single recursive memoization

@@ memo
inline frec0 (x)
    dump "0" x
    static-if (x < 10)
        (memo frec0) (const.add.i32.i32 x 1)
    else x

frec0 1
frec0 5
frec0 7

# cross recursive memoization

inline frec2

@@ memo
inline frec (x)
    dump "1" x
    static-if (x < 10)
        (memo frec2) (const.add.i32.i32 x 1)
    else x

inline frec2 (x)
    dump "2" x
    static-if (x < 10)
        (memo frec) (const.add.i32.i32 x 1)
    else x

frec 7
dump "x"
frec 5
dump "x"
frec 1

true
