
using import FunctionChain

fnchain f

global g_x : i32
g_x = 0
fn get_g_x () g_x

run-stage;

f;
assert ((get_g_x) == 0)

'append f
    fn ()
        (get_g_x) += 1

run-stage;

f;
assert ((get_g_x) == 1)

'append f
    fn ()
        (get_g_x) *= 2

run-stage;

f;
assert ((get_g_x) == 4)

'prepend f
    fn ()
        (get_g_x) *= 10

run-stage;

f;
assert ((get_g_x) == 82)

'clear f

run-stage;

f;
assert ((get_g_x) == 82)

let chain1 = (FunctionChain "test")
let chain2 = (FunctionChain "test")
assert (chain1 != chain2)
