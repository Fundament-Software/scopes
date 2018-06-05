
fn main ()
    fn testf (a b)
        + a b

    # unconst forces a translation to a C function pointer
    let f = (unconst (typify testf i32 i32))
    assert ((f 2 3) == 5)
    unconst true

'dump
    typify main


#fn testfunc (x y)
    x * y
let lib =
    import-c "callback.c"
        """"
            typedef int (*testfunc)(int x, int y);
            int call_testfunc(testfunc f, int x, int y) {
                return f(x,y);
            }
        '()

fn testf (x y)
    + x y

let z =
    lib.call_testfunc testf 2 3
print z
assert (z == 5)

