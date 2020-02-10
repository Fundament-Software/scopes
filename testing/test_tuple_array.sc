
using import testing

let tuple_i8_i32 = (tuple i8 i32)

run-stage;

do
    let k =
        arrayof i32 1 2 3

    print k
    assert ((@ k 0) == 1)
    assert ((@ k 1) == 2)
    assert ((@ k 2) == 3)

do
    let k =
        arrayof tuple_i8_i32
            tupleof (i8 1) 4
            tupleof (i8 2) 5
            tupleof (i8 3) 6

    assert ((@ k 0 0) == (i8 1))
    assert ((@ k 1 0) == (i8 2))
    assert ((@ k 2 0) == (i8 3))

    assert ((@ k 0 1) == 4)
    assert ((@ k 1 1) == 5)
    assert ((@ k 2 1) == 6)

do
    let a = (tupleof 1 606 true)
    let b = (tupleof 1 606 true)
    let c = (tupleof 1 606 false)

    test (a == b)
    test (b != c)

