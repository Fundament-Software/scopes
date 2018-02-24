
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

assert ((vec4 v.xy v.xy) == v.xyxy)

assert (v + v == (vec4 0 2 4 6))
assert (v + 1 == (vec4 1 2 3 4))
assert (1 + v == (vec4 1 2 3 4))
assert (v * v / 2 == (vec4 0 0.5 2 4.5))
assert (0 - v == (vec4 0 -1 -2 -3))

assert ((max (vec2 1.0 2.0) (vec2 4.0 0.0)) == (vec2 4.0 2.0))



#compile
    typify
        fn (a b)
            a * a / b + 1
        \ vec4 vec4
    'dump-function
