
using import testing
using import format
using import String

# string formatting

print (.. (format "{} {}!" "Hello" "World"))
print
    String (format "{0}buzz{1}fizz{0}" "fizz" "buzz")
print
    vvv String
    format "* {name} ({age})"
        name = "Joana"
        age = 42

test
    ==
        report
            ..
                format
                    "test test2 {2} {1} more {u}x{v}x{wx} {Q} {P} {}"
                    \ 1 2 3
                    Q = "test"
                    P =
                        local : String "hi"
                    u = 10
                    v = 20
                    wx = 30
        "test test2 3 2 more 10x20x30 test hi 1"

test-compiler-error
    format "{test"
test-compiler-error
    format "test {2p}"
test-compiler-error
    format "test {x}"


;