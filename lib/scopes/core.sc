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

#let
    __typify = sc_typify
    __compile = sc_compile
    __compile-object = sc_compile_object
    __compile-spirv = sc_compile_spirv
    __compile-glsl = sc_compile_glsl
    verify-stack! = sc_verify_stack
    enter-solver-cli! = sc_enter_solver_cli

    format-message = sc_format_message

    file? = sc_is_file
    directory? = sc_is_directory
    dirname = sc_dirname
    basename = sc_basename

    CompileError = sc_location_error_new
    RuntimeError = sc_runtime_error_new

    __hash = sc_hash
    __hash2x64 = sc_hash2x64
    __hashbytes = sc_hashbytes

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

# square list expressions are ast unquotes by default
let square-list = ast-unquote

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

fn error! (msg)
    raise (sc_runtime_error_new msg)

fn syntax-error! (anchor msg)
    raise (sc_location_error_new anchor msg)

fn compiler-error! (value)
    raise (sc_location_error_new (sc_get_active_anchor) value)

# print an unboxing error given two types
fn unbox-verify (value wantT)
    let haveT = (sc_value_type value)
    if (ptrcmp!= haveT wantT)
        syntax-error! (sc_value_anchor value)
            sc_string_join "can't unbox value of type "
                sc_string_join
                    sc_value_repr (box-pointer haveT)
                    sc_string_join " as value of type "
                        sc_value_repr (box-pointer wantT)

inline unbox-integer (value T)
    unbox-verify value T
    itrunc (sc_const_int_extract value) T

inline unbox-symbol (value T)
    unbox-verify value T
    bitcast (sc_const_int_extract value) T

inline unbox-pointer (value T)
    unbox-verify value T
    bitcast (sc_const_pointer_extract value) T

fn verify-count (count mincount maxcount)
    if (icmp>=s mincount 0)
        if (icmp<s count mincount)
            compiler-error!
                sc_string_join "at least "
                    sc_string_join (sc_value_repr (box-integer mincount))
                        sc_string_join " argument(s) expected, got "
                            sc_value_repr (box-integer count)
    if (icmp>=s maxcount 0)
        if (icmp>s count maxcount)
            compiler-error!
                sc_string_join "at most "
                    sc_string_join (sc_value_repr (box-integer maxcount))
                        sc_string_join " argument(s) expected, got "
                            sc_value_repr (box-integer count)

fn Value-none? (value)
    ptrcmp== (sc_value_type value) Nothing

# declare new pointer types at runtime
let TypeArrayPointer =
    sc_pointer_type type pointer-flag-non-writable unnamed
let ValueArrayPointer =
    sc_pointer_type Value pointer-flag-non-writable unnamed
let ASTMacroFunction = (sc_type_storage ASTMacro)
# dynamically construct a new symbol
let ellipsis-symbol = (sc_symbol_new "...")

# execute until here and treat the remainder as a new translation unit
compile-stage;

# we can now access TypeArrayPointer as a compile time value
let void =
    sc_arguments_type 0 (nullof TypeArrayPointer)

let typify =
    do
        fn typify (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 -1
            let src_fn = (sc_getarg args 0)
            let src_fn = (unbox-pointer src_fn Closure)
            let typecount = (sub argcount 1)
            let types = (alloca-array type typecount)
            loop (i j) = 1 0
            if (icmp<s i argcount)
                let ty = (sc_getarg args i)
                store (unbox-pointer ty type) (getelementptr types j)
                repeat (add i 1) (add j 1)
            sc_typify src_fn typecount (bitcast types TypeArrayPointer)

        let types = (alloca-array type 1:usize)
        store Value (getelementptr types 0)
        let types = (bitcast types TypeArrayPointer)
        let result = (sc_compile (sc_typify typify 1 types) 0:u64)
        let result-type = (sc_value_type result)
        if (ptrcmp!= result-type ASTMacroFunction)
            compiler-error!
                sc_string_join "AST macro must have type "
                    sc_string_join
                        sc_value_repr (box-pointer ASTMacroFunction)
                        sc_string_join " but has type "
                            sc_value_repr (box-pointer result-type)
        let ptr = (sc_const_pointer_extract result)
        bitcast ptr ASTMacro

compile-stage;

let function->ASTMacro =
    typify
        fn "function->ASTMacro" (f)
            bitcast f ASTMacro
        ASTMacroFunction

fn box-empty ()
    sc_argument_list_new;

fn box-none ()
    sc_const_aggregate_new Nothing 0 (undef ValueArrayPointer)

# take closure l, typify and compile it and return a function of ASTMacro type
inline ast-macro (l)
    function->ASTMacro (typify l Value)

inline box-ast-macro (l)
    box-pointer (ast-macro l)

let va-lfold va-lifold =
    do
        fn va-lfold (args use-indices)
            let argcount = (sc_argcount args)
            verify-count argcount 2 -1
            let init = (sc_getarg args 0)
            let f = (sc_getarg args 1)
            if (icmp== argcount 2)
                return init
            let ofs = (? use-indices 1 0)
            loop (i ret) = 2 init
            if (icmp<s i argcount)
                let arg =
                    sc_getarg args i
                let k = (sc_type_key (sc_value_type arg))
                let v = (sc_keyed_new unnamed arg)
                repeat (add i 1)
                    if use-indices
                        `(f [(sub i 2)] k v ret)
                    else
                        `(f k v ret)
            ret
        _
            ast-macro (fn "va-lfold" (args) (va-lfold args false))
            ast-macro (fn "va-ilfold" (args) (va-lfold args true))

let va-rfold va-rifold =
    do
        fn va-rfold (args use-indices)
            let argcount = (sc_argcount args)
            verify-count argcount 2 -1
            let init = (sc_getarg args 0)
            let f = (sc_getarg args 1)
            if (icmp== argcount 2)
                return init
            let ofs = (? use-indices 1 0)
            loop (i ret) = argcount init
            if (icmp>s i 2)
                let oi = i
                let i = (sub i 1)
                let arg =
                    sc_getarg args i
                let k = (sc_type_key (sc_value_type arg))
                let v = (sc_keyed_new unnamed arg)
                repeat i
                    if use-indices
                        `(f [(sub i 2)] k v ret)
                    else
                        `(f k v ret)
            ret
        _
            ast-macro (fn "va-rfold" (args) (va-rfold args false))
            ast-macro (fn "va-rifold" (args) (va-rfold args true))

inline raises-compile-error ()
    if false
        compiler-error! "hidden"

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

fn compare-type (args f)
    let argcount = (sc_argcount args)
    verify-count argcount 2 2
    let a = (sc_getarg args 0)
    let b = (sc_getarg args 1)
    if (sc_value_is_constant a)
        if (sc_value_is_constant b)
            return
                box-integer
                    f (unbox-pointer a type) (unbox-pointer b type)
    `(f args)

inline type-comparison-func (f)
    fn (args) (compare-type args (typify f type type))

# typecall
sc_type_set_symbol type '__call
    box-ast-macro
        fn "type-call" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 -1
            let self = (sc_getarg args 0)
            let T = (unbox-pointer self type)
            let ok f = (sc_type_at T '__typecall)
            if ok
                return `(f args)
            compiler-error!
                sc_string_join "no type constructor available for type "
                    sc_value_repr self

# method call syntax
sc_type_set_symbol Symbol '__call
    box-ast-macro
        fn "symbol-call" (args)
            fn resolve-method (self symval)
                let sym = (unbox-symbol symval Symbol)
                let T = (sc_value_type self)
                let ok f = (sc_type_at T sym)
                if ok
                    return f
                # if calling method of type, try typemethod
                if (ptrcmp== T type)
                    if (sc_value_is_constant self)
                        let self = (unbox-pointer self type)
                        let ok f = (sc_type_at self sym)
                        if ok
                            return f
                compiler-error!
                    sc_string_join "no method named "
                        sc_string_join (sc_value_repr symval)
                            sc_string_join " in value of type "
                                sc_value_repr (box-pointer T)

            let argcount = (sc_argcount args)
            verify-count argcount 2 -1
            let symval = (sc_getarg args 0)
            let self = (sc_getarg args 1)
            let expr = `([(resolve-method self symval)]
                [(sc_extract_argument_list_new args 1)])
            expr

inline gen-key-any-set (selftype fset)
    box-ast-macro
        fn "set-symbol" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 2 3
            let self = (sc_getarg args 0)
            let key value =
                if (icmp== argcount 3)
                    let key = (sc_getarg args 1)
                    let value = (sc_getarg args 2)
                    _ key value
                else
                    let arg = (sc_getarg args 1)
                    let key = (sc_type_key (sc_value_type arg))
                    let arg = (sc_keyed_new unnamed arg)
                    _ (box-symbol key) arg
            if (sc_value_is_constant self)
                if (sc_value_is_constant key)
                    if (sc_value_is_pure value)
                        let self = (unbox-pointer self selftype)
                        let key = (unbox-symbol key Symbol)
                        fset self key value
                        return `[]
            `(fset self key value)

# quick assignment of type attributes
sc_type_set_symbol type 'set-symbol (gen-key-any-set type sc_type_set_symbol)
sc_type_set_symbol Scope 'set-symbol (gen-key-any-set Scope sc_scope_set_symbol)

sc_type_set_symbol type 'pointer
    box-ast-macro
        fn "type-pointer" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 1
            let self = (sc_getarg args 0)
            let T = (unbox-pointer self type)
            box-pointer
                sc_pointer_type T pointer-flag-non-writable unnamed

# tuple type constructor
sc_type_set_symbol tuple '__typecall
    box-ast-macro
        fn "tuple" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 -1
            let pcount = (sub argcount 1)
            let types = (alloca-array type pcount)
            loop (i) = 1
            if (icmp<s i argcount)
                let arg = (sc_getarg args i)
                let T = (unbox-pointer arg type)
                store T (getelementptr types (sub i 1))
                repeat (add i 1)
            box-pointer (sc_tuple_type pcount types)

# array type constructor
sc_type_set_symbol array '__typecall
    box-ast-macro
        fn "tuple" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 3 3
            let ET = (unbox-pointer (sc_getarg args 1) type)
            let size = (sc_getarg args 2)
            let sizeT = (sc_value_type size)
            if (type< sizeT integer)
            else
                unbox-verify size integer
            let size =
                bitcast (sc_const_int_extract size) usize
            box-pointer (sc_array_type ET size)

# arguments type constructor
sc_type_set_symbol Arguments '__typecall
    box-ast-macro
        fn "Arguments" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 -1
            let pcount = (sub argcount 1)
            let types = (alloca-array type pcount)
            loop (i) = 1
            if (icmp<s i argcount)
                let arg = (sc_getarg args i)
                let T = (unbox-pointer arg type)
                store T (getelementptr types (sub i 1))
                repeat (add i 1)
            box-pointer (sc_arguments_type pcount types)

# function pointer type constructor
sc_type_set_symbol function '__typecall
    box-ast-macro
        fn "function" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 2 -1
            let rtype = (sc_getarg args 1)
            let rtype = (unbox-pointer rtype type)
            let pcount = (sub argcount 2)
            let types = (alloca-array type pcount)
            loop (i) = 2
            if (icmp<s i argcount)
                let arg = (sc_getarg args i)
                let T = (unbox-pointer arg type)
                store T (getelementptr types (sub i 2))
                repeat (add i 1)
            box-pointer (sc_function_type rtype pcount types)

sc_type_set_symbol type 'raising
    box-ast-macro
        fn "function-raising" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 2 2
            let self = (sc_getarg args 0)
            let except_type = (sc_getarg args 1)
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

let none? =
    ast-macro
        fn (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 1
            let value = (sc_getarg args 0)
            box-integer
                ptrcmp== (sc_value_type value) Nothing

fn unpack2 (args)
    let argcount = (sc_argcount args)
    verify-count argcount 2 2
    let a = (sc_getarg args 0)
    let b = (sc_getarg args 1)
    return a b

let const.icmp<=.i32.i32 =
    ast-macro
        fn (args)
            let a b = (unpack2 args)
            if (sc_value_is_constant a)
                if (sc_value_is_constant b)
                    let a = (unbox-integer a i32)
                    let b = (unbox-integer b i32)
                    return
                        box-integer (icmp<=s a b)
            compiler-error! "arguments must be constant"

let const.add.i32.i32 =
    ast-macro
        fn (args)
            let a b = (unpack2 args)
            if (sc_value_is_constant a)
                if (sc_value_is_constant b)
                    let a = (unbox-integer a i32)
                    let b = (unbox-integer b i32)
                    return
                        box-integer (add a b)
            compiler-error! "arguments must be constant"

let constbranch =
    ast-macro
        fn (args)
            let argcount = (sc_argcount args)
            verify-count argcount 3 3
            let cond = (sc_getarg args 0)
            let thenf = (sc_getarg args 1)
            let elsef = (sc_getarg args 2)
            if (sc_value_is_constant cond)
            else
                compiler-error! "condition must be constant"
            let value = (unbox-integer cond bool)
            `([(? value thenf elsef)])

sc_type_set_symbol Value '__typecall
    box-ast-macro
        fn (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 -1
            if (icmp== argcount 1)
                box-pointer (box-empty)
            else
                let value = (sc_getarg args 1)
                let T = (sc_value_type value)
                if (ptrcmp== T Value)
                    value
                elseif (sc_value_is_constant value)
                    box-pointer value
                elseif (ptrcmp== T Nothing)
                    box-none;
                else
                    sc_value_wrap T value

let __unbox =
    ast-macro
        fn (args)
            let argc = (sc_argcount args)
            verify-count argc 2 2
            let value = (sc_getarg args 0)
            let T = (sc_getarg args 1)
            let T = (unbox-pointer T type)
            sc_value_unwrap T value
let
    type== = (ast-macro (type-comparison-func ptrcmp==))
    type!= = (ast-macro (type-comparison-func ptrcmp!=))
    type< = (ast-macro (type-comparison-func type<))
    type<= = (ast-macro (type-comparison-func type<=))
    type> = (ast-macro (type-comparison-func type>))
    type>= = (ast-macro (type-comparison-func type>=))

compile-stage;

fn cons (values...)
    va-rifold none
        inline (i key value next)
            constbranch (none? next)
                inline ()
                    value
                inline ()
                    sc_list_cons (Value value) next
        values...

inline make-list (values...)
    va-rifold '()
        inline (i key value result)
            sc_list_cons (Value value) result
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
    va-lfold none
        inline (key value)
            'set-symbol self key value
        values...

'set-symbol type 'set-symbols set-symbols
'set-symbol Scope 'set-symbols set-symbols

'set-symbols Value
    constant? = sc_value_is_constant
    pure? = sc_value_is_pure
    kind = sc_value_kind
    none? = (typify Value-none? Value)
    __repr = sc_value_repr
    ast-repr = sc_value_ast_repr
    typeof = sc_value_type
    anchor = sc_value_anchor
    argcount = sc_argcount
    getarg = sc_getarg
    getarglist = sc_getarglist
    dekey =
        fn "dekey" (self)
            let k = (sc_type_key ('typeof self))
            _ k (sc_keyed_new unnamed self)

'set-symbols Scope
    @ = sc_scope_at
    next = sc_scope_next
    docstring = sc_scope_get_docstring
    set-docstring! = sc_scope_set_docstring
    parent = sc_scope_get_parent

'set-symbols string
    join = sc_string_join

'set-symbols list
    __countof = sc_list_count
    join = sc_list_join
    @ = sc_list_at
    next = sc_list_next
    decons = decons
    reverse = sc_list_reverse

'set-symbols type
    bitcount = sc_type_bitcountof
    signed? = sc_integer_type_is_signed
    element@ = sc_type_element_at
    element-count = sc_type_countof
    storage = sc_type_storage
    kind = sc_type_kind
    @ = sc_type_at
    local@ = sc_type_local_at
    opaque? = sc_type_is_opaque
    string = sc_type_string
    super = sc_typename_type_get_super
    set-super = sc_typename_type_set_super
    set-storage = sc_typename_type_set_storage
    return-type = sc_function_type_return_type
    key = sc_type_key
    variadic? = sc_function_type_is_variadic
    pointer? =
        fn (cls)
            icmp== ('kind cls) type-kind-pointer
    function? =
        fn (cls)
            icmp== ('kind cls) type-kind-function
    function-pointer? =
        fn (cls)
            if ('pointer? cls)
                if ('function? ('element@ cls 0))
                    return true
            return false
    set-pointer-element-type =
        fn (cls ET)
            sc_pointer_type_set_element_type cls ET
    set-pointer-storage-class =
        fn (cls storage-class)
            sc_pointer_type_set_storage_class cls storage-class
    set-pointer-immutable =
        fn (cls)
            sc_pointer_type_set_flags cls
                bor (sc_pointer_type_get_flags cls) pointer-flag-non-writable
    set-pointer-mutable =
        fn (cls)
            sc_pointer_type_set_flags cls
                band (sc_pointer_type_get_flags cls)
                    bxor pointer-flag-non-writable -1:u64
    strip-pointer-storage-class =
        fn (cls)
            sc_pointer_type_set_storage_class cls unnamed
    pointer-storage-class =
        fn (cls)
            sc_pointer_type_get_storage_class cls
    pointer-readable? =
        fn (cls)
            icmp== (band (sc_pointer_type_get_flags cls) pointer-flag-non-readable) 0:u64
    pointer-writable? =
        fn (cls)
            icmp== (band (sc_pointer_type_get_flags cls) pointer-flag-non-writable) 0:u64

#'set-symbols Closure
    frame = sc_closure_frame
    label = sc_closure_label

let rawstring = ('pointer i8)

inline box-cast (f)
    box-pointer (typify f type type Value)

inline not (value)
    bxor value true

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

# generator type
let Generator = (sc_typename_type "Generator")
'set-storage Generator ('storage Closure)

# syntax macro type
let SyntaxMacro = (sc_typename_type "SyntaxMacro")
let SyntaxMacroFunction =
    'pointer
        'raising
            function (Arguments list Scope) list list Scope
            Error
'set-storage SyntaxMacro SyntaxMacroFunction

# any extraction

inline unbox (value T)
    unbox-verify value T
    __unbox value T

fn value-imply (vT T expr)
    if true
        return `(unbox expr T)
    compiler-error! "unsupported type"

'set-symbols Value
    __imply =
        box-cast value-imply
    __rimply =
        box-cast
            fn "syntax-rimply" (vT T expr)
                if true
                    return `(Value expr)
                compiler-error! "unsupported type"

'set-symbols ASTMacro
    __rimply =
        box-cast
            fn "ASTMacro-rimply" (vT T expr)
                if (ptrcmp== vT ASTMacroFunction)
                    return `(bitcast expr T)
                elseif (ptrcmp== vT Closure)
                    if ('constant? expr)
                        return `(ast-macro expr)
                compiler-error! "unsupported type"

# integer casting

fn integer-imply (vT T expr)
    let ST =
        if (ptrcmp== T usize) ('storage T)
        else T
    if (icmp== ('kind ST) type-kind-integer)
        # constant i32 auto-expands to usize if not negative
        if ('constant? expr)
            if (ptrcmp== vT i32)
                let val = (unbox expr i32)
                if (ptrcmp== T usize)
                    if (icmp<s val 0)
                        compiler-error! "signed integer is negative"
                    return
                        box-integer (sext val usize)
        let args... = expr T
        let valw = ('bitcount vT)
        let destw = ('bitcount ST)
        # must have same signed bit
        if (icmp== ('signed? vT) ('signed? ST))
            if (icmp== destw valw)
                return `(bitcast args...)
            elseif (icmp>s destw valw)
                if ('signed? vT)
                    return `(sext args...)
                else
                    return `(zext args...)
    compiler-error! "unsupported type"

fn integer-as (vT T expr)
    let args... = expr T
    let T =
        if (ptrcmp== T usize) ('storage T)
        else T
    if (icmp== ('kind T) type-kind-integer)
        let valw = ('bitcount vT)
        let destw = ('bitcount T)
        if (icmp== destw valw)
            return `(bitcast args...)
        elseif (icmp>s destw valw)
            if ('signed? vT)
                return `(sext args...)
            else
                return `(zext args...)
        else
            return `(itrunc args...)
    elseif (icmp== ('kind T) type-kind-real)
        if ('signed? vT)
            return `(sitofp args...)
        else
            return `(uitofp args...)
    compiler-error! "unsupported type"

# only perform safe casts: i.e. float to double
fn real-imply (vT T expr)
    if (icmp== ('kind T) type-kind-real)
        let args... = expr T
        let valw = ('bitcount vT)
        let destw = ('bitcount T)
        if (icmp== destw valw)
            return `(bitcast args...)
        elseif (icmp>s destw valw)
            return `(fpext args...)
    compiler-error! "unsupported type"

# more aggressive cast that converts from all numerical types
fn real-as (vT T expr)
    let args... = expr T
    let T =
        if (ptrcmp== T usize) ('storage T)
        else T
    let kind = ('kind T)
    if (icmp== kind type-kind-real)
        let valw destw = ('bitcount vT) ('bitcount T)
        if (icmp== destw valw)
            return `(bitcast args...)
        elseif (icmp>s destw valw)
            return `(fpext args...)
        else
            return `(fptrunc args...)
    elseif (icmp== kind type-kind-integer)
        if ('signed? T)
            return `(fptosi args...)
        else
            return `(fptoui args...)
    compiler-error! "unsupported type"


inline box-binary-op (f)
    box-pointer (typify f type type Value Value)

inline single-binary-op-dispatch (destf)
    fn (lhsT rhsT lhs rhs)
        if (ptrcmp== lhsT rhsT)
            return `(destf lhs rhs)
        compiler-error! "unsupported type"

inline cast-error! (intro-string vT T)
    # create branch so we can trick the function into assuming
        there's another exit path
    compiler-error!
        sc_string_join intro-string
            sc_string_join
                '__repr (box-pointer vT)
                sc_string_join " to type "
                    '__repr (box-pointer T)

# receive a source type, a destination type and an expression, and return an
    untyped expression that transforms the value to said type, or raise an error
let CastFunctionType =
    'pointer ('raising (function Value type type Value) Error)

fn unbox-cast-function-type (anyf)
    unbox-pointer anyf CastFunctionType

fn attribute-format-error! (T symbol err)
    compiler-error!
        'join "wrong format for attribute "
            'join ('__repr (box-symbol symbol))
                'join " of type "
                    'join ('__repr (box-pointer T))
                        'join ": "
                            sc_format_error err

fn cast-expr (symbol rsymbol vT T expr)
    let ok anyf = ('@ vT symbol)
    if ok
        let f =
            try
                unbox-cast-function-type anyf
            except (err)
                attribute-format-error! vT symbol err
        try
            return true (f vT T expr)
        except (err)
            # ignore
    let ok anyf = ('@ T rsymbol)
    if ok
        let f =
            try
                unbox-cast-function-type anyf
            except (err)
                attribute-format-error! T rsymbol err
        try
            return true (f vT T expr)
        except (err)
            # ignore
    return false (nullof Value)

fn imply-expr (vT T expr)
    cast-expr '__imply '__rimply vT T expr
fn as-expr (vT T expr)
    cast-expr '__as '__ras vT T expr

let
    imply =
        ast-macro
            fn "imply-dispatch" (args)
                let argc = ('argcount args)
                verify-count argc 2 2
                let value = ('getarg args 0)
                let anyT = ('getarg args 1)
                let vT = ('typeof value)
                let T = (unbox-pointer anyT type)
                if (ptrcmp!= vT T)
                    let ok expr = (imply-expr vT T value)
                    if ok expr
                    else
                        cast-error! "can't implicitly cast value of type " vT T
                else value

    as =
        ast-macro
            fn "as-dispatch" (args)
                let argc = ('argcount args)
                verify-count argc 2 2
                let value = ('getarg args 0)
                let anyT = ('getarg args 1)
                let vT = ('typeof value)
                let T = (unbox-pointer anyT type)
                if (ptrcmp!= vT T)
                    let ok expr =
                        do
                            # try implicit cast first
                            let ok expr = (imply-expr vT T value)
                            if ok (_ ok expr)
                            else
                                # then try explicit cast
                                as-expr vT T value
                    if ok expr
                    else
                        cast-error! "can't cast value of type " vT T
                else value

let BinaryOpFunctionType =
    'pointer ('raising (function Value type type Value Value) Error)

fn unbox-binary-op-function-type (anyf)
    unbox-pointer anyf BinaryOpFunctionType

fn binary-op-expr (symbol lhsT rhsT lhs rhs)
    let ok anyf = ('@ lhsT symbol)
    if ok
        let f =
            try (unbox-binary-op-function-type anyf)
            except (err)
                attribute-format-error! lhsT symbol err
        try
            return true (f lhsT rhsT lhs rhs)
        except (err)
    return false (nullof Value)

fn sym-binary-op-expr (symbol rsymbol lhsT rhsT lhs rhs)
    let ok anyf = ('@ lhsT symbol)
    if ok
        let f =
            try (unbox-binary-op-function-type anyf)
            except (err)
                attribute-format-error! lhsT symbol err
        try
            return true (f lhsT rhsT lhs rhs)
        except (err)
    if (ptrcmp!= lhsT rhsT)
        let ok anyf = ('@ rhsT rsymbol)
        if ok
            let f =
                try (unbox-binary-op-function-type anyf)
                except (err)
                    attribute-format-error! rhsT rsymbol err
            try
                return true (f lhsT rhsT lhs rhs)
            except (err)
    return false (nullof Value)

# both types are typically the same
fn sym-binary-op-label-macro (args symbol rsymbol friendly-op-name)
    let argc = ('argcount args)
    verify-count argc 2 2
    let lhs rhs =
        'getarg args 0
        'getarg args 1
    let lhsT = ('typeof lhs)
    let rhsT = ('typeof rhs)
    # try direct version first
    let ok expr = (sym-binary-op-expr symbol rsymbol lhsT rhsT lhs rhs)
    if ok
        return expr
    # if types are unequal, we can try other options
    if (ptrcmp!= lhsT rhsT)
        do
            # can we cast rhsT to lhsT?
            let ok rhs = (imply-expr rhsT lhsT rhs)
            if ok
                # try again
                let ok expr = (binary-op-expr symbol lhsT lhsT lhs rhs)
                if ok
                    return expr
        do
            # can we cast lhsT to rhsT?
            let ok lhs = (imply-expr lhsT rhsT lhs)
            if ok
                # try again
                let ok expr = (binary-op-expr symbol rhsT rhsT lhs rhs)
                if ok
                    return expr
    # we give up
    compiler-error!
        'join "can't "
            'join friendly-op-name
                'join " values of types "
                    'join
                        '__repr (box-pointer lhsT)
                        'join " and "
                            '__repr (box-pointer rhsT)

# right hand has fixed type
fn asym-binary-op-label-macro (args symbol rtype friendly-op-name)
    let argc = ('argcount args)
    verify-count argc 2 2
    let lhs rhs =
        'getarg args 0
        'getarg args 1
    let lhsT = ('typeof lhs)
    let rhsT = ('typeof rhs)
    let ok f = ('@ lhsT symbol)
    if ok
        if (ptrcmp== rhsT rtype)
            return `(f args)
        # can we cast rhsT to rtype?
        let ok rhs = (imply-expr rhsT rtype rhs)
        if ok
            return `(f lhs rhs)
    # we give up
    compiler-error!
        'join "can't "
            'join friendly-op-name
                'join " values of types "
                    'join
                        '__repr (box-pointer lhsT)
                        'join " and "
                            '__repr (box-pointer rhsT)

fn unary-op-label-macro (args symbol friendly-op-name)
    let argc = ('argcount args)
    verify-count argc 1 1
    let lhs = ('getarg args 0)
    let lhsT = ('typeof lhs)
    let ok f = ('@ lhsT symbol)
    if ok
        return `(f args)
    compiler-error!
        'join "can't "
            'join friendly-op-name
                'join " value of type "
                    '__repr (box-pointer lhsT)

inline make-unary-op-dispatch (symbol friendly-op-name)
    ast-macro (fn (args) (unary-op-label-macro args symbol friendly-op-name))

inline make-sym-binary-op-dispatch (symbol rsymbol friendly-op-name)
    ast-macro (fn (args) (sym-binary-op-label-macro args symbol rsymbol friendly-op-name))

inline make-asym-binary-op-dispatch (symbol rtype friendly-op-name)
    ast-macro (fn (args) (asym-binary-op-label-macro args symbol rtype friendly-op-name))

# support for calling macro functions directly
'set-symbols SyntaxMacro
    __call =
        box-pointer
            inline (self at next scope)
                (bitcast self SyntaxMacroFunction) at next scope

'set-symbols Symbol
    __== = (box-binary-op (single-binary-op-dispatch icmp==))
    __!= = (box-binary-op (single-binary-op-dispatch icmp!=))
    unique = sc_symbol_new_unique
    vararg? = sc_symbol_is_vararg
    __imply =
        box-cast
            fn "syntax-imply" (vT T expr)
                if (ptrcmp== T string)
                    return `(sc_symbol_to_string expr)
                compiler-error! "unsupported type"

fn string@ (self i)
    let s = (sc_string_buffer self)
    load (getelementptr s i)

'set-symbols string
    __== = (box-binary-op (single-binary-op-dispatch ptrcmp==))
    __!= = (box-binary-op (single-binary-op-dispatch ptrcmp!=))
    __.. = (box-binary-op (single-binary-op-dispatch sc_string_join))
    __countof = sc_string_count
    __@ = string@
    __lslice = sc_string_lslice
    __rslice = sc_string_rslice

'set-symbols list
    __typecall =
        inline (self args...)
            make-list args...
    __.. = (box-binary-op (single-binary-op-dispatch sc_list_join))
    __repr =
        inline "list-repr" (self)
            '__repr (Value self)

inline single-signed-binary-op-dispatch (sf uf)
    fn (lhsT rhsT lhs rhs)
        if (ptrcmp== lhsT rhsT)
            return
                ast-quote
                    call [
                        \ do
                            if ('signed? lhsT)
                                Value sf
                            else
                                Value uf ]
                        \ lhs rhs
        compiler-error! "unsupported type"

fn dispatch-and-or (args flip)
    let argc = ('argcount args)
    verify-count argc 2 2
    let cond elsef =
        'getarg args 0
        'getarg args 1
    let call-elsef = `(elsef)
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
    __imply = (box-cast integer-imply)
    __as = (box-cast integer-as)
    __+ = (box-binary-op (single-binary-op-dispatch add))
    __- = (box-binary-op (single-binary-op-dispatch sub))
    __* = (box-binary-op (single-binary-op-dispatch mul))
    __// = (box-binary-op (single-signed-binary-op-dispatch sdiv udiv))
    __% = (box-binary-op (single-signed-binary-op-dispatch srem urem))
    __& = (box-binary-op (single-binary-op-dispatch band))
    __| = (box-binary-op (single-binary-op-dispatch bor))
    __^ = (box-binary-op (single-binary-op-dispatch bxor))
    #__~ =
    __<< = (box-binary-op (single-binary-op-dispatch shl))
    __>> = (box-binary-op (single-signed-binary-op-dispatch ashr lshr))
    __== = (box-binary-op (single-binary-op-dispatch icmp==))
    __!= = (box-binary-op (single-binary-op-dispatch icmp!=))
    __< = (box-binary-op (single-signed-binary-op-dispatch icmp<s icmp<u))
    __<= = (box-binary-op (single-signed-binary-op-dispatch icmp<=s icmp<=u))
    __> = (box-binary-op (single-signed-binary-op-dispatch icmp>s icmp>u))
    __>= = (box-binary-op (single-signed-binary-op-dispatch icmp>=s icmp>=u))

inline floordiv (a b)
    sdiv (fptosi a i32) (fptosi b i32)

'set-symbols real
    __imply = (box-cast real-imply)
    __as = (box-cast real-as)
    __== = (box-binary-op (single-binary-op-dispatch fcmp==o))
    __!= = (box-binary-op (single-binary-op-dispatch fcmp!=u))
    __> = (box-binary-op (single-binary-op-dispatch fcmp>o))
    __>= = (box-binary-op (single-binary-op-dispatch fcmp>=o))
    __< = (box-binary-op (single-binary-op-dispatch fcmp<o))
    __<= = (box-binary-op (single-binary-op-dispatch fcmp<=o))
    __+ = (box-binary-op (single-binary-op-dispatch fadd))
    __- = (box-binary-op (single-binary-op-dispatch fsub))
    #__neg
        inline (self)
            fsub (nullof (typeof self)) self
    __* = (box-binary-op (single-binary-op-dispatch fmul))
    __/ = (box-binary-op (single-binary-op-dispatch fdiv))
    #__rcp
        inline (self)
            fdiv (imply 1 (typeof self)) self
    __// = (box-binary-op (single-binary-op-dispatch floordiv))
    __% = (box-binary-op (single-binary-op-dispatch frem))


fn type-getattr-dynamic (T value)
    let ok val = (sc_type_at T value)
    if ok
        return val
    compiler-error! "no such attribute"

'set-symbols Value
    __== = (box-binary-op (single-binary-op-dispatch ptrcmp==))
    __!= = (box-binary-op (single-binary-op-dispatch ptrcmp!=))

'set-symbols type
    __== = (box-binary-op (single-binary-op-dispatch type==))
    __!= = (box-binary-op (single-binary-op-dispatch type!=))
    __< = (box-binary-op (single-binary-op-dispatch type<))
    __<= = (box-binary-op (single-binary-op-dispatch type<=))
    __> = (box-binary-op (single-binary-op-dispatch type>))
    __>= = (box-binary-op (single-binary-op-dispatch type>=))
    __@ = sc_type_element_at
    # (dispatch-attr T key thenf elsef)
    dispatch-attr =
        box-ast-macro
            fn "type-dispatch-attr" (args)
                let argc = ('argcount args)
                verify-count argc 4 4
                let self key thenf elsef =
                    'getarg args 0
                    'getarg args 1
                    'getarg args 2
                    'getarg args 3
                let self = (unbox-pointer self type)
                let key = (unbox-symbol key Symbol)
                let ok result = (sc_type_at self key)
                if ok
                    return `(thenf result)
                else
                    return `(elsef)
    __getattr =
        box-ast-macro
            fn "type-getattr" (args)
                let argc = ('argcount args)
                verify-count argc 2 2
                let self key =
                    'getarg args 0
                    'getarg args 1
                if ('constant? self)
                    if ('constant? key)
                        let self = (unbox-pointer self type)
                        let key = (unbox-symbol key Symbol)
                        let ok result = (sc_type_at self key)
                        if ok
                            return result
                        else
                            return `(_)
                `(type-getattr-dynamic args)

fn scope-getattr-dynamic (T value)
    let ok val = (sc_scope_at T value)
    if ok
        return val
    compiler-error! "no such attribute"

'set-symbols Scope
    __== = (box-binary-op (single-binary-op-dispatch ptrcmp==))
    __getattr =
        box-ast-macro
            fn "scope-getattr" (args)
                let argc = ('argcount args)
                verify-count argc 2 2
                let self key =
                    'getarg args 0
                    'getarg args 1
                if ('constant? self)
                    if ('constant? key)
                        let self = (unbox-pointer self Scope)
                        let key = (unbox-symbol key Symbol)
                        let ok result = (sc_scope_at self key)
                        if ok
                            return result
                        else
                            return `(_)
                `(scope-getattr-dynamic args)
    __typecall =
        box-ast-macro
            fn "scope-typecall" (args)
                """"There are four ways to create a new Scope:
                    ``Scope``
                        creates an empty scope without parent
                    ``Scope parent``
                        creates an empty scope descending from ``parent``
                    ``Scope none clone``
                        duplicate ``clone`` without a parent
                    ``Scope parent clone``
                        duplicate ``clone``, but descending from ``parent`` instead
                let argc = ('argcount args)
                verify-count argc 1 3
                switch argc
                case 1
                    `(sc_scope_new)
                case 2
                    `(sc_scope_new_subscope [ ('getarg args 1) ])
                default
                    # argc == 3
                    let parent = ('getarg args 1)
                    if (type== ('typeof parent) Nothing)
                        `(sc_scope_clone [ ('getarg args 2) ])
                    else
                        `(sc_scope_clone_subscope
                            [(sc_extract_argument_list_new args 1)])

#---------------------------------------------------------------------------
# null type
#---------------------------------------------------------------------------

""""The type of the `null` constant. This type is uninstantiable.
let NullType = (sc_typename_type "NullType")
sc_typename_type_set_storage NullType ('pointer void)
'set-symbols NullType
    __repr =
        box-pointer
            inline (self)
                sc_default_styler style-number "null"
    __imply =
        box-cast
            fn "null-imply" (clsT T expr)
                if (icmp== ('kind ('storage T)) type-kind-pointer)
                    return `(bitcast expr T)
                compiler-error! "cannot convert to type"
    #__==
        fn (a b flipped)
            if flipped
                if (pointer-type? (storageof (typeof a)))
                    icmp== (ptrtoint a usize) 0:usize
            else
                if (pointer-type? (storageof (typeof b)))
                    icmp== (ptrtoint b usize) 0:usize

#---------------------------------------------------------------------------

let
    and-branch = (ast-macro (fn (args) (dispatch-and-or args true)))
    or-branch = (ast-macro (fn (args) (dispatch-and-or args false)))
    #implyfn = (typify implyfn type type)
    #asfn = (typify asfn type type)
    countof = (make-unary-op-dispatch '__countof "count")
    unpack = (make-unary-op-dispatch '__unpack "unpack")
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
    / = (make-sym-binary-op-dispatch '__/ '__r/ "real-divide")
    // = (make-sym-binary-op-dispatch '__// '__r// "integer-divide")
    % = (make-sym-binary-op-dispatch '__% '__r% "modulate")
    & = (make-sym-binary-op-dispatch '__& '__r& "apply bitwise-and to")
    | = (make-sym-binary-op-dispatch '__| '__r| "apply bitwise-or to")
    ^ = (make-sym-binary-op-dispatch '__^ '__r^ "apply bitwise-xor to")
    << = (make-sym-binary-op-dispatch '__<< '__r<< "apply left shift with")
    >> = (make-sym-binary-op-dispatch '__>> '__r>> "apply right shift with")
    .. = (make-sym-binary-op-dispatch '__.. '__r.. "join")
    @ = (make-asym-binary-op-dispatch '__@ usize "apply subscript operator with")
    getattr = (make-asym-binary-op-dispatch '__getattr Symbol "get attribute from")
    lslice = (make-asym-binary-op-dispatch '__lslice usize "apply left-slice operator with")
    rslice = (make-asym-binary-op-dispatch '__rslice usize "apply right-slice operator with")

compile-stage;

let null = (nullof NullType)
#inline Syntax-unbox (self destT)
    imply ('datum self) destT

inline not (value)
    bxor (imply value bool) true

let function->SyntaxMacro =
    typify
        fn "function->SyntaxMacro" (f)
            bitcast f SyntaxMacro
        SyntaxMacroFunction

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

#fn cons (at next)
    sc_list_cons (Value at) next

fn type-repr-needs-suffix? (CT)
    if (== CT i32) false
    elseif (== CT bool) false
    elseif (== CT Nothing) false
    elseif (== CT NullType) false
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
    'dispatch-attr (typeof value) '__tostring
        inline (f)
            f value
        inline ()
            sc_value_tostring (Value value)

fn repr (value)
    let T = (typeof value)
    let s =
        'dispatch-attr T '__repr
            inline (f)
                f value
            inline ()
                sc_value_repr (Value value)
    if (type-repr-needs-suffix? T)
        .. s
            ..
                sc_default_styler style-operator ":"
                sc_default_styler style-type ('string T)

    else s

let print =
    do
        inline print-element (i key value)
            constbranch (const.icmp<=.i32.i32 i 0)
                inline ()
                inline ()
                    sc_write " "
            constbranch (== (typeof value) string)
                inline ()
                    sc_write value
                inline ()
                    sc_write (repr value)

        fn print (values...)
            va-lifold none print-element values...
            sc_write "\n"
            values...

'set-symbol integer '__typecall
    inline (cls value)
        as value cls

'set-symbol real '__typecall
    inline (cls value)
        as value cls

# implicit argument type coercion for functions, externs and typed labels
# --------------------------------------------------------------------------

let coerce-call-arguments =
    box-ast-macro
        fn "coerce-call-arguments" (args)
            let argc = ('argcount args)
            verify-count argc 1 -1
            let self = ('getarg args 0)
            let argc = (sub argc 1)
            let fptrT = ('typeof self)
            let fT = ('element@ fptrT 0)
            let pcount = ('element-count fT)
            if (== pcount argc)
                let outargs = (sc_call_new self)
                sc_call_set_rawcall outargs true
                loop (i) = 0
                if (< i argc)
                    let arg = ('getarg args (add i 1))
                    let argT = ('typeof arg)
                    let paramT = ('element@ fT i)
                    let outarg =
                        if (== argT paramT) arg
                        else `(imply arg paramT)
                    sc_call_append_argument outargs outarg
                    repeat (+ i 1)
                outargs
            else `(rawcall self [(sc_extract_argument_list_new args 1)])

#
    set-type-symbol! pointer 'set-element-type
        fn (cls ET)
            pointer-type-set-element-type cls ET
    set-type-symbol! pointer 'set-storage
        fn (cls storage)
            pointer-type-set-storage-class cls storage
    set-type-symbol! pointer 'immutable
        fn (cls ET)
            pointer-type-set-flags cls
                bor (pointer-type-flags cls) pointer-flag-non-writable
    set-type-symbol! pointer 'mutable
        fn (cls ET)
            pointer-type-set-flags cls
                band (pointer-type-flags cls)
                    bxor pointer-flag-non-writable -1:u64
    set-type-symbol! pointer 'strip-storage
        fn (cls ET)
            pointer-type-set-storage-class cls unnamed
    set-type-symbol! pointer 'storage
        fn (cls)
            pointer-type-storage-class cls
    set-type-symbol! pointer 'readable?
        fn (cls)
            == (& (pointer-type-flags cls) pointer-flag-non-readable) 0:u64

fn pointer-type-imply? (src dest)
    let ET = ('element@ src 0)
    let ET =
        if ('opaque? ET) ET
        else ('storage ET)
    if (not (icmp== ('kind ET) type-kind-pointer))
        # casts to voidstar are only permitted if we are not holding
        # a ref to another pointer
        if (type== dest voidstar)
            return true
        elseif (type== dest ('set-pointer-mutable voidstar))
            if ('pointer-writable? src)
                return true
    if (type== dest ('strip-pointer-storage-class src))
        return true
    elseif (type== dest ('set-pointer-immutable src))
        return true
    elseif (type== dest ('strip-pointer-storage-class ('set-pointer-immutable src)))
        return true
    return false

fn pointer-imply (vT T expr)
    if (icmp== ('kind T) type-kind-pointer)
        if (pointer-type-imply? vT T)
            return `(bitcast expr T)
    compiler-error! "unsupported type"

'set-symbols pointer
    __call = coerce-call-arguments
    __imply = (box-cast pointer-imply)

# dotted symbol expander
# --------------------------------------------------------------------------

let dot-char = 46:i8 # "."
let dot-sym = '.

fn dotted-symbol? (env head)
    if (== head dot-sym)
        return false
    let s = (as head string)
    let sz = (countof s)
    loop (i) = 0:usize
    if (== i sz)
        return false
    elseif (== (@ s i) dot-char)
        return true
    repeat (+ i 1:usize)

fn split-dotted-symbol (head start end tail)
    let s = (as head string)
    loop (i) = start
    if (== i end)
        # did not find a dot
        if (== start 0:usize)
            return (cons head tail)
        else
            return (cons (Symbol (rslice s start)) tail)
    if (== (@ s i) dot-char)
        let tail =
            # no remainder after dot
            if (== i (- end 1:usize)) tail
            else # remainder after dot, split the rest first
                split-dotted-symbol head (+ i 1:usize) end tail
        let result = (cons dot-sym tail)
        if (== i 0:usize)
            # no prefix before dot
            return result
        else
            # prefix before dot
            let size = (- i start)
            return
                cons (Symbol (lslice (rslice s start) size)) result
    repeat (+ i 1:usize)

# infix notation support
# --------------------------------------------------------------------------

fn get-ifx-symbol (name)
    Symbol (.. "#ifx:" name)

fn expand-define-infix (args scope order)
    let prec rest = ('decons args)
    let token rest = ('decons rest)
    let func rest = ('decons rest)
    let prec =
        as prec i32
    let token =
        as token Symbol
    let func =
        if (== ('typeof func) Nothing) token
        else
            as func Symbol
    'set-symbol scope (get-ifx-symbol token)
        Value (cons prec (cons order (cons func '())))
    return none scope

inline make-expand-define-infix (order)
    fn (args scope)
        expand-define-infix args scope order

fn get-ifx-op (env op)
    let sym = op
    if (== ('typeof sym) Symbol)
        '@ env (get-ifx-symbol (as sym Symbol))
    else
        return false (Value none)

fn has-infix-ops? (infix-table expr)
    # any expression of which one odd argument matches an infix operator
        has infix operations.
    loop (expr) = expr
    if (< (countof expr) 3:usize)
        return false
    let __ expr = ('decons expr)
    let at next = ('decons expr)
    let ok result = (get-ifx-op infix-table at)
    if ok
        return true
    repeat expr

fn unpack-infix-op (op)
    let op = (as op list)
    let op-prec rest = ('decons op)
    let op-order rest = ('decons rest)
    let op-func rest = ('decons rest)
    return
        as op-prec i32
        as op-order Symbol
        as op-func Symbol

inline infix-op (pred)
    fn infix-op (infix-table token prec)
        let ok op =
            get-ifx-op infix-table token
        if ok
            let op-prec = (unpack-infix-op op)
            ? (pred op-prec prec) op (Value none)
        else
            sc_set_active_anchor ('anchor token)
            compiler-error!
                "unexpected token in infix expression"
let infix-op-gt = (infix-op >)
let infix-op-ge = (infix-op >=)

fn rtl-infix-op-eq (infix-table token prec)
    let ok op =
        get-ifx-op infix-table token
    if ok
        let op-prec op-order = (unpack-infix-op op)
        if (== op-order '<)
            ? (== op-prec prec) op (Value none)
        else
            Value none
    else
        sc_set_active_anchor ('anchor token)
        compiler-error!
            "unexpected token in infix expression"

fn parse-infix-expr (infix-table lhs state mprec)
    loop (lhs state) = lhs state
    if (empty? state)
        return lhs state
    let la next-state = ('decons state)
    let op = (infix-op-ge infix-table la mprec)
    if (== ('typeof op) Nothing)
        return lhs state
    let op-prec op-order op-name = (unpack-infix-op op)
    let next-lhs next-state =
        do
            loop (rhs state) = ('decons next-state)
            if (empty? state)
                break (Value (list op-name lhs rhs)) state
            let ra __ = ('decons state)
            let lop = (infix-op-gt infix-table ra op-prec)
            let nextop =
                if (== ('typeof lop) Nothing)
                    rtl-infix-op-eq infix-table ra op-prec
                else lop
            if (== ('typeof nextop) Nothing)
                break (Value (list op-name lhs rhs)) state
            let nextop-prec = (unpack-infix-op nextop)
            let next-rhs next-state =
                parse-infix-expr infix-table rhs state nextop-prec
            repeat next-rhs next-state
    repeat next-lhs next-state

let parse-infix-expr =
    typify parse-infix-expr Scope Value list i32

#---------------------------------------------------------------------------

# install general list hook for this scope
# is called for every list the expander would otherwise consider a call
fn list-handler (topexpr env)
    let topexpr-at topexpr-next = ('decons topexpr)
    let sxexpr = topexpr-at
    let expr expr-anchor = sxexpr ('anchor sxexpr)
    if (!= ('typeof expr) list)
        return topexpr env
    let expr = (as expr list)
    let expr-at expr-next = ('decons expr)
    let head-key = expr-at
    let head =
        if (== ('typeof head-key) Symbol)
            let ok head = ('@ env (as head-key Symbol))
            if ok head
            else head-key
        else head-key
    let head =
        if (== ('typeof head) type)
            let ok attr = ('@ (as head type) '__macro)
            if ok attr
            else head
        else head
    if (== ('typeof head) SyntaxMacro)
        let head = (as head SyntaxMacro)
        let expr env = (head expr topexpr-next env)
        # todo: attach expr-anchor
        #let expr = (Syntax-wrap expr-anchor (Value expr) false)
        return (as expr list) env
    elseif (has-infix-ops? env expr)
        let at next = ('decons expr)
        let expr =
            parse-infix-expr env at next 0
        #let expr = (Syntax-wrap expr-anchor expr false)
        return (cons expr topexpr-next) env
    else
        return topexpr env

# install general symbol hook for this scope
# is called for every symbol the expander could not resolve
fn symbol-handler (topexpr env)
    let at next = ('decons topexpr)
    let sxname = at
    let name name-anchor = (as sxname Symbol) ('anchor sxname)
    if (dotted-symbol? env name)
        let s = (as name string)
        let sz = (countof s)
        let expr =
            Value (split-dotted-symbol name 0:usize sz '())
        #let expr = (Syntax-wrap name-anchor expr false)
        return (cons expr next) env
    return topexpr env

fn quasiquote-list
inline quasiquote-any (ox)
    let x = ox
    let T = ('typeof x)
    if (== T list)
        quasiquote-list (as x list)
    else
        list syntax-quote ox
fn quasiquote-list (x)
    if (empty? x)
        return (list syntax-quote x)
    let aat next = ('decons x)
    let at = aat
    let T = ('typeof at)
    if (== T list)
        let at = (as at list)
        if (not (empty? at))
            let at-at at-next = ('decons at)
            if (== ('typeof at-at) Symbol)
                let at-at = (as at-at Symbol)
                if (== at-at 'unquote-splice)
                    return
                        list (Value sc_list_join)
                            cons do at-next
                            quasiquote-list next
                elseif (== at-at 'square-list)
                    if (> (countof at-next) 1:usize)
                        return
                            list (Value sc_list_join)
                                list list (cons _ at-next)
                                quasiquote-list next
    elseif (== T Symbol)
        let at = (as at Symbol)
        if (== at 'unquote)
            return (cons do next)
        elseif (== at 'square-list)
            return (cons do next)
        elseif (== at 'quasiquote)
            return (quasiquote-list (quasiquote-list next))
    return
        list cons (quasiquote-any aat) (quasiquote-list next)

fn expand-and-or (expr f)
    if (empty? expr)
        compiler-error! "at least one argument expected"
    elseif (== (countof expr) 1:usize)
        return ('@ expr)
    let expr = ('reverse expr)
    loop (result head) = ('decons expr)
    if (empty? head)
        return result
    let at next = ('decons head)
    repeat (Value (list f at (list inline '() result))) next

#compile-stage
    let vals = (alloca-array type 2)
    store list (getelementptr vals 0)
    store Value (getelementptr vals 1)
    sc_compile
        sc_typify expand-and-or 2 vals
        0:u64
    exit 0

inline make-expand-and-or (f)
    fn (expr)
        expand-and-or expr f

fn ltr-multiop (args target)
    let argc = ('argcount args)
    verify-count argc 2 -1
    if (== argc 2)
        `(target args)
    else
        # call for multiple args
        let lhs = ('getarg args 0)
        loop (i lhs) = 1 lhs
        let rhs = ('getarg args i)
        let op = `(target lhs rhs)
        let i = (+ i 1)
        if (< i argc)
            repeat i op
        op

fn rtl-multiop (args target)
    let argc = ('argcount args)
    verify-count argc 2 -1
    if (== argc 2)
        `(target args)
    else
        # call for multiple args
        let lasti = (- argc 1)
        let rhs = ('getarg args lasti)
        loop (i rhs) = lasti rhs
        let i = (- i 1)
        let lhs = ('getarg args i)
        let op = `(target lhs rhs)
        if (> i 0)
            repeat i op
        op

# extracting options from varargs

# (va-option-branch key elsef args...)
fn va-option-branch (args)
    let argc = ('argcount args)
    verify-count argc 2 -1
    let key elsef =
        'getarg args 0
        'getarg args 1
    let key = (unbox-symbol key Symbol)
    loop (i) = 2
    if (< i argc)
        let arg = ('getarg args i)
        let argkey = ('key ('typeof arg))
        if (== key argkey)
            return
                sc_keyed_new unnamed arg
        repeat (+ i 1)
    `(elsef)

# modules
####

let package = (Scope)
'set-symbols package
    path =
        Value
            list
                .. compiler-dir "/lib/scopes/?.sc"
                .. compiler-dir "/lib/scopes/?/init.sc"
    modules = (Value (Scope))

fn clone-scope-contents (a b)
    """"Join two scopes ``a`` and ``b`` into a new scope so that the
        root of ``a`` descends from ``b``.
    # search first upwards for the root scope of a, then clone a
        piecewise with the cloned scopes as parents
    let parent = ('parent a)
    if (== parent null)
        return (Scope b a)
    Scope
        clone-scope-contents parent b
        a

'set-symbols typename
    __typecall =
        box-ast-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 1 -1
                let cls = ('getarg args 0)
                if (ptrcmp== ('typeof cls) type)
                    let cls = (as cls type)
                    if (ptrcmp== cls typename)
                        verify-count argc 2 4
                        let name = ('getarg args 1)
                        let T =
                            sc_typename_type
                                as name string
                        loop (i) = 2
                        if (< i argc)
                            let arg = ('getarg args i)
                            let k v = (sc_type_key (sc_value_type arg))
                            if (== k 'super)
                                sc_typename_type_set_super T (as v type)
                            elseif (== k 'storage)
                                sc_typename_type_set_storage T (as v type)
                            else
                                compiler-error! "super or storage key expected"
                        return (Value T)
                compiler-error!
                    sc_string_join "typename "
                        sc_string_join ('__repr cls)
                            " has no constructor"

'set-symbols Scope
    __.. =
        box-binary-op
            single-binary-op-dispatch clone-scope-contents

let constant? =
    ast-macro
        fn "constant?" (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let value = ('getarg args 0)
            Value ('constant? value)

let Closure->Generator =
    ast-macro
        fn "Closure->Generator" (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let self = ('getarg args 0)
            if (not ('constant? self))
                compiler-error! "Closure must be constant"
            let self = (as self Closure)
            let self = (bitcast self Generator)
            Value self

# (define name expr ...)
fn expand-define (expr)
    raises-compile-error;
    let defname = ('@ expr)
    let content = ('next expr)
    list let defname '=
        cons do content

let
    qq =
        syntax-macro
            fn (args)
                if (== (countof args) 1)
                    quasiquote-any ('@ args)
                else
                    quasiquote-list args
    # dot macro
    # (. value symbol ...)
    . =
        syntax-macro
            fn (args)
                fn op (a b)
                    let sym = (as b Symbol)
                    list getattr a (list syntax-quote sym)
                let a rest = ('decons args)
                let b rest = ('decons rest)
                loop (rest result) = rest (op a b)
                if (empty? rest)
                    result
                else
                    let c rest = ('decons rest)
                    repeat rest (op result c)
    and = (syntax-macro (make-expand-and-or and-branch))
    or = (syntax-macro (make-expand-and-or or-branch))
    define = (syntax-macro expand-define)
    define-infix> = (syntax-scope-macro (make-expand-define-infix '>))
    define-infix< = (syntax-scope-macro (make-expand-define-infix '<))
    .. = (ast-macro (fn (args) (rtl-multiop args (Value ..))))
    + = (ast-macro (fn (args) (ltr-multiop args (Value +))))
    * = (ast-macro (fn (args) (ltr-multiop args (Value *))))
    va-option-branch = (ast-macro va-option-branch)
    syntax-set-scope! =
        syntax-scope-macro
            fn (args syntax-scope)
                raises-compile-error;
                let scope rest = (decons args)
                return
                    none
                    as scope Scope

'set-symbol (__this-scope) (Symbol "#list")
    Value (typify list-handler list Scope)
'set-symbol (__this-scope) (Symbol "#symbol")
    Value (typify symbol-handler list Scope)

compile-stage;

define-infix< 50 +=
define-infix< 50 -=
define-infix< 50 *=
define-infix< 50 /=
define-infix< 50 //=
define-infix< 50 %=
define-infix< 50 >>=
define-infix< 50 <<=
define-infix< 50 &=
define-infix< 50 |=
define-infix< 50 ^=
define-infix< 50 =

define-infix> 100 or
define-infix> 200 and

define-infix> 300 <
define-infix> 300 >
define-infix> 300 <=
define-infix> 300 >=
define-infix> 300 !=
define-infix> 300 ==

define-infix> 340 |
define-infix> 350 ^
define-infix> 360 &

define-infix< 400 ..
define-infix> 450 <<
define-infix> 450 >>
define-infix> 500 -
define-infix> 500 +
define-infix> 600 %
define-infix> 600 /
define-infix> 600 //
define-infix> 600 *
define-infix< 700 ** pow
define-infix> 750 as
define-infix> 800 .
define-infix> 800 @

inline char (s)
    let s sz = (sc_string_buffer s)
    load s

# (va-option key args... else-body)
let va-option =
    syntax-macro
        fn (args)
            let key va body = (decons args 2)
            let sym = (as key Symbol)
            list va-option-branch (list syntax-quote sym)
                cons inline '() body
                va

#---------------------------------------------------------------------------
# for iterator
#---------------------------------------------------------------------------

'set-symbols Generator
    __typecall =
        inline "Generator-new" (cls iter init)
            Closure->Generator
                inline "get-iter-init" ()
                    _ iter init
    __call =
        ast-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 1 1
                let self = ('getarg args 0)
                if (not ('constant? self))
                    compiler-error! "Generator must be constant"
                let self = (self as Generator)
                let self = (bitcast self Closure)
                `(self)

# typical pattern for a generator:
    inline make-generator (init end?)
        Generator
            inline (fdone x)
                if (end? x)
                    # terminate
                    fdone;
                else
                    # return next iterator and result values
                    _ ('next x) ('@ x)
            init

let for =
    syntax-macro
        fn "for" (args)
            loop (it params) = args '()
            if (empty? it)
                compiler-error! "'in' expected"
            let sxat it = (decons it)
            let at = (sxat as Symbol)
            if (at != 'in)
                repeat it (cons sxat params)
            let generator-expr body = (decons it)
            let params = (sc_list_reverse params)
            let iter = (sc_symbol_new_unique "iter")
            let next = (sc_symbol_new_unique "next")
            let start = (sc_symbol_new_unique "start")
            inline fdone ()
                break;
            list do
                list let iter start '= (list (list (do as) generator-expr Generator))
                list loop (list next) '= start
                cons let next
                    'join params
                        list '=
                            list iter fdone next
                list inline 'continue '()
                    list repeat next
                cons do body
                list 'continue

#---------------------------------------------------------------------------
# module loading
#---------------------------------------------------------------------------

let wrap-if-not-compile-stage =
    ast-macro
        fn (args)
            raises-compile-error;
            let argc = ('argcount args)
            if (argc == 1)
                let arg = ('getarg args 0)
                if (('typeof arg) == CompileStage)
                    return arg
            `(Value args)

compile-stage;

fn make-module-path (pattern name)
    let sz = (countof pattern)
    loop (i start result) = 0:usize 0:usize ""
    if (i == sz)
        return (.. result (rslice pattern start))
    if ((@ pattern i) != (char "?"))
        repeat (i + 1:usize) start result
    else
        repeat (i + 1:usize) (i + 1:usize)
            .. result (rslice (lslice pattern i) start) name

fn exec-module (expr eval-scope)
    let ModuleFunctionType = ('pointer ('raising (function Value) Error))
    let StageFunctionType = ('pointer ('raising (function CompileStage) Error))
    let expr-anchor = ('anchor expr)
    sc_set_active_anchor expr-anchor
    let f = (sc_eval expr-anchor expr eval-scope)
    loop (f) = f
    # build a wrapper
    let wrapf =
        ast-quote
            fn "exec-module-stage" ()
                raises-compile-error;
                wrap-if-not-compile-stage (f)
    let wrapf = (sc_typify_template wrapf 0 (undef TypeArrayPointer))
    let f = (sc_compile wrapf 0:u64)
    if (('typeof f) == StageFunctionType)
        let fptr = (f as StageFunctionType)
        repeat (bitcast (fptr) Value)
    else
        let fptr = (f as ModuleFunctionType)
        fptr;

fn dots-to-slashes (pattern)
    let sz = (countof pattern)
    loop (i start result) = 0:usize 0:usize ""
    if (i == sz)
        return (.. result (rslice pattern start))
    let c = (@ pattern i)
    if (c == (char "/"))
        compiler-error!
            .. "no slashes permitted in module name: " pattern
    elseif (c == (char "\\"))
        compiler-error!
            .. "no slashes permitted in module name: " pattern
    elseif (c != (char "."))
        repeat (i + 1:usize) start result
    elseif (icmp== (i + 1:usize) sz)
        compiler-error!
            .. "invalid dot at ending of module '" pattern "'"
    else
        if (icmp== i start)
            if (icmp>u start 0:usize)
                repeat (i + 1:usize) (i + 1:usize)
                    .. result (rslice (lslice pattern i) start) "../"
        repeat (i + 1:usize) (i + 1:usize)
            .. result (rslice (lslice pattern i) start) "/"

fn load-module (module-name module-path opts...)
    if (not (sc_is_file module-path))
        compiler-error!
            .. "no such module: " module-path
    let module-path = (sc_realpath module-path)
    let module-dir = (sc_dirname module-path)
    let expr = (sc_parse_from_path module-path)
    let eval-scope =
        va-option scope opts...
            Scope (sc_get_globals)
    'set-symbols eval-scope
        main-module? =
            va-option main-module? opts... false
        module-path = module-path
        module-dir = module-dir
        module-name = module-name
    exec-module expr (Scope eval-scope)

fn patterns-from-namestr (base-dir namestr)
    # if namestr starts with a slash (because it started with a dot),
        we only search base-dir
    if ((@ namestr 0:usize) == (char "/"))
        list
            .. base-dir "?.sc"
            .. base-dir "?/init.sc"
    else
        package.path as list

let incomplete = (typename "incomplete")
fn require-from (base-dir name)
    #assert-typeof name Symbol
    let namestr = (dots-to-slashes (name as string))
    inline load-module-from-symbol (name)
        let package = ((fn () package))
        let modules = (package.modules as Scope)
        loop (patterns) = (patterns-from-namestr base-dir namestr)
        if (empty? patterns)
            break false (Value none)
        let pattern patterns = (decons patterns)
        let pattern = (pattern as string)
        let module-path = (sc_realpath (make-module-path pattern namestr))
        if (empty? module-path)
            repeat patterns
        let module-path-sym = (Symbol module-path)
        let ok content = ('@ modules module-path-sym)
        if ok
            if (('typeof content) == type)
                if (content == incomplete)
                    compiler-error!
                        .. "trying to import module " (repr name)
                            " while it is being imported"
            break true content
        if (not (sc_is_file module-path))
            repeat patterns
        'set-symbol modules module-path-sym incomplete
        let content = (load-module (name as string) module-path)
        'set-symbol modules module-path-sym content
        break true content
    let ok content = (load-module-from-symbol name)
    if ok
        return content
    sc_write "no such module '"
    sc_write (as name string)
    sc_write "' in paths:\n"
    loop (patterns) = (patterns-from-namestr base-dir namestr)
    if (empty? patterns)
        compiler-error! "failed to import module"
    let pattern patterns = (decons patterns)
    let pattern = (pattern as string)
    let module-path = (make-module-path pattern namestr)
    sc_write "    "
    sc_write module-path
    sc_write "\n"
    repeat patterns

""""export locals as a chain of two new scopes: a scope that contains
    all the constant values in the immediate scope, and a scope that contains
    the runtime values.
let locals =
    syntax-scope-macro
        fn "locals" (args scope)
            raises-compile-error;
            let docstr = ('docstring scope unnamed)
            let constant-scope = (Scope)
            if (not (empty? docstr))
                'set-docstring! constant-scope unnamed docstr
            let tmp =
                `(Scope constant-scope)
            loop (last-key result) = unnamed (list tmp)
            let key value =
                'next scope last-key
            if (key == unnamed)
                return
                    cons do tmp result
                    scope
            else
                let keydocstr = ('docstring scope key)
                repeat key
                    if (key == unnamed)
                        # skip
                        result
                    else
                        if ('constant? value)
                            'set-symbol constant-scope key value
                            'set-docstring! constant-scope key keydocstr
                            result
                        else
                            let value = (sc_extract_argument_new value 0)
                            cons
                                list sc_scope_set_symbol tmp (list syntax-quote key) (list Value value)
                                list sc_scope_set_docstring tmp (list syntax-quote key) keydocstr
                                result

#---------------------------------------------------------------------------
# using
#---------------------------------------------------------------------------

fn merge-scope-symbols (source target filter)
    fn process-keys (source target filter)
        loop (last-key) = unnamed
        let key value = ('next source last-key)
        if (key != unnamed)
            if
                or
                    none? filter
                    do
                        let keystr = (key as string)
                        sc_string_match filter keystr
                'set-symbol target key value
            repeat key
        else
            target
    fn filter-contents (source target filter)
        let parent = ('parent source)
        if (parent == null)
            return
                process-keys source target filter
        process-keys source
            filter-contents parent target filter
            filter
    filter-contents source target filter

let using =
    syntax-scope-macro
        fn "using" (args syntax-scope)
            let name rest = (decons args)
            let nameval = name
            if ((('typeof nameval) == Symbol) and ((nameval as Symbol) == 'import))
                let ok module-dir = ('@ syntax-scope 'module-dir)
                if (not ok)
                    compiler-error!
                        "using import requires module-dir symbol in scope"
                let module-dir = (module-dir as string)
                let name rest = (decons rest)
                let name = (name as Symbol)
                let module = ((require-from module-dir name) as Scope)
                return (list do none)
                    .. module syntax-scope

            let pattern =
                if (empty? rest)
                    '()
                else
                    let token pattern rest = (decons rest 2)
                    let token = (token as Symbol)
                    if (token != 'filter)
                        compiler-error!
                            "syntax: using <scope> [filter <filter-string>]"
                    let pattern = (pattern as string)
                    list pattern
            # attempt to import directly if possible
            inline process (src)
                _ (list do)
                    if (empty? pattern)
                        merge-scope-symbols src syntax-scope none
                    else
                        merge-scope-symbols src syntax-scope (('@ pattern) as string)
            if (('typeof nameval) == Symbol)
                let sym = (nameval as Symbol)
                let ok src = ('@ syntax-scope sym)
                if (ok and (('typeof src) == Scope))
                    return (process (src as Scope))
            elseif (('typeof nameval) == Scope)
                return (process (nameval as Scope))
            return
                list compile-stage
                    cons merge-scope-symbols name 'syntax-scope pattern
                syntax-scope

#fn from (args)
    inline load-from (src keys...)
        let loop (i result...) = (va-countof keys...)
        if (i == 0)
            result...
        else
            let i = (i - 1)
            let key = (va@ i keys...)
            loop i
                src @ key
                result...
    let src kw params = (decons args 2)
    if ((kw as Syntax as Symbol) != 'let)
        syntax-error! kw "`let` keyword expected"
    fn quotify (params)
        if (empty? params)
            unconst '()
        else
            let entry rest = (decons params)
            entry as Syntax as Symbol
            cons
                list syntax-quote entry
                quotify rest
    cons let
        .. params
            list '=
                cons load-from src
                    quotify params

# (define-macro name expr ...)
# implies builtin names:
    args : list
define define-syntax-macro
    syntax-macro
        fn "expand-define-syntax-macro" (expr)
            raises-compile-error;
            let name body = (decons expr)
            list define name
                list syntax-macro
                    cons fn '(args)
                        list raises-compile-error;
                        body

let __assert =
    ast-macro
        fn (args)
            fn check-assertion (result anchor msg)
                if (not result)
                    sc_set_active_anchor anchor
                    compiler-error!
                        .. "assertion failed: " msg
                return;

            let argc = ('argcount args)
            verify-count argc 2 2
            let expr msg =
                'getarg args 0
                'getarg args 1
            let anchor = (sc_get_active_anchor)
            if ('constant? expr)
                let msg = (msg as string)
                let val = (expr as bool)
                check-assertion val anchor msg
                box-empty;
            else
                `(check-assertion expr anchor msg)

let vector-reduce =
    ast-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 2 2
            let f v =
                'getarg args 0
                'getarg args 1
            let T = ('typeof v)
            let sz = ('element-count T)
            loop (v sz) = v sz
            # special cases for low vector sizes
            switch sz
            case 1
                ast-quote
                    extractelement v 0
            case 2
                ast-quote
                    f
                        extractelement v 0
                        extractelement v 1
            case 3
                ast-quote
                    f
                        f
                            extractelement v 0
                            extractelement v 1
                        extractelement v 2
            case 4
                ast-quote
                    f
                        f
                            extractelement v 0
                            extractelement v 1
                        f
                            extractelement v 2
                            extractelement v 3
            default
                let hsz = (sz >> 1)
                let fsz = (hsz << 1)
                if (fsz != sz)
                    compiler-error! "vector size must be a power of two"
                let hsz-value = (Value (hsz as usize))
                repeat
                    ast-quote
                        f
                            lslice v hsz-value
                            rslice v hsz-value
                    hsz

let __countof-aggregate =
    ast-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let self = ('getarg args 0)
            let T = ('typeof self)
            let sz = ('element-count T)
            Value (sz as usize)

compile-stage;

# (define-scope-macro name expr ...)
# implies builtin names:
    args : list
    scope : Scope
define-syntax-macro define-syntax-scope-macro
    let name body = (decons args)
    list define name
        list syntax-scope-macro
            cons fn '(args syntax-scope) body

# (define-block-scope-macro name expr ...)
# implies builtin names:
    expr : list
    next-expr : list
    scope : Scope
define-syntax-macro define-syntax-block-scope-macro
    let name body = (decons args)
    list define name
        list syntax-block-scope-macro
            cons fn '(expr next-expr syntax-scope) body

inline list-generator (self)
    Generator
        inline (fdone cell)
            if (empty? cell)
                fdone;
            else
                let at next = (decons cell)
                _ next at
        self

'set-symbols list
    __as =
        box-cast
            fn "list-as" (vT T expr)
                if (T == Generator)
                    return `(list-generator expr)
                compiler-error! "unsupported type"

'set-symbols Value
    args =
        inline "Value-args" (self)
            let argc = ('argcount self)
            Generator
                inline (fdone x)
                    if (x < argc)
                        _ (x + 1) ('getarg self x)
                    else
                        fdone;
                0

inline range (a b c)
    let num-type = (typeof a)
    let step =
        constbranch (none? c)
            inline () (1 as num-type)
            inline () c
    let from =
        constbranch (none? b)
            inline () (0 as num-type)
            inline () a
    let to =
        constbranch (none? b)
            inline () a
            inline () b
    Generator
        inline (fdone x)
            if (x < to)
                _ (x + step) x
            else
                fdone;
        from

fn parse-compile-flags (args)
    let argc = ('argcount args)
    loop (i flags) = 0 0:u64
    if (i < argc)
        let arg = ('getarg args i)
        let flag = (arg as Symbol)
        repeat (i + 1)
            | flags
                switch flag
                case 'dump-disassembly compile-flag-dump-disassembly
                case 'dump-module compile-flag-dump-module
                case 'dump-function compile-flag-dump-function
                case 'dump-time compile-flag-dump-time
                case 'no-debug-info compile-flag-no-debug-info
                case 'O1 compile-flag-O1
                case 'O2 compile-flag-O2
                case 'O3 compile-flag-O3
                default
                    compiler-error!
                        .. "illegal flag: " (repr flag)
                            ". try one of"
                            \ " " (repr 'dump-disassembly)
                            \ " " (repr 'dump-module)
                            \ " " (repr 'dump-function)
                            \ " " (repr 'dump-time)
                            \ " " (repr 'no-debug-info)
                            \ " " (repr 'O1)
                            \ " " (repr 'O2)
                            \ " " (repr 'O3)
    flags

let compile =
    ast-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 -1
            let func = ('getarg args 0)
            let flags =
                parse-compile-flags
                    'getarglist args 1
            if ('pure? func)
                sc_compile func flags
            else
                `(sc_compile func flags)

define-syntax-macro assert
    let cond msg body = (decons args 2)
    let msg =
        if ((countof args) == 2:usize) msg
        else
            Value
                repr cond
    list __assert cond msg

define-syntax-macro while
    let cond body = (decons args)
    list do
        list loop '()
        list inline 'continue '()
            list repeat
        list if cond
            cons do body
            list repeat

#-------------------------------------------------------------------------------
# tuples
#-------------------------------------------------------------------------------

inline make-unpack-function (extractf)
    ast-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let self = ('getarg args 0)
            let T = ('typeof self)
            let count = ('element-count T)
            let outargs = (sc_argument_list_new)
            loop (i) = 0
            if (icmp<s i count)
                sc_argument_list_append outargs `(extractf self i)
                repeat (add i 1)
            outargs

let __unpack-aggregate = (make-unpack-function extractvalue)

'set-symbols tuple
    __unpack = __unpack-aggregate
    __countof = __countof-aggregate
    __@ =
        inline (self index)
            extractvalue self index

let tupleof =
    ast-macro
        fn (args)
            let argc = ('argcount args)
            #verify-count argc 0 -1
            raises-compile-error;

            # build tuple type
            let field-types = (alloca-array type argc)
            loop (i) = 0
            if (i < argc)
                let arg = ('getarg args i)
                let T = ('typeof arg)
                store T (getelementptr field-types i)
                repeat (i + 1)

            # generate insert instructions
            let TT = (sc_tuple_type argc field-types)
            loop (i result) = 0 `(nullof TT)
            if (i < argc)
                let arg = ('getarg args i)
                repeat (i + 1)
                    `(insertvalue result arg i)
            result

#-------------------------------------------------------------------------------
# arrays
#-------------------------------------------------------------------------------

'set-symbols array
    __unpack = __unpack-aggregate
    __countof = __countof-aggregate
    __@ =
        inline (self index)
            extractvalue self index

let arrayof =
    ast-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 -1
            raises-compile-error;

            let ET = (('getarg args 0) as type)
            let numvals = (sub argc 1)

            # generate insert instructions
            let TT = (sc_array_type ET (usize numvals))
            loop (i result) = 0 `(nullof TT)
            if (i < numvals)
                let arg = ('getarg args (add i 1))
                let arg =
                    if ((sc_value_type arg) == ET) arg
                    else `(arg as ET)
                repeat (i + 1)
                    `(insertvalue result arg i)
            result

#-------------------------------------------------------------------------------
# vectors
#-------------------------------------------------------------------------------

fn any? (v)
    vector-reduce bor v
fn all? (v)
    vector-reduce band v

inline single-signed-vector-binary-op-dispatch (sf uf)
    fn (lhsT rhsT lhs rhs)
        if (ptrcmp== lhsT rhsT)
            let Ta = ('element@ lhsT 0)
            return
                ast-quote
                    call [
                        \ do
                            if ('signed? Ta)
                                Value sf
                            else
                                Value uf ]
                        \ lhs rhs
        compiler-error! "unsupported type"

'set-symbols integer
    __vector+ = (box-binary-op (single-binary-op-dispatch add))
    __vector- = (box-binary-op (single-binary-op-dispatch sub))
    __vector* = (box-binary-op (single-binary-op-dispatch mul))
    __vector// = (box-binary-op (single-signed-binary-op-dispatch sdiv udiv))
    __vector% = (box-binary-op (single-signed-binary-op-dispatch srem urem))
    __vector& = (box-binary-op (single-binary-op-dispatch band))
    __vector| = (box-binary-op (single-binary-op-dispatch bor))
    __vector^ = (box-binary-op (single-binary-op-dispatch bxor))
    __vector<< = (box-binary-op (single-binary-op-dispatch shl))
    __vector>> = (box-binary-op (single-signed-binary-op-dispatch ashr lshr))
    __vector== = (box-binary-op (single-binary-op-dispatch icmp==))
    __vector!= = (box-binary-op (single-binary-op-dispatch icmp!=))
    __vector> = (box-binary-op (single-signed-binary-op-dispatch icmp>s icmp>u))
    __vector>= = (box-binary-op (single-signed-binary-op-dispatch icmp>s icmp>=u))
    __vector< = (box-binary-op (single-signed-binary-op-dispatch icmp<s icmp<u))
    __vector<= = (box-binary-op (single-signed-binary-op-dispatch icmp<=s icmp<=u))

'set-symbols real
    __vector+ = (box-binary-op (single-binary-op-dispatch fadd))
    __vector- = (box-binary-op (single-binary-op-dispatch fsub))
    __vector* = (box-binary-op (single-binary-op-dispatch fmul))
    __vector/ = (box-binary-op (single-binary-op-dispatch fdiv))
    __vector% = (box-binary-op (single-binary-op-dispatch frem))
    __vector== = (box-binary-op (single-binary-op-dispatch fcmp==o))
    __vector!= = (box-binary-op (single-binary-op-dispatch fcmp!=u))
    __vector> = (box-binary-op (single-binary-op-dispatch fcmp>o))
    __vector>= = (box-binary-op (single-binary-op-dispatch fcmp>=o))
    __vector< = (box-binary-op (single-binary-op-dispatch fcmp<o))
    __vector<= = (box-binary-op (single-binary-op-dispatch fcmp<=o))

fn vector-binary-op-expr (symbol lhsT rhsT lhs rhs)
    let Ta = ('element@ lhsT 0)
    let ok f = ('@ Ta symbol)
    if ok
        let f = (unbox-binary-op-function-type f)
        return (f lhsT rhsT lhs rhs)
    compiler-error! "unsupported operation"

inline vector-binary-op-dispatch (symbol)
    box-binary-op
        fn (lhsT rhsT lhs rhs) (vector-binary-op-expr symbol lhsT rhsT lhs rhs)

'set-symbols vector
    __+ = (vector-binary-op-dispatch '__vector+)
    __- = (vector-binary-op-dispatch '__vector-)
    __* = (vector-binary-op-dispatch '__vector*)
    __/ = (vector-binary-op-dispatch '__vector/)
    __// = (vector-binary-op-dispatch '__vector//)
    __% = (vector-binary-op-dispatch '__vector%)
    __& = (vector-binary-op-dispatch '__vector&)
    __| = (vector-binary-op-dispatch '__vector|)
    __^ = (vector-binary-op-dispatch '__vector^)
    __== = (vector-binary-op-dispatch '__vector==)
    __!= = (vector-binary-op-dispatch '__vector!=)
    __> = (vector-binary-op-dispatch '__vector>)
    __>= = (vector-binary-op-dispatch '__vector>=)
    __< = (vector-binary-op-dispatch '__vector<)
    __<= = (vector-binary-op-dispatch '__vector<=)
    __lslice =
        ast-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 2 2
                let self offset =
                    'getarg args 0
                    'getarg args 1
                if (not ('constant? offset))
                    compiler-error! "slice offset must be constant"
                let T = ('typeof self)
                let sz = (('element-count T) as usize)
                let offset:usize = (offset as usize)
                if (offset:usize >= sz)
                    return self
                let offset = (offset:usize as i32)
                let maskvals = (alloca-array Value offset)
                loop (i) = 0
                if (i < offset)
                    store (Value i) (getelementptr maskvals i)
                    repeat (i + 1)
                let maskT =
                    sc_vector_type i32 offset:usize
                ast-quote
                    shufflevector self self
                        [ sc_const_aggregate_new maskT offset maskvals ]
    __rslice =
        ast-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 2 2
                let self offset =
                    'getarg args 0
                    'getarg args 1
                if (not ('constant? offset))
                    compiler-error! "slice offset must be constant"
                let T = ('typeof self)
                let sz = (('element-count T) as usize)
                let offset:usize = (offset as usize)
                if (offset:usize == 0)
                    return self
                let offset:usize =
                    ? (offset:usize > sz) sz offset:usize
                let total:usize = (sz - offset:usize)
                let offset = (offset:usize as i32)
                let total = (total:usize as i32)
                let maskvals = (alloca-array Value total)
                loop (i) = 0
                if (i < total)
                    store (Value (i + offset)) (getelementptr maskvals i)
                    repeat (i + 1)
                let maskT =
                    sc_vector_type i32 total:usize
                ast-quote
                    shufflevector self self
                        [ sc_const_aggregate_new maskT total maskvals ]
    __unpack = (Value (make-unpack-function extractelement))
    __countof = __countof-aggregate
    # vector type constructor
    __typecall =
        ast-macro
            fn "vector" (args)
                let argc = ('argcount args)
                verify-count argc 3 3
                let ET = (unbox-pointer ('getarg args 1) type)
                let size = ('getarg args 2)
                let sizeT = (sc_value_type size)
                if (type< sizeT integer)
                else
                    unbox-verify size integer
                let size =
                    bitcast (sc_const_int_extract size) usize
                box-pointer (sc_vector_type ET size)

let vectorof =
    ast-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 -1
            raises-compile-error;

            let ET = (('getarg args 0) as type)
            let numvals = (sub argc 1)

            # generate insert instructions
            let TT = (sc_vector_type ET (usize numvals))
            loop (i result) = 0 `(nullof TT)
            if (i < numvals)
                let arg = ('getarg args (add i 1))
                let arg =
                    if ((sc_value_type arg) == ET) arg
                    else `(arg as ET)
                repeat (i + 1) `(insertelement result arg i)
            result

#-------------------------------------------------------------------------------

let
    min =
        ast-macro
            fn (args)
                ltr-multiop args
                    Value
                        inline "min" (a b)
                            ? (<= a b) a b
    max =
        ast-macro
            fn (args)
                ltr-multiop args
                    Value
                        inline "max" (a b)
                            ? (>= a b) a b

inline clamp (x mn mx)
    ? (> x mx) mx
        ? (< x mn) mn x

#-------------------------------------------------------------------------------
# various C related sugar
#-------------------------------------------------------------------------------

# functions safecast to function pointers
'set-symbols Closure
    __imply =
        box-cast
            fn (srcT destT expr)
                if ('function-pointer? destT)
                    let funcT = ('element@ destT 0)
                    let sz = ('element-count funcT)
                    let func = (expr as Closure)
                    if ('variadic? funcT)
                        compiler-error! "cannot typify to variadic function"
                    let args = (alloca-array type sz)
                    for i in (range sz)
                        store ('element@ funcT i) (getelementptr args i)
                    let result =
                        sc_typify func sz args
                    let resultT = ('typeof result)
                    if (resultT != destT)
                        syntax-error! ('anchor result)
                            .. "function does not compile to type " (repr destT)
                                \ "but has type " (repr resultT)
                    return result
                compiler-error! "unsupported type"

let
    extern =
        ast-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 2 2
                let sym T =
                    'getarg args 0
                    'getarg args 1
                let sym = (sym as Symbol)
                let T = (T as type)
                Value
                    sc_extern_new sym T

#-------------------------------------------------------------------------------

define spice
    inline half-wrap-ast-macro (f)
        fn (args)
            return `[(f args)]

    inline wrap-ast-macro (f)
        ast-macro
            fn (args)
                return `[(f args)]

    syntax-macro
        fn "expand-spice" (expr)
            raises-compile-error;
            let name params body =
                do
                    let arg body = (decons expr)
                    if (('typeof arg) == list)
                        _ (Value "") arg body
                    else
                        let params body = (decons body)
                        _ arg params body

            let params = (params as list)
            let paramcount = ((countof params) as i32)

            let args = (Symbol "#args")
            loop (i rest body varargs) = 0 params body false
            if (not (empty? rest))
                let paramv rest = (decons rest)
                let param = (paramv as Symbol)
                let vararg? = ('vararg? param)
                let body =
                    if vararg?
                        if (not (empty? rest))
                            syntax-error! ('anchor paramv)
                                "vararg parameter is not in last place"
                        cons
                            qq
                                [let paramv] =
                                    [sc_getarglist args i];
                            body
                    else
                        cons
                            qq
                                [let paramv] =
                                    [sc_getarg args i];
                            body
                repeat (i + 1) rest body (| varargs vararg?)
            let content =
                cons (list args)
                    qq
                        [verify-count] ([sc_argcount args])
                            [(? varargs (sub paramcount 1) paramcount)]
                            [(? varargs -1 paramcount)]
                    body
            if (('typeof name) == Symbol)
                qq
                    [let name] =
                        [wrap-ast-macro]
                            [(cons inline (name as Symbol as string) content)]
            else
                qq
                    [half-wrap-ast-macro]
                        [(cons inline name content)]

compile-stage;

let infinite-range =
    Generator
        inline (fdone x)
            _ (x + 1) x
        0

inline zip (a b)
    let iter-a init-a = ((a as Generator))
    let iter-b init-b = ((b as Generator))
    Generator
        inline (fdone t)
            let a = (@ t 0)
            let b = (@ t 1)
            let next-a at-a... = (iter-a fdone a)
            let next-b at-b... = (iter-b fdone b)
            _ (tupleof next-a next-b) at-a... at-b...
        tupleof init-a init-b

inline enumerate (x)
    zip infinite-range x

#-------------------------------------------------------------------------------
# references
#-------------------------------------------------------------------------------

inline construct
inline copy-construct
inline destruct
inline move-construct

let ref = (typename "ref")

'set-symbols Value
    typeof& =
        fn (self)
            let T = ('typeof self)
            assert (T < ref)
                .. "argument must be of reference type, but is of type " (repr T)
            'element@ T 0

let ref-attribs-key = '__refattrs
'set-symbols type
    @& =
        fn "@&" (T name)
            loop (T) = T
            let ok attrs = ('local@ T ref-attribs-key)
            if ok
                let attrs = (attrs as type)
                let ok val = ('@ attrs name)
                if ok
                    return ok val
            if (T == typename)
                return false (nullof Value)
            repeat ('super T)
    set-symbol& =
        fn "set-symbol&" (T name value)
            let ok attrs = ('local@ T ref-attribs-key)
            let attrs =
                if ok (attrs as type)
                else
                    let attrs = (sc_typename_type (ref-attribs-key as string))
                    'set-symbol T ref-attribs-key (Value attrs)
                    attrs
            'set-symbol attrs name value

let deref =
    spice "deref" (values...)
        let count = ('argcount values...)
        let result = (sc_argument_list_new)
        for i kvalue in (enumerate ('args values...))
            let key value = ('dekey kvalue)
            let T = ('typeof value)
            sc_argument_list_append result
                if (T < ref)
                    let ok op = ('@ T '__deref)
                    if ok
                        let cmd =  `(op value)
                        if (key == unnamed) cmd
                        else
                            sc_keyed_new key cmd
                    else
                        syntax-error! ('anchor kvalue)
                            .. "deref attribute missing in value of type " (repr T)
                else
                    kvalue
        result

#set-type-symbol!& Any 'typeof
    inline (self)
        Any-typeof (deref self)

#set-type-symbol!& Any '__imply
    inline (src destT)
        Any-extract (deref src) destT

#inline box-binary-op (f)
    box-pointer (typify f type type Value Value)

#inline single-binary-op-dispatch (destf)
    fn (lhsT rhsT lhs rhs)
        if (ptrcmp== lhsT rhsT)
            return
                sc_call_new (Value destf) (Value-array lhs rhs)
        compiler-error! "unsupported type"

do
    fn ref-binary-op-expr (symbol func lhs rhs)
        `(func [ (deref as ASTMacro) ] lhs rhs)

    inline passthru-overload (sym func)
        'set-symbol ref sym
            box-binary-op
                fn (lhsT rhsT lhs rhs)
                    ref-binary-op-expr sym (Value func) lhs rhs

    passthru-overload '__== ==; passthru-overload '__!= !=
    passthru-overload '__< <; passthru-overload '__<= <=
    passthru-overload '__> >; passthru-overload '__>= >=
    passthru-overload '__& &; passthru-overload '__| |; passthru-overload '__^ ^
    passthru-overload '__+ +; passthru-overload '__- -
    passthru-overload '__* *; passthru-overload '__/ /
    #passthru-overload '__** pow
    passthru-overload '__// //
    passthru-overload '__% %
    passthru-overload '__<< <<; passthru-overload '__>> >>
    passthru-overload '__.. ..

#
    inline passthru-inplace-overload (methodname fallback)
        'set-symbol ref methodname

            fn (a b)
                let ET = (typeof& a)
                let op ok = (type@& ET methodname)
                if ok
                    return (op a b)
                let op ok = (type@& ET '__deref)
                if (not ok)
                    compiler-error!
                        .. (op-prettyname methodname)
                            \ " of reference not supported by value of type " (repr ET)
                    return;
                = a (fallback a b)
                true

    passthru-inplace-overload '__+= +
    passthru-inplace-overload '__-= -
    passthru-inplace-overload '__*= *
    passthru-inplace-overload '__/= /
    passthru-inplace-overload '__//= //
    passthru-inplace-overload '__%= %
    passthru-inplace-overload '__>>= >>
    passthru-inplace-overload '__<<= <<
    passthru-inplace-overload '__&= &
    passthru-inplace-overload '__|= |
    passthru-inplace-overload '__^= ^

fn ref-type (PT)
    assert (PT < pointer) "pointer type expected"
    let ET = ('element@ PT 0)
    let class = ('pointer-storage-class PT)
    let T =
        sc_typename_type
            ..
                if ('pointer-writable? PT) "&"
                else "(&)"
                if (class != unnamed)
                    .. "[" (class as string) "]"
                else ""
                tostring ET
    'set-super T ref
    'set-storage T PT
    'set-symbol T 'ElementType ET
    T

'set-symbols ref
    __= =
        ast-macro
            spice "ref=" (self value)
                let ET = ('typeof& self)
                let ok op = ('@& ET '__=)
                if ok `(op self value)
                else
                    ast-quote
                        destruct self
                        copy-construct self value
                        _;
    __typecall =
        ast-macro
            spice "ref-typecall" (cls T)
                let T = (T as type)
                if ((T < ref) or (not (T < pointer)))
                    compiler-error!
                        .. "cannot create reference type of reference type "
                            repr T
                ref-type T

#
    let voidstar = (pointer void)

    do
        fn define-ref-forward (failedf methodname)
            set-type-symbol! ref methodname
                inline (self args...)
                    let ET = (typeof& self)
                    let op success = (type@& ET methodname)
                    if success
                        return (op self args...)
                    let op ok = (type@& ET '__deref)
                    if (not ok)
                        compiler-error!
                            .. (op-prettyname methodname)
                                \ " of reference not supported by value of type " (repr ET)
                        return;
                    failedf (deref self) args...

        fn define-ref-forward-failable (failedf methodname)
            set-type-symbol! ref methodname
                inline (self args...)
                    let ET = (typeof& self)
                    let op success = (type@& ET methodname)
                    if success
                        let result... = (op self args...)
                        if (not (va-empty? result...))
                            return result...
                    let op ok = (type@& ET '__deref)
                    if (not ok)
                        return;
                    failedf (deref self) args...

        define-ref-forward (do -) '__neg
        define-ref-forward (do /) '__rcp
        define-ref-forward countof '__countof
        define-ref-forward unpack '__unpack
        define-ref-forward forward-hash '__hash
        define-ref-forward (do @) '__@
        define-ref-forward-failable forward-as '__as
        define-ref-forward-failable forward-getattr '__getattr

        set-type-symbol! ref '__repr
            fn (self)
                let ET = (typeof& self)
                let op ok = (type@& ET '__repr)
                if ok
                    return (op self)
                let op ok = (type@& ET '__deref)
                if ok
                    forward-repr (deref self)
                else
                    string-repr
                        bitcast self (storageof (typeof self))

        set-type-symbol! ref '__deref
            inline "ref-deref" (self)
                let ET = (typeof& self)
                let op ok = (type@& ET '__deref)
                if ok
                    op self
                else
                    compiler-error!
                        .. "cannot dereference value of type " (repr ET)

        set-type-symbol! ref '__typeattr
            inline "ref-typeattr" (cls name)
                let T = (storageof cls)
                let ET = (element-type T 0)
                let value success = (type@& ET name)
                if success
                    return value
                let op success = (type@& ET '__typeattr)
                if success
                    let result... = (op ET name)
                    if (va-empty? result...)
                    else
                        return result...

        set-type-symbol! ref '__call
            inline "ref-call" (self args...)
                let ET = (typeof& self)
                let op success = (type@& ET '__call)
                if success
                    return (op self args...)
                else
                    call (deref self) args...

        set-type-symbol! ref '__imply
            inline "ref-imply" (self destT)
                let ptrtype = (storageof (typeof self))
                if (type== destT ptrtype)
                    return (bitcast self ptrtype)
                if (pointer-type-imply? ptrtype destT)
                    return (bitcast self destT)
                let ET = (element-type ptrtype 0)
                if (array-type? ET)
                    let ET = (element-type ET 0)
                    let aptrtype = (pointer ET)
                    if (type== destT aptrtype)
                        return (bitcast self aptrtype)
                # try to ask target type for implicit refcast
                let op ok = (type@& ET '__imply)
                if ok
                    let result... = (op self destT)
                    if (not (va-empty? result...))
                        return result...
                # dereference if target type can implicitly cast
                let op ok = (type@& ET '__deref)
                if ok
                    return
                        forward-imply (deref self) destT

    #var has been removed; use `local`

    #global has been removed; use `static`

    # (typefn[!][&] type 'symbol (params) body ...)
    define-macro typefn
        let ty name params body = (decons args 3)
        list set-type-symbol! ty name
            cons fn params body
    define-macro typefn&
        let ty name params body = (decons args 3)
        list set-type-symbol!& ty name
            cons fn params body
    define-macro typeinline
        let ty name params body = (decons args 3)
        list set-type-symbol! ty name
            cons inline params body
    define-macro typeinline&
        let ty name params body = (decons args 3)
        list set-type-symbol!& ty name
            cons inline params body

    inline bitcast& (self destT)
        let T = (typeof self)
        assert (T < ref) "argument must be of reference type"
        let ST = (storageof T)
        # todo: ensure types are compatible
        (bitcast self ('set-element-type ST destT)) as ref

let deref = (deref as ASTMacro)

#-------------------------------------------------------------------------------
# default memory allocators
#-------------------------------------------------------------------------------

spice pointer-each (n op value args...)
    if ('constant? n)
        let n = (n as usize)
        if (n == 1:usize)
            return `(op (value as ref) args...)
    ast-quote
        loop (i) = 0
        if (i < n)
            op
                (getelementptr value i) as ref
                args...
            repeat (i + 1)

#inline pointer-each (n op value args...)
    let n = (imply n usize)
    if (n == 1:usize)
        op (value as ref) args...
    else
        # unroll only if <= 4 elements
        let loop (i) =
            if ((constant? n) and (n <= 4:usize)) 0:usize
            else (unconst 0:usize)
        if (i < n)
            op
                (getelementptr value i) as ref
                args...
            loop (i + 1:usize)
    return;

#
    inline pointer-each2 (n op value other)
        let n = (imply n usize)
        if (n == 1:usize)
            op (value as ref) (other as ref)
        else
            # unroll only if <= 4 elements
            let loop (i) =
                if ((constant? n) and (n <= 4:usize)) 0:usize
                else (unconst 0:usize)
            if (i < n)
                op
                    (getelementptr value i) as ref
                    (getelementptr other i) as ref
                loop (i + 1:usize)
        return;

    inline construct (value args...)
        """"Invokes the constructor for `value` of reference-like type,
            passing along optional argument set `args...`.
        let ET = (typeof& value)
        let op ok = (type@& ET '__new)
        if (not ok)
            compiler-error!
                .. "type " (repr ET) " has no constructor"
        pointer-each 1 op value args...

    inline construct-array (n value args...)
        """"Invokes the constructor for an array `value` of reference-like type,
            assuming that value is a pointer to an array element, passing along
            optional argument set `args...`.
        let ET = (typeof& value)
        let op ok = (type@& ET '__new)
        if (not ok)
            compiler-error!
                .. "type " (repr ET) " has no constructor"
        pointer-each n op value args...

    inline destruct (value)
        """"Invokes the destructor for `value` of reference-like type.
        let ET = (typeof& value)
        let op ok = (type@& ET '__delete)
        if (not ok)
            compiler-error!
                .. "type " (repr ET) " has no destructor"
        pointer-each 1 op value

    inline destruct-array (n value)
        """"Invokes the destructor for an array `value` of reference-like type,
            assuming that value is a pointer to an array element.
        let ET = (typeof& value)
        let op ok = (type@& ET '__delete)
        if (not ok)
            compiler-error!
                .. "type " (repr ET) " has no destructor"
        pointer-each n op value

    inline copy-construct (value source)
        """"Invokes the copy constructor for `value` of reference-like type if
            present, passing `source` as a value from which to copy.

            `source` does not have to be of reference type, but can also be of
            immutable element type.
        let ET = (typeof& value)
        let op ok = (type@& ET '__copy)
        if (not ok)
            compiler-error!
                .. "type " (repr ET) " has no copy constructor"
        pointer-each 1 op value source

    inline copy-construct-array (n value source)
        """"Invokes the copy constructor for an array `value` of reference-like type,
            passing `source` as a value from which to copy.

            `source` has to be the first (referenced) element of an array too.
        let ET = (typeof& value)
        let op ok = (type@& ET '__copy)
        if (not ok)
            compiler-error!
                .. "type " (repr ET) " has no copy constructor"
        assert ((typeof source) < ref)
        pointer-each2 n op value source

    inline move-construct (value source)
        """"Invokes the move constructor for `value` of reference-like type,
            passing `source` as the reference from which to move.
        let ET = (typeof& value)
        let op ok = (type@& ET '__move)
        if ok
            pointer-each2 1 op value source
        else
            # try copy, then destruct source
            copy-construct value source
            destruct source

    inline move-construct-array (n value source)
        """"Invokes the move constructor for an array of pointers `value`
            passing `source` as an array of pointers from which to move.
        let ET = (typeof& value)
        let op ok = (type@& ET '__move)
        if ok
            pointer-each2 n op value source
        else
            copy-construct-array n value source
            destruct-array n source

#
    let Memory = (typename "Memory")
    typeinline Memory '__typecall (cls T args...)
        if (((typeof T) == Symbol) and (T == 'copy))
            if ((va-countof args...) > 1)
                compiler-error! "copy constructor only takes one argument"
            (type@ cls 'copy) cls args...
        else cls
            (type@ cls 'new) cls T args...

    typeinline Memory 'delete (cls value)
        destruct value
        (type@ cls 'free) cls value

    typeinline Memory 'copy (cls value)
        let T = (typeof value)
        let ET =
            if (T < ref)
                element-type (storageof T) 0
            else T
        let self =
            ((type@ cls 'allocate) cls ET) as ref
        copy-construct self value
        self

    typeinline Memory 'new (cls T args...)
        let self =
            ((type@ cls 'allocate) cls T) as ref
        construct self args...
        self

    let HeapMemory = (typename "HeapMemory" (super = Memory))
    typeinline HeapMemory 'allocate (cls T)
        malloc T
    typeinline HeapMemory 'free (cls value)
        free value
        return;
    typeinline HeapMemory 'allocate-array (cls T count)
        malloc-array T count
    typeinline HeapMemory 'free-array (cls value count)
        free value
        return;

    let FunctionMemory = (typename "FunctionMemory" (super = Memory))
    typeinline FunctionMemory 'allocate (cls T)
        alloca T
    typeinline FunctionMemory 'free (cls value)
        return;
    typeinline FunctionMemory 'allocate-array (cls T count)
        alloca-array T count
    typeinline FunctionMemory 'free-array (cls value count)
        return;

    let GlobalMemory = (typename "GlobalMemory" (super = Memory))
    typeinline GlobalMemory 'allocate (cls T)
        static-alloc T
    typeinline GlobalMemory 'free (cls value)
        return;
    typeinline GlobalMemory 'allocate-array (cls T count)
        assert (constant? count) "count must be constant"
        bitcast
            static-alloc
                array T count
            'set-storage (pointer T 'mutable) 'Private
    typeinline GlobalMemory 'free-array (cls value count)
        return;

    typefn ref '__delete (self)
        let T = (typeof self)
        let ptrT = (storageof T)
        let class = ('storage ptrT)
        if (class == unnamed)
            'delete HeapMemory self
        elseif (class == 'Function)
            'delete FunctionMemory self
        elseif (class == 'Private)
            'delete GlobalMemory self
        else
            .. "cannot delete reference of type " (repr T)

#-------------------------------------------------------------------------------

define-syntax-block-scope-macro static-if
    fn process (body next-expr)
        if false
            return '() next-expr
        let cond body = (decons body)
        let elseexpr next-next-expr =
            if (empty? next-expr)
                _ '() next-expr
            else
                let else-expr next-next-expr = (decons next-expr)
                if (('typeof else-expr) == list)
                    let kw body = (decons (else-expr as list))
                    let kw = (kw as Symbol)
                    switch kw
                    case 'elseif
                        process body next-next-expr
                    case 'else
                        _ body next-next-expr
                    default
                        _ '() next-expr
                else
                    _ '() next-expr
        return
            list
                list constbranch cond
                    cons inline '() body
                    cons inline '() elseexpr
            next-next-expr
    let kw body = (decons expr)
    let body next-expr = (process body next-expr)
    return
        cons
            cons do body
            next-expr
        syntax-scope

define-syntax-block-scope-macro syntax-if
    fn process (syntax-scope body next-expr)
        if false
            return '() next-expr
        let cond body = (decons body)
        let elseexpr next-next-expr =
            if (empty? next-expr)
                _ '() next-expr
            else
                let else-expr next-next-expr = (decons next-expr)
                if (('typeof else-expr) == list)
                    let kw body = (decons (else-expr as list))
                    let kw = (kw as Symbol)
                    switch kw
                    case 'elseif
                        process syntax-scope body next-next-expr
                    case 'else
                        _ body next-next-expr
                    default
                        _ '() next-expr
                else
                    _ '() next-expr
        let cond = (sc_expand cond syntax-scope)
        if ((('typeof cond) == bool) and ('constant? cond))
            if (cond as bool)
                return body next-next-expr
            else
                return elseexpr next-next-expr
        syntax-error! ('anchor cond) "condition must be constant"
    let kw body = (decons expr)
    let body next-expr = (process syntax-scope body next-expr)
    return
        cons
            cons do body
            next-expr
        syntax-scope

define-syntax-scope-macro syntax-eval
    let subscope = (Scope syntax-scope)
    return
        exec-module (Value args) subscope
        syntax-scope

let
    io-write! = sc_write
    compiler-version = sc_compiler_version
    default-styler = sc_default_styler
    realpath = sc_realpath
    globals = sc_get_globals
    set-globals! = sc_set_globals
    __prompt = sc_prompt
    set-autocomplete-scope! = sc_set_autocomplete_scope
    exit = sc_exit
    launch-args = sc_launch_args
    set-signal-abort! = sc_set_signal_abort
    list-load = sc_parse_from_path
    list-parse = sc_parse_from_string
    set-anchor! = sc_set_active_anchor
    active-anchor = sc_get_active_anchor
    #eval = sc_eval
    format-error = sc_format_error
    import-c = sc_import_c

compile-stage;

set-globals! (__this-scope)

#-------------------------------------------------------------------------------
# REPL
#-------------------------------------------------------------------------------

fn compiler-version-string ()
    let vmin vmaj vpatch = (sc_compiler_version)
    .. "Scopes " (tostring vmin) "." (tostring vmaj)
        if (vpatch == 0) ""
        else
            .. "." (tostring vpatch)
        " ("
        if debug-build? "debug build, "
        else ""
        \ compiler-timestamp ")"

fn print-logo ()
    io-write! "  "; io-write! (default-styler style-string "\\\\\\"); io-write! "\n"
    io-write! "   "; io-write! (default-styler style-number "\\\\\\"); io-write! "\n"
    io-write! " "; io-write! (default-styler style-comment "///")
    io-write! (default-styler style-sfxfunction "\\\\\\"); io-write! "\n"
    io-write! (default-styler style-comment "///"); io-write! "  "
    io-write! (default-styler style-function "\\\\\\")

fn read-eval-print-loop ()
    fn repeat-string (n c)
        loop (i s) = 0:usize ""
        if (i == n)
            return s
        repeat (i + 1:usize)
            .. s c

    fn leading-spaces (s)
        let len = (countof s)
        loop (i) = 0:usize
        if (i == len)
            return s
        let c = (@ s i)
        if (c != (char " "))
            let s = (sc_string_buffer s)
            return (sc_string_new s i)
        repeat (i + 1:usize)

    fn blank? (s)
        let len = (countof s)
        loop (i) = 0:usize
        if (i == len)
            return true
        if ((@ s i) != (char " "))
            return false
        repeat (i + 1:usize)

    let cwd =
        realpath "."

    print-logo;
    print " "
        compiler-version-string;

    let global-scope = (globals)
    let eval-scope = (Scope global-scope)
    set-autocomplete-scope! eval-scope

    'set-symbol eval-scope 'module-dir cwd
    loop (preload cmdlist counter eval-scope) = "" "" 0 eval-scope
    fn make-idstr (counter)
        .. "$" (tostring counter)

    let idstr = (make-idstr counter)
    let promptstr =
        .. idstr " "
            default-styler style-comment "►"
    let promptlen = ((countof idstr) + 2:usize)
    let success cmd =
        __prompt
            ..
                if (empty? cmdlist) promptstr
                else
                    repeat-string promptlen "."
                " "
            preload
    if (not success)
        return;
    fn endswith-blank (s)
        let slen = (countof s)
        if (slen == 0:usize) false
        else
            (@ s (slen - 1:usize)) == (char " ")
    let enter-multiline = (endswith-blank cmd)
    let terminated? =
        (blank? cmd) or
            (empty? cmdlist) and (not enter-multiline)
    let cmdlist =
        .. cmdlist
            if enter-multiline
                lslice cmd ((countof cmd) - 1:usize)
            else cmd
            "\n"
    let preload =
        if terminated? ""
        else (leading-spaces cmd)
    if (not terminated?)
        repeat preload cmdlist counter eval-scope

    fn handle-retargs (counter eval-scope local-scope vals...)
        raises-compile-error;
        let tmp = (Symbol "#result...")
        # copy over values from local-scope
        do
            loop (key) = unnamed
            let key value = ('next local-scope key)
            if (key != unnamed)
                if (key != tmp)
                    'set-symbol eval-scope key value
                repeat key
        let count =
            va-lfold 0
                inline (key value k)
                    let idstr = (make-idstr (counter + k))
                    'set-symbol eval-scope (Symbol idstr) (Value value)
                    print idstr "="
                        repr value
                    k + 1
                vals...
        return eval-scope count

    let eval-scope count =
        try
            let expr = (list-parse cmdlist)
            let expr-anchor = ('anchor expr)
            set-anchor! expr-anchor
            let tmp = (Symbol "#result...")
            let expr =
                Value
                    list
                        list syntax-set-scope! eval-scope
                        list let tmp '=
                            cons inline-do
                                expr as list
                        #list __defer (list tmp)
                            list _ (list get-scope) (list locals) tmp
                        list handle-retargs counter
                            list __this-scope
                            list locals
                            tmp
            let f = (sc_compile (sc_eval expr-anchor (unbox-pointer expr list) eval-scope) 0:u64)
            let fptr =
                f as
                    'pointer
                        'raising
                            function (Arguments Scope i32)
                            Error
            set-anchor! expr-anchor
            fptr;
        except (exc)
            io-write!
                format-error exc
            io-write! "\n"
            _ eval-scope 0
    repeat "" "" (counter + count) eval-scope

#-------------------------------------------------------------------------------
# main
#-------------------------------------------------------------------------------

fn print-help (exename)
    print "usage:" exename
        """"[option [...]] [filename]

            Options:
            -h, --help                  print this text and exit.
            -v, --version               print program version and exit.
            -s, --signal-abort          raise SIGABRT when calling `abort!`.
            --                          terminate option list.
    exit 0

fn print-version ()
    print
        compiler-version-string;
    print "Executable path:" compiler-path
    exit 0

fn run-main ()
    let argc argv = (launch-args)
    let exename = (load (getelementptr argv 0))
    let exename = (sc_string_new_from_cstr exename)
    let sourcepath = (alloca string)
    let parse-options = (alloca bool)
    store "" sourcepath
    store true parse-options
    do
        loop (i) = 1
        if (i < argc)
            let k = (i + 1)
            let arg = (load (getelementptr argv i))
            let arg = (sc_string_new_from_cstr arg)
            if ((load parse-options) and ((@ arg 0:usize) == (char "-")))
                if ((arg == "--help") or (arg == "-h"))
                    print-help exename
                elseif ((== arg "--version") or (== arg "-v"))
                    print-version;
                elseif ((== arg "--signal-abort") or (== arg "-s"))
                    set-signal-abort! true
                elseif (== arg "--")
                    store false parse-options
                else
                    print
                        .. "unrecognized option: " arg
                            \ ". Try --help for help."
                    exit 1
                repeat k
            elseif ((load sourcepath) == "")
                store arg sourcepath
            # remainder is passed on to script
    let sourcepath = (load sourcepath)
    if (sourcepath == "")
        read-eval-print-loop;
    else
        let scope =
            Scope (globals)
        'set-symbol scope
            script-launch-args =
                fn ()
                    return sourcepath argc argv
        do  #try
            load-module "" sourcepath
                scope = scope
                main-module? = true
            _;
        #except (err)
            print
                default-styler style-error "error:"
                format-error err
        exit 0

raises-compile-error;
run-main;

return;