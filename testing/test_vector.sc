
let mask = (sugar-eval (vectorof i32 7 5 3 1))

fn do-ops (v w)
    v + w * w

fn test-vector-ops ()
    let v = (vectorof i32 10 20 (unpack (vectorof i32 30 40)))
    let w = (vectorof i32 1 2 3 4)
    assert
        all?
            (do-ops v w) == (vectorof i32 11 24 39 56)
    assert
        all?
            (shufflevector v w mask) == (vectorof i32 4 2 40 20)

let VT = (vector i32 4:usize)

# working with variables
test-vector-ops;

assert ((length (vectorof f32 2 10 11)) == 15.0)

assert
    all?
        ==
            ? true
                vectorof i32 1 2 3 4
                vectorof i32 5 6 7 8
            vectorof i32 1 2 3 4

assert
    all?
        ==
            ? (vectorof bool true false true false)
                vectorof i16 1 0 3 0
                vectorof i16 0 2 0 4
            vectorof i16 1 2 3 4
