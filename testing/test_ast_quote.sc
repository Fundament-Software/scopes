

typedef T : i32

    @@ ast-quote
    inline __typecall (cls x)
        bitcast x this-type

spice hello (x)
    print x
    true

@@ ast-quote; inline test ()
    fn ()
        hello 5
        print T

run-stage;

((test))

print (T 5)
