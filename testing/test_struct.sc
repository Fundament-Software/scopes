
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

do
    # a struct with name expression
    let T =
        struct (.. "my" "struct")
            x : i32
            y : i32
    assert (constant? T)
    assert (type? T)
    dump T

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
    let mval = (reference.from-pointer mval)
    mval = val
    mval

fn delete (val)
    free (bitcast val (storageof (typeof val)))

let testval =
    new (Val 1 2)
assert (testval.x == 1)
assert (testval.y == 2)
delete testval

none