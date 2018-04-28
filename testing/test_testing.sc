
using import testing

assert-compiler-error
    compiler-error! "this is a compiler error!"
assert-error
    error! "this is a runtime error!"
assert-compiler-error ((unconst 1) == (unconst "test"))
assert-error
    assert-compiler-error (1 == 1)

assert-error
    assert-compiler-error
        error! "this is a runtime error!"
assert-compiler-error
    assert-error
        compiler-error! "this is a compiler error!"

print "ok."


