
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
        dump "drop"
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

    VHandle12 = ('view-type VHandle1 2)

    VHandle01 = ('view-type VHandle0 1)
    VHandle012 = ('view-type VHandle01 2)

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


# receives handle but doesn't do anything with it:
    void<-(%1:Handle)(*)
fn f (h)
    return;
verify-type (typify f Handle) void VHandle1

# receives handle and passes it through.
    %1:Handle<-(%1:Handle)(*)
fn f (h)
    h
verify-type (typify f Handle) VHandle1 VHandle1

# receives handle, moves it and returns nothing.
    void<-(1:Handle)(*)
fn f (h)
    move h
    return;
verify-type (typify f Handle) void UHandle1

# receives handle, moves and returns it.
    R1:Handle<-(1:Handle)(*)
fn f (h)
    move h
verify-type (typify f Handle) UHandleR1 UHandle1

# receives no arguments and returns new handle
    R1:Handle<-()(*)
fn f ()
    Handle 0
verify-type (typify f) UHandleR1

# receives two handles and returns second handle and a new handle
    λ(%2:Handle R2:Handle)<-(%1:Handle %2:Handle)(*)
fn f (a b)
    viewing a b
    _ b (Handle 0)
verify-type (typify f Handle Handle) (Arguments VHandle2 UHandleR2) VHandle1 VHandle2

# receives two handles, conditionally resolves one and returns nothing.
    void<-(%1:Handle %2:Handle bool)(*)
fn f (a b x)
    if x a
    else b
    return;
verify-type (typify f Handle Handle bool) void VHandle1 VHandle2 bool

# views two handles and conditionally returns one.
    %1|2:Handle<-(%1:Handle %2:Handle bool)(*)
fn f (a b x)
    if x a
    else b
verify-type (typify f Handle Handle bool) VHandle12 VHandle1 VHandle2 bool

# same setup, but alternative structure
    %1|2:Handle<-(%1:Handle %2:Handle bool)(*)
fn f (a b x)
    if x
        return a
    b
verify-type (typify f Handle Handle bool) VHandle12 VHandle1 VHandle2 bool

# receives two handles and conditionally moves either one.
    Handle<-(Handle Handle bool)(*)
fn f (a b x)
    if x (move a)
    else (move b)
verify-type (typify f Handle Handle bool) UHandleR1 UHandle1 UHandle2 bool

# receives two handles, moves both into the function and conditionally
    moves either one.
    Handle<-(Handle Handle bool)(*)
fn f (a b x)
    let a = (move a)
    let b = (move b)
    if x
        return b
    # both a and b still accessible
    dump a b
    a
verify-type (typify f Handle Handle bool) UHandleR1 UHandle1 UHandle2 bool

# receives two handles, moves both into the function and conditionally
    moves either one, but one handle is moved into a subscope.
    error: cannot access value of type 1000:Handle because it has been moved
fn f (a b x)
    let a = (move a)
    let b = (move b)
    if x
        return (move a)
    # a inaccessible
    dump a
    b
assert-error (typify f Handle Handle bool)

# receives two handles and conditionally borrows one, but moves another.
    error: conflicting branch types %1:Handle and 1000:Handle
    we could force move on `a` here but that would be an ambiguous guess
    of programmer intent.
fn f (a b x)
    if x a
    else (move b)
assert-error (typify f Handle Handle bool)

print "ok"
run-stage;


# receives two handles and passes them to a function that conditionally returns one
    <view:0|1>Handle<-(<view>Handle <view>Handle bool)(*)
fn f (a b x)
    fn ff (a b x)
        if x a
        else b
    ff a b x
verify-type (typify f Handle Handle bool) HandleView01 HandleView HandleView bool

# receives four handles and conditionally returns one, switch version
    <view:0|1|2|3>Handle<-(<view>Handle <view>Handle bool)(*)
fn f (a b c d x)
    switch x
    case 0 a
    case 1 b
    case 2 c
    default d
verify-type (typify f Handle Handle Handle Handle i32)
    \ HandleView0123 HandleView HandleView HandleView HandleView i32

#fn f (x)
    fn ff (x) x
    let x = (move x)
    _ x (ff x)
#print (typify f Handle)

# receives a handle and casts it to i32
    <view:0>i32<-(<view>Handle)(*)
fn f (h)
    bitcast h i32
verify-type (typify f Handle) i32View0 HandleView

# receives a handle, casts it to i32 and back to Handle
    <view:0>Handle<-(<view>Handle)(*)
fn f (h)
    bitcast (bitcast h i32) Handle
verify-type (typify f Handle) HandleView0 HandleView

#fn f (h)
    bitcast (bitcast h i32) Handle
#'dump (typify f Handle)

# TODO: composition, decomposition

# TODO: loops

# TODO: try/except/raise

