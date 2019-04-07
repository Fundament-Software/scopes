
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
assert (scope.x as i32 == 2)
assert (scope.y as i32 == 3)
assert (scope.z as i32 == 5)
let scope = (inner-func2 2 3)
assert (scope.x as i32 == 2)
assert (scope.y as i32 == 3)
assert (scope.z as i32 == 5)

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

assert (T.x == 1)
assert (T.y == 2)
assert-compiler-error T.z

run-stage;

assert (T.z == 4)
