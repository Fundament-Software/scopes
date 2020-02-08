
fn function-with-exception ()
    for k in (range 16)
        if (k == 5)
            raise;
    10

fn main ()
    try
        let count = (function-with-exception)
        for i j in (zip (range count) (range 16))
    except ()
        # error raised
    return;

let s =
    compile-spirv 'fragment
        typify main
        'O2
        'dump-disassembly
assert (not (empty? s))

compile-glsl 0 'fragment
    typify main
    'dump-module
    'dump-disassembly
    'no-debug-info
    #'O2

true
