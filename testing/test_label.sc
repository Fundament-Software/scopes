
using import testing

spice gen-label-merge-test ()
    spice-quote
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

run-stage;

do
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
        test ((select 0 "zero") == "test1-test2-test3-zero")
        test ((select 1 "one") == "test1-test2-one")
        test ((select 2 "two") == "test1-two")
        test ((select 3 "three") == "?")

    testf select1
    testf select2

do
    # forward label form

    :: ok
    merge ok 1 2 3
    ok (x y z) ::
    print x y z

    let select1 = (gen-label-merge-test)
    fn select2 (k str)
        .. "test1-"
            :: test3
            .. "test2-"
                :: test2
                .. "test3-"
                    :: test1
                    switch k
                    case 0
                        merge test1 str
                    case 1
                        merge test2 str
                    case 2
                        merge test3 str
                    default
                        return "?"
                    test1 ::
                test2 ::
            test3 ::

    inline testf (select)
        test ((select 0 "zero") == "test1-test2-test3-zero")
        test ((select 1 "one") == "test1-test2-one")
        test ((select 2 "two") == "test1-two")
        test ((select 3 "three") == "?")

    testf select1
    testf select2
