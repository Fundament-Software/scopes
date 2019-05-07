
fn testfuncT (x y)
    x + y

# generate webassembly
compile-object
    "wasm32-unknown-unknown"
    module-dir .. "/test.wasm"
    do
        let testfunc =
            static-typify testfuncT i32 i32
        let testfunc2 =
            static-typify testfuncT f32 f32
        locals;
    #'no-debug-info
    'dump-module

# serve locally with
    python2 -m SimpleHTTPServer 8000
    or
    python -m http.server 8000

