
struct union K
    key : u32
    vals : (array u8 4:usize)

var x = (K)

x.vals = (arrayof u8 0xa0 0xb0 0xc0 0xd0)
assert
    x.key == 0xd0c0b0a0:u32


