
using import glm
using import glsl

let screen-tri-vertices =
    arrayof vec2
        vec2 -1 -1; vec2  3 -1; vec2 -1  3

run-stage;

inout uv : vec2
    location = 0

fn multiple-return-values ()
    if true
        _ 1 2 3 4
    else
        _ 5 6 7 8

fn set-vertex-position ()
    local screen-tri-vertices = screen-tri-vertices
    let pos = (screen-tri-vertices @ gl_VertexID)
    multiple-return-values;
    gl_Position = (vec4 pos.x pos.y 0 1)
    deref pos

fn vertex-shader ()
    let half = (vec2 0.5 0.5)
    let pos = (set-vertex-position)
    uv.out =
        (pos * half) + half
    return;

print
    compile-glsl 0 'vertex
        typify vertex-shader
        #'O3
        #'dump-module
        #'no-opts

uniform phase : f32
    location = 1
uniform smp : sampler2D
    location = 2

out out_Color : vec4

fn make-phase ()
    (sin phase) * 0.5 + 0.5

fn fragment-shader ()
    let uv = uv.in
    let size = (textureSize smp 0)
    let color = (vec4 uv (make-phase) size.x)
    out_Color = (color * (texture smp uv))
    return;

#'dump
    typify fragment-shader

print
    compile-glsl 0 'fragment
        typify fragment-shader
        #'dump-disassembly
        #'no-opts

# TODO: fix this case
#let default-vs =
    do
        uniform mvp : mat4
        in position : vec4

        fn main ()
            (gl_Position = position * mvp)

        (compile-glsl 330 'vertex (typify main))

;
