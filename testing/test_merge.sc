

fn test (x)
    if x
        return true
    else
        return false

assert (constant? (test true))

assert (not (constant? (test (unconst true))))

# verify that if-branches do not duplicate
do
    let T = (typename "test")
    set-type-symbol! T 'x 0
    if (unconst true)
        1
    let x = (type@ T 'x)
    set-type-symbol! T 'x (x + 1)
    dump "yep" x
    # if you see two yeps, the branch bifurcated the post label
    assert (x == 1)
