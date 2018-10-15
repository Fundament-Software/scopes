
spice kwok ()
    ast-quote
        1 + 2

spice test ()
    ast-quote
        fn "demofunc" ()
            print
                kwok;

#let test = (typify test)

compile-stage;

print
    ((test))

return;
