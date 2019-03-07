
# test if dots expand correctly in expression list
let k = (Scope)
'set-symbol k 'x true

run-stage;

fn X ()
    if k.x true
    else false

assert (X)


