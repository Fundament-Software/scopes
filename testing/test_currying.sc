
f := (a b c) -> (d f) -> b * f

assert (f <- (_ 1 2 3) <- (_ 4 5) == 10)

u := (a b) -> a + b; v := (a b) -> a - b; w := () -> (_ u v)

assert ((u 2 3) == 5)
assert ((v 2 3) == -1)
let p q = (w)
assert ((p 2 3) == 5)
assert ((q 2 3) == -1)

#let at next =
    decons
        sugar-quote
            u := (a b) -> a + b, v := (a b) -> a - b
#print
    parse-infix-expr (__this-scope) at next 0

op1 := (x y z) -> (_ z x)
op2 := (x) -> (_ x x 3)
op3 := (x y) -> x * y

assert ((list (op1 1 2 3)) == '(3 1))
assert ((list (op2 1)) == '(1 1 3))

assert ((op3 2 3) == 6)
