
spice kwok ()
    ast-quote
        1 + 2

dump kwok

spice test ()
    dump kwok
    ast-quote
        print
            kwok;

run-stage;

#print
    'ast-repr test

print
    test;

return;
