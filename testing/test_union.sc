
using import struct

do
    struct K union
        key : u32
        vals : (array u8 4)

    local x = (K)

    x.vals = (arrayof u8 0xa0 0xb0 0xc0 0xd0)
    assert
        x.key == 0xd0c0b0a0:u32

    let x = (K)
    assert (x.key == 0:u32)
    assert ((x.vals @ 0) == 0:u8)

do
    # union within a struct
    struct Color union
        rgba : u32
        c : (array u8 4)

    struct Leaf
        color : Color

    run-stage;

    local leaf = (Leaf)
    leaf.color.c = (arrayof u8 0xa0 0xb0 0xc0 0xd0)
    assert
        leaf.color.rgba == 0xd0c0b0a0:u32

