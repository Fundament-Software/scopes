
let t = (__this-scope)
let a b c = 1 2 3
loop (index = -1)
    let key value index =
        sc_scope_next t index
    if (index < 0)
        break;
    print key value
    index

for scope in ('lineage (globals))
    for k v in scope
        print k "=" v
        assert (('typeof k) == Symbol)

print;
for k v scope in ('all (globals))
    print k "=" v
    assert (('typeof k) == Symbol)
