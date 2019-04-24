
using import testing
using import FunctionChain

global g_x = 0
inline get_g_x () g_x

@@ report
fnchain f

# decorator syntax
@@ 'on f
fn ()
    (get_g_x) += 1

'append f
    fn ()
        (get_g_x) *= 2

'prepend f
    fn ()
        (get_g_x) *= 10

'prepend f
    fn ()
        (get_g_x) += 1

assert ((get_g_x) == 0)

f;
assert ((get_g_x) == 22)

'clear f

f;
assert ((get_g_x) == 22)

