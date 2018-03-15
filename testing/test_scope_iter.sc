
syntax-extend
    let t k = syntax-scope none
    let loop (last-key) = k
    let key value =
        Scope-next t (Any last-key)
    if (not (('typeof key) == Nothing))
        print (key as Symbol) (repr value)
        loop key
    syntax-scope

let loop (scope) = (globals)
if (scope != null)
    for k v in scope
        # print k "=" v
        assert ((typeof (k as Symbol)) == Symbol)
    loop (Scope-parent scope)
