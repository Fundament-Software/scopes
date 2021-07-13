
using import testing

spice-quote
    inline test-switch-quote (x)
        switch x
        spice-unquote
            _
                sc_case_new `0 `(print "zero")
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

using import switcher

# define and run the switch case
call
    switcher sw
        case 0
            print "zero"
        va-map
            inline (i)
                case i
                    print (tostring i)
            va-range 1 5
        default
            # this-condition and context... are implicitly bound
            # prints "???" <some number> "extra argument"
            print "???" this-condition switcher-context...
    3

# actually runs the switch case
for i in (range 11)
    sw i "extra argument"

# extend
switcher+ sw
    va-map
        inline (i)
            case i
                print (tostring i)
        va-range 5 10

# actually runs the switch case
for i in (range 11)
    sw i "extra argument"

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

switcher get-name
    default "???"
switcher get-arguments
    default -1

inline define-op (id name argcount)
    switcher+ get-name
        case id name
    switcher+ get-arguments
        case id argcount

# modular definitions
define-op 1 "foo" 3
define-op 2 "bar" 6

fn print-info (id)
    print "name:" (get-name id) "args:" (get-arguments id)

print-info 1 # prints name: foo args: 3
print-info 2 # prints name: bar args: 6
print-info 3 # prints name: ??? args: -1

test ((get-arguments 2) == 6)
test ((get-arguments 3) == -1)
test (constant? (get-arguments 2))


