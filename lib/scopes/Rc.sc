#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""Rc
    ==

    A reference counted value that is dropped when all users are dropped. This
    module provides a strong reference type `Rc`, as well as a weak reference
    type `Weak`.

define DEBUG_DOUBLE_FREES false

let
    STRONGRC_INDEX = 0
    WEAKRC_INDEX = 1

let RefType = i32
let MetaDataType =
    tuple RefType RefType

let HEADERSIZE = 16:usize

typedef ReferenceCounted
    let RefType = RefType
    let MetaDataType = MetaDataType

inline _mdptr (self)
    inttoptr (sub (ptrtoint (view self) usize) (sizeof MetaDataType)) (mutable @MetaDataType)
inline _baseptr (self)
    let T = (elementof (typeof self))
    let ptr =
        inttoptr (sub (ptrtoint (view self) usize) HEADERSIZE) (mutable @u8)
    ptr

typedef Weak < ReferenceCounted
typedef Rc < ReferenceCounted

let use-rc free-rc =
    static-if DEBUG_DOUBLE_FREES
        using import Set
        global used-pointers : (Set (mutable @u8))

        inline use-rc (ptr)
            'insert used-pointers ptr

        inline free-rc (self)
            let ptr = (_baseptr self)
            if ('in? used-pointers ptr)
                'discard used-pointers ptr
            else
                assert false "Rc: double free detected"
                unreachable;
            free ptr

        _ use-rc free-rc
    else
        _ (inline ()) (inline ())

@@ memo
inline gen-type (T)
    let storage-type =
        mutable pointer T
    let WeakType =
        typedef (.. "<Weak " (tostring T) ">") < Weak
            \ :: storage-type
    let RcType =
        typedef (.. "<Rc " (tostring T) ">") < Rc
            \ :: storage-type

    typedef+ WeakType
        let Type = T
        let RcType = RcType

        inline... __typecall
        case (cls, value : RcType)
            let md = (_mdptr value)
            let refcount =
                getelementptr md 0 WEAKRC_INDEX
            let rc = (add (load refcount) 1)
            store rc refcount
            bitcast (dupe (view value)) this-type
        case (cls)
            # null-weak that will never upgrade
            inttoptr 0:usize this-type

    typedef+ RcType
        let Type = T
        let WeakType = WeakType

        fn wrap (value)
            let self = (nullof storage-type)
            let MDT = super-type.MetaDataType
            let mdsize = (sizeof MDT)
            let fullsize = (HEADERSIZE + (sizeof T))
            let ptr = (malloc-array u8 fullsize)
            use-rc ptr
            let self = (inttoptr (add (ptrtoint ptr usize) HEADERSIZE) storage-type)
            store value self
            let mdptr = (_mdptr self)
            store 1 (getelementptr mdptr 0 STRONGRC_INDEX)
            store 0 (getelementptr mdptr 0 WEAKRC_INDEX)
            bitcast self this-type

        inline __typecall (cls args...)
            wrap (T args...)
    RcType

typedef UpgradeError : (tuple)
    inline __typecall (cls)
        bitcast none this-type

typedef+ ReferenceCounted
    fn strong-count (value)
        viewing value
        if (not (ptrtoint value usize))
            return 0
        let md = (_mdptr value)
        dupe (load (getelementptr md 0 STRONGRC_INDEX))

    fn weak-count (value)
        viewing value
        if (not (ptrtoint value usize))
            return 1
        let md = (_mdptr value)
        dupe (load (getelementptr md 0 WEAKRC_INDEX))

typedef+ Weak
    inline... __typecall
    case (cls, T : type)
        (gen-type T) . WeakType

    fn _drop (self)
        if (not (ptrtoint (view self) usize))
            return;
        let md = (_mdptr self)
        let refcount = (getelementptr md 0 WEAKRC_INDEX)
        let rc = (sub (load refcount) 1)
        assert (rc >= 0) "corrupt refcount encountered"
        store rc refcount
        if (rc == 0)
            let strongrefcount = (getelementptr md 0 STRONGRC_INDEX)
            if ((load strongrefcount) == 0)
                free-rc self
            # otherwise last strong reference will clean this up

    inline __rimply (T cls)
        static-if (T == Nothing)
            inline (value) (cls)

    inline __drop (self)
        _drop (deref self)

    @@ memo
    inline __== (cls other-cls)
        static-if (cls == other-cls)
            fn (self other)
                == (storagecast (view self)) (storagecast (view other))

    fn... __copy (self : Weak,)
        viewing self
        if (ptrtoint self usize)
            let md = (_mdptr self)
            let refcount =
                getelementptr md 0 WEAKRC_INDEX
            let rc = (add (load refcount) 1)
            store rc refcount
        deref (dupe self)

    fn upgrade (self)
        viewing self
        if (not (ptrtoint self usize))
            raise (UpgradeError)
        let md = (_mdptr self)
        let refcount = (getelementptr md 0 STRONGRC_INDEX)
        let rc = (load refcount)
        assert (rc >= 0) "corrupt refcount encountered"
        if (rc == 0)
            raise (UpgradeError)
        let RcType = ((typeof self) . RcType)
        let rc = (add rc 1)
        store rc refcount
        deref (bitcast (dupe self) RcType)

    fn force-upgrade (self)
        viewing self
        assert (ptrtoint self usize) "upgrading Weak failed"
        let md = (_mdptr self)
        let refcount = (getelementptr md 0 STRONGRC_INDEX)
        let rc = (load refcount)
        assert (rc >= 0) "corrupt refcount encountered"
        assert (rc > 0) "upgrading Weak failed"
        let RcType = ((typeof self) . RcType)
        let rc = (add rc 1)
        store rc refcount
        deref (bitcast (dupe self) RcType)

typedef+ Rc
    inline... __typecall
    case (cls, T : type)
        gen-type T

    inline new (T args...)
        (gen-type T) args...

    fn... __copy (value : Rc,)
        viewing value
        let md = (_mdptr value)
        let refcount =
            getelementptr md 0 STRONGRC_INDEX
        let rc = (load refcount)
        assert (rc >= 0) "corrupt refcount encountered"
        let rc = (add rc 1)
        store rc refcount
        deref (dupe value)

    inline wrap (value)
        ((gen-type (typeof value)) . wrap) value

    let _view = view
    inline... view (self : Rc,)
        ptrtoref (_view self)

    inline __countof (self)
        countof (view self)

    inline __= (selfT otherT)
        let otherT = (unqualified otherT)
        static-if (selfT == otherT)
            super-type.__= selfT otherT
        else
            inline (lhs rhs)
                (view lhs) = rhs

    inline __@ (self keys...)
        @ (view self) keys...

    inline __call (self ...)
        (view self) ...

    spice __methodcall (symbol self args...)
        'tag `(symbol (view self) args...) ('anchor args)

    spice __getattr (self key)
        'tag `(getattr (view self) key) ('anchor args)

    inline __repr (self)
        .. "(Rc " (repr (view self)) ")"

    @@ memo
    inline __== (cls other-cls)
        static-if (cls == other-cls)
            inline (self other)
                == (storagecast (_view self)) (storagecast (_view other))
        elseif (other-cls == cls.Type)
            inline (self other)
                == (@ (storagecast (_view self))) other

    @@ memo
    inline __r== (other-cls cls)
        static-if (other-cls == cls.Type)
            inline (other self)
                == other (@ (storagecast (_view self)))

    inline __hash (self)
        hash (storagecast (_view self))

    inline make-cast-op (f const?)
        spice "box-cast" (selfT otherT)
            selfT as:= type
            otherT as:= type
            static-if (not const?)
                let WeakT = ('@ selfT 'WeakType)
                if ((otherT == Weak) or (otherT == (WeakT as type)))
                    return WeakT
            let selfT = (('@ selfT 'Type) as type)
            let conv = (f selfT otherT const?)
            if (operator-valid? conv)
                return `(inline (value) (conv (view value)))
            return `()

    let __imply = (make-cast-op imply-converter false)
    let __static-imply = (make-cast-op imply-converter true)
    let __as = (make-cast-op as-converter false)

    fn _drop (self)
        viewing self
        returning void
        let md = (_mdptr self)
        let refcount = (getelementptr md 0 STRONGRC_INDEX)
        let rc = (sub (load refcount) 1)
        assert (rc >= 0) "corrupt refcount encountered"
        if (rc == 0)
            let payload = (view self)
            __drop payload
            # update refcount after drop so weak pointers don't attempt to
                delete this pointer prematurely
            store 0 refcount
            let weakrefcount = (getelementptr md 0 WEAKRC_INDEX)
            if ((load weakrefcount) == 0)
                free-rc self
            # otherwise the last weak reference will free this value
        else
            store rc refcount

    inline __drop (self)
        _drop (deref self)

    unlet _view _drop

do
    let Rc Weak UpgradeError
    locals;
