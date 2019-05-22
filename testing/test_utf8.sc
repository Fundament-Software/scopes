

using import itertools
using import testing

import UTF-8

inline string-buffer-sink (maxsize)
    let buf = (alloca-array i8 maxsize)
    Collector
        inline () 0
        inline (n) (n < maxsize)
        inline (n)
            sc_string_new buf (n as usize)
        inline (src n)
            let c srcbuf = (src)
            loop (i n = 0 n)
                if ((i == c) | (n == maxsize))
                    break n
                store (srcbuf @ i) (getelementptr buf n)
                _ (i + 1) (n + 1)

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
        string-buffer-sink 256

print dststr
test (dststr == srcstr)
