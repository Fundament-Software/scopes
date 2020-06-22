
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
        repr (dupe self)

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

    UHandleHandleR1 = ('unique-type (tuple Handle Handle) -1)

    UHandleE1 = ('unique-type Handle -257)

    VHandle = ('view-type Handle)

    VHandle0 = ('view-type Handle 0)
    VHandle1 = ('view-type Handle 1)
    VHandle2 = ('view-type Handle 2)
    VHandle3 = ('view-type Handle 3)
    VHandle4 = ('view-type Handle 4)

let
    VHandle12 = ('view-type VHandle1 2)
    VHandle34 = ('view-type VHandle3 4)

let
    VHandle1234 = ('view-type ('view-type VHandle12 3) 4)

    Vi321 = ('view-type i32 1)

run-stage;

# function type constructor canonicalizes types
fn verify-signature (Ta Tb)
    test (Ta == Tb)
        .. (repr Ta) "==" (repr Tb)
    print Ta "==" Tb

fn verify-raising-type (f ret raises args...)
    let Ta = ('typeof f)
    let Tb = (pointer.type ('raises (function.type ret args...) raises))
    verify-signature Ta Tb

fn verify-type (f ret args...)
    let Ta = ('typeof f)
    let Tb = (pointer.type (function.type ret args...))
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
    if (refcount != 0)
        print "pre-refcount should be zero but is" (deref refcount)
    test (refcount == 0)
    testfunc;
    if (refcount != 0)
        print "post-refcount should be zero but is" (deref refcount)
    test (refcount == 0)

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
test-error (typify f Handle)

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
test-error (typify f Handle Handle bool)

# receives a handle and casts a view to i32
    %1:i32<-(%1:Handle)(*)
fn f (h)
    bitcast (view h) i32
verify-type (typify f Handle) Vi321 VHandle1
test-refcount (inline () (f (Handle 1)) (_))

# receives a handle and dupes it, then creates a new Handle
    i32<-(1:Handle)(*)
fn f (h)
    bitcast (dupe h) Handle
verify-type (typify f Handle) UHandleR1 UHandle1
test-refcount (inline () (f (Handle 1)) (_))

# receives a handle, casts a view to i32 and back to Handle
    %1:Handle<-(%1:Handle)(*)
fn f (h)
    bitcast (bitcast (view h) i32) Handle
verify-type (typify f Handle) VHandle1 VHandle1
test-refcount (inline () (f (Handle 1)) (_))

# receives a handle, casts it to i32 and back to Handle
    R1:Handle<-(1:Handle)(*)
fn f (h)
    bitcast (bitcast h i32) Handle
verify-type (typify f Handle) UHandleR1 UHandle1
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
test-error (typify f Handle Handle)

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
test-error (typify f)

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
    pass 3
    do;
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
        pass 3
        do;
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
test-error (typify f Handle Handle)

# receive a handle, stop following it
fn f (x)
    lose x
verify-type (typify f Handle) void UHandle1

# receive a handle and make a copy of its storage
fn f (x)
    dupe (storagecast (view x))
verify-type (typify f Handle) i32 VHandle1

# receive a handle and convert it to storage
fn f (x)
    dupe (storagecast x)
verify-type (typify f Handle) i32 UHandle1

# allocate a mutable reference from constructor and from another handle
    and make a bunch of mutations
fn f ()
    # stack var, default constructor
    local x : Handle 0
    # stack var, init from immutable
    local h = (Handle 0) # immutable is moved into h
    # heap var, default constructor
    local q : Handle 0
    # drop h, move immutable into h
    h = (Handle 1)
    # drop x, move h into x
    x = h
    # auto-drop q and free its memory
    # auto-drop x
    ;
test-refcount (inline () (f))
f;

# TODO: composition, decomposition

inline swapvalue (a b i)
    """"swap out element #i from collection `a` with `b`
    # returns a view
    let oldb = (extractvalue a i)
    # turn old view to new unique
    let oldb = (bitcast (dupe oldb) (typeof oldb))
    # moves a and b, leaks existing value
    let newa = (insertvalue (move a) (move b) i)
    # return new collection and old element
    _ newa oldb

fn f ()
    let k = (undef (tuple Handle Handle))
    let k _old = (swapvalue k (Handle 1) 0)
    lose _old
    let k _old = (swapvalue k (Handle 2) 1)
    lose _old
    k
verify-type (typify f) UHandleHandleR1
test-refcount (inline () (f))

fn f ()
    let k = (tupleof (Handle 1) (Handle 2))
    ;
test-refcount (inline () (f))

# make viewed tuple when some arguments are views
fn f ()
    tupleof (One 200) (view (One 100))
    ;
test-refcount (inline () (f))

# globals

global x : One 303
fn get_x () x
print (get_x)
# global lives forever
test-error (One.test-refcount-balanced)

# TODO: builtins

fn test-drop ()
    # testing destructor order

    using import struct
    global idx = 0
    struct LoudDrop
        index : i32
        inline __typecall (cls)
            print "creating" (deref idx)
            local index = idx
            idx += 1
            super-type.__typecall cls
                index = index
        inline __drop (self)
            print "dropping" self.index
            idx -= 1
            assert (idx == self.index)
            ;

    dump (LoudDrop)
    dump (LoudDrop)
    dump (LoudDrop)
    dump (LoudDrop)
    dump (LoudDrop)
    dump (LoudDrop)
    dump (LoudDrop)
    dump (LoudDrop)
    dump (LoudDrop)
    dump (LoudDrop)
    dump (LoudDrop)
    dump (LoudDrop)
    return;

test-drop;
print "done"

do
    # test viewing on plain arguments

    using import struct

    fn make (json-string)

    fn parse (json-string)
        viewing json-string
        make json-string

    fn make2 (json-string)
        viewing json-string

    fn parse2 (json-string)
        make2 json-string

    # unique
    struct M
        s : string
    parse (nullof M)
    parse2 (nullof M)
    unlet M
    struct M plain
        s : string
    parse (nullof M)
    parse2 (nullof M)

;