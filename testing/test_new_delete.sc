
fn new (T args...)
    let value = ('from-pointer reference (malloc T))
    # try in-place constructor first
    let f ok = (type@ T 'new)
    if ok
        f T value args...
    else
        # init from immutable constructor instead
        store (T args...) value
    value

fn delete (value)
    free value

let k =
    new-array i32 20 5
#delete k
