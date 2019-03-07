print "test module loaded!"

let env = (Scope)
'set-symbol env 'compute
    fn (x)
        injected-var + x

env
