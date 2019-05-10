
using import testing

spice cause-compiler-error (msg)
    let msg = (msg as string)
    if false
        `()
    else
        error "this is a compiler error!"

run-stage;

test-error
    test false

test-compiler-error
    cause-compiler-error "this is a compiler error!"
test-error
    error "this is a runtime error!"
test-compiler-error (1 == "test")
test-error
    test-compiler-error (1 == 1)

test-error
    test-compiler-error
        error "this is a runtime error!"
test-compiler-error
    test-error
        cause-compiler-error "this is a compiler error!"

print "ok."


