
assert
    match '(1 2 3)
        '(1 2 3) true
        else false

let x = 3
let y = 4
assert
    match 5
        (or x y 5) true
        else false
