
assert ((hash 0) == (hash 0))
assert ((hash 0) != (hash 1))
assert (((hash 1 2 3 4) as integer as hash) == (hash (hash (hash 1 2) 3) 4))
assert (((hash 1) as integer) == 4116880583171815541:usize)
assert (((hash 2.0) as integer) == 13388820612621794461:usize)

assert ((hash i32) == (hash i32))

