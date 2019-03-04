
sugar isvar? ((key as Symbol))
    try
        getattr syntax-scope key
        true
    except (err)
        false

run-stage;

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
    'set-symbol d 'x 6

    let e = (Scope)
    'set-symbol e 'x 5
    let f = (Scope e)
    'set-symbol f 'x 4

    let a = (Scope)
    'set-symbol a 'x 3
    let b = (Scope a)
    'set-symbol b 'x 2
    let c = (Scope b)
    'set-symbol c 'x 1

    let a = (.. c f d)

    assert ((getattr a 'x) as i32 == 1)
    let a = ('parent a)
    assert ((getattr a 'x) as i32 == 2)
    let a = ('parent a)
    assert ((getattr a 'x) as i32 == 3)
    let a = ('parent a)
    assert ((getattr a 'x) as i32 == 4)
    let a = ('parent a)
    assert ((getattr a 'x) as i32 == 5)
    let a = ('parent a)
    assert ((getattr a 'x) as i32 == 6)
    let a = ('parent a)
    assert (a == null)

