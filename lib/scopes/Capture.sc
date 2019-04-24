#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""Capture
    =======

    A capture is a runtime closure that transparently captures (hence the name)
    runtime values outside of the function.

#-------------------------------------------------------------------------------
# runtime closures
#-------------------------------------------------------------------------------

typedef Capture
    inline __call (self args...)
        let f env = (unpack (storagecast self))
        f env args...

    @@ memo
    inline make-type (ftype)
        let ET = (elementof ftype)
        static-assert (ET < function)
        let ST = (tuple ftype voidstar)
        typedef (.. "Capture<" (tostring ET) ">") < this-type :: ST

    inline __drop (self)
        let f env = (unpack (storagecast self))
        free (bitcast (dupe env) (mutable pointer i8))

    inline __typecall (cls args...)
        static-if (cls == this-type)
            let ftype = args...
            make-type ftype
        else
            let f envtuple = args...
            let env = (malloc (typeof envtuple))
            store envtuple env
            let closure = (tupleof f (env as voidstar))
            follow closure cls

typedef CaptureTemplate
    inline typify (self types...)
        let T = (typeof self)
        let T* = (pointer T)
        let innerf = (T . __call)
        let newf =
            static-typify
                fn (env args...)
                    innerf
                        load (bitcast env T*)
                        args...
                \ voidstar types...
        (Capture (typeof newf)) newf self

typedef SpiceCapture

inline capture-parser (macroname head body genf)
    let T = ('typeof head)
    let symbol? = (T == Symbol)
    let namestr body =
        if symbol?
            _ (head as Symbol as string) body
        elseif (T == string)
            _ (head as string) body
        else
            _ "" (cons head body)
    let expr =
        sugar-match body
        case ((params...) ('curly-list args...) body...)
            genf namestr args... params... body...
        default
            error
                .. "syntax error: try (" macroname
                    " name|\"name\"| (param ...) {var ...} body ...)"
    if symbol?
        list let head '= expr
    else
        expr

spice unpack-capture (capture)
    let T = ('storageof ('typeof capture))
    `(unpack (bitcast capture T))

spice pack-capture (argtuple func)
    let T = ('typeof argtuple)
    let CaptureT =
        @@ spice-quote
        typedef [(.. "CaptureT" ('string T))] < CaptureTemplate : T
            let __call = func
    spice-quote
        bitcast argtuple CaptureT

# capture name|"name"| (param ...) {var ...} body ...
sugar capture (head body...)
    capture-parser "capture" head body...
        inline (namestr args params body)
            qq [pack-capture] ([tupleof] (unquote-splice args))
                [fn] [namestr] (self (unquote-splice params))
                    [let] (unquote-splice args) =
                        [unpack-capture] self
                    unquote-splice body

#-------------------------------------------------------------------------------

spice unpack-capture-spice (capture T)
    let T = (T as type)
    let ST = ('storageof T)
    `(unpack (bitcast (capture as T) ST))

spice pack-capture-spice (argtuple)
    let T = ('typeof argtuple)
    let CaptureT =
        @@ spice-quote
        typedef [(.. "Capture" ('string T))] < SpiceCapture : T
    spice-quote
        _
            bitcast argtuple CaptureT
            CaptureT

spice finalize-capture-spice (capture func)
    let T = ('typeof capture)
    spice-quote
        'set-symbol T '__call func
        capture

# spice-capture name|"name"| (param ...) {var ...} body ...
sugar spice-capture (head body...)
    capture-parser "spice-capture" head body...
        inline (namestr args params body)
            let payload = ('unique Symbol "payload")
            let payload-type = ('unique Symbol "payload-type")
            qq [do]
                [let] [payload] [payload-type] =
                    [pack-capture-spice] ([tupleof] (unquote-splice args))
                [finalize-capture-spice] [payload]
                    [spice] [namestr] (self (unquote-splice params))
                        [let] (unquote-splice args) =
                            [unpack-capture-spice] self [payload-type]
                        unquote-splice body

#-------------------------------------------------------------------------------

unlet unpack-capture pack-capture unpack-capture-spice pack-capture-spice
    \ finalize-capture-spice

do
    let Capture CaptureTemplate SpiceCapture capture spice-capture
    let decorate-capture = decorate-fn
    locals;
