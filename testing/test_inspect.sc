
using import testing
import inspect

# test introspection

fn somefunc (a b)
    if a
        return 1
    else
        return b

somefunc := `[(static-typify somefunc bool i32)]

print ('kind somefunc)
#print (('kind somefunc) == value-kind-const-string)


;