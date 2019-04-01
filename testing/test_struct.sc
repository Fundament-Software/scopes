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
assert (x == 1)
assert (y == 2)
assert (z == 3)
do
    # a struct with name expression
    let T =
        struct (.. "my" "struct")
            x : i32
            y : i32
    assert (('string T) == "mystruct")
    assert (('storageof T) == (tuple (x = i32) (y = i32)))

run-stage;

struct AnotherStruct plain
    x : i32
    y : i32
    z : i32

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

assert
    ('sum q) == 7

assert
    and
        q.x == 3
        q.y == 0
        q.z == 4

# init struct reference from immutable
local qq = q
assert
    and
        qq.x == 3
        qq.y == 0
        qq.z == 4

# init struct reference from other struct reference
local qqq = qq
assert
    and
        qqq.x == 3
        qqq.y == 0
        qqq.z == 4

fn test-direct-self-reference ()
    # direct self reference
    struct Cell plain
        at : i32
        next : (pointer this-type)

    run-stage;

    local cell3 = (Cell 3 null)
    local cell2 = (Cell 2 cell3)
    local cell1 = (Cell 1 cell2)

    assert
        cell1.next.next.at == 3

test-direct-self-reference;

run-stage;

do
    # forward declaration
    struct Cell

    let CellPtr =
        pointer Cell

    struct Cell plain
        at : i32
        next : CellPtr

    run-stage;

    local cell3 = (Cell 3 null)
    local cell2 = (Cell 2 cell3)
    local cell1 = (Cell 1 cell2)

    assert
        cell1.next.next.at == 3

    # using a struct on the heap
    struct Val plain
        x : i32
        y : i32

    run-stage;

    new testval = (Val 1 2)
    assert (testval.x == 1)
    assert (testval.y == 2)
    delete testval

none
