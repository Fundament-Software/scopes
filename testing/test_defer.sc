
fn do-some-stuff (use-branch)
    # when (do-some-stuff) exits, pass the return
      arguments to this function
    defer
        fn (...)
            print "do-some-stuff returned " ...
            # pass through return arguments
            ...
    print "do-some-stuff called"
    print "returning from do-some-stuff"
    # try to exit through multiple paths
    if use-branch
        return (unconst 2)
    unconst 1

# and of course we can use defer in the module itself
defer
    fn (...)
        print "module returned" ...
        # return something else instead
        true
assert ((do-some-stuff (unconst false)) == 1)
assert ((do-some-stuff (unconst true)) == 2)
