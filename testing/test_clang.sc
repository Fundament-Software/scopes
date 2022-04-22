
using import testing

let
    TESTVAL = "-DTESTVAL2"

vvv bind C
include
    options "-DTESTVAL" TESTVAL #"-c" "clangtest.o"
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
vvv bind Test2
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

print Test2.struct.X

# issue #54: support for `typeof` qualifier in imported C declarations
vvv bind Test
vvv include
""""#define LE_MACRO (1 << 6)
    typeof(LE_MACRO) get_value() { return LE_MACRO; }

assert ((Test.extern.get_value) == (1 << 6))


do
    vvv bind mod1
    vvv include
    """"typedef struct block_s {
            int _0; int _1; int _2;
        } block_t;

    vvv bind mod2
    include
        using mod1
        """"typedef struct block_s {
                int _0; int _1; int _2;
            } block_t;

            void process(block_t block) {}

    print
        mod2.extern.process (mod1.typedef.block_t 1 2 3)


do
    # enums assume typedef name if there is no name
    vvv bind mod
    vvv include
    """"typedef enum {
            A, B, C
        } EnumType;

    test ((tostring mod.typedef.EnumType) == "EnumType")

do
    vvv bind i
    vvv include
    """"#include <stdint.h>
        struct inotify_event {
            int      wd;
            uint32_t mask;
            uint32_t cookie;
            uint32_t len;
            char     name[];
        };
        unsigned long int query_inotify_event_size () {
            return sizeof(struct inotify_event);
        }

    print (storageof i.struct.inotify_event)
    print (sizeof i.struct.inotify_event) (i.extern.query_inotify_event_size)
    test ((sizeof i.struct.inotify_event) == (i.extern.query_inotify_event_size))

do
    # submitted by `Erik McClure#9999` on #scopes-dev
    # https://discord.com/channels/793835483708915752/796056890660225064/967005880937742366

    let module =
        include
            """"
                #include <assert.h>

                struct nkc {
                    int nkcInited;
                    struct nkc *ctx;
                    int keepRunning; // If this is commented out, it crashes on the first call instead of the second

                    struct nkc *window;
                };
                struct nkc_key_event {
                    int type;
                    int code;
                    int mod; // If this is commented out, it stops crashing.
                };
                union nkc_event {
                    int type;
                    struct nkc_key_event key;
                };

                union nkc_event
                nkc_poll_events(struct nkc* handle)
                {
                    assert (handle->nkcInited == 0x23456789);
                    assert (handle->ctx == 0x12345678);
                    assert (handle->keepRunning == 0x3456789A);
                    assert (handle->window == 0x23456789);
                    union nkc_event ne;
                    ne.key.type = 0x12345678;
                    ne.key.code = 0x23456789;
                    ne.key.mod = 0x3456789A;
                    return ne;
                }

            #options "-ggdb"

    let nkc = module.struct.nkc

    fn main ()
        local nkcx = (nkc)
        nkcx.nkcInited = 0x23456789
        nkcx.ctx = (inttoptr 0x12345678 (mutable @nkc));
        nkcx.keepRunning = 0x3456789A
        nkcx.window = (inttoptr 0x23456789 (mutable @nkc));
        inline docall ()
            print "calling..."
            let val = (module.extern.nkc_poll_events &nkcx)
            test (val.key.type == 0x12345678)
            test (val.key.code == 0x23456789)
            test (val.key.mod == 0x3456789A)
        docall;
        docall;
        docall;

    (main)

print "ok"

;