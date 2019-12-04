
using import testing

let
    TESTVAL = "-DTESTVAL2"

vvv bind C
include
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

        #define DOUBLEVAL 1.0
        #define FLOATVAL 1.0f
        #define INTVAL 3
        #define UINTVAL 3u
        #define LONGVAL 3ll
        #define ULONGVAL 0x3ull

        // initialized global
        int test_clang_global = 303;
        // uninitialized global
        int test_clang_global2;

        // eof

using C.extern filter "^(t.*)$"
using C.define filter "^(.*VAL)$"

test ((testfunc 2 3) == 6)

test (test_clang_global == 303)
deref test_clang_global2

static-assert ((returnof testfunc) == i32)

static-assert ((typeof DOUBLEVAL) == f64)
static-assert ((typeof FLOATVAL) == f32)
static-assert ((typeof INTVAL) == i32)
static-assert ((typeof UINTVAL) == u32)
static-assert ((typeof LONGVAL) == i64)
static-assert ((typeof ULONGVAL) == u64)

# bug: forward declaration after definition
vvv include
""""typedef struct X Y;

    struct X {
        int x,y;
    };

    struct X;

# bug: attempting to use incomplete typename $4
vvv include
""""
    typedef struct {
        int x;
        int y;
        long z;
    } U,K;

    typedef struct X {
    K k;
    int x;
    int y;
    } KK;

# issue #54: support for `typeof` qualifier in imported C declarations
vvv bind Test
vvv include
""""#define LE_MACRO (1 << 6)
    typeof(LE_MACRO) get_value() { return LE_MACRO; }

assert ((Test.extern.get_value) == (1 << 6))



