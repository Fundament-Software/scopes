
inline debugprint (f)
    inline (...)
        print (f ...)

inline multiply (z)
    inline (f)
        inline (x y)
            (f x y) * z

@@ debugprint
@@ multiply 2
fn test (x y)
    x + y

assert ((test 1 2) == 6)


typedef T
    inline replace-result (f)
        @@ spice-quote
        inline (cls x)
            print f
            x + 300

    @@ replace-result
    inline __typecall (cls x)
        compiler-error! "should not see me"

    unlet replace-result

vvv print
2 + 3

do
    inline incby1x3 (x y z)
        _
            x + 1; y + 1; z + 1

    @@ print
    @@ incby1x3
    let x y z = 3 4 5

    assert (x == 4)
    assert (y == 5)
    assert (z == 6)

do
    inline incby1 (x)
        x + 1

    @@ print
    @@ incby1
    let
        x = 3
        y = 4
        z = 5

    assert (x == 4)
    assert (y == 5)
    assert (z == 6)

run-stage;

assert ((T 3) == 303)

