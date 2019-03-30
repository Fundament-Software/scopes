
using import testing

spice cause-compiler-error (msg)
    let msg = (msg as string)
    if false
        box-empty;
    else
        error "this is a compiler error!"

run-stage;

assert-compiler-error
    cause-compiler-error "this is a compiler error!"
assert-error
    error "this is a runtime error!"
assert-compiler-error (1 == "test")
assert-error
    assert-compiler-error (1 == 1)

assert-error
    assert-compiler-error
        error "this is a runtime error!"
assert-compiler-error
    assert-error
        cause-compiler-error "this is a compiler error!"

print "ok."


