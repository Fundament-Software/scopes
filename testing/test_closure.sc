
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
        let x = (unconst x)
        inline ff (y)
            let y = (unconst y)
            add x y

    fn q ()
        (f 2) 3

    assert ((q) == 5)

fn test2 ()
    let quit =
        static 'copy false
    let event = (local 'copy 1)
    inline handle_events ()
        loop;
        if (event != 0)
            if (event == 1)
                quit = true
        else
            repeat;
    inline mainloop ()
        loop;
        if (not quit)
            handle_events;
            repeat;
    mainloop;

fn test3 ()
    fn handle_events ()
        if (unconst true)
            io-write! "\n"
        else
            handle_events;
    handle_events;

'dump
    typify test3

test2;
