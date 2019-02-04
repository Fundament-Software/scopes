

let x = (ptrtoref (alloca i32))
x = 5

x += 7
assert (x == 12)
x -= 3
assert (x == 9)
