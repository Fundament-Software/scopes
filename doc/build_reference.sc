import MutableArray

let loop (scope) = (globals)
let objs = (local (MutableArray Any))
if (scope != null)
    let a b = (Scope-parent scope)
    for k v in scope
        if (('typeof v) == Closure)
            let s =
                docstring (v as Closure)
            if (not (empty? s))
                'append objs v
            #
        #print k v
    loop (Scope-parent scope)


print
    """"Scopes Language Reference
        =========================
