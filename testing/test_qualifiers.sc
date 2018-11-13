
let x = i32
let x = (sc_view_type x 0)
let x = (sc_move_type x)
let x = (sc_view_type x 1)
let x = (sc_move_type x)
let x = (sc_key_type 'TEST x)
let x = (sc_key_type 'test x)
let x = (sc_mutate_type x)
let x = (sc_refer_type x 0:u64 unnamed)

print x

inline local (T)
    ptrtoref (alloca T)

inline new (T)
    ptrtoref (malloc T)

fn test ()
    let a = (new i32)
    a = 3
    let b = (new i32)
    b = 7
    let c = (local i32)
    c = a + b
    print c
    return;

dump-ast
    typify test

#fn test2 ()
    let c = (tupleof 1 (tupleof 2 3) true)
    let k = (local (typeof c))
    k = c
    assert ((@ k 0) == 1)
    return;

test;
#test2;

true