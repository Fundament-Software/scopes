""""a module docstring

run-stage;

using import testing

""""a docstring for x
let x = 5

""""a docstring for y
let y = 6

let scope =
    sugar-eval sugar-scope
test
    ('docstring scope 'x) == "a docstring for x\n"
test
    ('docstring scope 'y) == "a docstring for y\n"
print ('module-docstring scope)
test
    ('module-docstring scope) == "a module docstring\n"

let scope =
    do
        # reimports should also import docstrings
        let x y

        let scope =
            sugar-eval sugar-scope

        test
            ('docstring scope 'x) == "a docstring for x\n"
        test
            ('docstring scope 'y) == "a docstring for y\n"
        # locals should also export docstrings
        locals;

test
    ('docstring scope 'x) == "a docstring for x\n"
test
    ('docstring scope 'y) == "a docstring for y\n"

fn testfun (x y)
    """"a function docstring
    true

test (('docstring testfun) == "a function docstring\n")

true
