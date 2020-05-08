
using import testing

test
    (sin 0.0) == 0.0

test
    (cos 0.0) == 1.0

test
    2.0 ** 4.0 == 16.0

test
    (trunc 3.5) == 3.0

test
    (abs -3.5) == 3.5

test
    (sign -3.5) == -1.0
test
    (sign 3.5) == 1.0
test
    (sign 0.0) == 0.0

test
    (sqrt 9.0) == 3.0

test
    (length (vectorof f32 2.0 6.0 9.0)) == 11.0

test
    (length (normalize (vectorof f32 2.0 6.0 9.0))) == 1.0

test
    (distance (vectorof f32 10.0 10.0 10.0) (vectorof f32 8.0 4.0 1.0)) == 11.0

test
    all?
        (cross (vectorof f32 0 0 1) (vectorof f32 0 1 0)) == (vectorof f32 -1 0 0)

test ((fmix -1.0 1.0 0.75) == 0.5)

test
    all?
        ((fmix (vectorof f32 1.0 -1.0) (vectorof f32 -1.0 1.0) (vectorof f32 0.75 0.75)) == (vectorof f32 -0.5 0.5))

test ((step 0.0 0.5) == 1.0)
test
    all?
        ((step (vectorof f32 0.0 1.0) (vectorof f32 0.5 0.5)) == (vectorof f32 1.0 0.0))
