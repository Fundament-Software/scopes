print
    """"Scopes Language Reference
        =========================
let loop (scope) = (globals)
if (scope != null)
    let a b = (Scope-parent scope)
    for k v in scope
        if (('typeof v) == Closure)
            let s =
                docstring (v : Closure)
            if (not (empty? s))
                print s
        #print k v
    loop (Scope-parent scope)