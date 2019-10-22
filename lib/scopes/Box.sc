
typedef Box

    @@ memo
    inline gen-type (T)
        typedef (.. "<Box " (tostring T) ">") < this-type :: (mutable pointer T)
            let Type = T
            inline __typecall (cls args...)
                let ptr = (malloc T)
                store (T args...) ptr
                bitcast ptr this-type

    inline... __typecall
    case (cls, T : type)
        gen-type T

    inline new (T args...)
        (gen-type T) args...

    inline wrap (value)
        let ET = (typeof value)
        let ptr = (malloc ET)
        store value ptr
        bitcast ptr (gen-type ET)

    inline view (self)
        ptrtoref (storagecast (view self))

    inline __countof (self)
        countof (view self)

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

    inline __drop (self)
        __drop (view self)
        free self

    unlet gen-type

do
    let Box
    locals;
