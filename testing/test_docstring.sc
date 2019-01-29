""""a module docstring

let scope =
    syntax-eval syntax-scope

""""a docstring for x
let x = 5

""""a docstring for y
let y = 6

assert
    ('docstring scope 'x) == "a docstring for x\n"
assert
    ('docstring scope 'y) == "a docstring for y\n"
print ('docstring scope unnamed)
#FIXME
assert
    ('docstring scope unnamed) == "a module docstring\n"

let scope =
    do
        # reimports should also import docstrings
        let x y

        let scope =
            syntax-eval syntax-scope

        assert
            ('docstring scope 'x) == "a docstring for x\n"
        assert
            ('docstring scope 'y) == "a docstring for y\n"
        # locals should also export docstrings
        locals;

assert
    ('docstring scope 'x) == "a docstring for x\n"
assert
    ('docstring scope 'y) == "a docstring for y\n"

fn testfun (x y)
    """"a function docstring
    true

assert (('docstring testfun) == "a function docstring\n")

true
