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
    @@ memo
    inline gen-type (T)
        let content-type = (tuple T i32)
        typedef (.. "<RC " (tostring T) ">") < this-type
            \ :: (mutable pointer content-type)
            let Type = T

            fn wrap (value)
                let ptr = (malloc content-type)
                store value (getelementptr ptr 0 PAYLOAD_INDEX)
                store 1 (getelementptr ptr 0 REF_INDEX)
                bitcast ptr this-type

            inline __typecall (cls args...)
                wrap (T args...)

    inline... __typecall
    case (cls, T : type)
        gen-type T

    inline new (T args...)
        (gen-type T) args...

    fn refcount (value)
        viewing value
        let refcount = (getelementptr (storagecast value) 0 REF_INDEX)
        load refcount

    fn clone (value)
        viewing value
        let refcount = (getelementptr (storagecast value) 0 REF_INDEX)
        let rc = (add (load refcount) 1)
        store rc refcount
        dupe value

    inline wrap (value)
        ((gen-type (typeof value)) . wrap) value

    inline view (self)
        ptrtoref (getelementptr (view self) 0 PAYLOAD_INDEX)

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
        let refcount = (getelementptr (storagecast self) 0 REF_INDEX)
        let rc = (sub (load refcount) 1)
        assert (rc >= 0)
        if (rc > 0)
            store rc refcount
        else
            __drop (view self)
            free self

    unlet gen-type

do
    let Rc
    locals;
