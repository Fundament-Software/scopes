using import Array
using import String
using import testing

let source-path argc argv = (script-launch-args)

fn sum_two (x y)
    x + y

fn compile(kind)
    compile-object-to-buffer
        "wasm32-unknown-unknown"
        kind
        "sum_two"
        do
            let
                sum_two =
                    static-typify sum_two i32 i32
                sum_two_float =
                    static-typify sum_two f32 f32
            locals;
        #

fn debug_info(buffer)
    let length =
        (countof buffer)

    assert (length != 0)

    local values = ((Array i8))

    loop (a = 0)
        if (a < length)
            'insert values (@ buffer a)
            repeat (a + 1)
        else
            break a
    values

fn debug_print(buffer)
    let length = 
        (countof buffer)

    loop (a = 0)

        if (a < length)
            print ((@ buffer a) as string)
            repeat (a + 1)
        else
            break a

let result-object = 
    compile compiler-file-kind-object

let values-object =
    debug_info result-object

let result-asm =
    compile compiler-file-kind-asm

let values-asm =
    debug_info result-asm

if ((argv @ 0) != null)
    let arg-val =
        (string (@ argv 0))

    if (arg-val == "debug" or arg-val == "DEBUG")
        print "compiler-file-kind-object buffer values"
        debug_print result-object

        print "compiler-file-kind-asm buffer values"
        debug_print result-asm

        print "compiler-file-kind-object"
        print values-object

        print "compiler-file-kind-asm"
        print values-asm