bang

(dump-module)

import-c bang0 ./bang0.cpp (
    -DBANG_HEADER_ONLY
    -I./clang/lib/clang/3.8.0/include
    )

extern LLVMVoidType
    function-type (pointer-type i8) ()

extern dlopen
    function-type (pointer-type i8) ((pointer-type i8) i32)

extern printf
    function-type i32 ((pointer-type i8) ...)
extern sin
    function-type double (double)


function computesin ()
    function-type double ()
    call sin
        const-real double 0.5

function dostuff-hyphenated (a b)
    function-type i32 (i32 i32)
    do
        ?
            const-int i1 0
            ()
            call printf
                array-ref "value = %f! (false) %i %i\n"
                call computesin
                \ a b
    const-int i32 0

call dostuff-hyphenated
    const-int i32 1
    const-int i32 2

call printf
    array-ref "Hello World! %p\n"
    call bang_parse_file
        array-ref "bang1.b"
