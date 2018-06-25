#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""globals
    =======

    These names are bound in every fresh module and main program by default.
    Essential symbols are created by the compiler, and subsequent utility
    functions, macros and types are defined and documented in `core.sc`.

    The core module implements the remaining standard functions and macros,
    parses the command-line and optionally enters the REPL.

let
    __typify = sc_typify
    __compile = sc_compile
    __compile-object = sc_compile_object
    __compile-spirv = sc_compile_spirv
    __compile-glsl = sc_compile_glsl
    eval = sc_eval
    compiler-version = sc_compiler_version
    verify-stack! = sc_verify_stack
    enter-solver-cli! = sc_enter_solver_cli
    launch-args = sc_launch_args

    default-styler = sc_default_styler
    io-write! = sc_write
    format-message = sc_format_message
    __prompt = sc_prompt
    set-autocomplete-scope! = sc_set_autocomplete_scope

    file? = sc_is_file
    directory? = sc_is_directory
    realpath = sc_realpath
    dirname = sc_dirname
    basename = sc_basename

    globals = sc_get_globals
    set-globals! = sc_set_globals

    set-error! = sc_set_last_error
    set-location-error! = sc_set_last_location_error
    set-runtime-error! = sc_set_last_runtime_error
    get-error = sc_get_last_error
    format-error = sc_format_error
    CompileError = sc_location_error_new
    RuntimeError = sc_runtime_error_new
    exit = sc_exit
    set-signal-abort! = sc_set_signal_abort

    __hash = sc_hash
    __hash2x64 = sc_hash2x64
    __hashbytes = sc_hashbytes

    set-anchor! = sc_set_active_anchor
    active-anchor = sc_get_active_anchor

    import-c = sc_import_c
    load-library = sc_load_library

    Scope@ = sc_scope_at
    Scope-local@ = sc_scope_local_at
    Scope-docstring = sc_scope_get_docstring
    set-scope-docstring! = sc_scope_set_docstring
    Scope-new = sc_scope_new
    Scope-clone = sc_scope_clone
    Scope-new-expand = sc_scope_new_subscope
    Scope-clone-expand = sc_scope_clone_subscope
    Scope-parent = sc_scope_get_parent
    delete-scope-symbol! = sc_scope_del_symbol
    Scope-next = sc_scope_next

    string->Symbol = sc_symbol_new
    Symbol->string = sc_symbol_to_string

    string-join = sc_string_join
    string-new = sc_string_new
    string-match? = sc_string_match

    Any-repr = sc_any_repr
    Any-string = sc_any_string
    Any== = sc_any_eq

    list-cons = sc_list_cons
    list-join = sc_list_join
    list-dump = sc_list_dump

    Syntax-new = sc_syntax_new
    Syntax-wrap = sc_syntax_wrap
    Syntax-strip = sc_syntax_strip
    list-load = sc_syntax_from_path
    list-parse = sc_syntax_from_string

    element-type = sc_type_element_at
    type-countof = sc_type_countof
    sizeof = sc_type_sizeof
    runtime-type@ = sc_type_at
    element-index = sc_type_field_index
    element-name = sc_type_field_name
    type-kind = sc_type_kind
    storageof = sc_type_storage
    opaque? = sc_type_is_opaque
    type-name = sc_type_string
    type-next = sc_type_next
    set-type-symbol! = sc_type_set_symbol

    pointer-type = sc_pointer_type
    pointer-type-set-element-type = sc_pointer_type_set_element_type
    pointer-type-set-storage-class = sc_pointer_type_set_storage_class
    pointer-type-set-flags = sc_pointer_type_set_flags
    pointer-type-flags = sc_pointer_type_get_flags
    pointer-type-set-storage-class = sc_pointer_type_set_storage_class
    pointer-type-storage-class = sc_pointer_type_get_storage_class

    extern-type-location = sc_extern_type_location
    extern-type-binding = sc_extern_type_binding

    bitcountof = sc_type_bitcountof

    integer-type = sc_integer_type
    signed? = sc_integer_type_is_signed

    typename-type = sc_typename_type
    set-typename-super! = sc_typename_type_set_super
    superof = sc_typename_type_get_super
    set-typename-storage! = sc_typename_type_set_storage

    array-type = sc_array_type
    vector-type = sc_vector_type

    function-type-variadic? = sc_function_type_is_variadic

    Image-type = sc_image_type
    SampledImage-type = sc_sampled_image_type

fn odd?

fn even? (n)
    if (icmp!= n 0)
        odd? (sub n 1)
    else true
fn odd? (n)
    if (icmp!= n 0)
        even? (sub n 1)
    else false

if (even? 23)
    sc_write "even\n"
else
    sc_write "odd\n"

fn tuples (u...)
    sc_write "tuples!\n"
    _ 1 2 u...

if false
    sc_write "one!\n"
elseif false
    sc_write "two!\n"
else
    sc_write "yup!\n"

let x = (tuples 5 6 7 8)
dump x
