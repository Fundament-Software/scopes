
let tuple_i32_i32_i32 =
    tuple i32 i32 i32

let tuple_empty =
    tuple;

let i32x3 = (array i32 3)

run-stage;

do
    let val =
        Value
            tupleof
                1
                2
                3
    assert (('typeof val) == tuple_i32_i32_i32)
    let val = (val as tuple_i32_i32_i32)
    assert ((val @ 0) == 1)
    assert ((val @ 1) == 2)
    assert ((val @ 2) == 3)

do
    let val =
        Value
            vectorof f32 1 2.0 3
    print val

do
    let val =
        Value
            arrayof i32 1 2 3
    let arrayT = i32x3
    assert (('typeof val) == arrayT)
    let val = (val as arrayT)
    assert ((val @ 0) == 1)
    assert ((val @ 1) == 2)
    assert ((val @ 2) == 3)

do
    let val =
        Value (tupleof)
    assert (('typeof val) == tuple_empty)
    let val = (val as tuple_empty)
    print val
