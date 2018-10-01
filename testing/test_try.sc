do
    let ok =
        try
            print 1
            if true
                error! "runtime error"
            print 2
            true
        except (exc)
            print
                format-error exc
            false
    assert (ok == false)

fn test-loop-xp ()
    loop (counter) = 0
    if (counter == 10)
        return;
    try
        if (counter == 5)
            error! "loop error"
        print "success branch" counter
        # todo: when types don't merge, return void
        _;
    except (exc)
        print "fail branch" counter exc
        print
            format-error exc
        assert (counter == 5)
        _;
    repeat (counter + 1)

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
