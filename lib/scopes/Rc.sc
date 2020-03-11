#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""Rc
    ==

    A reference counted value that is dropped when all users are dropped.

let
    PAYLOAD_INDEX = 0
    REF_INDEX = 1

typedef Rc
    let RefType = i32

    @@ memo
    inline gen-type (T)
        let storage-type =
            tuple
                mutable pointer T
                mutable pointer RefType
        typedef (.. "<RC " (tostring T) ">") < this-type
            \ :: storage-type

            let Type = T

            fn wrap (value)
                let ref = (malloc RefType)
                let ptr = (malloc T)
                store value ptr
                store 1 ref
                let self = (nullof storage-type)
                dump self
                let self = (insertvalue self ptr PAYLOAD_INDEX)
                let self = (insertvalue self ref REF_INDEX)
                bitcast self this-type

            inline __typecall (cls args...)
                wrap (T args...)

    inline... __typecall
    case (cls, T : type)
        gen-type T

    inline new (T args...)
        (gen-type T) args...

    fn refcount (value)
        viewing value
        let refcount = (extractvalue value REF_INDEX)
        load refcount

    fn clone (value)
        viewing value
        let refcount = (extractvalue value REF_INDEX)
        let rc = (add (load refcount) 1)
        store rc refcount
        dupe value

    inline wrap (value)
        ((gen-type (typeof value)) . wrap) value

    inline view (self)
        ptrtoref (deref (extractvalue self PAYLOAD_INDEX))

    inline __countof (self)
        countof (view self)

    spice __= (selfT otherT)
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

    inline make-cast-op (f const?)
        spice "box-cast" (selfT otherT)
            selfT as:= type
            otherT as:= type
            let selfT = (('@ selfT 'Type) as type)
            let conv = (f selfT otherT const?)
            if (operator-valid? conv)
                return `(inline (value) (conv (view value)))
            return `()

    let __imply = (make-cast-op imply-converter false)
    let __static-imply = (make-cast-op imply-converter true)
    let __as = (make-cast-op as-converter false)

    fn __drop (self)
        let refcount = (deref (extractvalue self REF_INDEX))
        let rc = (sub (load refcount) 1)
        assert (rc >= 0)
        if (rc > 0)
            store rc refcount
        else
            let payload = (view self)
            __drop payload
            free (reftoptr payload)
            free refcount

    unlet gen-type

do
    let Rc
    locals;
