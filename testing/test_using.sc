
let S =
    'bind (Scope)
        test =
            fn () true

run-stage;

do
    let pattern = "^test$"
    using S filter pattern
    test;

do
    using S
    test;

do
    S.test;

do
    using import testing

    fn res ()
        123

    vvv bind common
    do
        let a = 1
        let b = 2
        locals;

    vvv bind A
    do
        using common
        let res = 123
        locals;

    print "==== scope A ===="
    local count = 0
    for k v in A
        count += 1
        print (tostring k)
    test (count == 3)

    # scope with dynamic component
    vvv bind B
    do
        using common
        let res = (res)
        locals;

    print "==== scope B ===="
    local count = 0
    for k v in B
        count += 1
        print (tostring k)
    test (count == 3)

;