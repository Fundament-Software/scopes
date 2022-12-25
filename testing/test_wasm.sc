fn sum_two (x y)
    x + y

# generate webassembly
compile-object
    "wasm32-unknown-unknown"
    compiler-file-kind-object
    module-dir .. "/_test.wasm"
    do
        let 
            sum_two = 
                static-typify sum_two i32 i32
            sum_two_float =
                static-typify sum_two f32 f32
        locals;

# link file using llvm's webassembly linker
    wasm-ld --no-entry --export-dynamic _test.wasm -o test.wasm

# serve locally with
    python2 -m SimpleHTTPServer 8000
    or
    python -m http.server 8000

# analysis with tools from
    https://github.com/WebAssembly/wabt

    particularly wasm2wat and wasm-validate