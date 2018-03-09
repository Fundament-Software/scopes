
# testing C ABI

#fn testfunc (x y)
    x * y
let lib =
    import-c "lib.c"
        """
            // passing small structs by-value
            typedef struct {
                float x,y;
            } Vec2;
            typedef struct {
                float x,y,z,w;
            } Vec4;

            #include <stdio.h>

            int testfunc (Vec2 a, Vec2 b, Vec4 c, Vec4 d) {
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
        '()

fn testf ()
    lib.testfunc
        lib.Vec2 1.0 2.0
        lib.Vec2 3.0 4.0
        lib.Vec4 5.0 6.0 7.0 8.0
        lib.Vec4 9.0 10.0 11.0 12.0

compile
    typify testf
    'dump-function

assert (1 == (testf))
