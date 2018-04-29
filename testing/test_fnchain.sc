
using import FunctionChain

fnchain f

let g_x = (static i32)

f;
assert (g_x == 0)

'append f
    fn ()
        g_x = g_x + 1

f;
assert (g_x == 1)

'append f
    fn ()
        g_x = g_x * 2

f;
assert (g_x == 4)

'prepend f
    fn ()
        g_x = g_x * 10

f;
assert (g_x == 82)

'clear f

f;
assert (g_x == 82)
