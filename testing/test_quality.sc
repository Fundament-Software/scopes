
# we used to assert that the complexity wasn't bigger than x, but now we
    track any change, better or worse, as changes are suspicious.

fn assert-depth (n f types...)
    let typed-f =
        typify f types...
    let c = (Label-countof-reachable typed-f)
    if (c != n)
        'dump typed-f
        let anchor = (Label-anchor typed-f)
        syntax-error! anchor
            .. "label complexity mismatch: " (repr c) " != " (repr n)

assert-depth 2:usize
    fn (a b)
        a + b
    \ i32 i32

assert-depth 2:usize
    fn ()
        print "hello world"

assert-depth 10:usize
    fn ()
        loop (i) = 0
        if (i < 16)
            repeat (i + 1)
        return;
