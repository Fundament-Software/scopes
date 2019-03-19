
using import testing

global refcount = 0

typedef Handle :: i32

    @@ spice-quote
    inline new-handle (idx)
        refcount += 1
        follow idx this-type

    @@ spice-quote
    inline __typecall (cls idx)
        #print "new handle" idx
        new-handle idx

    inline __repr (self)
        repr (storagecast self)

    inline __drop (self)
        #dump "drop"
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

    UHandleE1 = ('unique-type Handle -257)

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

fn verify-raising-type (f ret raises args...)
    let Ta = ('typeof f)
    let Tb = ('pointer ('raising (function ret args...) raises))
    verify-signature Ta Tb

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

# receives handle and passes a view of it through.
    %1:Handle<-(%1:Handle)(*)
fn f (h)
    view h
verify-type (typify f Handle) VHandle1 VHandle1
test-refcount (inline () (f (Handle 1)) (_))

# receives handle, moves it and returns nothing.
    void<-(1:Handle)(*)
fn f (h)
    move h
    return;
verify-type (typify f Handle) void UHandle1
test-refcount (inline () (f (Handle 1)))

# receives handle and returns it.
    R1:Handle<-(1:Handle)(*)
fn f (h)
    h
verify-type (typify f Handle) UHandleR1 UHandle1
test-refcount (inline () (f (Handle 1)))

# receives handle, moves it locally and then attempts to reaccess
    cannot access value of type 1:Handle because it has been moved
fn f (h)
    let hh = (do h)
    h
assert-error (typify f Handle)

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
test-refcount (inline () (f (Handle 1) (Handle 2)) (_))

# receives two handles, conditionally resolves one and returns nothing.
    void<-(1:Handle 2:Handle bool)(*)
fn f (a b x)
    if x a
    else b
    return;
verify-type (typify f Handle Handle bool) void UHandle1 UHandle2 bool
test-refcount (inline ()
    (f (Handle 1) (Handle 2) true)
    (f (Handle 1) (Handle 2) false))

# views two handles and conditionally returns one.
    %1|2:Handle<-(%1:Handle %2:Handle bool)(*)
fn f (a b x)
    if x (view a)
    else (view b)
verify-type (typify f Handle Handle bool) VHandle12 VHandle1 VHandle2 bool
test-refcount (inline ()
    (f (Handle 1) (Handle 2) true)
    (f (Handle 1) (Handle 2) false)
    (_))

# same setup, but alternative structure
    R1:Handle<-(1:Handle 2:Handle bool)(*)
fn f (a b x)
    if x
        return a
    b
verify-type (typify f Handle Handle bool) UHandleR1 UHandle1 UHandle2 bool
test-refcount (inline ()
    (f (Handle 1) (Handle 2) true)
    (f (Handle 1) (Handle 2) false)
    (_))

# receives two handles and conditionally moves either one.
    R1:Handle<-(1:Handle 2:Handle bool)(*)
fn f (a b x)
    if x a
    else b
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
        return a
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
    if x (view a)
    else b
assert-error (typify f Handle Handle bool)

# receives a handle and casts a view to i32
    %1:i32<-(%1:Handle)(*)
fn f (h)
    bitcast (view h) i32
verify-type (typify f Handle) Vi321 VHandle1
test-refcount (inline () (f (Handle 1)) (_))

# receives a handle and loses it, then back to Handle
    i32<-(1:Handle)(*)
fn f (h)
    follow (lose h) Handle
verify-type (typify f Handle) UHandleR1 UHandle1
test-refcount (inline () (f (Handle 1)) (_))

# receives a handle, casts it to i32 and back to Handle
    %1:Handle<-(%1:Handle)(*)
fn f (h)
    bitcast (bitcast h i32) Handle
verify-type (typify f Handle) VHandle1 VHandle1
test-refcount (inline () (f (Handle 1)) (_))

# receives four handles and conditionally returns one, switch version
    <view:0|1|2|3>Handle<-(<view>Handle <view>Handle bool)(*)
fn f (a b c d x)
    viewing a b c d
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
    (f (Handle 1) (Handle 2) (Handle 3) (Handle 4) 3)
    (_))

# receives two handles and passes them to a function that conditionally returns one
    %1|2:Handle<-(%1:Handle %2:Handle bool)(*)
fn f (a b c d x)
    viewing a b c d
    fn ff (a b x)
        if x a
        else b
    ff c d x
verify-type (typify f Handle Handle Handle Handle bool)
    \ VHandle34 VHandle1 VHandle2 VHandle3 VHandle4 bool
test-refcount (inline ()
    (f (Handle 1) (Handle 2) (Handle 3) (Handle 4) false)
    (f (Handle 1) (Handle 2) (Handle 3) (Handle 4) true)
    (_))

# receives two handles and passes one to a function that moves the argument,
    then attempts to access the moved argument
    error: cannot access value
fn f (a b)
    fn ff (x) x
    ff b
    dump b
    a
assert-error (typify f Handle Handle)

# attempting to move a parent value into a switch pass
    error: skippable switch pass moved value of type 1000:Handle which is from a parent scope
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

# creating a local unique in a switch pass
    void <- ()
fn f ()
    let x = (Handle 0)
    switch 10
    case 0
    pass 1
        let y = (Handle 1)
        dump y
    pass 2
    case 3
    default
        _;
test-refcount (inline () (f))

# state propagation through different kinds of scopes
    void <- (1:Handle)(*)
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
verify-type (typify f Handle) void UHandle1
test-refcount (inline () (f (Handle 0)))

# return or raise unique value
    R1:Handle <-> E1:Handle (bool)(*)
fn f-raises (k)
    let x y =
        Handle 1; Handle 2
    if k
        raise y
    x
verify-raising-type (typify f-raises bool) UHandleR1 UHandleE1 bool

# call exception raising function which returns handles on all paths
    R1:Handle <- (bool)(*)
fn f (k)
    let z = (Handle 3)
    try
        f-raises k
    except (err)
        z
verify-type (typify f bool) UHandleR1 bool
test-refcount (inline ()
    (f false)
    (f true))

# take two unique parameters and move them through a loop, implicitly discarding
    one and creating new ones occasionally
fn f (x y)
    loop (x i = x 0)
        if (i == 10)
            break x y
        if ((i % 3) == 0)
            repeat (Handle 2) (i + 1)
        if ((i % 3) == 1)
            repeat x (i + 1)
        repeat x (i + 1)
verify-type (typify f Handle Handle) (Arguments UHandleR1 UHandleR2) UHandle1 UHandle2
test-refcount (inline () (f (Handle 0) (Handle 1)))

# move a parameter inside a loop
    error: loop moved value of type 2:Handle which is from a parent scope
fn f (x y)
    loop (x i = x 0)
        if (i == 10)
            break x
        move y
        repeat x (i + 1)
assert-error (typify f Handle Handle)

# receive a handle and stop following it
fn f (x)
    lose x
verify-type (typify f Handle) i32 UHandle1

# TODO: composition, decomposition

fn f ()
    let x y z =
        Handle 0; Handle 1; Handle 2
    let d = (? true x y)
    let e = (? true d z)
    dump e
    dump-uniques;
    #dump (nullof Handle)
    return;

verify-type (typify f) void



# TODO: builtins