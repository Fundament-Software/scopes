
using import testing

fn source-ui (sxitem)
    let item = (sxitem as Any)
    let T = ('typeof item)
    if (T == list)
        let loop (item) = (item as list)
        if (empty? item)
            return;
        let subitem = ((list-at item) as Syntax)
        source-ui subitem
        loop (list-next item)
    #else
        print item
    return;

source-ui
    list-load module-path

fn test-inline-loop ()
    # inlines can only be recursive at compile time
    inline finite-loop (x)
        loop (x) = x
        if (x < 100)
            repeat (x + 1)
        print "done"

    finite-loop (unconst 100)


test-inline-loop;

assert-compiler-error
    do
        fn test-inline-loop ()
            # inlines can only be recursive at compile time
            inline finite-loop (x)
                if (x < 100)
                    finite-loop (x + 1)
                print "done"

            finite-loop (unconst 100)
        test-inline-loop;


