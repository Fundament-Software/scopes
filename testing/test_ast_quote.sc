
let T = (typename "T")
'set-plain-storage T i32

do
    let this-type = T
    method inline '__typecall (cls x)
        bitcast x T

run-stage;

print (T 5)

