#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""glsl
    ====

    The `glsl` module exports bridge symbols that make it possible to define
    and access external variables for shader programs.

using import glm
using import Capture
using import spicetools

typedef InOutType : (storageof type)
    fn __repr (self)
        repr (bitcast self type)

spice vector->vec-type (v)
    let T = ('typeof v)
    if (T < vector)
        let ET = ('element@ T 0)
        let count = ('element-count T)
        `(bitcast v (vec-type ET count))
    else v

run-stage;

#spice inout-get-in (self destT)
    let destT = (destT as type)
    let T = (bitcast (self as InOutType) type)
    let ET = (T.Type as type)
    if ((destT == immutable) or (destT == (T.Type as type)))
        return (getattr T 'in)
    error "unsupported type"

'set-symbols InOutType
    __getattr =
        spice "__getattr" (self name)
            let T = (bitcast (self as InOutType) type)
            let name = (name as Symbol)
            return (getattr T name)
    #__as =
        spice-cast-macro
            fn "__as" (cls destT)
                return `(inline (self) (inout-get-in self destT))
    #__= =
        box-binary-op
            fn "__=" (lhsT rhsT lhs rhs)
                let _lhsT = (bitcast (lhs as InOutType) type)
                if (ptrcmp== (_lhsT.Type as type) rhsT)
                    return `(assign rhs _lhsT.out)
                error "unequal types"

inline build-rtypes (f)
    # prefix    rtype
    f ""        vec4
    f "i"       ivec4
    f "u"       uvec4
inline build-dims (f)
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

typedef gsampler

inline coord-type (ET coords)
    static-if (coords == 1) ET
    else
        vec-type ET coords

inline make-gsampler (postfix dim arrayed ms coords)
    let icoordT = (coord-type i32 coords)
    let fcoordT = (coord-type f32 coords)
    let fcoordProjT = (coord-type f32 (const.add.i32.i32 coords 1))
    let fetch-has-lod-arg =
        switch dim
        pass '1D
        pass '2D
        case '3D
            ms == 0
        default
            false
    typedef (.. "gsampler" postfix) < gsampler
        let T = this-type

        inline... texture
        case (sampler : T, P : fcoordT)
            sample sampler P
        case (sampler : T, P : fcoordT, bias : f32)
            sample sampler P
                Bias = bias

        let texture-size =
            if fetch-has-lod-arg
                inline... texture-size
                case (sampler : T, lod : i32)
                    vector->vec-type
                        Image-query-size sampler
                            Lod = lod
            else
                inline... texture-size
                case (sampler : T,)
                    vector->vec-type
                        Image-query-size sampler

        inline... texture-offset
        case (sampler : T, P : fcoordT, offset : icoordT)
            sample sampler P
                Offset = offset
        case (sampler : T, P : fcoordT, offset : icoordT, bias : f32)
            sample sampler P
                Offset = offset
                Bias = bias

        inline... texture-proj
        case (sampler : T, P : fcoordProjT)
            sample sampler P
                Proj = true
        case (sampler : T, P : fcoordProjT, bias : f32)
            sample sampler P
                Proj = true
                Bias = bias

        inline... texture-gather
        case (sampler : T, P : fcoordT)
            sample sampler P
                Gather = 0
        case (sampler : T, P : fcoordT, comp : i32)
            sample sampler P
                Gather = comp

        inline... texture-lod
        case (sampler : T, P : fcoordT, lod : f32)
            sample sampler P
                Lod = lod

        inline... texture-query-lod
        case (sampler : T, P : fcoordT)
            vector->vec-type
                Image-query-lod sampler P

        let texture-levels = Image-query-levels
        let texture-samples = Image-query-samples

        let fetch =
            if fetch-has-lod-arg
                inline... fetch
                case (sampler : T, P : icoordT, lod : i32)
                    sample sampler P
                        Fetch = true
                        Lod = lod
            elseif (ms == 1)
                inline... fetch
                case (sampler : T, P : icoordT, sampleid : i32)
                    sample sampler P
                        Fetch = true
                        Sample = sampleid
            else
                inline... fetch
                case (sampler : T, P : icoordT)
                    sample sampler P
                        Fetch = true

        inline... fetch-offset
        case (sampler : T, P : icoordT, lod : i32, offset : icoordT)
            sample sampler P
                Fetch = true
                Lod = lod
                Offset = offset


inline make-sampler (prefix return-type postfix dim arrayed ms coords)
    let super =
        make-gsampler postfix dim arrayed ms coords
    let storage =
        SampledImage
            Image return-type dim 0 arrayed ms 1 'Unknown unnamed
    typedef (.. prefix "sampler" postfix) < super : storage

let scope = (Scope)
build-dims
    inline (postfix dim arrayed ms coords)
        let T = (make-gsampler postfix dim arrayed ms coords)
        'set-symbol scope (Symbol ('string T)) T

build-rtypes
    inline (prefix return-type)
        build-dims
            inline (postfix dim arrayed ms coords)
                'set-symbol scope (Symbol (.. prefix "image" postfix))
                    inline (format)
                        Image return-type dim 0 arrayed ms 2 format unnamed
                let samplerT = (make-sampler prefix return-type postfix dim arrayed ms coords)
                'set-symbol scope (Symbol ('string samplerT)) samplerT

inline gen-xvar-sugar (name f)
    sugar "def-xvar" (values...)
        spice local-new (name T layout...)
            let name = (name as Symbol)
            let T = (T as type)
            f name T layout...
        sugar-match values...
        case ((name as Symbol) ': T layout...)
            qq [let] [name] = ([local-new] '[name] [T] (unquote-splice layout...))
        default
            error
                .. "syntax: " name " <name> [: <type>] [location = i] [binding = j]"

inline wrap-xvar-global (f)
    fn (...)
        `(ptrtoref [(f ...)])

fn config-xvar (flags storage name T layout)
    local location = -1
    local binding = -1
    for arg in ('args layout)
        let k v = ('dekey arg)
        switch k
        case 'location
            location = (v as i32)
        case 'binding
            binding = (v as i32)
        default
            error (.. "unsupported key: " (k as string))
    sc_global_new name T flags storage location binding

fn config-buffer (name T layout)
    config-xvar global-flag-buffer-block 'Uniform name T layout

fn config-uniform (name T layout)
    let storage =
        if (('storageof T) < tuple) 'Uniform
        else 'UniformConstant
    config-xvar 0:u32 storage name T layout

fn config-inout (name T layout)
    let tname =
        .. "<inout " name " : " ('string T) ">"
    let f = (wrap-xvar-global config-xvar)
    let LT =
        @@ spice-quote
        typedef (do tname)
            let Type = T
            let in = [(f 0:u32 'Input name T layout)]
            let out = [(f 0:u32 'Output name T layout)]
    `(bitcast LT InOutType)

do
    let
        in = (gen-xvar-sugar "in" (wrap-xvar-global (inline (...) (config-xvar 0:u32 'Input ...))))
        out = (gen-xvar-sugar "out" (wrap-xvar-global (inline (...) (config-xvar 0:u32 'Output ...))))
        buffer = (gen-xvar-sugar "buffer" (wrap-xvar-global config-buffer))
        uniform = (gen-xvar-sugar "uniform" (wrap-xvar-global config-uniform))
        inout = (gen-xvar-sugar "inout" config-inout)

        gl_Position = `(ptrtoref [(extern 'spirv.Position vec4 (storage-class = 'Output))])
        gl_FragCoord = `(ptrtoref [(extern 'spirv.FragCoord vec4 (storage-class = 'Input))])
        gl_VertexID = `(ptrtoref [(extern 'spirv.VertexId i32 (storage-class = 'Input))])
        gl_FragDepth = `(ptrtoref [(extern 'spirv.FragDepth f32 (storage-class = 'Output))])

    inline texelFetch (sampler P ...)
        'fetch (sampler as gsampler) P ...

    inline texelFetchOffset (sampler P lod offset)
        'fetch-offset (sampler as gsampler) P lod offset

    inline texture (sampler P ...)
        'texture (sampler as gsampler) P ...

    inline textureProj (sampler P ...)
        'texture-proj (sampler as gsampler) P ...

    inline textureLod (sampler P lod)
        'texture-lod (sampler as gsampler) P lod

    inline textureOffset (sampler P offset ...)
        'texture-offset (sampler as gsampler) P offset ...

    inline textureGather (sampler P ...)
        'texture-gather (sampler as gsampler) P ...

    inline textureSize (sampler ...)
        'texture-size (sampler as gsampler) ...

    inline textureQueryLod (sampler P)
        'texture-query-lod (sampler as gsampler) P

    inline textureQueryLevels (sampler)
        'texture-query-levels (sampler as gsampler)

    inline textureSamples (sampler)
        'texture-samples (sampler as gsampler)

    inline imageLoad (image coord)
        Image-read (image as Image) coord

    inline imageStore (image coord data)
        Image-write (image as Image) coord data

    inline local_size (x y z)
        set-execution-mode! 'LocalSize x y z

    let packHalf2x16 = (extern 'glsl.std.450.PackHalf2x16 (function u32 vec2))
    let unpackHalf2x16 = (extern 'glsl.std.450.UnpackHalf2x16 (function vec2 u32))

    scope .. (locals)
