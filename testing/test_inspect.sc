
using import testing
using import enum
import inspect

# test introspection

fn somefunc (a b)
    print "somefunc called"
    if a
        return 1
    else
        return b

somefunc := `[(static-typify somefunc bool i32)]
assert ((tostring ('kind somefunc)) == "Function")
assert ((tostring ('kind i32)) == "IntegerType")

block := 'function-get-body somefunc

for instr in block
    print ('kind instr)
    switch ('kind instr)
    case ValueKind.Call
        print "   " ('icall-callee instr)
        for arg in ('icall-args instr)
            print "   " arg
        print (countof ('icall-exception-body instr))
        try
            print ('icall-exception-type instr)
        except (e)
            print "no exception type"
    default;
print ('kind ('terminator block))

;