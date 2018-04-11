
syntax-extend
    let t k = syntax-scope unnamed
    let loop (last-key) = k
    let key value =
        Scope-next t last-key
    if (key != unnamed)
        print key (repr value)
        loop key
    syntax-scope

let loop (scope) = (globals)
if (scope != null)
    for k v in scope
        #print k "=" v
        assert ((typeof k) == Symbol)
    loop (Scope-parent scope)
