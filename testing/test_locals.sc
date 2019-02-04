
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

