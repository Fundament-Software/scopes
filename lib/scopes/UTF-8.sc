using import enum

# declare i8   @llvm.ctlz.i8  (i8   <src>, i1 <is_zero_undef>)
let llvm.ctlz.i8 =
    extern 'llvm.ctlz.i8
        function i8 i8 bool
# declare i32   @llvm.ctlz.i32  (i32   <src>, i1 <is_zero_undef>)
let llvm.ctlz.u32 =
    extern 'llvm.ctlz.i32
        function u32 u32 bool

inline ctlz (c)
    llvm.ctlz.i8 c false
inline ctlz-u32 (c)
    llvm.ctlz.u32 c false

inline encoder (coll)
    """"convert a u32 codepoint as i8 byte array chunks
    inline _encoder (coll)
        let init full? done push = ((coll as Collector))
        let tmp = (alloca-array i8 4)
        Collector init full? done
            inline (src state...)
                let c = (imply (src) u32)
                # number of bytes required
                let nm = (ctlz-u32 c)
                let bytecount =
                    if (nm >= 25:u32)
                        # 7 bits
                        tmp @ 0 = (c & 0x7f:u32) as i8
                        1
                    elseif (nm >= 21:u32)
                        # 11 bits
                        tmp @ 0 = 0xc0:i8 | ((c >> 6:u32) & 0x1f:u32) as i8
                        tmp @ 1 = 0x80:i8 | (c & 0x3f:u32) as i8
                        2
                    elseif (nm >= 16:u32)
                        # 16 bits
                        tmp @ 0 = 0xe0:i8 | ((c >> 12:u32) & 0xf:u32) as i8
                        tmp @ 1 = 0x80:i8 | ((c >> 6:u32) & 0x3f:u32) as i8
                        tmp @ 2 = 0x80:i8 | (c & 0x3f:u32) as i8
                        3
                    else
                        # 21 bits
                        tmp @ 0 = 0xf0:i8 | ((c >> 18:u32) & 0x7:u32) as i8
                        tmp @ 1 = 0x80:i8 | ((c >> 12:u32) & 0x3f:u32) as i8
                        tmp @ 2 = 0x80:i8 | ((c >> 6:u32) & 0x3f:u32) as i8
                        tmp @ 3 = 0x80:i8 | (c & 0x3f:u32) as i8
                        4
                push (inline () (_ bytecount tmp)) state...
    static-if (none? coll) _encoder
    else (_encoder coll)

inline decoder (coll)
    """"convert a i8 character stream as UTF-8 codepoints
    inline _decoder (coll)
        let init full? done push = ((coll as Collector))
        Collector
            inline ()
                # which byte we expect and the codepoint we are building
                _ 0:i8 0:u32 (init)
            inline (b cp state...)
                full? state...
            inline (b cp state...)
                done state...
            inline (src b cp state...)
                let c = (imply (src) i8)
                # full state: expected byte (bits 4-5) and leading bits (bits 0-3)
                let st = (b | (ctlz (~ c)))
                let b cp ok? result =
                    switch st
                    # expecting new codepoint, 1 byte header
                    case 0b000000:i8
                        # 7 bits, reset
                        _ 0b000000:i8 0:u32 true (c as u32) 
                    # expecting new codepoint, 2 byte header
                    case 0b000010:i8
                        # 11 bits; start with bits 6-10, expect byte 1
                        return 0b010000:i8 ((c & 0b11111:i8) as u32) state... 
                    # expecting new codepoint, 3 byte header
                    case 0b000011:i8
                        # 16 bits; start with bits 12-15, expect byte 2
                        return 0b100000:i8 ((c & 0b1111:i8) as u32) state... 
                    # expecting new codepoint, 4 byte header
                    case 0b000100:i8
                        # 21 bits; start with bits 18-20, expect byte 3
                        return 0b110000:i8 ((c & 0b111:i8) as u32) state... 
                    # expecting byte 3, cont header
                    # expecting byte 2, cont header
                    pass 0b110001:i8
                    case 0b100001:i8
                        # read 6 bits, count down by 1
                        return (b - 0b10000:i8) 
                            (cp << 6:u32) | ((c & 0b111111:i8) as u32)
                            state... 
                    # expecting byte 1, cont header
                    case 0b010001:i8
                        # read 6 bits, complete & reset
                        let cp = ((cp << 6:u32) | ((c & 0b111111:i8) as u32))
                        _ 0b000000:i8 0:u32 true cp 
                    # illegal
                    default
                        # reset
                        _ 0b000000:i8 0:u32 false (c as u32)
                return b cp (push (inline () (_ ok? result)) state...) 
    static-if (none? coll) _decoder
    else (_decoder coll)

do
    let encoder decoder
    locals;
