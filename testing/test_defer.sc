
using import testing

fn do-some-stuff (use-branch)
    # when (do-some-stuff) exits, execute this function
    defer
        fn ()
            print "[5] do-some-stuff returned"
    print "[1] do-some-stuff called with argument" use-branch
    do
        # defer in subscope defers until end of subscope
        # we can also just make it a single command
        defer print "[2] do-some-stuff subscope exited"
    print "[3] returning from do-some-stuff"
    # defers can be chained
    defer print "[4] do-some-stuff returned "
    # try to exit through multiple paths
    if use-branch
        return 2
    do
        1

# and of course we can use defer in the module itself
defer print "module returned"
test ((do-some-stuff false) == 1)
test ((do-some-stuff true) == 2)

fn do-some-other-stuff ()
    if true
        # can return from subscope, and chain multiple defers
        defer print "[2] do-some-other-stuff returned "
        defer print "[1] do-some-other-stuff returned "
        return 303
    error "should never get here"

test ((do-some-other-stuff) == 303)

true