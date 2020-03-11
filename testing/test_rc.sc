
# refcounting

using import testing
using import Rc
using import glm

do
    let a = (Rc.wrap (One 303))
    let b = (Rc.new One 303)

    'check a

    let k = ((Rc i32) 3)
    report k
    dump k

    do
        test ((Rc.strong-count k) == 1)
        let k2 = (Rc.clone k)
        test ((Rc.strong-count k) == 2)
    test ((Rc.strong-count k) == 1)

    test (k * 2 == 6)

    k = 12
    test (k == 12)

    let q = ((Rc.view k) * 2)
    test (q == 24)

    dump a b

    let c = ((Rc vec3) 1 2 3)

    test ((Rc.strong-count c) == 1)
    test ((Rc.weak-count c) == 0)

    let w = (c as Weak)

    test ((Rc.strong-count c) == 1)
    test ((Rc.weak-count c) == 1)
    test ((Rc.strong-count w) == 1)
    test ((Rc.weak-count w) == 1)

    let v = ('upgrade w)
    let p = ('unwrap v)

    test ((Rc.strong-count c) == 2)
    test ((Rc.weak-count c) == 1)
    test ((Rc.strong-count w) == 2)
    test ((Rc.weak-count w) == 1)
    test ((Rc.strong-count p) == 2)
    test ((Rc.weak-count p) == 1)

    print c
    test (c.xz == (vec2 1 3))
    drop c
    drop v

    test ((Rc.strong-count w) == 0)
    test ((Rc.weak-count w) == 1)

    test (not ('upgrade w))

    ;

One.test-refcount-balanced;

# recursive declarations
do
    using import struct
    struct Node
        parent : (Rc this-type)

;