
using import testing

assert ((- 5) == -5)

assert ((/ 4) == 0.25)

assert ((- 5.0) == -5.0)
assert ((/ 4.0) == 0.25)

# ensure bits are masked
assert ((1 << 31) == -2147483648)
assert ((1 << 32) == 1)
assert ((1:i64 << 64:i64) == 1:i64)

assert-compiler-error
    do
        let x = (+ 1)

