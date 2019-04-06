
let x... = (_ 1 (_ 2 (_ 3 4)))
assert ((list x...) == '(1 2 3 4))

fn test-va (x y args...)
    let a b c d e = args...
    assert (none? e)
    assert (a == 1)
    assert (b == 2)
    assert (c == 3)
    assert (d == 4)
    assert (x == 5)
    assert (y == 0)

test-va 5 0 1 2 3 4

fn test-keys (x y z w)
    assert (x == 1)
    assert (y == 2)
    assert (z == 3)
    assert (w == 4)

test-keys (z = 3) (y = 2) 1 4
test-keys (z = 3) 1 (y = 2) 4
test-keys 1 (z = 3) 2 4

fn test-kwva-keys (x y args...)
    let a b c d e = args...
    assert (none? e)
    assert (a == 1)
    assert (b == 2)
    assert (c == 3)
    assert (d == 4)
    assert ((va-option z args... -1) == 1)
    assert ((va-option w args... -1) == 4)
    assert (x == 5)
    assert (y == 0)

test-kwva-keys
    call
        fn () # pass-through
            return (x = 5) (y = 0) (z = 1) 2 3 (w = 4)

test-kwva-keys (x = 5) (y = 0) (z = 1) 2 3 (w = 4)
test-kwva-keys 5 0 (z = 1) 2 3 (w = 4)

do
    # verify that trailing keyed arguments persist even for unknown arguments

    fn test (args...)
        let k =
            va-option x args...
                assert false
        assert ((typeof k) == i32)
        return;

    inline test2 (args...)
        let k =
            va-option x args...
                assert false
        assert ((typeof k) == i32)
        return;

    test
        x = 0

    test2
        x = 0

do
    # verify that single keyed arguments can be forwarded

    fn test2 (args...)
        let x = (va-option x args... (assert false))
        let y = (va-option y args... (assert false))
        assert ((typeof x) == i32)
        assert ((typeof y) == i32)
        return;

    fn test (x...)
        test2 x...
            y = 2
    test
        x = 1

