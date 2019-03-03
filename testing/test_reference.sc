
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
local x = 5
assert (x == 5)
x = 10              # references support assignment operator
assert (x == 10)

local y = 2
local z = 12
assert ((x + y) == z) # references pass-through overloadable operators

assert ((typeof (deref y)) == i32)

# bind same reference to different name via let
let w = y
# copy by value to a new, independent reference
local z = y
y = 3
assert (y == 3)
assert (z == 2)
assert (w == y)

# loop with a mutable counter
local i = 0
loop ()
    if (i < 10)
        i += 1
        repeat;
    break;
assert (i == 10)

# declare unsized mutable array on stack; the size can be a variable
local y = 5
let x = (alloca-array i32 y)
x @ 0 = 1
x @ 1 = x @ 0 + 1
x @ 2 = x @ 1 + 1
x @ 3 = x @ 2 + 1
x @ 4 = x @ 3 + 1
assert ((x @ 4) == 5)

typedef refable < integer : i32
    method inline '__typecall (cls)
        nullof cls

    method... '__init
    case (self)
        (storagecast self) = 0
    case (self, initval : i32)
        (storagecast self) = initval

    method '__init-copy (self other)
        (storagecast self) = other

    method 'value (self)
        storagecast self

    method 'inced (self)
        bitcast
            (storagecast self) + 1
            typeof self

    method 'inc (self)
        self = ('inced self)
        self

run-stage;

let q = (refable)
assert (('value q) == 0)
assert (('value ('inced q)) == 1)
assert (('value q) == 0)
local q : refable -1
assert (('value q) == -1)
assert (('value ('inc q)) == 0)
assert (('value q) == 0)
