
using import testing

fn inner-func (x y)
    let z = (x + y)
    locals;

fn inner-func2 (x y)
    do
        let x y
        let z = (x + y)
        locals;

let scope = (inner-func 2 3)
test (scope.x as i32 == 2)
test (scope.y as i32 == 3)
test (scope.z as i32 == 5)
let scope = (inner-func2 2 3)
test (scope.x as i32 == 2)
test (scope.y as i32 == 3)
test (scope.z as i32 == 5)

let locs =
    do
        let x = 1
        let y = 2
        let z = 3
        locals;

dump locs

let T = (typename "test")
typedef+ T
    let x y = 1 2
    let z = (1 + 3)

static-assert (T.x == 1)
static-assert (T.y == 2)
static-assert (T.z == 4)

