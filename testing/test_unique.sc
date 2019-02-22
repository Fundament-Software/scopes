
let T = (typename "T")
'set-unique T
'set-storage T i32
'set-symbols T
    __typecall =
        inline "new" (cls id)
            bitcast id cls
    __drop =
        fn "drop" (self)
            sc_write "dropped\n"
            destroy self

run-stage;

#fn test_select (x a b)
    let a = (move a)
    let b = (move b)
    if x a
    else b

fn test (a b c)
    let a = (move a)
    let b = (move b)
    #print a
    if true a
    else b

let test = (typify test T T T)
dump-ast test

dump (typeof test)
#print ('pointer (function T (sc_view_type T 1)))

#fn test-unique ()
    do
        fn test_select (x a b)
            if x a
            else b

        #dump-ast
            typify test_select bool T T

        # output:
            test_select 1
            > 1:test
            dropped 2:test
            dropped 1:test
            test_select 2
            > 4:test
            dropped 4:test
            dropped 3:test

        print "test_select 1"
        print ">"
            test_select true (T 1) (T 2)
        print "test_select 2"
        print ">"
            test_select false (T 3) (T 4)

    do
        fn test_select (x a b)
            let a = (move a)
            let b = (move b)
            if x a
            else b

        #dump-ast
            typify test_select bool T T

        # output:
            test_select 3
            dropped 2:test
            > 1:test
            dropped 1:test
            test_select 4
            dropped 3:test
            > 4:test
            dropped 4:test

        print "test_select 3"
        print ">"
            test_select true (T 1) (T 2)
        print "test_select 4"
        print ">"
            test_select false (T 3) (T 4)
    return;

#dump-ast
    typify test-unique