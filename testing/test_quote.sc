
spice kwok ()
    ast-quote
        1 + 2

spice test ()
    dump kwok
    ast-quote
        print
            kwok;

#let test = (typify test)

compile-stage;

print
    'ast-repr test

#print
    test;

return;
