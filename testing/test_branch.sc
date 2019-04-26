
using import testing

let T = (typename "test")

run-stage;

# impure compile time operations are only clearly determinable after the
    all-true blocks of a function have gone through, and their
    processing order isn't always clear
fn testf ()
    # force branching
    if true
        dump "branch A1"
        'define-symbol T 'x 0
    else
        dump "branch A2"
        'define-symbol T 'x 1
    if true
        dump "branch B1"
        'define-symbol T 'x 2
    else
        dump "branch B2"
        'define-symbol T 'x 3
sugar-eval
    test
        do
            try
                let x = ('@ T 'x)
                false
            except (x)
                true
testf;
test (T.x == 3)

# attempting to retype branching expression
test-compiler-error
    do
        fn test (x)
            print
                if x
                    true
                else
                    1

        test true

# attempting to return polymorphic results from runtime-conditional branches
test-compiler-error
    do
        fn test ()
            let k =
                if false
                    dump "one"
                    1
                elseif true
                    dump "two"
                    none
                elseif false
                    dump "three"
                    3
                else
                    dump "four"
                    4
            dump k
            return;

        test;
        true

true