

do
    let val =
        Any
            tupleof
                1
                unconst 2
                3
    let tupleT =
        tuple i32 i32 i32
    assert (('typeof val) == tupleT)
    let val = (val as tupleT)
    assert ((val @ 0) == 1)
    assert ((val @ 1) == 2)
    assert ((val @ 2) == 3)

do
    let val =
        Any
            vectorof f32 1 (unconst 2.0) 3
    print val

do
    let val =
        Any
            arrayof i32 1 (unconst 2) 3
    let arrayT = (array i32 3)
    assert (('typeof val) == arrayT)
    let val = (val as arrayT)
    assert ((val @ 0) == 1)
    assert ((val @ 1) == 2)
    assert ((val @ 2) == 3)

do
    let val =
        Any
            unconst (tupleof)
    let tupleT =
        tuple;
    assert (('typeof val) == tupleT)
    let val = (val as tupleT)
    print val