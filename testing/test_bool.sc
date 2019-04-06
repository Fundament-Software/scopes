
assert
    do
        if none false
        else true

static-assert
    do
        static-if none false
        else true

assert
    do
        if -1 true
        else false

assert
    do
        if 0 false
        else true

assert (none or true)
assert (0 or true)


