


fn store-value (x y)
    let T = (typename-type "CClosure")
    let EV =
        tupleof
            x = x
            y = y
    let TT = (typeof EV)
    set-typename-storage! T TT
    set-type-symbol! T 'call
        fn (self)
            let x y = (unpack (bitcast self TT))
            print x y
    bitcast EV T

let f =
    local 'copy
        store-value (unconst 25) (unconst "hello")


dump f
print "done"
print (f)
