
using import testing

do
    inline f (x)
        fn ff (y)
            add x y

    fn q ()
        (f 2) 3

    assert ((q) == 5)

do
    inline f (x)
        inline ff (y)
            add x y

    fn q ()
        (f 2) 3

    assert ((q) == 5)

fn test2 ()
    let quit = (ptrtoref (private bool))
    let event = (ptrtoref (alloca i32))
    event = 1
    inline handle_events ()
        loop ()
            if (event != 0)
                if (event == 1)
                    quit = true
                break;
    inline mainloop ()
        loop ()
            if quit
                break;
            handle_events;
    mainloop;

fn test3 ()
    fn handle_events ()
        # force void return signature
        if false
            return;
        if true
            io-write! "\n"
        else
            handle_events;
    handle_events;

dump-spice
    typify test3

test2;

true