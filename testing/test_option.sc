
using import testing
using import Option

# Option unwrap method
do
    let optT = (Option One)
    let opt = (optT (One 1234))
    let opt2 = (optT)
    let result =
        try ('try-unwrap opt)
        else
            error "unwrap failed"
    test (result == (One 1234))
    let fallback = (One 12345)
    let result =
        try (deref ('try-unwrap opt2))
        else (view fallback)
    test (result == (One 12345))
    ;

One.test-refcount-balanced;

;