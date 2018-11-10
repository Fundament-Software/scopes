
let x = i32
let x = (sc_view_type x 0)
let x = (sc_move_type x)
let x = (sc_view_type x 1)
let x = (sc_move_type x)
let x = (sc_keyed_type 'test x)
let x = (sc_keyed_type 'tost x)
let x = (sc_mutated_type x)

print x
