using import testing
using import glm
using import Box

#let val =
    Box 0
#let val2 =
    Box i32 2

#val = val2
#test (val == 2)



do
    let a = (Box.wrap (One 303))
    let b = ((Box One) 303)

    'check a

    let k = ((Box i32) 3)
    report k
    dump k

    test (k * 2 == 6)

    k = 12

    let q = ((Box.view k) * 2)
    test (q == 24)

    dump a b

    let c = ((Box vec3) 1 2 3)

    print c
    test (c.xz == (vec2 1 3))

    ;

One.test-refcount-balanced;

;