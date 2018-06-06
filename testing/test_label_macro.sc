
fn test (l)
    for k v in ('arguments l)
        print k v
    print ('continuation l)
    'return! l 4 5 6
    l

syntax-extend
    set-scope-symbol! syntax-scope 'test
        compile (typify test Label)
    syntax-scope

assert ((storageof LabelMacro) == (typeof test))

let test =
    bitcast test
        LabelMacro

let a b c = (test 1 2 3)

assert
    and
        a == 4
        b == 5
        c == 6

true



