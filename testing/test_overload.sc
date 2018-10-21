
using import testing

# define overloaded function
fn... testf
case (a : i32, b : i32)
    a * b
case (a : i32,)
    testf a a

# prints type
print testf
# prints function templates of the overloaded function
print testf.templates
# prints signatures of the overloaded function
print testf.parameter-types

# matches case 2
assert ((testf 3) == 9)
# matches case 1
assert ((testf 3 2) == 6)
# error: could not match argument types (i32 i32 i32) to overloaded function
  with types
      λ(i32 i32)
      λ(i32)
assert-compiler-error (testf 4 5 6)
