

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

# let as last statement in a body returns none
assert
    ==
        call
            fn ()
                let x = 5
        none



