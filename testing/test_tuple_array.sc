
using import testing

let tuple_i8_i32 = (tuple i8 i32)

run-stage;

do
    let k =
        arrayof i32 1 2 3

    print k
    test ((@ k 0) == 1)
    test ((@ k 1) == 2)
    test ((@ k 2) == 3)

do
    let k =
        arrayof tuple_i8_i32
            tupleof (i8 1) 4
            tupleof (i8 2) 5
            tupleof (i8 3) 6

    # since all arguments are constant, everything should be constant
    test (constant? k)

    test ((@ k 0 0) == (i8 1))
    test ((@ k 1 0) == (i8 2))
    test ((@ k 2 0) == (i8 3))

    test ((@ k 0 1) == 4)
    test ((@ k 1 1) == 5)
    test ((@ k 2 1) == 6)

do
    let a = (tupleof 1 606 true)
    let b = (tupleof 1 606 true)
    let c = (tupleof 1 606 false)

    test (a == b)
    test (b != c)

do
    let q =
        tupleof
            One 1
            One 2
            One 3
    let w = (move q)
    ;
One.test-refcount-balanced;

do
    let a b c =
        One 1
        One 2
        view (One 3)
    let q =
        tupleof a b c
    ;
One.test-refcount-balanced;

;