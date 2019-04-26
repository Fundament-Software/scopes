
using import testing

let i32x4 = (array i32 4)

run-stage;

inline test (f T)
    f (nullof T) 1

inline test+ (T) (test (do +) T)
inline test- (T) (test (do -) T)
inline test* (T) (test (do *) T)
inline test/ (T) (test (do /) T)
inline test@ (T) (test (do @) T)

inline Y (func T)
    func T

inline N (func T)
    test-compiler-error
        func T

# feature matrix syntax sugar
features    i8  i16 i32 i64 i32x4
    -----------------------------
    test+   Y   Y   Y   Y   N
    test-   Y   Y   Y   Y   N
    test*   Y   Y   Y   Y   N
    test/   Y   Y   Y   Y   N
    test@   N   N   N   N   Y


