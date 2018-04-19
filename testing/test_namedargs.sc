
fn somefunc(first? second? third? unmapped fourth?)
    assert (first? == 1)
    assert (second? == 2)
    assert (third? == 3)
    assert (fourth? == 4)
    assert (none? unmapped)
    return first? second? third? fourth?

let x y z w =
    somefunc 1 2
        fourth? = 4
        third? = 3

assert
    and
        x == 1
        y == 2
        z == 3
        w == 4
