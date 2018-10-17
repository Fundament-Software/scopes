
spice test (x y z args...)
    assert ((x as i32) == 1)
    assert ((y as i32) == 2)
    assert ((z as i32) == 3)
    assert (('argcount args...) == 3)
    let u v w =
        'getarg args... 0
        'getarg args... 1
        'getarg args... 2
    assert ((u as i32) == 4)
    assert ((v as i32) == 5)
    assert ((w as i32) == 6)
    `(+ x y z u v w)

compile-stage;

assert
    (test 1 2 3 4 5 6) == 21
