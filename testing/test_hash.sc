
using import testing

test ((hash 0) == (hash 0))
test ((hash 0) != (hash 1))
test (((hash 1 2 3 4) as integer as hash) == (hash (hash (hash 1 2) 3) 4))
test (((hash 1) as integer) == 4116880583171815541:usize)
test (((hash 2.0) as integer) == 13388820612621794461:usize)

test ((hash i32) == (hash i32))

