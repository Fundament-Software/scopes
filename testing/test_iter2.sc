
using import testing

fn do_loop (n)
    #for x y in (zip (range n) (range 0 100 2))
        print x y
    for x in (range n)
        if ((x % 2) == 1)
            continue;
        print x
    print "done"

fn main ()
    do_loop 10

main;

#do
    inline fold (init gen f)
        let iter start = ((gen as Generator))
        loop (result next = init start)
            inline _break ()
                break result
            let next args... = (iter _break next)
            repeat
                f _break result args...
                next

    let k =
        fold
            0
            range 1 10
            inline (break a b)
                print a b
                a + b

do
    let x = 1
    let q =
        fold (x) for c in (range 1 10)
            if (c == 5)
                # we can also break and repeat with a new value for x
                # but continue is useful when you just want to skip to the next
                # generator item without updating x
                continue;
            x * c

    assert (q == 72576)

let a b = 2 3
fold (a b) for i in (range 3)
    test (a == 2)
    test (b == 3)
    _ a b

let x y =
    fold (x y = 0 0) for c in (range 10)
        _ (x + c) (y - c)

assert (x == 45)
assert (y == -45)

test ((countof (range 3)) == 3)

;