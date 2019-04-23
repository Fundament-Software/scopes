
using import testing

# start out with general template
fn testf (a b)
    a * b
let testf = (static-typify testf i32 i32)

# define overloaded function and expand existing testf
fn... testf
case (a : i32,)
    testf a a
# try the template last
case using testf

do
    # expand overloaded function (in this scope only)
    fn... testf
    case (a : i32, b : i32, c : i32)
        testf a (testf b c)
    # try the previous testf last
    case using testf

    # matches testf (a : i32, b : i32, c : i32)
    assert ((testf 4 3 2) == 24)
    # matches testf (a : i32,)
    assert ((testf 3) == 9)
    # matches testf (a b)
    assert ((testf 3 2) == 6)


# error: could not match argument types (i32 i32 i32) to overloaded function
  with types
      λ(i32 Unknown)
      λ(i32)
assert-compiler-error (testf 4 5 6)

# prints type
print testf
# prints function templates of the overloaded function
print testf.templates
# prints signatures of the overloaded function
print testf.parameter-types

fn... test2

'append test2
    fn (a)
        a + a
    # signature pattern
    Arguments i32

'append test2
    # template
    fn (a)
        .. a a
    # signature pattern
    Arguments string

assert ((test2 5) == 10)
assert ((test2 "hi") == "hihi")

fn... test3
case (a : integer = 3, b = 1, c = -1, d...)
    _ a b c d...

assert ((list (test3)) == (list 3 1 -1))
assert ((list (test3 5 6 7 8 9 0)) == (list 5 6 7 8 9 0))

fn... test4
case (a : integer, b : integer) (_ "int" (a + b))
case (a : real, b : real) (_ "real" (a + b))

assert ((test4 1 2) == "int")
assert ((test4 1.0 2.0) == "real")
assert ((test4 1 2.0) == "real")
assert ((test4 1.0 2) == "real")

fn... test5
case (a : &i32,) true
case (a : i32,) false
case (a : &real,) 1
case (a : real,) 2

assert ((test5 1) == false)
local y = 2
assert ((test5 y) == true)
assert ((test5 1.0) == 2)
local z = 2.0
assert ((test5 z) == 1)
