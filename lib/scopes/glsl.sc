
using import glm

let gl_Position =
    extern 'spirv.Position vec4
        storage = 'Output
let gl_FragCoord =
    extern 'spirv.FragCoord vec4
        storage = 'Input
let gl_VertexID =
    extern 'spirv.VertexId i32
        storage = 'Input
let gl_FragDepth =
    extern 'spirv.FragDepth f32
        storage = 'Output

syntax-extend
    fn build-rtypes (f)
        # prefix    rtype
        f ""        vec4
        f "i"       ivec4
        f "u"       uvec4
    fn build-dims (f)
        # postfix       dim     arrayed ms  coords
        f "1D"          '1D     0       0   1
        f "2D"          '2D     0       0   2
        f "3D"          '3D     0       0   3
        f "Cube"        'Cube   0       0   3
        f "2DRect"      'Rect   0       0   2
        f "1DArray"     '1D     1       0   2
        f "2DArray"     '2D     1       0   3
        f "CubeArray"   'Cube   1       0   4
        f "Buffer"      'Buffer 0       0   1
        f "2DMS"        '2D     0       1   2
        f "2DMSArray"   '2D     1       1   3

    let sampler = (typename-type "sampler")

    fn coord-type (ET coords)
        if (coords == 1) ET
        else
            construct-vec-type ET (usize coords)

    fn make-gsampler (postfix dim arrayed ms coords)
        let T = (typename-type (.. "gsampler" postfix))
        set-typename-super! T sampler
        let icoordT = (coord-type i32 coords)
        let fcoordT = (coord-type f32 coords)
        let fetch-has-lod-arg =
            match dim
                (or '1D '2D '3D)
                    ms == 0
                else false
        set-type-symbol! T 'fetch
            if fetch-has-lod-arg
                fn... fetch
                    (sampler : T, P : icoordT, lod : i32)
                        sample sampler P
            elseif (ms == 1)
                fn... fetch
                    (sampler : T, P : icoordT, sampleid : i32)
                        sample sampler P
            else
                fn... fetch
                    (sampler : T, P : icoordT)
                        sample sampler P
        T

    fn make-sampler (prefix return-type postfix dim arrayed ms coords)
        let T = (typename-type (.. prefix "sampler" postfix))
        set-typename-super! T (make-gsampler postfix dim arrayed ms coords)
        set-typename-storage! T
            SampledImage-type
                Image-type return-type dim 0 arrayed ms 1 'Unknown unnamed
        T

    build-dims
        fn (postfix dim arrayed ms coords)
            let T = (make-gsampler postfix dim arrayed ms coords)
            set-scope-symbol! syntax-scope (Symbol (type-name T)) T

    build-rtypes
        fn (prefix return-type)
            build-dims
                fn (postfix dim arrayed ms coords)
                    set-scope-symbol! syntax-scope
                        Symbol (.. prefix "image" postfix)
                        fn (format)
                            Image-type return-type dim 0 arrayed ms 2 format unnamed
                    let samplerT = (make-sampler prefix return-type postfix dim arrayed ms coords)
                    set-scope-symbol! syntax-scope
                        Symbol (type-name samplerT)
                        samplerT
    syntax-scope

let XVarType = (typename-type "xvar")
set-typename-super! XVarType extern

define-macro xvar
    fn match-storage (storage)
        match storage
            'in 'Input
            'out 'Output
            else
                compiler-error!
                    .. "unsupported storage type: " (repr storage)
    fn xvar-extern (storage name T params...)
        let ET =
            if (storage == 'buffer)
                extern name T
                    storage = 'Uniform
                    'buffer
                    params...
            elseif (storage == 'uniform)
                extern name T
                    storage =
                        do
                            if ((storageof T) <: tuple) 'Uniform
                            else 'UniformConstant
                    params...
            else
                extern name T
                    storage = (match-storage storage)
                    params...
        let ETT = (typeof ET)
        let loc = (extern-type-location ETT)
        let bind = (extern-type-binding ETT)
        let tname =
            .. "<xvar "
                storage as string
                " "
                name as string
                " : "
                type-name T
                if (loc < 0) ""
                else (.. " location=" (string-repr loc))
                if (bind < 0) ""
                else (.. " binding=" (string-repr bind))
                ">"
        let TN = (typename-type tname)
        set-typename-super! TN XVarType
        set-typename-storage! TN ETT
        bitcast ET TN

    fn quote-if-symbol (sxarg)
        if (('typeof (sxarg as Syntax as Any)) == Symbol)
            Any (list quote sxarg)
        else sxarg

    let sxstorage name sxsep type rest = (decons args 4)
    let sep = (sxsep as Syntax as Symbol)

    if (sep != ':)
        syntax-error! sxsep "syntax: (uniform name : type ...)"
    let loop (rest params) = rest (unconst '())
    if (not (empty? rest))
        let sxarg rest = (decons rest)
        loop rest
            cons
                quote-if-symbol sxarg
                params
    list let name '=
        cons xvar-extern (quote-if-symbol sxstorage) (quote-if-symbol name) type params

fn texelFetch (sampler ...)
    'fetch sampler ...

fn imageLoad (image coord)
    Image-read (image as Image) coord

fn imageStore (image coord data)
    Image-write (image as Image) coord data

fn local_size (x y z)
    set-execution-mode! 'LocalSize x y z

let packHalf2x16 = (extern 'glsl.std.450.PackHalf2x16 (function u32 vec2))
let unpackHalf2x16 = (extern 'glsl.std.450.UnpackHalf2x16 (function vec2 u32))

locals;
