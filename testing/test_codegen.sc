
using import testing

fn elidable (x)
    extractvalue (insertvalue x 1 0) 0

compile (static-typify elidable (tuple i32 i32)) 'dump-function 'dump-disassembly
compile (static-typify elidable (tuple i32 i32)) 'dump-function 'dump-disassembly

;