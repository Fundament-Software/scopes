
using import testing

# make sure static isn't memoizing the returned pointer

global samplerate = 0
global buffersize = 0

samplerate = 44100
buffersize = 4096

fn main ()
    test (samplerate == 44100)
    test (buffersize == 4096)

main;

fn local-globals (x)
    global tmp = 100
    oldx := (deref tmp)
    tmp = x
    oldx

test ((local-globals 200) == 100)
test ((local-globals 300) == 200)
test ((local-globals 400) == 300)

