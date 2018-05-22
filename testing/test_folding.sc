
using import testing

inline print_stuff (x)
    print "line"
    inline ()
        print x

fn main ()
    let link = (print_stuff (unconst "hello"))
    link;

#dump-label
    typify main

main;

do
    define ascope (Scope)
    syntax-extend
        fn print_stuff2 (x)
            print "line"
            set-scope-symbol! ascope 'somefunc
                fn ()
                    print x

        fn main2 ()
            print_stuff2 (unconst "hello")
        main2;
        syntax-scope

    # this case is illegal
    assert-compiler-error
        ascope.somefunc;

true