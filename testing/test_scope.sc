
using import testing

sugar isvar? ((key as Symbol))
    try
        '@ sugar-scope key
        true
    except (err)
        false

run-stage;

do
    let x = 5
    test (isvar? x)
    test (not (isvar? y))

test (not (isvar? x))

do
    define X 5
    test (isvar? X)

test (not (isvar? X))

do
    let d =
        'bind (Scope)
            x = 6

    let e =
        'bind (Scope) 'x 5
    let f =
        'bind (Scope e) 'x 4

    let a =
        'bind (Scope) 'x 3
    let b =
        'bind (Scope a) 'x 2
    let c =
        'bind (Scope b) 'x 1

    let a = (.. c f d)

    test (('@ a 'x) as i32 == 1)
    let a = ('parent a)
    test (('@ a 'x) as i32 == 2)
    let a = ('parent a)
    test (('@ a 'x) as i32 == 3)
    let a = ('parent a)
    test (('@ a 'x) as i32 == 4)
    let a = ('parent a)
    test (('@ a 'x) as i32 == 5)
    let a = ('parent a)
    test (('@ a 'x) as i32 == 6)
    let a = ('parent a)
    test (a == null)
