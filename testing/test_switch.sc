
spice-quote
    inline test-switch-quote (x)
        switch x
        case 0
            print "!"
            print "zero"
        spice-unquote
            sc_case_new `1 `(print "one")
        [
            sc_pass_case_new `2 `(print "two or larger");
            sc_pass_case_new `3 `(print "three or larger");
            sc_pass_case_new `4 `(print "two to four");
        ]
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

# indirect case generation; does not support `pass` or `default`
# this example prints "3"
print
    switch 3
    embed
        va-lfold ()
            inline (k v ...)
                _ v
                    inline ()
                        print (tostring v)
                    static-if (none? (_ ... ()))
                    else
                        ...
            va-range 6
    default
        print "???"

print "done."
