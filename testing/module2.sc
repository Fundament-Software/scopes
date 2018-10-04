print
    .. "test module 2 loaded from " module-path

syntax-eval
    let ok val = ('@ package 'test_module2)
    assert (not ok) "module loaded twice"
'set-symbol package 'test_module2 true

let env = (Scope)
'set-symbol env 'compute
    fn (x y)
        x + y

env
