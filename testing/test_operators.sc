
using import testing

test ((- 5) == -5)

test ((/ 4) == 0.25)

test ((- 5.0) == -5.0)
test ((/ 4.0) == 0.25)

# ensure bits are masked
test ((1 << 31) == -2147483648)
test ((1 << 32) == 1)
test ((1:i64 << 64:i64) == 1:i64)

test-compiler-error
    do
        let x = (+ 1)

