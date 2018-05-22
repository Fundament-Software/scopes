

using import testing

fn test (...)
    print "------------------"
    print ...
    print "------------------"
Label-set-inline! (Closure-label test)

# error: function returns closure
fn gen-function ()
    fn () true

assert-compiler-error
    gen-function;

# fine: inline returns closure
inline gen-function2 ()
    fn () true

assert (((gen-function2)) == true)

inline gen-function3 (x)
    fn () x

# ok: function captures constant outside scope
assert (((gen-function3 true)) == true)

# error: function captures variable outside scope
assert-compiler-error
    ((gen-function3 (unconst true))) == true

inline gen-function4 (x)
    inline () x

# ok: inline captures variable outside scope
assert (((gen-function4 (unconst true))) == true)
