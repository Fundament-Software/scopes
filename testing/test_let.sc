
using import testing

do
    let x = (_ 1 2 3)
    let y = (_ 4 5 6)
    let z = (_ 7 8 9)

    assert (x == 1)
    assert (y == 4)
    assert (z == 7)

do
    let x y z = 1 2 (_ 3 4 5 6 7 8 9)

    assert (x == 1)
    assert (y == 2)
    assert (z == 3)

do
    # let returns all arguments it declared
    let x y z =
        let u v w =
            _ 1 2 3

    assert (x == 1)
    assert (y == 2)
    assert (z == 3)
    assert (u == 1)
    assert (v == 2)
    assert (w == 3)

do
    # if condition block scope is accessible in subsequent blocks
    fn get () (_ true 303)
    if (let ok n = (get))
        assert (n == 303)
    else
        error! "failed"

    assert (n == 303)

# ASSERT OK: no attribute 'x in scope
assert-error
    do
        let scope =
            do
                let x y z = 5 6 7
                unlet x y z
                locals;
        scope.z

