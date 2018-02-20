
let C =
    import-c "iching.c" "
        #include <stdlib.h>
        #include <time.h>
        "
        '()



C.srand
    u32
        C.time null

fn random ()
    (C.rand) / (i32 C.RAND_MAX)

for i in (range 6)
    let c =
        random;
    if (c < 0.25)
        print "==  =="
    elseif (c < 0.5)
        print "==  == o"
    elseif (c < 0.75)
        print "======"
    else
        print "====== o"
