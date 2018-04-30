using import Array

let loop (scope) = (globals)
let objs = (local (Array Any))
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

'sort objs
    fn (x)
        if (('typeof x) == Closure)
            let x = (x as Closure)
            let x = (Closure-label x)
            dump x
        return ""

print
    """"Scopes Language Reference
        =========================
