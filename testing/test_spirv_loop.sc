
fn main ()
    loop (i = 0)
        if (i == 16)
            break;
        i + 1
    return;

compile-glsl 'fragment
    typify main
    'dump-module
    'dump-disassembly
    'no-debug-info

true
