
fn main ()
    let loop (i) = (unconst 0)
    if (icmp<s i 16)
        loop (add i 1)
    return;



dump-label
    typify main

compile-glsl 'fragment
    typify main
    'dump-module
    'dump-disassembly
    'no-debug-info