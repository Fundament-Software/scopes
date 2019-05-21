
using import enum

# declare i8   @llvm.ctlz.i8  (i8   <src>, i1 <is_zero_undef>)
let llvm.ctlz.i8 =
    extern 'llvm.ctlz.i8
        function i8 i8 bool

enum UTF8
    # a codepoint has been read successfully
    Codepoint : u32
    # an illegal byte was encountered
    IllegalByte : i8
    # a byte from the middle of a stream was encountered
    Partial : i8
    # the stream ended before the codepoint was complete
    Incomplete

inline decoder (gen)
    """"Decode a i8 character stream encoded as UTF-8 as UTF8 enum value
    let init valid? at next = ((gen as Generator))

    Generator
        inline ()
            init
        inline (state...)
            valid? state...
        inline (state...)
            let c = ((at state...) as i8)
            switch (llvm.ctlz.i8 (~ c) true)
            case 0
                # 1 byte
                c as u32
            case 1
                # in the middle of a stream
                raise (UTF8Error.Incomplete c)
            case 2
                # 2 bytes

            case 3
                # 3 bytes
            case 4
                # 4 bytes

            default
                # illegal
                raise (UTF8Error.IllegalByte c)
        next

for ch in (decoder "Thö Quöck Brüwn Föx")
    print ch
