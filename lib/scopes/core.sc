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

    list-cons = sc_list_cons
    list-join = sc_list_join
    list-dump = sc_list_dump

    list-load = sc_parse_from_path
    list-parse = sc_parse_from_string

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

# first we alias u64 to the integer type that can hold a pointer
let intptr = u64

# pointer comparison as a template function, because we'll compare pointers of many types
fn ptrcmp!= (t1 t2)
    icmp!= (ptrtoint t1 intptr) (ptrtoint t2 intptr)

fn ptrcmp== (t1 t2)
    icmp== (ptrtoint t1 intptr) (ptrtoint t2 intptr)

fn box-integer (value)
    let T = (typeof value)
    sc_const_int_new T
        if (sc_integer_type_is_signed T)
            sext value u64
        else
            zext value u64

# turn a symbol-like value (storage type u64) to an Any
fn box-symbol (value)
    sc_const_int_new (typeof value)
        bitcast value u64

# turn a pointer value into an Any
fn box-pointer (value)
    sc_const_pointer_new (typeof value)
        bitcast value voidstar

fn raise-compile-error! (value)
    set-error! (CompileError value)
    __raise!;

# print an unboxing error given two types
fn unbox-verify (haveT wantT)
    if (ptrcmp!= haveT wantT)
        raise-compile-error!
            sc_string_join "can't unbox value of type "
                sc_string_join
                    sc_value_repr (box-pointer haveT)
                    sc_string_join " as value of type "
                        sc_value_repr (box-pointer wantT)

inline unbox-integer (value T)
    unbox-verify (sc_value_type value) T
    itrunc (sc_const_int_extract value) T

inline unbox-symbol (value T)
    unbox-verify (sc_value_type value) T
    bitcast (sc_const_int_extract value) T

# turn an Any back into a pointer
inline unbox-pointer (value T)
    unbox-verify (sc_value_type value) T
    bitcast (sc_const_pointer_extract value) T

fn verify-count (count mincount maxcount)
    if (icmp>=s mincount 0)
        if (icmp<s count mincount)
            raise-compile-error!
                sc_string_join "at least "
                    sc_string_join (sc_value_repr (box-integer mincount))
                        sc_string_join " argument(s) expected, got "
                            sc_value_repr (box-integer count)
    if (icmp>=s maxcount 0)
        if (icmp>s count maxcount)
            raise-compile-error!
                sc_string_join "at most "
                    sc_string_join (sc_value_repr (box-integer maxcount))
                        sc_string_join " argument(s) expected, got "
                            sc_value_repr (box-integer count)

fn Any-none? (value)
    let T = (extractvalue value 0)
    ptrcmp== T Nothing

syntax-extend
    let TypeArrayPointer =
        box-pointer (sc_pointer_type type pointer-flag-non-writable unnamed)
    let ValueArrayPointer =
        box-pointer (sc_pointer_type Value pointer-flag-non-writable unnamed)
    sc_scope_set_symbol syntax-scope 'TypeArrayPointer TypeArrayPointer
    sc_scope_set_symbol syntax-scope 'ValueArrayPointer ValueArrayPointer
    let T = (sc_type_storage ASTMacro)
    sc_scope_set_symbol syntax-scope 'ASTMacroFunction (box-pointer T)
    sc_scope_set_symbol syntax-scope 'ellipsis-symbol (box-symbol (sc_symbol_new "..."))
    syntax-scope

syntax-extend
    fn typify (args argcount)
        let args = (verify-count argcount 1 -1)
        dump args
        let src_fn = (load (getelementptr args 0))
        let src_fn = (unbox-pointer src_fn Closure)
        let typecount = (sub argcount 1)
        let types = (alloca-array type typecount)
        loop (i j) = 1 0
        if (icmp<s i argcount)
            let ty = (load (getelementptr args i))
            store (unbox-pointer ty type) (getelementptr types j)
            repeat (add i 1) (add j 1)
        sc_typify src_fn typecount (bitcast types TypeArrayPointer)

    do
        let types = (alloca-array type 2:usize)
        store ValueArrayPointer (getelementptr types 0)
        store i32 (getelementptr types 1)
        let types = (bitcast types TypeArrayPointer)
        let result = (sc_compile (sc_typify typify 2 types) 0:u64)
        let result-type = (sc_value_type result)
        if (ptrcmp!= result-type ASTMacroFunction)
            raise-compile-error!
                sc_string_join "AST macro must have type "
                    sc_value_repr (box-pointer ASTMacroFunction)
        let ptr = (sc_const_pointer_extract result)
        let result =
            sc_const_pointer_new ASTMacroFunction ptr
        sc_scope_set_symbol syntax-scope 'typify result

    syntax-scope
