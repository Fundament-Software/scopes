
spice-quote
    inline test-switch-quote (x)
        switch x
        case 0
            print "!"
            print "zero"
        spice-unquote
            sc_case_new `1 `(print "one")
        spice-unquote
            let a b c =
                sc_pass_case_new `2 `(print "two or larger");
                sc_pass_case_new `3 `(print "three or larger");
                sc_pass_case_new `4 `(print "two to four");
            _ a b c
        do;
        case 5
            print "five"
        default
            print "???"

run-stage;

print
    switch 3
    case 0
        print "!"
        print "zero"
    case 1
        print "one"
    pass 2
        print "two or larger"
    pass 3
        print "three or larger"
    pass 4
        print "two to four"
    do;
    case 5
        print "five"
    default
        print "???"

print
    test-switch-quote 3

print "done."
