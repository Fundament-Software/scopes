
using import glm

assert ((vec4 1 2 3 4) == (vec4 1 2 3 4))
assert ((vec4 1 (vec2 2 3) 4) == (vec4 1 2 3 4))
assert ((vec4 (vec3 0) 1) == (vec4 0 0 0 1))
assert ((vec4 1) == (vec4 1 1 1 1))

let v = (vec4 0 1 2 3)
assert (v.xy == (vec2 0 1))
assert (v.zwzw == (vec4 2 3 2 3))
assert (v.z == 2.0)
assert (v.xxyy == (vec4 0 0 1 1))
assert (v.rgb == (vec3 0 1 2))
assert (v.st == (vec2 0 1))
assert (v @ 2 == 2.0)

assert ((vec4 v.xy v.xy) == v.xyxy)

assert (v + v == (vec4 0 2 4 6))
assert (v + 1 == (vec4 1 2 3 4))
assert (1 + v == (vec4 1 2 3 4))
assert (v * v / 2 == (vec4 0 0.5 2 4.5))
assert (0 - v == (vec4 0 -1 -2 -3))

assert ((max (vec2 1.0 2.0) (vec2 4.0 0.0)) == (vec2 4.0 2.0))

for x in (vec3 1)
    assert (x == 1.0)

assert ((mat4 1) != (mat4 (unconst 0)))

assert ((mat4 1) == (mat4))

assert
    ==
        mat4
            mat2 10
        mat4
            \ 10 0 0 0
            \ 0 10 0 0
            \ 0 0 1 0
            \ 0 0 0 1

let m =
    mat4
        vec3 3; 1
        vec2 2; vec2 3
        vec4 1 2 3 4
        \ 1 2 (vec2 5)

assert
    == m
        mat4
            \ 3 3 3 1
            \ 2 2 3 3
            \ 1 2 3 4
            \ 1 2 5 5

do
    let m = (mat4x3 m)
    let n =
        transpose m
    assert
        == n
            mat3x4
                \ 3 2 1 1
                \ 3 2 2 2
                \ 3 3 3 5
    assert
        ==
            m * n
            mat3x3
                \ 15 17 23
                \ 17 21 31
                \ 23 31 52
    assert
        ==
            n * m
            mat4x4
                \ 27 21 18 24
                \ 21 17 15 21
                \ 18 15 14 20
                \ 24 21 20 30

    assert
        (m @ 1) == (vec3 2 2 3)
    assert
        ('row m 1) == (vec4 3 2 2 2)

    assert ((m * (vec4 2 3 4 5)) == (vec3 21 30 52))
    assert (((vec3 2 3 4) * m) == (vec4 27 22 20 28))


#let m = (mat3 m)
#print m

#compile
    typify
        fn (a b)
            a * a / b + 1
        \ vec4 vec4
    'dump-function
none