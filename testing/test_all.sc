
let modules =
    quote
        .test_abi
        .test_ansi_colors
        .test_array
        .test_assorted
        .test_call_override
        .test_callback
        .test_clang
        .test_closure
        .test_conversion
        .test_defer
        .test_dispatch
        .test_docstring
        .test_dots
        .test_enums
        .test_extraparams
        .test_folding
        .test_frame
        .test_from
        .test_fwdecl
        .test_glm
        .test_glsl
        .test_intrinsics
        .test_iter2
        .test_let
        .test_locals
        .test_loop
        .test_match
        .test_memoization
        .test_mutarray
        .test_namedargs
        .test_object
        .test_overload
        .test_quality
        .test_recursion
        .test_reference
        .test_regexp
        .test_scope_iter
        .test_scope
        .test_semicolon
        .test_string
        .test_struct
        .test_submod
        .test_tuple_array
        .test_union
        .test_using
        .test_varargs
        .test_vector
        .test_while
        .test_xpcall

fn run-tests ()
    let total =
        i32 (countof modules)

    let loop (modules failed-modules) = (unconst modules) (unconst '())
    if (empty? modules)
        let failed = (i32 (countof failed-modules))
        if (failed > 0)
            print;
            print "List of failed modules"
            print "======================"
            for m in failed-modules
                print "*" (m as Symbol as string)
        print;
        print total "tests executed," (total - failed) "succeeded," failed "failed."
        print "done."
        return;

    let module modules = (decons modules)
    let module = (module as Symbol)
    print "* running" (module as string)
    print "***********************************************"
    let ok =
        xpcall
            fn ()
                require-from module-dir module
                unconst true
            fn (exc)
                io-write!
                    format-exception exc
                unconst false
    loop modules
        if ok
            failed-modules
        else
            cons module failed-modules

run-tests;
