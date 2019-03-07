
using import Capture

do
    # immutable capture

    let a = 10
    let b = 20
    let f =
        capture [a b] (u v)
            print a b u v
            + a b u v

    assert ((f 1 2) == (+ 10 20 1 2))


let T = (array i32 16)
let f =
    spice-capture [T] ()
        dump T
        `(nullof T)

run-stage;

print (f)