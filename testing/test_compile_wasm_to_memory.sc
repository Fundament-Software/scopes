fn sum (x y)
    x + y

let result = 
    compile-wasm-to-buffer
        "test"
        compiler-file-kind-object
        do
            let 
                func =
                    static-typify sum i32 i32
            locals;
        #

print result