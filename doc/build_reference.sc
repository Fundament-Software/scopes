#!/usr/bin/env scopes
using import Array

fn starts-with-letter (s)
    let c = (s @ 0)
    or
        (c >= (char "a")) and (c <= (char "z"))
        (c >= (char "A")) and (c <= (char "Z"))

let objs = (local (Array Any))

loop (scope) = (globals)
if (scope != null)
    let a b = (Scope-parent scope)
    for k v in scope
        if (('typeof v) == Closure)
            'append objs v
    repeat (Scope-parent scope)

'sort objs
    fn (x)
        dump x
        if (('typeof x) == Closure)
            let x = (x as Closure)
            let x = (Closure-label x)
            let s =
                Label-prettyname x
            if (starts-with-letter s) s
            else
                if ((countof s) < 2:usize)
                    .. " 1" s
                else
                    .. " 2" s
        else
            unconst ""

print
    """"Scopes Language Reference
        =========================
for entry in objs
    if (('typeof entry) == Closure)
        let func = (entry as Closure)
        let label =
            Closure-label func
        let name =
            Label-prettyname label
        #for i param in (enumerate ('parameters label))
            print i param

print "done"
