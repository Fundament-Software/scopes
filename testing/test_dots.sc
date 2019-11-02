
using import testing

# test if dots expand correctly in expression list
let k =
    'bind (Scope) 'x true

run-stage;

fn X ()
    if k.x true
    else false

test (X)

fn split-test (expr)
    let a b = (symbol-handler (list expr) (__this-scope))
    a

let a =
    tupleof
        b =
            tupleof
                c = 5

report a.b.c

test
    ==
        split-test '...
        '(...)
test
    ==
        split-test 'args.
        '(args.)
test
    ==
        split-test 'args..
        '(args..)
test
    ==
        split-test 'args...
        '(args...)
test
    ==
        split-test '.args
        '(.args)
test
    ==
        split-test '..args
        '(..args)
test
    ==
        split-test '...args
        '(...args)

test
    ==
        '@ (split-test 'a.b.c)
        '(. a.b c)
test
    ==
        split-test 'a..b..c
        '(a..b..c)
test
    ==
        split-test 'a...b...c
        '(a...b...c)
test
    ==
        '@ (split-test '.a.b.c.)
        '(. .a.b c.)
test
    ==
        split-test '..a..b..c..
        '(..a..b..c..)
test
    ==
        split-test '...a...b...c...
        '(...a...b...c...)
