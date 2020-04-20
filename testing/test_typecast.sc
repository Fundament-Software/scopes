
# testing typecast handler installed in core.sc

# not yet implemented
#do
    fn testf (x)
        # prover will call typecast handler to attempt to streamline all return
            types. the first branch will determine the type of all other branches.
        if x
            10
        else
            10:u8

    static-typify testf

switch 0:u8
case 1
case 2
case 3
default;

;