
using import Capture

do
    # immutable capture

    let a = 10
    let b = 20

    @@ report
    capture cf (u v) {a b}
        print a b u v
        + a b u v

    fn testf (f)
        assert ((f 1 2) == (+ 10 20 1 2))

    testf cf

let T = (array.type i32 16)
spice-capture f () {T}
    dump T
    `(nullof T)

run-stage;

print (f)