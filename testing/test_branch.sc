
using import testing

# impure compile time operations are only clearly determinable after the
    all-true blocks of a function have gone through, and their
    processing order isn't always clear
let T = (typename "test")
fn test ()
    # force branching
    if (unconst true)
        dump "branch 1"
        set-type-symbol! T 'x true
    else
        dump "branch 2"
        set-type-symbol! T 'x false
    if (unconst true)
        dump "branch 1"
        set-type-symbol! T 'x true
    else
        dump "branch 2"
        set-type-symbol! T 'x false
let x ok = (type@ T 'x)
assert (not ok)
test;
assert (T.x == false)

# attempting to retype branching expression
assert-compiler-error
    do
        fn test (x)
            print
                if x
                    true
                else
                    1

        test (unconst true)

# attempting to return polymorphic results from runtime-conditional branches
assert-compiler-error
    do
        fn test ()
            let k =
                if (unconst false)
                    dump "one"
                    (unconst 1)
                elseif (unconst true)
                    dump "two"
                    (unconst none)
                elseif (unconst false)
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