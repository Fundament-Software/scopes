
fn testfuncT (x y)
    x + y

# generate webassembly
compile-object
    "wasm32-unknown-unknown-wasm"
    compiler-file-kind-object
    module-dir .. "/_test.wasm"
    do
        let testfunc =
            static-typify testfuncT i32 i32
        let testfunc2 =
            static-typify testfuncT f32 f32
        locals;
    #'no-debug-info
    'dump-module

# link file using llvm's webassembly linker
    wasm-ld --no-entry --export-dynamic _test.wasm -o test.wasm

# serve locally with
    python2 -m SimpleHTTPServer 8000
    or
    python -m http.server 8000

# analysis with tools from
    https://github.com/WebAssembly/wabt

    particularly wasm2wat and wasm-validate
