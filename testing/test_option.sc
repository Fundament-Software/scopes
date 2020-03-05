
using import testing
using import Option

# Option unwrap method
do
    let optT = (Option i32)
    let opt = (optT 1234)
    let opt2 = (optT)
    let result =
        try ('try-unwrap opt)
        else 0
    test (result == 1234)
    let result =
        try ('try-unwrap opt2)
        else 12345
    test (result == 12345)


;