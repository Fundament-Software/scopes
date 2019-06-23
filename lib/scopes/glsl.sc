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

do
    let srcpostfixes... =
        \ "8" "8i" "8ui" "8Snorm"
        \ "16" "16i" "16ui" "16f" "16Snorm"
        \ "32" "32i" "32ui" "32f"

    let dstpostfixes... = 
        \ "8" "8i" "8ui" "8_snorm"
        \ "16" "16i" "16ui" "16f" "16_snorm"
        \ "32" "32i" "32ui" "32f"

    let srcprefixes... =
        \ "R" "Rg" "Rgba"
    let dstprefixes... =
        \ "r" "rg" "rgba"

    va-map
        inline (i)
            let srcpostfix = (va@ i srcpostfixes...)
            let dstpostfix = (va@ i dstpostfixes...)
            va-map 
                inline (k)
                    let srcprefix = (va@ k srcprefixes...)
                    let dstprefix = (va@ k dstprefixes...)
                    let srcname = (Symbol (.. srcprefix srcpostfix))
                    let dstname = (Symbol (.. dstprefix dstpostfix))
                    'bind scope dstname srcname
                va-range (va-countof srcprefixes...)
        va-range (va-countof srcpostfixes...)

    build-dims
        inline (postfix dim arrayed ms coords)
            let T = (make-gsampler postfix dim arrayed ms coords)
            'bind scope (Symbol ('string T)) T

build-rtypes
    inline (prefix return-type)
        build-dims
            inline (postfix dim arrayed ms coords)
                'bind scope (Symbol (.. prefix "image" postfix))
                    inline (format)
                        Image return-type dim 0 arrayed ms 2 format unnamed
                let samplerT = (make-sampler prefix return-type postfix dim arrayed ms coords)
                'bind scope (Symbol ('string samplerT)) samplerT

inline gen-xvar-sugar (name f)
    fn parse-layout (layout)
        loop (layout outp = layout '())
            sugar-match layout
            case ((key '= value) layout...)
                let kv rest = (decons layout)
                repeat rest
                    cons kv outp
            case ((value as Symbol) layout...)
                let val rest = (decons layout)
                repeat rest
                    cons ('tag `[(qq [val] = true)] ('anchor val)) outp
            case ()
                # order is reversed, but should not matter
                break outp
            default
                error "unrecognized layout attribute pattern"

    sugar "def-xvar" (values...)
        spice local-new (name T layout...)
            let name = (name as Symbol)
            let T = (T as type)
            f ('anchor args) name T layout...
        sugar-match values...
        case ((name as Symbol) ': T layout...)
            let expr = (qq [local-new] '[name] [T]
                (unquote-splice (parse-layout layout...)))
            let expr = ('tag `expr ('anchor expression))
            qq [let] [name] = [expr]
        default
            error
                .. "syntax: " name " <name> [: <type>] [location = i] [binding = j]"

inline wrap-xvar-global (f)
    fn (...)
        `(ptrtoref [(f ...)])

fn config-xvar (flags storage anchor name T layout)
    local location = -1
    local binding = -1
    local flags = flags
    for arg in ('args layout)
        let k v = ('dekey arg)
        switch k
        case 'location
            location = (v as i32)
        case 'binding
            binding = (v as i32)
        case 'readonly
            if (flags & global-flag-non-readable)
                error "value is already tagged writeonly"
            flags = flags | global-flag-non-writable
        case 'writeonly
            if (flags & global-flag-non-writable)
                error "value is already tagged readonly"
            flags = flags | global-flag-non-readable
        case 'coherent
            flags = flags | global-flag-coherent
        case 'restrict
            flags = flags | global-flag-restrict
        default
            error (.. "unsupported key: " (k as string))
    'tag (sc_global_new name T flags storage location binding) anchor

fn config-buffer (anchor name T layout)
    config-xvar global-flag-buffer-block 'Uniform anchor name T layout

fn config-uniform (anchor name T layout)
    let storage =
        if (('storageof T) < tuple) 'Uniform
        else 'UniformConstant
    config-xvar 0:u32 storage anchor name T layout

fn config-inout (anchor name T layout)
    let tname =
        .. "<inout " (name as string) " : " ('string T) ">"
    let f = (wrap-xvar-global config-xvar)
    let LT =
        @@ spice-quote
        typedef (do tname)
            let Type = T
            let in = [(f 0:u32 'Input anchor name T layout)]
            let out = [(f 0:u32 'Output anchor name T layout)]
    `(bitcast LT InOutType)

inline gen-atomic-func (op)
    inline "atomicfn" (mem data)
        let memptr = (& mem)
        let ET = (elementof (typeof memptr))
        atomicrmw op memptr (imply data ET)

inline gen-signed-atomic-func (sop uop)
    inline "atomicsfn" (mem data)
        let memptr = (& mem)
        let ET = (elementof (typeof memptr))
        atomicrmw
            static-if (signed? ET) sop
            else uop
            \ memptr (imply data ET)

let
    FindILsb_u32 = (extern 'GLSL.std.450.FindILsb (function u32 u32))
    FindILsb_uvec2 = (extern 'GLSL.std.450.FindILsb (function uvec2 uvec2))
    FindILsb_uvec3 = (extern 'GLSL.std.450.FindILsb (function uvec3 uvec3))
    FindILsb_uvec4 = (extern 'GLSL.std.450.FindILsb (function uvec4 uvec4))
    FindILsb_i32 = (extern 'GLSL.std.450.FindILsb (function i32 i32))
    FindILsb_ivec2 = (extern 'GLSL.std.450.FindILsb (function ivec2 ivec2))
    FindILsb_ivec3 = (extern 'GLSL.std.450.FindILsb (function ivec3 ivec3))
    FindILsb_ivec4 = (extern 'GLSL.std.450.FindILsb (function ivec4 ivec4))

do
    let gsampler
    let
        in = (gen-xvar-sugar "in" (wrap-xvar-global (inline (...) (config-xvar 0:u32 'Input ...))))
        out = (gen-xvar-sugar "out" (wrap-xvar-global (inline (...) (config-xvar 0:u32 'Output ...))))
        buffer = (gen-xvar-sugar "buffer" (wrap-xvar-global config-buffer))
        uniform = (gen-xvar-sugar "uniform" (wrap-xvar-global config-uniform))
        inout = (gen-xvar-sugar "inout" config-inout)

        # gl_PerVertex
        gl_Position = (ptrtoref (extern 'spirv.Position vec4 (storage-class = 'Output)))
        gl_PointSize = (ptrtoref (extern 'spirv.PointSize f32 (storage-class = 'Output)))
        gl_ClipDistance = (ptrtoref (extern 'spirv.ClipDistance (array f32) (storage-class = 'Output)))

        gl_FragDepth = (ptrtoref (extern 'spirv.FragDepth f32 (storage-class = 'Output)))

        gl_FragCoord = (ptrtoref (extern 'spirv.FragCoord vec4 (storage-class = 'Input)))
        gl_VertexID = (ptrtoref (extern 'spirv.VertexId i32 (storage-class = 'Input)))
        gl_InstanceID = (ptrtoref (extern 'spirv.InstanceId i32 (storage-class = 'Input)))

        gl_NumWorkGroups = (ptrtoref (extern 'spirv.NumWorkgroups uvec3 (storage-class = 'Input)))
        gl_WorkGroupID = (ptrtoref (extern 'spirv.WorkgroupId uvec3 (storage-class = 'Input)))
        gl_LocalInvocationID = (ptrtoref (extern 'spirv.LocalInvocationId uvec3 (storage-class = 'Input)))
        gl_GlobalInvocationID = (ptrtoref (extern 'spirv.GlobalInvocationId uvec3 (storage-class = 'Input)))
        gl_LocalInvocationIndex = (ptrtoref (extern 'spirv.LocalInvocationIndex u32 (storage-class = 'Input)))

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

    let packHalf2x16 = (extern 'GLSL.std.450.PackHalf2x16 (function u32 vec2))
    let unpackHalf2x16 = (extern 'GLSL.std.450.UnpackHalf2x16 (function vec2 u32))

    inline... findLSB
    case (value : u32,)
        FindILsb_u32 value
    case (value : uvec2,)
        FindILsb_uvec2 value
    case (value : uvec3,)
        FindILsb_uvec3 value
    case (value : uvec4,)
        FindILsb_uvec4 value
    case (value : i32,)
        FindILsb_i32 value
    case (value : ivec2,)
        FindILsb_ivec2 value
    case (value : ivec3,)
        FindILsb_ivec3 value
    case (value : ivec4,)
        FindILsb_ivec4 value

    let
        atomicExchange = (gen-atomic-func xchg)
        atomicAdd = (gen-atomic-func add)
        atomicAnd = (gen-atomic-func band)
        atomicOr = (gen-atomic-func bor)
        atomicXor = (gen-atomic-func bxor)
        atomicMin = (gen-signed-atomic-func smin umin)
        atomicMax = (gen-signed-atomic-func smax umax)

    inline atomicCompSwap (mem compare data)
        let memptr = (& mem)
        let ET = (elementof (typeof memptr))
        cmpxchg memptr (imply compare ET) (imply data ET)

    inline barrier ()
        __barrier barrier-kind-control
    inline memoryBarrier ()
        __barrier barrier-kind-memory
    inline groupMemoryBarrier ()
        __barrier barrier-kind-memory-group
    inline memoryBarrierImage ()
        __barrier barrier-kind-memory-image
    inline memoryBarrierBuffer ()
        __barrier barrier-kind-memory-buffer
    inline memoryBarrierShared ()
        __barrier barrier-kind-memory-shared

    scope .. (locals)
