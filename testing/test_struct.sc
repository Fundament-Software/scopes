
using import testing
using import struct

inline begin-arg ()
let val =
    inline (f)
        f;

inline append-val (prevf x)
    inline (f)
        prevf
            inline ()
                _ x (f)

let val = (append-val val 1)
let val = (append-val val 2)
let val = (append-val val 3)

dump (val begin-arg)
let x y z = (val begin-arg)
test (x == 1)
test (y == 2)
test (z == 3)

# a struct with name expression - defined at runtime
let T =
    @@ spice-quote
    struct (.. "my" "struct")
        x : i32
        y : i32
test (not (constant? T))

run-stage;

test (('string T) == "mystruct")
test (('storageof T) == (tuple (x = i32) (y = i32)))

struct AnotherStruct plain
    x : i32
    y : i32
    z : i32

    static-assert (super-type == CStruct)

    inline __typecall (cls x z y)
        CStruct.__typecall cls
            x = x
            y = y
            z = z

    fn sum (self)
        + self.x self.y self.z

run-stage;

let q =
    AnotherStruct 3 4 0

dump q

test
    ('sum q) == 7

test
    and
        q.x == 3
        q.y == 0
        q.z == 4

# init struct reference from immutable
local qq = q
test
    and
        qq.x == 3
        qq.y == 0
        qq.z == 4

# init struct reference from other struct reference
local qqq = qq
test
    and
        qqq.x == 3
        qqq.y == 0
        qqq.z == 4

fn test-direct-self-reference ()
    # direct self reference
    struct Cell plain
        at : i32
        next : (pointer this-type)
        # field types are stored in this member for the duration of the
            declaration
        dump this-type.__fields__

    local cell3 = (Cell 3 null)
    local cell2 = (Cell 2 cell3)
    local cell1 = (Cell 1 cell2)

    test
        cell1.next.next.at == 3

test-direct-self-reference;

run-stage;

do
    # forward declaration
    struct Cell plain

    let CellPtr =
        pointer Cell

    struct Cell
        at : i32
        next : CellPtr

    local cell3 = (Cell 3 null)
    local cell2 = (Cell 2 cell3)
    local cell1 = (Cell 1 cell2)

    test
        cell1.next.next.at == 3

    let defaultcell = (Cell)
    test (defaultcell.at == 0)
    test (defaultcell.next == null)

    # using a struct on the heap
    struct Val plain
        x : i32
        y : i32

    local testval = (Val 1 2)
    test (testval.x == 1)
    test (testval.y == 2)
    drop testval

global drop_count = 0
do
    # non-plain struct inside non-plain struct
    struct Child
        value : i32

        inline __drop (self)
            drop_count += 1
            super-type.__drop self

    struct Parent
        child1 : Child
        child2 : Child

    let parent = (Parent)
    ;
test (drop_count == 2)

# default values
struct Defaults
    # member with default value
    w : i32 = 10

    # members can be defined conditionally
        but we must use the explicit form as we're not on the immediate level
    static-if false
        field 'y i8 9:i8
    else
        field 'y i32 4:i8

    # when the type is `Unknown`, it will be set from the default value
    z : Unknown = 5

    x = (1 + 2)

let val = (Defaults)
test (val.x == 3)
test (val.y == 4)
test (val.z == 5)
test (val.w == 10)

# test packing
do
    struct K plain packed
        w : i8
        k : i32

    assert ((sizeof K) == 5)


none
