do
    let ok =
        xpcall
            fn ()
                print 1
                error! "runtime error"
                print 2
                unconst true
            fn (exc)
                io-write!
                    format-exception exc
                unconst false

    assert (ok == false)

fn test-loop-xp ()
    let loop (counter) = (unconst 0)
    if (counter == 10)
        return;
    xpcall
        inline fn ()
            if (counter == 5)
                error! "loop error"
            print "success branch" counter
            return;
        inline fn (exc)
            print "fail branch" counter exc
            io-write!
                format-exception exc
            assert (counter == 5)
            return;
    loop (counter + 1)

test-loop-xp;
#let f =
    as
        compile
            typify test-loop-xp
            'dump-module
            'no-debug-info
        pointer
            function void

#f;
