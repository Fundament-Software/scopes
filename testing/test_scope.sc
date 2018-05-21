
define-scope-macro isvar?
    let key = (decons args)
    let key = (key as Syntax as Symbol)
    let _ ok = (@ syntax-scope key)
    return ok syntax-scope

do
    let x = 5
    assert (isvar? x)
    assert (not (isvar? y))

assert (not (isvar? x))

do
    define X 5
    assert (isvar? X)

assert (not (isvar? X))

do
    let d = (Scope)
    set-scope-symbol! d 'x 6

    let e = (Scope)
    set-scope-symbol! e 'x 5
    let f = (Scope e)
    set-scope-symbol! f 'x 4

    let a = (Scope)
    set-scope-symbol! a 'x 3
    let b = (Scope a)
    set-scope-symbol! b 'x 2
    let c = (Scope b)
    set-scope-symbol! c 'x 1

    let a = (.. c f d)

    assert ((a @ 'x) == 1)
    let a = ('parent a)
    assert ((a @ 'x) == 2)
    let a = ('parent a)
    assert ((a @ 'x) == 3)
    let a = ('parent a)
    assert ((a @ 'x) == 4)
    let a = ('parent a)
    assert ((a @ 'x) == 5)
    let a = ('parent a)
    assert ((a @ 'x) == 6)
    let a = ('parent a)
    assert (a == null)

