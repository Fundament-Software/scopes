using import Array

let objs = (local (Array Any))

loop (scope) = (globals)
if (scope != null)
    let a b = (Scope-parent scope)
    for k v in scope
        if (('typeof v) == Closure)
            let func = (v as Closure)
            let s = (docstring func)
            if (not (empty? s))
                let label =
                    Closure-label func
                print
                    Label-prettyname label
                'append objs v
                print (countof objs)
            #
        #print k v
    repeat (Scope-parent scope)

'sort objs
    fn (x)
        if (('typeof x) == Closure)
            let x = (x as Closure)
            let x = (Closure-label x)

        return ""

print
    """"Scopes Language Reference
        =========================
for entry in objs
    print entry

print "done"