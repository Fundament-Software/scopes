
fn test-va (x y args...)
    assert ((va-countof args...) == 4)
    assert ((va@ 0 args...) == 1)
    assert ((va@ 1 args...) == 2)
    assert ((va@ 2 args...) == 3)
    assert ((va@ 3 args...) == 4)
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
    assert ((va-countof args...) == 4) (va-countof args...)
    assert ((va@ 0 args...) == 1)
    assert ((va@ 1 args...) == 2)
    assert ((va@ 2 args...) == 3)
    assert ((va@ 3 args...) == 4)

    assert ((va@ 'z args...) == 1)
    assert ((va@ 'w args...) == 4)
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
        let k = (va@ 'x args...)
        assert ((typeof k) == i32)
        return;

    test
        x = 0
    test
        x = (unconst 0)

do
    # verify that single keyed arguments can be forwarded

    fn test2 (args...)
        let x = (va@ 'x args...)
        let y = (va@ 'y args...)
        assert ((typeof x) == i32)
        assert ((typeof y) == i32)
        return;

    fn test (x...)
        test2 x...
            y = (unconst 2)
    test
        x = (unconst 1)

