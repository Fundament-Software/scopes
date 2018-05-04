
define scope syntax-scope

""""a module docstring

""""a docstring for x
let x = 5

""""a docstring for y
let y = 6

assert
    (Scope-docstring scope unnamed) == "a module docstring\n"
assert
    (Scope-docstring scope 'x) == "a docstring for x\n"
assert
    (Scope-docstring scope 'y) == "a docstring for y\n"

fn testfun (x y)
    """"a function docstring
    true

assert ((docstring testfun) == "a function docstring\n")

true
