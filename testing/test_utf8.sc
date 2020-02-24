

using import itertools
using import testing

import UTF-8

let srcstr = "🤔Thö Quöck Brüwn Föx🤔"
let dststr =
    ->>
        # iterate string characters
        srcstr
        # build codepoints
        UTF-8.decoder
        # filter codepoints
        map
            inline (cp)
                if (cp < 0)
                    error "illegal byte in UTF-8 stream"
                cp
        # build bytestream
        UTF-8.encoder
        # build string
        string.collector 256

print dststr
test (dststr == srcstr)


print (UTF-8.char "?")