

using import testing

fn test (...)
    print "------------------"
    print ...
    print "------------------"
# programmatically changing a function to inline
# this is only executed at runtime though
sc_template_set_inline (sc_closure_get_template test)

fn gen-function ()
    fn () true

# error: constant of type Closure expected, got Call of type Closure
assert-compiler-error
    (((gen-function)) == true)

inline gen-function2 ()
    fn () true

# ok: inline returns closure
assert (((gen-function2)) == true)

inline gen-function4 (x)
    inline () x

fn test-function4 (x)
    # ok: inline captures variable outside scope
    assert (((gen-function4 x)) == true)

test-function4 true

inline gen-function5 (x)
    fn () x

# ok: function in inline captures constant outside scope
assert (((gen-function5 true)) == true)

fn test-function5 (x)
    # error: non-constant value of type bool is inaccessible from function
    assert-compiler-error
        ((gen-function5 x)) == true
test-function5 true

