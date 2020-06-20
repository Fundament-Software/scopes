using import testing

do
    let ok =
        try
            print 1
            if true
                error "runtime error"
            print 2
            true
        except (exc)
            print
                'format exc
            false
    assert (ok == false)

fn test-loop-xp ()
    loop (counter = 0)
        if (counter == 10)
            return;
        try
            if (counter == 5)
                error "loop error"
            print "success branch" counter
            true
        except (exc)
            print "fail branch" counter exc
            print
                'format exc
            assert (counter == 5)
            false
        counter + 1

test-loop-xp;

do
    # test void exception type
    try
        print "in try"
        raise;
    except (err)
        static-assert (none? err)
        print "in except"

    fn raise-void ()
        print "in raise-void"
        raise;

    fn try-catch-void ()
        try
            raise-void;
        except ()
            print "caught the void"

    try-catch-void;

do
    # test try/else
    fn test-try-else (x)
        try
            if (x == 1)
                raise true
            elseif (x == 2)
                raise (One 100)
            elseif (x == 3)
                error "error"
            else
                true
        else # catch-all permits polymorphic raise type
            print "error occurred with x =" x
            false

    test-try-else 0
    test-try-else 1
    test-try-else 2
    test-try-else 3
    One.test-refcount-balanced;


do
    # re-raising uniques
    try
        try
            if true
                let one =
                    try (One 100)
                    else
                        assert false "error creating One"
                        unreachable;
                raise one
        except (err)
            raise err
    except (err)
        ;

    One.test-refcount-balanced;
