
#-------------------------------------------------------------------------------
# compile time closures
#-------------------------------------------------------------------------------

let Capture = (typename "Capture")
let MutableCapture = (typename "MutableCapture" (super = Capture))

'set-symbols MutableCapture
    __copy =
        fn (self other)
            (type@& tuple '__copy) self other

define-syntax-macro capture
    fn make-typename (TT)
        let T =
            typename
                .. "Capture"
                    tostring TT
                super = Capture
                storage = TT
        T
    inline convert (self TT)
        unpack (bitcast self TT)
    inline convert& (self TT)
        unpack (bitcast& self TT)

    let args params body = (decons args 2)
    let arglist = (args as Syntax as Any as list)
    let head arglist = (decons arglist)
    if
        or (('typeof (head as Syntax as Any)) != Symbol)
            ((head as Syntax as Symbol) != 'square-list)
        syntax-error! head "square brackets expected"
    let params = (params as Syntax as list)
    let T = (Parameter 'T)
    let EV = (Parameter 'EV)
    let TT = (Parameter 'TT)
    let self = (Parameter 'self)
    list do
        list let EV '=
            cons tupleof arglist
        list let TT '= (list typeof EV)
        list let T '=
            list make-typename TT
                list fn '()
        list set-type-symbol! T (list quote '__call)
            cons fn (cons self params)
                cons let
                    .. arglist
                        list '= (list convert self TT)
                body
        list bitcast EV T

define-macro capture&
    fn make-typename (TT)
        let T =
            typename
                .. "MutableCapture"
                    string-repr TT
                super = MutableCapture
                storage = TT
        T
    inline convert& (self TT)
        unpack (bitcast& self TT)

    let args params body = (decons args 2)
    let arglist = (args as Syntax as Any as list)
    let head arglist = (decons arglist)
    if
        or (('typeof (head as Syntax as Any)) != Symbol)
            ((head as Syntax as Symbol) != 'square-list)
        syntax-error! head "square brackets expected"
    let params = (params as Syntax as list)
    let T = (Parameter 'T)
    let EV = (Parameter 'EV)
    let TT = (Parameter 'TT)
    let self = (Parameter 'self)
    list do
        list let EV '=
            cons tupleof arglist
        list let TT '= (list typeof EV)
        list let T '=
            list make-typename TT
                list fn '()
        list set-type-symbol!& T (list quote '__call)
            cons fn (cons self params)
                cons let
                    .. arglist
                        list '= (list convert& self TT)
                body
        list local (list quote 'copy)
            list bitcast EV T

locals;
