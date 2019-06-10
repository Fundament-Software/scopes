print
    .. "test module 2 loaded from " module-path

assert
    do
        try
            '@ package 'test_module2
            false
        except (err)
            true
    "module loaded twice"

'bind package 'test_module2 true

let env = (Scope)
'bind env 'compute
    fn (x y)
        x + y

env
