
spice gen-label-merge-test ()
    ast-quote
        fn (k str)
            .. "test1-"
                label test3
                    .. "test2-"
                        label test2
                            .. "test3-"
                                label test1
                                    switch k
                                    case 0
                                        merge test1 str
                                    case 1
                                        merge test2 str
                                    case 2
                                        merge test3 str
                                    default
                                        return "?"

compile-stage;

let select1 = (gen-label-merge-test)
fn select2 (k str)
    .. "test1-"
        label test3
            .. "test2-"
                label test2
                    .. "test3-"
                        label test1
                            switch k
                            case 0
                                merge test1 str
                            case 1
                                merge test2 str
                            case 2
                                merge test3 str
                            default
                                return "?"


inline testf (select)
    assert ((select 0 "zero") == "test1-test2-test3-zero")
    assert ((select 1 "one") == "test1-test2-one")
    assert ((select 2 "two") == "test1-two")
    assert ((select 3 "three") == "?")

testf select1
testf select2
