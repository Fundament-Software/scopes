
using import testing

global refcount = 0

typedef Handle :: i32

    @@ spice-quote
    inline new-handle (idx)
        refcount += 1
        bitcast idx this-type

    @@ spice-quote
    inline __typecall (cls idx)
        #print "new handle" idx
        new-handle idx

    inline __repr (self)
        repr (storagecast self)

    inline __drop (self)
        refcount -= 1
        #print "drop handle" (storagecast self)
        _;

let
    UHandle0 = ('unique-type Handle 0)
    UHandle1 = ('unique-type Handle 1)
    UHandle2 = ('unique-type Handle 2)
    UHandle3 = ('unique-type Handle 3)
    UHandle4 = ('unique-type Handle 4)
    UHandle5 = ('unique-type Handle 5)

    UHandleR1 = ('unique-type Handle -1)
    UHandleR2 = ('unique-type Handle -2)

    VHandle = ('view-type Handle)

    VHandle0 = ('view-type Handle 0)
    VHandle1 = ('view-type Handle 1)
    VHandle2 = ('view-type Handle 2)
    VHandle3 = ('view-type Handle 3)
    VHandle4 = ('view-type Handle 4)

    VHandle12 = ('view-type VHandle1 2)
    VHandle34 = ('view-type VHandle3 4)

    VHandle1234 = ('view-type ('view-type VHandle12 3) 4)

    Vi321 = ('view-type i32 1)

run-stage;

# function type constructor canonicalizes types
fn verify-signature (Ta Tb)
    assert (Ta == Tb)
        .. (repr Ta) "==" (repr Tb)
    print Ta "==" Tb

fn verify-type (f ret args...)
    let Ta = ('typeof f)
    let Tb = ('pointer (function ret args...))
    verify-signature Ta Tb

#do
    verify-signature (function void Handle Handle) (function void UHandle0 UHandle1)
    verify-signature (function void UHandle0 UHandle0) (function void UHandle0 UHandle1)
    verify-signature (function void UHandle2 UHandle1) (function void UHandle0 UHandle1)
    verify-signature (function void UHandle0 UHandle0 VHandle0) (function void UHandle0 UHandle1 VHandle01)
    verify-signature (function void VHandle VHandle01) (function void VHandle0 VHandle1)
    verify-signature (function void VHandle1 VHandle1) (function void VHandle0 VHandle0)

    verify-signature
        ('raising (function (Arguments Handle Handle) Handle Handle) Handle)
        ('raising (function (Arguments UHandle2 UHandle3) UHandle0 UHandle1) UHandle4)

    verify-signature
        function (Arguments UHandle0 VHandle VHandle0) UHandle0 UHandle0 VHandle0
        function (Arguments UHandle2 VHandle3 VHandle012) UHandle0 UHandle1 VHandle01

#run-stage;

inline test-refcount (f)
    fn testfunc ()
        f;
        return;
    assert (refcount == 0)
    testfunc;
    assert (refcount == 0)

# receives handle but doesn't do anything with it:
    void<-(%1:Handle)(*)
fn f (h)
    return;
verify-type (typify f Handle) void VHandle1
test-refcount (inline () (f (Handle 1)))

# receives handle and passes it through.
    %1:Handle<-(%1:Handle)(*)
fn f (h)
    h
verify-type (typify f Handle) VHandle1 VHandle1
test-refcount (inline () (f (Handle 1)))

# receives handle, moves it and returns nothing.
    void<-(1:Handle)(*)
fn f (h)
    move h
    return;
verify-type (typify f Handle) void UHandle1
test-refcount (inline () (f (Handle 1)))

# receives handle, moves and returns it.
    R1:Handle<-(1:Handle)(*)
fn f (h)
    move h
verify-type (typify f Handle) UHandleR1 UHandle1
test-refcount (inline () (f (Handle 1)))

# receives no arguments and returns new handle
    R1:Handle<-()(*)
fn f ()
    Handle 0
verify-type (typify f) UHandleR1
test-refcount (inline () (f))

# receives two handles and returns second handle and a new handle
    λ(%2:Handle R2:Handle)<-(%1:Handle %2:Handle)(*)
fn f (a b)
    viewing a b
    _ b (Handle 0)
verify-type (typify f Handle Handle) (Arguments VHandle2 UHandleR2) VHandle1 VHandle2
test-refcount (inline () (f (Handle 1) (Handle 2)))

# receives two handles, conditionally resolves one and returns nothing.
    void<-(%1:Handle %2:Handle bool)(*)
fn f (a b x)
    if x a
    else b
    return;
verify-type (typify f Handle Handle bool) void VHandle1 VHandle2 bool
test-refcount (inline ()
    (f (Handle 1) (Handle 2) true)
    (f (Handle 1) (Handle 2) false))

# views two handles and conditionally returns one.
    %1|2:Handle<-(%1:Handle %2:Handle bool)(*)
fn f (a b x)
    if x a
    else b
verify-type (typify f Handle Handle bool) VHandle12 VHandle1 VHandle2 bool
test-refcount (inline ()
    (f (Handle 1) (Handle 2) true)
    (f (Handle 1) (Handle 2) false))

# same setup, but alternative structure
    %1|2:Handle<-(%1:Handle %2:Handle bool)(*)
fn f (a b x)
    if x
        return a
    b
verify-type (typify f Handle Handle bool) VHandle12 VHandle1 VHandle2 bool
test-refcount (inline ()
    (f (Handle 1) (Handle 2) true)
    (f (Handle 1) (Handle 2) false))

# receives two handles and conditionally moves either one.
    R1:Handle<-(1:Handle 2:Handle bool)(*)
fn f (a b x)
    if x (move a)
    else (move b)
verify-type (typify f Handle Handle bool) UHandleR1 UHandle1 UHandle2 bool
test-refcount (inline ()
    (f (Handle 1) (Handle 2) true)
    (f (Handle 1) (Handle 2) false))

# receives two handles, moves both into the function and conditionally
    moves either one.
    R2:Handle<-(1:Handle 2:Handle bool)(*)
fn f (a b x)
    let a = (move a)
    let b = (move b)
    if x
        return b
    # both a and b still accessible
    dump a b
    a
verify-type (typify f Handle Handle bool) UHandleR1 UHandle1 UHandle2 bool
test-refcount (inline ()
    (f (Handle 1) (Handle 2) true)
    (f (Handle 1) (Handle 2) false))

# receives two handles, moves both into the function and conditionally
    moves either one, but one handle is moved into a subscope, which
    we are returning from.
    R2:Handle<-(1:Handle 2:Handle bool)(*)
fn f (a b x)
    let a = (move a)
    if x
        # will drop b
        return (move a)
    let b = (move b)
    # a still accessible
    dump a
    # will drop a
    b
verify-type (typify f Handle Handle bool) UHandleR1 UHandle1 UHandle2 bool
test-refcount (inline ()
    (f (Handle 1) (Handle 2) true)
    (f (Handle 1) (Handle 2) false))

# receives two handles and conditionally borrows one, but moves another.
    error: conflicting branch types %1:Handle and 1000:Handle
    we could force move on `a` here but that would be an ambiguous guess
    of programmer intent.
fn f (a b x)
    if x a
    else (move b)
assert-error (typify f Handle Handle bool)

# receives a handle and casts it to i32
    %1:i32<-(%1:Handle)(*)
fn f (h)
    bitcast h i32
verify-type (typify f Handle) Vi321 VHandle1
test-refcount (inline () (f (Handle 1)))

# receives a handle, casts it to i32 and back to Handle
    %1:Handle<-(%1:Handle)(*)
fn f (h)
    bitcast (bitcast h i32) Handle
verify-type (typify f Handle) VHandle1 VHandle1
test-refcount (inline () (f (Handle 1)))

# receives four handles and conditionally returns one, switch version
    <view:0|1|2|3>Handle<-(<view>Handle <view>Handle bool)(*)
fn f (a b c d x)
    switch x
    case 0 a
    case 1 b
    case 2 c
    default d
verify-type (typify f Handle Handle Handle Handle i32)
    \ VHandle1234 VHandle1 VHandle2 VHandle3 VHandle4 i32
test-refcount (inline ()
    (f (Handle 1) (Handle 2) (Handle 3) (Handle 4) 0)
    (f (Handle 1) (Handle 2) (Handle 3) (Handle 4) 1)
    (f (Handle 1) (Handle 2) (Handle 3) (Handle 4) 2)
    (f (Handle 1) (Handle 2) (Handle 3) (Handle 4) 3))

# receives two handles and passes them to a function that conditionally returns one
    %1|2:Handle<-(%1:Handle %2:Handle bool)(*)
fn f (a b c d x)
    fn ff (a b x)
        if x a
        else b
    ff c d x
verify-type (typify f Handle Handle Handle Handle bool)
    \ VHandle34 VHandle1 VHandle2 VHandle3 VHandle4 bool
test-refcount (inline ()
    (f (Handle 1) (Handle 2) (Handle 3) (Handle 4) false)
    (f (Handle 1) (Handle 2) (Handle 3) (Handle 4) true))

# receives two handles and passes one to a function that moves the argument,
    then attempts to access the moved argument
    error: cannot access value
fn f (a b)
    fn ff (x) (move x)
    ff b
    dump b
    a
assert-error (typify f Handle Handle)

# attempting to move a parent value into a switch pass
fn f ()
    let x = (Handle 0)
    switch 10
    case 0
    pass 1
        move x
        _;
    pass 2
    case 3
    default
        _;
assert-error (typify f)

# state propagation through different kinds of scopes
fn f (x)
    let c = (Handle 0)
    if false
    elseif false
    elseif false
    elseif false
    else
        switch 10
        case 0
            label custom
                merge custom
                    do
                        move x
                        _;
        pass 1
        pass 2
        case 3
        default
            _;
#'dump (typify f Handle)
verify-type (typify f Handle) void UHandle1
test-refcount (inline () (f (Handle 0)))

# TODO:
    let c = (unique)
    label done2
        label done
            if true
                # label done2: c is marked as to be moved, so move c
                merge done2
            let d = (unique)
            if true
                move c
                # label done2: mark c as to be moved
                # auto-drop d as part of constructor rollup
                merge done2
            elseif true
                move c
                # label done: mark c as to be moved
                # auto-drop d as part of constructor rollup
                merge done
            elseif true
                # label done: c is marked as to be moved, so auto-drop c
                # auto-drop d as part of constructor rollup
                merge done
            else
                # we're continuing in parent block
            # c not moved, so still available here

            # implicit merge to done
            # label done: c is marked as to be moved, so auto-drop c
            # auto-drop d as part of constructor rollup
            merge done
        # c definitely moved here
        # label done2: c is marked as to be moved, nothing to do
        merge done2

#let c = (unique)
#label done
    if cond1
        # merging to done
        # read tagged done: c to be moved, but is still alive, so drop c now
            -- which means we need a local copy of the parent state that
                isn't updated when the parent evolves.
        merge done
    let d = (move c)
    # after above statement, c is marked as moved
    if cond2
        # merging to done
        # auto-drop d
        # read tagged done: c to be moved, which already happened
        merge done

    # implicitly merging to done
    # auto-drop d
    # tag done: c to be moved, which already happened
# after above statement, c is marked as moved


# WHAT ABOUT VALUES THAT WE MOVE UP?
    probably not an issue since we only borrow upvars, and others require local
    moves?
#let c = (unique)
#let d = (unique)
#label done
    if cond
        move c
        # c got moved, tag as moved
        merge done d
    else
        # drop c - now we can't view c :/
        merge done c




# TODO: composition, decomposition

# TODO: loops

# TODO: try/except/raise

