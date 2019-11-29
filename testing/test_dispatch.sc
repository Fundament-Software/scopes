
using import testing

#do
    let m =
        fn-dispatcher
            call
                type-matcher
                    x = i32; y = i32; z = i32
                fn (a b c)
                    add a (add b c)
            call
                type-matcher f32 f32 f32
                fn (a b c)
                    fadd a (fadd b c)
            call
                type-matcher string string string
                fn (a b c)
                    .. a b c

    let x = (local 'copy 1)
    assert ((m x 2 3) == 6)
    assert ((m 1.0 2.0 3.0) == 6.0)
    assert ((m "a" "b" "c") == "abc")

do
    fn... add3
    case (a : i32, b : i32, c : i32)
        add a (add b c)
    case (a : f32, b : f32, c : f32)
        fadd a (fadd b c)
    case (a : string, b : string, c : string)
        sc_string_join a (sc_string_join b c)

    # reference type will be implicitly converted to value type
    let x = (ptrtoref (alloca i32))
    x = 1
    test ((add3 x 2 3) == 6)
    test ((add3 1.0 2.0 3.0) == 6.0)
    test ((add3 "a" "b" "c") == "abc")

    #dump
        add3 true

do
    # recursive dispatch
    fn... div2
    case (a : f32, b : f32)
        fdiv a b
    case (a : i32, b : i32)
        this-function (f32 a) (f32 b)
    case (a : i32, b : f32)
        this-function (f32 a) b
    case (a : f32, b : i32)
        this-function a (f32 b)

    fn unconst (x) x

    test ((div2 (unconst 4.0) (unconst 2.0)) == 2.0)
    test ((div2 (unconst 4  ) (unconst 2  )) == 2.0)
    test ((div2 (unconst 4.0) (unconst 2  )) == 2.0)
    test ((div2 (unconst 4  ) (unconst 2.0)) == 2.0)


false

