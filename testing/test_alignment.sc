
using import testing

# test alignment
inline test-alignment (T)
    let size alignment =
        ptrtoint (getelementptr (nullof (mutable pointer T)) 1) u64
        ptrtoint (getelementptr (nullof (mutable pointer (tuple bool T))) 0 1) u64
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

va-map
    inline (i)
        i := i + 1
        test-alignment (integer i)
        test-alignment (integer (i + 64))
        test-alignment (integer (i + 64 * 2))
        test-alignment (integer (i + 64 * 3))
    va-range 64
