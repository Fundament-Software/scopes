
spice test ()
    spice kwok ()
        ast-quote
            1 + 2

    ast-quote
        fn ()
            print
                kwok;

compile-stage;

print
    ((test))

return;
