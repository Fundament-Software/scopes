
using import testing

fn source-ui (sxitem)
    let item = (sxitem as Value)
    let T = ('typeof item)
    if (T == list)
        loop (item = (item as list))
            if (empty? item)
                return;
            let subitem = ('@ item)
            source-ui subitem
            repeat ('next item)
    return;

source-ui
    list-load module-path

fn test-inline-loop ()
    # inlines can only be recursive at compile time
    inline finite-loop (x)
        loop (x = x)
            if (x < 100)
                repeat (x + 1)
            break;
        print "done"

    finite-loop 100


test-inline-loop;

# maximum number of compile time recursions exceeded
assert-compiler-error
    do
        fn test-inline-loop ()
            inline finite-loop (x)
                if (x < 100)
                    finite-loop (x + 1)
                print "done"

            finite-loop 100
        test-inline-loop;

# maximum number of compile time recursions exceeded
assert-compiler-error
    do
        fn test-template-loop (x ...)
            if (x < 100)
                test-template-loop x true ...
            print "done"

        test-template-loop 0

fn test-late-head-recursion (x)
    # tag return type
    if false
        return 0
    if (x == 24) 1
    else
        (test-late-head-recursion (x + 1)) * 2

assert ((test-late-head-recursion 0) == 16777216)

fn test-early-head-recursion (x)
    # tag return type
    if false
        return 0
    if (x != 24)
        (test-early-head-recursion (x + 1)) * 2
    else 1

assert ((test-early-head-recursion 0) == 16777216)

fn test-hidden-infinite-recursion (x)
    if (x != 24)
        (test-hidden-infinite-recursion (x + 1)) * 2
    elseif (x != 36)
        (test-hidden-infinite-recursion (x + 1)) * 2
    else
        (test-hidden-infinite-recursion (x + 1)) * 2

# non-returning expression isn't last expression in sequence
assert-compiler-error
    do
        test-hidden-infinite-recursion 0
        print "ok"

true
