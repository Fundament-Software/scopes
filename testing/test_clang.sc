
let
    TESTVAL = "-DTESTVAL2"

include
    filter "^t.*$"
    options "-DTESTVAL" TESTVAL
""""#ifndef TESTVAL
        #error "expected define"
    #endif
    #ifndef TESTVAL2
        #error "expected define 2"
    #endif
    int testfunc (int x, int y) {
        return x * y;
    }

assert ((testfunc 2 3) == 6)
