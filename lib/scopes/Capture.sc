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
        case (('curly-list args...) (params...) body...)
            genf namestr args... params... body...
        default
            error
                .. "syntax error: try (" macroname
                    " name|\"name\"| {var ...} (param ...) body ...)"
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
        typedef [(.. "Capture" ('string T))] < Capture : T
            let __call = func
    spice-quote
        bitcast argtuple CaptureT

# capture name|"name"| {var ...} (param ...) body ...
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
        typedef [(.. "Capture" ('string T))] < Capture : T
    spice-quote
        _
            bitcast argtuple CaptureT
            CaptureT

spice finalize-capture-spice (capture func)
    let T = ('typeof capture)
    spice-quote
        'set-symbol T '__call func
        capture

# spice-capture name|"name"| {var ...} (param ...) body ...
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
    let Capture capture spice-capture
    let decorate-capture = decorate-fn
    locals;
