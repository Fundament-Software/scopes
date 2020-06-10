
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
    test ((testf 4 3 2) == 24)
    # matches testf (a : i32,)
    test ((testf 3) == 9)
    # matches testf (a b)
    test ((testf 3 2) == 6)


# error: could not match argument types (i32 i32 i32) to overloaded function
  with types
      λ(i32 Unknown)
      λ(i32)
test-compiler-error (testf 4 5 6)

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

test ((test2 5) == 10)
test ((test2 "hi") == "hihi")

fn... test3
case (a : integer = 3, b = 1, c = -1, d...)
    _ a b c d...

test ((list (test3)) == (list 3 1 -1))
test ((list (test3 5 6 7 8 9 0)) == (list 5 6 7 8 9 0))

fn... test4
case (a : integer, b : integer) (_ "int" (a + b))
case (a : real, b : real) (_ "real" (a + b))

test ((test4 1 2) == "int")
test ((test4 1.0 2.0) == "real")
test ((test4 1 2.0) == "real")
test ((test4 1.0 2) == "real")

@@ memo
inline ArrayPattern (element-type)
    typedef (.. "[" (tostring element-type) " x ...]")
        inline __typematch (cls T)
            static-if (T < array)
                (elementof T) == element-type
            else false
        inline __rimply (cls T)
            inline (self) self

fn... test5
case (a : (ArrayPattern i32),) 3
case (a : (ArrayPattern f32),) 4
case (a : &i32,) true
case (a : i32,) false
case (a : &real,) 1
case (a : real,) 2

test ((test5 1) == false)
local y = 2
test ((test5 y) == true)
test ((test5 1.0) == 2)
local z = 2.0
test ((test5 z) == 1)
test ((test5 (arrayof i32 1 2 3)) == 3)
test ((test5 (arrayof f32 1 2 3)) == 4)
test-compiler-error (test5 (arrayof i64 1 2 3))

# inlined case form
fn... test6 (a : i32, b, c) 1
case (x,) 2

test ((test6 1 2 3) == 1)
test ((test6 1) == 2)

do
    typedef K : f32
    typedef M : i32
    typedef L : i32

    # typematcher
    fn... test7
    case (a : (typematch T < integer),) 1
    case (a : (typematch T < real),) 2
    case (a : (typematch (storageof T) < real),) 3
    #case (a : (typematch T in (tupleof M L)),) 3

    test ((test7 1) == 1)
    test ((test7 1.0) == 2)
    test ((test7 (nullof K)) == 3)

