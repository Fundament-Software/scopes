
fn new (T args...)
    let value = (malloc T)
    # try in-place constructor first
    let f ok = (type@ T 'new)
    if ok
        f T value args...
    else
        # init from immutable constructor instead
        store (T args...) value
    value

fn new-array (T size args...)
    let size = (usize size)
    # store a size header
    let offset = (sizeof usize)
    dump offset
    let value =
        malloc-array u8 (add (mul size (sizeof T)) offset)
    store size (bitcast value (pointer usize 'mutable))
    dump value
    let value =
        bitcast (getelementptr value offset) (pointer T 'mutable)
    dump value
    # try in-place array constructor first
    let f ok = (type@ T 'new-array)
    if ok
        f T size value args...
    else
        # init array by element
        # try in-place constructor next
        let f ok = (type@ T 'new)
        let f =
            if ok f
            else
                # fallback to immutable constructor
                let initval = (T args...)
                fn (T value)
                    store initval value
        let loop (i) = (unconst 0:usize)
        if (icmp<u i size)
            f T (getelementptr value i) args...
            loop (add i 1:usize)
    value

fn delete (value)
    free value

let k =
    new-array i32 20 5
#delete k
