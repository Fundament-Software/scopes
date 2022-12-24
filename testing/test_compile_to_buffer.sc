using import Array
using import String

# compile-to-buffer (target module-name file-kind table flags...)

fn sum_two (x y)
    x + y

# object version

let result = 
    compile-to-buffer 
        "wasm32-unknown-unknown"
        "sum_two"
        compiler-file-kind-object
        do
            let 
                sum_two = 
                    static-typify sum_two i32 i32
                sum_two_float =
                    static-typify sum_two f32 f32
            locals;
        #

let length =
    (countof result)

local values = ((Array i8))

loop (a = 0)
    if (a < length)
        'insert values (@ result a)
        repeat (a + 1)
    else
        break a

print "binary"
print values

# assembly version
