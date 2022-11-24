
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

test ((bitcount 5:i16) == 2:i16)
test ((findmsb 6) == 2)
test ((findlsb 6) == 1)
test ((bitreverse 0b10010110:i8) == 0b01101001:i8)
test (all? ((findmsb (vectorof i32 1 3 5 9)) == (vectorof i32 0 1 2 3)))
test (all? ((findlsb (vectorof i8 1 2 3 4)) == (vectorof i8 0 1 0 2)))

# ensure pow allows argument adjustment
print
    256 ** (/ 3)

# fewer arguments are passed through
test ((+ 1) == 1)
test (va-empty? (+))

test ((floor 3) == 3)
test ((floor 3.5) == 3.0)
test (all? ((floor (vectorof i32 1 2)) == (vectorof i32 1 2)))
test (all? ((floor (vectorof f32 1.5 2.5)) == (vectorof f32 1 2)))


do
    let x y = 2 3

    test (not (x == y))
    test
        not x == y
    # don't wrap single multiline expressions
    test
        not
            if true false
            else true
    # but wrap multiple multiline expressions
    test
        not
            &
            if true false
            else true
            if true false
            else true
    test
        not (x == y) | (not constant? y)
    test
        not not not not true

# := auto-wraps the right hand side, accepts multiple left hand arguments
x := + 1 2
test (x == 3)
x := 4
test (x == 4)
x y z := 1, 2, 3
test (and (x == 1) (y == 2) (z == 3))
x := 1.0
# as:= auto-wraps right hand side as well
x as:= integer 32
test (x == 1)



;