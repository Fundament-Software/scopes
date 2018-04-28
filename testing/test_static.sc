
# make sure static isn't memoizing the returned pointer

let samplerate =
    static i32 0
let buffersize =
    static i32 0

samplerate = 44100
buffersize = 4096

assert (samplerate == 44100)
assert (buffersize == 4096)
