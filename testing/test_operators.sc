
using import testing

assert ((- 5) == -5)

assert ((/ 4) == 0.25)

assert ((- 5.0) == -5.0)
assert ((/ 4.0) == 0.25)

assert-compiler-error
    do
        let x = (+ 1)

