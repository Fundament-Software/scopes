
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
