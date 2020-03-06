
# refcounting

using import testing
using import RC
using import glm

do
    let a = (RC.wrap (One 303))
    let b = (RC.new One 303)

    'check a

    let k = ((RC i32) 3)
    report k
    dump k

    do
        test ((RC.refcount k) == 1)
        let k2 = (RC.clone k)
        test ((RC.refcount k) == 2)
    test ((RC.refcount k) == 1)

    test (k * 2 == 6)

    k = 12
    test (k == 12)

    let q = ((RC.view k) * 2)
    test (q == 24)

    dump a b

    let c = ((RC vec3) 1 2 3)

    print c
    test (c.xz == (vec2 1 3))

    ;

One.test-refcount-balanced;

;