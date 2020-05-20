
using import testing

test ((hash 0) == (hash 0))
test ((hash 0) != (hash 1))
test (((hash 1 2 3 4) as integer as hash) == (hash (hash (hash 1 2) 3) 4))
test (((hash 1) as integer) == 4116880583171815541:usize)
test (((hash 2.0) as integer) == 13388820612621794461:usize)

test ((hash i32) == (hash i32))

do
    let u128 = (integer 128)
    let u256 = (integer 256)
    # a few prime numbers just below 2^64
    let a b c d =
        0xFFFFFFFFFFFFFFC5:u64
        0xFFFFFFFFFFFFFF7D:u64
        0xFFFFFFFFFFFFFF6B:u64
        0xFFFFFFFFFFFFFE87:u64

    let bigA = (((u128) | a) | (((u128) | b) << 64)) # ba
    test ((hash bigA) == (hash a b))
    let bigB = (((u128) | c) | (((u128) | d) << 64)) # dc
    let bigC = (((u256) | bigA) | (((u256) | bigB) << 128)) # dcba
    test ((hash bigC) == (hash a b c d))
