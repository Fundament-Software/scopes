
do
    fn main ()
        loop (i) = 0
        if (i < 16)
            repeat (i + 1)
        return;

    'dump main
    dump;
    'dump
        typify main

    compile-glsl 'fragment
        typify main
        'dump-module
        'dump-disassembly
        'no-debug-info