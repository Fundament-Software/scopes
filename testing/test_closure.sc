
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
        if (event != 0)
            if (event == 1)
                quit = true
        else
            handle_events;
    inline mainloop ()
        if (not quit)
            handle_events;
            mainloop;
    mainloop;

fn test3 ()
    fn handle_events ()
        if (unconst true)
            io-write! "\n"
        else
            handle_events;
    handle_events;

#dump-label
    Closure-label test3

dump-label
    typify test3
#compile
    typify test3

test2;
