
fn main ()
    for i in (range 16)
    return;


compile-glsl 'fragment
    typify main
    'dump-module
    'dump-disassembly
    'no-debug-info
    'O2

true
