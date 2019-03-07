
fn test-match (q)
    let x = 3
    let y = 4
    let z = 5
    match q
    case (or x y z) 0
    case 2 1
    default 2

assert ((test-match 0) == 2)
assert ((test-match 1) == 2)
assert ((test-match 2) == 1)
assert ((test-match 3) == 0)
assert ((test-match 4) == 0)
assert ((test-match 5) == 0)

true
