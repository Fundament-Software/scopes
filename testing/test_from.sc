

let T =
    do
        let k = (typename-type "T")
        set-type-symbol! k 'A i32
        set-type-symbol! k 'B u32
        typefn k 'test () 303
        k

from T let A B test
assert (A == i32)
assert (B == u32)
assert ((test) == 303)
