
# make sure static isn't memoizing the returned pointer

global samplerate = 0
global buffersize = 0

samplerate = 44100
buffersize = 4096

fn test ()
    assert (samplerate == 44100)
    assert (buffersize == 4096)

test;
