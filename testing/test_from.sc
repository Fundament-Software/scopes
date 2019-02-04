

let T =
    do
        let k = (typename "T")
        'set-symbols k
            A = i32
            B = u32
            test =
                fn "test" () 303
        k

compile-stage;

from T let A B test
assert (A == i32)
assert (B == u32)
assert ((test) == 303)

