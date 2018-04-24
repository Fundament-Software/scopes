#do
    let ok =
        xpcall
            fn ()
                print 1
                error! "runtime error"
                print 2
                unconst true            
            fn (exc)
                #io-write!
                    format-exception exc
                unconst false

    assert (ok == false)

fn test-loop-xp ()
    let loop (counter) = (unconst 0)
    if (counter == 10)
        return;
    xpcall
        label ()
            if (counter == 5)
                error! "loop error"
            print "success" counter
            loop (counter + 1)
        label (exc)
            print "fail"
            loop (counter + 1)

let f =
    as
        compile
            typify test-loop-xp
            'dump-module
            'no-debug-info
        pointer
            function void

dump "hi"
f;
