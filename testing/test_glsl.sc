
using import glm
using import glsl

fn set-vertex-position ()
    local screen-tri-vertices =
        arrayof vec2
            vec2 -1 -1
            vec2  3 -1
            vec2 -1  3
    let pos = (screen-tri-vertices @ gl_VertexID)
    gl_Position = (vec4 pos.x pos.y 0 1)
    deref pos

inout uv : vec2
    location = 0
uniform phase : f32
    location = 1
uniform smp : sampler2D
    location = 2

let vertex-code =
    do
        fn vertex-shader ()
            let half = (vec2 0.5 0.5)
            let pos = (set-vertex-position)
            uv.out =
                (pos * half) + half
            return;

        let code =
            compile-glsl 'vertex
                typify vertex-shader
                #'O3
                #'dump-module
                #'no-opts
        print code
        code

let fragment-code =
    do
        out out_Color : vec4

        fn make-phase ()
            (sin (phase as immutable)) * 0.5 + 0.5

        fn fragment-shader ()
            let uv = (uv as vec2)
            #let uv = uv.in
            let size = (textureSize smp 0)
            assert ((typeof size) == ivec2)
            let color = (vec4 uv.xy (make-phase) 1)
            out_Color =
                color *
                    texture smp uv
            return;

        'dump
            typify fragment-shader

        let code =
            compile-glsl 'fragment
                typify fragment-shader
                'dump-disassembly
                #'no-opts
        print code
        code
