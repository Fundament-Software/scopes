
spice kwok ()
    spice-quote
        1 + 2

dump kwok

spice test ()
    dump kwok
    spice-quote
        print
            kwok;

run-stage;

#print
    'ast-repr test

print
    test;


# flatten concatenation of arguments
let args =
    spice-quote
        _ (1 + 2) (3 + 4) (5 + 6)

print
    sc_value_kind_string
        'kind args

'dump
    spice-quote
        _ 0 args

'dump
    spice-quote
        _ args 4


using import testing

do
    let k = `5
    test (('typeof `k) == i32)

    # references should also be embedded
    local k = `5
    test (('typeof `k) == i32)

;
