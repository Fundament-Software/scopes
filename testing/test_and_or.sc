
using import testing

do
    let x y z = 0 1 2
    test
        and
            x == 0
            y == 1
            z == 2
        "multideclaration failed"

test
    ==
        and
            do
                print "and#1"
                true
            do
                print "and#2"
                true
            do
                print "and#3"
                true
            do
                print "and#4"
                false
            do
                error "should never see this"
                false
        false
    "'and' for more than two arguments failed"

test
    ==
        or
            do
                print "or#1"
                false
            do
                print "or#2"
                false
            do
                print "or#3"
                false
            do
                print "or#4"
                true
            do
                error "should never see this"
                true
        true
    "'or' for more than two arguments failed"

# FIX:
    error: new branch result type bool conflicts with previous type (mutable& (storage = 'Function) bool)
fn unconst (x) x
local val = true
if ((unconst true) and val)
    print "yes"
if (val and (unconst true))
    print "yes"

;