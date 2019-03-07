
sugar test (x y z args...)
    assert ((x as i32) == 1)
    assert ((y as i32) == 2)
    assert ((z as i32) == 3)
    assert ((countof args...) == 3)
    let u v w = (decons args... 3)
    assert ((u as i32) == 4)
    assert ((v as i32) == 5)
    assert ((w as i32) == 6)
    print sugar-scope expr-head
    list (do +) 3 3

sugar test2 (x y z args...)
    assert ((x as i32) == 1)
    assert ((y as i32) == 2)
    assert ((z as i32) == 3)
    assert ((countof args...) == 3)
    let u v w = (decons args... 3)
    assert ((u as i32) == 4)
    assert ((v as i32) == 5)
    assert ((w as i32) == 6)
    print sugar-scope expr-head
    return
        list (do +) 6 6
        next-expr

run-stage;

assert
    (test 1 2 3 4 5 6) == 6

assert
    (test2 1 2 3 4 5 6) == 12

fn test-match (expr)
    sugar-match expr
    case (('kwok (a : i32) b q...) c...)
        print "case1" a b q...
        print c...
        return;
    case ('kwok 'kwok x)
        print "case2" x
        return;
    case (x (y z) w)
        print "case3" x y z w
        return;
    case (x y...)
        print "case4" x y...
        return;
    default
        compiler-error! "wrong!"

test-match '((kwok 2 4 5 6) 3 4)
test-match '(kwok kwok 20)
test-match '(1 (2 3) 4)
test-match '(1 2 3)

return;