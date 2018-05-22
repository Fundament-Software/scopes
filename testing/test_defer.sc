
fn do-some-stuff (use-branch)
    # when (do-some-stuff) exits, pass the return
      arguments to this function
    defer
        fn (...)
            print "[5] do-some-stuff returned " ...
            # pass through return arguments
            ...
    print "[1] do-some-stuff called with argument" use-branch
    do
        # defer in subscope defers until end of subscope
        defer
            fn (...)
                print "[2] do-some-stuff subscope exited"
    print "[3] returning from do-some-stuff"
    # defers can be chained
    defer
        fn (...)
            print "[4] do-some-stuff returned " ...
            # pass through return arguments
            ...
    # try to exit through multiple paths
    if use-branch
        return (unconst 2)
    do
        unconst 1

# and of course we can use defer in the module itself
defer
    fn (...)
        print "module returned" ...
        # return something else instead
        true
assert ((do-some-stuff (unconst false)) == 1)
assert ((do-some-stuff (unconst true)) == 2)

fn do-some-other-stuff ()
    do
        # can return from subscope, and chain multiple defers
        defer
            fn (...)
                print "[2] do-some-other-stuff returned " ...
                # pass through return arguments
                ...
        defer
            fn (...)
                print "[1] do-some-other-stuff returned " ...
                # pass through return arguments
                ...
        return (unconst 303)
    error! "should never get here"
    return (unconst 1)

assert ((do-some-other-stuff) == 303)

do
    #   a case that failed with SCOPES_TRUNCATE_FORWARDING_CONTINUATIONS
        enabled.
    fn source-ui (sxitem level)
        #print "enter" level
        defer
            fn (...)
                #print "exit" level
                ...
        let item = (sxitem as Any)
        let T = ('typeof item)
        if (T == list)
            return;
        elseif (T == Symbol)
            _;
        else
            _;
    typify source-ui Any i32

true