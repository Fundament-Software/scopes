
spice kwok ()
    spice-quote
        1 + 2

dump kwok

spice test ()
    dump kwok
    spice-quote
        print
            kwok;

run-stage;

#print
    'ast-repr test

print
    test;

return;
