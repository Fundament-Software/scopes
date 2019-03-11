
using import testing

global refcount = 0

typedef Handle :: i32

    @@ spice-quote
    fn new-handle (idx)
        refcount += 1
        bitcast idx this-type

    @@ spice-quote
    inline __typecall (cls idx)
        #print "new handle" idx
        new-handle idx

    inline __repr (self)
        repr (storagecast self)

    fn __drop (self)
        refcount -= 1
        #print "drop handle" (storagecast self)
        return;

let HandleView = ('view-type Handle)
let HandleView0 = ('view-type Handle 0)
let HandleView1 = ('view-type Handle 1)
let HandleView01 = ('view-type HandleView0 1)
let HandleView0123 = ('view-type ('view-type HandleView01 2) 3)
let i32View = ('view-type i32)
let i32View0 = ('view-type i32View 0)

run-stage;

# move x - take ownership
# forget x - no longer track object

fn verify-type (f ret args...)
    let Ta = ('typeof f)
    let Tb = ('pointer (function ret args...))
    assert (Ta == Tb)
        .. (repr Ta) " == " (repr Tb)


# receives handle but doesn't do anything with it:
    void<-(<view>Handle)(*)
fn f (h)
    return;
verify-type (typify f Handle) void HandleView

# receives handle and passes it through.
    <view:0>Handle<-(<view>Handle)(*)
fn f (h) h
verify-type (typify f Handle) HandleView0 HandleView

# receives handle, moves it and returns nothing.
    void<-(Handle)(*)
fn f (h)
    move h
    return;
verify-type (typify f Handle) void Handle

# receives handle, moves and returns it.
    Handle<-(Handle)(*)
fn f (h)
    move h
verify-type (typify f Handle) Handle Handle

# receives no arguments and returns new handle
    Handle<-()(*)
fn f ()
    Handle 0
verify-type (typify f) Handle

# receives two handles and returns second handle and a new handle
    λ(<view:1>Handle Handle)<-(<view>Handle <view>Handle)(*)
fn f (a b)
    _ b (Handle 0)
verify-type (typify f Handle Handle) (Arguments HandleView1 Handle) HandleView HandleView

# receives two handles, conditionally resolves one and returns nothing.
    void<-(<view>Handle <view>Handle bool)(*)
fn f (a b x)
    if x a
    else b
    return;
verify-type (typify f Handle Handle bool) void HandleView HandleView bool

# receives two handles and conditionally returns one.
    <view:0|1>Handle<-(<view>Handle <view>Handle bool)(*)
fn f (a b x)
    if x a
    else b
verify-type (typify f Handle Handle bool) HandleView01 HandleView HandleView bool

# same setup, but alternative structure
    <view:0|1>Handle<-(<view>Handle <view>Handle bool)(*)
fn f (a b x)
    if x
        return a
    b
verify-type (typify f Handle Handle bool) HandleView01 HandleView HandleView bool

# receives two handles and conditionally moves either one.
    Handle<-(Handle Handle bool)(*)
fn f (a b x)
    if x (move a)
    else (move b)
verify-type (typify f Handle Handle bool) Handle Handle Handle bool

# receives two handles and conditionally borrows one, but moves another.
    error: conflicting attempt in branch-merge to move or view unique value(s)
    we could force move on `a` here but that would be an ambiguous guess
    of programmer intent.
fn f (a b x)
    if x a
    else (move b)
assert-error (typify f Handle Handle bool)

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

