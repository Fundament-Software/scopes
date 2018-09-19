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

fn Value-none? (value)
    ptrcmp== (sc_value_type value) Nothing

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
    fn typify (argcount args)
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
        store i32 (getelementptr types 0)
        store ValueArrayPointer (getelementptr types 1)
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

# take closure l, typify and compile it and return a function of ASTMacro type
inline ast-macro (l)
    function->ASTMacro (typify l i32 ValueArrayPointer)

inline box-ast-macro (l)
    box-pointer (ast-macro l)

syntax-extend
    fn va-lfold (argcount args use-indices)
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
            let k = (sc_type_key (sc_value_type arg))
            let v = (sc_keyed_new unnamed arg)
            # key
            store (box-symbol k) (getelementptr callargs (add ofs 0))
            # value
            store v (getelementptr callargs (add ofs 1))
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

    fn va-rfold (argcount args use-indices)
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
            # optional index
            if use-indices
                store (box-integer (sub i 1)) (getelementptr callargs 0)
            let k = (sc_type_key (sc_value_type arg))
            let v = (sc_keyed_new unnamed arg)
            # key
            store (box-symbol k) (getelementptr callargs (add ofs 0))
            # value
            store v (getelementptr callargs (add ofs 1))
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

    sc_scope_set_symbol syntax-scope 'va-lfold (box-ast-macro (fn "va-lfold" (argcount args) (va-lfold argcount args false)))
    sc_scope_set_symbol syntax-scope 'va-lifold (box-ast-macro (fn "va-ilfold" (argcount args) (va-lfold argcount args true)))
    sc_scope_set_symbol syntax-scope 'va-rfold (box-ast-macro (fn "va-rfold" (argcount args) (va-rfold argcount args false)))
    sc_scope_set_symbol syntax-scope 'va-rifold (box-ast-macro (fn "va-rifold" (argcount args) (va-rfold argcount args true)))

    # generate alloca instruction for multiple Values
    sc_scope_set_symbol syntax-scope 'Value-array
        box-ast-macro
            fn "Value-array" (argc argv)
                verify-count argc 1 -1
                # ensure that the return signature is correct
                let instr = (alloca-array Value (add argc 2))
                let callargs = (alloca-array Value 2)
                let boxed-argc = (box-integer argc)
                store (box-pointer Value) (getelementptr callargs 0)
                store boxed-argc (getelementptr callargs 1)
                let arr = (sc_call_new (box-symbol alloca-array) 2 callargs)
                store arr (getelementptr instr 0)
                loop (i) = 0
                if (icmp<s i argc)
                    let gepargs = (alloca-array Value 2)
                    store arr (getelementptr gepargs 0)
                    store (box-integer i) (getelementptr gepargs 1)
                    let storeargs = (alloca-array Value 2)
                    store (load (getelementptr argv i)) (getelementptr storeargs 0)
                    store
                        sc_call_new (box-symbol getelementptr) 2 gepargs
                        getelementptr storeargs 1
                    store
                        sc_call_new (box-symbol store) 2 storeargs
                        getelementptr instr (add i 1)
                    repeat (add i 1)
                let retargs = (alloca-array Value 2)
                store boxed-argc (getelementptr retargs 0)
                store arr (getelementptr retargs 1)
                store
                    sc_argument_list_new 2 retargs
                    getelementptr instr (add argc 1)
                sc_block_new (add argc 2) instr

    # unpack
    sc_scope_set_symbol syntax-scope 'loadarrayptrs
        box-ast-macro
            fn "unpack-array" (argc argv)
                verify-count argc 2 -1
                let src = (load (getelementptr argv 0))
                let instr = (alloca-array Value (sub argc 1))
                loop (i) = 1
                if (icmp<s i argc)
                    let gepargs = (alloca-array Value 2)
                    store src (getelementptr gepargs 0)
                    store (load (getelementptr argv i)) (getelementptr gepargs 1)
                    let loadargs = (alloca-array Value 1)
                    store
                        sc_call_new (box-symbol getelementptr) 2 gepargs
                        getelementptr loadargs 0
                    store
                        sc_call_new (box-symbol load) 1 loadargs
                        getelementptr instr (sub i 1)
                    repeat (add i 1)
                sc_argument_list_new (sub argc 1) instr

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

    fn compare-type (argcount args f)
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
        fn (argcount args) (compare-type argcount args (typify f type type))

    sc_scope_set_symbol syntax-scope 'type== (box-ast-macro (type-comparison-func ptrcmp==))
    sc_scope_set_symbol syntax-scope 'type!= (box-ast-macro (type-comparison-func ptrcmp!=))
    sc_scope_set_symbol syntax-scope 'type< (box-ast-macro (type-comparison-func type<))
    sc_scope_set_symbol syntax-scope 'type<= (box-ast-macro (type-comparison-func type<=))
    sc_scope_set_symbol syntax-scope 'type> (box-ast-macro (type-comparison-func type>))
    sc_scope_set_symbol syntax-scope 'type>= (box-ast-macro (type-comparison-func type>=))

    # typecall
    sc_type_set_symbol type '__call
        box-ast-macro
            fn "type-call" (argcount args)
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
            fn "symbol-call" (argcount args)
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
            fn "set-symbol" (argcount args)
                verify-count argcount 2 3
                let self = (load (getelementptr args 0))
                let key value =
                    if (icmp== argcount 3)
                        let key = (load (getelementptr args 1))
                        let value = (load (getelementptr args 2))
                        _ key value
                    else
                        let arg = (load (getelementptr args 1))
                        let key = (sc_type_key (sc_value_type arg))
                        _ (box-symbol key) arg
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
            fn "type-pointer" (argcount args)
                verify-count argcount 1 1
                let self = (load (getelementptr args 0))
                let T = (unbox-pointer self type)
                box-pointer
                    sc_pointer_type T pointer-flag-non-writable unnamed

    # tuple type constructor
    sc_type_set_symbol tuple '__typecall
        box-ast-macro
            fn "tuple" (argcount args)
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

    # arguments type constructor
    sc_type_set_symbol Arguments '__typecall
        box-ast-macro
            fn "Arguments" (argcount args)
                verify-count argcount 1 -1
                let pcount = (sub argcount 1)
                let types = (alloca-array type pcount)
                loop (i) = 1
                if (icmp<s i argcount)
                    let arg = (load (getelementptr args i))
                    let T = (unbox-pointer arg type)
                    store T (getelementptr types (sub i 1))
                    repeat (add i 1)
                box-pointer (sc_arguments_type pcount types)

    # function pointer type constructor
    sc_type_set_symbol function '__typecall
        box-ast-macro
            fn "function" (argcount args)
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
            fn "function-raising" (argcount args)
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
            fn (argcount args)
                verify-count argcount 1 1
                let value = (load (getelementptr args 0))
                box-integer
                    ptrcmp== (sc_value_type value) Nothing

    fn unpack2 (argcount args)
        verify-count argcount 2 2
        let a = (load (getelementptr args 0))
        let b = (load (getelementptr args 1))
        return a b

    sc_scope_set_symbol syntax-scope 'const.icmp<=.i32.i32
        box-ast-macro
            fn (argcount args)
                let a b = (unpack2 argcount args)
                if (sc_value_is_constant a)
                    if (sc_value_is_constant b)
                        let a = (unbox-integer a i32)
                        let b = (unbox-integer b i32)
                        return
                            box-integer (icmp<=s a b)
                raise-compile-error! "arguments must be constant"

    sc_scope_set_symbol syntax-scope 'const.add.i32.i32
        box-ast-macro
            fn (argcount args)
                let a b = (unpack2 argcount args)
                if (sc_value_is_constant a)
                    if (sc_value_is_constant b)
                        let a = (unbox-integer a i32)
                        let b = (unbox-integer b i32)
                        return
                            box-integer (add a b)
                raise-compile-error! "arguments must be constant"

    sc_scope_set_symbol syntax-scope 'constbranch
        box-ast-macro
            fn (argcount args)
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
            fn (argcount args)
                verify-count argcount 2 2
                let value = (load (getelementptr args 1))
                let T = (sc_value_type value)
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

fn cons (values...)
    va-rifold
        inline (i key value next)
            constbranch (none? next)
                inline ()
                    value
                inline ()
                    sc_list_cons (Value value) next
        values...

fn make-list (values...)
    constbranch (const.icmp<=.i32.i32 (va-countof values...) 0)
        inline () '()
        inline ()
            va-rifold
                inline (i key value next)
                    sc_list_cons (Value value)
                        constbranch (none? next)
                            inline () '()
                            inline () next
                values...

inline decons (self count)
    let count =
        constbranch (none? count)
            inline () 1
            inline () count
    let at next = (sc_list_decons self)
    _ at
        constbranch (const.icmp<=.i32.i32 count 1)
            inline () next
            inline () (decons next (const.add.i32.i32 count -1))

inline set-symbols (self values...)
    va-lfold
        inline (key value)
            'set-symbol self key value
        values...

'set-symbol type 'set-symbols set-symbols
'set-symbol Scope 'set-symbols set-symbols

'set-symbols Value
    constant? = sc_value_is_constant
    none? = (typify Value-none? Value)
    __repr = sc_value_repr
    typeof = sc_value_type

'set-symbols Scope
    @ = sc_scope_at

'set-symbols string
    join = sc_string_join

'set-symbols list
    __countof = sc_list_count
    join = sc_list_join
    @ = sc_list_at
    next = sc_list_next
    decons = decons
    reverse = sc_list_reverse

# label accessors
#'set-symbols Label
    verify-argument-count = verify-argument-count
    verify-keyed-count = verify-keyed-count
    keyed = sc_label_get_keyed
    set-keyed = sc_label_set_keyed
    arguments = sc_label_get_arguments
    set-arguments = sc_label_set_arguments
    parameters = sc_label_get_parameters
    enter = sc_label_get_enter
    set-enter = sc_label_set_enter
    dump = sc_label_dump
    function-type = sc_label_function_type
    set-rawcall = sc_label_set_rawcall
    set-rawcont = sc_label_set_rawcont
    frame = sc_label_frame
    anchor = sc_label_anchor
    body-anchor = sc_label_body_anchor
    append-parameter = sc_label_append_parameter
    return-keyed = Label-return-keyed
    return = Label-return

'set-symbols type
    bitcount = sc_type_bitcountof
    signed? = sc_integer_type_is_signed
    element@ = sc_type_element_at
    element-count = sc_type_countof
    storage = sc_type_storage
    kind = sc_type_kind
    @ = sc_type_at
    opaque? = sc_type_is_opaque
    string = sc_type_string
    super = sc_typename_type_get_super
    set-super = sc_typename_type_set_super
    set-storage = sc_typename_type_set_storage

#'set-symbols Closure
    frame = sc_closure_frame
    label = sc_closure_label

let rawstring = ('pointer i8)

inline box-cast-dispatch (f)
    box-pointer (typify f type type)

inline not (value)
    bxor value true

syntax-extend
    # a supertype to be used for conversions
    let immutable = (sc_typename_type "immutable")
    sc_typename_type_set_super integer immutable
    sc_typename_type_set_super real immutable
    sc_typename_type_set_super vector immutable
    sc_typename_type_set_super Symbol immutable
    sc_typename_type_set_super CEnum immutable

    let aggregate = (sc_typename_type "aggregate")
    sc_typename_type_set_super array aggregate
    sc_typename_type_set_super tuple aggregate

    let opaquepointer = (sc_typename_type "opaquepointer")
    sc_typename_type_set_super string opaquepointer
    sc_typename_type_set_super type opaquepointer

    sc_typename_type_set_super usize integer

    # syntax macro type
    let SyntaxMacro = (sc_typename_type "SyntaxMacro")
    let SyntaxMacroFunctionType =
        'pointer
            'raising
                function (Arguments list Scope) list list Scope
                Error
    sc_typename_type_set_storage SyntaxMacro SyntaxMacroFunctionType

    # any extraction

    inline unbox-u32 (value T)
        unbox-verify (extractvalue value 0) T
        bitcast (itrunc (extractvalue value 1) u32) T

    inline unbox-bitcast (value T)
        unbox-verify (extractvalue value 0) T
        bitcast (extractvalue value 1) T

    inline unbox-hidden-pointer (value T)
        unbox-verify (extractvalue value 0) T
        load (inttoptr (extractvalue value 1) ('pointer T))

    fn value-imply (vT T)
        let storageT = ('storage T)
        let kind = ('kind storageT)
        if (icmp== kind type-kind-pointer)
            return (box-pointer unbox-pointer)
        elseif (icmp== kind type-kind-integer)
            return (box-pointer unbox-integer)
        elseif (icmp== kind type-kind-real)
            if (ptrcmp== storageT f32)
                return (box-pointer unbox-u32)
            elseif (ptrcmp== storageT f64)
                return (box-pointer unbox-bitcast)
        elseif (bor (icmp== kind type-kind-tuple) (icmp== kind type-kind-array))
            return (box-pointer unbox-hidden-pointer)
        raise-compile-error! "unsupported type"

    inline value-rimply (cls value)
        Value value

    'set-symbols Value
        __imply =
            box-cast-dispatch value-imply
        __rimply =
            box-cast-dispatch
                fn "syntax-imply" (T vT)
                    if true
                        return (Value value-rimply)
                    raise-compile-error! "unsupported type"

    # integer casting

    fn integer-imply (vT T)
        let T =
            if (ptrcmp== T usize) ('storage T)
            else T
        if (icmp== ('kind T) type-kind-integer)
            let valw = ('bitcount vT)
            let destw = ('bitcount T)
            # must have same signed bit
            if (icmp== ('signed? vT) ('signed? T))
                if (icmp== destw valw)
                    return (box-symbol bitcast)
                elseif (icmp>s destw valw)
                    if ('signed? vT)
                        return (box-symbol sext)
                    else
                        return (box-symbol zext)
        raise-compile-error! "unsupported type"

    fn integer-as (vT T)
        let T =
            if (ptrcmp== T usize) ('storage T)
            else T
        if (icmp== ('kind T) type-kind-integer)
            let valw = ('bitcount vT)
            let destw = ('bitcount T)
            if (icmp== destw valw)
                return (box-symbol bitcast)
            elseif (icmp>s destw valw)
                if ('signed? vT)
                    return (box-symbol sext)
                else
                    return (box-symbol zext)
            else
                return (box-symbol itrunc)
        elseif (icmp== ('kind T) type-kind-real)
            if ('signed? vT)
                return (box-symbol sitofp)
            else
                return (box-symbol uitofp)
        raise-compile-error! "unsupported type"

    inline box-binary-op-dispatch (f)
        box-pointer (typify f type type)
    inline single-binary-op-dispatch (destf)
        fn (lhsT rhsT)
            if (ptrcmp== lhsT rhsT)
                return (Value destf)
            raise-compile-error! "unsupported type"

    inline gen-cast-error (intro-string)
        ast-macro
            fn "cast-error" (argc argv)
                verify-count argc 2 2
                let value T = (loadarrayptrs argv 0 1)
                let vT = ('typeof value)
                let T = (unbox-pointer T type)
                # create branch so we can trick the function into assuming
                    there's another exit path
                if true
                    raise-compile-error!
                        sc_string_join intro-string
                            sc_string_join
                                '__repr (box-pointer vT)
                                sc_string_join " to type "
                                    '__repr (box-pointer T)
                undef Value

    let DispatchCastFunctionType =
        'pointer ('raising (function Value type type) Error)

    fn unbox-dispatch-cast-function-type (anyf)
        unbox-pointer anyf DispatchCastFunctionType

    fn attribute-format-error! (T symbol err)
        raise-compile-error!
            'join "wrong format for attribute "
                'join ('__repr (box-symbol symbol))
                    'join " of type "
                        'join ('__repr (box-pointer T))
                            'join ": "
                                format-error err

    fn get-cast-dispatcher (symbol rsymbol vT T)
        let ok anyf = ('@ vT symbol)
        if ok
            let f =
                try
                    unbox-dispatch-cast-function-type anyf
                except (err)
                    attribute-format-error! vT symbol err
            try
                return true (f vT T) false
            except (err)
                # ignore
        let ok anyf = ('@ T rsymbol)
        if ok
            let f =
                try
                    unbox-dispatch-cast-function-type anyf
                except (err)
                    attribute-format-error! T rsymbol err
            try
                return true (f T vT) true
            except (err)
                # ignore
        return false (undef Value) false

    #syntax-extend
        let types = (alloca-array type 4:usize)
        store Symbol (getelementptr types 0)
        store Symbol (getelementptr types 1)
        store type (getelementptr types 2)
        store type (getelementptr types 3)
        let result = (sc_compile (sc_typify get-cast-dispatcher 4 types)
            compile-flag-dump-module)
        exit 0
        syntax-scope

    fn implyfn (vT T)
        get-cast-dispatcher '__imply '__rimply vT T
    fn asfn (vT T)
        get-cast-dispatcher '__as '__ras vT T

    let imply =
        box-ast-macro
            fn "imply-dispatch" (argc argv)
                verify-count argc 2 2
                let value anyT = (loadarrayptrs argv 0 1)
                let vT = ('typeof value)
                let T = (unbox-pointer anyT type)
                if (ptrcmp!= vT T)
                    let ok f reverse = (implyfn vT T)
                    if ok
                        sc_call_new f
                            Value-array
                                if reverse
                                    _ anyT value
                                else
                                    _ value anyT
                    else
                        sc_call_new
                            box-pointer
                                gen-cast-error "can't implicitly cast value of type "
                            \ 0 (undef ValueArrayPointer)
                else value

    let as =
        box-ast-macro
            fn "as-dispatch" (argc argv)
                verify-count argc 2 2
                let value anyT = (loadarrayptrs argv 0 1)
                let vT = ('typeof value)
                let T = (unbox-pointer anyT type)
                if (ptrcmp!= vT T)
                    let ok f reverse =
                        do
                            # try implicit cast first
                            let ok f reverse = (implyfn vT T)
                            if ok (_ ok f reverse)
                            else
                                # then try explicit cast
                                asfn vT T
                    if ok
                        sc_call_new f
                            Value-array
                                if reverse
                                    _ anyT value
                                else
                                    _ value anyT
                    else
                        sc_call_new
                            box-pointer
                                gen-cast-error "can't cast value of type "
                            \ 0 (undef ValueArrayPointer)
                else value

    let UnaryOpFunctionType =
        'pointer ('raising (function Value type) Error)

    let BinaryOpFunctionType =
        'pointer ('raising (function Value type type) Error)

    fn unbox-binary-op-function-type (anyf)
        unbox-pointer anyf BinaryOpFunctionType

    fn get-binary-op-dispatcher (symbol lhsT rhsT)
        let ok anyf = ('@ lhsT symbol)
        if ok
            let f =
                try (unbox-binary-op-function-type anyf)
                except (err)
                    attribute-format-error! lhsT symbol err
            try
                return true (f lhsT rhsT)
            except (err)
        return false (undef Value)

    fn binary-op-cast-macro (castf lhsT rhs)
        let args = (alloca-array Value 2)
        store rhs (getelementptr args 0)
        store (box-pointer lhsT) (getelementptr args 1)
        sc_call_new castf 2 args

    # both types are typically the same
    fn sym-binary-op-label-macro (argc argv symbol rsymbol friendly-op-name)
        verify-count argc 2 2
        let lhs rhs = (loadarrayptrs argv 0 1)
        let lhsT = ('typeof lhs)
        let rhsT = ('typeof rhs)
        # try direct version first
        let ok f = (get-binary-op-dispatcher symbol lhsT rhsT)
        if ok
            return
                sc_call_new f argc argv
        # if types are unequal, we can try other options
        if (ptrcmp!= lhsT rhsT)
            # try reverse version next
            let ok f = (get-binary-op-dispatcher rsymbol rhsT lhsT)
            if ok
                return
                    sc_call_new f (Value-array rhs lhs)
            # can the operation be performed on the lhs type?
            let ok f = (get-binary-op-dispatcher symbol lhsT lhsT)
            if ok
                # can we cast rhsT to lhsT?
                let ok castf reverse = (implyfn rhsT lhsT)
                if ok
                    if (not reverse)
                        return
                            sc_call_new f
                                Value-array lhs
                                    binary-op-cast-macro castf lhsT rhs
            # can the operation be performed on the rhs type?
            let ok f = (get-binary-op-dispatcher symbol rhsT rhsT)
            if ok
                # can we cast lhsT to rhsT?
                let ok castf reverse = (implyfn lhsT rhsT)
                if ok
                    if (not reverse)
                        return
                            sc_call_new f
                                Value-array
                                    binary-op-cast-macro castf rhsT lhs
                                    rhs
        # we give up
        raise-compile-error!
            'join "can't "
                'join friendly-op-name
                    'join " values of types "
                        'join
                            '__repr (box-pointer lhsT)
                            'join " and "
                                '__repr (box-pointer rhsT)

    # right hand has fixed type
    fn asym-binary-op-label-macro (argc argv symbol rtype friendly-op-name)
        verify-count argc 2 2
        let lhs rhs = (loadarrayptrs argv 0 1)
        let lhsT = ('typeof lhs)
        let rhsT = ('typeof rhs)
        let ok f = ('@ lhsT symbol)
        if ok
            if (ptrcmp== rhsT rtype)
                return
                    sc_call_new f argc argv
            # can we cast rhsT to rtype?
            let ok castf reverse = (implyfn rhsT rtype)
            if ok
                if (not reverse)
                    return
                        sc_call_new f
                            Value-array lhs
                                binary-op-cast-macro castf rtype rhs
        # we give up
        raise-compile-error!
            'join "can't "
                'join friendly-op-name
                    'join " values of types "
                        'join
                            '__repr (box-pointer lhsT)
                            'join " and "
                                '__repr (box-pointer rhsT)

    fn unary-op-label-macro (argc argv symbol friendly-op-name)
        verify-count argc 1 1
        let lhs = (loadarrayptrs argv 0)
        let lhsT = ('typeof lhs)
        let ok f = ('@ lhsT symbol)
        if ok
            return
                sc_call_new f argc argv
        raise-compile-error!
            'join "can't "
                'join friendly-op-name
                    'join " value of type "
                        '__repr (box-pointer lhsT)

    inline make-unary-op-dispatch (symbol friendly-op-name)
        box-ast-macro (fn (argc argv) (unary-op-label-macro argc argv symbol friendly-op-name))

    inline make-sym-binary-op-dispatch (symbol rsymbol friendly-op-name)
        box-ast-macro (fn (argc argv) (sym-binary-op-label-macro argc argv symbol rsymbol friendly-op-name))

    inline make-asym-binary-op-dispatch (symbol rtype friendly-op-name)
        box-ast-macro (fn (argc argv) (asym-binary-op-label-macro argc argv symbol rtype friendly-op-name))

    # support for calling macro functions directly
    'set-symbols SyntaxMacro
        __call =
            box-pointer
                inline (self at next scope)
                    (bitcast self SyntaxMacroFunctionType) at next scope

    inline symbol-imply (self destT)
        sc_symbol_to_string self

    'set-symbols Symbol
        __== = (box-binary-op-dispatch (single-binary-op-dispatch icmp==))
        __!= = (box-binary-op-dispatch (single-binary-op-dispatch icmp!=))
        __imply =
            box-cast-dispatch
                fn "syntax-imply" (vT T)
                    if (ptrcmp== T string)
                        return (Value symbol-imply)
                    raise-compile-error! "unsupported type"

    fn string@ (self i)
        let s = (sc_string_buffer self)
        load (getelementptr s i)

    'set-symbols string
        __== = (box-binary-op-dispatch (single-binary-op-dispatch ptrcmp==))
        __!= = (box-binary-op-dispatch (single-binary-op-dispatch ptrcmp!=))
        __.. = (box-binary-op-dispatch (single-binary-op-dispatch sc_string_join))
        __countof = sc_string_count
        __@ = string@
        __lslice = sc_string_lslice
        __rslice = sc_string_rslice

    'set-symbols list
        __typecall =
            inline (self args...)
                make-list args...
        __.. = (box-binary-op-dispatch (single-binary-op-dispatch sc_list_join))
        __repr =
            inline "list-repr" (self)
                '__repr (Value self)

    inline single-signed-binary-op-dispatch (sf uf)
        fn (lhsT rhsT)
            if (ptrcmp== lhsT rhsT)
                if ('signed? lhsT)
                    return (Value sf)
                else
                    return (Value uf)
            raise-compile-error! "unsupported type"

    fn dispatch-and-or (argc argv flip)
        verify-count argc 2 2
        let cond elsef = (loadarrayptrs argv 0 1)
        let call-elsef = (sc_call_new elsef 0 (undef ValueArrayPointer))
        if ('constant? cond)
            let value = (unbox-integer cond bool)
            return
                if (bxor value flip) cond
                else call-elsef
        let ifval = (sc_if_new)
        if flip
            sc_if_append_then_clause ifval cond call-elsef
            sc_if_append_else_clause ifval cond
        else
            sc_if_append_then_clause ifval cond cond
            sc_if_append_else_clause ifval call-elsef
        ifval

    'set-symbols integer
        __imply = (box-cast-dispatch integer-imply)
        __as = (box-cast-dispatch integer-as)
        __+ = (box-binary-op-dispatch (single-binary-op-dispatch add))
        __- = (box-binary-op-dispatch (single-binary-op-dispatch sub))
        __== = (box-binary-op-dispatch (single-binary-op-dispatch icmp==))
        __!= = (box-binary-op-dispatch (single-binary-op-dispatch icmp!=))
        __< = (box-binary-op-dispatch (single-signed-binary-op-dispatch icmp<s icmp<u))
        __<= = (box-binary-op-dispatch (single-signed-binary-op-dispatch icmp<=s icmp<=u))
        __> = (box-binary-op-dispatch (single-signed-binary-op-dispatch icmp>s icmp>u))
        __>= = (box-binary-op-dispatch (single-signed-binary-op-dispatch icmp>=s icmp>=u))

    'set-symbols type
        __== = (box-binary-op-dispatch (single-binary-op-dispatch type==))
        __!= = (box-binary-op-dispatch (single-binary-op-dispatch type!=))
        __< = (box-binary-op-dispatch (single-binary-op-dispatch type<))
        __<= = (box-binary-op-dispatch (single-binary-op-dispatch type<=))
        __> = (box-binary-op-dispatch (single-binary-op-dispatch type>))
        __>= = (box-binary-op-dispatch (single-binary-op-dispatch type>=))
        __@ = sc_type_element_at
        __getattr =
            box-ast-macro
                fn "type-getattr" (argc argv)
                    verify-count argc 2 2
                    let self key = (loadarrayptrs argv 0 1)
                    if ('constant? self)
                        if ('constant? key)
                            let self = (unbox-pointer self type)
                            let key = (unbox-symbol key Symbol)
                            let ok result = (sc_type_at self key)
                            return
                                sc_argument_list_new
                                    Value-array (box-integer ok)
                                        if ok
                                            result
                                        else
                                            box-none;
                    sc_call_new (Value sc_type_at) argc argv

    'set-symbols Scope
        __getattr = sc_scope_at
        __typecall =
            box-ast-macro
                fn "scope-typecall" (argc argv)
                    """"There are four ways to create a new Scope:
                        ``Scope``
                            creates an empty scope without parent
                        ``Scope parent``
                            creates an empty scope descending from ``parent``
                        ``Scope none clone``
                            duplicate ``clone`` without a parent
                        ``Scope parent clone``
                            duplicate ``clone``, but descending from ``parent`` instead
                    verify-count argc 1 3
                    if (icmp== argc 1)
                        sc_call_new (Value Scope-new) 0 (undef ValueArrayPointer)
                    elseif (icmp== argc 2)
                        sc_call_new (Value Scope-new-expand) 1 (getelementptr argv 1)
                    else
                        # argc == 3
                        let parent = (loadarrayptrs argv 1)
                        if (type== ('typeof parent) Nothing)
                            sc_call_new (Value Scope-new-expand) 1 (getelementptr argv 2)
                        else
                            sc_call_new (Value Scope-clone-expand) 2 (getelementptr argv 1)

    'set-symbols syntax-scope
        and-branch = (box-ast-macro (fn (argc argv) (dispatch-and-or argc argv true)))
        or-branch = (box-ast-macro (fn (argc argv) (dispatch-and-or argc argv false)))
        immutable = (box-pointer immutable)
        aggregate = (box-pointer aggregate)
        opaquepointer = (box-pointer opaquepointer)
        SyntaxMacro = (box-pointer SyntaxMacro)
        SyntaxMacroFunctionType = (box-pointer SyntaxMacroFunctionType)
        DispatchCastFunctionType = (box-pointer DispatchCastFunctionType)
        BinaryOpFunctionType = (box-pointer BinaryOpFunctionType)
        implyfn = (box-pointer (typify implyfn type type))
        asfn = (box-pointer (typify asfn type type))
        imply = imply
        as = as
        countof = (make-unary-op-dispatch '__countof "count")
        ~ = (make-unary-op-dispatch '__~ "bitwise-negate")
        == = (make-sym-binary-op-dispatch '__== '__r== "compare")
        != = (make-sym-binary-op-dispatch '__!= '__r!= "compare")
        < = (make-sym-binary-op-dispatch '__< '__r< "compare")
        <= = (make-sym-binary-op-dispatch '__<= '__r<= "compare")
        > = (make-sym-binary-op-dispatch '__> '__r> "compare")
        >= = (make-sym-binary-op-dispatch '__>= '__r>= "compare")
        + = (make-sym-binary-op-dispatch '__+ '__r+ "add")
        - = (make-sym-binary-op-dispatch '__- '__r- "subtract")
        * = (make-sym-binary-op-dispatch '__* '__r* "multiply")
        / = (make-sym-binary-op-dispatch '__/ '__r/ "divide")
        % = (make-sym-binary-op-dispatch '__% '__r% "modulate")
        & = (make-sym-binary-op-dispatch '__& '__r& "apply bitwise-and to")
        | = (make-sym-binary-op-dispatch '__| '__r| "apply bitwise-or to")
        ^ = (make-sym-binary-op-dispatch '__^ '__r^ "apply bitwise-xor to")
        .. = (make-sym-binary-op-dispatch '__.. '__r.. "join")
        @ = (make-asym-binary-op-dispatch '__@ usize "apply subscript operator with")
        getattr = (make-asym-binary-op-dispatch '__getattr Symbol "get attribute from")
        lslice = (make-asym-binary-op-dispatch '__lslice usize "apply left-slice operator with")
        rslice = (make-asym-binary-op-dispatch '__rslice usize "apply right-slice operator with")
        #constbranch = constbranch
    syntax-scope

#inline Syntax-unbox (self destT)
    imply ('datum self) destT

inline not (value)
    bxor (imply value bool) true

let function->SyntaxMacro =
    typify
        fn "function->SyntaxMacro" (f)
            bitcast f SyntaxMacro
        SyntaxMacroFunctionType

inline syntax-block-scope-macro (f)
    function->SyntaxMacro (typify f list list Scope)

inline syntax-scope-macro (f)
    syntax-block-scope-macro
        fn (at next scope)
            let at scope = (f ('next at) scope)
            return (cons (Value at) next) scope

inline syntax-macro (f)
    syntax-block-scope-macro
        fn (at next scope)
            return (cons (Value (f ('next at))) next) scope

fn empty? (value)
    == (countof value) 0:usize

fn cons (at next)
    sc_list_cons (Value at) next

fn type-repr-needs-suffix? (CT)
    if (== CT i32) false
    elseif (== CT bool) false
    elseif (== CT Nothing) false
    elseif (== CT f32) false
    elseif (== CT string) false
    elseif (== CT list) false
    elseif (== CT Symbol) false
    elseif (== CT type) false
    elseif (== ('kind CT) type-kind-vector)
        let ET = ('element@ CT 0)
        if (== ET i32) false
        elseif (== ET bool) false
        elseif (== ET f32) false
        else true
    else true

fn tostring (value)
    let T = (typeof value)
    let ok f = (getattr T '__tostring)
    constbranch ok
        inline ()
            f value
        inline ()
            sc_value_tostring (Value value)

fn repr (value)
    let T = (typeof value)
    let ok f = (getattr T '__repr)
    let s =
        constbranch ok
            inline ()
                f value
            inline ()
                sc_value_repr (Value value)
    if (type-repr-needs-suffix? T)
        .. s
            ..
                default-styler style-operator ":"
                default-styler style-type ('string T)

    else s

let print =
    do
        fn print-element (i key value)
            if (!= i 0)
                io-write! " "
            constbranch (== (typeof value) string)
                inline ()
                    io-write! value
                inline ()
                    io-write! (repr value)

        fn print (values...)
            va-lifold print-element values...
            io-write! "\n"
            values...

