

fn test (x)
    if x
        return true
    else
        return false

assert (constant? (test true))

assert (not (constant? (test (unconst true))))

# verify that if-branches do not duplicate
let T = (typename "test")
do
    set-type-symbol! T 'x 0
    if (unconst true)
        1
    let x = (type@ T 'x)
    let x = (x + 1)
    set-type-symbol! T 'x x
    dump "yep" x
    # if you see two yeps, the branch bifurcated the post label

let x = (type@ T 'x)
assert (x == 1)
