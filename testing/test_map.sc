
using import Map
using import testing

local map : (Map Symbol i32)
inline iter-pairs1 (f)
    f 'A 101
    f 'B 303
    f 'C 606
    f 'D 909
    f 'E 11
    f 'F 22
    f 'G 33
    f 'H 44
inline iter-pairs2 (f)
    f 'I 111
    f 'J 333
    f 'K 666
    f 'L 999
    f 'M 1111
    f 'N 2222
    f 'O 3333
    f 'P 4444
inline iter-pairs (f)
    iter-pairs1 f
    iter-pairs2 f

#print (countof map)
iter-pairs
    inline (key value)
        'set map key value
'dump map

test ('D in map)

for k v in map
    print k v

iter-pairs
    inline (key value)
        if (('getdefault map key -1) == -1)
            print key value
            error "key missing"
iter-pairs1
    inline (key value)
        'discard map key
iter-pairs1
    inline (key value)
        if (('getdefault map key -1) != -1)
            print key value
            error "key not deleted"
iter-pairs2
    inline (key value)
        if (('getdefault map key -1) == -1)
            print key value
            error "key missing"
iter-pairs2
    inline (key value)
        'discard map key
iter-pairs2
    inline (key value)
        if (('getdefault map key -1) != -1)
            print key value
            error "key not deleted"

#------------------------------------------------------------------------------

using import Set

# set set
local set : (Set i32)
iter-pairs
    inline (key value)
        'insert set value
'dump set

test (909 in set)
test (909:i16 in set)

for val in set
    print val
iter-pairs
    inline (key value)
        if (not ('in? set value))
            print value
            error "value missing"
iter-pairs1
    inline (key value)
        'discard set value
iter-pairs1
    inline (key value)
        if ('in? set value)
            print value
            error "value not discarded"
iter-pairs2
    inline (key value)
        if (not ('in? set value))
            print value
            error "value missing"

run-stage;

do
    # test for LLVM struct type conversion error
    let SomeMap = (Map i32 i32)

    # this works:
    # inline make-some-map ()
    #     (SomeMap)

    # this doesn't work:
    fn make-some-map ()
        (SomeMap)

    # this works only in some circumstances. Seems to fail on a Struct initializer, for example.
    # fn make-some-map ()
    #     local m = (SomeMap)
    #     deref m

    let sm = (make-some-map)
    ;

do
    # another test for LLVM struct type conversion error
    let SomeMap = (Map string u32)

    using import struct
    struct A
        m : SomeMap
        fn fill-m ()
            local m = (SomeMap)
            'set m "a" 0:u32
            deref m
        inline __typecall (cls)
            Struct.__typecall cls
                m = (fill-m)

    # codegen backend failure
    let a = (A)
    ;

do
    # ensure keys coerce correctly and don't hash in surprising ways, leading
    # to unexpected "key not found" situations
    local m : (Map i32 i32)
    'set m 10 1
    if (not ('in? m 10:u8))
        error "key hashed incorrectly"
;
