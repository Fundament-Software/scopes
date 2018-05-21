

using import testing

fn test (...)
    print "------------------"
    print ...
    print "------------------"
Label-set-inline! (Closure-label test)

fn main ()
    test (unconst 1) (unconst 2) (unconst 3)
    test (unconst 1) (unconst 2) (unconst 3)

#print
    dump-label
        Closure-label test

print
    dump-label
        typify main
