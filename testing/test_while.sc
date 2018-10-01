
let a = (alloca i32)
store 0 a
let b = (alloca i32)
store 0 b

while ((load a) != 10)
    store ((load a) + 1) a
    store (load a) b

assert ((load b) == 10)
