
#-------------------------------------------------------------------------------
# runtime closures
#-------------------------------------------------------------------------------

typedef Capture
typedef MutableCapture < Capture

run-stage;

#'set-symbols MutableCapture
    __copy =
        fn (self other)
            (type@& tuple '__copy) self other

spice unpack-capture (capture)
    let T = ('storageof ('typeof capture))
    `(unpack (bitcast capture T))

spice pack-capture (argtuple func)
    let T = ('typeof argtuple)
    let CaptureT =
        typedef (.. "Capture" ('string T)) < Capture : T
    'set-symbol CaptureT '__call func
    `(bitcast argtuple CaptureT)

sugar capture (('square-list args...) (params...) body...)
    qq [pack-capture] ([tupleof] (unquote-splice args...))
        [fn] (self (unquote-splice params...))
            [let] (unquote-splice args...) =
                [unpack-capture] self
            unquote-splice body...

locals;
