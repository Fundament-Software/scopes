
inline debugprint (f)
    inline (...)
        print (f ...)

inline multiply (z)
    inline (f)
        inline (x y)
            (f x y) * z

! debugprint
! multiply 2
fn test (x y)
    x + y

assert ((test 1 2) == 6)


let T = (typename "T")

inline replace-result (f)
    ! ast-quote
    inline (cls x)
        print f
        x + 300

# test method decorators
! replace-result
method inline '__typecall T (cls x)
    compiler-error! "should not see me"

! print
2 + 3

! print
let x y z = 3 4 5

run-stage;

assert ((T 3) == 303)
