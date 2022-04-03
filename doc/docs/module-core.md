<style type="text/css" rel="stylesheet">body { counter-reset: chapter 6; }</style>

globals
=======

These names are bound in every fresh module and main program by default.
Essential symbols are created by the compiler, and subsequent utility
functions, macros and types are defined and documented in `core.sc`.

The core module implements the remaining standard functions and macros,
parses the command-line and optionally enters the REPL.

*define*{.property} `backslash-char`{.descname} [](#scopes.define.backslash-char "Permalink to this definition"){.headerlink} {#scopes.define.backslash-char}

:   A constant of type `i8`.

*define*{.property} `barrier-kind-control`{.descname} [](#scopes.define.barrier-kind-control "Permalink to this definition"){.headerlink} {#scopes.define.barrier-kind-control}

:   A constant of type `i32`.

*define*{.property} `barrier-kind-memory`{.descname} [](#scopes.define.barrier-kind-memory "Permalink to this definition"){.headerlink} {#scopes.define.barrier-kind-memory}

:   A constant of type `i32`.

*define*{.property} `barrier-kind-memory-buffer`{.descname} [](#scopes.define.barrier-kind-memory-buffer "Permalink to this definition"){.headerlink} {#scopes.define.barrier-kind-memory-buffer}

:   A constant of type `i32`.

*define*{.property} `barrier-kind-memory-group`{.descname} [](#scopes.define.barrier-kind-memory-group "Permalink to this definition"){.headerlink} {#scopes.define.barrier-kind-memory-group}

:   A constant of type `i32`.

*define*{.property} `barrier-kind-memory-image`{.descname} [](#scopes.define.barrier-kind-memory-image "Permalink to this definition"){.headerlink} {#scopes.define.barrier-kind-memory-image}

:   A constant of type `i32`.

*define*{.property} `barrier-kind-memory-shared`{.descname} [](#scopes.define.barrier-kind-memory-shared "Permalink to this definition"){.headerlink} {#scopes.define.barrier-kind-memory-shared}

:   A constant of type `i32`.

*define*{.property} `cache-dir`{.descname} [](#scopes.define.cache-dir "Permalink to this definition"){.headerlink} {#scopes.define.cache-dir}

:   A constant of type `string`.

*define*{.property} `compile-flag-O0`{.descname} [](#scopes.define.compile-flag-O0 "Permalink to this definition"){.headerlink} {#scopes.define.compile-flag-O0}

:   A constant of type `u64`.

*define*{.property} `compile-flag-O1`{.descname} [](#scopes.define.compile-flag-O1 "Permalink to this definition"){.headerlink} {#scopes.define.compile-flag-O1}

:   A constant of type `u64`.

*define*{.property} `compile-flag-O2`{.descname} [](#scopes.define.compile-flag-O2 "Permalink to this definition"){.headerlink} {#scopes.define.compile-flag-O2}

:   A constant of type `u64`.

*define*{.property} `compile-flag-O3`{.descname} [](#scopes.define.compile-flag-O3 "Permalink to this definition"){.headerlink} {#scopes.define.compile-flag-O3}

:   A constant of type `u64`.

*define*{.property} `compile-flag-cache`{.descname} [](#scopes.define.compile-flag-cache "Permalink to this definition"){.headerlink} {#scopes.define.compile-flag-cache}

:   A constant of type `u64`.

*define*{.property} `compile-flag-dump-disassembly`{.descname} [](#scopes.define.compile-flag-dump-disassembly "Permalink to this definition"){.headerlink} {#scopes.define.compile-flag-dump-disassembly}

:   A constant of type `u64`.

*define*{.property} `compile-flag-dump-function`{.descname} [](#scopes.define.compile-flag-dump-function "Permalink to this definition"){.headerlink} {#scopes.define.compile-flag-dump-function}

:   A constant of type `u64`.

*define*{.property} `compile-flag-dump-module`{.descname} [](#scopes.define.compile-flag-dump-module "Permalink to this definition"){.headerlink} {#scopes.define.compile-flag-dump-module}

:   A constant of type `u64`.

*define*{.property} `compile-flag-dump-time`{.descname} [](#scopes.define.compile-flag-dump-time "Permalink to this definition"){.headerlink} {#scopes.define.compile-flag-dump-time}

:   A constant of type `u64`.

*define*{.property} `compile-flag-module`{.descname} [](#scopes.define.compile-flag-module "Permalink to this definition"){.headerlink} {#scopes.define.compile-flag-module}

:   A constant of type `u64`.

*define*{.property} `compile-flag-no-debug-info`{.descname} [](#scopes.define.compile-flag-no-debug-info "Permalink to this definition"){.headerlink} {#scopes.define.compile-flag-no-debug-info}

:   A constant of type `u64`.

*define*{.property} `compiler-dir`{.descname} [](#scopes.define.compiler-dir "Permalink to this definition"){.headerlink} {#scopes.define.compiler-dir}

:   A string containing the folder path to the compiler environment. Typically
    the compiler environment is the folder that contains the `bin` folder
    containing the compiler executable.

*define*{.property} `compiler-file-kind-asm`{.descname} [](#scopes.define.compiler-file-kind-asm "Permalink to this definition"){.headerlink} {#scopes.define.compiler-file-kind-asm}

:   A constant of type `i32`.

*define*{.property} `compiler-file-kind-bc`{.descname} [](#scopes.define.compiler-file-kind-bc "Permalink to this definition"){.headerlink} {#scopes.define.compiler-file-kind-bc}

:   A constant of type `i32`.

*define*{.property} `compiler-file-kind-llvm`{.descname} [](#scopes.define.compiler-file-kind-llvm "Permalink to this definition"){.headerlink} {#scopes.define.compiler-file-kind-llvm}

:   A constant of type `i32`.

*define*{.property} `compiler-file-kind-object`{.descname} [](#scopes.define.compiler-file-kind-object "Permalink to this definition"){.headerlink} {#scopes.define.compiler-file-kind-object}

:   A constant of type `i32`.

*define*{.property} `compiler-path`{.descname} [](#scopes.define.compiler-path "Permalink to this definition"){.headerlink} {#scopes.define.compiler-path}

:   A string constant containing the file path to the compiler executable.

*define*{.property} `compiler-timestamp`{.descname} [](#scopes.define.compiler-timestamp "Permalink to this definition"){.headerlink} {#scopes.define.compiler-timestamp}

:   A string constant indicating the time and date the compiler was built.

*define*{.property} `debug-build?`{.descname} [](#scopes.define.debug-build? "Permalink to this definition"){.headerlink} {#scopes.define.debug-build?}

:   A boolean constant indicating if the compiler was built in debug mode.

*define*{.property} `default-target-triple`{.descname} [](#scopes.define.default-target-triple "Permalink to this definition"){.headerlink} {#scopes.define.default-target-triple}

:   A constant of type `string`.

*define*{.property} `e`{.descname} [](#scopes.define.e "Permalink to this definition"){.headerlink} {#scopes.define.e}

:   Euler's number, also known as Napier's constant. Explicitly type-annotated
    versions of the constant are available as `e:f32` and `e:f64`

*define*{.property} `e:f32`{.descname} [](#scopes.define.e:f32 "Permalink to this definition"){.headerlink} {#scopes.define.e:f32}

:   See `e`.

*define*{.property} `e:f64`{.descname} [](#scopes.define.e:f64 "Permalink to this definition"){.headerlink} {#scopes.define.e:f64}

:   See `e`.

*define*{.property} `false`{.descname} [](#scopes.define.false "Permalink to this definition"){.headerlink} {#scopes.define.false}

:   A constant of type `bool`.

*define*{.property} `global-flag-block`{.descname} [](#scopes.define.global-flag-block "Permalink to this definition"){.headerlink} {#scopes.define.global-flag-block}

:   A constant of type `u32`.

*define*{.property} `global-flag-buffer-block`{.descname} [](#scopes.define.global-flag-buffer-block "Permalink to this definition"){.headerlink} {#scopes.define.global-flag-buffer-block}

:   A constant of type `u32`.

*define*{.property} `global-flag-coherent`{.descname} [](#scopes.define.global-flag-coherent "Permalink to this definition"){.headerlink} {#scopes.define.global-flag-coherent}

:   A constant of type `u32`.

*define*{.property} `global-flag-flat`{.descname} [](#scopes.define.global-flag-flat "Permalink to this definition"){.headerlink} {#scopes.define.global-flag-flat}

:   A constant of type `u32`.

*define*{.property} `global-flag-non-readable`{.descname} [](#scopes.define.global-flag-non-readable "Permalink to this definition"){.headerlink} {#scopes.define.global-flag-non-readable}

:   A constant of type `u32`.

*define*{.property} `global-flag-non-writable`{.descname} [](#scopes.define.global-flag-non-writable "Permalink to this definition"){.headerlink} {#scopes.define.global-flag-non-writable}

:   A constant of type `u32`.

*define*{.property} `global-flag-restrict`{.descname} [](#scopes.define.global-flag-restrict "Permalink to this definition"){.headerlink} {#scopes.define.global-flag-restrict}

:   A constant of type `u32`.

*define*{.property} `global-flag-thread-local`{.descname} [](#scopes.define.global-flag-thread-local "Permalink to this definition"){.headerlink} {#scopes.define.global-flag-thread-local}

:   A constant of type `u32`.

*define*{.property} `global-flag-volatile`{.descname} [](#scopes.define.global-flag-volatile "Permalink to this definition"){.headerlink} {#scopes.define.global-flag-volatile}

:   A constant of type `u32`.

*define*{.property} `list-handler-symbol`{.descname} [](#scopes.define.list-handler-symbol "Permalink to this definition"){.headerlink} {#scopes.define.list-handler-symbol}

:   A constant of type `Symbol`.

*define*{.property} `none`{.descname} [](#scopes.define.none "Permalink to this definition"){.headerlink} {#scopes.define.none}

:   A constant of type `Nothing`.

*define*{.property} `null`{.descname} [](#scopes.define.null "Permalink to this definition"){.headerlink} {#scopes.define.null}

:   A constant of type `NullType`.

*define*{.property} `operating-system`{.descname} [](#scopes.define.operating-system "Permalink to this definition"){.headerlink} {#scopes.define.operating-system}

:   A string constant indicating the operating system the compiler was built
    for. It is equal to `"linux"` for Linux builds, `"windows"` for Windows
    builds, `"macos"` for macOS builds and `"unknown"` otherwise.

*define*{.property} `pi`{.descname} [](#scopes.define.pi "Permalink to this definition"){.headerlink} {#scopes.define.pi}

:   The number π, the ratio of a circle's circumference C to its diameter d.
    Explicitly type-annotated versions of the constant are available as `pi:f32`
    and `pi:f64`.

*define*{.property} `pi:f32`{.descname} [](#scopes.define.pi:f32 "Permalink to this definition"){.headerlink} {#scopes.define.pi:f32}

:   See `pi`.

*define*{.property} `pi:f64`{.descname} [](#scopes.define.pi:f64 "Permalink to this definition"){.headerlink} {#scopes.define.pi:f64}

:   See `pi`.

*define*{.property} `pointer-flag-non-readable`{.descname} [](#scopes.define.pointer-flag-non-readable "Permalink to this definition"){.headerlink} {#scopes.define.pointer-flag-non-readable}

:   A constant of type `u64`.

*define*{.property} `pointer-flag-non-writable`{.descname} [](#scopes.define.pointer-flag-non-writable "Permalink to this definition"){.headerlink} {#scopes.define.pointer-flag-non-writable}

:   A constant of type `u64`.

*define*{.property} `project-dir`{.descname} [](#scopes.define.project-dir "Permalink to this definition"){.headerlink} {#scopes.define.project-dir}

:   A constant of type `string`.

*define*{.property} `question-mark-char`{.descname} [](#scopes.define.question-mark-char "Permalink to this definition"){.headerlink} {#scopes.define.question-mark-char}

:   A constant of type `i8`.

*define*{.property} `slash-char`{.descname} [](#scopes.define.slash-char "Permalink to this definition"){.headerlink} {#scopes.define.slash-char}

:   A constant of type `i8`.

*define*{.property} `style-comment`{.descname} [](#scopes.define.style-comment "Permalink to this definition"){.headerlink} {#scopes.define.style-comment}

:   A constant of type `Symbol`.

*define*{.property} `style-error`{.descname} [](#scopes.define.style-error "Permalink to this definition"){.headerlink} {#scopes.define.style-error}

:   A constant of type `Symbol`.

*define*{.property} `style-function`{.descname} [](#scopes.define.style-function "Permalink to this definition"){.headerlink} {#scopes.define.style-function}

:   A constant of type `Symbol`.

*define*{.property} `style-instruction`{.descname} [](#scopes.define.style-instruction "Permalink to this definition"){.headerlink} {#scopes.define.style-instruction}

:   A constant of type `Symbol`.

*define*{.property} `style-keyword`{.descname} [](#scopes.define.style-keyword "Permalink to this definition"){.headerlink} {#scopes.define.style-keyword}

:   A constant of type `Symbol`.

*define*{.property} `style-location`{.descname} [](#scopes.define.style-location "Permalink to this definition"){.headerlink} {#scopes.define.style-location}

:   A constant of type `Symbol`.

*define*{.property} `style-none`{.descname} [](#scopes.define.style-none "Permalink to this definition"){.headerlink} {#scopes.define.style-none}

:   A constant of type `Symbol`.

*define*{.property} `style-number`{.descname} [](#scopes.define.style-number "Permalink to this definition"){.headerlink} {#scopes.define.style-number}

:   A constant of type `Symbol`.

*define*{.property} `style-operator`{.descname} [](#scopes.define.style-operator "Permalink to this definition"){.headerlink} {#scopes.define.style-operator}

:   A constant of type `Symbol`.

*define*{.property} `style-sfxfunction`{.descname} [](#scopes.define.style-sfxfunction "Permalink to this definition"){.headerlink} {#scopes.define.style-sfxfunction}

:   A constant of type `Symbol`.

*define*{.property} `style-string`{.descname} [](#scopes.define.style-string "Permalink to this definition"){.headerlink} {#scopes.define.style-string}

:   A constant of type `Symbol`.

*define*{.property} `style-symbol`{.descname} [](#scopes.define.style-symbol "Permalink to this definition"){.headerlink} {#scopes.define.style-symbol}

:   A constant of type `Symbol`.

*define*{.property} `style-type`{.descname} [](#scopes.define.style-type "Permalink to this definition"){.headerlink} {#scopes.define.style-type}

:   A constant of type `Symbol`.

*define*{.property} `style-warning`{.descname} [](#scopes.define.style-warning "Permalink to this definition"){.headerlink} {#scopes.define.style-warning}

:   A constant of type `Symbol`.

*define*{.property} `symbol-handler-symbol`{.descname} [](#scopes.define.symbol-handler-symbol "Permalink to this definition"){.headerlink} {#scopes.define.symbol-handler-symbol}

:   A constant of type `Symbol`.

*define*{.property} `tau`{.descname} [](#scopes.define.tau "Permalink to this definition"){.headerlink} {#scopes.define.tau}

:   The number τ, the ratio of a circle's circumference C to its radius r.
    Explicitly type-annotated versions of the constant are available as `tau:f32`
    and `tau:f64`.

*define*{.property} `tau:f32`{.descname} [](#scopes.define.tau:f32 "Permalink to this definition"){.headerlink} {#scopes.define.tau:f32}

:   See `tau`.

*define*{.property} `tau:f64`{.descname} [](#scopes.define.tau:f64 "Permalink to this definition"){.headerlink} {#scopes.define.tau:f64}

:   See `tau`.

*define*{.property} `true`{.descname} [](#scopes.define.true "Permalink to this definition"){.headerlink} {#scopes.define.true}

:   A constant of type `bool`.

*define*{.property} `type-kind-arguments`{.descname} [](#scopes.define.type-kind-arguments "Permalink to this definition"){.headerlink} {#scopes.define.type-kind-arguments}

:   A constant of type `i32`.

*define*{.property} `type-kind-array`{.descname} [](#scopes.define.type-kind-array "Permalink to this definition"){.headerlink} {#scopes.define.type-kind-array}

:   A constant of type `i32`.

*define*{.property} `type-kind-function`{.descname} [](#scopes.define.type-kind-function "Permalink to this definition"){.headerlink} {#scopes.define.type-kind-function}

:   A constant of type `i32`.

*define*{.property} `type-kind-image`{.descname} [](#scopes.define.type-kind-image "Permalink to this definition"){.headerlink} {#scopes.define.type-kind-image}

:   A constant of type `i32`.

*define*{.property} `type-kind-integer`{.descname} [](#scopes.define.type-kind-integer "Permalink to this definition"){.headerlink} {#scopes.define.type-kind-integer}

:   A constant of type `i32`.

*define*{.property} `type-kind-matrix`{.descname} [](#scopes.define.type-kind-matrix "Permalink to this definition"){.headerlink} {#scopes.define.type-kind-matrix}

:   A constant of type `i32`.

*define*{.property} `type-kind-pointer`{.descname} [](#scopes.define.type-kind-pointer "Permalink to this definition"){.headerlink} {#scopes.define.type-kind-pointer}

:   A constant of type `i32`.

*define*{.property} `type-kind-qualify`{.descname} [](#scopes.define.type-kind-qualify "Permalink to this definition"){.headerlink} {#scopes.define.type-kind-qualify}

:   A constant of type `i32`.

*define*{.property} `type-kind-real`{.descname} [](#scopes.define.type-kind-real "Permalink to this definition"){.headerlink} {#scopes.define.type-kind-real}

:   A constant of type `i32`.

*define*{.property} `type-kind-sampled-image`{.descname} [](#scopes.define.type-kind-sampled-image "Permalink to this definition"){.headerlink} {#scopes.define.type-kind-sampled-image}

:   A constant of type `i32`.

*define*{.property} `type-kind-sampler`{.descname} [](#scopes.define.type-kind-sampler "Permalink to this definition"){.headerlink} {#scopes.define.type-kind-sampler}

:   A constant of type `i32`.

*define*{.property} `type-kind-tuple`{.descname} [](#scopes.define.type-kind-tuple "Permalink to this definition"){.headerlink} {#scopes.define.type-kind-tuple}

:   A constant of type `i32`.

*define*{.property} `type-kind-typename`{.descname} [](#scopes.define.type-kind-typename "Permalink to this definition"){.headerlink} {#scopes.define.type-kind-typename}

:   A constant of type `i32`.

*define*{.property} `type-kind-vector`{.descname} [](#scopes.define.type-kind-vector "Permalink to this definition"){.headerlink} {#scopes.define.type-kind-vector}

:   A constant of type `i32`.

*define*{.property} `typed-symbol-handler-symbol`{.descname} [](#scopes.define.typed-symbol-handler-symbol "Permalink to this definition"){.headerlink} {#scopes.define.typed-symbol-handler-symbol}

:   A constant of type `Symbol`.

*define*{.property} `typename-flag-plain`{.descname} [](#scopes.define.typename-flag-plain "Permalink to this definition"){.headerlink} {#scopes.define.typename-flag-plain}

:   A constant of type `u32`.

*define*{.property} `unknown-anchor`{.descname} [](#scopes.define.unknown-anchor "Permalink to this definition"){.headerlink} {#scopes.define.unknown-anchor}

:   A constant of type `Anchor`.

*define*{.property} `unnamed`{.descname} [](#scopes.define.unnamed "Permalink to this definition"){.headerlink} {#scopes.define.unnamed}

:   A constant of type `Symbol`.

*define*{.property} `unroll-limit`{.descname} [](#scopes.define.unroll-limit "Permalink to this definition"){.headerlink} {#scopes.define.unroll-limit}

:   A constant of type `i32` indicating the maximum number of recursions
    permitted for an inline. When this number is exceeded, an error is raised
    during typechecking. Currently, the limit is set at 64 recursions. This
    restriction has been put in place to prevent the compiler from overflowing
    its stack memory.

*define*{.property} `value-kind-alloca`{.descname} [](#scopes.define.value-kind-alloca "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-alloca}

:   A constant of type `i32`.

*define*{.property} `value-kind-annotate`{.descname} [](#scopes.define.value-kind-annotate "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-annotate}

:   A constant of type `i32`.

*define*{.property} `value-kind-argument-list`{.descname} [](#scopes.define.value-kind-argument-list "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-argument-list}

:   A constant of type `i32`.

*define*{.property} `value-kind-argument-list-template`{.descname} [](#scopes.define.value-kind-argument-list-template "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-argument-list-template}

:   A constant of type `i32`.

*define*{.property} `value-kind-atomicrmw`{.descname} [](#scopes.define.value-kind-atomicrmw "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-atomicrmw}

:   A constant of type `i32`.

*define*{.property} `value-kind-barrier`{.descname} [](#scopes.define.value-kind-barrier "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-barrier}

:   A constant of type `i32`.

*define*{.property} `value-kind-binop`{.descname} [](#scopes.define.value-kind-binop "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-binop}

:   A constant of type `i32`.

*define*{.property} `value-kind-call`{.descname} [](#scopes.define.value-kind-call "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-call}

:   A constant of type `i32`.

*define*{.property} `value-kind-call-template`{.descname} [](#scopes.define.value-kind-call-template "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-call-template}

:   A constant of type `i32`.

*define*{.property} `value-kind-case-template`{.descname} [](#scopes.define.value-kind-case-template "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-case-template}

:   A constant of type `i32`.

*define*{.property} `value-kind-cast`{.descname} [](#scopes.define.value-kind-cast "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-cast}

:   A constant of type `i32`.

*define*{.property} `value-kind-cmpxchg`{.descname} [](#scopes.define.value-kind-cmpxchg "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-cmpxchg}

:   A constant of type `i32`.

*define*{.property} `value-kind-compile-stage`{.descname} [](#scopes.define.value-kind-compile-stage "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-compile-stage}

:   A constant of type `i32`.

*define*{.property} `value-kind-cond-template`{.descname} [](#scopes.define.value-kind-cond-template "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-cond-template}

:   A constant of type `i32`.

*define*{.property} `value-kind-condbr`{.descname} [](#scopes.define.value-kind-condbr "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-condbr}

:   A constant of type `i32`.

*define*{.property} `value-kind-const-aggregate`{.descname} [](#scopes.define.value-kind-const-aggregate "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-const-aggregate}

:   A constant of type `i32`.

*define*{.property} `value-kind-const-int`{.descname} [](#scopes.define.value-kind-const-int "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-const-int}

:   A constant of type `i32`.

*define*{.property} `value-kind-const-pointer`{.descname} [](#scopes.define.value-kind-const-pointer "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-const-pointer}

:   A constant of type `i32`.

*define*{.property} `value-kind-const-real`{.descname} [](#scopes.define.value-kind-const-real "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-const-real}

:   A constant of type `i32`.

*define*{.property} `value-kind-discard`{.descname} [](#scopes.define.value-kind-discard "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-discard}

:   A constant of type `i32`.

*define*{.property} `value-kind-exception`{.descname} [](#scopes.define.value-kind-exception "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-exception}

:   A constant of type `i32`.

*define*{.property} `value-kind-execution-mode`{.descname} [](#scopes.define.value-kind-execution-mode "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-execution-mode}

:   A constant of type `i32`.

*define*{.property} `value-kind-expression`{.descname} [](#scopes.define.value-kind-expression "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-expression}

:   A constant of type `i32`.

*define*{.property} `value-kind-extract-argument`{.descname} [](#scopes.define.value-kind-extract-argument "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-extract-argument}

:   A constant of type `i32`.

*define*{.property} `value-kind-extract-argument-template`{.descname} [](#scopes.define.value-kind-extract-argument-template "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-extract-argument-template}

:   A constant of type `i32`.

*define*{.property} `value-kind-extract-element`{.descname} [](#scopes.define.value-kind-extract-element "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-extract-element}

:   A constant of type `i32`.

*define*{.property} `value-kind-extract-value`{.descname} [](#scopes.define.value-kind-extract-value "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-extract-value}

:   A constant of type `i32`.

*define*{.property} `value-kind-fcmp`{.descname} [](#scopes.define.value-kind-fcmp "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-fcmp}

:   A constant of type `i32`.

*define*{.property} `value-kind-free`{.descname} [](#scopes.define.value-kind-free "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-free}

:   A constant of type `i32`.

*define*{.property} `value-kind-function`{.descname} [](#scopes.define.value-kind-function "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-function}

:   A constant of type `i32`.

*define*{.property} `value-kind-get-element-ptr`{.descname} [](#scopes.define.value-kind-get-element-ptr "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-get-element-ptr}

:   A constant of type `i32`.

*define*{.property} `value-kind-global`{.descname} [](#scopes.define.value-kind-global "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-global}

:   A constant of type `i32`.

*define*{.property} `value-kind-global-string`{.descname} [](#scopes.define.value-kind-global-string "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-global-string}

:   A constant of type `i32`.

*define*{.property} `value-kind-icmp`{.descname} [](#scopes.define.value-kind-icmp "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-icmp}

:   A constant of type `i32`.

*define*{.property} `value-kind-image-query-levels`{.descname} [](#scopes.define.value-kind-image-query-levels "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-image-query-levels}

:   A constant of type `i32`.

*define*{.property} `value-kind-image-query-lod`{.descname} [](#scopes.define.value-kind-image-query-lod "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-image-query-lod}

:   A constant of type `i32`.

*define*{.property} `value-kind-image-query-samples`{.descname} [](#scopes.define.value-kind-image-query-samples "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-image-query-samples}

:   A constant of type `i32`.

*define*{.property} `value-kind-image-query-size`{.descname} [](#scopes.define.value-kind-image-query-size "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-image-query-size}

:   A constant of type `i32`.

*define*{.property} `value-kind-image-read`{.descname} [](#scopes.define.value-kind-image-read "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-image-read}

:   A constant of type `i32`.

*define*{.property} `value-kind-image-write`{.descname} [](#scopes.define.value-kind-image-write "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-image-write}

:   A constant of type `i32`.

*define*{.property} `value-kind-insert-element`{.descname} [](#scopes.define.value-kind-insert-element "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-insert-element}

:   A constant of type `i32`.

*define*{.property} `value-kind-insert-value`{.descname} [](#scopes.define.value-kind-insert-value "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-insert-value}

:   A constant of type `i32`.

*define*{.property} `value-kind-keyed`{.descname} [](#scopes.define.value-kind-keyed "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-keyed}

:   A constant of type `i32`.

*define*{.property} `value-kind-keyed-template`{.descname} [](#scopes.define.value-kind-keyed-template "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-keyed-template}

:   A constant of type `i32`.

*define*{.property} `value-kind-label`{.descname} [](#scopes.define.value-kind-label "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-label}

:   A constant of type `i32`.

*define*{.property} `value-kind-label-template`{.descname} [](#scopes.define.value-kind-label-template "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-label-template}

:   A constant of type `i32`.

*define*{.property} `value-kind-load`{.descname} [](#scopes.define.value-kind-load "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-load}

:   A constant of type `i32`.

*define*{.property} `value-kind-loop`{.descname} [](#scopes.define.value-kind-loop "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-loop}

:   A constant of type `i32`.

*define*{.property} `value-kind-loop-arguments`{.descname} [](#scopes.define.value-kind-loop-arguments "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-loop-arguments}

:   A constant of type `i32`.

*define*{.property} `value-kind-loop-label`{.descname} [](#scopes.define.value-kind-loop-label "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-loop-label}

:   A constant of type `i32`.

*define*{.property} `value-kind-loop-label-arguments`{.descname} [](#scopes.define.value-kind-loop-label-arguments "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-loop-label-arguments}

:   A constant of type `i32`.

*define*{.property} `value-kind-malloc`{.descname} [](#scopes.define.value-kind-malloc "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-malloc}

:   A constant of type `i32`.

*define*{.property} `value-kind-merge`{.descname} [](#scopes.define.value-kind-merge "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-merge}

:   A constant of type `i32`.

*define*{.property} `value-kind-merge-template`{.descname} [](#scopes.define.value-kind-merge-template "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-merge-template}

:   A constant of type `i32`.

*define*{.property} `value-kind-parameter`{.descname} [](#scopes.define.value-kind-parameter "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-parameter}

:   A constant of type `i32`.

*define*{.property} `value-kind-parameter-template`{.descname} [](#scopes.define.value-kind-parameter-template "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-parameter-template}

:   A constant of type `i32`.

*define*{.property} `value-kind-pure-cast`{.descname} [](#scopes.define.value-kind-pure-cast "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-pure-cast}

:   A constant of type `i32`.

*define*{.property} `value-kind-quote`{.descname} [](#scopes.define.value-kind-quote "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-quote}

:   A constant of type `i32`.

*define*{.property} `value-kind-raise`{.descname} [](#scopes.define.value-kind-raise "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-raise}

:   A constant of type `i32`.

*define*{.property} `value-kind-repeat`{.descname} [](#scopes.define.value-kind-repeat "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-repeat}

:   A constant of type `i32`.

*define*{.property} `value-kind-return`{.descname} [](#scopes.define.value-kind-return "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-return}

:   A constant of type `i32`.

*define*{.property} `value-kind-sample`{.descname} [](#scopes.define.value-kind-sample "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-sample}

:   A constant of type `i32`.

*define*{.property} `value-kind-select`{.descname} [](#scopes.define.value-kind-select "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-select}

:   A constant of type `i32`.

*define*{.property} `value-kind-shuffle-vector`{.descname} [](#scopes.define.value-kind-shuffle-vector "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-shuffle-vector}

:   A constant of type `i32`.

*define*{.property} `value-kind-store`{.descname} [](#scopes.define.value-kind-store "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-store}

:   A constant of type `i32`.

*define*{.property} `value-kind-switch`{.descname} [](#scopes.define.value-kind-switch "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-switch}

:   A constant of type `i32`.

*define*{.property} `value-kind-switch-template`{.descname} [](#scopes.define.value-kind-switch-template "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-switch-template}

:   A constant of type `i32`.

*define*{.property} `value-kind-template`{.descname} [](#scopes.define.value-kind-template "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-template}

:   A constant of type `i32`.

*define*{.property} `value-kind-triop`{.descname} [](#scopes.define.value-kind-triop "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-triop}

:   A constant of type `i32`.

*define*{.property} `value-kind-undef`{.descname} [](#scopes.define.value-kind-undef "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-undef}

:   A constant of type `i32`.

*define*{.property} `value-kind-unop`{.descname} [](#scopes.define.value-kind-unop "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-unop}

:   A constant of type `i32`.

*define*{.property} `value-kind-unquote`{.descname} [](#scopes.define.value-kind-unquote "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-unquote}

:   A constant of type `i32`.

*define*{.property} `value-kind-unreachable`{.descname} [](#scopes.define.value-kind-unreachable "Permalink to this definition"){.headerlink} {#scopes.define.value-kind-unreachable}

:   A constant of type `i32`.

*define*{.property} `working-dir`{.descname} [](#scopes.define.working-dir "Permalink to this definition"){.headerlink} {#scopes.define.working-dir}

:   A constant of type `string`.

*type*{.property} `_:`{.descname} [](#scopes.type._: "Permalink to this definition"){.headerlink} {#scopes.type._:}

:   An opaque type labeled `Arguments`.

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes._:.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes._:.spice.__typecall}

    :   

*type*{.property} `Accessor`{.descname} [](#scopes.type.Accessor "Permalink to this definition"){.headerlink} {#scopes.type.Accessor}

:   A plain type of storage type `(opaque@ _Closure)`.

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls closure&ensp;*)[](#scopes.Accessor.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.Accessor.inline.__typecall}

    :   

*type*{.property} `Anchor`{.descname} [](#scopes.type.Anchor "Permalink to this definition"){.headerlink} {#scopes.type.Anchor}

:   A plain type of storage type `(opaque@ _Anchor)`.

    *spice*{.property} `__hash`{.descname} (*&ensp;...&ensp;*)[](#scopes.Anchor.spice.__hash "Permalink to this definition"){.headerlink} {#scopes.Anchor.spice.__hash}

    :   

*type*{.property} `Arguments`{.descname} [](#scopes.type.Arguments "Permalink to this definition"){.headerlink} {#scopes.type.Arguments}

:   An opaque type.

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.Arguments.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.Arguments.spice.__typecall}

    :   

*type*{.property} `Builtin`{.descname} [](#scopes.type.Builtin "Permalink to this definition"){.headerlink} {#scopes.type.Builtin}

:   A plain type of storage type `u64`.

    *spice*{.property} `__hash`{.descname} (*&ensp;...&ensp;*)[](#scopes.Builtin.spice.__hash "Permalink to this definition"){.headerlink} {#scopes.Builtin.spice.__hash}

    :   

*type*{.property} `CEnum`{.descname} [](#scopes.type.CEnum "Permalink to this definition"){.headerlink} {#scopes.type.CEnum}

:   An opaque type of supertype `immutable`.

    *spice*{.property} `__!=`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__!= "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__!=}

    :   

    *spice*{.property} `__&`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__& "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__&}

    :   

    *spice*{.property} `__*`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__* "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__*}

    :   

    *spice*{.property} `__+`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__+ "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__+}

    :   

    *spice*{.property} `__-`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__- "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__-}

    :   

    *spice*{.property} `__/`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__/ "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__/}

    :   

    *spice*{.property} `__//`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__// "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__//}

    :   

    *spice*{.property} `__<`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__< "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__<}

    :   

    *spice*{.property} `__<=`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__<= "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__<=}

    :   

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__== "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__==}

    :   

    *spice*{.property} `__>`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__> "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__>}

    :   

    *spice*{.property} `__>=`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__>= "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__>=}

    :   

    *spice*{.property} `__^`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__^ "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__^}

    :   

    *inline*{.property} `__hash`{.descname} (*&ensp;self&ensp;*)[](#scopes.CEnum.inline.__hash "Permalink to this definition"){.headerlink} {#scopes.CEnum.inline.__hash}

    :   

    *spice*{.property} `__imply`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__imply "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__imply}

    :   

    *inline*{.property} `__neg`{.descname} (*&ensp;self&ensp;*)[](#scopes.CEnum.inline.__neg "Permalink to this definition"){.headerlink} {#scopes.CEnum.inline.__neg}

    :   

    *spice*{.property} `__rimply`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__rimply "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__rimply}

    :   

    *spice*{.property} `__static-imply`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__static-imply "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__static-imply}

    :   

    *spice*{.property} `__|`{.descname} (*&ensp;...&ensp;*)[](#scopes.CEnum.spice.__| "Permalink to this definition"){.headerlink} {#scopes.CEnum.spice.__|}

    :   

    *inline*{.property} `__~`{.descname} (*&ensp;self&ensp;*)[](#scopes.CEnum.inline.__~ "Permalink to this definition"){.headerlink} {#scopes.CEnum.inline.__~}

    :   

*type*{.property} `CStruct`{.descname} [](#scopes.type.CStruct "Permalink to this definition"){.headerlink} {#scopes.type.CStruct}

:   An opaque type.

    *spice*{.property} `__copy`{.descname} (*&ensp;...&ensp;*)[](#scopes.CStruct.spice.__copy "Permalink to this definition"){.headerlink} {#scopes.CStruct.spice.__copy}

    :   

    *spice*{.property} `__drop`{.descname} (*&ensp;...&ensp;*)[](#scopes.CStruct.spice.__drop "Permalink to this definition"){.headerlink} {#scopes.CStruct.spice.__drop}

    :   

    *spice*{.property} `__getattr`{.descname} (*&ensp;...&ensp;*)[](#scopes.CStruct.spice.__getattr "Permalink to this definition"){.headerlink} {#scopes.CStruct.spice.__getattr}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.CStruct.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.CStruct.spice.__typecall}

    :   

*type*{.property} `CUnion`{.descname} [](#scopes.type.CUnion "Permalink to this definition"){.headerlink} {#scopes.type.CUnion}

:   An opaque type.

    *spice*{.property} `__getattr`{.descname} (*&ensp;...&ensp;*)[](#scopes.CUnion.spice.__getattr "Permalink to this definition"){.headerlink} {#scopes.CUnion.spice.__getattr}

    :   

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls value...&ensp;*)[](#scopes.CUnion.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.CUnion.inline.__typecall}

    :   

*type*{.property} `Closure`{.descname} [](#scopes.type.Closure "Permalink to this definition"){.headerlink} {#scopes.type.Closure}

:   A plain type of storage type `(opaque@ _Closure)`.

    *spice*{.property} `__!=`{.descname} (*&ensp;...&ensp;*)[](#scopes.Closure.spice.__!= "Permalink to this definition"){.headerlink} {#scopes.Closure.spice.__!=}

    :   

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.Closure.spice.__== "Permalink to this definition"){.headerlink} {#scopes.Closure.spice.__==}

    :   

    *spice*{.property} `__hash`{.descname} (*&ensp;...&ensp;*)[](#scopes.Closure.spice.__hash "Permalink to this definition"){.headerlink} {#scopes.Closure.spice.__hash}

    :   

    *spice*{.property} `__imply`{.descname} (*&ensp;...&ensp;*)[](#scopes.Closure.spice.__imply "Permalink to this definition"){.headerlink} {#scopes.Closure.spice.__imply}

    :   

    *compiledfn*{.property} `docstring`{.descname} (*&ensp;...&ensp;*)[](#scopes.Closure.compiledfn.docstring "Permalink to this definition"){.headerlink} {#scopes.Closure.compiledfn.docstring}

    :   An external function of type `(string <-: (Closure))`.

*type*{.property} `Collector`{.descname} [](#scopes.type.Collector "Permalink to this definition"){.headerlink} {#scopes.type.Collector}

:   A plain type of storage type `(opaque@ _Closure)`.

    *spice*{.property} `__call`{.descname} (*&ensp;...&ensp;*)[](#scopes.Collector.spice.__call "Permalink to this definition"){.headerlink} {#scopes.Collector.spice.__call}

    :   

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls init valid? at collect&ensp;*)[](#scopes.Collector.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.Collector.inline.__typecall}

    :   

*type*{.property} `CompileStage`{.descname} [](#scopes.type.CompileStage "Permalink to this definition"){.headerlink} {#scopes.type.CompileStage}

:   A plain type of storage type `(tuple _Value Anchor)`.

*type*{.property} `Error`{.descname} [](#scopes.type.Error "Permalink to this definition"){.headerlink} {#scopes.type.Error}

:   A plain type of storage type `(opaque@ _Error)`.

    *spice*{.property} `__copy`{.descname} (*&ensp;...&ensp;*)[](#scopes.Error.spice.__copy "Permalink to this definition"){.headerlink} {#scopes.Error.spice.__copy}

    :   

    *inline*{.property} `append`{.descname} (*&ensp;self anchor traceback-msg&ensp;*)[](#scopes.Error.inline.append "Permalink to this definition"){.headerlink} {#scopes.Error.inline.append}

    :   

    *compiledfn*{.property} `dump`{.descname} (*&ensp;...&ensp;*)[](#scopes.Error.compiledfn.dump "Permalink to this definition"){.headerlink} {#scopes.Error.compiledfn.dump}

    :   An external function of type `(void <-: (Error))`.

    *compiledfn*{.property} `format`{.descname} (*&ensp;...&ensp;*)[](#scopes.Error.compiledfn.format "Permalink to this definition"){.headerlink} {#scopes.Error.compiledfn.format}

    :   An external function of type `(string <-: (Error))`.

*type*{.property} `Generator`{.descname} [](#scopes.type.Generator "Permalink to this definition"){.headerlink} {#scopes.type.Generator}

:   Generators provide a protocol for iterating the contents of containers and
    enumerating sequences. They are primarily used by `for` and `fold`, but can
    also be used separately.
    
    Each generator instance is equivalent to a closure that when called returns
    four functions:
    
    * A function `state... <- fn start ()` which returns the initial state of
      the generator as an arbitrary number of arbitrarily typed values. The
      initially returned state defines the format of the generators internal
      state.
    * A function `bool <- fn valid? (state...)` which takes the current
      generator state and returns `true` when the generator can resolve the
      state to a collection item, otherwise `false`, indicating that the
      generator has been depleted.
    * A function `value... <- fn at (state...)` which takes the current
      generator state and returns the collection item this state maps to. The
      function may not be called for a state for which `valid?` has reported
      to be depleted.
    * A function `state... <- fn next (state...)` which takes the current
      generator state and returns the state mapping to the next item in the
      collection. The new state must have the same type signature as the
      previous state. The function may not be called for a state for which
      `valid?` has reported to be depleted.
    
    Any of these functions may be called multiple times with any valid state,
    effectively restarting the Generator at an arbitrary point, as Generators
    are not expected to have side effects. In controlled circumstances a
    Generator may choose to be impure, but should be documented accordingly.
    
    Here is a typical pattern for constructing a generator:
    
        :::scopes
        inline make-generator (container)
            Generator
                inline "start" ()
                    # return the first iterator of sequence (might not be valid)
                    'start container
                inline "valid?" (it...)
                    # return true if the iterator is still valid
                    'valid-iterator? container it...
                inline "at" (it...)
                    # return variadic result at iterator
                    '@ container it...
                inline "next" (it...)
                    # return the next iterator in sequence
                    'next container it...
    
    The generator can then be subsequently used like this:
    
        :::scopes
        # this example prints up to two elements returned by a generator
        # generate a new instance bound to container
        let gen = (make-generator container)
        # extract all methods
        let start valid? at next = (gen)
        # get the init state
        let state... = (start)
        # check if the state is valid
        if (valid? state...)
            # container has at least one item; print it
            print (at state...)
            # advance to the next state
            let state... = (next state...)
            if (valid? state...)
                # container has one more item; print it
                print (at state...)
        # we are done; no cleanup necessary

    *spice*{.property} `__call`{.descname} (*&ensp;self&ensp;*)[](#scopes.Generator.spice.__call "Permalink to this definition"){.headerlink} {#scopes.Generator.spice.__call}
    
    :   Returns, in this order, the four functions `start`, `valid?`,
        `init` and `next` which are required to enumerate generator
        `self`.

    *inline*{.property} `__countof`{.descname} (*&ensp;self&ensp;*)[](#scopes.Generator.inline.__countof "Permalink to this definition"){.headerlink} {#scopes.Generator.inline.__countof}

    :   

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls start valid? at next&ensp;*)[](#scopes.Generator.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.Generator.inline.__typecall}

    :   Takes four functions `start`, `valid?`, `at` and `next`
        and returns a new generator ready for use.

*type*{.property} `Image`{.descname} [](#scopes.type.Image "Permalink to this definition"){.headerlink} {#scopes.type.Image}

:   An opaque type.

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.Image.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.Image.spice.__typecall}

    :   

    *compiledfn*{.property} `type`{.descname} (*&ensp;...&ensp;*)[](#scopes.Image.compiledfn.type "Permalink to this definition"){.headerlink} {#scopes.Image.compiledfn.type}

    :   An external function of type `(type <-: (type Symbol i32 i32 i32 i32 Symbol Symbol))`.

*type*{.property} `MethodsAccessor`{.descname} [](#scopes.type.MethodsAccessor "Permalink to this definition"){.headerlink} {#scopes.type.MethodsAccessor}

:   An opaque type.

    *spice*{.property} `__typeattr`{.descname} (*&ensp;...&ensp;*)[](#scopes.MethodsAccessor.spice.__typeattr "Permalink to this definition"){.headerlink} {#scopes.MethodsAccessor.spice.__typeattr}

    :   

*type*{.property} `Nothing`{.descname} [](#scopes.type.Nothing "Permalink to this definition"){.headerlink} {#scopes.type.Nothing}

:   A plain type of storage type `(tuple )`.

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.Nothing.spice.__== "Permalink to this definition"){.headerlink} {#scopes.Nothing.spice.__==}

    :   

    *spice*{.property} `__copy`{.descname} (*&ensp;...&ensp;*)[](#scopes.Nothing.spice.__copy "Permalink to this definition"){.headerlink} {#scopes.Nothing.spice.__copy}

    :   

    *inline*{.property} `__hash`{.descname} (*&ensp;self&ensp;*)[](#scopes.Nothing.inline.__hash "Permalink to this definition"){.headerlink} {#scopes.Nothing.inline.__hash}

    :   

    *inline*{.property} `__tobool`{.descname} ()[](#scopes.Nothing.inline.__tobool "Permalink to this definition"){.headerlink} {#scopes.Nothing.inline.__tobool}

    :   

*type*{.property} `NullType`{.descname} [](#scopes.type.NullType "Permalink to this definition"){.headerlink} {#scopes.type.NullType}

:   The type of the `null` constant. This type is uninstantiable.

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.NullType.spice.__== "Permalink to this definition"){.headerlink} {#scopes.NullType.spice.__==}

    :   

    *spice*{.property} `__imply`{.descname} (*&ensp;...&ensp;*)[](#scopes.NullType.spice.__imply "Permalink to this definition"){.headerlink} {#scopes.NullType.spice.__imply}

    :   

    *spice*{.property} `__r==`{.descname} (*&ensp;...&ensp;*)[](#scopes.NullType.spice.__r== "Permalink to this definition"){.headerlink} {#scopes.NullType.spice.__r==}

    :   

    *inline*{.property} `__repr`{.descname} (*&ensp;self&ensp;*)[](#scopes.NullType.inline.__repr "Permalink to this definition"){.headerlink} {#scopes.NullType.inline.__repr}

    :   

*type*{.property} `OverloadedFunction`{.descname} [](#scopes.type.OverloadedFunction "Permalink to this definition"){.headerlink} {#scopes.type.OverloadedFunction}

:   An opaque type.

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.OverloadedFunction.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.OverloadedFunction.spice.__typecall}

    :   

    *spice*{.property} `append`{.descname} (*&ensp;...&ensp;*)[](#scopes.OverloadedFunction.spice.append "Permalink to this definition"){.headerlink} {#scopes.OverloadedFunction.spice.append}

    :   

*type*{.property} `Qualify`{.descname} [](#scopes.type.Qualify "Permalink to this definition"){.headerlink} {#scopes.type.Qualify}

:   An opaque type.

*type*{.property} `Raises`{.descname} [](#scopes.type.Raises "Permalink to this definition"){.headerlink} {#scopes.type.Raises}

:   An opaque type.

*type*{.property} `SampledImage`{.descname} [](#scopes.type.SampledImage "Permalink to this definition"){.headerlink} {#scopes.type.SampledImage}

:   An opaque type.

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.SampledImage.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.SampledImage.spice.__typecall}

    :   

    *compiledfn*{.property} `type`{.descname} (*&ensp;...&ensp;*)[](#scopes.SampledImage.compiledfn.type "Permalink to this definition"){.headerlink} {#scopes.SampledImage.compiledfn.type}

    :   An external function of type `(type <-: (type))`.

*type*{.property} `Sampler`{.descname} [](#scopes.type.Sampler "Permalink to this definition"){.headerlink} {#scopes.type.Sampler}

:   A plain type of supertype `immutable` and of storage type `Sampler`.

*type*{.property} `Scope`{.descname} [](#scopes.type.Scope "Permalink to this definition"){.headerlink} {#scopes.type.Scope}

:   A plain type of storage type `(opaque@ _Scope)`.

    *compiledfn*{.property} `@`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.compiledfn.@ "Permalink to this definition"){.headerlink} {#scopes.Scope.compiledfn.@}

    :   An external function of type `(Value <-: (Scope Value) raises Error)`.

    *spice*{.property} `__..`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.spice.__.. "Permalink to this definition"){.headerlink} {#scopes.Scope.spice.__..}

    :   

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.spice.__== "Permalink to this definition"){.headerlink} {#scopes.Scope.spice.__==}

    :   

    *spice*{.property} `__as`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.spice.__as "Permalink to this definition"){.headerlink} {#scopes.Scope.spice.__as}

    :   

    *spice*{.property} `__getattr`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.spice.__getattr "Permalink to this definition"){.headerlink} {#scopes.Scope.spice.__getattr}

    :   

    *spice*{.property} `__hash`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.spice.__hash "Permalink to this definition"){.headerlink} {#scopes.Scope.spice.__hash}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.Scope.spice.__typecall}

    :   

    *spice*{.property} `bind`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.spice.bind "Permalink to this definition"){.headerlink} {#scopes.Scope.spice.bind}

    :   

    *inline*{.property} `bind-symbols`{.descname} (*&ensp;self values...&ensp;*)[](#scopes.Scope.inline.bind-symbols "Permalink to this definition"){.headerlink} {#scopes.Scope.inline.bind-symbols}

    :   

    *compiledfn*{.property} `bind-with-docstring`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.compiledfn.bind-with-docstring "Permalink to this definition"){.headerlink} {#scopes.Scope.compiledfn.bind-with-docstring}

    :   An external function of type `(Scope <-: (Scope Value Value string))`.

    *spice*{.property} `define`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.spice.define "Permalink to this definition"){.headerlink} {#scopes.Scope.spice.define}

    :   

    *inline*{.property} `define-symbols`{.descname} (*&ensp;self values...&ensp;*)[](#scopes.Scope.inline.define-symbols "Permalink to this definition"){.headerlink} {#scopes.Scope.inline.define-symbols}

    :   

    *inline*{.property} `deleted`{.descname} (*&ensp;self&ensp;*)[](#scopes.Scope.inline.deleted "Permalink to this definition"){.headerlink} {#scopes.Scope.inline.deleted}

    :   

    *compiledfn*{.property} `docstring`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.compiledfn.docstring "Permalink to this definition"){.headerlink} {#scopes.Scope.compiledfn.docstring}

    :   An external function of type `(string <-: (Scope Value))`.

    *inline*{.property} `lineage`{.descname} (*&ensp;self&ensp;*)[](#scopes.Scope.inline.lineage "Permalink to this definition"){.headerlink} {#scopes.Scope.inline.lineage}

    :   

    *compiledfn*{.property} `local@`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.compiledfn.local@ "Permalink to this definition"){.headerlink} {#scopes.Scope.compiledfn.local@}

    :   An external function of type `(Value <-: (Scope Value) raises Error)`.

    *compiledfn*{.property} `module-docstring`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.compiledfn.module-docstring "Permalink to this definition"){.headerlink} {#scopes.Scope.compiledfn.module-docstring}

    :   An external function of type `(string <-: (Scope))`.

    *compiledfn*{.property} `next`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.compiledfn.next "Permalink to this definition"){.headerlink} {#scopes.Scope.compiledfn.next}

    :   An external function of type `((_: Value Value i32) <-: (Scope i32))`.

    *compiledfn*{.property} `next-deleted`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.compiledfn.next-deleted "Permalink to this definition"){.headerlink} {#scopes.Scope.compiledfn.next-deleted}

    :   An external function of type `((_: Value i32) <-: (Scope i32))`.

    *compiledfn*{.property} `parent`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.compiledfn.parent "Permalink to this definition"){.headerlink} {#scopes.Scope.compiledfn.parent}

    :   An external function of type `(Scope <-: (Scope))`.

    *compiledfn*{.property} `reparent`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.compiledfn.reparent "Permalink to this definition"){.headerlink} {#scopes.Scope.compiledfn.reparent}

    :   An external function of type `(Scope <-: (Scope Scope))`.

    *compiledfn*{.property} `unbind`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.compiledfn.unbind "Permalink to this definition"){.headerlink} {#scopes.Scope.compiledfn.unbind}

    :   An external function of type `(Scope <-: (Scope Value))`.

    *compiledfn*{.property} `unparent`{.descname} (*&ensp;...&ensp;*)[](#scopes.Scope.compiledfn.unparent "Permalink to this definition"){.headerlink} {#scopes.Scope.compiledfn.unparent}

    :   An external function of type `(Scope <-: (Scope))`.

*type*{.property} `SourceFile`{.descname} [](#scopes.type.SourceFile "Permalink to this definition"){.headerlink} {#scopes.type.SourceFile}

:   A plain type of storage type `(opaque@ _SourceFile)`.

*type*{.property} `SpiceMacro`{.descname} [](#scopes.type.SpiceMacro "Permalink to this definition"){.headerlink} {#scopes.type.SpiceMacro}

:   A plain type of storage type `(opaque@ (Value <-: (Value) raises Error))`.

    *spice*{.property} `__rimply`{.descname} (*&ensp;...&ensp;*)[](#scopes.SpiceMacro.spice.__rimply "Permalink to this definition"){.headerlink} {#scopes.SpiceMacro.spice.__rimply}

    :   

*type*{.property} `SpiceMacroFunction`{.descname} [](#scopes.type.SpiceMacroFunction "Permalink to this definition"){.headerlink} {#scopes.type.SpiceMacroFunction}

:   A plain type labeled `(opaque@ (Value <-: (Value) raises Error))` of supertype `pointer` and of storage type `(opaque@ (Value <-: (Value) raises Error))`.

*type*{.property} `Struct`{.descname} [](#scopes.type.Struct "Permalink to this definition"){.headerlink} {#scopes.type.Struct}

:   An opaque type.

    *spice*{.property} `__drop`{.descname} (*&ensp;...&ensp;*)[](#scopes.Struct.spice.__drop "Permalink to this definition"){.headerlink} {#scopes.Struct.spice.__drop}

    :   

    *builtin*{.property} `__getattr`{.descname} (*&ensp;...&ensp;*)[](#scopes.Struct.builtin.__getattr "Permalink to this definition"){.headerlink} {#scopes.Struct.builtin.__getattr}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.Struct.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.Struct.spice.__typecall}

    :   

*type*{.property} `SugarMacro`{.descname} [](#scopes.type.SugarMacro "Permalink to this definition"){.headerlink} {#scopes.type.SugarMacro}

:   A plain type of storage type `(opaque@ ((_: List Scope) <-: (List Scope) raises Error))`.

    *spice*{.property} `__call`{.descname} (*&ensp;...&ensp;*)[](#scopes.SugarMacro.spice.__call "Permalink to this definition"){.headerlink} {#scopes.SugarMacro.spice.__call}

    :   

*type*{.property} `SugarMacroFunction`{.descname} [](#scopes.type.SugarMacroFunction "Permalink to this definition"){.headerlink} {#scopes.type.SugarMacroFunction}

:   A plain type labeled `(opaque@ ((_: List Scope) <-: (List Scope) raises Error))` of supertype `pointer` and of storage type `(opaque@ ((_: List Scope) <-: (List Scope) raises Error))`.

*type*{.property} `Symbol`{.descname} [](#scopes.type.Symbol "Permalink to this definition"){.headerlink} {#scopes.type.Symbol}

:   A plain type of supertype `immutable` and of storage type `u64`.

    *spice*{.property} `__!=`{.descname} (*&ensp;...&ensp;*)[](#scopes.Symbol.spice.__!= "Permalink to this definition"){.headerlink} {#scopes.Symbol.spice.__!=}

    :   

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.Symbol.spice.__== "Permalink to this definition"){.headerlink} {#scopes.Symbol.spice.__==}

    :   

    *spice*{.property} `__as`{.descname} (*&ensp;...&ensp;*)[](#scopes.Symbol.spice.__as "Permalink to this definition"){.headerlink} {#scopes.Symbol.spice.__as}

    :   

    *spice*{.property} `__call`{.descname} (*&ensp;...&ensp;*)[](#scopes.Symbol.spice.__call "Permalink to this definition"){.headerlink} {#scopes.Symbol.spice.__call}

    :   

    *spice*{.property} `__hash`{.descname} (*&ensp;...&ensp;*)[](#scopes.Symbol.spice.__hash "Permalink to this definition"){.headerlink} {#scopes.Symbol.spice.__hash}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.Symbol.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.Symbol.spice.__typecall}

    :   

    *inline*{.property} `unique`{.descname} (*&ensp;cls name&ensp;*)[](#scopes.Symbol.inline.unique "Permalink to this definition"){.headerlink} {#scopes.Symbol.inline.unique}

    :   

    *compiledfn*{.property} `variadic?`{.descname} (*&ensp;...&ensp;*)[](#scopes.Symbol.compiledfn.variadic? "Permalink to this definition"){.headerlink} {#scopes.Symbol.compiledfn.variadic?}

    :   An external function of type `(bool <-: (Symbol))`.

*type*{.property} `TypeArrayPointer`{.descname} [](#scopes.type.TypeArrayPointer "Permalink to this definition"){.headerlink} {#scopes.type.TypeArrayPointer}

:   A plain type labeled `(@ type)` of supertype `pointer` and of storage type `(@ type)`.

*type*{.property} `TypeInitializer`{.descname} [](#scopes.type.TypeInitializer "Permalink to this definition"){.headerlink} {#scopes.type.TypeInitializer}

:   An opaque type.

    *inline*{.property} `__static-imply`{.descname} (*&ensp;cls T&ensp;*)[](#scopes.TypeInitializer.inline.__static-imply "Permalink to this definition"){.headerlink} {#scopes.TypeInitializer.inline.__static-imply}

    :   

*type*{.property} `Unknown`{.descname} [](#scopes.type.Unknown "Permalink to this definition"){.headerlink} {#scopes.type.Unknown}

:   A plain type of storage type `(opaque@ _type)`.

*type*{.property} `Value`{.descname} [](#scopes.type.Value "Permalink to this definition"){.headerlink} {#scopes.type.Value}

:   A plain type of storage type `(tuple _Value Anchor)`.

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.spice.__== "Permalink to this definition"){.headerlink} {#scopes.Value.spice.__==}

    :   

    *inline*{.property} `__as`{.descname} (*&ensp;vT T&ensp;*)[](#scopes.Value.inline.__as "Permalink to this definition"){.headerlink} {#scopes.Value.inline.__as}

    :   

    *spice*{.property} `__copy`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.spice.__copy "Permalink to this definition"){.headerlink} {#scopes.Value.spice.__copy}

    :   

    *inline*{.property} `__hash`{.descname} (*&ensp;self&ensp;*)[](#scopes.Value.inline.__hash "Permalink to this definition"){.headerlink} {#scopes.Value.inline.__hash}

    :   

    *compiledfn*{.property} `__repr`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.compiledfn.__repr "Permalink to this definition"){.headerlink} {#scopes.Value.compiledfn.__repr}

    :   An external function of type `(string <-: (Value))`.

    *inline*{.property} `__rimply`{.descname} (*&ensp;vT T&ensp;*)[](#scopes.Value.inline.__rimply "Permalink to this definition"){.headerlink} {#scopes.Value.inline.__rimply}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.Value.spice.__typecall}

    :   

    *compiledfn*{.property} `anchor`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.compiledfn.anchor "Permalink to this definition"){.headerlink} {#scopes.Value.compiledfn.anchor}

    :   An external function of type `(Anchor <-: (Value))`.

    *compiledfn*{.property} `argcount`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.compiledfn.argcount "Permalink to this definition"){.headerlink} {#scopes.Value.compiledfn.argcount}

    :   An external function of type `(i32 <-: (Value))`.

    *inline*{.property} `arglist-sink`{.descname} (*&ensp;N&ensp;*)[](#scopes.Value.inline.arglist-sink "Permalink to this definition"){.headerlink} {#scopes.Value.inline.arglist-sink}

    :   

    *inline*{.property} `args`{.descname} (*&ensp;self&ensp;*)[](#scopes.Value.inline.args "Permalink to this definition"){.headerlink} {#scopes.Value.inline.args}

    :   

    *compiledfn*{.property} `constant?`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.compiledfn.constant? "Permalink to this definition"){.headerlink} {#scopes.Value.compiledfn.constant?}

    :   An external function of type `(bool <-: (Value))`.

    *fn*{.property} `dekey`{.descname} (*&ensp;self&ensp;*)[](#scopes.Value.fn.dekey "Permalink to this definition"){.headerlink} {#scopes.Value.fn.dekey}

    :   

    *inline*{.property} `dump`{.descname} (*&ensp;self&ensp;*)[](#scopes.Value.inline.dump "Permalink to this definition"){.headerlink} {#scopes.Value.inline.dump}

    :   

    *compiledfn*{.property} `getarg`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.compiledfn.getarg "Permalink to this definition"){.headerlink} {#scopes.Value.compiledfn.getarg}

    :   An external function of type `(Value <-: (Value i32))`.

    *compiledfn*{.property} `getarglist`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.compiledfn.getarglist "Permalink to this definition"){.headerlink} {#scopes.Value.compiledfn.getarglist}

    :   An external function of type `(Value <-: (Value i32))`.

    *compiledfn*{.property} `kind`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.compiledfn.kind "Permalink to this definition"){.headerlink} {#scopes.Value.compiledfn.kind}

    :   An external function of type `(i32 <-: (Value))`.

    *compiledfn*{.property} `none?`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.compiledfn.none? "Permalink to this definition"){.headerlink} {#scopes.Value.compiledfn.none?}

    :   A compiled function of type `(bool <-: (Value))`.

    *compiledfn*{.property} `pure?`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.compiledfn.pure? "Permalink to this definition"){.headerlink} {#scopes.Value.compiledfn.pure?}

    :   An external function of type `(bool <-: (Value))`.

    *compiledfn*{.property} `qualified-typeof`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.compiledfn.qualified-typeof "Permalink to this definition"){.headerlink} {#scopes.Value.compiledfn.qualified-typeof}

    :   An external function of type `(type <-: (Value))`.

    *compiledfn*{.property} `qualifiersof`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.compiledfn.qualifiersof "Permalink to this definition"){.headerlink} {#scopes.Value.compiledfn.qualifiersof}

    :   An external function of type `(type <-: (Value))`.

    *inline*{.property} `reverse-args`{.descname} (*&ensp;self&ensp;*)[](#scopes.Value.inline.reverse-args "Permalink to this definition"){.headerlink} {#scopes.Value.inline.reverse-args}

    :   

    *compiledfn*{.property} `spice-repr`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.compiledfn.spice-repr "Permalink to this definition"){.headerlink} {#scopes.Value.compiledfn.spice-repr}

    :   An external function of type `(string <-: (Value))`.

    *inline*{.property} `tag`{.descname} (*&ensp;self anchor&ensp;*)[](#scopes.Value.inline.tag "Permalink to this definition"){.headerlink} {#scopes.Value.inline.tag}

    :   

    *compiledfn*{.property} `typeof`{.descname} (*&ensp;...&ensp;*)[](#scopes.Value.compiledfn.typeof "Permalink to this definition"){.headerlink} {#scopes.Value.compiledfn.typeof}

    :   An external function of type `(type <-: (Value))`.

*type*{.property} `ValueArrayPointer`{.descname} [](#scopes.type.ValueArrayPointer "Permalink to this definition"){.headerlink} {#scopes.type.ValueArrayPointer}

:   A plain type labeled `(@ Value)` of supertype `pointer` and of storage type `(@ Value)`.

*type*{.property} `Variadic`{.descname} [](#scopes.type.Variadic "Permalink to this definition"){.headerlink} {#scopes.type.Variadic}

:   An opaque type labeled `...`.

*type*{.property} `aggregate`{.descname} [](#scopes.type.aggregate "Permalink to this definition"){.headerlink} {#scopes.type.aggregate}

:   An opaque type.

    *spice*{.property} `__copy`{.descname} (*&ensp;...&ensp;*)[](#scopes.aggregate.spice.__copy "Permalink to this definition"){.headerlink} {#scopes.aggregate.spice.__copy}

    :   

    *spice*{.property} `__drop`{.descname} (*&ensp;...&ensp;*)[](#scopes.aggregate.spice.__drop "Permalink to this definition"){.headerlink} {#scopes.aggregate.spice.__drop}

    :   

*type*{.property} `array`{.descname} [](#scopes.type.array "Permalink to this definition"){.headerlink} {#scopes.type.array}

:   An opaque type of supertype `aggregate`.

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.array.spice.__== "Permalink to this definition"){.headerlink} {#scopes.array.spice.__==}

    :   

    *inline*{.property} `__@`{.descname} (*&ensp;self index&ensp;*)[](#scopes.array.inline.__@ "Permalink to this definition"){.headerlink} {#scopes.array.inline.__@}

    :   

    *spice*{.property} `__as`{.descname} (*&ensp;...&ensp;*)[](#scopes.array.spice.__as "Permalink to this definition"){.headerlink} {#scopes.array.spice.__as}

    :   

    *spice*{.property} `__countof`{.descname} (*&ensp;...&ensp;*)[](#scopes.array.spice.__countof "Permalink to this definition"){.headerlink} {#scopes.array.spice.__countof}

    :   

    *spice*{.property} `__imply`{.descname} (*&ensp;...&ensp;*)[](#scopes.array.spice.__imply "Permalink to this definition"){.headerlink} {#scopes.array.spice.__imply}

    :   

    *spice*{.property} `__rimply`{.descname} (*&ensp;...&ensp;*)[](#scopes.array.spice.__rimply "Permalink to this definition"){.headerlink} {#scopes.array.spice.__rimply}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.array.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.array.spice.__typecall}

    :   

    *spice*{.property} `__typematch`{.descname} (*&ensp;...&ensp;*)[](#scopes.array.spice.__typematch "Permalink to this definition"){.headerlink} {#scopes.array.spice.__typematch}

    :   

    *spice*{.property} `__unpack`{.descname} (*&ensp;...&ensp;*)[](#scopes.array.spice.__unpack "Permalink to this definition"){.headerlink} {#scopes.array.spice.__unpack}

    :   

    *inline*{.property} `type`{.descname} (*&ensp;element-type size&ensp;*)[](#scopes.array.inline.type "Permalink to this definition"){.headerlink} {#scopes.array.inline.type}

    :   

*type*{.property} `bool`{.descname} [](#scopes.type.bool "Permalink to this definition"){.headerlink} {#scopes.type.bool}

:   A plain type of supertype `integer` and of storage type `bool`.

*type*{.property} `char`{.descname} [](#scopes.type.char "Permalink to this definition"){.headerlink} {#scopes.type.char}

:   A plain type labeled `i8` of supertype `integer` and of storage type `i8`.

*type*{.property} `constant`{.descname} [](#scopes.type.constant "Permalink to this definition"){.headerlink} {#scopes.type.constant}

:   An opaque type.

*type*{.property} `f128`{.descname} [](#scopes.type.f128 "Permalink to this definition"){.headerlink} {#scopes.type.f128}

:   A plain type of supertype `real` and of storage type `f128`.

*type*{.property} `f16`{.descname} [](#scopes.type.f16 "Permalink to this definition"){.headerlink} {#scopes.type.f16}

:   A plain type of supertype `real` and of storage type `f16`.

*type*{.property} `f32`{.descname} [](#scopes.type.f32 "Permalink to this definition"){.headerlink} {#scopes.type.f32}

:   A plain type of supertype `real` and of storage type `f32`.

*type*{.property} `f64`{.descname} [](#scopes.type.f64 "Permalink to this definition"){.headerlink} {#scopes.type.f64}

:   A plain type of supertype `real` and of storage type `f64`.

*type*{.property} `f80`{.descname} [](#scopes.type.f80 "Permalink to this definition"){.headerlink} {#scopes.type.f80}

:   A plain type of supertype `real` and of storage type `f80`.

*type*{.property} `function`{.descname} [](#scopes.type.function "Permalink to this definition"){.headerlink} {#scopes.type.function}

:   An opaque type.

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.function.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.function.spice.__typecall}

    :   

    *spice*{.property} `type`{.descname} (*&ensp;...&ensp;*)[](#scopes.function.spice.type "Permalink to this definition"){.headerlink} {#scopes.function.spice.type}

    :   

*type*{.property} `hash`{.descname} [](#scopes.type.hash "Permalink to this definition"){.headerlink} {#scopes.type.hash}

:   A plain type of storage type `u64`.

    *spice*{.property} `__!=`{.descname} (*&ensp;...&ensp;*)[](#scopes.hash.spice.__!= "Permalink to this definition"){.headerlink} {#scopes.hash.spice.__!=}

    :   

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.hash.spice.__== "Permalink to this definition"){.headerlink} {#scopes.hash.spice.__==}

    :   

    *spice*{.property} `__as`{.descname} (*&ensp;...&ensp;*)[](#scopes.hash.spice.__as "Permalink to this definition"){.headerlink} {#scopes.hash.spice.__as}

    :   

    *inline*{.property} `__hash`{.descname} (*&ensp;self&ensp;*)[](#scopes.hash.inline.__hash "Permalink to this definition"){.headerlink} {#scopes.hash.inline.__hash}

    :   

    *spice*{.property} `__ras`{.descname} (*&ensp;...&ensp;*)[](#scopes.hash.spice.__ras "Permalink to this definition"){.headerlink} {#scopes.hash.spice.__ras}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.hash.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.hash.spice.__typecall}

    :   

    *inline*{.property} `from-bytes`{.descname} (*&ensp;data size&ensp;*)[](#scopes.hash.inline.from-bytes "Permalink to this definition"){.headerlink} {#scopes.hash.inline.from-bytes}

    :   

*type*{.property} `i16`{.descname} [](#scopes.type.i16 "Permalink to this definition"){.headerlink} {#scopes.type.i16}

:   A plain type of supertype `integer` and of storage type `i16`.

*type*{.property} `i32`{.descname} [](#scopes.type.i32 "Permalink to this definition"){.headerlink} {#scopes.type.i32}

:   A plain type of supertype `integer` and of storage type `i32`.

*type*{.property} `i64`{.descname} [](#scopes.type.i64 "Permalink to this definition"){.headerlink} {#scopes.type.i64}

:   A plain type of supertype `integer` and of storage type `i64`.

*type*{.property} `i8`{.descname} [](#scopes.type.i8 "Permalink to this definition"){.headerlink} {#scopes.type.i8}

:   A plain type of supertype `integer` and of storage type `i8`.

*type*{.property} `immutable`{.descname} [](#scopes.type.immutable "Permalink to this definition"){.headerlink} {#scopes.type.immutable}

:   An opaque type.

    *spice*{.property} `__copy`{.descname} (*&ensp;...&ensp;*)[](#scopes.immutable.spice.__copy "Permalink to this definition"){.headerlink} {#scopes.immutable.spice.__copy}

    :   

*type*{.property} `incomplete`{.descname} [](#scopes.type.incomplete "Permalink to this definition"){.headerlink} {#scopes.type.incomplete}

:   An opaque type.

*type*{.property} `integer`{.descname} [](#scopes.type.integer "Permalink to this definition"){.headerlink} {#scopes.type.integer}

:   An opaque type of supertype `immutable`.

    *spice*{.property} `__!=`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__!= "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__!=}

    :   

    *spice*{.property} `__%`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__% "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__%}

    :   

    *spice*{.property} `__&`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__& "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__&}

    :   

    *spice*{.property} `__*`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__* "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__*}

    :   

    *spice*{.property} `__**`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__** "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__**}

    :   

    *spice*{.property} `__+`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__+ "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__+}

    :   

    *spice*{.property} `__-`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__- "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__-}

    :   

    *spice*{.property} `__/`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__/ "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__/}

    :   

    *spice*{.property} `__//`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__// "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__//}

    :   

    *spice*{.property} `__<`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__< "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__<}

    :   

    *spice*{.property} `__<<`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__<< "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__<<}

    :   

    *spice*{.property} `__<=`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__<= "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__<=}

    :   

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__== "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__==}

    :   

    *spice*{.property} `__>`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__> "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__>}

    :   

    *spice*{.property} `__>=`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__>= "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__>=}

    :   

    *spice*{.property} `__>>`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__>> "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__>>}

    :   

    *spice*{.property} `__^`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__^ "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__^}

    :   

    *spice*{.property} `__as`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__as "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__as}

    :   

    *spice*{.property} `__hash`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__hash "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__hash}

    :   

    *spice*{.property} `__imply`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__imply "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__imply}

    :   

    *inline*{.property} `__ln`{.descname} (*&ensp;self&ensp;*)[](#scopes.integer.inline.__ln "Permalink to this definition"){.headerlink} {#scopes.integer.inline.__ln}

    :   

    *inline*{.property} `__neg`{.descname} (*&ensp;self&ensp;*)[](#scopes.integer.inline.__neg "Permalink to this definition"){.headerlink} {#scopes.integer.inline.__neg}

    :   

    *inline*{.property} `__rcp`{.descname} (*&ensp;self&ensp;*)[](#scopes.integer.inline.__rcp "Permalink to this definition"){.headerlink} {#scopes.integer.inline.__rcp}

    :   

    *spice*{.property} `__static-imply`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__static-imply "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__static-imply}

    :   

    *spice*{.property} `__tobool`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__tobool "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__tobool}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__typecall}

    :   

    *builtin*{.property} `__vector!=`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.builtin.__vector!= "Permalink to this definition"){.headerlink} {#scopes.integer.builtin.__vector!=}

    :   

    *spice*{.property} `__vector%`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__vector% "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__vector%}

    :   

    *builtin*{.property} `__vector&`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.builtin.__vector& "Permalink to this definition"){.headerlink} {#scopes.integer.builtin.__vector&}

    :   

    *builtin*{.property} `__vector*`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.builtin.__vector* "Permalink to this definition"){.headerlink} {#scopes.integer.builtin.__vector*}

    :   

    *builtin*{.property} `__vector+`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.builtin.__vector+ "Permalink to this definition"){.headerlink} {#scopes.integer.builtin.__vector+}

    :   

    *builtin*{.property} `__vector-`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.builtin.__vector- "Permalink to this definition"){.headerlink} {#scopes.integer.builtin.__vector-}

    :   

    *spice*{.property} `__vector//`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__vector// "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__vector//}

    :   

    *spice*{.property} `__vector<`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__vector< "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__vector<}

    :   

    *builtin*{.property} `__vector<<`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.builtin.__vector<< "Permalink to this definition"){.headerlink} {#scopes.integer.builtin.__vector<<}

    :   

    *spice*{.property} `__vector<=`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__vector<= "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__vector<=}

    :   

    *builtin*{.property} `__vector==`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.builtin.__vector== "Permalink to this definition"){.headerlink} {#scopes.integer.builtin.__vector==}

    :   

    *spice*{.property} `__vector>`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__vector> "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__vector>}

    :   

    *spice*{.property} `__vector>=`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__vector>= "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__vector>=}

    :   

    *spice*{.property} `__vector>>`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__vector>> "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__vector>>}

    :   

    *builtin*{.property} `__vector^`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.builtin.__vector^ "Permalink to this definition"){.headerlink} {#scopes.integer.builtin.__vector^}

    :   

    *builtin*{.property} `__vector|`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.builtin.__vector| "Permalink to this definition"){.headerlink} {#scopes.integer.builtin.__vector|}

    :   

    *spice*{.property} `__|`{.descname} (*&ensp;...&ensp;*)[](#scopes.integer.spice.__| "Permalink to this definition"){.headerlink} {#scopes.integer.spice.__|}

    :   

    *inline*{.property} `__~`{.descname} (*&ensp;self&ensp;*)[](#scopes.integer.inline.__~ "Permalink to this definition"){.headerlink} {#scopes.integer.inline.__~}

    :   

*type*{.property} `intptr`{.descname} [](#scopes.type.intptr "Permalink to this definition"){.headerlink} {#scopes.type.intptr}

:   A plain type labeled `u64` of supertype `integer` and of storage type `u64`.

*type*{.property} `list`{.descname} [](#scopes.type.list "Permalink to this definition"){.headerlink} {#scopes.type.list}

:   A plain type labeled `List` of storage type `(opaque@ _List)`.

    *compiledfn*{.property} `@`{.descname} (*&ensp;...&ensp;*)[](#scopes.list.compiledfn.@ "Permalink to this definition"){.headerlink} {#scopes.list.compiledfn.@}

    :   An external function of type `(Value <-: (List))`.

    *spice*{.property} `__..`{.descname} (*&ensp;...&ensp;*)[](#scopes.list.spice.__.. "Permalink to this definition"){.headerlink} {#scopes.list.spice.__..}

    :   

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.list.spice.__== "Permalink to this definition"){.headerlink} {#scopes.list.spice.__==}

    :   

    *spice*{.property} `__as`{.descname} (*&ensp;...&ensp;*)[](#scopes.list.spice.__as "Permalink to this definition"){.headerlink} {#scopes.list.spice.__as}

    :   

    *spice*{.property} `__countof`{.descname} (*&ensp;...&ensp;*)[](#scopes.list.spice.__countof "Permalink to this definition"){.headerlink} {#scopes.list.spice.__countof}

    :   

    *inline*{.property} `__repr`{.descname} (*&ensp;self&ensp;*)[](#scopes.list.inline.__repr "Permalink to this definition"){.headerlink} {#scopes.list.inline.__repr}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.list.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.list.spice.__typecall}

    :   

    *spice*{.property} `__unpack`{.descname} (*&ensp;...&ensp;*)[](#scopes.list.spice.__unpack "Permalink to this definition"){.headerlink} {#scopes.list.spice.__unpack}

    :   

    *inline*{.property} `cons-sink`{.descname} (*&ensp;self&ensp;*)[](#scopes.list.inline.cons-sink "Permalink to this definition"){.headerlink} {#scopes.list.inline.cons-sink}

    :   

    *spice*{.property} `decons`{.descname} (*&ensp;...&ensp;*)[](#scopes.list.spice.decons "Permalink to this definition"){.headerlink} {#scopes.list.spice.decons}

    :   

    *compiledfn*{.property} `dump`{.descname} (*&ensp;...&ensp;*)[](#scopes.list.compiledfn.dump "Permalink to this definition"){.headerlink} {#scopes.list.compiledfn.dump}

    :   An external function of type `(List <-: (List))`.

    *fn*{.property} `first-anchor`{.descname} (*&ensp;self&ensp;*)[](#scopes.list.fn.first-anchor "Permalink to this definition"){.headerlink} {#scopes.list.fn.first-anchor}

    :   

    *compiledfn*{.property} `join`{.descname} (*&ensp;...&ensp;*)[](#scopes.list.compiledfn.join "Permalink to this definition"){.headerlink} {#scopes.list.compiledfn.join}

    :   An external function of type `(List <-: (List List))`.

    *compiledfn*{.property} `next`{.descname} (*&ensp;...&ensp;*)[](#scopes.list.compiledfn.next "Permalink to this definition"){.headerlink} {#scopes.list.compiledfn.next}

    :   An external function of type `(List <-: (List))`.

    *compiledfn*{.property} `reverse`{.descname} (*&ensp;...&ensp;*)[](#scopes.list.compiledfn.reverse "Permalink to this definition"){.headerlink} {#scopes.list.compiledfn.reverse}

    :   An external function of type `(List <-: (List))`.

    *fn*{.property} `rjoin`{.descname} (*&ensp;lside rside&ensp;*)[](#scopes.list.fn.rjoin "Permalink to this definition"){.headerlink} {#scopes.list.fn.rjoin}

    :   

    *compiledfn*{.property} `serialize`{.descname} (*&ensp;...&ensp;*)[](#scopes.list.compiledfn.serialize "Permalink to this definition"){.headerlink} {#scopes.list.compiledfn.serialize}

    :   An external function of type `(string <-: (List))`.

    *fn*{.property} `token-split`{.descname} (*&ensp;expr token errmsg&ensp;*)[](#scopes.list.fn.token-split "Permalink to this definition"){.headerlink} {#scopes.list.fn.token-split}

    :   

*type*{.property} `matrix`{.descname} [](#scopes.type.matrix "Permalink to this definition"){.headerlink} {#scopes.type.matrix}

:   An opaque type of supertype `immutable`.

    *builtin*{.property} `__@`{.descname} (*&ensp;...&ensp;*)[](#scopes.matrix.builtin.__@ "Permalink to this definition"){.headerlink} {#scopes.matrix.builtin.__@}

    :   

    *spice*{.property} `__countof`{.descname} (*&ensp;...&ensp;*)[](#scopes.matrix.spice.__countof "Permalink to this definition"){.headerlink} {#scopes.matrix.spice.__countof}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.matrix.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.matrix.spice.__typecall}

    :   

    *spice*{.property} `__unpack`{.descname} (*&ensp;...&ensp;*)[](#scopes.matrix.spice.__unpack "Permalink to this definition"){.headerlink} {#scopes.matrix.spice.__unpack}

    :   

    *inline*{.property} `type`{.descname} (*&ensp;element-type size&ensp;*)[](#scopes.matrix.inline.type "Permalink to this definition"){.headerlink} {#scopes.matrix.inline.type}

    :   

*type*{.property} `nodefault`{.descname} [](#scopes.type.nodefault "Permalink to this definition"){.headerlink} {#scopes.type.nodefault}

:   An opaque type.

*type*{.property} `noreturn`{.descname} [](#scopes.type.noreturn "Permalink to this definition"){.headerlink} {#scopes.type.noreturn}

:   An opaque type.

*type*{.property} `opaquepointer`{.descname} [](#scopes.type.opaquepointer "Permalink to this definition"){.headerlink} {#scopes.type.opaquepointer}

:   An opaque type.

*type*{.property} `pointer`{.descname} [](#scopes.type.pointer "Permalink to this definition"){.headerlink} {#scopes.type.pointer}

:   An opaque type.

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.pointer.spice.__== "Permalink to this definition"){.headerlink} {#scopes.pointer.spice.__==}

    :   

    *inline*{.property} `__@`{.descname} (*&ensp;self index&ensp;*)[](#scopes.pointer.inline.__@ "Permalink to this definition"){.headerlink} {#scopes.pointer.inline.__@}

    :   

    *spice*{.property} `__as`{.descname} (*&ensp;...&ensp;*)[](#scopes.pointer.spice.__as "Permalink to this definition"){.headerlink} {#scopes.pointer.spice.__as}

    :   

    *spice*{.property} `__call`{.descname} (*&ensp;...&ensp;*)[](#scopes.pointer.spice.__call "Permalink to this definition"){.headerlink} {#scopes.pointer.spice.__call}

    :   

    *spice*{.property} `__copy`{.descname} (*&ensp;...&ensp;*)[](#scopes.pointer.spice.__copy "Permalink to this definition"){.headerlink} {#scopes.pointer.spice.__copy}

    :   

    *inline*{.property} `__getattr`{.descname} (*&ensp;self key&ensp;*)[](#scopes.pointer.inline.__getattr "Permalink to this definition"){.headerlink} {#scopes.pointer.inline.__getattr}

    :   

    *spice*{.property} `__hash`{.descname} (*&ensp;...&ensp;*)[](#scopes.pointer.spice.__hash "Permalink to this definition"){.headerlink} {#scopes.pointer.spice.__hash}

    :   

    *spice*{.property} `__imply`{.descname} (*&ensp;...&ensp;*)[](#scopes.pointer.spice.__imply "Permalink to this definition"){.headerlink} {#scopes.pointer.spice.__imply}

    :   

    *spice*{.property} `__ras`{.descname} (*&ensp;...&ensp;*)[](#scopes.pointer.spice.__ras "Permalink to this definition"){.headerlink} {#scopes.pointer.spice.__ras}

    :   

    *inline*{.property} `__toref`{.descname} (*&ensp;self&ensp;*)[](#scopes.pointer.inline.__toref "Permalink to this definition"){.headerlink} {#scopes.pointer.inline.__toref}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.pointer.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.pointer.spice.__typecall}

    :   

    *inline*{.property} `type`{.descname} (*&ensp;T&ensp;*)[](#scopes.pointer.inline.type "Permalink to this definition"){.headerlink} {#scopes.pointer.inline.type}

    :   

*type*{.property} `rawstring`{.descname} [](#scopes.type.rawstring "Permalink to this definition"){.headerlink} {#scopes.type.rawstring}

:   A plain type labeled `(@ i8)` of supertype `pointer` and of storage type `(@ i8)`.

*type*{.property} `real`{.descname} [](#scopes.type.real "Permalink to this definition"){.headerlink} {#scopes.type.real}

:   An opaque type of supertype `immutable`.

    *spice*{.property} `__!=`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__!= "Permalink to this definition"){.headerlink} {#scopes.real.spice.__!=}

    :   

    *spice*{.property} `__%`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__% "Permalink to this definition"){.headerlink} {#scopes.real.spice.__%}

    :   

    *spice*{.property} `__*`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__* "Permalink to this definition"){.headerlink} {#scopes.real.spice.__*}

    :   

    *spice*{.property} `__**`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__** "Permalink to this definition"){.headerlink} {#scopes.real.spice.__**}

    :   

    *spice*{.property} `__+`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__+ "Permalink to this definition"){.headerlink} {#scopes.real.spice.__+}

    :   

    *spice*{.property} `__-`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__- "Permalink to this definition"){.headerlink} {#scopes.real.spice.__-}

    :   

    *spice*{.property} `__/`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__/ "Permalink to this definition"){.headerlink} {#scopes.real.spice.__/}

    :   

    *spice*{.property} `__//`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__// "Permalink to this definition"){.headerlink} {#scopes.real.spice.__//}

    :   

    *spice*{.property} `__<`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__< "Permalink to this definition"){.headerlink} {#scopes.real.spice.__<}

    :   

    *spice*{.property} `__<=`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__<= "Permalink to this definition"){.headerlink} {#scopes.real.spice.__<=}

    :   

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__== "Permalink to this definition"){.headerlink} {#scopes.real.spice.__==}

    :   

    *spice*{.property} `__>`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__> "Permalink to this definition"){.headerlink} {#scopes.real.spice.__>}

    :   

    *spice*{.property} `__>=`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__>= "Permalink to this definition"){.headerlink} {#scopes.real.spice.__>=}

    :   

    *spice*{.property} `__as`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__as "Permalink to this definition"){.headerlink} {#scopes.real.spice.__as}

    :   

    *spice*{.property} `__hash`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__hash "Permalink to this definition"){.headerlink} {#scopes.real.spice.__hash}

    :   

    *spice*{.property} `__imply`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__imply "Permalink to this definition"){.headerlink} {#scopes.real.spice.__imply}

    :   

    *builtin*{.property} `__ln`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.builtin.__ln "Permalink to this definition"){.headerlink} {#scopes.real.builtin.__ln}

    :   

    *spice*{.property} `__neg`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.spice.__neg "Permalink to this definition"){.headerlink} {#scopes.real.spice.__neg}

    :   

    *inline*{.property} `__rcp`{.descname} (*&ensp;self&ensp;*)[](#scopes.real.inline.__rcp "Permalink to this definition"){.headerlink} {#scopes.real.inline.__rcp}

    :   

    *inline*{.property} `__tobool`{.descname} (*&ensp;self&ensp;*)[](#scopes.real.inline.__tobool "Permalink to this definition"){.headerlink} {#scopes.real.inline.__tobool}

    :   

    *inline*{.property} `__typecall`{.descname} (*&ensp;cls value&ensp;*)[](#scopes.real.inline.__typecall "Permalink to this definition"){.headerlink} {#scopes.real.inline.__typecall}

    :   

    *builtin*{.property} `__vector!=`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.builtin.__vector!= "Permalink to this definition"){.headerlink} {#scopes.real.builtin.__vector!=}

    :   

    *builtin*{.property} `__vector%`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.builtin.__vector% "Permalink to this definition"){.headerlink} {#scopes.real.builtin.__vector%}

    :   

    *builtin*{.property} `__vector*`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.builtin.__vector* "Permalink to this definition"){.headerlink} {#scopes.real.builtin.__vector*}

    :   

    *builtin*{.property} `__vector**`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.builtin.__vector** "Permalink to this definition"){.headerlink} {#scopes.real.builtin.__vector**}

    :   

    *builtin*{.property} `__vector+`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.builtin.__vector+ "Permalink to this definition"){.headerlink} {#scopes.real.builtin.__vector+}

    :   

    *builtin*{.property} `__vector-`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.builtin.__vector- "Permalink to this definition"){.headerlink} {#scopes.real.builtin.__vector-}

    :   

    *builtin*{.property} `__vector/`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.builtin.__vector/ "Permalink to this definition"){.headerlink} {#scopes.real.builtin.__vector/}

    :   

    *builtin*{.property} `__vector<`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.builtin.__vector< "Permalink to this definition"){.headerlink} {#scopes.real.builtin.__vector<}

    :   

    *builtin*{.property} `__vector<=`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.builtin.__vector<= "Permalink to this definition"){.headerlink} {#scopes.real.builtin.__vector<=}

    :   

    *builtin*{.property} `__vector==`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.builtin.__vector== "Permalink to this definition"){.headerlink} {#scopes.real.builtin.__vector==}

    :   

    *builtin*{.property} `__vector>`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.builtin.__vector> "Permalink to this definition"){.headerlink} {#scopes.real.builtin.__vector>}

    :   

    *builtin*{.property} `__vector>=`{.descname} (*&ensp;...&ensp;*)[](#scopes.real.builtin.__vector>= "Permalink to this definition"){.headerlink} {#scopes.real.builtin.__vector>=}

    :   

*type*{.property} `string`{.descname} [](#scopes.type.string "Permalink to this definition"){.headerlink} {#scopes.type.string}

:   A plain type of supertype `opaquepointer` and of storage type `(opaque@ _string)`.

    *spice*{.property} `__!=`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.spice.__!= "Permalink to this definition"){.headerlink} {#scopes.string.spice.__!=}

    :   

    *spice*{.property} `__..`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.spice.__.. "Permalink to this definition"){.headerlink} {#scopes.string.spice.__..}

    :   

    *spice*{.property} `__<`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.spice.__< "Permalink to this definition"){.headerlink} {#scopes.string.spice.__<}

    :   

    *spice*{.property} `__<=`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.spice.__<= "Permalink to this definition"){.headerlink} {#scopes.string.spice.__<=}

    :   

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.spice.__== "Permalink to this definition"){.headerlink} {#scopes.string.spice.__==}

    :   

    *spice*{.property} `__>`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.spice.__> "Permalink to this definition"){.headerlink} {#scopes.string.spice.__>}

    :   

    *spice*{.property} `__>=`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.spice.__>= "Permalink to this definition"){.headerlink} {#scopes.string.spice.__>=}

    :   

    *fn*{.property} `__@`{.descname} (*&ensp;self i&ensp;*)[](#scopes.string.fn.__@ "Permalink to this definition"){.headerlink} {#scopes.string.fn.__@}

    :   

    *spice*{.property} `__as`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.spice.__as "Permalink to this definition"){.headerlink} {#scopes.string.spice.__as}

    :   

    *spice*{.property} `__countof`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.spice.__countof "Permalink to this definition"){.headerlink} {#scopes.string.spice.__countof}

    :   

    *inline*{.property} `__hash`{.descname} (*&ensp;self&ensp;*)[](#scopes.string.inline.__hash "Permalink to this definition"){.headerlink} {#scopes.string.inline.__hash}

    :   

    *spice*{.property} `__imply`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.spice.__imply "Permalink to this definition"){.headerlink} {#scopes.string.spice.__imply}

    :   

    *compiledfn*{.property} `__lslice`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.compiledfn.__lslice "Permalink to this definition"){.headerlink} {#scopes.string.compiledfn.__lslice}

    :   An external function of type `(string <-: (string usize))`.

    *spice*{.property} `__ras`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.spice.__ras "Permalink to this definition"){.headerlink} {#scopes.string.spice.__ras}

    :   

    *compiledfn*{.property} `__rslice`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.compiledfn.__rslice "Permalink to this definition"){.headerlink} {#scopes.string.compiledfn.__rslice}

    :   An external function of type `(string <-: (string usize))`.

    *compiledfn*{.property} `buffer`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.compiledfn.buffer "Permalink to this definition"){.headerlink} {#scopes.string.compiledfn.buffer}

    :   An external function of type `((_: (@ i8) usize) <-: (string))`.

    *inline*{.property} `collector`{.descname} (*&ensp;maxsize&ensp;*)[](#scopes.string.inline.collector "Permalink to this definition"){.headerlink} {#scopes.string.inline.collector}

    :   

    *compiledfn*{.property} `join`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.compiledfn.join "Permalink to this definition"){.headerlink} {#scopes.string.compiledfn.join}

    :   An external function of type `(string <-: (string string))`.

    *compiledfn*{.property} `match?`{.descname} (*&ensp;...&ensp;*)[](#scopes.string.compiledfn.match? "Permalink to this definition"){.headerlink} {#scopes.string.compiledfn.match?}

    :   An external function of type `((_: bool i32 i32) <-: (string string) raises Error)`.

    *inline*{.property} `range`{.descname} (*&ensp;self start end&ensp;*)[](#scopes.string.inline.range "Permalink to this definition"){.headerlink} {#scopes.string.inline.range}

    :   

*type*{.property} `tuple`{.descname} [](#scopes.type.tuple "Permalink to this definition"){.headerlink} {#scopes.type.tuple}

:   An opaque type of supertype `aggregate`.

    *builtin*{.property} `__@`{.descname} (*&ensp;...&ensp;*)[](#scopes.tuple.builtin.__@ "Permalink to this definition"){.headerlink} {#scopes.tuple.builtin.__@}

    :   

    *spice*{.property} `__countof`{.descname} (*&ensp;...&ensp;*)[](#scopes.tuple.spice.__countof "Permalink to this definition"){.headerlink} {#scopes.tuple.spice.__countof}

    :   

    *builtin*{.property} `__getattr`{.descname} (*&ensp;...&ensp;*)[](#scopes.tuple.builtin.__getattr "Permalink to this definition"){.headerlink} {#scopes.tuple.builtin.__getattr}

    :   

    *spice*{.property} `__hash`{.descname} (*&ensp;...&ensp;*)[](#scopes.tuple.spice.__hash "Permalink to this definition"){.headerlink} {#scopes.tuple.spice.__hash}

    :   

    *inline*{.property} `__rin`{.descname} (*&ensp;T selfT&ensp;*)[](#scopes.tuple.inline.__rin "Permalink to this definition"){.headerlink} {#scopes.tuple.inline.__rin}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.tuple.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.tuple.spice.__typecall}

    :   

    *spice*{.property} `__unpack`{.descname} (*&ensp;...&ensp;*)[](#scopes.tuple.spice.__unpack "Permalink to this definition"){.headerlink} {#scopes.tuple.spice.__unpack}

    :   

    *inline*{.property} `emit`{.descname} (*&ensp;self keys...&ensp;*)[](#scopes.tuple.inline.emit "Permalink to this definition"){.headerlink} {#scopes.tuple.inline.emit}

    :   

    *spice*{.property} `explode`{.descname} (*&ensp;...&ensp;*)[](#scopes.tuple.spice.explode "Permalink to this definition"){.headerlink} {#scopes.tuple.spice.explode}

    :   

    *spice*{.property} `packed`{.descname} (*&ensp;...&ensp;*)[](#scopes.tuple.spice.packed "Permalink to this definition"){.headerlink} {#scopes.tuple.spice.packed}

    :   

    *spice*{.property} `packed-type`{.descname} (*&ensp;...&ensp;*)[](#scopes.tuple.spice.packed-type "Permalink to this definition"){.headerlink} {#scopes.tuple.spice.packed-type}

    :   

    *spice*{.property} `type`{.descname} (*&ensp;...&ensp;*)[](#scopes.tuple.spice.type "Permalink to this definition"){.headerlink} {#scopes.tuple.spice.type}

    :   

*type*{.property} `type`{.descname} [](#scopes.type.type "Permalink to this definition"){.headerlink} {#scopes.type.type}

:   A plain type of supertype `opaquepointer` and of storage type `(opaque@ _type)`.

    *compiledfn*{.property} `@`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.@ "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.@}

    :   An external function of type `(Value <-: (type Symbol) raises Error)`.

    *spice*{.property} `__!=`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.spice.__!= "Permalink to this definition"){.headerlink} {#scopes.type.spice.__!=}

    :   

    *spice*{.property} `__<`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.spice.__< "Permalink to this definition"){.headerlink} {#scopes.type.spice.__<}

    :   

    *spice*{.property} `__<=`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.spice.__<= "Permalink to this definition"){.headerlink} {#scopes.type.spice.__<=}

    :   

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.spice.__== "Permalink to this definition"){.headerlink} {#scopes.type.spice.__==}

    :   

    *spice*{.property} `__>`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.spice.__> "Permalink to this definition"){.headerlink} {#scopes.type.spice.__>}

    :   

    *spice*{.property} `__>=`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.spice.__>= "Permalink to this definition"){.headerlink} {#scopes.type.spice.__>=}

    :   

    *compiledfn*{.property} `__@`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.__@ "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.__@}

    :   An external function of type `(type <-: (type i32) raises Error)`.

    *spice*{.property} `__call`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.spice.__call "Permalink to this definition"){.headerlink} {#scopes.type.spice.__call}

    :   

    *spice*{.property} `__countof`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.spice.__countof "Permalink to this definition"){.headerlink} {#scopes.type.spice.__countof}

    :   

    *spice*{.property} `__getattr`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.spice.__getattr "Permalink to this definition"){.headerlink} {#scopes.type.spice.__getattr}

    :   

    *spice*{.property} `__hash`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.spice.__hash "Permalink to this definition"){.headerlink} {#scopes.type.spice.__hash}

    :   

    *sugar*{.property} (`__macro`{.descname} *&ensp;...&ensp;*) [](#scopes.type.sugar.__macro "Permalink to this definition"){.headerlink} {#scopes.type.sugar.__macro}

    :   

    *spice*{.property} `__toptr`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.spice.__toptr "Permalink to this definition"){.headerlink} {#scopes.type.spice.__toptr}

    :   

    *inline*{.property} `__toref`{.descname} (*&ensp;self&ensp;*)[](#scopes.type.inline.__toref "Permalink to this definition"){.headerlink} {#scopes.type.inline.__toref}

    :   

    *compiledfn*{.property} `alignof`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.alignof "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.alignof}

    :   An external function of type `(usize <-: (type) raises Error)`.

    *compiledfn*{.property} `bitcount`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.bitcount "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.bitcount}

    :   An external function of type `(i32 <-: (type))`.

    *fn*{.property} `change-element-type`{.descname} (*&ensp;cls ET&ensp;*)[](#scopes.type.fn.change-element-type "Permalink to this definition"){.headerlink} {#scopes.type.fn.change-element-type}

    :   

    *fn*{.property} `change-storage-class`{.descname} (*&ensp;cls storage-class&ensp;*)[](#scopes.type.fn.change-storage-class "Permalink to this definition"){.headerlink} {#scopes.type.fn.change-storage-class}

    :   

    *spice*{.property} `define-symbol`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.spice.define-symbol "Permalink to this definition"){.headerlink} {#scopes.type.spice.define-symbol}

    :   

    *inline*{.property} `define-symbols`{.descname} (*&ensp;self values...&ensp;*)[](#scopes.type.inline.define-symbols "Permalink to this definition"){.headerlink} {#scopes.type.inline.define-symbols}

    :   

    *spice*{.property} `dispatch-attr`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.spice.dispatch-attr "Permalink to this definition"){.headerlink} {#scopes.type.spice.dispatch-attr}

    :   

    *compiledfn*{.property} `docstring`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.docstring "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.docstring}

    :   An external function of type `(string <-: (type Symbol))`.

    *compiledfn*{.property} `element-count`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.element-count "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.element-count}

    :   An external function of type `(i32 <-: (type) raises Error)`.

    *compiledfn*{.property} `element@`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.element@ "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.element@}

    :   An external function of type `(type <-: (type i32) raises Error)`.

    *inline*{.property} `elements`{.descname} (*&ensp;self&ensp;*)[](#scopes.type.inline.elements "Permalink to this definition"){.headerlink} {#scopes.type.inline.elements}

    :   

    *fn*{.property} `function-pointer?`{.descname} (*&ensp;cls&ensp;*)[](#scopes.type.fn.function-pointer? "Permalink to this definition"){.headerlink} {#scopes.type.fn.function-pointer?}

    :   

    *fn*{.property} `function?`{.descname} (*&ensp;cls&ensp;*)[](#scopes.type.fn.function? "Permalink to this definition"){.headerlink} {#scopes.type.fn.function?}

    :   

    *fn*{.property} `immutable`{.descname} (*&ensp;cls&ensp;*)[](#scopes.type.fn.immutable "Permalink to this definition"){.headerlink} {#scopes.type.fn.immutable}

    :   

    *inline*{.property} `key-type`{.descname} (*&ensp;self key&ensp;*)[](#scopes.type.inline.key-type "Permalink to this definition"){.headerlink} {#scopes.type.inline.key-type}

    :   

    *compiledfn*{.property} `keyof`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.keyof "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.keyof}

    :   An external function of type `((_: Symbol type) <-: (type))`.

    *compiledfn*{.property} `kind`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.kind "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.kind}

    :   An external function of type `(i32 <-: (type))`.

    *compiledfn*{.property} `local@`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.local@ "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.local@}

    :   An external function of type `(Value <-: (type Symbol) raises Error)`.

    *fn*{.property} `mutable`{.descname} (*&ensp;cls&ensp;*)[](#scopes.type.fn.mutable "Permalink to this definition"){.headerlink} {#scopes.type.fn.mutable}

    :   

    *fn*{.property} `mutable&`{.descname} (*&ensp;cls&ensp;*)[](#scopes.type.fn.mutable& "Permalink to this definition"){.headerlink} {#scopes.type.fn.mutable&}

    :   

    *compiledfn*{.property} `offsetof`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.offsetof "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.offsetof}

    :   An external function of type `(usize <-: (type i32) raises Error)`.

    *compiledfn*{.property} `opaque?`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.opaque? "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.opaque?}

    :   An external function of type `(bool <-: (type))`.

    *compiledfn*{.property} `plain?`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.plain? "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.plain?}

    :   An external function of type `(bool <-: (type))`.

    *fn*{.property} `pointer->refer-type`{.descname} (*&ensp;cls&ensp;*)[](#scopes.type.fn.pointer->refer-type "Permalink to this definition"){.headerlink} {#scopes.type.fn.pointer->refer-type}

    :   

    *fn*{.property} `pointer-storage-class`{.descname} (*&ensp;cls&ensp;*)[](#scopes.type.fn.pointer-storage-class "Permalink to this definition"){.headerlink} {#scopes.type.fn.pointer-storage-class}

    :   

    *fn*{.property} `pointer?`{.descname} (*&ensp;cls&ensp;*)[](#scopes.type.fn.pointer? "Permalink to this definition"){.headerlink} {#scopes.type.fn.pointer?}

    :   

    *spice*{.property} `raises`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.spice.raises "Permalink to this definition"){.headerlink} {#scopes.type.spice.raises}

    :   

    *fn*{.property} `readable?`{.descname} (*&ensp;cls&ensp;*)[](#scopes.type.fn.readable? "Permalink to this definition"){.headerlink} {#scopes.type.fn.readable?}

    :   

    *fn*{.property} `refer->pointer-type`{.descname} (*&ensp;cls&ensp;*)[](#scopes.type.fn.refer->pointer-type "Permalink to this definition"){.headerlink} {#scopes.type.fn.refer->pointer-type}

    :   

    *compiledfn*{.property} `refer?`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.refer? "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.refer?}

    :   An external function of type `(bool <-: (type))`.

    *compiledfn*{.property} `return-type`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.return-type "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.return-type}

    :   An external function of type `((_: type type) <-: (type))`.

    *compiledfn*{.property} `set-docstring`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.set-docstring "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.set-docstring}

    :   An external function of type `(void <-: (type Symbol string))`.

    *inline*{.property} `set-opaque`{.descname} (*&ensp;type&ensp;*)[](#scopes.type.inline.set-opaque "Permalink to this definition"){.headerlink} {#scopes.type.inline.set-opaque}

    :   

    *inline*{.property} `set-plain-storage`{.descname} (*&ensp;type storage-type&ensp;*)[](#scopes.type.inline.set-plain-storage "Permalink to this definition"){.headerlink} {#scopes.type.inline.set-plain-storage}

    :   

    *inline*{.property} `set-storage`{.descname} (*&ensp;type storage-type&ensp;*)[](#scopes.type.inline.set-storage "Permalink to this definition"){.headerlink} {#scopes.type.inline.set-storage}

    :   

    *spice*{.property} `set-symbol`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.spice.set-symbol "Permalink to this definition"){.headerlink} {#scopes.type.spice.set-symbol}

    :   

    *inline*{.property} `set-symbols`{.descname} (*&ensp;self values...&ensp;*)[](#scopes.type.inline.set-symbols "Permalink to this definition"){.headerlink} {#scopes.type.inline.set-symbols}

    :   

    *compiledfn*{.property} `signed?`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.signed? "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.signed?}

    :   An external function of type `(bool <-: (type))`.

    *compiledfn*{.property} `sizeof`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.sizeof "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.sizeof}

    :   An external function of type `(usize <-: (type) raises Error)`.

    *compiledfn*{.property} `storageof`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.storageof "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.storageof}

    :   An external function of type `(type <-: (type) raises Error)`.

    *compiledfn*{.property} `string`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.string "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.string}

    :   An external function of type `(string <-: (type))`.

    *fn*{.property} `strip-pointer-storage-class`{.descname} (*&ensp;cls&ensp;*)[](#scopes.type.fn.strip-pointer-storage-class "Permalink to this definition"){.headerlink} {#scopes.type.fn.strip-pointer-storage-class}

    :   

    *compiledfn*{.property} `strip-qualifiers`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.strip-qualifiers "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.strip-qualifiers}

    :   An external function of type `(type <-: (type))`.

    *compiledfn*{.property} `superof`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.superof "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.superof}

    :   An external function of type `(type <-: (type))`.

    *inline*{.property} `symbols`{.descname} (*&ensp;self&ensp;*)[](#scopes.type.inline.symbols "Permalink to this definition"){.headerlink} {#scopes.type.inline.symbols}

    :   

    *compiledfn*{.property} `unique-type`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.unique-type "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.unique-type}

    :   An external function of type `(type <-: (type i32))`.

    *compiledfn*{.property} `unsized?`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.unsized? "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.unsized?}

    :   An external function of type `(bool <-: (type) raises Error)`.

    *compiledfn*{.property} `variadic?`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.variadic? "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.variadic?}

    :   An external function of type `(bool <-: (type))`.

    *inline*{.property} `view-type`{.descname} (*&ensp;self id&ensp;*)[](#scopes.type.inline.view-type "Permalink to this definition"){.headerlink} {#scopes.type.inline.view-type}

    :   

    *compiledfn*{.property} `view?`{.descname} (*&ensp;...&ensp;*)[](#scopes.type.compiledfn.view? "Permalink to this definition"){.headerlink} {#scopes.type.compiledfn.view?}

    :   An external function of type `(bool <-: (type))`.

    *fn*{.property} `writable?`{.descname} (*&ensp;cls&ensp;*)[](#scopes.type.fn.writable? "Permalink to this definition"){.headerlink} {#scopes.type.fn.writable?}

    :   

*type*{.property} `typename`{.descname} [](#scopes.type.typename "Permalink to this definition"){.headerlink} {#scopes.type.typename}

:   An opaque type.

    *spice*{.property} `__!=`{.descname} (*&ensp;...&ensp;*)[](#scopes.typename.spice.__!= "Permalink to this definition"){.headerlink} {#scopes.typename.spice.__!=}

    :   

    *spice*{.property} `__=`{.descname} (*&ensp;...&ensp;*)[](#scopes.typename.spice.__= "Permalink to this definition"){.headerlink} {#scopes.typename.spice.__=}

    :   

    *spice*{.property} `__methodcall`{.descname} (*&ensp;...&ensp;*)[](#scopes.typename.spice.__methodcall "Permalink to this definition"){.headerlink} {#scopes.typename.spice.__methodcall}

    :   

    *spice*{.property} `__toptr`{.descname} (*&ensp;...&ensp;*)[](#scopes.typename.spice.__toptr "Permalink to this definition"){.headerlink} {#scopes.typename.spice.__toptr}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.typename.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.typename.spice.__typecall}

    :   

    *compiledfn*{.property} `type`{.descname} (*&ensp;...&ensp;*)[](#scopes.typename.compiledfn.type "Permalink to this definition"){.headerlink} {#scopes.typename.compiledfn.type}

    :   An external function of type `(type <-: (string type) raises Error)`.

*type*{.property} `u16`{.descname} [](#scopes.type.u16 "Permalink to this definition"){.headerlink} {#scopes.type.u16}

:   A plain type of supertype `integer` and of storage type `u16`.

*type*{.property} `u32`{.descname} [](#scopes.type.u32 "Permalink to this definition"){.headerlink} {#scopes.type.u32}

:   A plain type of supertype `integer` and of storage type `u32`.

*type*{.property} `u64`{.descname} [](#scopes.type.u64 "Permalink to this definition"){.headerlink} {#scopes.type.u64}

:   A plain type of supertype `integer` and of storage type `u64`.

*type*{.property} `u8`{.descname} [](#scopes.type.u8 "Permalink to this definition"){.headerlink} {#scopes.type.u8}

:   A plain type of supertype `integer` and of storage type `u8`.

*type*{.property} `union`{.descname} [](#scopes.type.union "Permalink to this definition"){.headerlink} {#scopes.type.union}

:   An opaque type.

*type*{.property} `usize`{.descname} [](#scopes.type.usize "Permalink to this definition"){.headerlink} {#scopes.type.usize}

:   A plain type of supertype `integer` and of storage type `u64`.

*type*{.property} `vector`{.descname} [](#scopes.type.vector "Permalink to this definition"){.headerlink} {#scopes.type.vector}

:   An opaque type of supertype `immutable`.

    *spice*{.property} `__!=`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__!= "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__!=}

    :   

    *spice*{.property} `__%`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__% "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__%}

    :   

    *spice*{.property} `__&`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__& "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__&}

    :   

    *spice*{.property} `__*`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__* "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__*}

    :   

    *spice*{.property} `__**`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__** "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__**}

    :   

    *spice*{.property} `__+`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__+ "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__+}

    :   

    *spice*{.property} `__-`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__- "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__-}

    :   

    *spice*{.property} `__/`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__/ "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__/}

    :   

    *spice*{.property} `__//`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__// "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__//}

    :   

    *spice*{.property} `__<`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__< "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__<}

    :   

    *spice*{.property} `__<<`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__<< "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__<<}

    :   

    *spice*{.property} `__<=`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__<= "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__<=}

    :   

    *spice*{.property} `__==`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__== "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__==}

    :   

    *spice*{.property} `__>`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__> "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__>}

    :   

    *spice*{.property} `__>=`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__>= "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__>=}

    :   

    *spice*{.property} `__>>`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__>> "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__>>}

    :   

    *builtin*{.property} `__@`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.builtin.__@ "Permalink to this definition"){.headerlink} {#scopes.vector.builtin.__@}

    :   

    *spice*{.property} `__^`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__^ "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__^}

    :   

    *spice*{.property} `__countof`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__countof "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__countof}

    :   

    *spice*{.property} `__lslice`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__lslice "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__lslice}

    :   

    *spice*{.property} `__rslice`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__rslice "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__rslice}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__typecall}

    :   

    *spice*{.property} `__unpack`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__unpack "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__unpack}

    :   

    *spice*{.property} `__|`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.__| "Permalink to this definition"){.headerlink} {#scopes.vector.spice.__|}

    :   

    *spice*{.property} `smear`{.descname} (*&ensp;...&ensp;*)[](#scopes.vector.spice.smear "Permalink to this definition"){.headerlink} {#scopes.vector.spice.smear}

    :   

    *inline*{.property} `type`{.descname} (*&ensp;element-type size&ensp;*)[](#scopes.vector.inline.type "Permalink to this definition"){.headerlink} {#scopes.vector.inline.type}

    :   

*type*{.property} `void`{.descname} [](#scopes.type.void "Permalink to this definition"){.headerlink} {#scopes.type.void}

:   An opaque type of supertype `Arguments`.

*type*{.property} `voidstar`{.descname} [](#scopes.type.voidstar "Permalink to this definition"){.headerlink} {#scopes.type.voidstar}

:   A plain type labeled `(opaque@ void)` of supertype `pointer` and of storage type `(opaque@ void)`.

*inline*{.property} `%=`{.descname} (*&ensp;lhs rhs&ensp;*)[](#scopes.inline.%= "Permalink to this definition"){.headerlink} {#scopes.inline.%=}

:   

*inline*{.property} `&=`{.descname} (*&ensp;lhs rhs&ensp;*)[](#scopes.inline.&= "Permalink to this definition"){.headerlink} {#scopes.inline.&=}

:   

*inline*{.property} `*=`{.descname} (*&ensp;lhs rhs&ensp;*)[](#scopes.inline.*= "Permalink to this definition"){.headerlink} {#scopes.inline.*=}

:   

*inline*{.property} `+=`{.descname} (*&ensp;lhs rhs&ensp;*)[](#scopes.inline.+= "Permalink to this definition"){.headerlink} {#scopes.inline.+=}

:   

*inline*{.property} `-=`{.descname} (*&ensp;lhs rhs&ensp;*)[](#scopes.inline.-= "Permalink to this definition"){.headerlink} {#scopes.inline.-=}

:   

*inline*{.property} `..=`{.descname} (*&ensp;lhs rhs&ensp;*)[](#scopes.inline...= "Permalink to this definition"){.headerlink} {#scopes.inline...=}

:   

*inline*{.property} `//=`{.descname} (*&ensp;lhs rhs&ensp;*)[](#scopes.inline.//= "Permalink to this definition"){.headerlink} {#scopes.inline.//=}

:   

*inline*{.property} `/=`{.descname} (*&ensp;lhs rhs&ensp;*)[](#scopes.inline./= "Permalink to this definition"){.headerlink} {#scopes.inline./=}

:   

*inline*{.property} `<<=`{.descname} (*&ensp;lhs rhs&ensp;*)[](#scopes.inline.<<= "Permalink to this definition"){.headerlink} {#scopes.inline.<<=}

:   

*inline*{.property} `>>=`{.descname} (*&ensp;lhs rhs&ensp;*)[](#scopes.inline.>>= "Permalink to this definition"){.headerlink} {#scopes.inline.>>=}

:   

*inline*{.property} `^=`{.descname} (*&ensp;lhs rhs&ensp;*)[](#scopes.inline.^= "Permalink to this definition"){.headerlink} {#scopes.inline.^=}

:   

*inline*{.property} `|=`{.descname} (*&ensp;lhs rhs&ensp;*)[](#scopes.inline.|= "Permalink to this definition"){.headerlink} {#scopes.inline.|=}

:   

*fn*{.property} `Value-none?`{.descname} (*&ensp;value&ensp;*)[](#scopes.fn.Value-none? "Permalink to this definition"){.headerlink} {#scopes.fn.Value-none?}

:   

*inline*{.property} `aggregate-type-constructor`{.descname} (*&ensp;start f&ensp;*)[](#scopes.inline.aggregate-type-constructor "Permalink to this definition"){.headerlink} {#scopes.inline.aggregate-type-constructor}

:   

*fn*{.property} `all?`{.descname} (*&ensp;v&ensp;*)[](#scopes.fn.all? "Permalink to this definition"){.headerlink} {#scopes.fn.all?}

:   

*fn*{.property} `any?`{.descname} (*&ensp;v&ensp;*)[](#scopes.fn.any? "Permalink to this definition"){.headerlink} {#scopes.fn.any?}

:   

*fn*{.property} `as-converter`{.descname} (*&ensp;vQT T static?&ensp;*)[](#scopes.fn.as-converter "Permalink to this definition"){.headerlink} {#scopes.fn.as-converter}

:   

*fn*{.property} `autoboxer`{.descname} (*&ensp;T x&ensp;*)[](#scopes.fn.autoboxer "Permalink to this definition"){.headerlink} {#scopes.fn.autoboxer}

:   

*inline*{.property} `balanced-binary-op-dispatch`{.descname} (*&ensp;symbol rsymbol friendly-op-name&ensp;*)[](#scopes.inline.balanced-binary-op-dispatch "Permalink to this definition"){.headerlink} {#scopes.inline.balanced-binary-op-dispatch}

:   

*fn*{.property} `balanced-binary-operation`{.descname} (*&ensp;args symbol rsymbol friendly-op-name&ensp;*)[](#scopes.fn.balanced-binary-operation "Permalink to this definition"){.headerlink} {#scopes.fn.balanced-binary-operation}

:   

*fn*{.property} `balanced-binary-operator`{.descname} (*&ensp;symbol rsymbol lhsT rhsT lhs-static? rhs-static?&ensp;*)[](#scopes.fn.balanced-binary-operator "Permalink to this definition"){.headerlink} {#scopes.fn.balanced-binary-operator}

:   For an operation performed on two argument types, of which either
    type can provide a suitable candidate, return a matching operator.
    This function only works inside a spice macro.

*inline*{.property} `balanced-lvalue-binary-op-dispatch`{.descname} (*&ensp;symbol friendly-op-name&ensp;*)[](#scopes.inline.balanced-lvalue-binary-op-dispatch "Permalink to this definition"){.headerlink} {#scopes.inline.balanced-lvalue-binary-op-dispatch}

:   

*fn*{.property} `balanced-lvalue-binary-operation`{.descname} (*&ensp;args symbol friendly-op-name&ensp;*)[](#scopes.fn.balanced-lvalue-binary-operation "Permalink to this definition"){.headerlink} {#scopes.fn.balanced-lvalue-binary-operation}

:   

*fn*{.property} `balanced-lvalue-binary-operator`{.descname} (*&ensp;symbol lhsT rhsT rhs-static?&ensp;*)[](#scopes.fn.balanced-lvalue-binary-operator "Permalink to this definition"){.headerlink} {#scopes.fn.balanced-lvalue-binary-operator}

:   For an operation performed on two argument types, of which only the
    left type type can provide a suitable candidate, return a matching operator.
    This function only works inside a spice macro.

*fn*{.property} `bin`{.descname} (*&ensp;value&ensp;*)[](#scopes.fn.bin "Permalink to this definition"){.headerlink} {#scopes.fn.bin}

:   

*fn*{.property} `binary-op-error`{.descname} (*&ensp;friendly-op-name lhsT rhsT&ensp;*)[](#scopes.fn.binary-op-error "Permalink to this definition"){.headerlink} {#scopes.fn.binary-op-error}

:   

*fn*{.property} `binary-operator`{.descname} (*&ensp;symbol lhsT rhsT&ensp;*)[](#scopes.fn.binary-operator "Permalink to this definition"){.headerlink} {#scopes.fn.binary-operator}

:   For an operation performed on two argument types, of which only
    the left type can provide a suitable candidate, find a matching
    operator function. This function only works inside a spice macro.

*fn*{.property} `binary-operator-r`{.descname} (*&ensp;rsymbol lhsT rhsT&ensp;*)[](#scopes.fn.binary-operator-r "Permalink to this definition"){.headerlink} {#scopes.fn.binary-operator-r}

:   For an operation performed on two argument types, of which only
    the right type can provide a suitable candidate, find a matching
    operator function. This function only works inside a spice macro.

*fn*{.property} `box-integer`{.descname} (*&ensp;value&ensp;*)[](#scopes.fn.box-integer "Permalink to this definition"){.headerlink} {#scopes.fn.box-integer}

:   

*fn*{.property} `box-pointer`{.descname} (*&ensp;value&ensp;*)[](#scopes.fn.box-pointer "Permalink to this definition"){.headerlink} {#scopes.fn.box-pointer}

:   

*inline*{.property} `box-spice-macro`{.descname} (*&ensp;l&ensp;*)[](#scopes.inline.box-spice-macro "Permalink to this definition"){.headerlink} {#scopes.inline.box-spice-macro}

:   

*fn*{.property} `box-symbol`{.descname} (*&ensp;value&ensp;*)[](#scopes.fn.box-symbol "Permalink to this definition"){.headerlink} {#scopes.fn.box-symbol}

:   

*fn*{.property} `build-typify-function`{.descname} (*&ensp;f&ensp;*)[](#scopes.fn.build-typify-function "Permalink to this definition"){.headerlink} {#scopes.fn.build-typify-function}

:   

*fn*{.property} `cast-converter`{.descname} (*&ensp;symbol rsymbol vQT T&ensp;*)[](#scopes.fn.cast-converter "Permalink to this definition"){.headerlink} {#scopes.fn.cast-converter}

:   For two given types, find a matching conversion function.
    This function only works inside a spice macro.

*inline*{.property} `cast-error`{.descname} (*&ensp;intro-string vT T&ensp;*)[](#scopes.inline.cast-error "Permalink to this definition"){.headerlink} {#scopes.inline.cast-error}

:   

*fn*{.property} `check-count`{.descname} (*&ensp;count mincount maxcount&ensp;*)[](#scopes.fn.check-count "Permalink to this definition"){.headerlink} {#scopes.fn.check-count}

:   

*inline*{.property} `clamp`{.descname} (*&ensp;x mn mx&ensp;*)[](#scopes.inline.clamp "Permalink to this definition"){.headerlink} {#scopes.inline.clamp}

:   

*fn*{.property} `clone-scope-contents`{.descname} (*&ensp;a b&ensp;*)[](#scopes.fn.clone-scope-contents "Permalink to this definition"){.headerlink} {#scopes.fn.clone-scope-contents}

:   Join two scopes `a` and `b` into a new scope so that the
    root of `a` descends from `b`.

*fn*{.property} `compare-type`{.descname} (*&ensp;args f&ensp;*)[](#scopes.fn.compare-type "Permalink to this definition"){.headerlink} {#scopes.fn.compare-type}

:   

*inline*{.property} `compile`{.descname} (*&ensp;func flags...&ensp;*)[](#scopes.inline.compile "Permalink to this definition"){.headerlink} {#scopes.inline.compile}

:   

*inline*{.property} `compile-glsl`{.descname} (*&ensp;version target func flags...&ensp;*)[](#scopes.inline.compile-glsl "Permalink to this definition"){.headerlink} {#scopes.inline.compile-glsl}

:   

*inline*{.property} `compile-object`{.descname} (*&ensp;target file-kind path table flags...&ensp;*)[](#scopes.inline.compile-object "Permalink to this definition"){.headerlink} {#scopes.inline.compile-object}

:   

*inline*{.property} `compile-spirv`{.descname} (*&ensp;version target func flags...&ensp;*)[](#scopes.inline.compile-spirv "Permalink to this definition"){.headerlink} {#scopes.inline.compile-spirv}

:   

*fn*{.property} `compiler-version-string`{.descname} ()[](#scopes.fn.compiler-version-string "Permalink to this definition"){.headerlink} {#scopes.fn.compiler-version-string}

:   

*inline*{.property} `convert-assert-args`{.descname} (*&ensp;args cond msg&ensp;*)[](#scopes.inline.convert-assert-args "Permalink to this definition"){.headerlink} {#scopes.inline.convert-assert-args}

:   

*fn*{.property} `dec`{.descname} (*&ensp;value&ensp;*)[](#scopes.fn.dec "Permalink to this definition"){.headerlink} {#scopes.fn.dec}

:   

*inline*{.property} `defer-type`{.descname} (*&ensp;...&ensp;*)[](#scopes.inline.defer-type "Permalink to this definition"){.headerlink} {#scopes.inline.defer-type}

:   

*fn*{.property} `dispatch-and-or`{.descname} (*&ensp;args flip&ensp;*)[](#scopes.fn.dispatch-and-or "Permalink to this definition"){.headerlink} {#scopes.fn.dispatch-and-or}

:   

*inline*{.property} `distance`{.descname} (*&ensp;a b&ensp;*)[](#scopes.inline.distance "Permalink to this definition"){.headerlink} {#scopes.inline.distance}

:   

*fn*{.property} `dots-to-slashes`{.descname} (*&ensp;pattern&ensp;*)[](#scopes.fn.dots-to-slashes "Permalink to this definition"){.headerlink} {#scopes.fn.dots-to-slashes}

:   

*fn*{.property} `dotted-symbol?`{.descname} (*&ensp;env head&ensp;*)[](#scopes.fn.dotted-symbol? "Permalink to this definition"){.headerlink} {#scopes.fn.dotted-symbol?}

:   

*inline*{.property} `empty?`{.descname} (*&ensp;value&ensp;*)[](#scopes.inline.empty? "Permalink to this definition"){.headerlink} {#scopes.inline.empty?}

:   

*inline*{.property} `enumerate`{.descname} (*&ensp;x T&ensp;*)[](#scopes.inline.enumerate "Permalink to this definition"){.headerlink} {#scopes.inline.enumerate}

:   

*fn*{.property} `error`{.descname} (*&ensp;msg&ensp;*)[](#scopes.fn.error "Permalink to this definition"){.headerlink} {#scopes.fn.error}

:   

*fn*{.property} `error@`{.descname} (*&ensp;anchor traceback-msg error-msg&ensp;*)[](#scopes.fn.error@ "Permalink to this definition"){.headerlink} {#scopes.fn.error@}

:   Usage example:
    
        :::scopes
        error@ ('anchor value) "while checking parameter" "error in value"

*fn*{.property} `error@+`{.descname} (*&ensp;error anchor traceback-msg&ensp;*)[](#scopes.fn.error@+ "Permalink to this definition"){.headerlink} {#scopes.fn.error@+}

:   Usage example:
    
        :::scopes
        except (err)
            error@+ err ('anchor value) "while processing stream"

*fn*{.property} `exec-module`{.descname} (*&ensp;expr eval-scope&ensp;*)[](#scopes.fn.exec-module "Permalink to this definition"){.headerlink} {#scopes.fn.exec-module}

:   

*fn*{.property} `expand-and-or`{.descname} (*&ensp;expr f&ensp;*)[](#scopes.fn.expand-and-or "Permalink to this definition"){.headerlink} {#scopes.fn.expand-and-or}

:   

*fn*{.property} `expand-apply`{.descname} (*&ensp;expr&ensp;*)[](#scopes.fn.expand-apply "Permalink to this definition"){.headerlink} {#scopes.fn.expand-apply}

:   

*fn*{.property} `expand-define`{.descname} (*&ensp;expr&ensp;*)[](#scopes.fn.expand-define "Permalink to this definition"){.headerlink} {#scopes.fn.expand-define}

:   

*fn*{.property} `expand-define-infix`{.descname} (*&ensp;args scope order&ensp;*)[](#scopes.fn.expand-define-infix "Permalink to this definition"){.headerlink} {#scopes.fn.expand-define-infix}

:   

*fn*{.property} `expand-infix-let`{.descname} (*&ensp;expr&ensp;*)[](#scopes.fn.expand-infix-let "Permalink to this definition"){.headerlink} {#scopes.fn.expand-infix-let}

:   

*inline*{.property} `extern-new`{.descname} (*&ensp;name T attrs...&ensp;*)[](#scopes.inline.extern-new "Permalink to this definition"){.headerlink} {#scopes.inline.extern-new}

:   

*fn*{.property} `extract-integer`{.descname} (*&ensp;value&ensp;*)[](#scopes.fn.extract-integer "Permalink to this definition"){.headerlink} {#scopes.fn.extract-integer}

:   

*fn*{.property} `extract-name-params-body`{.descname} (*&ensp;expr&ensp;*)[](#scopes.fn.extract-name-params-body "Permalink to this definition"){.headerlink} {#scopes.fn.extract-name-params-body}

:   

*fn*{.property} `extract-single-arg`{.descname} (*&ensp;args&ensp;*)[](#scopes.fn.extract-single-arg "Permalink to this definition"){.headerlink} {#scopes.fn.extract-single-arg}

:   

*fn*{.property} `extract-single-type-arg`{.descname} (*&ensp;args&ensp;*)[](#scopes.fn.extract-single-type-arg "Permalink to this definition"){.headerlink} {#scopes.fn.extract-single-type-arg}

:   

*fn*{.property} `find-library`{.descname} (*&ensp;name library-search-path&ensp;*)[](#scopes.fn.find-library "Permalink to this definition"){.headerlink} {#scopes.fn.find-library}

:   

*fn*{.property} `find-module-path`{.descname} (*&ensp;base-dir name env&ensp;*)[](#scopes.fn.find-module-path "Permalink to this definition"){.headerlink} {#scopes.fn.find-module-path}

:   

*inline*{.property} `floor`{.descname} (*&ensp;x&ensp;*)[](#scopes.inline.floor "Permalink to this definition"){.headerlink} {#scopes.inline.floor}

:   

*inline*{.property} `function->SpiceMacro`{.descname} (*&ensp;f&ensp;*)[](#scopes.inline.function->SpiceMacro "Permalink to this definition"){.headerlink} {#scopes.inline.function->SpiceMacro}

:   

*inline*{.property} `gen-allocator-sugar`{.descname} (*&ensp;copyf newf&ensp;*)[](#scopes.inline.gen-allocator-sugar "Permalink to this definition"){.headerlink} {#scopes.inline.gen-allocator-sugar}

:   

*inline*{.property} `gen-cast-op`{.descname} (*&ensp;f str&ensp;*)[](#scopes.inline.gen-cast-op "Permalink to this definition"){.headerlink} {#scopes.inline.gen-cast-op}

:   

*inline*{.property} `gen-cast?`{.descname} (*&ensp;converterf&ensp;*)[](#scopes.inline.gen-cast? "Permalink to this definition"){.headerlink} {#scopes.inline.gen-cast?}

:   

*inline*{.property} `gen-match-block-parser`{.descname} (*&ensp;handle-case&ensp;*)[](#scopes.inline.gen-match-block-parser "Permalink to this definition"){.headerlink} {#scopes.inline.gen-match-block-parser}

:   

*fn*{.property} `gen-match-matcher`{.descname} (*&ensp;failfunc expr scope cond&ensp;*)[](#scopes.fn.gen-match-matcher "Permalink to this definition"){.headerlink} {#scopes.fn.gen-match-matcher}

:   features:
    <constant> -> (input == <constant>)
    (or <expr_a> <expr_b>) -> (or <expr_a> <expr_b>)
    
    TODO:
    (: x T) -> ((typeof input) == T), let x = input
    <unknown symbol> -> unpack as symbol

*fn*{.property} `gen-or-matcher`{.descname} (*&ensp;failfunc expr scope params&ensp;*)[](#scopes.fn.gen-or-matcher "Permalink to this definition"){.headerlink} {#scopes.fn.gen-or-matcher}

:   

*fn*{.property} `gen-sugar-matcher`{.descname} (*&ensp;failfunc expr scope params&ensp;*)[](#scopes.fn.gen-sugar-matcher "Permalink to this definition"){.headerlink} {#scopes.fn.gen-sugar-matcher}

:   

*fn*{.property} `gen-vector-reduction`{.descname} (*&ensp;f v sz&ensp;*)[](#scopes.fn.gen-vector-reduction "Permalink to this definition"){.headerlink} {#scopes.fn.gen-vector-reduction}

:   

*fn*{.property} `get-ifx-op`{.descname} (*&ensp;env op&ensp;*)[](#scopes.fn.get-ifx-op "Permalink to this definition"){.headerlink} {#scopes.fn.get-ifx-op}

:   

*fn*{.property} `get-ifx-symbol`{.descname} (*&ensp;name&ensp;*)[](#scopes.fn.get-ifx-symbol "Permalink to this definition"){.headerlink} {#scopes.fn.get-ifx-symbol}

:   

*fn*{.property} `has-infix-ops?`{.descname} (*&ensp;infix-table expr&ensp;*)[](#scopes.fn.has-infix-ops? "Permalink to this definition"){.headerlink} {#scopes.fn.has-infix-ops?}

:   

*fn*{.property} `hex`{.descname} (*&ensp;value&ensp;*)[](#scopes.fn.hex "Permalink to this definition"){.headerlink} {#scopes.fn.hex}

:   

*fn*{.property} `imply-converter`{.descname} (*&ensp;vQT T static?&ensp;*)[](#scopes.fn.imply-converter "Permalink to this definition"){.headerlink} {#scopes.fn.imply-converter}

:   

*inline*{.property} `infinite-range`{.descname} (*&ensp;T&ensp;*)[](#scopes.inline.infinite-range "Permalink to this definition"){.headerlink} {#scopes.inline.infinite-range}

:   A `Generator` that iterates through all integer values starting at 0. This
    generator never terminates; when it exceeds the maximum integer value, it
    overflows and continues with the minimum integer value of that type.

*inline*{.property} `infix-op`{.descname} (*&ensp;pred&ensp;*)[](#scopes.inline.infix-op "Permalink to this definition"){.headerlink} {#scopes.inline.infix-op}

:   

*fn*{.property} `infix-op-ge`{.descname} (*&ensp;infix-table token prec&ensp;*)[](#scopes.fn.infix-op-ge "Permalink to this definition"){.headerlink} {#scopes.fn.infix-op-ge}

:   

*fn*{.property} `infix-op-gt`{.descname} (*&ensp;infix-table token prec&ensp;*)[](#scopes.fn.infix-op-gt "Permalink to this definition"){.headerlink} {#scopes.fn.infix-op-gt}

:   

*inline*{.property} `intdiv`{.descname} (*&ensp;a b&ensp;*)[](#scopes.inline.intdiv "Permalink to this definition"){.headerlink} {#scopes.inline.intdiv}

:   

*fn*{.property} `integer->string`{.descname} (*&ensp;value base&ensp;*)[](#scopes.fn.integer->string "Permalink to this definition"){.headerlink} {#scopes.fn.integer->string}

:   

*fn*{.property} `integer-as`{.descname} (*&ensp;vT T&ensp;*)[](#scopes.fn.integer-as "Permalink to this definition"){.headerlink} {#scopes.fn.integer-as}

:   

*fn*{.property} `integer-imply`{.descname} (*&ensp;vT T&ensp;*)[](#scopes.fn.integer-imply "Permalink to this definition"){.headerlink} {#scopes.fn.integer-imply}

:   

*fn*{.property} `integer-static-imply`{.descname} (*&ensp;vT T&ensp;*)[](#scopes.fn.integer-static-imply "Permalink to this definition"){.headerlink} {#scopes.fn.integer-static-imply}

:   

*fn*{.property} `integer-tobool`{.descname} (*&ensp;args&ensp;*)[](#scopes.fn.integer-tobool "Permalink to this definition"){.headerlink} {#scopes.fn.integer-tobool}

:   

*fn*{.property} `load-module`{.descname} (*&ensp;module-name module-path env opts...&ensp;*)[](#scopes.fn.load-module "Permalink to this definition"){.headerlink} {#scopes.fn.load-module}

:   

*fn*{.property} `ltr-multiop`{.descname} (*&ensp;args target mincount&ensp;*)[](#scopes.fn.ltr-multiop "Permalink to this definition"){.headerlink} {#scopes.fn.ltr-multiop}

:   

*inline*{.property} `make-const-type-property-function`{.descname} (*&ensp;func&ensp;*)[](#scopes.inline.make-const-type-property-function "Permalink to this definition"){.headerlink} {#scopes.inline.make-const-type-property-function}

:   

*inline*{.property} `make-const-value-property-function`{.descname} (*&ensp;func&ensp;*)[](#scopes.inline.make-const-value-property-function "Permalink to this definition"){.headerlink} {#scopes.inline.make-const-value-property-function}

:   

*inline*{.property} `make-expand-and-or`{.descname} (*&ensp;f&ensp;*)[](#scopes.inline.make-expand-and-or "Permalink to this definition"){.headerlink} {#scopes.inline.make-expand-and-or}

:   

*inline*{.property} `make-expand-define-infix`{.descname} (*&ensp;order&ensp;*)[](#scopes.inline.make-expand-define-infix "Permalink to this definition"){.headerlink} {#scopes.inline.make-expand-define-infix}

:   

*inline*{.property} `make-inplace-let-op`{.descname} (*&ensp;op&ensp;*)[](#scopes.inline.make-inplace-let-op "Permalink to this definition"){.headerlink} {#scopes.inline.make-inplace-let-op}

:   

*inline*{.property} `make-inplace-op`{.descname} (*&ensp;op&ensp;*)[](#scopes.inline.make-inplace-op "Permalink to this definition"){.headerlink} {#scopes.inline.make-inplace-op}

:   

*fn*{.property} `make-module-path`{.descname} (*&ensp;pattern name&ensp;*)[](#scopes.fn.make-module-path "Permalink to this definition"){.headerlink} {#scopes.fn.make-module-path}

:   

*inline*{.property} `make-unpack-function`{.descname} (*&ensp;extractf&ensp;*)[](#scopes.inline.make-unpack-function "Permalink to this definition"){.headerlink} {#scopes.inline.make-unpack-function}

:   

*inline*{.property} `memo`{.descname} (*&ensp;f&ensp;*)[](#scopes.inline.memo "Permalink to this definition"){.headerlink} {#scopes.inline.memo}

:   

*inline*{.property} `memoize`{.descname} (*&ensp;f&ensp;*)[](#scopes.inline.memoize "Permalink to this definition"){.headerlink} {#scopes.inline.memoize}

:   

*fn*{.property} `merge-scope-symbols`{.descname} (*&ensp;source target filter&ensp;*)[](#scopes.fn.merge-scope-symbols "Permalink to this definition"){.headerlink} {#scopes.fn.merge-scope-symbols}

:   

*fn*{.property} `mod`{.descname} (*&ensp;a b&ensp;*)[](#scopes.fn.mod "Permalink to this definition"){.headerlink} {#scopes.fn.mod}

:   

*fn*{.property} `next-head?`{.descname} (*&ensp;next&ensp;*)[](#scopes.fn.next-head? "Permalink to this definition"){.headerlink} {#scopes.fn.next-head?}

:   

*fn*{.property} `nodefault?`{.descname} (*&ensp;x&ensp;*)[](#scopes.fn.nodefault? "Permalink to this definition"){.headerlink} {#scopes.fn.nodefault?}

:   

*fn*{.property} `oct`{.descname} (*&ensp;value&ensp;*)[](#scopes.fn.oct "Permalink to this definition"){.headerlink} {#scopes.fn.oct}

:   

*fn*{.property} `operator-valid?`{.descname} (*&ensp;value&ensp;*)[](#scopes.fn.operator-valid? "Permalink to this definition"){.headerlink} {#scopes.fn.operator-valid?}

:   

*fn*{.property} `patterns-from-namestr`{.descname} (*&ensp;base-dir namestr env&ensp;*)[](#scopes.fn.patterns-from-namestr "Permalink to this definition"){.headerlink} {#scopes.fn.patterns-from-namestr}

:   

*fn*{.property} `pointer-as`{.descname} (*&ensp;vT T&ensp;*)[](#scopes.fn.pointer-as "Permalink to this definition"){.headerlink} {#scopes.fn.pointer-as}

:   

*fn*{.property} `pointer-imply`{.descname} (*&ensp;vT T&ensp;*)[](#scopes.fn.pointer-imply "Permalink to this definition"){.headerlink} {#scopes.fn.pointer-imply}

:   

*fn*{.property} `pointer-ras`{.descname} (*&ensp;T vT&ensp;*)[](#scopes.fn.pointer-ras "Permalink to this definition"){.headerlink} {#scopes.fn.pointer-ras}

:   

*fn*{.property} `pointer-type-imply?`{.descname} (*&ensp;src dest&ensp;*)[](#scopes.fn.pointer-type-imply? "Permalink to this definition"){.headerlink} {#scopes.fn.pointer-type-imply?}

:   

*fn*{.property} `powi`{.descname} (*&ensp;base exponent&ensp;*)[](#scopes.fn.powi "Permalink to this definition"){.headerlink} {#scopes.fn.powi}

:   

*inline*{.property} `print`{.descname} (*&ensp;values...&ensp;*)[](#scopes.inline.print "Permalink to this definition"){.headerlink} {#scopes.inline.print}

:   

*fn*{.property} `print-logo`{.descname} ()[](#scopes.fn.print-logo "Permalink to this definition"){.headerlink} {#scopes.fn.print-logo}

:   

*fn*{.property} `ptrcmp!=`{.descname} (*&ensp;t1 t2&ensp;*)[](#scopes.fn.ptrcmp!= "Permalink to this definition"){.headerlink} {#scopes.fn.ptrcmp!=}

:   

*fn*{.property} `ptrcmp==`{.descname} (*&ensp;t1 t2&ensp;*)[](#scopes.fn.ptrcmp== "Permalink to this definition"){.headerlink} {#scopes.fn.ptrcmp==}

:   

*inline*{.property} `quasiquote-any`{.descname} (*&ensp;x&ensp;*)[](#scopes.inline.quasiquote-any "Permalink to this definition"){.headerlink} {#scopes.inline.quasiquote-any}

:   

*fn*{.property} `quasiquote-list`{.descname} (*&ensp;x&ensp;*)[](#scopes.fn.quasiquote-list "Permalink to this definition"){.headerlink} {#scopes.fn.quasiquote-list}

:   

*inline*{.property} `range`{.descname} (*&ensp;a b c&ensp;*)[](#scopes.inline.range "Permalink to this definition"){.headerlink} {#scopes.inline.range}

:   

*fn*{.property} `real-as`{.descname} (*&ensp;vT T&ensp;*)[](#scopes.fn.real-as "Permalink to this definition"){.headerlink} {#scopes.fn.real-as}

:   

*fn*{.property} `real-imply`{.descname} (*&ensp;vT T&ensp;*)[](#scopes.fn.real-imply "Permalink to this definition"){.headerlink} {#scopes.fn.real-imply}

:   

*fn*{.property} `require-from`{.descname} (*&ensp;base-dir name env&ensp;*)[](#scopes.fn.require-from "Permalink to this definition"){.headerlink} {#scopes.fn.require-from}

:   

*inline*{.property} `rrange`{.descname} (*&ensp;a b c&ensp;*)[](#scopes.inline.rrange "Permalink to this definition"){.headerlink} {#scopes.inline.rrange}

:   Same as `range`, but iterates range in reverse; arguments are passed
    in the same format, so `rrange` can act as a drop-in replacement for
    `range`.

*fn*{.property} `rtl-infix-op-eq`{.descname} (*&ensp;infix-table token prec&ensp;*)[](#scopes.fn.rtl-infix-op-eq "Permalink to this definition"){.headerlink} {#scopes.fn.rtl-infix-op-eq}

:   

*fn*{.property} `rtl-multiop`{.descname} (*&ensp;args target mincount&ensp;*)[](#scopes.fn.rtl-multiop "Permalink to this definition"){.headerlink} {#scopes.fn.rtl-multiop}

:   

*inline*{.property} `runtime-aggregate-type-constructor`{.descname} (*&ensp;f&ensp;*)[](#scopes.inline.runtime-aggregate-type-constructor "Permalink to this definition"){.headerlink} {#scopes.inline.runtime-aggregate-type-constructor}

:   

*inline*{.property} `safe-integer-cast`{.descname} (*&ensp;self T&ensp;*)[](#scopes.inline.safe-integer-cast "Permalink to this definition"){.headerlink} {#scopes.inline.safe-integer-cast}

:   

*fn*{.property} `sc_argument_list_join`{.descname} (*&ensp;a b&ensp;*)[](#scopes.fn.sc_argument_list_join "Permalink to this definition"){.headerlink} {#scopes.fn.sc_argument_list_join}

:   

*inline*{.property} `sc_argument_list_join_values`{.descname} (*&ensp;a b...&ensp;*)[](#scopes.inline.sc_argument_list_join_values "Permalink to this definition"){.headerlink} {#scopes.inline.sc_argument_list_join_values}

:   

*inline*{.property} `sc_argument_list_map_filter_new`{.descname} (*&ensp;maxN mapf&ensp;*)[](#scopes.inline.sc_argument_list_map_filter_new "Permalink to this definition"){.headerlink} {#scopes.inline.sc_argument_list_map_filter_new}

:   

*inline*{.property} `sc_argument_list_map_new`{.descname} (*&ensp;N mapf&ensp;*)[](#scopes.inline.sc_argument_list_map_new "Permalink to this definition"){.headerlink} {#scopes.inline.sc_argument_list_map_new}

:   

*inline*{.property} `select-op-macro`{.descname} (*&ensp;sop uop fop numargs&ensp;*)[](#scopes.inline.select-op-macro "Permalink to this definition"){.headerlink} {#scopes.inline.select-op-macro}

:   

*inline*{.property} `signed-vector-binary-op`{.descname} (*&ensp;sf uf&ensp;*)[](#scopes.inline.signed-vector-binary-op "Permalink to this definition"){.headerlink} {#scopes.inline.signed-vector-binary-op}

:   

*inline*{.property} `simple-binary-op`{.descname} (*&ensp;f&ensp;*)[](#scopes.inline.simple-binary-op "Permalink to this definition"){.headerlink} {#scopes.inline.simple-binary-op}

:   For cases where the type only interacts with itself.

*inline*{.property} `simple-folding-autotype-binary-op`{.descname} (*&ensp;f unboxer&ensp;*)[](#scopes.inline.simple-folding-autotype-binary-op "Permalink to this definition"){.headerlink} {#scopes.inline.simple-folding-autotype-binary-op}

:   

*inline*{.property} `simple-folding-autotype-signed-binary-op`{.descname} (*&ensp;sf uf unboxer&ensp;*)[](#scopes.inline.simple-folding-autotype-signed-binary-op "Permalink to this definition"){.headerlink} {#scopes.inline.simple-folding-autotype-signed-binary-op}

:   

*inline*{.property} `simple-folding-binary-op`{.descname} (*&ensp;f unboxer boxer&ensp;*)[](#scopes.inline.simple-folding-binary-op "Permalink to this definition"){.headerlink} {#scopes.inline.simple-folding-binary-op}

:   

*inline*{.property} `simple-folding-signed-binary-op`{.descname} (*&ensp;sf uf unboxer boxer&ensp;*)[](#scopes.inline.simple-folding-signed-binary-op "Permalink to this definition"){.headerlink} {#scopes.inline.simple-folding-signed-binary-op}

:   

*inline*{.property} `simple-folding-unary-op`{.descname} (*&ensp;f unboxer boxer&ensp;*)[](#scopes.inline.simple-folding-unary-op "Permalink to this definition"){.headerlink} {#scopes.inline.simple-folding-unary-op}

:   

*inline*{.property} `simple-signed-binary-op`{.descname} (*&ensp;sf uf&ensp;*)[](#scopes.inline.simple-signed-binary-op "Permalink to this definition"){.headerlink} {#scopes.inline.simple-signed-binary-op}

:   

*inline*{.property} `slice`{.descname} (*&ensp;value start end&ensp;*)[](#scopes.inline.slice "Permalink to this definition"){.headerlink} {#scopes.inline.slice}

:   

*inline*{.property} `spice-binary-op-macro`{.descname} (*&ensp;f&ensp;*)[](#scopes.inline.spice-binary-op-macro "Permalink to this definition"){.headerlink} {#scopes.inline.spice-binary-op-macro}

:   To be used for binary operators of which either type can provide an
    operation. Returns a callable operator `(f lhs rhs)` that performs the
    operation or no arguments if the operation can not be performed.

*inline*{.property} `spice-cast-macro`{.descname} (*&ensp;f&ensp;*)[](#scopes.inline.spice-cast-macro "Permalink to this definition"){.headerlink} {#scopes.inline.spice-cast-macro}

:   To be used for __as, __ras, __imply and __rimply
    returns a callable converter (f value) that performs the cast or
    no arguments if the cast can not be performed.

*inline*{.property} `spice-converter-macro`{.descname} (*&ensp;f&ensp;*)[](#scopes.inline.spice-converter-macro "Permalink to this definition"){.headerlink} {#scopes.inline.spice-converter-macro}

:   To be used for a converter that needs to do additional
    dispatch (i.e. do something else when the value is a constant).
    Returns a quote that performs the cast (f value T).

*inline*{.property} `spice-macro`{.descname} (*&ensp;l&ensp;*)[](#scopes.inline.spice-macro "Permalink to this definition"){.headerlink} {#scopes.inline.spice-macro}

:   

*fn*{.property} `split-dotted-symbol`{.descname} (*&ensp;env name&ensp;*)[](#scopes.fn.split-dotted-symbol "Permalink to this definition"){.headerlink} {#scopes.fn.split-dotted-symbol}

:   

*inline*{.property} `static-compile`{.descname} (*&ensp;func flags...&ensp;*)[](#scopes.inline.static-compile "Permalink to this definition"){.headerlink} {#scopes.inline.static-compile}

:   

*inline*{.property} `static-compile-glsl`{.descname} (*&ensp;version target func flags...&ensp;*)[](#scopes.inline.static-compile-glsl "Permalink to this definition"){.headerlink} {#scopes.inline.static-compile-glsl}

:   

*inline*{.property} `static-compile-spirv`{.descname} (*&ensp;version target func flags...&ensp;*)[](#scopes.inline.static-compile-spirv "Permalink to this definition"){.headerlink} {#scopes.inline.static-compile-spirv}

:   

*fn*{.property} `string@`{.descname} (*&ensp;self i&ensp;*)[](#scopes.fn.string@ "Permalink to this definition"){.headerlink} {#scopes.fn.string@}

:   

*inline*{.property} `sugar-block-scope-macro`{.descname} (*&ensp;f&ensp;*)[](#scopes.inline.sugar-block-scope-macro "Permalink to this definition"){.headerlink} {#scopes.inline.sugar-block-scope-macro}

:   

*inline*{.property} `sugar-macro`{.descname} (*&ensp;f&ensp;*)[](#scopes.inline.sugar-macro "Permalink to this definition"){.headerlink} {#scopes.inline.sugar-macro}

:   

*inline*{.property} `sugar-scope-macro`{.descname} (*&ensp;f&ensp;*)[](#scopes.inline.sugar-scope-macro "Permalink to this definition"){.headerlink} {#scopes.inline.sugar-scope-macro}

:   

*inline*{.property} `swap`{.descname} (*&ensp;a b&ensp;*)[](#scopes.inline.swap "Permalink to this definition"){.headerlink} {#scopes.inline.swap}

:   Safely exchanges the contents of two references.

*inline*{.property} `type-comparison-func`{.descname} (*&ensp;f&ensp;*)[](#scopes.inline.type-comparison-func "Permalink to this definition"){.headerlink} {#scopes.inline.type-comparison-func}

:   

*inline*{.property} `type-factory`{.descname} (*&ensp;f&ensp;*)[](#scopes.inline.type-factory "Permalink to this definition"){.headerlink} {#scopes.inline.type-factory}

:   

*inline*{.property} `typeinit`{.descname} (*&ensp;...&ensp;*)[](#scopes.inline.typeinit "Permalink to this definition"){.headerlink} {#scopes.inline.typeinit}

:   

*inline*{.property} `typematcher`{.descname} (*&ensp;...&ensp;*)[](#scopes.inline.typematcher "Permalink to this definition"){.headerlink} {#scopes.inline.typematcher}

:   

*inline*{.property} `unary-op-dispatch`{.descname} (*&ensp;symbol friendly-op-name&ensp;*)[](#scopes.inline.unary-op-dispatch "Permalink to this definition"){.headerlink} {#scopes.inline.unary-op-dispatch}

:   

*fn*{.property} `unary-op-error`{.descname} (*&ensp;friendly-op-name T&ensp;*)[](#scopes.fn.unary-op-error "Permalink to this definition"){.headerlink} {#scopes.fn.unary-op-error}

:   

*fn*{.property} `unary-operation`{.descname} (*&ensp;args symbol friendly-op-name&ensp;*)[](#scopes.fn.unary-operation "Permalink to this definition"){.headerlink} {#scopes.fn.unary-operation}

:   

*fn*{.property} `unary-operator`{.descname} (*&ensp;symbol T&ensp;*)[](#scopes.fn.unary-operator "Permalink to this definition"){.headerlink} {#scopes.fn.unary-operator}

:   For an operation performed on one variable argument type, find a
    matching operator function. This function only works inside a spice
    macro.

*inline*{.property} `unary-or-balanced-binary-op-dispatch`{.descname} (*&ensp;usymbol ufriendly-op-name symbol rsymbol friendly-op-name&ensp;*)[](#scopes.inline.unary-or-balanced-binary-op-dispatch "Permalink to this definition"){.headerlink} {#scopes.inline.unary-or-balanced-binary-op-dispatch}

:   

*fn*{.property} `unary-or-balanced-binary-operation`{.descname} (*&ensp;args usymbol ufriendly-op-name symbol rsymbol friendly-op-name&ensp;*)[](#scopes.fn.unary-or-balanced-binary-operation "Permalink to this definition"){.headerlink} {#scopes.fn.unary-or-balanced-binary-operation}

:   

*inline*{.property} `unary-or-unbalanced-binary-op-dispatch`{.descname} (*&ensp;usymbol ufriendly-op-name symbol rtype friendly-op-name&ensp;*)[](#scopes.inline.unary-or-unbalanced-binary-op-dispatch "Permalink to this definition"){.headerlink} {#scopes.inline.unary-or-unbalanced-binary-op-dispatch}

:   

*fn*{.property} `unary-or-unbalanced-binary-operation`{.descname} (*&ensp;args usymbol ufriendly-op-name symbol rtype friendly-op-name&ensp;*)[](#scopes.fn.unary-or-unbalanced-binary-operation "Permalink to this definition"){.headerlink} {#scopes.fn.unary-or-unbalanced-binary-operation}

:   

*inline*{.property} `unbalanced-binary-op-dispatch`{.descname} (*&ensp;symbol rtype friendly-op-name&ensp;*)[](#scopes.inline.unbalanced-binary-op-dispatch "Permalink to this definition"){.headerlink} {#scopes.inline.unbalanced-binary-op-dispatch}

:   

*fn*{.property} `unbalanced-binary-operation`{.descname} (*&ensp;args symbol rtype friendly-op-name&ensp;*)[](#scopes.fn.unbalanced-binary-operation "Permalink to this definition"){.headerlink} {#scopes.fn.unbalanced-binary-operation}

:   

*inline*{.property} `unbox`{.descname} (*&ensp;value T&ensp;*)[](#scopes.inline.unbox "Permalink to this definition"){.headerlink} {#scopes.inline.unbox}

:   

*inline*{.property} `unbox-integer`{.descname} (*&ensp;value T&ensp;*)[](#scopes.inline.unbox-integer "Permalink to this definition"){.headerlink} {#scopes.inline.unbox-integer}

:   

*inline*{.property} `unbox-pointer`{.descname} (*&ensp;value T&ensp;*)[](#scopes.inline.unbox-pointer "Permalink to this definition"){.headerlink} {#scopes.inline.unbox-pointer}

:   

*inline*{.property} `unbox-symbol`{.descname} (*&ensp;value T&ensp;*)[](#scopes.inline.unbox-symbol "Permalink to this definition"){.headerlink} {#scopes.inline.unbox-symbol}

:   

*fn*{.property} `unbox-verify`{.descname} (*&ensp;value wantT&ensp;*)[](#scopes.fn.unbox-verify "Permalink to this definition"){.headerlink} {#scopes.fn.unbox-verify}

:   

*fn*{.property} `uncomma`{.descname} (*&ensp;l&ensp;*)[](#scopes.fn.uncomma "Permalink to this definition"){.headerlink} {#scopes.fn.uncomma}

:   uncomma list l, wrapping all comma separated symbols as new lists.
    
    Usage example:
    
        :::scopes
        (uncomma '(a , b c d , e f , g h)) -> '(a (b c d) (e f) (g h))

*fn*{.property} `unpack-infix-op`{.descname} (*&ensp;op&ensp;*)[](#scopes.fn.unpack-infix-op "Permalink to this definition"){.headerlink} {#scopes.fn.unpack-infix-op}

:   

*fn*{.property} `unpack2`{.descname} (*&ensp;args&ensp;*)[](#scopes.fn.unpack2 "Permalink to this definition"){.headerlink} {#scopes.fn.unpack2}

:   

*inline*{.property} `va-join`{.descname} (*&ensp;a...&ensp;*)[](#scopes.inline.va-join "Permalink to this definition"){.headerlink} {#scopes.inline.va-join}

:   

*fn*{.property} `value-as`{.descname} (*&ensp;vT T expr&ensp;*)[](#scopes.fn.value-as "Permalink to this definition"){.headerlink} {#scopes.fn.value-as}

:   

*inline*{.property} `vector-binary-op-dispatch`{.descname} (*&ensp;symbol&ensp;*)[](#scopes.inline.vector-binary-op-dispatch "Permalink to this definition"){.headerlink} {#scopes.inline.vector-binary-op-dispatch}

:   

*fn*{.property} `vector-binary-operator`{.descname} (*&ensp;symbol lhsT rhsT&ensp;*)[](#scopes.fn.vector-binary-operator "Permalink to this definition"){.headerlink} {#scopes.fn.vector-binary-operator}

:   

*fn*{.property} `verify-count`{.descname} (*&ensp;count mincount maxcount&ensp;*)[](#scopes.fn.verify-count "Permalink to this definition"){.headerlink} {#scopes.fn.verify-count}

:   

*sugar*{.property} (`.`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.. "Permalink to this definition"){.headerlink} {#scopes.sugar..}

:   

*sugar*{.property} (`::`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.:: "Permalink to this definition"){.headerlink} {#scopes.sugar.::}

:   

*sugar*{.property} (`:=`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.:= "Permalink to this definition"){.headerlink} {#scopes.sugar.:=}

:   

*sugar*{.property} (`<-`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.<- "Permalink to this definition"){.headerlink} {#scopes.sugar.<-}

:   

*sugar*{.property} (`<-:`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.<-: "Permalink to this definition"){.headerlink} {#scopes.sugar.<-:}

:   

*sugar*{.property} (`@@`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.@@ "Permalink to this definition"){.headerlink} {#scopes.sugar.@@}

:   

*sugar*{.property} (`and`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.and "Permalink to this definition"){.headerlink} {#scopes.sugar.and}

:   

*sugar*{.property} (`as:=`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.as:= "Permalink to this definition"){.headerlink} {#scopes.sugar.as:=}

:   

*sugar*{.property} (`assert`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.assert "Permalink to this definition"){.headerlink} {#scopes.sugar.assert}

:   

*sugar*{.property} (`bind`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.bind "Permalink to this definition"){.headerlink} {#scopes.sugar.bind}

:   

*sugar*{.property} (`chain-typed-symbol-handler`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.chain-typed-symbol-handler "Permalink to this definition"){.headerlink} {#scopes.sugar.chain-typed-symbol-handler}

:   

*sugar*{.property} (`decorate-fn`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.decorate-fn "Permalink to this definition"){.headerlink} {#scopes.sugar.decorate-fn}

:   

*sugar*{.property} (`decorate-inline`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.decorate-inline "Permalink to this definition"){.headerlink} {#scopes.sugar.decorate-inline}

:   

*sugar*{.property} (`decorate-let`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.decorate-let "Permalink to this definition"){.headerlink} {#scopes.sugar.decorate-let}

:   

*sugar*{.property} (`decorate-struct`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.decorate-struct "Permalink to this definition"){.headerlink} {#scopes.sugar.decorate-struct}

:   

*sugar*{.property} (`decorate-type`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.decorate-type "Permalink to this definition"){.headerlink} {#scopes.sugar.decorate-type}

:   

*sugar*{.property} (`decorate-typedef`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.decorate-typedef "Permalink to this definition"){.headerlink} {#scopes.sugar.decorate-typedef}

:   

*sugar*{.property} (`decorate-vvv`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.decorate-vvv "Permalink to this definition"){.headerlink} {#scopes.sugar.decorate-vvv}

:   

*sugar*{.property} (`define`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.define "Permalink to this definition"){.headerlink} {#scopes.sugar.define}

:   

*sugar*{.property} (`define-infix<`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.define-infix< "Permalink to this definition"){.headerlink} {#scopes.sugar.define-infix<}

:   

*sugar*{.property} (`define-infix>`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.define-infix> "Permalink to this definition"){.headerlink} {#scopes.sugar.define-infix>}

:   

*sugar*{.property} (`define-sugar-block-scope-macro`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.define-sugar-block-scope-macro "Permalink to this definition"){.headerlink} {#scopes.sugar.define-sugar-block-scope-macro}

:   

*sugar*{.property} (`define-sugar-macro`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.define-sugar-macro "Permalink to this definition"){.headerlink} {#scopes.sugar.define-sugar-macro}

:   

*sugar*{.property} (`define-sugar-scope-macro`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.define-sugar-scope-macro "Permalink to this definition"){.headerlink} {#scopes.sugar.define-sugar-scope-macro}

:   

*sugar*{.property} (`dispatch`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.dispatch "Permalink to this definition"){.headerlink} {#scopes.sugar.dispatch}

:   

*sugar*{.property} (`fn...`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.fn... "Permalink to this definition"){.headerlink} {#scopes.sugar.fn...}

:   

*sugar*{.property} (`fold `{.descname} (*&ensp;state ...&ensp;* `=`{.descname} *&ensp;init...&ensp;*) `for`{.descname} *&ensp;name ...&ensp;* `in`{.descname} *&ensp;gen body...&ensp;*) [](#scopes.sugar.fold-state-init-for-name-in-gen-body "Permalink to this definition"){.headerlink} {#scopes.sugar.fold-state-init-for-name-in-gen-body}

:   This is a combination of the `loop` and `for` forms. It enumerates all
    elements in collection or sequence `gen`, unpacking each element and
    binding its arguments to the names defined by `name ...`, while
    the loop state `state ...` is initialized from `init...`.

    Similar to `loop`, the body expression must return the next state of
    the loop. The state of `gen` is transparently maintained and does not
    have to be managed.

    Unlike `for`, `fold` requires calls to `break` to pass a state
    compatible with `state ...`. Otherwise they serve the same function.

    Usage example:

        :::scopes
        # add numbers from 0 to 9, skipping number 5, and print the result
        print
            fold (sum = 0) for i in (range 100)
                if (i == 10)
                    # abort the loop
                    break sum
                if (i == 5)
                    # skip this index
                    continue;
                # continue with the next state for sum
                sum + i

*sugar*{.property} (`fold-locals`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.fold-locals "Permalink to this definition"){.headerlink} {#scopes.sugar.fold-locals}

:   

*sugar*{.property} (`for`{.descname} *&ensp;name ...&ensp;* `in`{.descname} *&ensp;gen body...&ensp;*) [](#scopes.sugar.for-name-in-gen-body "Permalink to this definition"){.headerlink} {#scopes.sugar.for-name-in-gen-body}

:   Defines a loop that enumerates all elements in collection or sequence
    `gen`, unpacking each element and binding its arguments to the names
    defined by `name ...`.

    `gen` must either be of type `Generator` or provide a cast to
    `Generator`.

    Within the loop body, special forms `break` and `continue` can be used
    to abort the loop early or skip ahead to the next element. The loop
    will always evaluate to no arguments.

    For a loop form that allows maintaining additional state and break
    with a value, see `fold`.

    Usage example:

        :::scopes
        # print numbers from 0 to 9, skipping number 5
        for i in (range 100)
            if (i == 10)
                # abort the loop
                break;
            if (i == 5)
                # skip this index
                continue;
            print i

*sugar*{.property} (`from`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.from "Permalink to this definition"){.headerlink} {#scopes.sugar.from}

:   

*sugar*{.property} (`global`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.global "Permalink to this definition"){.headerlink} {#scopes.sugar.global}

:   

*sugar*{.property} (`import`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.import "Permalink to this definition"){.headerlink} {#scopes.sugar.import}

:   

*sugar*{.property} (`include`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.include "Permalink to this definition"){.headerlink} {#scopes.sugar.include}

:   

*sugar*{.property} (`inline...`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.inline... "Permalink to this definition"){.headerlink} {#scopes.sugar.inline...}

:   

*sugar*{.property} (`local`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.local "Permalink to this definition"){.headerlink} {#scopes.sugar.local}

:   declares a mutable variable on the local function stack.
    
    Syntax:
        local name : type = value
        local name = value
        local = value
        local : type = value
        local : type args...
        local type args...

*sugar*{.property} (`locals`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.locals "Permalink to this definition"){.headerlink} {#scopes.sugar.locals}

:   Export locals as a chain of up to two new scopes: a scope that contains
    all the constant values in the immediate scope, and a scope that contains
    the runtime values. If all values in the scope are constant, then the
    resulting scope will also be constant.

*sugar*{.property} (`match`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.match "Permalink to this definition"){.headerlink} {#scopes.sugar.match}

:   

*sugar*{.property} (`not`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.not "Permalink to this definition"){.headerlink} {#scopes.sugar.not}

:   

*sugar*{.property} (`or`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.or "Permalink to this definition"){.headerlink} {#scopes.sugar.or}

:   

*sugar*{.property} (`qq`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.qq "Permalink to this definition"){.headerlink} {#scopes.sugar.qq}

:   

*sugar*{.property} (`shared-library`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.shared-library "Permalink to this definition"){.headerlink} {#scopes.sugar.shared-library}

:   

*sugar*{.property} (`spice`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.spice "Permalink to this definition"){.headerlink} {#scopes.sugar.spice}

:   

*sugar*{.property} (`static-assert`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.static-assert "Permalink to this definition"){.headerlink} {#scopes.sugar.static-assert}

:   

*sugar*{.property} (`static-if`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.static-if "Permalink to this definition"){.headerlink} {#scopes.sugar.static-if}

:   

*sugar*{.property} (`static-match`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.static-match "Permalink to this definition"){.headerlink} {#scopes.sugar.static-match}

:   

*sugar*{.property} (`static-shared-library`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.static-shared-library "Permalink to this definition"){.headerlink} {#scopes.sugar.static-shared-library}

:   

*sugar*{.property} (`static-try`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.static-try "Permalink to this definition"){.headerlink} {#scopes.sugar.static-try}

:   

*sugar*{.property} (`sugar`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.sugar "Permalink to this definition"){.headerlink} {#scopes.sugar.sugar}

:   

*sugar*{.property} (`sugar-eval`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.sugar-eval "Permalink to this definition"){.headerlink} {#scopes.sugar.sugar-eval}

:   

*sugar*{.property} (`sugar-if`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.sugar-if "Permalink to this definition"){.headerlink} {#scopes.sugar.sugar-if}

:   

*sugar*{.property} (`sugar-match`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.sugar-match "Permalink to this definition"){.headerlink} {#scopes.sugar.sugar-match}

:   

*sugar*{.property} (`sugar-set-scope!`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.sugar-set-scope! "Permalink to this definition"){.headerlink} {#scopes.sugar.sugar-set-scope!}

:   

*sugar*{.property} (`type+`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.type+ "Permalink to this definition"){.headerlink} {#scopes.sugar.type+}

:   

*sugar*{.property} (`typedef`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.typedef "Permalink to this definition"){.headerlink} {#scopes.sugar.typedef}

:   A type declaration syntax; when the name is a string, the type is declared
    at runtime.

*sugar*{.property} (`typedef+`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.typedef+ "Permalink to this definition"){.headerlink} {#scopes.sugar.typedef+}

:   

*sugar*{.property} (`typematch`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.typematch "Permalink to this definition"){.headerlink} {#scopes.sugar.typematch}

:   

*sugar*{.property} (`unlet`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.unlet "Permalink to this definition"){.headerlink} {#scopes.sugar.unlet}

:   

*sugar*{.property} (`using`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.using "Permalink to this definition"){.headerlink} {#scopes.sugar.using}

:   

*sugar*{.property} (`va-option`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.va-option "Permalink to this definition"){.headerlink} {#scopes.sugar.va-option}

:   

*sugar*{.property} (`vvv`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.vvv "Permalink to this definition"){.headerlink} {#scopes.sugar.vvv}

:   

*sugar*{.property} (`while`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.while "Permalink to this definition"){.headerlink} {#scopes.sugar.while}

:   

*builtin*{.property} `?`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.? "Permalink to this definition"){.headerlink} {#scopes.builtin.?}

:   

*builtin*{.property} `_`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin._ "Permalink to this definition"){.headerlink} {#scopes.builtin._}

:   

*builtin*{.property} `Image-query-levels`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.Image-query-levels "Permalink to this definition"){.headerlink} {#scopes.builtin.Image-query-levels}

:   

*builtin*{.property} `Image-query-lod`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.Image-query-lod "Permalink to this definition"){.headerlink} {#scopes.builtin.Image-query-lod}

:   

*builtin*{.property} `Image-query-samples`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.Image-query-samples "Permalink to this definition"){.headerlink} {#scopes.builtin.Image-query-samples}

:   

*builtin*{.property} `Image-query-size`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.Image-query-size "Permalink to this definition"){.headerlink} {#scopes.builtin.Image-query-size}

:   

*builtin*{.property} `Image-read`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.Image-read "Permalink to this definition"){.headerlink} {#scopes.builtin.Image-read}

:   

*builtin*{.property} `Image-texel-pointer`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.Image-texel-pointer "Permalink to this definition"){.headerlink} {#scopes.builtin.Image-texel-pointer}

:   

*builtin*{.property} `Image-write`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.Image-write "Permalink to this definition"){.headerlink} {#scopes.builtin.Image-write}

:   

*builtin*{.property} `acos`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.acos "Permalink to this definition"){.headerlink} {#scopes.builtin.acos}

:   

*builtin*{.property} `acosh`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.acosh "Permalink to this definition"){.headerlink} {#scopes.builtin.acosh}

:   

*builtin*{.property} `add`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.add "Permalink to this definition"){.headerlink} {#scopes.builtin.add}

:   

*builtin*{.property} `add-nsw`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.add-nsw "Permalink to this definition"){.headerlink} {#scopes.builtin.add-nsw}

:   

*builtin*{.property} `add-nuw`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.add-nuw "Permalink to this definition"){.headerlink} {#scopes.builtin.add-nuw}

:   

*builtin*{.property} `alloca`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.alloca "Permalink to this definition"){.headerlink} {#scopes.builtin.alloca}

:   

*builtin*{.property} `alloca-array`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.alloca-array "Permalink to this definition"){.headerlink} {#scopes.builtin.alloca-array}

:   

*builtin*{.property} `ashr`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.ashr "Permalink to this definition"){.headerlink} {#scopes.builtin.ashr}

:   

*builtin*{.property} `asin`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.asin "Permalink to this definition"){.headerlink} {#scopes.builtin.asin}

:   

*builtin*{.property} `asinh`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.asinh "Permalink to this definition"){.headerlink} {#scopes.builtin.asinh}

:   

*builtin*{.property} `assign`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.assign "Permalink to this definition"){.headerlink} {#scopes.builtin.assign}

:   

*builtin*{.property} `atan`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.atan "Permalink to this definition"){.headerlink} {#scopes.builtin.atan}

:   

*builtin*{.property} `atan2`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.atan2 "Permalink to this definition"){.headerlink} {#scopes.builtin.atan2}

:   

*builtin*{.property} `atanh`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.atanh "Permalink to this definition"){.headerlink} {#scopes.builtin.atanh}

:   

*builtin*{.property} `atomic`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.atomic "Permalink to this definition"){.headerlink} {#scopes.builtin.atomic}

:   

*builtin*{.property} `atomicrmw`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.atomicrmw "Permalink to this definition"){.headerlink} {#scopes.builtin.atomicrmw}

:   

*builtin*{.property} `band`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.band "Permalink to this definition"){.headerlink} {#scopes.builtin.band}

:   

*builtin*{.property} `bitcast`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.bitcast "Permalink to this definition"){.headerlink} {#scopes.builtin.bitcast}

:   

*builtin*{.property} `bitcount`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.bitcount "Permalink to this definition"){.headerlink} {#scopes.builtin.bitcount}

:   

*builtin*{.property} `bitreverse`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.bitreverse "Permalink to this definition"){.headerlink} {#scopes.builtin.bitreverse}

:   

*builtin*{.property} `bnand`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.bnand "Permalink to this definition"){.headerlink} {#scopes.builtin.bnand}

:   

*builtin*{.property} `bor`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.bor "Permalink to this definition"){.headerlink} {#scopes.builtin.bor}

:   

*builtin*{.property} `branch`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.branch "Permalink to this definition"){.headerlink} {#scopes.builtin.branch}

:   

*builtin*{.property} `break`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.break "Permalink to this definition"){.headerlink} {#scopes.builtin.break}

:   

*builtin*{.property} `bxor`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.bxor "Permalink to this definition"){.headerlink} {#scopes.builtin.bxor}

:   

*builtin*{.property} `call`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.call "Permalink to this definition"){.headerlink} {#scopes.builtin.call}

:   

*builtin*{.property} `cmpxchg`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.cmpxchg "Permalink to this definition"){.headerlink} {#scopes.builtin.cmpxchg}

:   

*builtin*{.property} `cos`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.cos "Permalink to this definition"){.headerlink} {#scopes.builtin.cos}

:   

*builtin*{.property} `cosh`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.cosh "Permalink to this definition"){.headerlink} {#scopes.builtin.cosh}

:   

*builtin*{.property} `cross`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.cross "Permalink to this definition"){.headerlink} {#scopes.builtin.cross}

:   

*builtin*{.property} `degrees`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.degrees "Permalink to this definition"){.headerlink} {#scopes.builtin.degrees}

:   

*builtin*{.property} `deref`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.deref "Permalink to this definition"){.headerlink} {#scopes.builtin.deref}

:   

*builtin*{.property} `discard`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.discard "Permalink to this definition"){.headerlink} {#scopes.builtin.discard}

:   

*builtin*{.property} `do`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.do "Permalink to this definition"){.headerlink} {#scopes.builtin.do}

:   

*builtin*{.property} `dropped?`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.dropped? "Permalink to this definition"){.headerlink} {#scopes.builtin.dropped?}

:   

*builtin*{.property} `dump`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.dump "Permalink to this definition"){.headerlink} {#scopes.builtin.dump}

:   

*builtin*{.property} `dump-debug`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.dump-debug "Permalink to this definition"){.headerlink} {#scopes.builtin.dump-debug}

:   

*builtin*{.property} `dump-spice`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.dump-spice "Permalink to this definition"){.headerlink} {#scopes.builtin.dump-spice}

:   

*builtin*{.property} `dump-template`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.dump-template "Permalink to this definition"){.headerlink} {#scopes.builtin.dump-template}

:   

*builtin*{.property} `dump-uniques`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.dump-uniques "Permalink to this definition"){.headerlink} {#scopes.builtin.dump-uniques}

:   

*builtin*{.property} `dupe`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.dupe "Permalink to this definition"){.headerlink} {#scopes.builtin.dupe}

:   

*builtin*{.property} `embed`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.embed "Permalink to this definition"){.headerlink} {#scopes.builtin.embed}

:   

*builtin*{.property} `exp`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.exp "Permalink to this definition"){.headerlink} {#scopes.builtin.exp}

:   

*builtin*{.property} `exp2`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.exp2 "Permalink to this definition"){.headerlink} {#scopes.builtin.exp2}

:   

*builtin*{.property} `extractelement`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.extractelement "Permalink to this definition"){.headerlink} {#scopes.builtin.extractelement}

:   

*builtin*{.property} `extractvalue`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.extractvalue "Permalink to this definition"){.headerlink} {#scopes.builtin.extractvalue}

:   

*builtin*{.property} `fabs`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fabs "Permalink to this definition"){.headerlink} {#scopes.builtin.fabs}

:   

*builtin*{.property} `fadd`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fadd "Permalink to this definition"){.headerlink} {#scopes.builtin.fadd}

:   

*builtin*{.property} `fcmp!=o`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fcmp!=o "Permalink to this definition"){.headerlink} {#scopes.builtin.fcmp!=o}

:   

*builtin*{.property} `fcmp!=u`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fcmp!=u "Permalink to this definition"){.headerlink} {#scopes.builtin.fcmp!=u}

:   

*builtin*{.property} `fcmp-ord`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fcmp-ord "Permalink to this definition"){.headerlink} {#scopes.builtin.fcmp-ord}

:   

*builtin*{.property} `fcmp-uno`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fcmp-uno "Permalink to this definition"){.headerlink} {#scopes.builtin.fcmp-uno}

:   

*builtin*{.property} `fcmp<=o`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fcmp<=o "Permalink to this definition"){.headerlink} {#scopes.builtin.fcmp<=o}

:   

*builtin*{.property} `fcmp<=u`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fcmp<=u "Permalink to this definition"){.headerlink} {#scopes.builtin.fcmp<=u}

:   

*builtin*{.property} `fcmp<o`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fcmp<o "Permalink to this definition"){.headerlink} {#scopes.builtin.fcmp<o}

:   

*builtin*{.property} `fcmp<u`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fcmp<u "Permalink to this definition"){.headerlink} {#scopes.builtin.fcmp<u}

:   

*builtin*{.property} `fcmp==o`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fcmp==o "Permalink to this definition"){.headerlink} {#scopes.builtin.fcmp==o}

:   

*builtin*{.property} `fcmp==u`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fcmp==u "Permalink to this definition"){.headerlink} {#scopes.builtin.fcmp==u}

:   

*builtin*{.property} `fcmp>=o`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fcmp>=o "Permalink to this definition"){.headerlink} {#scopes.builtin.fcmp>=o}

:   

*builtin*{.property} `fcmp>=u`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fcmp>=u "Permalink to this definition"){.headerlink} {#scopes.builtin.fcmp>=u}

:   

*builtin*{.property} `fcmp>o`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fcmp>o "Permalink to this definition"){.headerlink} {#scopes.builtin.fcmp>o}

:   

*builtin*{.property} `fcmp>u`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fcmp>u "Permalink to this definition"){.headerlink} {#scopes.builtin.fcmp>u}

:   

*builtin*{.property} `fdiv`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fdiv "Permalink to this definition"){.headerlink} {#scopes.builtin.fdiv}

:   

*builtin*{.property} `findlsb`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.findlsb "Permalink to this definition"){.headerlink} {#scopes.builtin.findlsb}

:   

*builtin*{.property} `findmsb`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.findmsb "Permalink to this definition"){.headerlink} {#scopes.builtin.findmsb}

:   

*builtin*{.property} `fma`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fma "Permalink to this definition"){.headerlink} {#scopes.builtin.fma}

:   

*builtin*{.property} `fmix`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fmix "Permalink to this definition"){.headerlink} {#scopes.builtin.fmix}

:   

*builtin*{.property} `fmul`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fmul "Permalink to this definition"){.headerlink} {#scopes.builtin.fmul}

:   

*builtin*{.property} `fn`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fn "Permalink to this definition"){.headerlink} {#scopes.builtin.fn}

:   

*builtin*{.property} `fneg`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fneg "Permalink to this definition"){.headerlink} {#scopes.builtin.fneg}

:   

*builtin*{.property} `fpext`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fpext "Permalink to this definition"){.headerlink} {#scopes.builtin.fpext}

:   

*builtin*{.property} `fptosi`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fptosi "Permalink to this definition"){.headerlink} {#scopes.builtin.fptosi}

:   

*builtin*{.property} `fptoui`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fptoui "Permalink to this definition"){.headerlink} {#scopes.builtin.fptoui}

:   

*builtin*{.property} `fptrunc`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fptrunc "Permalink to this definition"){.headerlink} {#scopes.builtin.fptrunc}

:   

*builtin*{.property} `free`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.free "Permalink to this definition"){.headerlink} {#scopes.builtin.free}

:   

*builtin*{.property} `frem`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.frem "Permalink to this definition"){.headerlink} {#scopes.builtin.frem}

:   

*builtin*{.property} `frexp`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.frexp "Permalink to this definition"){.headerlink} {#scopes.builtin.frexp}

:   

*builtin*{.property} `fsign`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fsign "Permalink to this definition"){.headerlink} {#scopes.builtin.fsign}

:   

*builtin*{.property} `fsub`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.fsub "Permalink to this definition"){.headerlink} {#scopes.builtin.fsub}

:   

*builtin*{.property} `getelementptr`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.getelementptr "Permalink to this definition"){.headerlink} {#scopes.builtin.getelementptr}

:   

*builtin*{.property} `getelementref`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.getelementref "Permalink to this definition"){.headerlink} {#scopes.builtin.getelementref}

:   

*builtin*{.property} `hide-traceback`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.hide-traceback "Permalink to this definition"){.headerlink} {#scopes.builtin.hide-traceback}

:   

*builtin*{.property} `icmp!=`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.icmp!= "Permalink to this definition"){.headerlink} {#scopes.builtin.icmp!=}

:   

*builtin*{.property} `icmp<=s`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.icmp<=s "Permalink to this definition"){.headerlink} {#scopes.builtin.icmp<=s}

:   

*builtin*{.property} `icmp<=u`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.icmp<=u "Permalink to this definition"){.headerlink} {#scopes.builtin.icmp<=u}

:   

*builtin*{.property} `icmp<s`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.icmp<s "Permalink to this definition"){.headerlink} {#scopes.builtin.icmp<s}

:   

*builtin*{.property} `icmp<u`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.icmp<u "Permalink to this definition"){.headerlink} {#scopes.builtin.icmp<u}

:   

*builtin*{.property} `icmp==`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.icmp== "Permalink to this definition"){.headerlink} {#scopes.builtin.icmp==}

:   

*builtin*{.property} `icmp>=s`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.icmp>=s "Permalink to this definition"){.headerlink} {#scopes.builtin.icmp>=s}

:   

*builtin*{.property} `icmp>=u`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.icmp>=u "Permalink to this definition"){.headerlink} {#scopes.builtin.icmp>=u}

:   

*builtin*{.property} `icmp>s`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.icmp>s "Permalink to this definition"){.headerlink} {#scopes.builtin.icmp>s}

:   

*builtin*{.property} `icmp>u`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.icmp>u "Permalink to this definition"){.headerlink} {#scopes.builtin.icmp>u}

:   

*builtin*{.property} `if`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.if "Permalink to this definition"){.headerlink} {#scopes.builtin.if}

:   

*builtin*{.property} `indirect-let`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.indirect-let "Permalink to this definition"){.headerlink} {#scopes.builtin.indirect-let}

:   

*builtin*{.property} `inline`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.inline "Permalink to this definition"){.headerlink} {#scopes.builtin.inline}

:   

*builtin*{.property} `insertelement`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.insertelement "Permalink to this definition"){.headerlink} {#scopes.builtin.insertelement}

:   

*builtin*{.property} `insertvalue`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.insertvalue "Permalink to this definition"){.headerlink} {#scopes.builtin.insertvalue}

:   

*builtin*{.property} `inttoptr`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.inttoptr "Permalink to this definition"){.headerlink} {#scopes.builtin.inttoptr}

:   

*builtin*{.property} `inversesqrt`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.inversesqrt "Permalink to this definition"){.headerlink} {#scopes.builtin.inversesqrt}

:   

*builtin*{.property} `itrunc`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.itrunc "Permalink to this definition"){.headerlink} {#scopes.builtin.itrunc}

:   

*builtin*{.property} `label`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.label "Permalink to this definition"){.headerlink} {#scopes.builtin.label}

:   

*builtin*{.property} `ldexp`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.ldexp "Permalink to this definition"){.headerlink} {#scopes.builtin.ldexp}

:   

*builtin*{.property} `length`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.length "Permalink to this definition"){.headerlink} {#scopes.builtin.length}

:   

*builtin*{.property} `let`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.let "Permalink to this definition"){.headerlink} {#scopes.builtin.let}

:   

*builtin*{.property} `load`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.load "Permalink to this definition"){.headerlink} {#scopes.builtin.load}

:   

*builtin*{.property} `log`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.log "Permalink to this definition"){.headerlink} {#scopes.builtin.log}

:   

*builtin*{.property} `log2`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.log2 "Permalink to this definition"){.headerlink} {#scopes.builtin.log2}

:   

*builtin*{.property} `loop`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.loop "Permalink to this definition"){.headerlink} {#scopes.builtin.loop}

:   

*builtin*{.property} `lose`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.lose "Permalink to this definition"){.headerlink} {#scopes.builtin.lose}

:   

*builtin*{.property} `lshr`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.lshr "Permalink to this definition"){.headerlink} {#scopes.builtin.lshr}

:   

*builtin*{.property} `malloc`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.malloc "Permalink to this definition"){.headerlink} {#scopes.builtin.malloc}

:   

*builtin*{.property} `malloc-array`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.malloc-array "Permalink to this definition"){.headerlink} {#scopes.builtin.malloc-array}

:   

*builtin*{.property} `merge`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.merge "Permalink to this definition"){.headerlink} {#scopes.builtin.merge}

:   

*builtin*{.property} `move`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.move "Permalink to this definition"){.headerlink} {#scopes.builtin.move}

:   

*builtin*{.property} `mul`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.mul "Permalink to this definition"){.headerlink} {#scopes.builtin.mul}

:   

*builtin*{.property} `mul-nsw`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.mul-nsw "Permalink to this definition"){.headerlink} {#scopes.builtin.mul-nsw}

:   

*builtin*{.property} `mul-nuw`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.mul-nuw "Permalink to this definition"){.headerlink} {#scopes.builtin.mul-nuw}

:   

*builtin*{.property} `normalize`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.normalize "Permalink to this definition"){.headerlink} {#scopes.builtin.normalize}

:   

*builtin*{.property} `nullof`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.nullof "Permalink to this definition"){.headerlink} {#scopes.builtin.nullof}

:   

*builtin*{.property} `powf`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.powf "Permalink to this definition"){.headerlink} {#scopes.builtin.powf}

:   

*builtin*{.property} `ptrtoint`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.ptrtoint "Permalink to this definition"){.headerlink} {#scopes.builtin.ptrtoint}

:   

*builtin*{.property} `ptrtoref`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.ptrtoref "Permalink to this definition"){.headerlink} {#scopes.builtin.ptrtoref}

:   

*builtin*{.property} `radians`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.radians "Permalink to this definition"){.headerlink} {#scopes.builtin.radians}

:   

*builtin*{.property} `raise`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.raise "Permalink to this definition"){.headerlink} {#scopes.builtin.raise}

:   

*builtin*{.property} `raising`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.raising "Permalink to this definition"){.headerlink} {#scopes.builtin.raising}

:   

*builtin*{.property} `rawcall`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.rawcall "Permalink to this definition"){.headerlink} {#scopes.builtin.rawcall}

:   

*builtin*{.property} `reftoptr`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.reftoptr "Permalink to this definition"){.headerlink} {#scopes.builtin.reftoptr}

:   

*builtin*{.property} `repeat`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.repeat "Permalink to this definition"){.headerlink} {#scopes.builtin.repeat}

:   

*builtin*{.property} `return`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.return "Permalink to this definition"){.headerlink} {#scopes.builtin.return}

:   

*builtin*{.property} `returning`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.returning "Permalink to this definition"){.headerlink} {#scopes.builtin.returning}

:   

*builtin*{.property} `round`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.round "Permalink to this definition"){.headerlink} {#scopes.builtin.round}

:   

*builtin*{.property} `roundeven`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.roundeven "Permalink to this definition"){.headerlink} {#scopes.builtin.roundeven}

:   

*builtin*{.property} `run-stage`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.run-stage "Permalink to this definition"){.headerlink} {#scopes.builtin.run-stage}

:   

*builtin*{.property} `sample`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.sample "Permalink to this definition"){.headerlink} {#scopes.builtin.sample}

:   

*builtin*{.property} `sdiv`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.sdiv "Permalink to this definition"){.headerlink} {#scopes.builtin.sdiv}

:   

*builtin*{.property} `set-execution-mode`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.set-execution-mode "Permalink to this definition"){.headerlink} {#scopes.builtin.set-execution-mode}

:   

*builtin*{.property} `sext`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.sext "Permalink to this definition"){.headerlink} {#scopes.builtin.sext}

:   

*builtin*{.property} `shl`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.shl "Permalink to this definition"){.headerlink} {#scopes.builtin.shl}

:   

*builtin*{.property} `shufflevector`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.shufflevector "Permalink to this definition"){.headerlink} {#scopes.builtin.shufflevector}

:   

*builtin*{.property} `sin`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.sin "Permalink to this definition"){.headerlink} {#scopes.builtin.sin}

:   

*builtin*{.property} `sinh`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.sinh "Permalink to this definition"){.headerlink} {#scopes.builtin.sinh}

:   

*builtin*{.property} `sitofp`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.sitofp "Permalink to this definition"){.headerlink} {#scopes.builtin.sitofp}

:   

*builtin*{.property} `smax`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.smax "Permalink to this definition"){.headerlink} {#scopes.builtin.smax}

:   

*builtin*{.property} `smin`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.smin "Permalink to this definition"){.headerlink} {#scopes.builtin.smin}

:   

*builtin*{.property} `spice-quote`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.spice-quote "Permalink to this definition"){.headerlink} {#scopes.builtin.spice-quote}

:   

*builtin*{.property} `spice-unquote`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.spice-unquote "Permalink to this definition"){.headerlink} {#scopes.builtin.spice-unquote}

:   

*builtin*{.property} `spice-unquote-arguments`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.spice-unquote-arguments "Permalink to this definition"){.headerlink} {#scopes.builtin.spice-unquote-arguments}

:   

*builtin*{.property} `sqrt`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.sqrt "Permalink to this definition"){.headerlink} {#scopes.builtin.sqrt}

:   

*builtin*{.property} `square-list`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.square-list "Permalink to this definition"){.headerlink} {#scopes.builtin.square-list}

:   

*builtin*{.property} `srem`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.srem "Permalink to this definition"){.headerlink} {#scopes.builtin.srem}

:   

*builtin*{.property} `ssign`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.ssign "Permalink to this definition"){.headerlink} {#scopes.builtin.ssign}

:   

*builtin*{.property} `step`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.step "Permalink to this definition"){.headerlink} {#scopes.builtin.step}

:   

*builtin*{.property} `store`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.store "Permalink to this definition"){.headerlink} {#scopes.builtin.store}

:   

*builtin*{.property} `sub`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.sub "Permalink to this definition"){.headerlink} {#scopes.builtin.sub}

:   

*builtin*{.property} `sub-nsw`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.sub-nsw "Permalink to this definition"){.headerlink} {#scopes.builtin.sub-nsw}

:   

*builtin*{.property} `sub-nuw`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.sub-nuw "Permalink to this definition"){.headerlink} {#scopes.builtin.sub-nuw}

:   

*builtin*{.property} `sugar-log`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.sugar-log "Permalink to this definition"){.headerlink} {#scopes.builtin.sugar-log}

:   

*builtin*{.property} `sugar-quote`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.sugar-quote "Permalink to this definition"){.headerlink} {#scopes.builtin.sugar-quote}

:   

*builtin*{.property} `swapvalue`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.swapvalue "Permalink to this definition"){.headerlink} {#scopes.builtin.swapvalue}

:   

*builtin*{.property} `switch`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.switch "Permalink to this definition"){.headerlink} {#scopes.builtin.switch}

:   

*builtin*{.property} `tan`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.tan "Permalink to this definition"){.headerlink} {#scopes.builtin.tan}

:   

*builtin*{.property} `tanh`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.tanh "Permalink to this definition"){.headerlink} {#scopes.builtin.tanh}

:   

*builtin*{.property} `trunc`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.trunc "Permalink to this definition"){.headerlink} {#scopes.builtin.trunc}

:   

*builtin*{.property} `try`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.try "Permalink to this definition"){.headerlink} {#scopes.builtin.try}

:   

*builtin*{.property} `typeof`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.typeof "Permalink to this definition"){.headerlink} {#scopes.builtin.typeof}

:   

*builtin*{.property} `udiv`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.udiv "Permalink to this definition"){.headerlink} {#scopes.builtin.udiv}

:   

*builtin*{.property} `uitofp`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.uitofp "Permalink to this definition"){.headerlink} {#scopes.builtin.uitofp}

:   

*builtin*{.property} `umax`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.umax "Permalink to this definition"){.headerlink} {#scopes.builtin.umax}

:   

*builtin*{.property} `umin`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.umin "Permalink to this definition"){.headerlink} {#scopes.builtin.umin}

:   

*builtin*{.property} `undef`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.undef "Permalink to this definition"){.headerlink} {#scopes.builtin.undef}

:   

*builtin*{.property} `unique-visible?`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.unique-visible? "Permalink to this definition"){.headerlink} {#scopes.builtin.unique-visible?}

:   

*builtin*{.property} `unreachable`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.unreachable "Permalink to this definition"){.headerlink} {#scopes.builtin.unreachable}

:   

*builtin*{.property} `urem`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.urem "Permalink to this definition"){.headerlink} {#scopes.builtin.urem}

:   

*builtin*{.property} `va-countof`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.va-countof "Permalink to this definition"){.headerlink} {#scopes.builtin.va-countof}

:   

*builtin*{.property} `view`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.view "Permalink to this definition"){.headerlink} {#scopes.builtin.view}

:   

*builtin*{.property} `viewing`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.viewing "Permalink to this definition"){.headerlink} {#scopes.builtin.viewing}

:   

*builtin*{.property} `volatile`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.volatile "Permalink to this definition"){.headerlink} {#scopes.builtin.volatile}

:   

*builtin*{.property} `volatile-load`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.volatile-load "Permalink to this definition"){.headerlink} {#scopes.builtin.volatile-load}

:   

*builtin*{.property} `volatile-store`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.volatile-store "Permalink to this definition"){.headerlink} {#scopes.builtin.volatile-store}

:   

*builtin*{.property} `xchg`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.xchg "Permalink to this definition"){.headerlink} {#scopes.builtin.xchg}

:   

*builtin*{.property} `zext`{.descname} (*&ensp;...&ensp;*)[](#scopes.builtin.zext "Permalink to this definition"){.headerlink} {#scopes.builtin.zext}

:   

*spice*{.property} `%`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.% "Permalink to this definition"){.headerlink} {#scopes.spice.%}

:   

*spice*{.property} `&`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.& "Permalink to this definition"){.headerlink} {#scopes.spice.&}

:   

*spice*{.property} `*`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.* "Permalink to this definition"){.headerlink} {#scopes.spice.*}

:   

*spice*{.property} `+`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.+ "Permalink to this definition"){.headerlink} {#scopes.spice.+}

:   

*spice*{.property} `-`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.- "Permalink to this definition"){.headerlink} {#scopes.spice.-}

:   

*spice*{.property} `/`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice./ "Permalink to this definition"){.headerlink} {#scopes.spice./}

:   

*spice*{.property} `<`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.< "Permalink to this definition"){.headerlink} {#scopes.spice.<}

:   

*spice*{.property} `=`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.= "Permalink to this definition"){.headerlink} {#scopes.spice.=}

:   

*spice*{.property} `>`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.> "Permalink to this definition"){.headerlink} {#scopes.spice.>}

:   

*spice*{.property} `@`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.@ "Permalink to this definition"){.headerlink} {#scopes.spice.@}

:   

*spice*{.property} `^`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.^ "Permalink to this definition"){.headerlink} {#scopes.spice.^}

:   

*spice*{.property} `|`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.| "Permalink to this definition"){.headerlink} {#scopes.spice.|}

:   

*spice*{.property} `~`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.~ "Permalink to this definition"){.headerlink} {#scopes.spice.~}

:   

*spice*{.property} `!=`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.!= "Permalink to this definition"){.headerlink} {#scopes.spice.!=}

:   

*spice*{.property} `&?`{.descname} (*&ensp;value&ensp;*)[](#scopes.spice.&? "Permalink to this definition"){.headerlink} {#scopes.Generator.spice.&?}

:   Returns `true` if `value` is a reference, otherwise `false`.

*spice*{.property} `**`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.** "Permalink to this definition"){.headerlink} {#scopes.spice.**}

:   

*spice*{.property} `..`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice... "Permalink to this definition"){.headerlink} {#scopes.spice...}

:   

*spice*{.property} `//`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.// "Permalink to this definition"){.headerlink} {#scopes.spice.//}

:   

*spice*{.property} `<<`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.<< "Permalink to this definition"){.headerlink} {#scopes.spice.<<}

:   

*spice*{.property} `<=`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.<= "Permalink to this definition"){.headerlink} {#scopes.spice.<=}

:   

*spice*{.property} `==`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.== "Permalink to this definition"){.headerlink} {#scopes.spice.==}

:   

*spice*{.property} `>=`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.>= "Permalink to this definition"){.headerlink} {#scopes.spice.>=}

:   

*spice*{.property} `>>`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.>> "Permalink to this definition"){.headerlink} {#scopes.spice.>>}

:   

*spice*{.property} `_not`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice._not "Permalink to this definition"){.headerlink} {#scopes.spice._not}

:   

*spice*{.property} `_static-compile`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice._static-compile "Permalink to this definition"){.headerlink} {#scopes.spice._static-compile}

:   

*spice*{.property} `_static-compile-glsl`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice._static-compile-glsl "Permalink to this definition"){.headerlink} {#scopes.spice._static-compile-glsl}

:   

*spice*{.property} `_static-compile-spirv`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice._static-compile-spirv "Permalink to this definition"){.headerlink} {#scopes.spice._static-compile-spirv}

:   

*spice*{.property} `Closure->Accessor`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.Closure->Accessor "Permalink to this definition"){.headerlink} {#scopes.spice.Closure->Accessor}

:   

*spice*{.property} `Closure->Collector`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.Closure->Collector "Permalink to this definition"){.headerlink} {#scopes.spice.Closure->Collector}

:   

*spice*{.property} `Closure->Generator`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.Closure->Generator "Permalink to this definition"){.headerlink} {#scopes.spice.Closure->Generator}

:   

*spice*{.property} `abs`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.abs "Permalink to this definition"){.headerlink} {#scopes.spice.abs}

:   

*spice*{.property} `alignof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.alignof "Permalink to this definition"){.headerlink} {#scopes.spice.alignof}

:   

*spice*{.property} `and-branch`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.and-branch "Permalink to this definition"){.headerlink} {#scopes.spice.and-branch}

:   

*spice*{.property} `append-to-scope`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.append-to-scope "Permalink to this definition"){.headerlink} {#scopes.spice.append-to-scope}

:   

*spice*{.property} `append-to-type`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.append-to-type "Permalink to this definition"){.headerlink} {#scopes.spice.append-to-type}

:   

*spice*{.property} `argumentsof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.argumentsof "Permalink to this definition"){.headerlink} {#scopes.spice.argumentsof}

:   

*spice*{.property} `arrayof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.arrayof "Permalink to this definition"){.headerlink} {#scopes.spice.arrayof}

:   

*spice*{.property} `as`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.as "Permalink to this definition"){.headerlink} {#scopes.spice.as}

:   

*spice*{.property} `as?`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.as? "Permalink to this definition"){.headerlink} {#scopes.spice.as?}

:   

*spice*{.property} `bindingof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.bindingof "Permalink to this definition"){.headerlink} {#scopes.spice.bindingof}

:   

*spice*{.property} `bitcountof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.bitcountof "Permalink to this definition"){.headerlink} {#scopes.spice.bitcountof}

:   

*spice*{.property} `coerce-call-arguments`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.coerce-call-arguments "Permalink to this definition"){.headerlink} {#scopes.spice.coerce-call-arguments}

:   

*spice*{.property} `cons`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.cons "Permalink to this definition"){.headerlink} {#scopes.spice.cons}

:   

*spice*{.property} `const.add.i32.i32`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.const.add.i32.i32 "Permalink to this definition"){.headerlink} {#scopes.spice.const.add.i32.i32}

:   

*spice*{.property} `const.icmp<=.i32.i32`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.const.icmp<=.i32.i32 "Permalink to this definition"){.headerlink} {#scopes.spice.const.icmp<=.i32.i32}

:   

*spice*{.property} `constant?`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.constant? "Permalink to this definition"){.headerlink} {#scopes.spice.constant?}

:   

*spice*{.property} `copy`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.copy "Permalink to this definition"){.headerlink} {#scopes.spice.copy}

:   

*spice*{.property} `countof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.countof "Permalink to this definition"){.headerlink} {#scopes.spice.countof}

:   

*spice*{.property} `decons`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.decons "Permalink to this definition"){.headerlink} {#scopes.spice.decons}

:   

*spice*{.property} `defer`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.defer "Permalink to this definition"){.headerlink} {#scopes.spice.defer}

:   

*spice*{.property} `drop`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.drop "Permalink to this definition"){.headerlink} {#scopes.spice.drop}

:   

*spice*{.property} `elementof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.elementof "Permalink to this definition"){.headerlink} {#scopes.spice.elementof}

:   

*spice*{.property} `elementsof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.elementsof "Permalink to this definition"){.headerlink} {#scopes.spice.elementsof}

:   

*spice*{.property} `extern`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.extern "Permalink to this definition"){.headerlink} {#scopes.spice.extern}

:   

*spice*{.property} `gen-union-extractvalue`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.gen-union-extractvalue "Permalink to this definition"){.headerlink} {#scopes.spice.gen-union-extractvalue}

:   

*spice*{.property} `getattr`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.getattr "Permalink to this definition"){.headerlink} {#scopes.spice.getattr}

:   

*spice*{.property} `hash-storage`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.hash-storage "Permalink to this definition"){.headerlink} {#scopes.spice.hash-storage}

:   

*spice*{.property} `hash1`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.hash1 "Permalink to this definition"){.headerlink} {#scopes.spice.hash1}

:   

*spice*{.property} `imply`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.imply "Permalink to this definition"){.headerlink} {#scopes.spice.imply}

:   

*spice*{.property} `imply?`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.imply? "Permalink to this definition"){.headerlink} {#scopes.spice.imply?}

:   

*spice*{.property} `in`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.in "Permalink to this definition"){.headerlink} {#scopes.spice.in}

:   

*spice*{.property} `integer->integer`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.integer->integer "Permalink to this definition"){.headerlink} {#scopes.spice.integer->integer}

:   

*spice*{.property} `integer->real`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.integer->real "Permalink to this definition"){.headerlink} {#scopes.spice.integer->real}

:   

*spice*{.property} `key`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.key "Permalink to this definition"){.headerlink} {#scopes.spice.key}

:   

*spice*{.property} `keyof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.keyof "Permalink to this definition"){.headerlink} {#scopes.spice.keyof}

:   

*spice*{.property} `list-constructor`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.list-constructor "Permalink to this definition"){.headerlink} {#scopes.spice.list-constructor}

:   

*spice*{.property} `ln`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.ln "Permalink to this definition"){.headerlink} {#scopes.spice.ln}

:   

*spice*{.property} `locationof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.locationof "Permalink to this definition"){.headerlink} {#scopes.spice.locationof}

:   

*spice*{.property} `lslice`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.lslice "Permalink to this definition"){.headerlink} {#scopes.spice.lslice}

:   

*spice*{.property} `max`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.max "Permalink to this definition"){.headerlink} {#scopes.spice.max}

:   

*spice*{.property} `memocall`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.memocall "Permalink to this definition"){.headerlink} {#scopes.spice.memocall}

:   

*spice*{.property} `methodsof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.methodsof "Permalink to this definition"){.headerlink} {#scopes.spice.methodsof}

:   This function can be used in conjunction with `from`:
    
        :::scopes
        from (methodsof <object>) let method1 method2
    
    Now the imported methods are implicitly bound to `<object>` and can be
    called directly.

*spice*{.property} `min`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.min "Permalink to this definition"){.headerlink} {#scopes.spice.min}

:   

*spice*{.property} `mutable`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.mutable "Permalink to this definition"){.headerlink} {#scopes.spice.mutable}

:   

*spice*{.property} `mutable?`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.mutable? "Permalink to this definition"){.headerlink} {#scopes.spice.mutable?}

:   

*spice*{.property} `mutable@`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.mutable@ "Permalink to this definition"){.headerlink} {#scopes.spice.mutable@}

:   

*spice*{.property} `none?`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.none? "Permalink to this definition"){.headerlink} {#scopes.spice.none?}

:   

*spice*{.property} `offsetof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.offsetof "Permalink to this definition"){.headerlink} {#scopes.spice.offsetof}

:   

*spice*{.property} `opaque`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.opaque "Permalink to this definition"){.headerlink} {#scopes.spice.opaque}

:   

*spice*{.property} `or-branch`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.or-branch "Permalink to this definition"){.headerlink} {#scopes.spice.or-branch}

:   

*spice*{.property} `overloaded-fn-append`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.overloaded-fn-append "Permalink to this definition"){.headerlink} {#scopes.spice.overloaded-fn-append}

:   

*spice*{.property} `packedtupleof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.packedtupleof "Permalink to this definition"){.headerlink} {#scopes.spice.packedtupleof}

:   

*spice*{.property} `parse-compile-flags`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.parse-compile-flags "Permalink to this definition"){.headerlink} {#scopes.spice.parse-compile-flags}

:   

*spice*{.property} `plain?`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.plain? "Permalink to this definition"){.headerlink} {#scopes.spice.plain?}

:   

*spice*{.property} `pow`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.pow "Permalink to this definition"){.headerlink} {#scopes.spice.pow}

:   

*spice*{.property} `private`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.private "Permalink to this definition"){.headerlink} {#scopes.spice.private}

:   

*spice*{.property} `protect`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.protect "Permalink to this definition"){.headerlink} {#scopes.spice.protect}

:   

*spice*{.property} `qualifiersof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.qualifiersof "Permalink to this definition"){.headerlink} {#scopes.spice.qualifiersof}

:   

*spice*{.property} `raiseof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.raiseof "Permalink to this definition"){.headerlink} {#scopes.spice.raiseof}

:   

*spice*{.property} `raises`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.raises "Permalink to this definition"){.headerlink} {#scopes.spice.raises}

:   

*spice*{.property} `real->integer`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.real->integer "Permalink to this definition"){.headerlink} {#scopes.spice.real->integer}

:   

*spice*{.property} `real->real`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.real->real "Permalink to this definition"){.headerlink} {#scopes.spice.real->real}

:   

*spice*{.property} `report`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.report "Permalink to this definition"){.headerlink} {#scopes.spice.report}

:   

*spice*{.property} `repr`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.repr "Permalink to this definition"){.headerlink} {#scopes.spice.repr}

:   

*spice*{.property} `returnof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.returnof "Permalink to this definition"){.headerlink} {#scopes.spice.returnof}

:   

*spice*{.property} `rslice`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.rslice "Permalink to this definition"){.headerlink} {#scopes.spice.rslice}

:   

*spice*{.property} `sabs`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.sabs "Permalink to this definition"){.headerlink} {#scopes.spice.sabs}

:   

*spice*{.property} `safe-shl`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.safe-shl "Permalink to this definition"){.headerlink} {#scopes.spice.safe-shl}

:   

*spice*{.property} `sign`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.sign "Permalink to this definition"){.headerlink} {#scopes.spice.sign}

:   

*spice*{.property} `signed`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.signed "Permalink to this definition"){.headerlink} {#scopes.spice.signed}

:   

*spice*{.property} `signed?`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.signed? "Permalink to this definition"){.headerlink} {#scopes.spice.signed?}

:   

*spice*{.property} `sizeof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.sizeof "Permalink to this definition"){.headerlink} {#scopes.spice.sizeof}

:   

*spice*{.property} `static-branch`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.static-branch "Permalink to this definition"){.headerlink} {#scopes.spice.static-branch}

:   

*spice*{.property} `static-error`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.static-error "Permalink to this definition"){.headerlink} {#scopes.spice.static-error}

:   

*spice*{.property} `static-integer->integer`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.static-integer->integer "Permalink to this definition"){.headerlink} {#scopes.spice.static-integer->integer}

:   

*spice*{.property} `static-integer->real`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.static-integer->real "Permalink to this definition"){.headerlink} {#scopes.spice.static-integer->real}

:   

*spice*{.property} `static-library`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.static-library "Permalink to this definition"){.headerlink} {#scopes.spice.static-library}

:   

*spice*{.property} `static-try-closure`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.static-try-closure "Permalink to this definition"){.headerlink} {#scopes.spice.static-try-closure}

:   

*spice*{.property} `static-typify`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.static-typify "Permalink to this definition"){.headerlink} {#scopes.spice.static-typify}

:   

*spice*{.property} `storagecast`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.storagecast "Permalink to this definition"){.headerlink} {#scopes.spice.storagecast}

:   

*spice*{.property} `storageof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.storageof "Permalink to this definition"){.headerlink} {#scopes.spice.storageof}

:   

*spice*{.property} `superof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.superof "Permalink to this definition"){.headerlink} {#scopes.spice.superof}

:   

*spice*{.property} `tostring`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.tostring "Permalink to this definition"){.headerlink} {#scopes.spice.tostring}

:   

*spice*{.property} `tupleof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.tupleof "Permalink to this definition"){.headerlink} {#scopes.spice.tupleof}

:   

*spice*{.property} `type!=`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.type!= "Permalink to this definition"){.headerlink} {#scopes.spice.type!=}

:   

*spice*{.property} `type<`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.type< "Permalink to this definition"){.headerlink} {#scopes.spice.type<}

:   

*spice*{.property} `type<=`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.type<= "Permalink to this definition"){.headerlink} {#scopes.spice.type<=}

:   

*spice*{.property} `type==`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.type== "Permalink to this definition"){.headerlink} {#scopes.spice.type==}

:   

*spice*{.property} `type>`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.type> "Permalink to this definition"){.headerlink} {#scopes.spice.type>}

:   

*spice*{.property} `type>=`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.type>= "Permalink to this definition"){.headerlink} {#scopes.spice.type>=}

:   

*spice*{.property} `typeattr`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.typeattr "Permalink to this definition"){.headerlink} {#scopes.spice.typeattr}

:   

*spice*{.property} `typify`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.typify "Permalink to this definition"){.headerlink} {#scopes.spice.typify}

:   

*spice*{.property} `union-storage-type`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.union-storage-type "Permalink to this definition"){.headerlink} {#scopes.spice.union-storage-type}

:   

*spice*{.property} `union-storageof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.union-storageof "Permalink to this definition"){.headerlink} {#scopes.spice.union-storageof}

:   

*spice*{.property} `uniqueof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.uniqueof "Permalink to this definition"){.headerlink} {#scopes.spice.uniqueof}

:   

*spice*{.property} `unpack`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.unpack "Permalink to this definition"){.headerlink} {#scopes.spice.unpack}

:   

*spice*{.property} `unqualified`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.unqualified "Permalink to this definition"){.headerlink} {#scopes.spice.unqualified}

:   

*spice*{.property} `unsized?`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.unsized? "Permalink to this definition"){.headerlink} {#scopes.spice.unsized?}

:   

*spice*{.property} `va-append-va`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.va-append-va "Permalink to this definition"){.headerlink} {#scopes.spice.va-append-va}

:    (va-append-va (inline () (_ b ...)) a...) -> a... b...

*spice*{.property} `va-empty?`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.va-empty? "Permalink to this definition"){.headerlink} {#scopes.spice.va-empty?}

:   

*spice*{.property} `va-lfold`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.va-lfold "Permalink to this definition"){.headerlink} {#scopes.spice.va-lfold}

:   

*spice*{.property} `va-lifold`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.va-lifold "Permalink to this definition"){.headerlink} {#scopes.spice.va-lifold}

:   

*spice*{.property} `va-map`{.descname} (*&ensp;f ...&ensp;*)[](#scopes.spice.va-map "Permalink to this definition"){.headerlink} {#scopes.spice.va-map}

:   Filter each argument in `...` through `f` and return the resulting list
    of arguments. Arguments where `f` returns void are filtered from the
    result.

*spice*{.property} `va-option-branch`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.va-option-branch "Permalink to this definition"){.headerlink} {#scopes.spice.va-option-branch}

:   

*spice*{.property} `va-range`{.descname} (*&ensp;a [ b ]&ensp;*)[](#scopes.spice.va-range "Permalink to this definition"){.headerlink} {#scopes.spice.va-range}

:   If `b` is not specified, returns a sequence of integers from zero to `b`,
    otherwise a sequence of integers from `a` to `b`.

*spice*{.property} `va-rfold`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.va-rfold "Permalink to this definition"){.headerlink} {#scopes.spice.va-rfold}

:   

*spice*{.property} `va-rifold`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.va-rifold "Permalink to this definition"){.headerlink} {#scopes.spice.va-rifold}

:   

*spice*{.property} `va-split`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.va-split "Permalink to this definition"){.headerlink} {#scopes.spice.va-split}

:    (va-split n a...) -> (inline () a...[n .. (va-countof a...)-1]) a...[0 .. n-1]

*spice*{.property} `va-unnamed`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.va-unnamed "Permalink to this definition"){.headerlink} {#scopes.spice.va-unnamed}

:   Filter all keyed values.

*spice*{.property} `va@`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.va@ "Permalink to this definition"){.headerlink} {#scopes.spice.va@}

:   

*spice*{.property} `vector-reduce`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.vector-reduce "Permalink to this definition"){.headerlink} {#scopes.spice.vector-reduce}

:   

*spice*{.property} `vectorof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.vectorof "Permalink to this definition"){.headerlink} {#scopes.spice.vectorof}

:   

*spice*{.property} `verify-stepsize`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.verify-stepsize "Permalink to this definition"){.headerlink} {#scopes.spice.verify-stepsize}

:   

*spice*{.property} `viewof`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.viewof "Permalink to this definition"){.headerlink} {#scopes.spice.viewof}

:   

*spice*{.property} `wrap-if-not-run-stage`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.wrap-if-not-run-stage "Permalink to this definition"){.headerlink} {#scopes.spice.wrap-if-not-run-stage}

:   

*spice*{.property} `zip`{.descname} (*&ensp;...&ensp;*)[](#scopes.spice.zip "Permalink to this definition"){.headerlink} {#scopes.spice.zip}

:   

*compiledfn*{.property} `compiler-version`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.compiler-version "Permalink to this definition"){.headerlink} {#scopes.compiledfn.compiler-version}

:   An external function of type `((_: i32 i32 i32) <-: ())`.

*compiledfn*{.property} `debugtrap`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.debugtrap "Permalink to this definition"){.headerlink} {#scopes.compiledfn.debugtrap}

:   An external function of type `(void <-: ())`.

*compiledfn*{.property} `default-styler`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.default-styler "Permalink to this definition"){.headerlink} {#scopes.compiledfn.default-styler}

:   An external function of type `(string <-: (Symbol string))`.

*compiledfn*{.property} `exit`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.exit "Permalink to this definition"){.headerlink} {#scopes.compiledfn.exit}

:   An external function of type `(noreturn <-: (i32))`.

*compiledfn*{.property} `function->SugarMacro`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.function->SugarMacro "Permalink to this definition"){.headerlink} {#scopes.compiledfn.function->SugarMacro}

:   A compiled function of type `(SugarMacro <-: ((opaque@ ((_: List Scope) <-: (List Scope) raises Error))))`.

*compiledfn*{.property} `globals`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.globals "Permalink to this definition"){.headerlink} {#scopes.compiledfn.globals}

:   An external function of type `(Scope <-: ())`.

*compiledfn*{.property} `io-write!`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.io-write! "Permalink to this definition"){.headerlink} {#scopes.compiledfn.io-write!}

:   An external function of type `(void <-: (string))`.

*compiledfn*{.property} `launch-args`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.launch-args "Permalink to this definition"){.headerlink} {#scopes.compiledfn.launch-args}

:   An external function of type `((_: i32 (@ (@ i8))) <-: ())`.

*compiledfn*{.property} `list-handler`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.list-handler "Permalink to this definition"){.headerlink} {#scopes.compiledfn.list-handler}

:   A compiled function of type `((_: List Scope) <-: (List Scope) raises Error)`.

*compiledfn*{.property} `list-load`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.list-load "Permalink to this definition"){.headerlink} {#scopes.compiledfn.list-load}

:   An external function of type `(Value <-: (string) raises Error)`.

*compiledfn*{.property} `list-parse`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.list-parse "Permalink to this definition"){.headerlink} {#scopes.compiledfn.list-parse}

:   An external function of type `(Value <-: (string) raises Error)`.

*compiledfn*{.property} `load-library`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.load-library "Permalink to this definition"){.headerlink} {#scopes.compiledfn.load-library}

:   An external function of type `(void <-: (string) raises Error)`.

*compiledfn*{.property} `load-object`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.load-object "Permalink to this definition"){.headerlink} {#scopes.compiledfn.load-object}

:   An external function of type `(void <-: (string) raises Error)`.

*compiledfn*{.property} `parse-infix-expr`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.parse-infix-expr "Permalink to this definition"){.headerlink} {#scopes.compiledfn.parse-infix-expr}

:   A compiled function of type `((_: Value List) <-: (Scope Value List i32) raises Error)`.

*compiledfn*{.property} `realpath`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.realpath "Permalink to this definition"){.headerlink} {#scopes.compiledfn.realpath}

:   An external function of type `(string <-: (string))`.

*compiledfn*{.property} `sc_abort`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_abort "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_abort}

:   An external function of type `(noreturn <-: ())`.

*compiledfn*{.property} `sc_anchor_column`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_anchor_column "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_anchor_column}

:   An external function of type `(i32 <-: (Anchor))`.

*compiledfn*{.property} `sc_anchor_lineno`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_anchor_lineno "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_anchor_lineno}

:   An external function of type `(i32 <-: (Anchor))`.

*compiledfn*{.property} `sc_anchor_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_anchor_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_anchor_new}

:   An external function of type `(Anchor <-: (Symbol i32 i32 i32))`.

*compiledfn*{.property} `sc_anchor_offset`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_anchor_offset "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_anchor_offset}

:   An external function of type `(Anchor <-: (Anchor i32))`.

*compiledfn*{.property} `sc_anchor_path`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_anchor_path "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_anchor_path}

:   An external function of type `(Symbol <-: (Anchor))`.

*compiledfn*{.property} `sc_argcount`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_argcount "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_argcount}

:   An external function of type `(i32 <-: (Value))`.

*compiledfn*{.property} `sc_argument_list_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_argument_list_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_argument_list_new}

:   An external function of type `(Value <-: (i32 (@ Value)))`.

*compiledfn*{.property} `sc_arguments_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_arguments_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_arguments_type}

:   An external function of type `(type <-: (i32 (@ type)))`.

*compiledfn*{.property} `sc_arguments_type_argcount`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_arguments_type_argcount "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_arguments_type_argcount}

:   An external function of type `(i32 <-: (type))`.

*compiledfn*{.property} `sc_arguments_type_getarg`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_arguments_type_getarg "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_arguments_type_getarg}

:   An external function of type `(type <-: (type i32))`.

*compiledfn*{.property} `sc_arguments_type_join`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_arguments_type_join "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_arguments_type_join}

:   An external function of type `(type <-: (type type))`.

*compiledfn*{.property} `sc_array_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_array_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_array_type}

:   An external function of type `(type <-: (type usize) raises Error)`.

*compiledfn*{.property} `sc_basename`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_basename "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_basename}

:   An external function of type `(string <-: (string))`.

*compiledfn*{.property} `sc_cache_misses`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_cache_misses "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_cache_misses}

:   An external function of type `(i32 <-: ())`.

*compiledfn*{.property} `sc_call_append_argument`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_call_append_argument "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_call_append_argument}

:   An external function of type `(void <-: (Value Value))`.

*compiledfn*{.property} `sc_call_is_rawcall`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_call_is_rawcall "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_call_is_rawcall}

:   An external function of type `(bool <-: (Value))`.

*compiledfn*{.property} `sc_call_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_call_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_call_new}

:   An external function of type `(Value <-: (Value))`.

*compiledfn*{.property} `sc_call_set_rawcall`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_call_set_rawcall "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_call_set_rawcall}

:   An external function of type `(void <-: (Value bool))`.

*compiledfn*{.property} `sc_case_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_case_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_case_new}

:   An external function of type `(Value <-: (Value Value))`.

*compiledfn*{.property} `sc_closure_get_context`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_closure_get_context "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_closure_get_context}

:   An external function of type `(Value <-: (Closure))`.

*compiledfn*{.property} `sc_closure_get_docstring`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_closure_get_docstring "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_closure_get_docstring}

:   An external function of type `(string <-: (Closure))`.

*compiledfn*{.property} `sc_closure_get_template`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_closure_get_template "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_closure_get_template}

:   An external function of type `(Value <-: (Closure))`.

*compiledfn*{.property} `sc_compile`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_compile "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_compile}

:   An external function of type `(Value <-: (Value u64) raises Error)`.

*compiledfn*{.property} `sc_compile_glsl`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_compile_glsl "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_compile_glsl}

:   An external function of type `(string <-: (i32 Symbol Value u64) raises Error)`.

*compiledfn*{.property} `sc_compile_object`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_compile_object "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_compile_object}

:   An external function of type `(void <-: (string i32 string Scope u64) raises Error)`.

*compiledfn*{.property} `sc_compile_spirv`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_compile_spirv "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_compile_spirv}

:   An external function of type `(string <-: (i32 Symbol Value u64) raises Error)`.

*compiledfn*{.property} `sc_compiler_version`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_compiler_version "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_compiler_version}

:   An external function of type `((_: i32 i32 i32) <-: ())`.

*compiledfn*{.property} `sc_cond_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_cond_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_cond_new}

:   An external function of type `(Value <-: (Value Value Value))`.

*compiledfn*{.property} `sc_const_aggregate_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_const_aggregate_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_const_aggregate_new}

:   An external function of type `(Value <-: (type i32 (@ Value)))`.

*compiledfn*{.property} `sc_const_extract_at`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_const_extract_at "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_const_extract_at}

:   An external function of type `(Value <-: (Value i32))`.

*compiledfn*{.property} `sc_const_int_extract`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_const_int_extract "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_const_int_extract}

:   An external function of type `(u64 <-: (Value))`.

*compiledfn*{.property} `sc_const_int_extract_word`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_const_int_extract_word "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_const_int_extract_word}

:   An external function of type `(u64 <-: (Value i32))`.

*compiledfn*{.property} `sc_const_int_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_const_int_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_const_int_new}

:   An external function of type `(Value <-: (type u64))`.

*compiledfn*{.property} `sc_const_int_word_count`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_const_int_word_count "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_const_int_word_count}

:   An external function of type `(i32 <-: (Value))`.

*compiledfn*{.property} `sc_const_int_words_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_const_int_words_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_const_int_words_new}

:   An external function of type `(Value <-: (type i32 (@ u64)))`.

*compiledfn*{.property} `sc_const_null_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_const_null_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_const_null_new}

:   An external function of type `(Value <-: (type) raises Error)`.

*compiledfn*{.property} `sc_const_pointer_extract`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_const_pointer_extract "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_const_pointer_extract}

:   An external function of type `((opaque@ void) <-: (Value))`.

*compiledfn*{.property} `sc_const_pointer_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_const_pointer_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_const_pointer_new}

:   An external function of type `(Value <-: (type (opaque@ void)))`.

*compiledfn*{.property} `sc_const_real_extract`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_const_real_extract "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_const_real_extract}

:   An external function of type `(f64 <-: (Value))`.

*compiledfn*{.property} `sc_const_real_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_const_real_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_const_real_new}

:   An external function of type `(Value <-: (type f64))`.

*compiledfn*{.property} `sc_default_case_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_default_case_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_default_case_new}

:   An external function of type `(Value <-: (Value))`.

*compiledfn*{.property} `sc_default_styler`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_default_styler "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_default_styler}

:   An external function of type `(string <-: (Symbol string))`.

*compiledfn*{.property} `sc_default_target_triple`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_default_target_triple "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_default_target_triple}

:   An external function of type `(string <-: ())`.

*compiledfn*{.property} `sc_dirname`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_dirname "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_dirname}

:   An external function of type `(string <-: (string))`.

*compiledfn*{.property} `sc_do_case_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_do_case_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_do_case_new}

:   An external function of type `(Value <-: (Value))`.

*compiledfn*{.property} `sc_dump_error`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_dump_error "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_dump_error}

:   An external function of type `(void <-: (Error))`.

*compiledfn*{.property} `sc_empty_argument_list`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_empty_argument_list "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_empty_argument_list}

:   An external function of type `(Value <-: ())`.

*compiledfn*{.property} `sc_enter_solver_cli`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_enter_solver_cli "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_enter_solver_cli}

:   An external function of type `(void <-: ())`.

*compiledfn*{.property} `sc_error_append_calltrace`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_error_append_calltrace "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_error_append_calltrace}

:   An external function of type `(void <-: (Error Value))`.

*compiledfn*{.property} `sc_error_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_error_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_error_new}

:   An external function of type `(Error <-: (string))`.

*compiledfn*{.property} `sc_eval`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_eval "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_eval}

:   An external function of type `(Value <-: (Anchor List Scope) raises Error)`.

*compiledfn*{.property} `sc_eval_inline`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_eval_inline "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_eval_inline}

:   An external function of type `(Anchor <-: (Value List Scope) raises Error)`.

*compiledfn*{.property} `sc_eval_stage`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_eval_stage "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_eval_stage}

:   An external function of type `(Value <-: (Anchor List Scope) raises Error)`.

*compiledfn*{.property} `sc_exit`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_exit "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_exit}

:   An external function of type `(noreturn <-: (i32))`.

*compiledfn*{.property} `sc_expand`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_expand "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_expand}

:   An external function of type `((_: Value List Scope) <-: (Value List Scope) raises Error)`.

*compiledfn*{.property} `sc_expression_append`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_expression_append "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_expression_append}

:   An external function of type `(void <-: (Value Value))`.

*compiledfn*{.property} `sc_expression_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_expression_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_expression_new}

:   An external function of type `(Value <-: ())`.

*compiledfn*{.property} `sc_expression_set_scoped`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_expression_set_scoped "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_expression_set_scoped}

:   An external function of type `(void <-: (Value))`.

*compiledfn*{.property} `sc_extract_argument_list_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_extract_argument_list_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_extract_argument_list_new}

:   An external function of type `(Value <-: (Value i32))`.

*compiledfn*{.property} `sc_extract_argument_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_extract_argument_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_extract_argument_new}

:   An external function of type `(Value <-: (Value i32))`.

*compiledfn*{.property} `sc_format_error`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_format_error "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_format_error}

:   An external function of type `(string <-: (Error))`.

*compiledfn*{.property} `sc_format_message`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_format_message "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_format_message}

:   An external function of type `(string <-: (Anchor string))`.

*compiledfn*{.property} `sc_function_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_function_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_function_type}

:   An external function of type `(type <-: (type i32 (@ type)))`.

*compiledfn*{.property} `sc_function_type_is_variadic`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_function_type_is_variadic "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_function_type_is_variadic}

:   An external function of type `(bool <-: (type))`.

*compiledfn*{.property} `sc_function_type_raising`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_function_type_raising "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_function_type_raising}

:   An external function of type `(type <-: (type type))`.

*compiledfn*{.property} `sc_function_type_return_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_function_type_return_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_function_type_return_type}

:   An external function of type `((_: type type) <-: (type))`.

*compiledfn*{.property} `sc_get_globals`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_get_globals "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_get_globals}

:   An external function of type `(Scope <-: ())`.

*compiledfn*{.property} `sc_get_original_globals`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_get_original_globals "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_get_original_globals}

:   An external function of type `(Scope <-: ())`.

*compiledfn*{.property} `sc_getarg`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_getarg "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_getarg}

:   An external function of type `(Value <-: (Value i32))`.

*compiledfn*{.property} `sc_getarglist`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_getarglist "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_getarglist}

:   An external function of type `(Value <-: (Value i32))`.

*compiledfn*{.property} `sc_global_binding`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_global_binding "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_global_binding}

:   An external function of type `(i32 <-: (Value) raises Error)`.

*compiledfn*{.property} `sc_global_descriptor_set`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_global_descriptor_set "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_global_descriptor_set}

:   An external function of type `(i32 <-: (Value) raises Error)`.

*compiledfn*{.property} `sc_global_flags`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_global_flags "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_global_flags}

:   An external function of type `(u32 <-: (Value) raises Error)`.

*compiledfn*{.property} `sc_global_location`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_global_location "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_global_location}

:   An external function of type `(i32 <-: (Value) raises Error)`.

*compiledfn*{.property} `sc_global_name`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_global_name "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_global_name}

:   An external function of type `(Symbol <-: (Value) raises Error)`.

*compiledfn*{.property} `sc_global_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_global_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_global_new}

:   An external function of type `(Value <-: (Symbol type u32 Symbol))`.

*compiledfn*{.property} `sc_global_set_binding`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_global_set_binding "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_global_set_binding}

:   An external function of type `(void <-: (Value i32) raises Error)`.

*compiledfn*{.property} `sc_global_set_constructor`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_global_set_constructor "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_global_set_constructor}

:   An external function of type `(void <-: (Value Value) raises Error)`.

*compiledfn*{.property} `sc_global_set_descriptor_set`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_global_set_descriptor_set "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_global_set_descriptor_set}

:   An external function of type `(void <-: (Value i32) raises Error)`.

*compiledfn*{.property} `sc_global_set_initializer`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_global_set_initializer "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_global_set_initializer}

:   An external function of type `(void <-: (Value Value) raises Error)`.

*compiledfn*{.property} `sc_global_set_location`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_global_set_location "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_global_set_location}

:   An external function of type `(void <-: (Value i32) raises Error)`.

*compiledfn*{.property} `sc_global_storage_class`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_global_storage_class "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_global_storage_class}

:   An external function of type `(Symbol <-: (Value) raises Error)`.

*compiledfn*{.property} `sc_global_string_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_global_string_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_global_string_new}

:   An external function of type `(Value <-: ((@ i8) usize))`.

*compiledfn*{.property} `sc_global_string_new_from_cstr`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_global_string_new_from_cstr "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_global_string_new_from_cstr}

:   An external function of type `(Value <-: ((@ i8)))`.

*compiledfn*{.property} `sc_hash`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_hash "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_hash}

:   An external function of type `(u64 <-: (u64 usize))`.

*compiledfn*{.property} `sc_hash2x64`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_hash2x64 "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_hash2x64}

:   An external function of type `(u64 <-: (u64 u64))`.

*compiledfn*{.property} `sc_hashbytes`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_hashbytes "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_hashbytes}

:   An external function of type `(u64 <-: ((@ i8) usize))`.

*compiledfn*{.property} `sc_identity`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_identity "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_identity}

:   An external function of type `(Value <-: (Value))`.

*compiledfn*{.property} `sc_image_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_image_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_image_type}

:   An external function of type `(type <-: (type Symbol i32 i32 i32 i32 Symbol Symbol))`.

*compiledfn*{.property} `sc_import_c`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_import_c "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_import_c}

:   An external function of type `(Scope <-: (string string List Scope) raises Error)`.

*compiledfn*{.property} `sc_integer_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_integer_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_integer_type}

:   An external function of type `(type <-: (i32 bool))`.

*compiledfn*{.property} `sc_integer_type_is_signed`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_integer_type_is_signed "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_integer_type_is_signed}

:   An external function of type `(bool <-: (type))`.

*compiledfn*{.property} `sc_is_directory`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_is_directory "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_is_directory}

:   An external function of type `(bool <-: (string))`.

*compiledfn*{.property} `sc_is_file`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_is_file "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_is_file}

:   An external function of type `(bool <-: (string))`.

*compiledfn*{.property} `sc_key_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_key_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_key_type}

:   An external function of type `(type <-: (Symbol type))`.

*compiledfn*{.property} `sc_keyed_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_keyed_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_keyed_new}

:   An external function of type `(Value <-: (Symbol Value))`.

*compiledfn*{.property} `sc_label_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_label_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_label_new}

:   An external function of type `(Value <-: (i32 Symbol))`.

*compiledfn*{.property} `sc_label_set_body`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_label_set_body "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_label_set_body}

:   An external function of type `(void <-: (Value Value))`.

*compiledfn*{.property} `sc_launch_args`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_launch_args "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_launch_args}

:   An external function of type `((_: i32 (@ (@ i8))) <-: ())`.

*compiledfn*{.property} `sc_list_at`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_list_at "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_list_at}

:   An external function of type `(Value <-: (List))`.

*compiledfn*{.property} `sc_list_compare`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_list_compare "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_list_compare}

:   An external function of type `(bool <-: (List List))`.

*compiledfn*{.property} `sc_list_cons`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_list_cons "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_list_cons}

:   An external function of type `(List <-: (Value List))`.

*compiledfn*{.property} `sc_list_count`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_list_count "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_list_count}

:   An external function of type `(i32 <-: (List))`.

*compiledfn*{.property} `sc_list_decons`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_list_decons "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_list_decons}

:   An external function of type `((_: Value List) <-: (List))`.

*compiledfn*{.property} `sc_list_dump`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_list_dump "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_list_dump}

:   An external function of type `(List <-: (List))`.

*compiledfn*{.property} `sc_list_join`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_list_join "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_list_join}

:   An external function of type `(List <-: (List List))`.

*compiledfn*{.property} `sc_list_next`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_list_next "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_list_next}

:   An external function of type `(List <-: (List))`.

*compiledfn*{.property} `sc_list_repr`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_list_repr "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_list_repr}

:   An external function of type `(string <-: (List))`.

*compiledfn*{.property} `sc_list_reverse`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_list_reverse "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_list_reverse}

:   An external function of type `(List <-: (List))`.

*compiledfn*{.property} `sc_list_serialize`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_list_serialize "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_list_serialize}

:   An external function of type `(string <-: (List))`.

*compiledfn*{.property} `sc_load_library`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_load_library "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_load_library}

:   An external function of type `(void <-: (string) raises Error)`.

*compiledfn*{.property} `sc_load_object`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_load_object "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_load_object}

:   An external function of type `(void <-: (string) raises Error)`.

*compiledfn*{.property} `sc_loop_arguments`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_loop_arguments "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_loop_arguments}

:   An external function of type `(Value <-: (Value))`.

*compiledfn*{.property} `sc_loop_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_loop_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_loop_new}

:   An external function of type `(Value <-: (Value))`.

*compiledfn*{.property} `sc_loop_set_body`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_loop_set_body "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_loop_set_body}

:   An external function of type `(void <-: (Value Value))`.

*compiledfn*{.property} `sc_map_get`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_map_get "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_map_get}

:   An external function of type `(Value <-: (Value) raises Error)`.

*compiledfn*{.property} `sc_map_set`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_map_set "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_map_set}

:   An external function of type `(void <-: (Value Value))`.

*compiledfn*{.property} `sc_matrix_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_matrix_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_matrix_type}

:   An external function of type `(type <-: (type usize) raises Error)`.

*compiledfn*{.property} `sc_merge_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_merge_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_merge_new}

:   An external function of type `(Value <-: (Value Value))`.

*compiledfn*{.property} `sc_mutate_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_mutate_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_mutate_type}

:   An external function of type `(type <-: (type))`.

*compiledfn*{.property} `sc_packed_tuple_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_packed_tuple_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_packed_tuple_type}

:   An external function of type `(type <-: (i32 (@ type)) raises Error)`.

*compiledfn*{.property} `sc_parameter_is_variadic`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_parameter_is_variadic "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_parameter_is_variadic}

:   An external function of type `(bool <-: (Value))`.

*compiledfn*{.property} `sc_parameter_name`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_parameter_name "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_parameter_name}

:   An external function of type `(Symbol <-: (Value))`.

*compiledfn*{.property} `sc_parameter_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_parameter_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_parameter_new}

:   An external function of type `(Value <-: (Symbol))`.

*compiledfn*{.property} `sc_parse_from_path`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_parse_from_path "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_parse_from_path}

:   An external function of type `(Value <-: (string) raises Error)`.

*compiledfn*{.property} `sc_parse_from_string`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_parse_from_string "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_parse_from_string}

:   An external function of type `(Value <-: (string) raises Error)`.

*compiledfn*{.property} `sc_pass_case_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_pass_case_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_pass_case_new}

:   An external function of type `(Value <-: (Value Value))`.

*compiledfn*{.property} `sc_pointer_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_pointer_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_pointer_type}

:   An external function of type `(type <-: (type u64 Symbol))`.

*compiledfn*{.property} `sc_pointer_type_get_flags`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_pointer_type_get_flags "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_pointer_type_get_flags}

:   An external function of type `(u64 <-: (type))`.

*compiledfn*{.property} `sc_pointer_type_get_storage_class`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_pointer_type_get_storage_class "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_pointer_type_get_storage_class}

:   An external function of type `(Symbol <-: (type))`.

*compiledfn*{.property} `sc_pointer_type_set_element_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_pointer_type_set_element_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_pointer_type_set_element_type}

:   An external function of type `(type <-: (type type))`.

*compiledfn*{.property} `sc_pointer_type_set_flags`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_pointer_type_set_flags "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_pointer_type_set_flags}

:   An external function of type `(type <-: (type u64))`.

*compiledfn*{.property} `sc_pointer_type_set_storage_class`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_pointer_type_set_storage_class "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_pointer_type_set_storage_class}

:   An external function of type `(type <-: (type Symbol))`.

*compiledfn*{.property} `sc_prompt`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_prompt "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_prompt}

:   An external function of type `((_: bool string) <-: (string string))`.

*compiledfn*{.property} `sc_prompt_add_completion`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_prompt_add_completion "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_prompt_add_completion}

:   An external function of type `(void <-: ((opaque@ void) (@ i8)))`.

*compiledfn*{.property} `sc_prompt_add_completion_from_scope`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_prompt_add_completion_from_scope "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_prompt_add_completion_from_scope}

:   An external function of type `(void <-: ((opaque@ void) (@ i8) i32 Scope))`.

*compiledfn*{.property} `sc_prompt_load_history`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_prompt_load_history "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_prompt_load_history}

:   An external function of type `(void <-: (string))`.

*compiledfn*{.property} `sc_prompt_save_history`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_prompt_save_history "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_prompt_save_history}

:   An external function of type `(void <-: (string))`.

*compiledfn*{.property} `sc_prompt_set_autocomplete_handler`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_prompt_set_autocomplete_handler "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_prompt_set_autocomplete_handler}

:   An external function of type `(void <-: ((opaque@ (void <-: ((@ i8) (opaque@ void))))))`.

*compiledfn*{.property} `sc_prove`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_prove "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_prove}

:   An external function of type `(Value <-: (Value) raises Error)`.

*compiledfn*{.property} `sc_quote_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_quote_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_quote_new}

:   An external function of type `(Value <-: (Value))`.

*compiledfn*{.property} `sc_realpath`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_realpath "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_realpath}

:   An external function of type `(string <-: (string))`.

*compiledfn*{.property} `sc_refer_flags`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_refer_flags "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_refer_flags}

:   An external function of type `(u64 <-: (type))`.

*compiledfn*{.property} `sc_refer_storage_class`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_refer_storage_class "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_refer_storage_class}

:   An external function of type `(Symbol <-: (type))`.

*compiledfn*{.property} `sc_refer_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_refer_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_refer_type}

:   An external function of type `(type <-: (type u64 Symbol))`.

*compiledfn*{.property} `sc_sampled_image_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_sampled_image_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_sampled_image_type}

:   An external function of type `(type <-: (type))`.

*compiledfn*{.property} `sc_scope_at`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_at "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_at}

:   An external function of type `(Value <-: (Scope Value) raises Error)`.

*compiledfn*{.property} `sc_scope_bind`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_bind "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_bind}

:   An external function of type `(Scope <-: (Scope Value Value))`.

*compiledfn*{.property} `sc_scope_bind_with_docstring`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_bind_with_docstring "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_bind_with_docstring}

:   An external function of type `(Scope <-: (Scope Value Value string))`.

*compiledfn*{.property} `sc_scope_docstring`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_docstring "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_docstring}

:   An external function of type `(string <-: (Scope Value))`.

*compiledfn*{.property} `sc_scope_get_parent`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_get_parent "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_get_parent}

:   An external function of type `(Scope <-: (Scope))`.

*compiledfn*{.property} `sc_scope_local_at`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_local_at "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_local_at}

:   An external function of type `(Value <-: (Scope Value) raises Error)`.

*compiledfn*{.property} `sc_scope_module_docstring`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_module_docstring "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_module_docstring}

:   An external function of type `(string <-: (Scope))`.

*compiledfn*{.property} `sc_scope_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_new}

:   An external function of type `(Scope <-: ())`.

*compiledfn*{.property} `sc_scope_new_subscope`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_new_subscope "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_new_subscope}

:   An external function of type `(Scope <-: (Scope))`.

*compiledfn*{.property} `sc_scope_new_subscope_with_docstring`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_new_subscope_with_docstring "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_new_subscope_with_docstring}

:   An external function of type `(Scope <-: (Scope string))`.

*compiledfn*{.property} `sc_scope_new_with_docstring`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_new_with_docstring "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_new_with_docstring}

:   An external function of type `(Scope <-: (string))`.

*compiledfn*{.property} `sc_scope_next`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_next "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_next}

:   An external function of type `((_: Value Value i32) <-: (Scope i32))`.

*compiledfn*{.property} `sc_scope_next_deleted`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_next_deleted "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_next_deleted}

:   An external function of type `((_: Value i32) <-: (Scope i32))`.

*compiledfn*{.property} `sc_scope_reparent`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_reparent "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_reparent}

:   An external function of type `(Scope <-: (Scope Scope))`.

*compiledfn*{.property} `sc_scope_unbind`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_unbind "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_unbind}

:   An external function of type `(Scope <-: (Scope Value))`.

*compiledfn*{.property} `sc_scope_unparent`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_scope_unparent "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_scope_unparent}

:   An external function of type `(Scope <-: (Scope))`.

*compiledfn*{.property} `sc_set_globals`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_set_globals "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_set_globals}

:   An external function of type `(void <-: (Scope))`.

*compiledfn*{.property} `sc_set_signal_abort`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_set_signal_abort "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_set_signal_abort}

:   An external function of type `(void <-: (bool))`.

*compiledfn*{.property} `sc_set_typecast_handler`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_set_typecast_handler "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_set_typecast_handler}

:   An external function of type `(void <-: ((opaque@ (Value <-: (Value type) raises Error))))`.

*compiledfn*{.property} `sc_spirv_to_glsl`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_spirv_to_glsl "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_spirv_to_glsl}

:   An external function of type `(string <-: (string))`.

*compiledfn*{.property} `sc_string_buffer`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_string_buffer "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_string_buffer}

:   An external function of type `((_: (@ i8) usize) <-: (string))`.

*compiledfn*{.property} `sc_string_compare`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_string_compare "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_string_compare}

:   An external function of type `(i32 <-: (string string))`.

*compiledfn*{.property} `sc_string_count`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_string_count "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_string_count}

:   An external function of type `(usize <-: (string))`.

*compiledfn*{.property} `sc_string_join`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_string_join "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_string_join}

:   An external function of type `(string <-: (string string))`.

*compiledfn*{.property} `sc_string_lslice`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_string_lslice "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_string_lslice}

:   An external function of type `(string <-: (string usize))`.

*compiledfn*{.property} `sc_string_match`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_string_match "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_string_match}

:   An external function of type `((_: bool i32 i32) <-: (string string) raises Error)`.

*compiledfn*{.property} `sc_string_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_string_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_string_new}

:   An external function of type `(string <-: ((@ i8) usize))`.

*compiledfn*{.property} `sc_string_new_from_cstr`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_string_new_from_cstr "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_string_new_from_cstr}

:   An external function of type `(string <-: ((@ i8)))`.

*compiledfn*{.property} `sc_string_rslice`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_string_rslice "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_string_rslice}

:   An external function of type `(string <-: (string usize))`.

*compiledfn*{.property} `sc_strip_qualifiers`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_strip_qualifiers "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_strip_qualifiers}

:   An external function of type `(type <-: (type))`.

*compiledfn*{.property} `sc_switch_append`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_switch_append "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_switch_append}

:   An external function of type `(void <-: (Value Value))`.

*compiledfn*{.property} `sc_switch_append_case`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_switch_append_case "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_switch_append_case}

:   An external function of type `(void <-: (Value Value Value))`.

*compiledfn*{.property} `sc_switch_append_default`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_switch_append_default "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_switch_append_default}

:   An external function of type `(void <-: (Value Value))`.

*compiledfn*{.property} `sc_switch_append_do`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_switch_append_do "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_switch_append_do}

:   An external function of type `(void <-: (Value Value))`.

*compiledfn*{.property} `sc_switch_append_pass`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_switch_append_pass "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_switch_append_pass}

:   An external function of type `(void <-: (Value Value Value))`.

*compiledfn*{.property} `sc_switch_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_switch_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_switch_new}

:   An external function of type `(Value <-: (Value))`.

*compiledfn*{.property} `sc_symbol_count`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_symbol_count "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_symbol_count}

:   An external function of type `(usize <-: ())`.

*compiledfn*{.property} `sc_symbol_is_variadic`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_symbol_is_variadic "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_symbol_is_variadic}

:   An external function of type `(bool <-: (Symbol))`.

*compiledfn*{.property} `sc_symbol_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_symbol_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_symbol_new}

:   An external function of type `(Symbol <-: (string))`.

*compiledfn*{.property} `sc_symbol_new_unique`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_symbol_new_unique "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_symbol_new_unique}

:   An external function of type `(Symbol <-: (string))`.

*compiledfn*{.property} `sc_symbol_style`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_symbol_style "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_symbol_style}

:   An external function of type `(Symbol <-: (Symbol))`.

*compiledfn*{.property} `sc_symbol_to_string`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_symbol_to_string "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_symbol_to_string}

:   An external function of type `(string <-: (Symbol))`.

*compiledfn*{.property} `sc_template_append_parameter`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_template_append_parameter "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_template_append_parameter}

:   An external function of type `(void <-: (Value Value))`.

*compiledfn*{.property} `sc_template_get_name`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_template_get_name "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_template_get_name}

:   An external function of type `(Symbol <-: (Value))`.

*compiledfn*{.property} `sc_template_is_inline`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_template_is_inline "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_template_is_inline}

:   An external function of type `(bool <-: (Value))`.

*compiledfn*{.property} `sc_template_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_template_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_template_new}

:   An external function of type `(Value <-: (Symbol))`.

*compiledfn*{.property} `sc_template_parameter`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_template_parameter "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_template_parameter}

:   An external function of type `(Value <-: (Value i32))`.

*compiledfn*{.property} `sc_template_parameter_count`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_template_parameter_count "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_template_parameter_count}

:   An external function of type `(i32 <-: (Value))`.

*compiledfn*{.property} `sc_template_set_body`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_template_set_body "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_template_set_body}

:   An external function of type `(void <-: (Value Value))`.

*compiledfn*{.property} `sc_template_set_inline`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_template_set_inline "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_template_set_inline}

:   An external function of type `(void <-: (Value))`.

*compiledfn*{.property} `sc_template_set_name`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_template_set_name "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_template_set_name}

:   An external function of type `(void <-: (Value Symbol))`.

*compiledfn*{.property} `sc_tuple_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_tuple_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_tuple_type}

:   An external function of type `(type <-: (i32 (@ type)) raises Error)`.

*compiledfn*{.property} `sc_type_alignof`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_alignof "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_alignof}

:   An external function of type `(usize <-: (type) raises Error)`.

*compiledfn*{.property} `sc_type_at`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_at "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_at}

:   An external function of type `(Value <-: (type Symbol) raises Error)`.

*compiledfn*{.property} `sc_type_bitcountof`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_bitcountof "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_bitcountof}

:   An external function of type `(i32 <-: (type))`.

*compiledfn*{.property} `sc_type_compatible`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_compatible "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_compatible}

:   An external function of type `(bool <-: (type type))`.

*compiledfn*{.property} `sc_type_countof`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_countof "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_countof}

:   An external function of type `(i32 <-: (type) raises Error)`.

*compiledfn*{.property} `sc_type_debug_abi`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_debug_abi "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_debug_abi}

:   An external function of type `(void <-: (type))`.

*compiledfn*{.property} `sc_type_del_symbol`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_del_symbol "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_del_symbol}

:   An external function of type `(void <-: (type Symbol))`.

*compiledfn*{.property} `sc_type_element_at`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_element_at "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_element_at}

:   An external function of type `(type <-: (type i32) raises Error)`.

*compiledfn*{.property} `sc_type_field_index`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_field_index "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_field_index}

:   An external function of type `(i32 <-: (type Symbol) raises Error)`.

*compiledfn*{.property} `sc_type_field_name`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_field_name "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_field_name}

:   An external function of type `(Symbol <-: (type i32) raises Error)`.

*compiledfn*{.property} `sc_type_get_docstring`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_get_docstring "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_get_docstring}

:   An external function of type `(string <-: (type Symbol))`.

*compiledfn*{.property} `sc_type_is_default_suffix`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_is_default_suffix "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_is_default_suffix}

:   An external function of type `(bool <-: (type))`.

*compiledfn*{.property} `sc_type_is_opaque`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_is_opaque "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_is_opaque}

:   An external function of type `(bool <-: (type))`.

*compiledfn*{.property} `sc_type_is_plain`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_is_plain "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_is_plain}

:   An external function of type `(bool <-: (type))`.

*compiledfn*{.property} `sc_type_is_refer`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_is_refer "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_is_refer}

:   An external function of type `(bool <-: (type))`.

*compiledfn*{.property} `sc_type_is_superof`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_is_superof "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_is_superof}

:   An external function of type `(bool <-: (type type))`.

*compiledfn*{.property} `sc_type_is_unsized`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_is_unsized "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_is_unsized}

:   An external function of type `(bool <-: (type) raises Error)`.

*compiledfn*{.property} `sc_type_is_view`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_is_view "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_is_view}

:   An external function of type `(bool <-: (type))`.

*compiledfn*{.property} `sc_type_key`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_key "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_key}

:   An external function of type `((_: Symbol type) <-: (type))`.

*compiledfn*{.property} `sc_type_kind`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_kind "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_kind}

:   An external function of type `(i32 <-: (type))`.

*compiledfn*{.property} `sc_type_local_at`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_local_at "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_local_at}

:   An external function of type `(Value <-: (type Symbol) raises Error)`.

*compiledfn*{.property} `sc_type_next`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_next "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_next}

:   An external function of type `((_: Symbol Value) <-: (type Symbol))`.

*compiledfn*{.property} `sc_type_offsetof`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_offsetof "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_offsetof}

:   An external function of type `(usize <-: (type i32) raises Error)`.

*compiledfn*{.property} `sc_type_set_docstring`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_set_docstring "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_set_docstring}

:   An external function of type `(void <-: (type Symbol string))`.

*compiledfn*{.property} `sc_type_set_symbol`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_set_symbol "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_set_symbol}

:   An external function of type `(void <-: (type Symbol Value))`.

*compiledfn*{.property} `sc_type_sizeof`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_sizeof "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_sizeof}

:   An external function of type `(usize <-: (type) raises Error)`.

*compiledfn*{.property} `sc_type_storage`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_storage "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_storage}

:   An external function of type `(type <-: (type) raises Error)`.

*compiledfn*{.property} `sc_type_string`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_type_string "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_type_string}

:   An external function of type `(string <-: (type))`.

*compiledfn*{.property} `sc_typename_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_typename_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_typename_type}

:   An external function of type `(type <-: (string type) raises Error)`.

*compiledfn*{.property} `sc_typename_type_get_super`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_typename_type_get_super "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_typename_type_get_super}

:   An external function of type `(type <-: (type))`.

*compiledfn*{.property} `sc_typename_type_set_opaque`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_typename_type_set_opaque "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_typename_type_set_opaque}

:   An external function of type `(void <-: (type) raises Error)`.

*compiledfn*{.property} `sc_typename_type_set_storage`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_typename_type_set_storage "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_typename_type_set_storage}

:   An external function of type `(void <-: (type type u32) raises Error)`.

*compiledfn*{.property} `sc_typify`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_typify "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_typify}

:   An external function of type `(Value <-: (Closure i32 (@ type)) raises Error)`.

*compiledfn*{.property} `sc_typify_template`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_typify_template "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_typify_template}

:   An external function of type `(Value <-: (Value i32 (@ type)) raises Error)`.

*compiledfn*{.property} `sc_union_storage_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_union_storage_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_union_storage_type}

:   An external function of type `(type <-: (i32 (@ type)) raises Error)`.

*compiledfn*{.property} `sc_unique_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_unique_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_unique_type}

:   An external function of type `(type <-: (type i32))`.

*compiledfn*{.property} `sc_unquote_new`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_unquote_new "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_unquote_new}

:   An external function of type `(Value <-: (Value))`.

*compiledfn*{.property} `sc_value_anchor`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_value_anchor "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_value_anchor}

:   An external function of type `(Anchor <-: (Value))`.

*compiledfn*{.property} `sc_value_ast_repr`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_value_ast_repr "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_value_ast_repr}

:   An external function of type `(string <-: (Value))`.

*compiledfn*{.property} `sc_value_block_depth`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_value_block_depth "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_value_block_depth}

:   An external function of type `(i32 <-: (Value))`.

*compiledfn*{.property} `sc_value_compare`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_value_compare "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_value_compare}

:   An external function of type `(bool <-: (Value Value))`.

*compiledfn*{.property} `sc_value_content_repr`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_value_content_repr "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_value_content_repr}

:   An external function of type `(string <-: (Value))`.

*compiledfn*{.property} `sc_value_is_constant`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_value_is_constant "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_value_is_constant}

:   An external function of type `(bool <-: (Value))`.

*compiledfn*{.property} `sc_value_is_pure`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_value_is_pure "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_value_is_pure}

:   An external function of type `(bool <-: (Value))`.

*compiledfn*{.property} `sc_value_kind`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_value_kind "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_value_kind}

:   An external function of type `(i32 <-: (Value))`.

*compiledfn*{.property} `sc_value_kind_string`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_value_kind_string "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_value_kind_string}

:   An external function of type `(string <-: (i32))`.

*compiledfn*{.property} `sc_value_qualified_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_value_qualified_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_value_qualified_type}

:   An external function of type `(type <-: (Value))`.

*compiledfn*{.property} `sc_value_repr`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_value_repr "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_value_repr}

:   An external function of type `(string <-: (Value))`.

*compiledfn*{.property} `sc_value_tostring`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_value_tostring "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_value_tostring}

:   An external function of type `(string <-: (Value))`.

*compiledfn*{.property} `sc_value_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_value_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_value_type}

:   An external function of type `(type <-: (Value))`.

*compiledfn*{.property} `sc_value_unwrap`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_value_unwrap "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_value_unwrap}

:   An external function of type `(Value <-: (type Value))`.

*compiledfn*{.property} `sc_value_wrap`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_value_wrap "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_value_wrap}

:   An external function of type `(Value <-: (type Value))`.

*compiledfn*{.property} `sc_valueref_tag`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_valueref_tag "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_valueref_tag}

:   An external function of type `(Value <-: (Anchor Value))`.

*compiledfn*{.property} `sc_vector_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_vector_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_vector_type}

:   An external function of type `(type <-: (type usize) raises Error)`.

*compiledfn*{.property} `sc_view_type`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_view_type "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_view_type}

:   An external function of type `(type <-: (type i32))`.

*compiledfn*{.property} `sc_write`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.sc_write "Permalink to this definition"){.headerlink} {#scopes.compiledfn.sc_write}

:   An external function of type `(void <-: (string))`.

*compiledfn*{.property} `set-globals!`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.set-globals! "Permalink to this definition"){.headerlink} {#scopes.compiledfn.set-globals!}

:   An external function of type `(void <-: (Scope))`.

*compiledfn*{.property} `set-signal-abort!`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.set-signal-abort! "Permalink to this definition"){.headerlink} {#scopes.compiledfn.set-signal-abort!}

:   An external function of type `(void <-: (bool))`.

*compiledfn*{.property} `spice-macro-verify-signature`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.spice-macro-verify-signature "Permalink to this definition"){.headerlink} {#scopes.compiledfn.spice-macro-verify-signature}

:   A compiled function of type `(void <-: ((opaque@ (Value <-: (Value) raises Error))))`.

*compiledfn*{.property} `symbol-handler`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.symbol-handler "Permalink to this definition"){.headerlink} {#scopes.compiledfn.symbol-handler}

:   A compiled function of type `((_: List Scope) <-: (List Scope) raises Error)`.

*compiledfn*{.property} `trap`{.descname} (*&ensp;...&ensp;*)[](#scopes.compiledfn.trap "Permalink to this definition"){.headerlink} {#scopes.compiledfn.trap}

:   An external function of type `(noreturn <-: ())`.

