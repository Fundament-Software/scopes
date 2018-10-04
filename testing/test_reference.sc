
#
    local T args ...
    local 'copy value

    new T args ...
    new 'copy value

    static T args ...
    static 'copy value

    and then there's the question of array construction?

    local (T args ...)

    T args ... -> (inline (dest) ...)


    copy local value
    move local value
    new local T args ...

    copy [local] value
    move [local] value
    new [local] T args ...

    local copy value
    local move value
    local Array args ...

    new copy value
    new move value
    new Array args ...

    static copy value
    static move value
    static Array args ...

    local x = value
    local x <- value
    local x : Array args ...

    new x = value
    new x <- value
    new x : Array args ...

    static x = value
    static x <- value
    static x : Array args ...





# `local` creates a stack variable of reference type
let x = (local 'copy 5)
assert (x == 5)
x = 10              # references support assignment operator
assert (x == 10)

let y = (local 'copy 2)
let z = (local 'copy 12)
assert ((x + y) == z) # references pass-through overloadable operators

assert ((typeof (deref y)) == i32)

# bind same reference to different name via let
let w = y
# copy by value to a new, independent reference
let z = (local 'copy y)
y = 3
assert (y == 3)
assert (z == 2)
assert (w == y)

# loop with a mutable counter
let i = (local 'copy 0)
let loop ()
if (i < 10)
    i = i + 1
    loop;
assert (i == 10)

# declare unsized mutable array on stack; the size can be a variable
let y = (local 'copy 5)
let x = (alloca-array i32 (y as immutable))
x @ 0 = 1
x @ 1 = x @ 0 + 1
x @ 2 = x @ 1 + 1
x @ 3 = x @ 2 + 1
x @ 4 = x @ 3 + 1
assert ((x @ 4) == 5)

let T = (typename "refable" (storage = i32) (super = integer))

typefn T '__typecall (cls)
    nullof cls

typeinline& T '__new (self)
    supercall T '__new self

typefn T 'value (self)
    bitcast self (storageof T)

typeinline& T 'value (self)
    bitcast (load self) (storageof T)

typefn T 'inc (self)
    bitcast
        (bitcast self (storageof T)) + 1
        T

typeinline& T 'inc (self)
    self =
        'inc (deref self)
    self

let q = (T)
assert (('value q) == 0)
assert (('value ('inc q)) == 1)
assert (('value q) == 0)
let q = (local T)
assert (('value q) == 0)
assert (('value ('inc q)) == 1)
assert (('value q) == 1)
