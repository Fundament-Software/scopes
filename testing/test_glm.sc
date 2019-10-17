
using import testing
using import glm

do
    local v = (vec4 1 2 3 4)
    test (v.yw == (vec2 2 4))
    v.xz = (vec2 10 20)
    test (v == (vec4 10 2 20 4))
    test ((v.xz + v.yw) == (vec2 12 24))

    local v = (vec2 4 3)
    test (v.yyxx == (vec4 3 3 4 4))
    test (v.y == 3)
    v.y = 2
    test (v == (vec2 4 2))

# default init
test ((vec4) == (vec4 0 0 0 0))

test ((vec4 1 2 3 4) == (vec4 1 2 3 4))
test ((vec4 1 (vec2 2 3) 4) == (vec4 1 2 3 4))
test ((vec4 (vec3 0) 1) == (vec4 0 0 0 1))
test ((vec4 1) == (vec4 1 1 1 1))

# smear scalar by implicit cast
test ((imply 1.0 vec4) == (vec4 1 1 1 1))
# smear constant scalar of convertible type by implicit cast
let q = (imply 1 vec4)
static-assert (constant? q)
test (q == (vec4 1 1 1 1))

let v = (vec4 0 1 2 3)
test (v.xy == (vec2 0 1))
test (v.zwzw == (vec4 2 3 2 3))
test (v.z == 2.0)
test (v.xxyy == (vec4 0 0 1 1))
test (v.rgb == (vec3 0 1 2))
test (v.st == (vec2 0 1))
test (v @ 2 == 2.0)

test ((ivec3 (vec3 5)) == (ivec3 5))

test ((vec4 v.xy v.xy) == v.xyxy)

test (v + v == (vec4 0 2 4 6))
# operation with scalar of same element type
test (v + 1.0 == (vec4 1 2 3 4))
# operation with scalar of similar element type
test (v + 1 == (vec4 1 2 3 4))
test (1 + v == (vec4 1 2 3 4))
test (v * v / 2 == (vec4 0 0.5 2 4.5))
test (0 - v == (vec4 0 -1 -2 -3))
test ((- v) == (vec4 0 -1 -2 -3))

test ((- (ivec4 1 2 3 4)) == (ivec4 -1 -2 -3 -4))

test ((/ (vec4 2 4 2 4)) == (vec4 0.5 0.25 0.5 0.25))

test ((max (vec2 1.0 2.0) (vec2 4.0 0.0)) == (vec2 4.0 2.0))

for x in (vec3 1)
    test (x == 1.0)

test ((mat4 1) != (mat4 0))

test ((mat4 1) == (mat4))

test
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


test
    == m
        mat4
            \ 3 3 3 1
            \ 2 2 3 3
            \ 1 2 3 4
            \ 1 2 5 5

do
    # do identity transformations yield the original matrix?
    test 
        (m * (mat4)) == m
    test 
        ((mat4) * m * (mat4)) == m

    # matrices can be multiplied if lhs has as many columns as rhs has rows
    do
        (mat2x3) * (mat4x2)
        let m = 
            mat2x3
                vec3 2 3 5
                vec3 7 11 13
        let n = 
            mat4x2
                vec2 17 19
                vec2 23 29
                vec2 31 37
                vec2 41 43
        test
            == (m * n)
                mat4x3
                    \ 167 260 332
                    \ 249 388 492
                    \ 321 500 636
                    \ 383 596 764
        (mat3x2) * (mat4x3)
        let m = 
            mat3x2
                vec2 2 3
                vec2 5 7
                vec2 11 13
        let n = 
            mat4x3
                vec3 17 19 23
                vec3 29 31 37
                vec3 41 43 47
                vec3 53 59 61
        test 
            == (m * n)
                mat4x2
                    \ 382 483
                    \ 620 785
                    \ 814 1035
                    \ 1072 1365

    test-compiler-error
        (mat3x3) * (mat4x2)

    let m = (mat4x3 m)
    let n =
        transpose m

    test 
        ((mat3) * m * n) == (m * n)

    test
        == n
            mat3x4
                \ 3 2 1 1
                \ 3 2 2 2
                \ 3 3 3 5
    test
        ==
            m * n
            mat3x3
                \ 15 17 23
                \ 17 21 31
                \ 23 31 52
    test
        ==
            n * m
            mat4x4
                \ 27 21 18 24
                \ 21 17 15 21
                \ 18 15 14 20
                \ 24 21 20 30

    test
        (m @ 1) == (vec3 2 2 3)
    test
        ('row m 1) == (vec4 3 2 2 2)

    test ((m * (vec4 2 3 4 5)) == (vec3 21 30 52))
    test (((vec3 2 3 4) * m) == (vec4 27 22 20 28))

#let m = (mat3 m)
#print m

#compile
    typify
        fn (a b)
            a * a / b + 1
        \ vec4 vec4
    'dump-function
none
