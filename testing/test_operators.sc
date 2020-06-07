
using import testing

test ((- 5) == -5)

test ((/ 4) == 0.25)

test ((- 5.0) == -5.0)
test ((/ 4.0) == 0.25)

# ensure bits are masked
test ((1 << 31) == -2147483648)
test ((1 << 32) == 1)
test ((1:i64 << 64:i64) == 1:i64)

print
    4 ** 3

test ((ctpop 5:i16) == 2:i16)
test ((ctlz 6) == 29)
test ((cttz 6) == 1)
test ((bitreverse 0b10010110:i8) == 0b01101001:i8)

test (all? ((ctlz (vectorof i32 1 2 4 8)) == (vectorof i32 31 30 29 28)))
test (all? ((cttz (vectorof i8 1 2 3 4)) == (vectorof i8 0 1 0 2)))

# ensure pow allows argument adjustment
print
    256 ** (/ 3)

test-compiler-error
    do
        let x = (+ 1)

