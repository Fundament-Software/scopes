
fn main ()
    for i j in (zip (range 16) (range 16))
    return;

let s =
    compile-spirv 'fragment
        typify main
        'O2
assert (not (empty? s))

compile-glsl 'fragment
    typify main
    'dump-module
    'dump-disassembly
    'no-debug-info
    'O2

true
