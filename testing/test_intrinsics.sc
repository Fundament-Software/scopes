
assert
    (sin 0.0) == 0.0

assert
    (cos 0.0) == 1.0

assert
    (sin (unconst 0.0)) == 0.0

assert
    (cos (unconst 0.0)) == 1.0

assert
    (abs -3.5) == 3.5
assert
    (abs (unconst -3.5)) == 3.5

assert
    (sign -3.5) == -1.0
assert
    (sign (unconst -3.5)) == -1.0
assert
    (sign (unconst 3.5)) == 1.0
assert
    (sign (unconst 0.0)) == 0.0

assert
    (sqrt 9.0) == 3.0
assert
    (sqrt (unconst 9.0)) == 3.0

assert
    (length (vectorof f32 2.0 6.0 9.0)) == 11.0

assert
    (length (unconst (vectorof f32 2.0 6.0 9.0))) == 11.0

