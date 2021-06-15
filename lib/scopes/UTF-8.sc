#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""UTF-8
    =====

    This module provides UTF-8 encoder and decoder collectors, as well as
    a UTF-8 aware `char` function.

using import enum

# declare i8   @llvm.ctlz.i8  (i8   <src>, i1 <is_zero_undef>)
let llvm.ctlz.char =
    extern 'llvm.ctlz.char
        function char char bool
# declare i32   @llvm.ctlz.i32  (i32   <src>, i1 <is_zero_undef>)
let llvm.ctlz.u32 =
    extern 'llvm.ctlz.i32
        function u32 u32 bool

inline ctlz (c)
    llvm.ctlz.char c false
inline ctlz-u32 (c)
    llvm.ctlz.u32 c false

inline encoder (coll)
    """"Convert an integer codepoint to char bytes;
        the collector forwards a byte at a time.
    inline _encoder (coll)
        let init full? done push = ((coll as Collector))
        let tmp = (alloca-array char 4)
        Collector init full? done
            inline (src state...)
                let c = ((src) as u32)
                # number of bytes required
                let nm = (ctlz-u32 c)
                let bytecount =
                    if (nm >= 25:u32)
                        # 7 bits
                        tmp @ 0 = (c & 0x7f:u32) as char
                        1
                    elseif (nm >= 21:u32)
                        # 11 bits
                        tmp @ 0 = 0xc0:char | ((c >> 6:u32) & 0x1f:u32) as char
                        tmp @ 1 = 0x80:char | (c & 0x3f:u32) as char
                        2
                    elseif (nm >= 16:u32)
                        # 16 bits
                        tmp @ 0 = 0xe0:char | ((c >> 12:u32) & 0xf:u32) as char
                        tmp @ 1 = 0x80:char | ((c >> 6:u32) & 0x3f:u32) as char
                        tmp @ 2 = 0x80:char | (c & 0x3f:u32) as char
                        3
                    else
                        # 21 bits
                        tmp @ 0 = 0xf0:char | ((c >> 18:u32) & 0x7:u32) as char
                        tmp @ 1 = 0x80:char | ((c >> 12:u32) & 0x3f:u32) as char
                        tmp @ 2 = 0x80:char | ((c >> 6:u32) & 0x3f:u32) as char
                        tmp @ 3 = 0x80:char | (c & 0x3f:u32) as char
                        4
                loop (i state... = 0 state...)
                    if (i == bytecount)
                        break state...
                    let c = (deref (tmp @ i))
                    _ (i + 1)
                        push (inline () c) state...
    static-if (none? coll) _encoder
    else (_encoder coll)

let BYTE_STEP = (1:u32 << 30:u32)
let BYTE_MASK = (BYTE_STEP | (BYTE_STEP << 1:u32))
inline decoder (coll)
    """"Convert a char character stream as UTF-8 codepoints of type i32.
        Invalid bytes are forwarded as negative numbers; negating the number
        yields the offending byte character.
    inline _decoder (coll)
        let init full? done push = ((coll as Collector))
        Collector
            inline ()
                #   bits 30-31: which nth byte we expect,
                    bits 0-20: the codepoint we are building
                    every time we append to the codepoint,
                    the upper bits are cleared implicitly
                _ 0:u32 (init)
            inline (cp state...)
                full? state...
            inline (cp state...)
                done state...
            inline (src cp state...)
                let c = (imply (src) char)
                # full state: expected byte (bits 30-31) and leading bits (bits 0-3)
                let b = (cp & BYTE_MASK)
                let st = (b | (ctlz (~ c)))
                let skip =
                    inline "#hidden" (cp)
                        return cp state...
                let cp result =
                    switch st
                    # expecting new codepoint, 1 byte header
                    case 0:u32
                        # 7 bits, reset
                        _ 0:u32 (c as u32)
                    # expecting new codepoint, 2 byte header
                    case 2:u32
                        # 11 bits; start with bits 6-10, expect byte 1
                        skip (BYTE_STEP | ((c & 0b11111:char) as u32))
                    # expecting new codepoint, 3 byte header
                    case 3:u32
                        # 16 bits; start with bits 12-15, expect byte 2
                        skip ((BYTE_STEP * 2:u32) | ((c & 0b1111:char) as u32))
                    # expecting new codepoint, 4 byte header
                    case 4:u32
                        # 21 bits; start with bits 18-20, expect byte 3
                        skip ((BYTE_STEP * 3:u32) | ((c & 0b111:char) as u32))
                    # expecting byte 3, cont header
                    # expecting byte 2, cont header
                    pass ((BYTE_STEP * 3:u32) | 1:u32)
                    pass ((BYTE_STEP * 2:u32) | 1:u32)
                    # AFTER: `pass` fallthrough sections must end in a `do` or
                        `default` block, which allows us to prefix all switch
                        options with `pass`.
                    do
                        # read 6 bits, count down by 1
                        skip ((b - BYTE_STEP) | (cp << 6:u32) |
                            ((c & 0b111111:char) as u32))
                    # expecting byte 1, cont header
                    case (BYTE_STEP | 1:u32)
                        # read 6 bits, complete & reset
                        let cp = ((cp << 6:u32) | ((c & 0b111111:char) as u32))
                        _ 0:u32 cp
                    # illegal
                    default
                        # reset
                        _ 0:u32 (- (c as u32))
                return cp (push (inline () (result as i32)) state...)
    static-if (none? coll) _decoder
    else (_decoder coll)

spice char32 (value)
    using import itertools
    let value =
        match ('typeof value)
        case Symbol (value as Symbol as string)
        default (value as string)
    local result = 0:i32
    local stored = false
    ->> value decoder
        map
            inline (cp)
                if (cp < 0)
                    error "illegal byte in UTF-8 stream"
                if stored
                    error "string contains more than one UTF-8 character"
                result = cp
                stored = true
                ;
        drain
    result

do
    let encoder decoder char32
    locals;
