

using import testing

fn test (...)
    print "------------------"
    print ...
    print "------------------"
# programmatically changing a function to inline
Label-set-inline! (Closure-label test)

fn gen-function ()
    fn () true

# ok: function returns closure
assert (((gen-function)) == true)

inline gen-function2 ()
    fn () true

# ok: inline returns closure
assert (((gen-function2)) == true)

inline gen-function3 (x)
    fn () x

# ok: function in inline captures constant outside scope
assert (((gen-function3 true)) == true)

# error: function in inline captures variable outside scope
assert-compiler-error
    ((gen-function3 (unconst true))) == true

inline gen-function4 (x)
    inline () x

# ok: inline captures variable outside scope
assert (((gen-function4 (unconst true))) == true)

fn gen-function5 (x)
    fn () x

# ok: function in inline captures constant outside scope
assert (((gen-function5 true)) == true)

# error: function in inline captures variable outside scope
assert-compiler-error
    ((gen-function5 (unconst true))) == true


fn rebuild (x)
    if (list-empty? x)
        tie-const x '()
    else
        let at rest = (decons x)
        cons (at as i32 + 1)
            rebuild rest

let l = (quote 1 2 3 4 5)
let m = (rebuild l)
assert (constant? m)
# compile time recursion fine
assert (m == '(2 3 4 5 6))
# run time recursion fine
assert ((rebuild (unconst l)) == '(2 3 4 5 6))

inline rebuild2 (x)
    if (list-empty? x)
        tie-const x '()
    else
        let at rest = (decons x)
        cons (at as i32 + 1)
            rebuild2 rest

let m = (rebuild2 l)
assert (constant? m)
# compile time recursion fine
assert (m == '(2 3 4 5 6))
# error: non-tail recursion not possible with inlines
assert-compiler-error
    assert ((rebuild2 (unconst l)) == '(2 3 4 5 6))

