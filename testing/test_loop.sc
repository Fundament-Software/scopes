using import testing

# default form
let x =
    do
        loop (i k = 0 64)
            # last line in loop is fed back into loop
            # breaks must be explicit
            if (i < k)
                _ (i + 1) k
            else
                break i
test (x == 64)

# while loop
local i = 10
while (i != 0)
    i = i - 1
    if (i == 3)
        continue;
    print i

# infinite loop
do
    local x = 0
    loop ()
        if (x < 100)
            x += 1
            repeat;
        else
            break;

fn test_2d_loops ()
    let w h = 4 4

    # method 1: two nested loops using generators
    for y in (range h)
        for x in (range w)
            print x y

    print;

    # method 2: two basic nested loops (permitting immutable state changes)
    loop (y = 0)
        if (y < h)
            do
                loop (x = 0)
                    if (x < w)
                        print x y
                        repeat (x + 1)
                    else
                        break;
            repeat (y + 1)
        else
            break;

    print;

    # method 3: single loop, two counters
    if (h <= 0)
        return;
    let y1 = (h - 1)
    loop (x y = 0 0)
        if (x < w)
            print x y
            repeat (x + 1) y
        elseif (y < y1)
            repeat 0 (y + 1)
        else
            break;

    print;

    # method 4: single loop, one counter
    let size = (w * h)
    loop (i = 0)
        if (i < size)
            let x y = (i % w) (i // h)
            print x y
            repeat (i + 1)
        else
            break;

test_2d_loops;

# returning from an inner loop
fn looper ()
    for i in (range 32)
        if (i == 5)
            return false
    true

test (not (looper))

test-compiler-error
    embed
        # loop that never breaks
        loop (i = 0)
            repeat (i + 1)
        true

true
