
let t = (__this-scope)
loop (last-key = unnamed)
    let key value =
        sc_scope_next t last-key
    if (key == unnamed)
        break;
    print key (repr value)
    key

loop (scope = (globals))
    if (scope == null)
        break;
    for k v in scope
        #print k "=" v
        assert ((typeof k) == Symbol)
    sc_scope_get_parent scope
