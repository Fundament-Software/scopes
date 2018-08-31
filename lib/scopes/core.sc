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
    raise (CompileError value)

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
        verify-count argcount 1 -1
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
                    sc_string_join
                        sc_value_repr (box-pointer ASTMacroFunction)
                        sc_string_join " but has type "
                            sc_value_repr (box-pointer result-type)
        let ptr = (sc_const_pointer_extract result)
        let result =
            sc_const_pointer_new ASTMacro ptr
        sc_scope_set_symbol syntax-scope 'typify result

    syntax-scope

let function->ASTMacro =
    typify
        fn "function->ASTMacro" (f)
            bitcast f ASTMacro
        ASTMacroFunction

fn box-empty ()
    sc_argument_list_new 0 (undef ValueArrayPointer)

fn box-none ()
    sc_const_tuple_new Nothing 0 (undef ValueArrayPointer)

# take closure l, typify and compile it and return an Any of LabelMacro type
inline ast-macro (l)
    function->ASTMacro (typify l ValueArrayPointer i32)

inline box-ast-macro (l)
    box-pointer (ast-macro l)

syntax-extend
    fn va-lfold (args argcount use-indices)
        verify-count argcount 1 -1
        let f = (load (getelementptr args 0))
        if (icmp== argcount 1)
            return (box-empty)
        let ofs = (? use-indices 1 0)
        let callargs = (alloca-array Value (add 3 ofs))
        loop (i ret) = 1 (undef Value)
        if (icmp<s i argcount)
            let arg =
                load (getelementptr args i)
            # optional index
            if use-indices
                store (box-integer (sub i 1)) (getelementptr callargs 0)
            # key
            store (box-symbol unnamed) (getelementptr callargs (add ofs 0))
            # value
            store arg (getelementptr callargs (add ofs 1))
            let callargcount =
                add ofs
                    if (icmp>s i 1)
                        # append previous result
                        store ret (getelementptr callargs (add ofs 2))
                        3
                    else 2
            repeat (add i 1)
                sc_call_new f callargcount callargs
        ret

    fn va-rfold (args argcount use-indices)
        verify-count argcount 1 -1
        let f = (load (getelementptr args 0))
        if (icmp== argcount 1)
            return (box-empty)
        let ofs = (? use-indices 1 0)
        let callargs = (alloca-array Value (add 3 ofs))
        loop (i ret) = argcount (undef Value)
        if (icmp>s i 1)
            let oi = i
            let i = (sub i 1)
            let arg =
                load (getelementptr args i)
            sc_write (sc_value_repr arg)

            # optional index
            if use-indices
                store (box-integer (sub i 1)) (getelementptr callargs 0)
            # key
            store (box-symbol unnamed) (getelementptr callargs (add ofs 0))
            # value
            store arg (getelementptr callargs (add ofs 1))
            let callargcount =
                add ofs
                    if (icmp!= oi argcount)
                        # append previous result
                        store ret (getelementptr callargs (add ofs 2))
                        3
                    else 2
            repeat i
                sc_call_new f callargcount callargs
        ret

    sc_scope_set_symbol syntax-scope 'va-lfold (box-ast-macro (fn "va-lfold" (args argcount) (va-lfold args argcount false)))
    sc_scope_set_symbol syntax-scope 'va-lifold (box-ast-macro (fn "va-ilfold" (args argcount) (va-lfold args argcount true)))
    sc_scope_set_symbol syntax-scope 'va-rfold (box-ast-macro (fn "va-rfold" (args argcount) (va-rfold args argcount false)))
    sc_scope_set_symbol syntax-scope 'va-rifold (box-ast-macro (fn "va-rifold" (args argcount) (va-rfold args argcount true)))

    fn type< (T superT)
        loop (T) = T
        let value = (sc_typename_type_get_super T)
        if (ptrcmp== value superT) true
        elseif (ptrcmp== value typename) false
        else (repeat value)

    fn type<= (T superT)
        if (ptrcmp== T superT) true
        else (type< T superT)

    fn type> (superT T)
        bxor (type<= T superT) true

    fn type>= (superT T)
        bxor (type< T superT) true

    fn compare-type (args argcount f)
        verify-count argcount 2 2
        let a = (load (getelementptr args 0))
        let b = (load (getelementptr args 1))
        if (sc_value_is_constant a)
            if (sc_value_is_constant b)
                return
                    box-integer
                        f (unbox-pointer a type) (unbox-pointer b type)
        sc_call_new (box-pointer f) 2 args

    inline type-comparison-func (f)
        fn (args argcount) (compare-type args argcount (typify f type type))

    sc_scope_set_symbol syntax-scope 'type== (box-ast-macro (type-comparison-func ptrcmp==))
    sc_scope_set_symbol syntax-scope 'type!= (box-ast-macro (type-comparison-func ptrcmp!=))
    sc_scope_set_symbol syntax-scope 'type< (box-ast-macro (type-comparison-func type<))
    sc_scope_set_symbol syntax-scope 'type<= (box-ast-macro (type-comparison-func type<=))
    sc_scope_set_symbol syntax-scope 'type> (box-ast-macro (type-comparison-func type>))
    sc_scope_set_symbol syntax-scope 'type>= (box-ast-macro (type-comparison-func type>=))

    # typecall
    sc_type_set_symbol type '__call
        box-ast-macro
            fn "type-call" (args argcount)
                verify-count argcount 1 -1
                let self = (load (getelementptr args 0))
                let T = (unbox-pointer self type)
                let ok f = (sc_type_at T '__typecall)
                if ok
                    return
                        sc_call_new f argcount args
                raise-compile-error!
                    sc_string_join "no type constructor available for type "
                        sc_value_repr self

    # method call syntax
    sc_type_set_symbol Symbol '__call
        box-ast-macro
            fn "symbol-call" (args argcount)
                verify-count argcount 2 -1
                let symval = (load (getelementptr args 0))
                let sym = (unbox-symbol symval Symbol)
                let self = (load (getelementptr args 1))
                let T = (sc_value_type self)
                let ok f = (sc_type_at T sym)
                if ok
                    sc_call_new f (sub argcount 1) (getelementptr args 1)
                else
                    raise-compile-error!
                        sc_string_join "no method named "
                            sc_string_join (sc_value_repr symval)
                                sc_string_join " in value of type "
                                    sc_value_repr (box-pointer T)

    inline gen-key-any-set (selftype fset)
        box-ast-macro
            fn "set-symbol" (args argcount)
                verify-count argcount 2 3
                let self = (load (getelementptr args 0))
                let key value =
                    if (icmp== argcount 3)
                        let key = (load (getelementptr args 1))
                        let value = (load (getelementptr args 2))
                        _ key value
                    else
                        let key value = (sc_key_value (load (getelementptr args 1)))
                        _ (box-symbol key) value
                if (sc_value_is_constant self)
                    if (sc_value_is_constant key)
                        if (sc_value_is_constant value)
                            let self = (unbox-pointer self selftype)
                            let key = (unbox-symbol key Symbol)
                            fset self key value
                            return (box-empty)
                let callargs = (alloca-array Value 3)
                store self (getelementptr callargs 0)
                store key (getelementptr callargs 1)
                store value (getelementptr callargs 2)
                sc_call_new (box-pointer fset) 3 callargs

    # quick assignment of type attributes
    sc_type_set_symbol type 'set-symbol (gen-key-any-set type sc_type_set_symbol)
    sc_type_set_symbol Scope 'set-symbol (gen-key-any-set Scope sc_scope_set_symbol)

    sc_type_set_symbol type 'pointer
        box-ast-macro
            fn "type-pointer" (args argcount)
                verify-count argcount 1 1
                let self = (load (getelementptr args 0))
                let T = (unbox-pointer self type)
                box-pointer
                    sc_pointer_type T pointer-flag-non-writable unnamed

    # tuple type constructor
    sc_type_set_symbol tuple '__typecall
        box-ast-macro
            fn "tuple" (args argcount)
                verify-count argcount 1 -1
                let pcount = (sub argcount 1)
                let types = (alloca-array type pcount)
                loop (i) = 1
                if (icmp<s i argcount)
                    let arg = (load (getelementptr args i))
                    let T = (unbox-pointer arg type)
                    store T (getelementptr types (sub i 1))
                    repeat (add i 1)
                box-pointer (sc_tuple_type pcount types)

    # function pointer type constructor
    sc_type_set_symbol function '__typecall
        box-ast-macro
            fn "function" (args argcount)
                verify-count argcount 2 -1
                let rtype = (load (getelementptr args 1))
                let rtype = (unbox-pointer rtype type)
                let pcount = (sub argcount 2)
                let types = (alloca-array type pcount)
                loop (i) = 2
                if (icmp<s i argcount)
                    let arg = (load (getelementptr args i))
                    let T = (unbox-pointer arg type)
                    store T (getelementptr types (sub i 2))
                    repeat (add i 1)
                box-pointer (sc_function_type rtype pcount types)

    sc_type_set_symbol type 'raising
        box-ast-macro
            fn "function-raising" (args argcount)
                verify-count argcount 2 2
                let self = (load (getelementptr args 0))
                let except_type = (load (getelementptr args 1))
                let T = (unbox-pointer self type)
                let exceptT = (unbox-pointer except_type type)
                box-pointer
                    sc_function_type_raising T exceptT

    # closure constructor
    #sc_type_set_symbol Closure '__typecall
        box-pointer
            inline (cls func frame)
                sc_closure_new func frame

    # symbol constructor
    sc_type_set_symbol Symbol '__typecall
        box-pointer
            inline (cls str)
                sc_symbol_new str

    sc_scope_set_symbol syntax-scope 'none?
        box-ast-macro
            fn (args argcount)
                verify-count argcount 1 1
                let value = (load (getelementptr args 0))
                box-integer
                    ptrcmp== (sc_value_type value) Nothing

    fn unpack2 (args argcount)
        verify-count argcount 2 2
        let a = (load (getelementptr args 0))
        let b = (load (getelementptr args 1))
        return a b

    sc_scope_set_symbol syntax-scope 'const.icmp<=.i32.i32
        box-ast-macro
            fn (args argcount)
                let a b = (unpack2 args argcount)
                if (sc_value_is_constant a)
                    if (sc_value_is_constant b)
                        let a = (unbox-integer a i32)
                        let b = (unbox-integer b i32)
                        return
                            box-integer (icmp<=s a b)
                raise-compile-error! "arguments must be constant"

    sc_scope_set_symbol syntax-scope 'const.add.i32.i32
        box-ast-macro
            fn (args argcount)
                let a b = (unpack2 args argcount)
                if (sc_value_is_constant a)
                    if (sc_value_is_constant b)
                        let a = (unbox-integer a i32)
                        let b = (unbox-integer b i32)
                        return
                            box-integer (add a b)
                raise-compile-error! "arguments must be constant"

    sc_scope_set_symbol syntax-scope 'constbranch
        box-ast-macro
            fn (args argcount)
                verify-count argcount 3 3
                let cond = (load (getelementptr args 0))
                let thenf = (load (getelementptr args 1))
                let elsef = (load (getelementptr args 2))
                if (sc_value_is_constant cond)
                else
                    raise-compile-error! "condition must be constant"
                let value = (unbox-integer cond bool)
                sc_call_new
                    ? value thenf elsef
                    0
                    undef ValueArrayPointer

    sc_type_set_symbol Value '__typecall
        box-ast-macro
            fn (args argcount)
                verify-count argcount 2 2
                let value = (load (getelementptr args 1))
                let T = (sc_value_type value)
                inline make-none ()
                    Any-wrap none
                if (ptrcmp== T Value)
                    value
                elseif (sc_value_is_constant value)
                    box-pointer value
                elseif (ptrcmp== T Nothing)
                    let blockargs = (alloca-array Value 2)
                    store value (getelementptr blockargs 0)
                    store (box-none) (getelementptr blockargs 1)
                    sc_block_new 2 blockargs
                else
                    let storageT = (sc_type_storage T)
                    let kind = (sc_type_kind storageT)
                    let argptr = (getelementptr args 1)
                    if (icmp== kind type-kind-pointer)
                        sc_call_new (box-pointer box-pointer) 1 argptr
                    elseif (icmp== kind type-kind-integer)
                        sc_call_new (box-pointer box-integer) 1 argptr
                    #elseif (bor (icmp== kind type-kind-tuple) (icmp== kind type-kind-array))
                    #elseif (icmp== kind type-kind-vector)
                    #elseif (icmp== kind type-kind-real)
                    else
                        raise-compile-error!
                            sc_string_join "can't box value of type "
                                sc_value_repr (box-pointer T)

    syntax-scope

