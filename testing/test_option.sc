
using import testing
using import Option

let optT = (Option One)

# Option unwrap method
do
    let opt = (optT (One 1234))
    test ((typeof opt) < Option)
    let opt2 = (optT)
    let result =
        try ('unwrap opt)
        else
            error "unwrap failed"
    test (result == (One 1234))
    let fallback = (One 12345)
    let result =
        try (deref ('unwrap opt2))
        else fallback
    test (result == (One 12345))
    ;

do
    # wrapping
    let opt = (Option.wrap (One 1234))
    test ((typeof opt) == optT)
    ;

# Option dispatch method
do
    # implicit cast to generic class
    let opt = (imply (One 1234) Option)
    test ((typeof opt) == optT)
    let result =
        dispatch opt
        case Some (val) val
        default
            error "unwrap failed"
    test (result == (One 1234))
    ;

One.test-refcount-balanced;

;