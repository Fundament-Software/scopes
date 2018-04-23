
do
    struct union K
        key : u32
        vals : (array u8 4:usize)

    let x = (local K)

    x.vals = (arrayof u8 0xa0 0xb0 0xc0 0xd0)
    assert
        x.key == 0xd0c0b0a0:u32

    let x = (K)
    dump x.key
    dump
        x.vals @ 0

do
    # union within a struct
    struct union Color
        rgba : u32
        c : (array u8 4:usize)

    struct Leaf
        color : Color

    let leaf = (local Leaf)
    leaf.color.c = (arrayof u8 0xa0 0xb0 0xc0 0xd0)
    assert
        leaf.color.rgba == 0xd0c0b0a0:u32
