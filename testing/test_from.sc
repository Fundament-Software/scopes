
let T =
    do
        let k = (typename "T")
        'set-symbols k
            A = i32
            B = u32
            test =
                fn "test" () 303
        k

run-stage;

do
    from T let A (M = B) B test
    assert (A == i32)
    assert (B == u32)
    assert (M == B)
    assert ((test) == 303)

do
    using import struct
    using import testing

    struct Q
        value : i32

        fn set (self value)
            self.value = value

        fn get (self)
            self.value

    local t : Q
    from (methodsof t) let set get

    test ((get) == 0)
    set 100
    test ((get) == 100)



;