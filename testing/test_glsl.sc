
using import glm
using import glsl

fn set-vertex-position ()
    let screen-tri-vertices =
        local 'copy
            arrayof vec2
                vec2 -1 -1
                vec2  3 -1
                vec2 -1  3
    let pos = (screen-tri-vertices @ gl_VertexID)
    gl_Position = (vec4 pos.x pos.y 0 1)
    deref pos

xvar inout uv : vec2
    location = 0
xvar uniform phase : f32
    location = 1
dump
    pointer-type-storage-class
        element-type (typeof phase) 0
dump
    extern-type-location (typeof phase)
    extern-type-binding (typeof phase)

let vertex-code =
    do
        fn vertex-shader ()
            let half = (vec2 0.5 0.5)
            let pos = (set-vertex-position)
            uv =
                (pos * half) + half
            return;

        let code =
            compile-glsl 'vertex
                typify vertex-shader
                #'dump-disassembly
                #'dump-module
                #'no-opts
        print code
        code

let fragment-code =
    do
        xvar out out_Color : vec4

        fn make-phase ()
            (sin (phase as immutable)) * 0.5 + 0.5

        fn fragment-shader ()
            let uv = (uv as immutable)
            let color = (vec4 uv.xy (make-phase) 1)
            out_Color = color
            return;

        let code =
            compile-glsl 'fragment
                typify fragment-shader
                #'dump-disassembly
                #'no-opts
        print code
        code
