
using import testing
using import FunctionChain

global g_x : i32
g_x = 0
fn get_g_x () g_x

@@ print
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

# constant of type FunctionChain expected, got expression of type FunctionChain
assert-compiler-error (f)
assert ((get_g_x) == 0)

run-stage;

f;
assert ((get_g_x) == 22)

'clear f

# clear not executed yet, so the previous chain still applies
f;
assert ((get_g_x) == 462)

run-stage;

f;
assert ((get_g_x) == 462)

