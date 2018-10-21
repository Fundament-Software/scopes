
using import testing

# start out with general template
fn testf (a b)
    a * b
let testf = (typify testf i32 i32)

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

