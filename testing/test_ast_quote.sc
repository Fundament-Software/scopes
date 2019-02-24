

let T = (typename "T")
'set-plain-storage T i32

! ast-quote
fn test ()
    print T

# declare a function and assign it as attribute to T
method '__typecall T (cls x)
    bitcast x T

# declare an inline and assign it as attribute to T
method inline '__typecall T (cls x)
    bitcast x T

do
    # this form is best used inside struct-like bodies; in this example,
        we jerry-rig the required definition of this-type
    let this-type = T

    # declare a function and assign it as attribute to this-type
    method '__typecall (cls x)
        bitcast x T

    # declare an inline and assign it as attribute to this-type
    # also test method decorators
    ! ast-quote
    method inline '__typecall (cls x)
        bitcast x T

run-stage;

test;

print (T 5)
