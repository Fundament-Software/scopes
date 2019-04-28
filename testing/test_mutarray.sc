
using import Array
using import testing

let TESTSIZE = (1:usize << 16:usize)

let i32Arrayx65536 = (Array i32 TESTSIZE)
let i32Arrayx65536_2 = (Array i32 TESTSIZE)
static-assert (i32Arrayx65536 == i32Arrayx65536_2)
static-assert (i32Arrayx65536 < FixedArray)
let i32Array = (Array i32)
let i32Array2 = (Array i32)
static-assert (i32Array == i32Array2)
static-assert (i32Array < GrowingArray)
let i32Arrayx16 = (Array i32 16)
let i32Arrayx32 = (Array i32 32)

let i32ArrayArray = (Array i32Array)
let i32Arrayx16Array = (Array i32Arrayx16)
let i32ArrayArrayx16 = (Array i32Array 16)
let i32Arrayx16Arrayx16 = (Array i32Arrayx16 16)

let StringArray = (Array string)

let fullrange = (range TESTSIZE)

do
    # mutable array with fixed upper capacity
    local a : i32Arrayx65536
    report a
    assert (('capacity a) == TESTSIZE)
    for i in fullrange
        assert ((countof a) == i)
        'append a (i32 i)
    for i in fullrange
        assert ((a @ i) == (i32 i))
    # generator support
    for i k in (enumerate a)
        assert ((a @ i) == i)

inline test-array-of-array (Tx Ty)
    do
        dump "test-array-of-array" Tx Ty
        report "test-array-of-array" Tx Ty
        # array of array
        let i32Array = Tx
        let i32ArrayArray = Ty
        local a : i32ArrayArray
        for x in (range 16)
            let b = ('emplace-append a)
            assert ((countof b) == 0) (repr (countof b))
            for y in (range 16)
                'append b (x * 16 + y)
        assert ((countof a) == 16)
        report a
        for x b in (enumerate a)
            report b
            assert ((countof b) == 16)
            for y n in (enumerate b)
                assert ((x * 16 + y) == n)
    report "done"

test-array-of-array i32Arrayx16 i32Arrayx16Array
test-array-of-array i32Arrayx16 i32Arrayx16Arrayx16
test-array-of-array i32Array i32ArrayArrayx16
test-array-of-array i32Array i32ArrayArray

do
    # mutable array with dynamic capacity
    local a : i32Array
        capacity = 12
    report a
    assert (('capacity a) >= 12)
    for i in fullrange
        assert ((countof a) == i)
        'append a (i32 i)
    assert (('capacity a) >= TESTSIZE)
    for i in fullrange
        assert ((a @ i) == (i32 i))
    # generator support
    for i k in (enumerate a)
        assert ((a @ i) == i)


inline test-sort-array (T)
    dump "testing sorting" T

    let sequence... = 3 1 9 5 0 7 12 3 99 -20
    let sorted-sequence... = -20 0 1 3 3 5 7 9 12 99
    let reverse-sorted-sequence... = 99 12 9 7 5 3 3 1 0 -20

    # sorting a fixed mutable array
    local a : T
    va-lfold none
        inline (key k)
            'append a k
        sequence...

    inline verify-element (i key k)
        assert ((a @ i) == k)

    va-lifold none verify-element sequence...

    'sort a
    va-lifold none verify-element sorted-sequence...

    # custom sorting key
    'sort a (inline (x) (- x))
    va-lifold none verify-element reverse-sorted-sequence...

test-sort-array i32Arrayx32
test-sort-array i32Array

dump "sorting a bunch of values"

do
    let sequence... = "yes" "this" "is" "dog" ""
    let sorted-sequence... = "" "dog" "is" "this" "yes"

    local a : StringArray
    va-lfold none
        inline (key k)
            'append a k
        sequence...
    assert ((countof a) == 5)
    inline verify-element (i key k)
        assert ((a @ i) == k)
    va-lifold none verify-element sequence...
    'sort a
    va-lifold none verify-element sorted-sequence...

dump "sorting big array"
report "big sort"

fn test-sort ()
    local a : i32Array
    let N = 1000000
    for i in (range N)
        'append a
            if ((i % 2) == 0)
                i
            else
                N - i
    report "sorting big array..."
    'sort a
    report "done."
    # verify the array is sorted
    local x = (a @ 0)
    for k in a
        let x1 = k
        assert (x1 >= x)
        x = x1

test-sort;

fn test-one ()
    One.test-refcount-balanced;

    local a : (Array One)
    let N = 1000
    for i in (range N)
        'append a
            if ((i % 2) == 0)
                One i
            else
                One (N - i)
    report "sorting array of ones..."
    'sort a
    report "done."
    # verify the array is sorted
    local x = ('value (a @ 0))
    for k in a
        let x1 = ('value k)
        test (x1 >= x)
        x = x1
    ;

# handling of unique elements
test-one;
One.test-refcount-balanced;
