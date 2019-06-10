
using import testing

sugar isvar? ((key as Symbol))
    try
        getattr sugar-scope key
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
    let d = (Scope)
    'bind d 'x 6

    let e = (Scope)
    'bind e 'x 5
    let f = (Scope e)
    'bind f 'x 4

    let a = (Scope)
    'bind a 'x 3
    let b = (Scope a)
    'bind b 'x 2
    let c = (Scope b)
    'bind c 'x 1

    let a = (.. c f d)

    test ((getattr a 'x) as i32 == 1)
    let a = ('parent a)
    test ((getattr a 'x) as i32 == 2)
    let a = ('parent a)
    test ((getattr a 'x) as i32 == 3)
    let a = ('parent a)
    test ((getattr a 'x) as i32 == 4)
    let a = ('parent a)
    test ((getattr a 'x) as i32 == 5)
    let a = ('parent a)
    test ((getattr a 'x) as i32 == 6)
    let a = ('parent a)
    test (a == null)
