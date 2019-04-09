
# test if dots expand correctly in expression list
let k = (Scope)
'set-symbol k 'x true

run-stage;

fn X ()
    if k.x true
    else false

assert (X)

fn split-test (expr)
    let a b = (symbol-handler (list expr) (__this-scope))
    '@ a

let a =
    tupleof
        b =
            tupleof
                c = 5

report a.b.c

assert
    ==
        split-test '...
        '...
assert
    ==
        split-test 'args.
        'args.
assert
    ==
        split-test 'args..
        'args..
assert
    ==
        split-test 'args...
        'args...
assert
    ==
        split-test '.args
        '.args
assert
    ==
        split-test '..args
        '..args
assert
    ==
        split-test '...args
        '...args

assert
    ==
        split-test 'a.b.c
        '(. a.b c)
assert
    ==
        split-test 'a..b..c
        'a..b..c
assert
    ==
        split-test 'a...b...c
        'a...b...c
assert
    ==
        split-test '.a.b.c.
        '(. .a.b c.)
assert
    ==
        split-test '..a..b..c..
        '..a..b..c..
assert
    ==
        split-test '...a...b...c...
        '...a...b...c...
