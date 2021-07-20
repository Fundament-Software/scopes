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

let DropFuncType =
    pointer (function void voidstar)

@@ memo
inline dropper (T)
    static-typify
        fn (env)
            let tmp = (@ (bitcast env (mutable pointer T)))
            __drop (view tmp)
            lose tmp
            ;
        voidstar

typedef Capture
    inline __call (self args...)
        let f env = (unpack (storagecast (view self)))
        f env args...

    @@ memo
    inline make-type (ftype fdrop)
        static-assert (ftype < function) "function type expected"
        let ST = (tuple (pointer ftype) voidstar DropFuncType)
        typedef (.. "Capture<" (tostring ftype) ">") < this-type :: ST
            let FunctionType = ftype

    inline __drop (self)
        let f env dropf = (unpack (storagecast (view self)))
        dropf env
        free (bitcast env (mutable pointer i8))

    inline __typecall (cls args...)
        static-if (cls == this-type)
            let ftype = args...
            make-type ftype
        else
            let f envtuple = args...
            let envT = (typeof envtuple)
            let env = (malloc envT)
            store envtuple env
            let closure = (tupleof f (env as voidstar) (dropper envT))
            bitcast closure cls

    inline function (return-type param-types...)
        function return-type (viewof voidstar 1) param-types...

typedef CaptureTemplate
    inline __drop (self)
        __drop (storagecast self)

    @@ spice-cast-macro
    fn __imply (selfT otherT)
        if (otherT < Capture)
            let ft = (('@ otherT 'FunctionType) as type)
            let count = ('element-count ft)
            let typeargs =
                sc_argument_list_map_new (count - 1)
                    inline (i)
                        let i = (i + 1)
                        'element@ ft i
            let F = (sc_prove `('typify-function selfT typeargs))
            if (('element@ ('typeof F) 0) == ft)
                return `(inline (self) ('build-instance self F))
        `()

    inline typify-function (cls types...)
        let T* = (pointer cls)
        let innerf = (cls . __call)
        static-typify
            fn (env args...)
                innerf
                    ptrtoref (bitcast env T*)
                    args...
            \ (viewof voidstar 1) types...

    inline build-instance (self f)
        (Capture (elementof (typeof f))) f self

    inline instance (self types...)
        let newf = (typify-function (typeof self) types...)
        build-instance self newf

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
            vvv bind argnames unreflist argvalues
            loop (argnames unreflist argvalues args = '() '() '() args...)
                sugar-match args
                case ((name as Symbol) rest...)
                    let namestr = (name as string)
                    let k = (decons args)
                    if ((lslice namestr 1) == "&")
                        let anchor = ('anchor k)
                        # reference
                        let k =
                            'tag `[(Symbol (rslice namestr 1))] anchor
                        let expr = ('tag `[(qq [&] ([view] [k]))] anchor)
                        let unreflist =
                            cons
                                qq [let] [k] = ([@] [k])
                                unreflist
                        repeat (cons k argnames) unreflist (cons expr argvalues) rest...
                    else
                        repeat (cons k argnames) unreflist (cons k argvalues) rest...
                case (('view (name as Symbol)) rest...)
                    let w = (decons args)
                    let x k = (decons (w as list) 2)
                    repeat (cons k argnames) unreflist (cons w argvalues) rest...
                case ()
                    break ('reverse argnames) unreflist ('reverse argvalues)
                default
                    hide-traceback;
                    let k = (decons args)
                    error@ ('anchor k) "while parsing capture names"
                        \ "syntax error: captured names must have format {var|(view var) ...}"
            genf namestr argnames unreflist argvalues params... body...
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
    `(unpack (bitcast (view capture) T))

spice pack-capture (argtuple func)
    let T = ('typeof argtuple)
    let CaptureT =
        typename.type (.. "CaptureT" ('string T)) CaptureTemplate
    'set-storage CaptureT T
    'set-symbols CaptureT
        __call = func
    spice-quote
        bitcast argtuple CaptureT

# capture name|"name"| (param ...) {var ...} body ...
sugar capture (head body...)
    capture-parser "capture" head body...
        inline (namestr argnames unreflist argvalues params body)
            qq [pack-capture] ([tupleof] (unquote-splice argvalues))
                [fn] [namestr] (this-capture (unquote-splice params))
                    unquote-splice
                        if (empty? argnames) '()
                        else
                            qq (([let] (unquote-splice argnames) =
                                ([unpack-capture] this-capture)))
                    unquote-splice unreflist
                    ;
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
        inline (namestr argnames unreflist argvalues params body)
            let payload = ('unique Symbol "payload")
            let payload-type = ('unique Symbol "payload-type")
            qq [do]
                [let] [payload] [payload-type] =
                    [pack-capture-spice] ([tupleof] (unquote-splice argvalues))
                [finalize-capture-spice] [payload]
                    [spice] [namestr] (this-capture (unquote-splice params))
                        unquote-splice
                            if (empty? argnames) '()
                            else
                                qq (([let] (unquote-splice argnames) =
                                    ([unpack-capture-spice] this-capture [payload-type])))
                        unquote-splice unreflist
                        ;
                        unquote-splice body

#-------------------------------------------------------------------------------

unlet unpack-capture pack-capture unpack-capture-spice pack-capture-spice
    \ finalize-capture-spice

do
    let Capture CaptureTemplate SpiceCapture capture spice-capture
    let decorate-capture = decorate-fn
    locals;
