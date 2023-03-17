
using import testing
using import enum
import inspect

# test introspection

fn somefunc (a b)
    if a
        return 1
    else
        return b

somefunc := `[(static-typify somefunc bool i32)]

assert ((tostring ('kind somefunc)) == "Function")

assert ((tostring ('kind i32)) == "IntegerType")

;