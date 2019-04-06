
f := (a b c) -> (d f) -> b * f

assert (f <- (_ 1 2 3) <- (_ 4 5) == 10)

op1 := (x y z) -> (_ z x)
op2 := (x) -> (_ x x 3)
op3 := (x y) -> x * y

assert ((list (op1 1 2 3)) == '(3 1))
assert ((list (op2 1)) == '(1 1 3))

assert ((op3 2 3) == 6)


