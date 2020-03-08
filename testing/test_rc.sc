
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
        test ((Rc.refcount k) == 1)
        let k2 = (Rc.clone k)
        test ((Rc.refcount k) == 2)
    test ((Rc.refcount k) == 1)

    test (k * 2 == 6)

    k = 12
    test (k == 12)

    let q = ((Rc.view k) * 2)
    test (q == 24)

    dump a b

    let c = ((Rc vec3) 1 2 3)

    print c
    test (c.xz == (vec2 1 3))

    ;

One.test-refcount-balanced;

;