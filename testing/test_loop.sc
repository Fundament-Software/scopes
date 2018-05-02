
# explicit, unrollable form
let x =
    do
        let aloop (i k) = (unconst 0) (unconst 10)
        if (i < k)
            aloop (i + 1) k
        else i
assert (x == 10)

# non-unrolled form
let x =
    do
        loop (i k) = 0 64
        if (i < k)
            repeat (i + 1) k
        else i
assert (x == 64)

let i = (local 'copy 10)
while (i != 0)
    i = i - 1
    if (i == 3)
        continue;
    print i

# infinite loop
do
    let x =
        local i32 0
    loop () =
    if (x < 100)
        x += 1
        repeat;

# infinite loop, even shorter form
do
    let x =
        local i32 0
    loop;
    if (x < 100)
        x += 1
        repeat;

fn test_2d_loops ()
    let w h = 4 4

    # method 1: two nested loops using generators
    for y in (range h)
        for x in (range w)
            print x y

    print;

    # method 2: two basic nested loops (permitting immutable state changes)
    loop (y) = 0
    if (y < h)
        do
            loop (x) = 0
            if (x < w)
                print x y
                repeat (x + 1)
        repeat (y + 1)

    print;

    # method 3: single loop, two counters
    if (h <= 0)
        return;
    let y1 = (h - 1)
    loop (x y) = 0 0
    if (x < w)
        print x y
        repeat (x + 1) y
    elseif (y < y1)
        repeat 0 (y + 1)

    print;

    # method 4: single loop, one counter
    let size = (w * h)
    loop (i) = 0
    if (i < size)
        let x y = (i % w) (i // h)
        print x y
        repeat (i + 1)

test_2d_loops;
