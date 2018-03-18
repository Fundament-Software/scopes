
let S =
    CStruct "TheCoolStruct"
        x = f32
        y = f32
        z = f32

let s =
    S 1 2 3

assert
    and
        s.x == 1
        s.y == 2
        s.z == 3

fn begin-arg ()
let val =
    fn (f)
        f;

fn append-val (prevf x)
    fn (f)
        prevf
            fn ()
                return x (f)

let val = (append-val val 1)
let val = (append-val val 2)
let val = (append-val val 3)

print (val begin-arg)

struct AnotherStruct
    x : i32
    y : i32
    z : i32

    method 'apply-type (cls x y z)
        'structof cls
            x = x
            y = y
            z = z

    method 'sum (self)
        + self.x self.y self.z

let q =
    AnotherStruct 3 0 4

assert
    ('sum q) == 7

assert
    and
        q.x == 3
        q.y == 0
        q.z == 4

do
    # direct self reference
    struct Cell
        at : i32
        next : (pointer Cell)

    var cell3 = (Cell 3 null)
    var cell2 = (Cell 2 cell3)
    var cell1 = (Cell 1 cell2)

    assert
        cell1.next.next.at == 3

do
    # forward declaration
    struct Cell

    let CellPtr =
        pointer Cell

    struct Cell
        at : i32
        next : CellPtr

    var cell3 = (Cell 3 null)
    var cell2 = (Cell 2 cell3)
    var cell1 = (Cell 1 cell2)

    assert
        cell1.next.next.at == 3


# using a struct on the heap
struct Val
    x : i32
    y : i32
fn new (val)
    let mval = (malloc (typeof val))
    let mval = (('from-pointer-type reference (typeof mval)) mval)
    mval = val
    mval

let testval =
    new (Val 1 2)
assert (testval.x == 1)
assert (testval.y == 2)


none