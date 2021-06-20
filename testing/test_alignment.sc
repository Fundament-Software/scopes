
using import testing

# test alignment
inline test-alignment (T verbose?)
    let size alignment =
        ptrtoint (getelementptr (nullof (mutable pointer T)) 1) u64
        ptrtoint (getelementptr (nullof (mutable pointer (tuple bool T))) 0 1) u64
    static-if verbose?
        print T "size =" size "alignment =" alignment
    if (size != (sizeof T))
        print T "size mismatch:" (sizeof T) "!=" size
    if (alignment != (alignof T))
        print T "align mismatch:" (alignof T) "!=" alignment
    test
        == (alignof T) alignment
    test
        == (sizeof T) size

test-alignment (vector f32 3)
test-alignment (vector i8 3)
test-alignment (vector f32 9)
test-alignment (vector i8 11)
test-alignment (vector bool 32) true
test-alignment (vector bool 8) true
let i1 = (integer 1 true)
test-alignment (vector i1 32) true

do
    let K = (alloca (vector bool 8))
    fn unconst (x) x
    let V = (vectorof bool true true false false true false true false)
    store (unconst V) K
    let PK = (bitcast K (mutable pointer u8))
    print (PK @ 0)
    print (PK @ 1)
    print (PK @ 2)
    print (PK @ 3)
    let PK = (bitcast K (mutable pointer u8))
    test ((PK @ 0) == 0b1010011)

va-map
    inline (i)
        i := i + 1
        test-alignment (integer i)
        test-alignment (integer (i + 64))
        test-alignment (integer (i + 64 * 2))
        test-alignment (integer (i + 64 * 3))
    va-range 64

