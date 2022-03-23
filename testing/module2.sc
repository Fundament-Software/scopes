print
    .. "test module 2 loaded from " module-path

# as package has been removed, we can't use it to test double loads
#assert
    do
        try
            '@ package 'test_module2
            false
        except (err)
            true
    "module loaded twice"
#'set-symbol package 'test_module2 true

let env =
    'bind (Scope) 'compute
        fn (x y)
            x + y

env
