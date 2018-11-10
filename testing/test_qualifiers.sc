
let x = i32
let x = (sc_view_type x 0)
let x = (sc_move_type x)
let x = (sc_view_type x 1)
let x = (sc_move_type x)
let x = (sc_key_type 'TEST x)
let x = (sc_key_type 'test x)
let x = (sc_mutate_type x)
let x = (sc_refer_type x 0:u64 unnamed)

print x
