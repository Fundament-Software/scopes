
using import testing


# testing C ABI

#fn testfunc (x y)
    x * y
vvv bind lib
vvv include
""""// passing small structs by-value
    typedef struct _Vec2 {
        float x,y;
    } Vec2;
    typedef struct _Vec3 {
        float x,y,z;
    } Vec3;
    typedef struct _Vec4 {
        float x,y,z,w;
    } Vec4;
    typedef struct _IVec2 {
        int x,y;
    } IVec2;
    typedef struct _Vec2x2 {
        Vec2 a;
        Vec2 b;
    } Vec2x2;

    #include <stdio.h>

    int testfunc_ivec2_ivec2 (IVec2 a, IVec2 b) {
        printf("a.x = %i, a.y = %i\n", a.x, a.y);
        printf("b.x = %i, b.y = %i\n", b.x, b.y);
        return
            (a.x == 1) && (a.y == 2)
            && (b.x == 3) && (b.y == 4);
    }

    int testfunc_vec3_vec3 (Vec3 a, Vec3 b) {
        printf("a.x = %f, a.y = %f, a.z = %f\n", a.x, a.y, a.z);
        printf("b.x = %f, b.y = %f, b.z = %f\n", b.x, b.y, b.z);
        return
            (a.x == 1) && (a.y == 2) && (a.z == 3)
            && (b.x == 4) && (b.y == 5) && (b.z == 6);
    }

    int testfunc_vec2_vec2_vec4_vec4 (Vec2 a, Vec2 b, Vec4 c, Vec4 d) {
        printf("a.x = %f, a.y = %f\n", a.x, a.y);
        printf("b.x = %f, b.y = %f\n", b.x, b.y);
        printf("c.x = %f, c.y = %f, c.z = %f, c.w = %f\n", c.x, c.y, c.z, c.w);
        printf("d.x = %f, d.y = %f, d.z = %f, d.w = %f\n", d.x, d.y, d.z, d.w);
        return
            (a.x == 1.0f) && (a.y == 2.0f)
            && (b.x == 3.0f) && (b.y == 4.0f)
            && (c.x == 5.0f) && (c.y == 6.0f) && (c.z == 7.0f) && (c.w == 8.0f)
            && (d.x == 9.0f) && (d.y == 10.0f) && (d.z == 11.0f) && (d.w == 12.0f);
    }

    int testfunc_vec2x2(Vec2x2 q) {
        printf("q.a.x = %f, q.a.y = %f\n", q.a.x, q.a.y);
        printf("q.b.x = %f, q.b.y = %f\n", q.b.x, q.b.y);
        return (q.a.x == 1.0f) && (q.a.y == 2.0f)
            && (q.b.x == 3.0f) && (q.b.y == 4.0f);
    }
vvv bind lib
do
    using lib.typedef
    using lib.extern
    locals;

fn testf1 ()
    lib.testfunc_ivec2_ivec2
        lib.IVec2 1 2
        lib.IVec2 3 4

fn testf2 ()
    lib.testfunc_vec2_vec2_vec4_vec4
        lib.Vec2 1.0 2.0
        lib.Vec2 3.0 4.0
        lib.Vec4 5.0 6.0 7.0 8.0
        lib.Vec4 9.0 10.0 11.0 12.0

fn testf3 ()
    lib.testfunc_vec2x2
        lib.Vec2x2
            lib.Vec2 1.0 2.0
            lib.Vec2 3.0 4.0

fn testf4 ()
    lib.testfunc_vec3_vec3
        lib.Vec3 1.0 2.0 3.0
        lib.Vec3 4.0 5.0 6.0

compile
    `[(typify testf4)]
    'dump-module

test (1 == (testf4))
test (1 == (testf3))
test (1 == (testf2))
test (1 == (testf1))

do
    vvv bind cfun
    include
        """"typedef struct Mesh {
                int vertexCount;
                int triangleCount;
                float *vertices;
                float *texcoords;
            } Mesh;
            Mesh cfun (Mesh param) {
                return param;
            }

    let s =
        cfun.typedef.Mesh
            vertices = (malloc-array f32 1000)
            vertexCount = 250
            triangleCount = 303
    let s2 = (cfun.extern.cfun s)
    test (s.vertexCount == s2.vertexCount)
    test (s.triangleCount == s2.triangleCount)
    test (s.vertices == s2.vertices)

do
    let AB =
        do
            using import struct

            struct AB plain
                a : i32
                b : i32
                c : i32

    fn mrv1 ()
        let sptr = (malloc AB)
        sptr.a = 1
        sptr.b = 2
        sptr.c = 3
        # wrong ABI when references are being returned, produces crash
        _ sptr.a sptr.b sptr.c

    fn mrv2 ()
        let sptr = (malloc-array i32 3)
        sptr @ 0 = 1
        sptr @ 1 = 2
        sptr @ 2 = 3
        # wrong ABI when references are being returned, produces crash
        _ (sptr @ 0) (sptr @ 1) (sptr @ 2)

    #compile
        static-typify mrv1
        'dump-function

    let a b c = (mrv1)
    print a b c

    let a b c = (mrv2)
    print a b c


