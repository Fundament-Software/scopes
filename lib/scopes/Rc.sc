#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""Rc
    ==

    A reference counted value that is dropped when all users are dropped. This
    module provides a strong reference type `Rc`, as well as a weak reference
    type `Weak`.

let
    PAYLOAD_INDEX = 0
    METADATA_INDEX = 1
    STRONGRC_INDEX = 0
    WEAKRC_INDEX = 1

typedef ReferenceCounted
    let RefType = i32
    let MetaDataType =
        tuple RefType RefType

typedef Weak < ReferenceCounted
typedef Rc < ReferenceCounted

@@ memo
inline gen-type (T)
    let storage-type =
        tuple
            mutable pointer T
            mutable pointer Weak.MetaDataType
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
            let md = (extractvalue value METADATA_INDEX)
            let refcount =
                getelementptr
                    extractvalue value METADATA_INDEX
                    \ 0 WEAKRC_INDEX
            let rc = (add (load refcount) 1)
            store rc refcount
            bitcast (dupe (view value)) this-type
        case (cls)
            # null-weak that will never upgrade
            let self = (nullof storage-type)
            bitcast self this-type

    typedef+ RcType
        let Type = T
        let WeakType = WeakType

        fn wrap (value)
            let self = (nullof storage-type)
            let ptr = (malloc T)
            store value ptr
            let self = (insertvalue self ptr PAYLOAD_INDEX)
            let mdptr = (malloc super-type.MetaDataType)
            store 1 (getelementptr mdptr 0 STRONGRC_INDEX)
            store 0 (getelementptr mdptr 0 WEAKRC_INDEX)
            let self = (insertvalue self mdptr METADATA_INDEX)
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
        let md = (extractvalue value METADATA_INDEX)
        if (not (ptrtoint md usize))
            return 0
        load (getelementptr md 0 STRONGRC_INDEX)

    fn weak-count (value)
        viewing value
        let md = (extractvalue value METADATA_INDEX)
        if (not (ptrtoint md usize))
            return 1
        load (getelementptr md 0 WEAKRC_INDEX)

typedef+ Weak
    inline... __typecall
    case (cls, T : type)
        (gen-type T) . WeakType

    fn _drop (self)
        let md = (extractvalue self METADATA_INDEX)
        if (not (ptrtoint md usize))
            return;
        let refcount = (getelementptr md 0 WEAKRC_INDEX)
        let rc = (sub (load refcount) 1)
        assert (rc >= 0) "corrupt refcount encountered"
        store rc refcount
        if (rc == 0)
            let strongrefcount = (getelementptr md 0 STRONGRC_INDEX)
            if ((load strongrefcount) == 0)
                free md
            # otherwise last strong reference will clean this up

    inline __drop (self)
        _drop (deref self)

    @@ memo
    inline __== (cls other-cls)
        static-if (cls == other-cls)
            fn (self other)
                == (extractvalue self METADATA_INDEX) (extractvalue other METADATA_INDEX)

    fn... clone (self : Weak,)
        viewing self
        let md = (extractvalue self METADATA_INDEX)
        if (ptrtoint md usize)
            let refcount =
                getelementptr md 0 WEAKRC_INDEX
            let rc = (add (load refcount) 1)
            store rc refcount
        deref (dupe self)

    fn upgrade (self)
        viewing self
        let md = (extractvalue self METADATA_INDEX)
        if (not (ptrtoint md usize))
            raise (UpgradeError)
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
        let md = (extractvalue self METADATA_INDEX)
        assert (ptrtoint md usize) "upgrading Weak failed"
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

    fn... clone
    case (value : Rc,)
        viewing value
        let refcount =
            getelementptr
                extractvalue value METADATA_INDEX
                \ 0 STRONGRC_INDEX
        let rc = (load refcount)
        assert (rc >= 0) "corrupt refcount encountered"
        let rc = (add rc 1)
        store rc refcount
        deref (dupe value)
    case (value : Weak,)
        viewing value
        'clone value

    inline wrap (value)
        ((gen-type (typeof value)) . wrap) value

    let _view = view
    inline... view (self : Rc,)
        ptrtoref (deref (extractvalue self PAYLOAD_INDEX))

    inline __countof (self)
        countof (view self)

    inline __= (selfT otherT)
        static-if (selfT == otherT)
            super-type.__= selfT otherT
        else
            inline (lhs rhs)
                (view lhs) = rhs

    inline __@ (self keys...)
        @ (view self) keys...

    spice __methodcall (symbol self args...)
        'tag `(symbol (view self) args...) ('anchor args)

    spice __getattr (self key)
        'tag `(getattr (view self) key) ('anchor args)

    spice __repr (self)
        'tag `(forward-repr (view self)) ('anchor args)

    @@ memo
    inline __== (cls other-cls)
        static-if (cls == other-cls)
            inline (self other)
                == (extractvalue self PAYLOAD_INDEX) (extractvalue other PAYLOAD_INDEX)

    inline __hash (self)
        hash (extractvalue self PAYLOAD_INDEX)

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
        returning void
        let md = (extractvalue self METADATA_INDEX)
        let refcount = (getelementptr md 0 STRONGRC_INDEX)
        let rc = (sub (load refcount) 1)
        assert (rc >= 0) "corrupt refcount encountered"
        if (rc == 0)
            let payload = (view self)
            __drop payload
            free (reftoptr payload)
            store 0 refcount
            let weakrefcount = (getelementptr md 0 WEAKRC_INDEX)
            if ((load weakrefcount) == 0)
                free md
            # otherwise last weak reference will clean this up
        else
            store rc refcount

    inline __drop (self)
        _drop (deref self)

    unlet _view _drop

do
    let Rc Weak UpgradeError
    locals;
