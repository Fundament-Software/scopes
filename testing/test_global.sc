
using import struct
using import testing

# make sure static isn't memoizing the returned pointer

global samplerate = 44100
global buffersize = 4096

fn main ()
    test (samplerate == 44100)
    test (buffersize == 4096)

fn main2 ()
    test (samplerate == 11024)
    test (buffersize == 256)

main;
samplerate = 11024
buffersize = 256
main2;

fn local-globals (x)
    global tmp = 100
    oldx := (deref tmp)
    tmp = x
    oldx

test ((local-globals 200) == 100)
test ((local-globals 300) == 200)
test ((local-globals 400) == 300)

struct GlobalVal
    value = -1

fn struct-access ()
    global gv : GlobalVal 10
    gv.value += 1
    deref gv.value

test ((struct-access) == 11)
test ((struct-access) == 12)
test ((struct-access) == 13)

# properly moving an object across a run-stage
local o = (One 101)
test ((One.refcount) == 1)
run-stage;
test ((One.refcount) == 1)
One.reset-refcount;

;
