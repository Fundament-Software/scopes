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

# square list expressions are ast unquotes by default
let square-list = spice-unquote-arguments

# first we alias u64 to the integer type that can hold a pointer
let intptr = u64

# pointer comparison as a template function, because we'll compare pointers of many types
fn ptrcmp!= (t1 t2)
    icmp!= (ptrtoint t1 intptr) (ptrtoint t2 intptr)

fn ptrcmp== (t1 t2)
    icmp== (ptrtoint t1 intptr) (ptrtoint t2 intptr)

fn box-integer (value)
    let T = (typeof value)
    sc_const_int_new (sc_get_active_anchor) T
        if (sc_integer_type_is_signed T)
            sext value u64
        else
            zext value u64

# turn a symbol-like value (storage type u64) to an Any
fn box-symbol (value)
    sc_const_int_new (sc_get_active_anchor) (typeof value)
        bitcast value u64

# turn a pointer value into an Any
fn box-pointer (value)
    sc_const_pointer_new (sc_get_active_anchor) (typeof value)
        bitcast value voidstar

fn error! (msg)
    raise (sc_runtime_error_new msg)

fn sugar-error! (anchor msg)
    raise (sc_location_error_new anchor msg)

fn compiler-error! (value)
    raise (sc_location_error_new (sc_get_active_anchor) value)

# print an unboxing error given two types
fn unbox-verify (value wantT)
    let haveT = (sc_value_type value)
    if (ptrcmp!= haveT wantT)
        sugar-error! (sc_value_anchor value)
            sc_string_join "can't unbox value of type "
                sc_string_join
                    sc_value_repr (box-pointer haveT)
                    sc_string_join " as value of type "
                        sc_value_repr (box-pointer wantT)
    if (sc_value_is_constant value)
    else
        sugar-error! (sc_value_anchor value)
            sc_string_join "constant of type "
                sc_string_join
                    sc_value_repr (box-pointer haveT)
                    sc_string_join " expected, got expression of type "
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
let SpiceMacroFunction = (sc_type_storage SpiceMacro)
# dynamically construct a new symbol
let ellipsis-symbol = (sc_symbol_new "...")

# execute until here and treat the remainder as a new translation unit
run-stage;

# we can now access TypeArrayPointer as a compile time value
let void =
    sc_arguments_type 0 (nullof TypeArrayPointer)

fn build-typify-function (f)
    let types = (alloca-array type 1:usize)
    store Value (getelementptr types 0)
    let types = (bitcast types TypeArrayPointer)
    let result = (sc_compile (sc_typify f 1 types) 0:u64)
    let result-type = (sc_value_type result)
    if (ptrcmp!= result-type SpiceMacroFunction)
        compiler-error!
            sc_string_join "AST macro must have type "
                sc_string_join
                    sc_value_repr (box-pointer SpiceMacroFunction)
                    sc_string_join " but has type "
                        sc_value_repr (box-pointer result-type)
    let ptr = (sc_const_pointer_extract result)
    bitcast ptr SpiceMacro

let typify =
    do
        fn typify (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 -1
            let src_fn = (sc_getarg args 0)
            let typifyfn =
                if (ptrcmp== (sc_value_type src_fn) Closure)
                    `sc_typify
                else
                    `sc_typify_template
            let typecount = (sub argcount 1)
            spice-quote
                let types = (alloca-array type typecount)
                spice-unquote
                    let body = (sc_expression_new (sc_get_active_anchor))
                    loop (i j = 1 0)
                        if (icmp== i argcount)
                            break;
                        let ty = (sc_getarg args i)
                        if (ptrcmp!= (sc_value_type ty) type)
                            compiler-error! "type expected"
                        sc_expression_append body
                            `(store ty (getelementptr types j))
                        _ (add i 1) (add j 1)
                    body
                typifyfn src_fn typecount (bitcast types TypeArrayPointer)

        build-typify-function typify

let const-typify =
    do
        fn const-typify (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 -1
            let src_fn = (sc_getarg args 0)
            let src_fn = (unbox-pointer src_fn Closure)
            let typecount = (sub argcount 1)
            let types = (alloca-array type typecount)
            loop (i j = 1 0)
                if (icmp== i argcount)
                    break;
                let ty = (sc_getarg args i)
                store (unbox-pointer ty type) (getelementptr types j)
                _ (add i 1) (add j 1)
            sc_typify src_fn typecount (bitcast types TypeArrayPointer)

        build-typify-function const-typify

run-stage;

let spice-macro-verify-signature =
    const-typify (fn "spice-macro-verify-signature" (f)) SpiceMacroFunction

inline function->SpiceMacro (f)
    spice-macro-verify-signature f
    bitcast f SpiceMacro

fn box-empty ()
    sc_argument_list_new (sc_get_active_anchor)

fn box-none ()
    sc_const_aggregate_new (sc_get_active_anchor) Nothing 0 (undef ValueArrayPointer)

# take closure l, typify and compile it and return a function of SpiceMacro type
inline spice-macro (l)
    function->SpiceMacro (const-typify l Value)

inline box-spice-macro (l)
    box-pointer (spice-macro l)

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
            loop (i ret = 2 init)
                if (icmp== i argcount)
                    break ret
                let arg =
                    sc_getarg args i
                let k = (sc_type_key (sc_value_qualified_type arg))
                let v = (sc_keyed_new (sc_value_anchor arg) unnamed arg)
                _ (add i 1)
                    if use-indices
                        `(f [(sub i 2)] k v ret)
                    else
                        `(f k v ret)
        _
            spice-macro (fn "va-lfold" (args) (va-lfold args false))
            spice-macro (fn "va-ilfold" (args) (va-lfold args true))

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
            loop (i ret = argcount init)
                if (icmp<=s i 2)
                    break ret
                let oi = i
                let i = (sub i 1)
                let arg =
                    sc_getarg args i
                let k = (sc_type_key (sc_value_qualified_type arg))
                let v = (sc_keyed_new (sc_value_anchor arg) unnamed arg)
                _ i
                    if use-indices
                        `(f [(sub i 2)] k v ret)
                    else
                        `(f k v ret)
        _
            spice-macro (fn "va-rfold" (args) (va-rfold args false))
            spice-macro (fn "va-rifold" (args) (va-rfold args true))

inline raises-compile-error ()
    if false
        compiler-error! "hidden"

inline type< (T superT)
    sc_type_is_superof superT T

let type> = sc_type_is_superof

fn type<= (T superT)
    bxor (type> T superT) true

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
    fn (args) (compare-type args (const-typify f type type))

let storagecast =
    box-spice-macro
        fn "storagecast" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 1
            let self = (sc_getarg args 0)
            let T = (sc_type_storage (sc_value_type self))
            return `(bitcast self T)

# typecall
sc_type_set_symbol type '__call
    box-spice-macro
        fn "type-call" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 -1
            let self = (sc_getarg args 0)
            let T = (unbox-pointer self type)
            let f = (sc_type_at T '__typecall)
            return `(f args)

# method call syntax
sc_type_set_symbol Symbol '__call
    box-spice-macro
        fn "symbol-call" (args)
            fn resolve-method (self symval)
                let sym = (unbox-symbol symval Symbol)
                let T = (sc_value_type self)
                try
                    return (sc_type_at T sym)
                except (err)
                # if calling method of type, try typemethod
                if (ptrcmp== T type)
                    if (sc_value_is_constant self)
                        let self = (unbox-pointer self type)
                        try
                            return (sc_type_at self sym)
                        except (err)
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
                [(sc_extract_argument_list_new (sc_value_anchor args) args 1)])
            expr

do
    fn get-key-value-args (args)
        let argcount = (sc_argcount args)
        verify-count argcount 2 3
        let self = (sc_getarg args 0)
        if (icmp== argcount 3)
            let key = (sc_getarg args 1)
            let value = (sc_getarg args 2)
            return self key value
        else
            let arg = (sc_getarg args 1)
            let key = (sc_type_key (sc_value_qualified_type arg))
            let arg = (sc_keyed_new (sc_value_anchor arg) unnamed arg)
            return self (box-symbol key) arg

    inline gen-key-any-set (selftype fset)
        box-spice-macro
            fn "set-symbol" (args)
                let self key value = (get-key-value-args args)
                `(fset self key value)

    inline gen-key-any-define (selftype fset)
        box-spice-macro
            fn "define-symbol" (args)
                let self key value = (get-key-value-args args)
                if (sc_value_is_constant self)
                    if (sc_value_is_constant key)
                        if (sc_value_is_pure value)
                            let self = (unbox-pointer self selftype)
                            let key = (unbox-symbol key Symbol)
                            fset self key value
                            return `[]
                compiler-error! "all arguments must be constant"

    # quick assignment of type attributes
    sc_type_set_symbol type 'set-symbol (gen-key-any-set type sc_type_set_symbol)
    sc_type_set_symbol Scope 'set-symbol (gen-key-any-set Scope sc_scope_set_symbol)
    sc_type_set_symbol type 'define-symbol (gen-key-any-define type sc_type_set_symbol)
    sc_type_set_symbol Scope 'define-symbol (gen-key-any-define Scope sc_scope_set_symbol)

sc_type_set_symbol type 'pointer
    box-spice-macro
        fn "type-pointer" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 1
            let self = (sc_getarg args 0)
            if (sc_value_is_constant self)
                let T = (unbox-pointer self type)
                box-pointer
                    sc_pointer_type T pointer-flag-non-writable unnamed
            else
                `(sc_pointer_type self pointer-flag-non-writable unnamed)

# tuple type constructor
sc_type_set_symbol tuple '__typecall
    box-spice-macro
        fn "tuple" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 -1
            let pcount = (sub argcount 1)
            spice-quote
                let types = (alloca-array type pcount)
                spice-unquote
                    let body = (sc_expression_new (sc_get_active_anchor))
                    loop (i = 1)
                        if (icmp== i argcount)
                            break;
                        let arg = (sc_getarg args i)
                        let k = (sc_type_key (sc_value_qualified_type arg))
                        let arg = (sc_keyed_new (sc_value_anchor arg) unnamed arg)
                        if (ptrcmp!= (sc_value_type arg) type)
                            compiler-error! "type expected"
                        sc_expression_append body
                            `(store
                                (sc_key_type k arg)
                                (getelementptr types [(sub i 1)]))
                        add i 1
                    body
                sc_tuple_type pcount types

# arguments type constructor
sc_type_set_symbol Arguments '__typecall
    box-spice-macro
        fn "Arguments" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 -1
            let pcount = (sub argcount 1)
            let types = (alloca-array type pcount)
            loop (i = 1)
                if (icmp== i argcount)
                    break;
                let arg = (sc_getarg args i)
                let T = (unbox-pointer arg type)
                store T (getelementptr types (sub i 1))
                add i 1
            box-pointer (sc_arguments_type pcount types)

# function pointer type constructor
sc_type_set_symbol function '__typecall
    box-spice-macro
        fn "function" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 2 -1
            let pcount = (sub argcount 2)
            let constant? =
                loop (i = 1)
                    if (icmp== i argcount)
                        break true
                    let arg = (sc_getarg args i)
                    if (sc_value_is_constant arg)
                        repeat (add i 1)
                    break false
            let rtype = (sc_getarg args 1)
            if constant?
                let rtype = (unbox-pointer rtype type)
                let types = (alloca-array type pcount)
                loop (i = 2)
                    if (icmp== i argcount)
                        break;
                    let arg = (sc_getarg args i)
                    let T = (unbox-pointer arg type)
                    store T (getelementptr types (sub i 2))
                    add i 1
                box-pointer (sc_function_type rtype pcount types)
            else
                spice-quote
                    let types = (alloca-array type pcount)
                    spice-unquote
                        let expr = (sc_expression_new (sc_get_active_anchor))
                        loop (i = 2)
                            if (icmp== i argcount)
                                break;
                            let arg = (sc_getarg args i)
                            sc_expression_append expr
                                `(store arg (getelementptr types [(sub i 2)]))
                            add i 1
                        expr
                    sc_function_type rtype pcount types

sc_type_set_symbol type 'raising
    box-spice-macro
        fn "function-raising" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 2 2
            let self = (sc_getarg args 0)
            let except_type = (sc_getarg args 1)
            if (sc_value_is_constant self)
                if (sc_value_is_constant except_type)
                    let T = (unbox-pointer self type)
                    let exceptT = (unbox-pointer except_type type)
                    return
                        box-pointer
                            sc_function_type_raising T exceptT
            `(sc_function_type_raising self except_type)

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
    spice-macro
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
    spice-macro
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
    spice-macro
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
    spice-macro
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
    box-spice-macro
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
                    box-pointer
                        box-none;
                else
                    sc_value_wrap T value

let __unbox =
    spice-macro
        fn (args)
            let argc = (sc_argcount args)
            verify-count argc 2 2
            let value = (sc_getarg args 0)
            let T = (sc_getarg args 1)
            let T = (unbox-pointer T type)
            sc_value_unwrap T value
let
    type== = (spice-macro (type-comparison-func ptrcmp==))
    type!= = (spice-macro (type-comparison-func ptrcmp!=))
    type<= = (spice-macro (type-comparison-func type<=))
    type>= = (spice-macro (type-comparison-func type>=))

run-stage;

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

inline define-symbols (self values...)
    va-lfold none
        inline (key value)
            'define-symbol self key value
        values...

'define-symbol type 'set-symbols set-symbols
'define-symbol Scope 'set-symbols set-symbols
'define-symbol type 'define-symbols define-symbols
'define-symbol Scope 'define-symbols define-symbols

'define-symbols Value
    constant? = sc_value_is_constant
    pure? = sc_value_is_pure
    kind = sc_value_kind
    none? = (const-typify Value-none? Value)
    __repr = sc_value_repr
    spice-repr = sc_value_ast_repr
    dump =
        inline (self)
            sc_write (sc_value_ast_repr self)
    typeof = sc_value_type
    qualified-typeof = sc_value_qualified_type
    anchor = sc_value_anchor
    argcount = sc_argcount
    getarg = sc_getarg
    getarglist = sc_getarglist
    dekey =
        fn "dekey" (self)
            let k = (sc_type_key ('qualified-typeof self))
            _ k (sc_keyed_new (sc_value_anchor self) unnamed self)

'define-symbols Scope
    @ = sc_scope_at
    next = sc_scope_next
    docstring = sc_scope_get_docstring
    set-docstring! = sc_scope_set_docstring
    parent = sc_scope_get_parent

'define-symbols string
    join = sc_string_join
    match? = sc_string_match

'define-symbols Error
    format = sc_format_error

'define-symbols list
    __countof = sc_list_count
    join = sc_list_join
    @ = sc_list_at
    next = sc_list_next
    decons = decons
    reverse = sc_list_reverse
    dump = sc_list_dump

'define-symbols SampledImage
    __typecall =
        inline (cls ...)
            sc_sampled_image_type ...

'define-symbols Image
    __typecall =
        inline (cls ...)
            sc_image_type ...

'define-symbols type
    bitcount = sc_type_bitcountof
    signed? = sc_integer_type_is_signed
    element@ = sc_type_element_at
    element-count = sc_type_countof
    storageof = sc_type_storage
    kind = sc_type_kind
    sizeof = sc_type_sizeof
    alignof = sc_type_alignof
    @ = sc_type_at
    local@ = sc_type_local_at
    opaque? = sc_type_is_opaque
    string = sc_type_string
    superof = sc_typename_type_get_super
    set-super = sc_typename_type_set_super
    set-storage =
        inline (type storage-type)
            sc_typename_type_set_storage type storage-type 0:u32
    set-plain-storage =
        inline (type storage-type)
            sc_typename_type_set_storage type storage-type typename-flag-plain
    return-type = sc_function_type_return_type
    key = sc_type_key
    key-type =
        inline (self key)
            sc_key_type key self
    unique-type = sc_unique_type
    view-type =
        inline (self id)
            constbranch (none? id)
                inline ()
                    sc_view_type self -1
                inline ()
                    sc_view_type self id
    refer? = sc_type_is_refer
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
    change-element-type =
        fn (cls ET)
            sc_pointer_type_set_element_type cls ET
    change-storage-class =
        fn (cls storage-class)
            sc_pointer_type_set_storage_class cls storage-class
    immutable =
        fn (cls)
            sc_pointer_type_set_flags cls
                bor (sc_pointer_type_get_flags cls) pointer-flag-non-writable
    mutable =
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
    readable? =
        fn (cls)
            icmp== (band (sc_pointer_type_get_flags cls) pointer-flag-non-readable) 0:u64
    writable? =
        fn (cls)
            icmp== (band (sc_pointer_type_get_flags cls) pointer-flag-non-writable) 0:u64
    pointer->refer-type =
        fn "pointer->refer-type" (cls)
            sc_refer_type
                sc_type_element_at cls 0
                sc_pointer_type_get_flags cls
                sc_pointer_type_get_storage_class cls

#'define-symbols Closure
    frame = sc_closure_frame
    label = sc_closure_label

let rawstring = ('pointer i8)

inline box-cast (f)
    box-pointer (const-typify f type type Value)

inline not (value)
    bxor value true

# supertype for unique structs
let Struct = (sc_typename_type "Struct")

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
'set-plain-storage Generator ('storageof Closure)

# syntax macro type
let SugarMacro = (sc_typename_type "SugarMacro")
let SugarMacroFunction =
    'pointer
        'raising
            function (Arguments list Scope) list list Scope
            Error
'set-plain-storage SugarMacro SugarMacroFunction

# any extraction

inline unbox (value T)
    unbox-verify value T
    __unbox value T

fn value-as (vT T expr)
    if true
        return `(unbox expr T)
    compiler-error! "unsupported type"

'set-symbols Value
    __as =
        box-cast value-as
    __rimply =
        box-cast
            fn "Value-rimply" (vT T expr)
                if true
                    return `(Value expr)
                compiler-error! "unsupported type"

'set-symbols SpiceMacro
    __rimply =
        box-cast
            fn "SpiceMacro-rimply" (vT T expr)
                if (ptrcmp== vT SpiceMacroFunction)
                    return `(bitcast expr T)
                elseif (ptrcmp== vT Closure)
                    if ('constant? expr)
                        return `(spice-macro expr)
                compiler-error! "unsupported type"

# integer casting

fn integer-imply (vT T expr)
    let ST =
        if (ptrcmp== T usize) ('storageof T)
        else T
    let args... = expr T
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
    elseif (icmp== ('kind ST) type-kind-real)
        # constant i32 auto-converts to real
        if ('constant? expr)
            if (ptrcmp== vT i32)
                if ('signed? vT)
                    return `(sitofp args...)
                else
                    return `(uitofp args...)
    compiler-error! "unsupported type"

fn integer-as (vT T expr)
    let args... = expr T
    let T =
        if (ptrcmp== T usize) ('storageof T)
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
        if (ptrcmp== T usize) ('storageof T)
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
    box-pointer (const-typify f type type Value Value)

inline single-binary-op-dispatch (destf)
    fn (lhsT rhsT lhs rhs)
        if (ptrcmp== lhsT rhsT)
            return `(destf lhs rhs)
        compiler-error! "unsupported type"

inline cast-error! (intro-string vT T err)
    compiler-error!
        sc_string_join intro-string
            sc_string_join ('__repr (box-pointer vT))
                sc_string_join " to type "
                    sc_string_join ('__repr (box-pointer T))
                        sc_string_join ": "
                            sc_format_error err

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
    try
        let anyf = ('@ vT symbol)
        let f =
            try (unbox-cast-function-type anyf)
            except (err)
                attribute-format-error! vT symbol err
        return (f vT T expr)
    except (lhs-err)
        let anyf =
            try
                '@ T rsymbol
            except (err)
                raise lhs-err
        let f =
            try (unbox-cast-function-type anyf)
            except (err)
                attribute-format-error! T rsymbol err
        return (f vT T expr)

fn imply-expr (vT T expr)
    if (ptrcmp== vT T)
        return expr
    if (sc_type_is_superof T vT)
        return expr
    cast-expr '__imply '__rimply vT T expr

fn as-expr (vT T expr)
    if (ptrcmp== vT T)
        return expr
    if (sc_type_is_superof T vT)
        return expr
    try
        # try implicit cast first
        cast-expr '__imply '__rimply vT T expr
    except (imply-err)
        cast-expr '__as '__ras vT T expr

let
    imply =
        spice-macro
            fn "imply-dispatch" (args)
                let argc = ('argcount args)
                verify-count argc 2 2
                let value = ('getarg args 0)
                let anyT = ('getarg args 1)
                let vT = ('typeof value)
                let T = (unbox-pointer anyT type)
                try
                    imply-expr vT T value
                except (err)
                    cast-error!
                        \ "can't coerce value of type " vT T err

    as =
        spice-macro
            fn "as-dispatch" (args)
                let argc = ('argcount args)
                verify-count argc 2 2
                let value = ('getarg args 0)
                let anyT = ('getarg args 1)
                let vT = ('typeof value)
                let T = (unbox-pointer anyT type)
                try
                    # then try explicit cast
                    as-expr vT T value
                except (err)
                    cast-error! "can't cast value of type " vT T err

let BinaryOpFunctionType =
    'pointer ('raising (function Value type type Value Value) Error)

fn unbox-binary-op-function-type (anyf)
    unbox-pointer anyf BinaryOpFunctionType

# assuming both types are the same
fn binary-op-expr (symbol lhsT rhsT lhs rhs)
    let anyf = ('@ lhsT symbol)
    let f =
        try (unbox-binary-op-function-type anyf)
        except (err)
            attribute-format-error! lhsT symbol err
    f lhsT rhsT lhs rhs

# assuming both types are different
fn sym-binary-op-expr (symbol rsymbol lhsT rhsT lhs rhs)
    try
        return (binary-op-expr symbol lhsT rhsT lhs rhs)
    except (lhs-err)
        let anyf =
            try
                '@ rhsT rsymbol
            except (geterr)
                raise lhs-err
        let f =
            try (unbox-binary-op-function-type anyf)
            except (err)
                attribute-format-error! rhsT rsymbol err
        f lhsT rhsT lhs rhs

fn binary-op-error! (friendly-op-name lhsT rhsT err)
    compiler-error!
        'join "can't "
            'join friendly-op-name
                'join " values of types "
                    'join ('__repr (box-pointer lhsT))
                        'join " and "
                            'join ('__repr (box-pointer rhsT))
                                'join ": "
                                    sc_format_error err

# both types are typically the same
fn sym-binary-op-label-macro (args symbol rsymbol friendly-op-name)
    let argc = ('argcount args)
    verify-count argc 2 2
    let lhs rhs =
        'getarg args 0
        'getarg args 1
    let lhsT = ('typeof lhs)
    let rhsT = ('typeof rhs)
    if (ptrcmp== lhsT rhsT)
        try
            # use simple version
            return (binary-op-expr symbol lhsT lhsT lhs rhs)
        except (err)
            binary-op-error! friendly-op-name lhsT rhsT err
    # asymmetrical types
    # try direct operation first (from both sides)
    let err =
        try
            return (sym-binary-op-expr symbol rsymbol lhsT rhsT lhs rhs)
        except (err) err
    # try other options
    try
        # can we cast rhsT to lhsT?
        let rhs = (imply-expr rhsT lhsT rhs)
        # try again
        return (binary-op-expr symbol lhsT lhsT lhs rhs)
    except (err)
    try
        # can we cast lhsT to rhsT?
        let lhs = (imply-expr lhsT rhsT lhs)
        # try again
        return (binary-op-expr symbol rhsT rhsT lhs rhs)
    except (err)
    # we give up
    binary-op-error! friendly-op-name lhsT rhsT err

# right hand has fixed type
fn asym-binary-op-label-macro (args symbol rtype friendly-op-name)
    let argc = ('argcount args)
    verify-count argc 2 2
    let lhs rhs =
        'getarg args 0
        'getarg args 1
    let lhsT = ('typeof lhs)
    let rhsT = ('typeof rhs)
    try
        let f = ('@ lhsT symbol)
        if (ptrcmp== rhsT rtype)
            return `(f args)
        # can we cast rhsT to rtype?
        let rhs = (imply-expr rhsT rtype rhs)
        return `(f lhs rhs)
    except (err)
        # we give up
        compiler-error!
            'join "can't "
                'join friendly-op-name
                    'join " values of types "
                        'join ('__repr (box-pointer lhsT))
                            'join " and "
                                'join ('__repr (box-pointer rhsT))
                                    'join ": "
                                        sc_format_error err

fn unary-op-label-macro (args symbol friendly-op-name)
    let argc = ('argcount args)
    verify-count argc 1 1
    let lhs = ('getarg args 0)
    let lhsT = ('typeof lhs)
    try
        let f = ('@ lhsT symbol)
        return `(f args)
    except (err)
        compiler-error!
            'join "can't "
                'join friendly-op-name
                    'join " value of type "
                        'join ('__repr (box-pointer lhsT))
                            'join ": "
                                sc_format_error err

fn unary-sym-binary-op-label-macro (args usymbol ufriendly-op-name symbol rsymbol friendly-op-name)
    let argc = ('argcount args)
    if (icmp== argc 1)
        unary-op-label-macro args usymbol ufriendly-op-name
    else
        sym-binary-op-label-macro args symbol rsymbol friendly-op-name

inline make-unary-op-dispatch (symbol friendly-op-name)
    spice-macro (fn (args) (unary-op-label-macro args symbol friendly-op-name))

inline make-unary-sym-binary-op-dispatch (usymbol ufriendly-op-name symbol rsymbol friendly-op-name)
    spice-macro (fn (args) (unary-sym-binary-op-label-macro
        args usymbol ufriendly-op-name symbol rsymbol friendly-op-name))

inline make-sym-binary-op-dispatch (symbol rsymbol friendly-op-name)
    spice-macro (fn (args) (sym-binary-op-label-macro args symbol rsymbol friendly-op-name))

inline make-asym-binary-op-dispatch (symbol rtype friendly-op-name)
    spice-macro (fn (args) (asym-binary-op-label-macro args symbol rtype friendly-op-name))

# support for calling macro functions directly
'set-symbols SugarMacro
    __call =
        box-pointer
            inline (self at next scope)
                (bitcast self SugarMacroFunction) at next scope

'define-symbols Symbol
    unique =
        inline (cls name)
            sc_symbol_new_unique name
    variadic? = sc_symbol_is_variadic

'set-symbols Symbol
    __== = (box-binary-op (single-binary-op-dispatch icmp==))
    __!= = (box-binary-op (single-binary-op-dispatch icmp!=))
    __imply =
        box-cast
            fn "sugar-imply" (vT T expr)
                if (ptrcmp== T string)
                    return `(sc_symbol_to_string expr)
                compiler-error! "unsupported type"

fn string@ (self i)
    let s = (sc_string_buffer self)
    load (getelementptr s i)

'define-symbols string
    buffer = sc_string_buffer
    __countof = sc_string_count
    __@ = string@
    __lslice = sc_string_lslice
    __rslice = sc_string_rslice

'set-symbols string
    __== = (box-binary-op (single-binary-op-dispatch ptrcmp==))
    __!= = (box-binary-op (single-binary-op-dispatch ptrcmp!=))
    __.. = (box-binary-op (single-binary-op-dispatch sc_string_join))
    __< = (box-binary-op (single-binary-op-dispatch (inline (a b) (icmp<s (sc_string_compare a b) 0))))
    __<= = (box-binary-op (single-binary-op-dispatch (inline (a b) (icmp<=s (sc_string_compare a b) 0))))
    __> = (box-binary-op (single-binary-op-dispatch (inline (a b) (icmp>s (sc_string_compare a b) 0))))
    __>= = (box-binary-op (single-binary-op-dispatch (inline (a b) (icmp>=s (sc_string_compare a b) 0))))

'define-symbols list
    __typecall =
        inline (self args...)
            make-list args...
    __repr =
        inline "list-repr" (self)
            sc_list_repr self

'set-symbols list
    __.. = (box-binary-op (single-binary-op-dispatch sc_list_join))
    __== = (box-binary-op (single-binary-op-dispatch sc_list_compare))

inline single-signed-binary-op-dispatch (sf uf)
    fn (lhsT rhsT lhs rhs)
        if (ptrcmp== lhsT rhsT)
            return
                spice-quote
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
    let anchor = (sc_get_active_anchor)
    let ifval = (sc_if_new anchor)
    if flip
        sc_if_append_then_clause ifval anchor cond call-elsef
        sc_if_append_else_clause ifval anchor cond
    else
        sc_if_append_then_clause ifval anchor cond cond
        sc_if_append_else_clause ifval anchor call-elsef
    ifval

'set-symbols integer
    __imply = (box-cast integer-imply)
    __as = (box-cast integer-as)
    __+ = (box-binary-op (single-binary-op-dispatch add))
    __- = (box-binary-op (single-binary-op-dispatch sub))
    __neg = (box-pointer (inline (self) (sub (nullof (typeof self)) self)))
    __* = (box-binary-op (single-binary-op-dispatch mul))
    __// = (box-binary-op (single-signed-binary-op-dispatch sdiv udiv))
    __/ =
        box-binary-op
            single-signed-binary-op-dispatch
                inline (a b) (fdiv (sitofp a f32) (sitofp b f32))
                inline (a b) (fdiv (uitofp a f32) (uitofp b f32))
    __rcp = (spice-quote (inline (self) (fdiv 1.0 (as self f32))))
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
    __neg = (box-pointer (inline (self) (fsub (nullof (typeof self)) self)))
    __* = (box-binary-op (single-binary-op-dispatch fmul))
    __/ = (box-binary-op (single-binary-op-dispatch fdiv))
    __rcp = (box-pointer (inline (self) (fdiv (uitofp 1 (typeof self)) self)))
    __// = (box-binary-op (single-binary-op-dispatch floordiv))
    __% = (box-binary-op (single-binary-op-dispatch frem))


'set-symbols Value
    __== = (box-binary-op (single-binary-op-dispatch sc_value_compare))

'set-symbols Closure
    __== = (box-binary-op (single-binary-op-dispatch ptrcmp==))
    __!= = (box-binary-op (single-binary-op-dispatch ptrcmp!=))

'define-symbols type
    __@ = sc_type_element_at

'set-symbols type
    __== = (box-binary-op (single-binary-op-dispatch type==))
    __!= = (box-binary-op (single-binary-op-dispatch type!=))
    __< = (box-binary-op (single-binary-op-dispatch type<))
    __<= = (box-binary-op (single-binary-op-dispatch type<=))
    __> = (box-binary-op (single-binary-op-dispatch type>))
    __>= = (box-binary-op (single-binary-op-dispatch type>=))
    # (dispatch-attr T key thenf elsef)
    dispatch-attr =
        box-spice-macro
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
                try
                    let result = (sc_type_at self key)
                    return `(thenf result)
                except (err)
                    return `(elsef)
    __getattr =
        box-spice-macro
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
                        return (sc_type_at self key)
                `(sc_type_at args)

'set-symbols Scope
    __== = (box-binary-op (single-binary-op-dispatch ptrcmp==))
    __getattr =
        box-spice-macro
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
                        return (sc_scope_at self key)
                `(sc_scope_at args)
    __typecall =
        box-spice-macro
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
                            [(sc_extract_argument_list_new (sc_value_anchor args) args 1)])

#---------------------------------------------------------------------------
# null type
#---------------------------------------------------------------------------

""""The type of the `null` constant. This type is uninstantiable.
let NullType = (sc_typename_type "NullType")
'set-plain-storage NullType ('pointer void)
'set-symbols NullType
    __repr =
        box-pointer
            inline (self)
                sc_default_styler style-number "null"
    __imply =
        box-cast
            fn "null-imply" (clsT T expr)
                if (icmp== ('kind ('storageof T)) type-kind-pointer)
                    return `(bitcast expr T)
                compiler-error! "cannot convert to type"
    __== =
        box-binary-op
            fn (lhsT rhsT lhs rhs)
                if (icmp== ('kind ('storageof rhsT)) type-kind-pointer)
                    return `(icmp== (ptrtoint rhs usize) 0:usize)
                compiler-error! "only pointers can be compared to null"
    __r== =
        box-binary-op
            fn (lhsT rhsT lhs rhs)
                if (icmp== ('kind ('storageof lhsT)) type-kind-pointer)
                    return `(icmp== (ptrtoint lhs usize) 0:usize)
                compiler-error! "only pointers can be compared to null"

let
    and-branch = (spice-macro (fn (args) (dispatch-and-or args true)))
    or-branch = (spice-macro (fn (args) (dispatch-and-or args false)))
    #implyfn = (const-typify implyfn type type)
    #asfn = (const-typify asfn type type)
    countof = (make-unary-op-dispatch '__countof "count")
    unpack = (make-unary-op-dispatch '__unpack "unpack")
    hash1 = (make-unary-op-dispatch '__hash "hash")
    ~ = (make-unary-op-dispatch '__~ "bitwise-negate")
    == = (make-sym-binary-op-dispatch '__== '__r== "compare")
    != = (make-sym-binary-op-dispatch '__!= '__r!= "compare")
    < = (make-sym-binary-op-dispatch '__< '__r< "compare")
    <= = (make-sym-binary-op-dispatch '__<= '__r<= "compare")
    > = (make-sym-binary-op-dispatch '__> '__r> "compare")
    >= = (make-sym-binary-op-dispatch '__>= '__r>= "compare")
    + = (make-sym-binary-op-dispatch '__+ '__r+ "add")
    - = (make-unary-sym-binary-op-dispatch '__neg "negate" '__- '__r- "subtract")
    * = (make-sym-binary-op-dispatch '__* '__r* "multiply")
    / = (make-unary-sym-binary-op-dispatch '__rcp "invert" '__/ '__r/ "real-divide")
    // = (make-sym-binary-op-dispatch '__// '__r// "integer-divide")
    % = (make-sym-binary-op-dispatch '__% '__r% "modulate")
    & = (make-sym-binary-op-dispatch '__& '__r& "apply bitwise-and to")
    | = (make-sym-binary-op-dispatch '__| '__r| "apply bitwise-or to")
    ^ = (make-sym-binary-op-dispatch '__^ '__r^ "apply bitwise-xor to")
    << = (make-sym-binary-op-dispatch '__<< '__r<< "apply left shift with")
    >> = (make-sym-binary-op-dispatch '__>> '__r>> "apply right shift with")
    .. = (make-sym-binary-op-dispatch '__.. '__r.. "join")
    = = (make-sym-binary-op-dispatch '__= '__r= "apply assignment with")
    @ = (make-asym-binary-op-dispatch '__@ integer "apply subscript operator with")
    getattr = (make-asym-binary-op-dispatch '__getattr Symbol "get attribute from")
    lslice = (make-asym-binary-op-dispatch '__lslice usize "apply left-slice operator with")
    rslice = (make-asym-binary-op-dispatch '__rslice usize "apply right-slice operator with")

'set-symbols typename
    # inverted compare attempts regular compare
    __!= =
        box-binary-op
            fn (lhsT rhsT lhs rhs)
                if (ptrcmp== lhsT rhsT)
                    return `(not (== lhs rhs))
                compiler-error! "unequal types"
    # default assignment operator
    __= =
        box-binary-op
            fn (lhsT rhsT lhs rhs)
                if (ptrcmp== lhsT rhsT)
                    return
                        spice-quote
                            __drop lhs
                            assign rhs lhs
                compiler-error! "unequal types"

let missing-constructor =
    spice-macro
        fn "missing-constructor" (args)
            if false
                return `[]
            let argc = ('argcount args)
            verify-count argc 1 -1
            let cls = ('getarg args 0)
            compiler-error!
                sc_string_join "typename "
                    sc_string_join ('__repr cls)
                        " has no constructor"

run-stage;

let null = (nullof NullType)

#inline sugar-unbox (self destT)
    imply ('datum self) destT

inline not (value)
    bxor (imply value bool) true

let function->SugarMacro =
    const-typify
        fn "function->SugarMacro" (f)
            bitcast f SugarMacro
        SugarMacroFunction

inline sugar-block-scope-macro (f)
    function->SugarMacro (const-typify f list list Scope)

inline sugar-scope-macro (f)
    sugar-block-scope-macro
        fn (at next scope)
            let at scope = (f ('next at) scope)
            return (cons (Value at) next) scope

inline sugar-macro (f)
    sugar-block-scope-macro
        fn (at next scope)
            return (cons (Value (f ('next at))) next) scope

fn empty? (value)
    == (countof value) 0

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

        inline print (values...)
            va-lifold none print-element values...
            sc_write "\n"
            values...

'define-symbol integer '__typecall
    inline (cls value)
        as value cls

'define-symbol real '__typecall
    inline (cls value)
        as value cls

'set-symbols string
    __imply =
        box-cast
            fn "string-imply" (vT T expr)
                let string->rawstring =
                    spice-macro
                        fn (args)
                            let argc = ('argcount args)
                            verify-count argc 1 1
                            let str = ('getarg args 0)
                            if ('constant? str)
                                let s c = (sc_string_buffer (as str string))
                                `s
                            else
                                spice-quote
                                    do
                                        let s c = (sc_string_buffer str)
                                        s
                if (ptrcmp== T rawstring)
                    return `(string->rawstring expr)
                compiler-error! "unsupported type"

# implicit argument type coercion for functions, externs and typed labels
# --------------------------------------------------------------------------

let coerce-call-arguments =
    box-spice-macro
        fn "coerce-call-arguments" (args)
            let argc = ('argcount args)
            verify-count argc 1 -1
            let self = ('getarg args 0)
            let argc = (sub argc 1)
            let fptrT = ('typeof self)
            let fT = ('element@ fptrT 0)
            let pcount = ('element-count fT)
            if (== pcount argc)
                let outargs = (sc_call_new (sc_get_active_anchor) self)
                sc_call_set_rawcall outargs true
                loop (i = 0)
                    if (== i argc)
                        break outargs
                    let arg = ('getarg args (add i 1))
                    let argT = ('typeof arg)
                    let paramT = ('element@ fT i)
                    let outarg =
                        if (== argT paramT) arg
                        else `(imply arg paramT)
                    sc_call_append_argument outargs outarg
                    + i 1
            else `(rawcall self [('getarglist args 1)])

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
    set-type-symbol! pointer 'storageof
        fn (cls)
            pointer-type-storage-class cls
    set-type-symbol! pointer 'readable?
        fn (cls)
            == (& (pointer-type-flags cls) pointer-flag-non-readable) 0:u64

fn pointer-type-imply? (src dest)
    let ET = ('element@ src 0)
    let ET =
        if ('opaque? ET) ET
        else ('storageof ET)
    if (not (icmp== ('kind ET) type-kind-pointer))
        # casts to voidstar are only permitted if we are not holding
        # a ref to another pointer
        if (type== dest voidstar)
            return true
        elseif (type== dest ('mutable voidstar))
            if ('writable? src)
                return true
    if (type== dest ('strip-pointer-storage-class src))
        return true
    elseif (type== dest ('immutable src))
        return true
    elseif (type== dest ('strip-pointer-storage-class ('immutable src)))
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

'define-symbols pointer
    __typecall =
        inline (cls T)
            sc_pointer_type T pointer-flag-non-writable unnamed

# dotted symbol expander
# --------------------------------------------------------------------------

let dot-char = 46:i8 # "."
let dot-sym = '.

fn dotted-symbol? (env head)
    if (== head dot-sym)
        return false
    let s = (as head string)
    let sz = (countof s)
    loop (i = 0:usize)
        if (== i sz)
            return false
        elseif (== (@ s i) dot-char)
            return true
        + i 1:usize

fn split-dotted-symbol (head start end tail)
    let s = (as head string)
    loop (i = start)
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
        + i 1:usize

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
    '@ env (get-ifx-symbol (as op Symbol))

fn has-infix-ops? (infix-table expr)
    # any expression of which one odd argument matches an infix operator
        has infix operations.
    loop (expr = expr)
        if (< (countof expr) 3)
            return false
        let __ expr = ('decons expr)
        let at next = ('decons expr)
        try
            get-ifx-op infix-table at
            return true
        except (err)
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
        let op =
            try (get-ifx-op infix-table token)
            except (err)
                sc_set_active_anchor ('anchor token)
                compiler-error!
                    "unexpected token in infix expression"
        let op-prec = (unpack-infix-op op)
        ? (pred op-prec prec) op (Value none)

let infix-op-gt = (infix-op >)
let infix-op-ge = (infix-op >=)

fn rtl-infix-op-eq (infix-table token prec)
    let op =
        try (get-ifx-op infix-table token)
        except (err)
            sc_set_active_anchor ('anchor token)
            compiler-error!
                "unexpected token in infix expression"
    let op-prec op-order = (unpack-infix-op op)
    if (== op-order '<)
        ? (== op-prec prec) op (Value none)
    else
        Value none

fn parse-infix-expr (infix-table lhs state mprec)
    loop (lhs state = lhs state)
        if (empty? state)
            return lhs state
        let la next-state = ('decons state)
        let op = (infix-op-ge infix-table la mprec)
        if (== ('typeof op) Nothing)
            return lhs state
        let op-prec op-order op-name = (unpack-infix-op op)
        loop (rhs state = ('decons next-state))
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
            _ next-rhs next-state

let parse-infix-expr =
    const-typify parse-infix-expr Scope Value list i32

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
        try
            '@ env (as head-key Symbol)
        except (err) head-key
    let head =
        try
            '@ (as head type) '__macro
        except (err) head
    if (== ('typeof head) SugarMacro)
        let head = (as head SugarMacro)
        sc_set_active_anchor expr-anchor
        let expr env = (head expr topexpr-next env)
        return (as expr list) env
    elseif (has-infix-ops? env expr)
        let at next = ('decons expr)
        sc_set_active_anchor expr-anchor
        let expr =
            parse-infix-expr env at next 0
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
        #let expr = (sugar-wrap name-anchor expr false)
        return (cons expr next) env
    return topexpr env

fn quasiquote-list
inline quasiquote-any (ox)
    let x = ox
    let T = ('typeof x)
    if (== T list)
        quasiquote-list (as x list)
    else
        list sugar-quote ox
fn quasiquote-list (x)
    if (empty? x)
        return (list sugar-quote x)
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
                    if (> (countof at-next) 1)
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
    elseif (== (countof expr) 1)
        return ('@ expr)
    let expr = ('reverse expr)
    loop (result head = ('decons expr))
        if (empty? head)
            return result
        let at next = ('decons head)
        _ (Value (list f at (list inline '() result))) next

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
        loop (i lhs = 1 lhs)
            let rhs = ('getarg args i)
            let op = `(target lhs rhs)
            let i = (+ i 1)
            if (== i argc)
                break op
            _ i op

fn rtl-multiop (args target)
    let argc = ('argcount args)
    verify-count argc 2 -1
    if (== argc 2)
        `(target args)
    else
        # call for multiple args
        let lasti = (- argc 1)
        let rhs = ('getarg args lasti)
        loop (i rhs = lasti rhs)
            let i = (- i 1)
            let lhs = ('getarg args i)
            let op = `(target lhs rhs)
            if (== i 0)
                break op
            _ i op

# extracting options from varargs

# (va-option-branch key elsef args...)
fn va-option-branch (args)
    let argc = ('argcount args)
    verify-count argc 2 -1
    let key elsef =
        'getarg args 0
        'getarg args 1
    let key = (unbox-symbol key Symbol)
    loop (i = 2)
        if (== i argc)
            break;
        let arg = ('getarg args i)
        let argkey = ('key ('qualified-typeof arg))
        if (== key argkey)
            return
                sc_keyed_new (sc_value_anchor arg) unnamed arg
        + i 1
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
    if (== parent (nullof Scope))
        return (Scope b a)
    Scope
        clone-scope-contents parent b
        a

'define-symbols typename
    __typecall =
        inline (cls name)
            constbranch (== cls typename)
                inline ()
                    sc_typename_type name
                inline ()
                    missing-constructor cls

'set-symbols Scope
    __.. =
        box-binary-op
            single-binary-op-dispatch clone-scope-contents

fn extract-single-arg (args)
    let argc = ('argcount args)
    verify-count argc 1 1
    'getarg args 0

inline make-const-type-property-function (func)
    spice-macro
        fn (args)
            let value = (extract-single-arg args)
            let val = (func (as value type))
            `val

let
    constant? =
        spice-macro
            fn "constant?" (args)
                let value = (extract-single-arg args)
                Value ('constant? value)
    storageof = (make-const-type-property-function sc_type_storage)
    superof = (make-const-type-property-function sc_typename_type_get_super)
    sizeof = (make-const-type-property-function sc_type_sizeof)
    alignof = (make-const-type-property-function sc_type_alignof)

#del extract-single-arg
#del make-const-type-property-function

let Closure->Generator =
    spice-macro
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
        sugar-macro
            fn (args)
                if (== (countof args) 1)
                    quasiquote-any ('@ args)
                else
                    quasiquote-list args
    # dot macro
    # (. value symbol ...)
    . =
        sugar-macro
            fn (args)
                fn op (a b)
                    let sym = (as b Symbol)
                    list getattr a (list sugar-quote sym)
                let a rest = ('decons args)
                let b rest = ('decons rest)
                loop (rest result = rest (op a b))
                    if (empty? rest)
                        break result
                    let c rest = ('decons rest)
                    _ rest (op result c)
    and = (sugar-macro (make-expand-and-or and-branch))
    or = (sugar-macro (make-expand-and-or or-branch))
    define = (sugar-macro expand-define)
    define-infix> = (sugar-scope-macro (make-expand-define-infix '>))
    define-infix< = (sugar-scope-macro (make-expand-define-infix '<))
    .. = (spice-macro (fn (args) (rtl-multiop args (Value ..))))
    + = (spice-macro (fn (args) (ltr-multiop args (Value +))))
    * = (spice-macro (fn (args) (ltr-multiop args (Value *))))
    @ = (spice-macro (fn (args) (ltr-multiop args (Value @))))
    va-option-branch = (spice-macro va-option-branch)
    sugar-set-scope! =
        sugar-scope-macro
            fn (args sugar-scope)
                raises-compile-error;
                let scope rest = (decons args)
                return
                    none
                    as scope Scope

'set-symbol (__this-scope) (Symbol "#list")
    Value (const-typify list-handler list Scope)
'set-symbol (__this-scope) (Symbol "#symbol")
    Value (const-typify symbol-handler list Scope)

inline select-op-macro (sop fop numargs)
    inline scalar-type (T)
        let ST = ('storageof T)
        if (type== ('superof ST) vector)
            'element@ ST 0
        else ST
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc numargs numargs
            let a b =
                'getarg args 0; 'getarglist args 1
            let T = (scalar-type ('typeof a))
            let fun =
                if (type== ('superof T) integer) `sop
                elseif (type== ('superof T) real) `fop
                else
                    compiler-error!
                        sc_string_join "invalid argument type: "
                            sc_string_join (sc_value_repr (box-pointer T))
                                ". integer or real vector or scalar expected"
            `(fun a b)

fn powi (base exponent)
    # special case for constant base 2
    if (icmp== base 2)
        return
            shl 1 exponent
    loop (result cur exponent = 1 base exponent)
        if (icmp== exponent 0)
            return result
        else
            repeat
                do
                    if (icmp== (band exponent 1) 0) result
                    else
                        mul result cur
                mul cur cur
                lshr exponent 1

inline sabs (x)
    let zero = (nullof (typeof x))
    ? (icmp<s x zero) (sub zero x) x

let pow = (select-op-macro powi powf 2)
let abs = (select-op-macro sabs fabs 1)
let sign = (select-op-macro ssign fsign 1)

let hash = (sc_typename_type "hash")
'set-plain-storage hash u64

run-stage;

inline make-inplace-op (op)
    inline (lhs rhs)
        = lhs (op lhs rhs)

let
    -= = (make-inplace-op -)
    += = (make-inplace-op +)
    *= = (make-inplace-op *)
    /= = (make-inplace-op /)
    //= = (make-inplace-op //)
    %= = (make-inplace-op %)
    >>= = (make-inplace-op >>)
    <<= = (make-inplace-op <<)
    &= = (make-inplace-op &)
    |= = (make-inplace-op |)
    ^= = (make-inplace-op ^)
    ..= = (make-inplace-op ..)

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
define-infix< 50 ..=
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
define-infix> 780 :
define-infix> 800 .
define-infix> 800 @

inline char (s)
    let s sz = (sc_string_buffer s)
    load s

# (va-option key args... else-body)
let va-option =
    sugar-macro
        fn (args)
            let key va body = (decons args 2)
            let sym = (as key Symbol)
            list va-option-branch (list sugar-quote sym)
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
        spice-macro
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

# for <name> ... in <generator> body ...
define for
    inline fdone ()
        break;

    sugar-block-scope-macro
        fn "expand-for" (expr next-expr scope)
            let head args = (decons expr)
            let it params =
                loop (it params = args '())
                    if (empty? it)
                        compiler-error! "'in' expected"
                    let sxat it = (decons it)
                    let at = (sxat as Symbol)
                    if (at == 'in)
                        break it params
                    _ it (cons sxat params)
            let generator-expr body = (decons it)
            let subscope = (Scope scope)
            spice-quote
                let iter start =
                    (as [(sc_expand generator-expr '() subscope)] Generator);
            return
                cons
                    spice-quote iter start # order expressions
                        loop (next = start)
                            let next args... = (iter fdone next)
                            inline continue ()
                                repeat next
                            spice-unquote
                                let expr =
                                    loop (params expr = params (list '= args...))
                                        if (empty? params)
                                            break expr
                                        let param next = (decons params)
                                        _ next (cons param expr)
                                let expr = (cons let expr)
                                'set-symbol subscope 'continue continue
                                sc_expand (cons do expr body) '() subscope
                            next
                    next-expr
                scope

#---------------------------------------------------------------------------
# hashing
#---------------------------------------------------------------------------

let hash-storage =
    spice-macro
        fn "hash-storage" (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let value = ('getarg args 0)
            let OT = ('typeof value)
            let T =
                if ('opaque? OT) OT
                else ('storageof OT)
            let conv_u64 =
                switch ('kind T)
                case type-kind-integer
                    `(zext (bitcast value T) u64)
                case type-kind-pointer
                    `(ptrtoint value u64)
                default
                    if (type== T f32)
                        `(zext (bitcast value u32) u64)
                    elseif (type== T f64)
                        `(bitcast value u64)
                    else
                        compiler-error!
                            .. "can't hash storage of type " (repr OT)
            `(bitcast (sc_hash conv_u64 [('sizeof T)]) hash)

'set-symbols hash
    __hash = (inline (self) self)
    __== = integer.__==
    __!= = integer.__!=
    __as =
        box-cast
            fn "hash-as" (vT T expr)
                let ST = ('storageof vT)
                if (T == ST)
                    return `(bitcast expr T)
                elseif (T == integer)
                    return `(bitcast expr ST)
                compiler-error! "unsupported type"
    __ras =
        box-cast
            fn "hash-as" (vT T expr)
                if (vT == ('storageof vT))
                    return `(bitcast expr T)
                compiler-error! "unsupported type"
    __typecall =
        do
            inline hash2 (a b)
                bitcast
                    sc_hash2x64
                        bitcast (hash1 a) u64
                        bitcast (hash1 b) u64
                    hash
            spice-macro
                fn "hash-typecall" (args)
                    let argc = ('argcount args)
                    verify-count argc 2 -1
                    let value = ('getarg args 1)
                    if (argc == 2)
                        `(hash1 value)
                    else
                        ltr-multiop ('getarglist args 1) hash2

va-lfold none
    inline (key T)
        'set-symbol T '__hash hash-storage
    \ integer pointer real type Closure Builtin Symbol string Scope

#---------------------------------------------------------------------------
# module loading
#---------------------------------------------------------------------------

let wrap-if-not-run-stage =
    spice-macro
        fn (args)
            raises-compile-error;
            let argc = ('argcount args)
            if (argc == 1)
                let arg = ('getarg args 0)
                if (('typeof arg) == CompileStage)
                    return arg
            `(Value args)

let incomplete = (typename "incomplete")

run-stage;

fn make-module-path (pattern name)
    let sz = (countof pattern)
    loop (i start result = 0:usize 0:usize "")
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
    let f = (sc_eval expr-anchor (expr as list) eval-scope)
    loop (f = f)
        # build a wrapper
        let wrapf =
            spice-quote
                fn "exec-module-stage" ()
                    raises-compile-error;
                    wrap-if-not-run-stage (f)
        let wrapf = (sc_typify_template wrapf 0 (undef TypeArrayPointer))
        let f = (sc_compile wrapf 0:u64)
        if (('typeof f) == StageFunctionType)
            let fptr = (f as StageFunctionType)
            repeat (bitcast (fptr) Value)
        else
            let fptr = (f as ModuleFunctionType)
            break (fptr)

fn dots-to-slashes (pattern)
    let sz = (countof pattern)
    loop (i start result = 0:usize 0:usize "")
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
            do
                let newscope = (Scope (sc_get_globals))
                'set-docstring! newscope unnamed ""
                newscope
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

inline slice (value start end)
    rslice (lslice value end) start

fn require-from (base-dir name)
    #assert-typeof name Symbol
    let namestr = (dots-to-slashes (name as string))
    let package = ((fn () package))
    let modules = (package.modules as Scope)
    let all-patterns = (patterns-from-namestr base-dir namestr)
    loop (patterns = all-patterns)
        if (empty? patterns)
            sc_write "no such module '"
            sc_write (as name string)
            sc_write "' in paths:\n"
            loop (patterns = all-patterns)
                if (empty? patterns)
                    compiler-error! "failed to import module"
                let pattern patterns = (decons patterns)
                let pattern = (pattern as string)
                let module-path = (make-module-path pattern namestr)
                sc_write "    "
                sc_write module-path
                sc_write "\n"
                patterns
        let pattern patterns = (decons patterns)
        let pattern = (pattern as string)
        let module-path = (sc_realpath (make-module-path pattern namestr))
        if (empty? module-path)
            repeat patterns
        let module-path-sym = (Symbol module-path)
        let content =
            try ('@ modules module-path-sym)
            except (err)
                if (not (sc_is_file module-path))
                    repeat patterns
                'set-symbol modules module-path-sym incomplete
                let content = (load-module (name as string) module-path)
                'set-symbol modules module-path-sym content
                return content
        if (('typeof content) == type)
            if (content == incomplete)
                compiler-error!
                    .. "trying to import module " (repr name)
                        " while it is being imported"
        return content

let import =
    sugar-scope-macro
        fn "import" (args scope)
            fn resolve-scope (scope namestr start)
                let sz = (countof namestr)
                loop (i start = start start)
                    if (i == sz)
                        return (Symbol (slice namestr start i))
                    if ((@ namestr i) == (char "."))
                        if (i == start)
                            repeat (add i 1:usize) (add i 1:usize)
                    repeat (add i 1:usize) start
            let sxname rest = (decons args)
            let name = (sxname as Symbol)
            let namestr = (name as string)
            let module-dir = (scope.module-dir as string)
            let key = (resolve-scope scope namestr 0:usize)
            let module = (require-from module-dir name)
            'set-symbol scope key module
            _ module scope

""""export locals as a chain of two new scopes: a scope that contains
    all the constant values in the immediate scope, and a scope that contains
    the runtime values.
let locals =
    sugar-scope-macro
        fn "locals" (args scope)
            raises-compile-error;

            fn stage-constant? (value)
                ('pure? value) and (('typeof value) != SpiceMacro)

            let build-local =
                spice-macro
                    fn (args)
                        let constant-scope = (('getarg args 0) as Scope)
                        let tmp = ('getarg args 1)
                        let key = ('getarg args 2)
                        let value = ('getarg args 3)
                        let keydocstr = (('getarg args 4) as string)
                        if (stage-constant? value)
                            let key = (key as Symbol)
                            'set-symbol constant-scope key value
                            'set-docstring! constant-scope key keydocstr
                            `none
                        else
                            let wrapvalue =
                                if (('typeof value) == Value) value
                                else (Value value)
                            spice-quote
                                sc_scope_set_symbol tmp key wrapvalue
                                sc_scope_set_docstring tmp key keydocstr

            let build-locals =
                spice-macro
                    fn (args)
                        let scope = (('getarg args 0) as Scope)
                        let docstr = ('docstring scope unnamed)
                        let constant-scope = (Scope)
                        if (not (empty? docstr))
                            'set-docstring! constant-scope unnamed docstr
                        let tmp = `(Scope constant-scope)
                        let block = (sc_expression_new (sc_get_active_anchor))
                        sc_expression_append block tmp
                        loop (last-key = unnamed)
                            let key value = ('next scope last-key)
                            if (key == unnamed)
                                sc_expression_append block tmp
                                return block
                            #if (not (stage-constant? value))
                            let keydocstr = ('docstring scope key)
                            let value = (sc_extract_argument_new (sc_value_anchor value) value 0)
                            sc_expression_append block
                                `(build-local constant-scope tmp key value keydocstr)
                            repeat key

            return `(build-locals scope) scope

fn set-symbols-from-scope (T scope)
    loop (last-key = unnamed)
        let key value =
            'next scope last-key
        if (key == unnamed)
            return;
        #let keydocstr = ('docstring scope key)
        'set-symbol T key value
        #'set-docstring! T key keydocstr
        repeat key

#---------------------------------------------------------------------------
# using
#---------------------------------------------------------------------------

fn merge-scope-symbols (source target filter)
    fn process-keys (source target filter)
        loop (last-key = unnamed)
            let key value = ('next source last-key)
            if (key != unnamed)
                if
                    or
                        none? filter
                        do
                            let keystr = (key as string)
                            'match? filter keystr
                    'set-symbol target key value
                repeat key
            else
                break target
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
    sugar-scope-macro
        fn "using" (args sugar-scope)
            let name rest = (decons args)
            let nameval = name
            if ((('typeof nameval) == Symbol) and ((nameval as Symbol) == 'import))
                let module-dir = (('@ sugar-scope 'module-dir) as string)
                let name rest = (decons rest)
                let name = (name as Symbol)
                let module = ((require-from module-dir name) as Scope)
                return (list do none)
                    .. module sugar-scope

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
                        merge-scope-symbols src sugar-scope none
                    else
                        merge-scope-symbols src sugar-scope (('@ pattern) as string)
            if (('typeof nameval) == Symbol)
                let sym = (nameval as Symbol)
                label skip
                    let src =
                        try
                            ('@ sugar-scope sym) as Scope
                        except (err)
                            merge skip
                    return (process src)
            elseif (('typeof nameval) == Scope)
                return (process (nameval as Scope))
            compiler-error! "using: scope expeced"
            #return
                list run-stage
                    cons merge-scope-symbols name 'sugar-scope pattern
                sugar-scope

# (define-macro name expr ...)
# implies builtin names:
    args : list
define define-sugar-macro
    sugar-macro
        fn "expand-define-sugar-macro" (expr)
            raises-compile-error;
            let name body = (decons expr)
            list define name
                list sugar-macro
                    cons fn '(args)
                        list raises-compile-error;
                        body

let __assert =
    spice-macro
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
                if (('typeof msg) != string)
                    compiler-error! "string expected as second argument"
                `(check-assertion expr anchor msg)

let vector-reduce =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 2 2
            let f v =
                'getarg args 0
                'getarg args 1
            let T = ('typeof v)
            let sz = ('element-count T)
            loop (v sz = v sz)
                # special cases for low vector sizes
                switch sz
                case 1
                    break
                        spice-quote
                            extractelement v 0
                case 2
                    break
                        spice-quote
                            f
                                extractelement v 0
                                extractelement v 1
                case 3
                    break
                        spice-quote
                            f
                                f
                                    extractelement v 0
                                    extractelement v 1
                                extractelement v 2
                case 4
                    break
                        spice-quote
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
                        spice-quote
                            f
                                lslice v hsz-value
                                rslice v hsz-value
                        hsz

let __countof-aggregate =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let self = ('getarg args 0)
            let T = ('typeof self)
            let sz = ('element-count T)
            Value (sz as usize)

run-stage;

# (define-scope-macro name expr ...)
# implies builtin names:
    args : list
    scope : Scope
define-sugar-macro define-sugar-scope-macro
    let name body = (decons args)
    list define name
        list sugar-scope-macro
            cons fn '(args sugar-scope) body

# (define-block-scope-macro name expr ...)
# implies builtin names:
    expr : list
    next-expr : list
    scope : Scope
define-sugar-macro define-sugar-block-scope-macro
    let name body = (decons args)
    list define name
        list sugar-block-scope-macro
            cons fn '(expr next-expr sugar-scope) body

'set-symbols type
    symbols =
        inline "symbols" (self)
            Generator
                inline (fdone key)
                    let key value =
                        sc_type_next self key
                    if (key == unnamed)
                        fdone;
                    else
                        _ key key value
                unnamed
    elements =
        inline "elements" (self)
            let count = ('element-count self)
            Generator
                inline (fdone i)
                    if (i == count)
                        fdone;
                    else
                        _ (i + 1) ('element@ self i)
                0

inline scope-generator (self)
    Generator
        inline (fdone key)
            let key value =
                sc_scope_next self key
            if (key == unnamed)
                fdone;
            else
                _ key key value
        unnamed

inline list-generator (self)
    Generator
        inline (fdone cell)
            if (empty? cell)
                fdone;
            else
                let at next = (decons cell)
                _ next at
        self

'set-symbols Scope
    __as =
        box-cast
            fn "scope-as" (vT T expr)
                if (T == Generator)
                    return `(scope-generator expr)
                compiler-error! "unsupported type"

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

let parse-compile-flags =
    spice-macro
        fn (args)
            inline flag-error (flag)
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
            let argc = ('argcount args)
            loop (i flags = 0 0:u64)
                if (i == argc)
                    break `flags
                let arg = ('getarg args i)
                let flag = (arg as Symbol)
                let flag =
                    switch flag
                    case 'dump-disassembly compile-flag-dump-disassembly
                    case 'dump-module compile-flag-dump-module
                    case 'dump-function compile-flag-dump-function
                    case 'dump-time compile-flag-dump-time
                    case 'no-debug-info compile-flag-no-debug-info
                    case 'O1 compile-flag-O1
                    case 'O2 compile-flag-O2
                    case 'O3 compile-flag-O3
                    default (flag-error flag)
                _ (i + 1) (flags | flag)

spice-quote
    inline compile (func flags...)
        sc_compile func (parse-compile-flags flags...)

    inline compile-glsl (target func flags...)
        sc_compile_glsl target func (parse-compile-flags flags...)

    inline compile-object (func table flags...)
        sc_compile_object func table (parse-compile-flags flags...)

define-sugar-macro assert
    let cond msg body = (decons args 2)
    let msg =
        if ((countof args) == 2) msg
        else
            if (('typeof cond) == list)
                `[(sc_list_repr (cond as list))]
            else
                `[(repr cond)]
    list __assert cond msg

define-sugar-macro while
    let cond body = (decons args)
    list loop '()
        list inline 'continue '()
            list repeat
        list if cond
            cons do body
            list repeat
        list 'else
            list break

#-------------------------------------------------------------------------------
# tuples
#-------------------------------------------------------------------------------

inline make-unpack-function (extractf)
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let self = ('getarg args 0)
            let T = ('typeof self)
            let count = ('element-count T)
            let outargs = (sc_argument_list_new (sc_get_active_anchor))
            loop (i = 0)
                if (icmp== i count)
                    break outargs
                sc_argument_list_append outargs `(extractf self i)
                add i 1

let __unpack-aggregate = (make-unpack-function extractvalue)

'set-symbols tuple
    __unpack = __unpack-aggregate
    __countof = __countof-aggregate
    __getattr = extractvalue
    __@ = extractvalue

let tupleof =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            #verify-count argc 0 -1
            raises-compile-error;

            # build tuple type
            let field-types = (alloca-array type argc)
            loop (i = 0)
                if (i == argc)
                    break;
                let k arg = ('dekey ('getarg args i))
                let T = ('key-type ('typeof arg) k)
                store T (getelementptr field-types i)
                i + 1

            # generate insert instructions
            let TT = (sc_tuple_type argc field-types)
            loop (i result = 0 `(nullof TT))
                if (i == argc)
                    break result
                let arg = ('getarg args i)
                _ (i + 1)
                    `(insertvalue result arg i)

#-------------------------------------------------------------------------------
# arrays
#-------------------------------------------------------------------------------

'set-symbols array
    __unpack = __unpack-aggregate
    __countof = __countof-aggregate
    __@ =
        inline (self index)
            extractvalue self index
    __typecall =
        inline "array.__typecall" (cls element-type size)
            sc_array_type element-type (size as usize)
    __as =
        do
            inline array-generator (arr)
                let count = (countof arr)
                let stackarr = (ptrtoref (alloca (typeof arr)))
                stackarr = arr
                Generator
                    inline (fdone x)
                        if (< x count)
                            _ (+ x 1:usize) (@ stackarr x)
                        else
                            fdone;
                    0:usize
            box-cast
                fn "array.__as" (vT T expr)
                    if (T == Generator)
                        return `(array-generator expr)
                    compiler-error! "unsupported type"

let arrayof =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 -1
            raises-compile-error;

            let ET = (('getarg args 0) as type)
            let numvals = (sub argc 1)

            let TT = (sc_array_type ET (usize numvals))

            # generate insert instructions
            loop (i result = 0 `(undef TT))
                if (i == numvals)
                    break result
                let arg = ('getarg args (add i 1))
                let arg =
                    if ((sc_value_type arg) == ET) arg
                    else `(arg as ET)
                _ (i + 1)
                    `(insertvalue result arg i)

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
                spice-quote
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
    let f =
        try ('@ Ta symbol)
        except (err)
            compiler-error! "unsupported operation"
    let f = (unbox-binary-op-function-type f)
    return (f lhsT rhsT lhs rhs)

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
        spice-macro
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
                loop (i = 0)
                    if (i == offset)
                        break;
                    store (Value i) (getelementptr maskvals i)
                    i + 1
                let maskT =
                    sc_vector_type i32 offset:usize
                spice-quote
                    shufflevector self self
                        [ sc_const_aggregate_new maskT offset maskvals ]
    __rslice =
        spice-macro
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
                loop (i = 0)
                    if (i == total)
                        break;
                    store (Value (i + offset)) (getelementptr maskvals i)
                    i + 1
                let maskT =
                    sc_vector_type i32 total:usize
                spice-quote
                    shufflevector self self
                        [ sc_const_aggregate_new maskT total maskvals ]
    __unpack = (Value (make-unpack-function extractelement))
    __countof = __countof-aggregate
    # vector type constructor
    __typecall =
        inline "vector.__typecall" (cls element-type size)
            sc_vector_type element-type (size as usize)
    #__typecall =
        spice-macro
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
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 -1
            raises-compile-error;

            let ET = (('getarg args 0) as type)
            let numvals = (sub argc 1)

            # generate insert instructions
            let TT = (sc_vector_type ET (usize numvals))
            loop (i result = 0 `(nullof TT))
                if (i == numvals)
                    break result
                let arg = ('getarg args (add i 1))
                let arg =
                    if ((sc_value_type arg) == ET) arg
                    else `(arg as ET)
                _ (i + 1) `(insertelement result arg i)

#-------------------------------------------------------------------------------

let
    min =
        spice-macro
            fn (args)
                ltr-multiop args
                    Value
                        inline "min" (a b)
                            ? (<= a b) a b
    max =
        spice-macro
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
    docstring = sc_closure_get_docstring
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
                        sugar-error! ('anchor result)
                            .. "function does not compile to type " (repr destT)
                                \ "but has type " (repr resultT)
                    return result
                compiler-error! "unsupported type"

inline extern (name T attrs...)
    let storage-class = (va-option storage attrs... unnamed)
    sc_global_new (sc_get_active_anchor) name T 0:u32 storage-class -1 -1

let
    private =
        spice-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 1 1
                let T = ('getarg args 0)
                let T = (T as type)
                extern unnamed T (storage = 'Private)

#-------------------------------------------------------------------------------

fn extract-name-params-body (expr)
    let arg body = (decons expr)
    if (('typeof arg) == list)
        return (Value "") (arg as list) body
    else
        let params body = (decons body)
        return arg (params as list) body

fn check-count (count mincount maxcount)
    if (icmp>=s mincount 0)
        if (icmp<s count mincount)
            return false
    if (icmp>=s maxcount 0)
        if (icmp>s count maxcount)
            return false
    return true

fn next-head? (next)
    if (not (empty? next))
        let expr next = (decons next)
        if (('typeof expr) == list)
            let at = ('@ (expr as list))
            if (('typeof at) == Symbol)
                return (at as Symbol)
    unnamed

inline gen-match-block-parser (handle-case)
    sugar-block-scope-macro
        fn (expr next scope)
            let head arg argrest = (decons expr 2)
            let arg argrest = (sc_expand arg argrest scope)
            let outnext = (alloca-array list 1)
            let outexpr next =
                spice-quote
                    label ok-label
                        inline return-ok (args...)
                            merge ok-label args...
                        spice-unquote
                            let outexpr = (sc_expression_new (sc_get_active_anchor))
                            loop (next = next)
                                let head = (next-head? next)
                                switch head
                                case 'case
                                    let expr next = (decons next)
                                    let expr = (expr as list)
                                    let head cond body = (decons expr 2)
                                    sc_expression_append outexpr
                                        spice-quote
                                            label case-label
                                                inline fail-case ()
                                                    merge case-label
                                                let token = arg
                                                spice-unquote
                                                    let newscope = (Scope scope)
                                                    let unpack-expr =
                                                        handle-case fail-case token newscope cond
                                                    let body =
                                                        sc_expand (cons do body) '() newscope
                                                    spice-quote
                                                        unpack-expr
                                                        return-ok body
                                    repeat next
                                case 'default
                                    let expr next = (decons next)
                                    let expr = (expr as list)
                                    let head body = (decons expr)
                                    let body =
                                        sc_expand (cons do body) '() scope
                                    sc_expression_append outexpr `(return-ok body)
                                    store next outnext
                                    break outexpr
                                default
                                    compiler-error! "default branch missing"
            return (cons outexpr (load outnext)) scope

fn gen-sugar-matcher (failfunc expr scope params)
    if false
        return `[]
    let params = (params as list)
    let paramcount = (countof params)
    let outexpr = (sc_expression_new (sc_get_active_anchor))
    loop (i rest next varargs = 0 params expr false)
        if (not (empty? rest))
            let paramv rest = (decons rest)
            let T = ('typeof paramv)
            if (T == Symbol)
                let param = (paramv as Symbol)
                let variadic? = ('variadic? param)
                let arg next =
                    if variadic?
                        if (not (empty? rest))
                            sugar-error! ('anchor paramv)
                                "variadic match pattern is not in last place"
                        _ next `[]
                    else
                        spice-quote
                            let arg next =
                                sc_list_decons next
                        _ arg next
                sc_expression_append outexpr arg
                'set-symbol scope param arg
                repeat (i + 1) rest next (| varargs variadic?)
            elseif (T == list)
                let param = (paramv as list)
                let head head-rest = (decons param)
                let mid mid-rest = (decons head-rest)
                if ((('typeof head) == Symbol) and ((head as Symbol) == 'sugar-quote))
                    let head = (head as Symbol)
                    let sym = ((decons head-rest) as Symbol)
                    sc_expression_append outexpr
                        spice-quote
                            let arg next = (sc_list_decons next)
                            if (ptrcmp!= ('typeof arg) Symbol)
                                failfunc;
                            if ((arg as Symbol) != sym)
                                failfunc;
                    repeat (i + 1) rest next varargs
                elseif ((('typeof mid) == Symbol) and ((mid as Symbol) == 'as))
                    let exprT = (decons mid-rest)
                    let exprT = (sc_expand exprT '() scope)
                    let param = (head as Symbol)
                    if ('variadic? param)
                        sugar-error! ('anchor head)
                            "vararg parameter cannot be typed"
                    sc_expression_append outexpr
                        spice-quote
                            let arg next =
                                sc_list_decons next
                            let arg =
                                if (('constant? arg) and (('typeof arg) == exprT))
                                    arg as exprT
                                else
                                    failfunc;
                    'set-symbol scope param arg
                    repeat (i + 1) rest next varargs
                else
                    sc_expression_append outexpr
                        spice-quote
                            let arg next = (sc_list_decons next)
                            let arg =
                                if (ptrcmp!= ('typeof arg) list)
                                    failfunc;
                                else
                                    arg as list
                            spice-unquote
                                gen-sugar-matcher failfunc arg scope param
                    repeat (i + 1) rest next varargs
            else
                sugar-error! ('anchor paramv)
                    "unsupported pattern"
        return
            spice-quote
                if (not (check-count (sc_list_count expr)
                        [(? varargs (sub paramcount 1) paramcount)]
                        [(? varargs -1 paramcount)]))
                    failfunc;
                outexpr

define sugar-match
    gen-match-block-parser gen-sugar-matcher

#-------------------------------------------------------------------------------

define sugar
    inline wrap-sugar-macro (f)
        sugar-block-scope-macro
            fn (expr next scope)
                let new-expr new-next = (f expr next scope)
                return
                    constbranch (none? new-next)
                        inline ()
                            cons new-expr next
                        inline ()
                            cons new-expr new-next
                    scope

    sugar-block-scope-macro
        fn "expand-sugar" (expr next scope)
            raises-compile-error;
            let head expr = (decons expr)
            let name params body =
                extract-name-params-body expr
            let func =
                spice-quote
                    inline (expr next-expr sugar-scope)
                        let head expr = (sc_list_decons expr)
                        label ok-label
                            inline return-ok (args...)
                                merge ok-label args...
                            label fail-label
                                inline fail-case ()
                                    merge fail-label
                                spice-unquote
                                    let subscope = (Scope scope)
                                    'set-symbols subscope
                                        next-expr = next-expr
                                        sugar-scope = sugar-scope
                                        expr-head = head
                                    let unpack-expr =
                                        gen-sugar-matcher fail-case expr subscope params
                                    let body =
                                        sc_expand (cons do body) '() subscope
                                    spice-quote
                                        unpack-expr
                                        return-ok body
                            compiler-error! "syntax error"
            let outexpr =
                if (('typeof name) == Symbol)
                    qq
                        [let name] =
                            [wrap-sugar-macro func];
                else
                    qq
                        [wrap-sugar-macro func];
            return (cons outexpr next) scope

#-------------------------------------------------------------------------------

fn uncomma (l)
    """"uncomma list l, wrapping all comma separated symbols as new lists
        example:
            (uncomma '(a , b c d , e f , g h)) -> '(a (b c d) (e f) (g h))
    fn comma-separated? (l)
        loop (next = l)
            if (empty? next)
                return false
            let at next = (decons next)
            if ((('typeof at) == Symbol) and ((at as Symbol) == ',))
                return true
            repeat next
    fn merge-lists (anchor current total)
        if ((countof current) == 0)
            sugar-error! anchor "unexpected comma"
        cons
            if ((countof current) == 1) ('@ current)
            else (Value current)
            total
    if (comma-separated? l)
        fn process (l)
            raises-compile-error;
            if (empty? l)
                return (nullof Anchor) '() '()
            let at next = (decons l)
            let anchor current total = (process next)
            let anchor = ('anchor at)
            if ((('typeof at) == Symbol) and ((at as Symbol) == ',))
                if (empty? next)
                    return anchor '() total
                return anchor '() (merge-lists anchor current total)
            else
                return anchor (cons at current) total
        return (merge-lists (process l))
    else l

inline parse-argument-matcher (failfunc expr scope params cb)
    #if false
        return `[]
    let params = (params as list)
    let params = (uncomma params)
    let paramcount = (countof params)
    loop (i rest varargs = 0 params false)
        if (empty? rest)
            return
                spice-quote
                    if (not (check-count (sc_argcount expr)
                            [(? varargs (sub paramcount 1) paramcount)]
                            [(? varargs -1 paramcount)]))
                        failfunc;
        let paramv rest = (decons rest)
        let T = ('typeof paramv)
        if (T == Symbol)
            let param = (paramv as Symbol)
            let variadic? = ('variadic? param)
            let arg =
                if variadic?
                    if (not (empty? rest))
                        sugar-error! ('anchor paramv)
                            "vararg parameter is not in last place"
                    `(sc_getarglist expr i)
                else
                    `(sc_getarg expr i)
            cb param arg
            repeat (i + 1) rest (| varargs variadic?)
        elseif (T == list)
            let param = (paramv as list)
            let head head-rest = (decons param)
            let mid mid-rest = (decons head-rest)
            if ((('typeof mid) == Symbol) and ((mid as Symbol) == ':))
                let exprT = (decons mid-rest)
                let exprT = (sc_expand exprT '() scope)
                let param = (head as Symbol)
                if ('variadic? param)
                    sugar-error! ('anchor head)
                        "vararg parameter cannot be typed"
                spice-quote
                    let arg = (sc_getarg expr i)
                    let arg =
                        try (imply-expr ('typeof arg) exprT arg)
                        except (err)
                            failfunc;
                cb param arg
                repeat (i + 1) rest varargs
            elseif ((('typeof mid) == Symbol) and ((mid as Symbol) == 'as))
                let exprT = (decons mid-rest)
                let exprT = (sc_expand exprT '() scope)
                let param = (head as Symbol)
                if ('variadic? param)
                    sugar-error! ('anchor head)
                        "vararg parameter cannot be typed"
                spice-quote
                    let arg = (sc_getarg expr i)
                    let arg =
                        if (('constant? arg) and (('typeof arg) == exprT))
                            arg as exprT
                        else
                            failfunc;
                cb param arg
                repeat (i + 1) rest varargs
        sugar-error! ('anchor paramv) "unsupported pattern"

fn gen-argument-matcher (failfunc expr scope params)
    let outexpr = (sc_expression_new (sc_get_active_anchor))
    let outargs = (sc_argument_list_new (sc_get_active_anchor))
    'set-symbol scope '*... outargs
    let header =
        parse-argument-matcher failfunc expr scope params
            inline (param arg)
                sc_expression_append outexpr arg
                sc_argument_list_append outargs arg
                'set-symbol scope param arg
    spice-quote
        header
        outexpr

define spice-match
    gen-match-block-parser gen-argument-matcher

#inline spice-macro (f)
    spice-macro-verify-signature f
    bitcast (const-typify f Value) SpiceMacro

define spice
    sugar-macro
        fn "expand-spice" (expr)
            raises-compile-error;
            let name params body =
                extract-name-params-body expr
            let paramcount = ((countof params) as i32)

            let args = (Symbol "#args")
            loop (i rest body varargs = 0 params body false)
                if (not (empty? rest))
                    let paramv rest = (decons rest)
                    let param = (paramv as Symbol)
                    let variadic? = ('variadic? param)
                    let body =
                        if variadic?
                            if (not (empty? rest))
                                sugar-error! ('anchor paramv)
                                    "vararg parameter is not in last place"
                            cons
                                qq
                                    [let paramv] =
                                        [`sc_getarglist args i];
                                body
                        else
                            cons
                                qq
                                    [let paramv] =
                                        [`sc_getarg args i];
                                body
                    repeat (i + 1) rest body (| varargs variadic?)
                let content =
                    cons (list args)
                        qq
                            [verify-count] ([`sc_argcount args])
                                [(? varargs (sub paramcount 1) paramcount)]
                                [(? varargs -1 paramcount)]
                        body
                break
                    if (('typeof name) == Symbol)
                        qq
                            [let name] =
                                [spice-macro]
                                    [fn] [(name as Symbol as string)] (args)
                                        [spice-quote]
                                            [spice-unquote]
                                                [(cons inline content)] args
                    else
                        qq
                            [spice-macro]
                                [fn name] (args)
                                    [spice-quote]
                                        [spice-unquote]
                                            [(cons inline content)] args

#-------------------------------------------------------------------------------

fn gen-match-matcher

fn gen-or-matcher (failfunc expr scope params)
    spice-quote
        label or-ok
            spice-unquote
                loop (prefix params = `[] params)
                    let at params = (decons params)
                    if (empty? params)
                        break
                            spice-quote
                                prefix
                                spice-unquote
                                    let unpack-expr =
                                        gen-match-matcher failfunc expr
                                            \ (Scope scope) at
                                    spice-quote unpack-expr (merge or-ok)
                    repeat
                        spice-quote
                            label or-fail
                                inline fail-case ()
                                    merge or-fail
                                prefix
                                spice-unquote
                                    let unpack-expr =
                                        gen-match-matcher fail-case expr
                                            \ (Scope scope) at
                                    spice-quote unpack-expr (merge or-ok)
                        params

fn gen-match-matcher (failfunc expr scope cond)
    """"features:
        <constant> -> (input == <constant>)
        (or <expr_a> <expr_b>) -> (or <expr_a> <expr_b>)

        TODO:
        (: x T) -> ((typeof input) == T), let x = input
        <unknown symbol> -> unpack as symbol
    if false
        return `[]
    let condT = ('typeof cond)
    if (condT == list)
        let cond-anchor = ('anchor cond)
        let cond = (uncomma (cond as list))
        let cond =
            if (has-infix-ops? scope cond)
                let at next = ('decons cond)
                sc_set_active_anchor ('anchor at)
                let expr =
                    parse-infix-expr scope at next 0
                expr as list
            else cond
        let head rest = (decons cond)
        let T = ('typeof head)
        if (T == Symbol)
            let token = (head as Symbol)
            if (token == 'or)
                return (gen-or-matcher failfunc expr scope rest)
        sugar-error! cond-anchor
            .. "unsupported pattern: " (repr cond)
    let cond =
        sc_expand cond '() scope
    spice-quote
        if (expr != cond)
            failfunc;

#-------------------------------------------------------------------------------

define match
    gen-match-block-parser gen-match-matcher

let OverloadedFunction = (typename "OverloadedFunction")

run-stage;

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
# function memoization
#-------------------------------------------------------------------------------

inline memoize (f)
    fn (args...)
        let key = `[f args...]
        let value = (sc_map_get key)
        if (value == null)
            let value =
                `[(f args...)]
            sc_map_set key value
            value
        else value

inline type-factory (f)
    let f = (memoize f)
    fn (...)
        ((f ...) as type)

#-------------------------------------------------------------------------------
# function overloading
#-------------------------------------------------------------------------------

fn get-overloaded-fn-append ()
    spice "overloaded-fn-append" (T args...)
        let outtype = (T as type)
        let functions = ('@ outtype 'templates)
        let functypes = ('@ outtype 'parameter-types)
        for i in (range 0 ('argcount args...) 2)
            let f = ('getarg args... i)
            let ftype = ('getarg args... (i + 1))
            if (('typeof ftype) == Nothing)
                let fT = ('typeof f)
                if ('function-pointer? fT)
                    if ((('kind f) != value-kind-function)
                        and (not ('constant? f)))
                        sugar-error! ('anchor f) "argument must be constant or function"
                    let fT = ('element@ fT 0)
                    let argcount = ('element-count fT)
                    loop (k types = 0 void)
                        if (k < argcount)
                            let argT = ('element@ fT k)
                            repeat (k + 1)
                                sc_arguments_type_join types argT
                        sc_argument_list_append functions f
                        sc_argument_list_append functypes types
                        break;
                elseif (fT == type)
                    if (fT == outtype)
                        sugar-error! ('anchor f) "cannot inherit from own type"
                    let fT = (f as type)
                    if (fT < OverloadedFunction)
                        let fns = ('@ fT 'templates)
                        let ftypes = ('@ fT 'parameter-types)
                        # copy over existing options
                        for func ftype in (zip ('args fns) ('args ftypes))
                            sc_argument_list_append functions func
                            sc_argument_list_append functypes ftype
                elseif (fT == Closure)
                    # ensure argument is constant
                    f as Closure
                    # append as templated option
                    sc_argument_list_append functions f
                    sc_argument_list_append functypes Variadic
                else
                    sugar-error! ('anchor f)
                        .. "cannot embed argument of type "
                            repr ('typeof f)
                            " in overloaded function"
            else
                let T = (ftype as type)
                sc_argument_list_append functions f
                sc_argument_list_append functypes ftype
        T

'set-symbols OverloadedFunction
    append = (get-overloaded-fn-append)
    __typecall =
        spice "dispatch-overloaded-function" (cls args...)
            let T = (cls as type)
            let fns = ('@ T 'templates)
            let ftypes = ('@ T 'parameter-types)
            let count = ('argcount args...)
            for f FT in (zip ('args fns) ('args ftypes))
                let FT = (FT as type)
                let argcount = (sc_arguments_type_argcount FT)
                let variadic? =
                    (argcount > 0) and
                        ((sc_arguments_type_getarg FT (argcount - 1)) == Variadic)
                if variadic?
                    if ((count + 1) < argcount)
                        continue;
                elseif (count != argcount)
                    continue;
                label break-next
                    let outargs = (sc_call_new (sc_get_active_anchor) f)
                    sc_call_set_rawcall outargs true
                    let lasti = (argcount - 1)
                    for i arg in (enumerate ('args args...))
                        let argT = ('typeof arg)
                        let paramT = (sc_arguments_type_getarg FT (min i lasti))
                        let outarg =
                            if (paramT == Unknown) arg
                            elseif (paramT == Variadic) arg
                            elseif (argT == paramT) arg
                            else
                                try (imply-expr argT paramT arg)
                                except (err)
                                    merge break-next
                        sc_call_append_argument outargs outarg
                    return outargs
            # if we got here, there was no match
            compiler-error!
                .. "could not match argument types ("
                    do
                        loop (i str = 0 "")
                            if (i < count)
                                repeat (i + 1)
                                    .. str
                                        ? (i == 0) "" " "
                                        repr ('typeof ('getarg args... i))
                            break str
                    ") to overloaded function with types"
                    do
                        let fcount = ('argcount ftypes)
                        loop (i str = 0 "")
                            if (i < fcount)
                                repeat (i + 1)
                                    .. str
                                        "\n    "
                                        repr (('getarg ftypes i) as type)
                            break str

sugar fn... (name...)
    let finalize-overloaded-fn = (get-overloaded-fn-append)
    let fn-name =
        sugar-match name...
        case (name as Symbol;) name
        case (name as string;) (Symbol name)
        case () unnamed
        default
            compiler-error!
                """"syntax: (fn... name|"name") (case pattern body...) ...
    let anchor = (sc_get_active_anchor)
    let outargs = (sc_argument_list_new anchor)
    let outtype = (sc_typename_type (fn-name as string))
    'set-super outtype OverloadedFunction
    'set-symbols outtype
        templates = (sc_argument_list_new anchor)
        parameter-types = (sc_argument_list_new anchor)
    let bodyscope = (Scope sugar-scope)
    sugar-match name...
    case (name as Symbol;)
        'set-symbol bodyscope fn-name outtype
    default;
    loop (next = next-expr)
        sugar-match next
        case (('case 'using body...) rest...)
            let obj = (sc_expand (cons do body...) '() sugar-scope)
            sc_argument_list_append outargs obj
            sc_argument_list_append outargs `none
            repeat rest...
        case (('case condv body...) rest...)
            do
                let tmpl = (sc_template_new ('anchor condv) fn-name)
                sc_argument_list_append outargs tmpl
                let scope = (Scope bodyscope)
                loop (expr types = (uncomma (condv as list)) void)
                    sugar-match expr
                    case ()
                        let body = (sc_expand (cons do body...) '() scope)
                        sc_template_set_body tmpl body
                        sc_argument_list_append outargs types
                        break;
                    case ((arg as Symbol) ': T)
                        sugar-error! ('anchor condv) "single typed parameter definition is missing trailing comma or semicolon"
                    case ((arg as Symbol) rest...)
                        if ('variadic? arg)
                            if (not (empty? rest...))
                                sugar-error! ('anchor condv) "variadic parameter must be in last place"
                        let param = (sc_parameter_new ('anchor condv) arg)
                        sc_template_append_parameter tmpl param
                        'set-symbol scope arg param
                        repeat rest...
                            sc_arguments_type_join types
                                ? ('variadic? arg) Variadic Unknown
                    case (((arg as Symbol) ': T) rest...)
                        if ('variadic? arg)
                            sugar-error! ('anchor condv) "a typed parameter can't be variadic"
                        let T = ((sc_expand T '() sugar-scope) as type)
                        let param = (sc_parameter_new ('anchor condv) arg)
                        sc_template_append_parameter tmpl param
                        'set-symbol scope arg param
                        repeat rest...
                            sc_arguments_type_join types T
                    default
                        sugar-error! ('anchor condv) "syntax: (parameter-name[: type], ...)"
            repeat rest...
        default
            sugar-match name...
            case (name as Symbol;)
                'set-symbol sugar-scope fn-name outtype
            default;
            return
                `(finalize-overloaded-fn outtype outargs)
                next

sugar from (src 'let params...)
    spice load-from (src keys...)
        let args = (sc_argument_list_new (sc_get_active_anchor))
        let count = ('argcount keys...)
        loop (i = 0)
            if (i == count)
                break;
            let key = ('getarg keys... i)
            sc_argument_list_append args
                `(getattr src key)
            i + 1
        args

    fn quotify (params)
        if (empty? params)
            return '()
        let entry rest = (decons params)
        entry as Symbol
        cons
            list sugar-quote entry
            quotify rest

    cons let
        .. params...
            list '=
                cons load-from src
                    quotify params...

run-stage;

define-sugar-block-scope-macro static-if
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
        sugar-scope

define-sugar-block-scope-macro sugar-if
    fn process (sugar-scope body next-expr)
        if false
            return '() next-expr
        let cond body = (decons body)
        let cond body = (sc_expand cond body sugar-scope)
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
                        process sugar-scope body next-next-expr
                    case 'else
                        _ body next-next-expr
                    default
                        _ '() next-expr
                else
                    _ '() next-expr
        if ((('typeof cond) == bool) and ('constant? cond))
            if (cond as bool)
                return body next-next-expr
            else
                return elseexpr next-next-expr
        sugar-error! ('anchor cond) "condition must be constant"
    let kw body = (decons expr)
    let body next-expr = (process sugar-scope body next-expr)
    return
        cons
            cons do body
            next-expr
        sugar-scope

define-sugar-block-scope-macro @@
    raises-compile-error;
    let kw body = (decons expr)
    let head = (kw as Symbol)
    let result next-expr =
        loop (body next-expr result = body next-expr '())
            if (empty? next-expr)
                compiler-error! "decorator is not applied to anything"
            let result =
                cons
                    if ((countof body) == 1)
                        '@ body
                    else
                        `body
                    result
            let follow-expr next-next-expr = (decons next-expr)
            if (('typeof follow-expr) != list)
                compiler-error! "decorator must be applied to expression"
            let kw body = (decons (follow-expr as list))
            let kw = (kw as Symbol)
            if (kw == head)
                # more decorators
                repeat body next-next-expr result
            else
                # terminating actual expression
                let newkw = (Symbol (.. "decorate-" (kw as string)))
                break
                    cons newkw follow-expr result
                    next-next-expr
    return
        cons result next-expr
        sugar-scope

define-sugar-block-scope-macro vvv
    raises-compile-error;
    let kw body = (decons expr)
    let head = (kw as Symbol)
    let result next-expr =
        loop (body next-expr result = body next-expr '())
            if (empty? next-expr)
                compiler-error! "expression decorator is not applied to anything"
            let result =
                cons
                    if ((countof body) == 1)
                        '@ body
                    else
                        `body
                    result
            let follow-expr next-next-expr = (decons next-expr)
            break
                cons 'decorate-* follow-expr result
                next-next-expr
    return
        cons result next-expr
        sugar-scope

define-sugar-macro decorate-*
    raises-compile-error;
    let expr decorators = (decons args)
    loop (in out = decorators expr)
        if (empty? in)
            break out
        let decorator in = (decons in)
        repeat in
            Value (cons decorator (list out))

define-sugar-macro decorate-fn
    raises-compile-error;
    let fnexpr decorators = (decons args)
    let kw name = (decons (fnexpr as list) 2)
    let result =
        loop (in out = decorators fnexpr)
            if (empty? in)
                break out
            let decorator in = (decons in)
            repeat in
                Value (cons decorator (list out))
    if (('typeof name) == Symbol)
        Value (list let name '= result)
    else
        result

let decorate-inline = decorate-fn

define-sugar-macro decorate-let
    raises-compile-error;
    let letexpr decorators = (decons args)
    let letexpr = (letexpr as list)
    let kw entry = (decons letexpr 2)
    if (('typeof entry) == list)
        # map form: wrap each arg
        let result =
            loop (in out = ('next letexpr) '())
                if (empty? in)
                    break out
                let entry in = (decons in)
                let k eq val = (decons (entry as list) 2)
                let result =
                    loop (in out = decorators val)
                        if (empty? in)
                            break out
                        let decorator in = (decons in)
                        repeat in
                            list (cons decorator out)
                repeat in
                    cons
                        cons k eq result
                        out
        cons let ('reverse result)
    else
        # unpack form: wrap all args
        let params values =
            loop (expr params = letexpr '())
                if (empty? expr)
                    compiler-error! "reimport form not supported for decorate-let"
                let val rest = (decons expr)
                if ((('typeof val) == Symbol) and ((val as Symbol) == '=))
                    break params rest
                _ rest (cons val params)
        let result =
            loop (in out = decorators (cons _ values))
                if (empty? in)
                    break out
                let decorator in = (decons in)
                repeat in
                    cons decorator (list out)
        loop (in out = params (list '= result))
            if (empty? in)
                break out
            let param params = (decons in)
            repeat params
                cons param out

define-sugar-scope-macro sugar-eval
    let subscope = (Scope sugar-scope)
    'set-symbol subscope 'sugar-scope sugar-scope
    return
        exec-module (Value args) subscope
        sugar-scope

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
    import-c = sc_import_c

run-stage;

#-------------------------------------------------------------------------------
# unlet
#-------------------------------------------------------------------------------

sugar unlet ((name as Symbol) names...)
    sc_scope_del_symbol sugar-scope name
    for name in names...
        let name = (name as Symbol)
        getattr sugar-scope name
        sc_scope_del_symbol sugar-scope name
    `[]

#-------------------------------------------------------------------------------
# fold iteration
#-------------------------------------------------------------------------------

# fold (<name> ... = <init> ...) for <name> ... in <expr>
sugar fold ((binding...) 'for expr...)
    fn split-until (expr token errmsg)
        loop (it params = expr '())
            if (empty? it)
                compiler-error! errmsg
            let sxat it = (decons it)
            let at = (sxat as Symbol)
            if (at == token)
                break it params
            _ it (cons sxat params)
    let it params = (split-until expr... 'in "'in' expected")
    let init foldparams = (split-until binding... '= "'=' expected")
    let generator-expr body = (decons it)
    let subscope = (Scope sugar-scope)
    spice-quote
        let iter start =
            (as [(sc_expand generator-expr '() subscope)] Generator);
    let next = ('unique Symbol "next")
    let bindings breakargs =
        loop (inp bindings breakargs = foldparams (cons '= start init) '())
            if (empty? inp)
                break (cons next bindings) breakargs
            let at next = (decons inp)
            _ next (cons at bindings) (cons at breakargs)
    let itercall =
        qq [iter]
            [inline] ()
                [(cons break breakargs)]
            [next]
    let letexpr =
        loop (inp letexpr = params (list '= itercall))
            if (empty? inp)
                break (cons let next letexpr)
            let at next = (decons inp)
            _ next (cons at letexpr)
    let repeatexpr = (cons repeat next breakargs)
    qq [loop] [bindings]
        [letexpr]
        [inline] repeat (...)
            [repeat] [next] ...
        [inline] continue () [repeatexpr]
        [repeat] [next]
            [(cons do body)]

#-------------------------------------------------------------------------------
# typedef
#-------------------------------------------------------------------------------

sugar typedef (name body...)
    let declaration? = (('typeof name) == Symbol)
    let typedecl =
        label got-typedecl
            if declaration?
                if (empty? body...)
                    # forward declaration - we build the type at syntax time
                    return
                        qq [let] [name] = [(typename (name as Symbol as string))]

                let symname = (name as Symbol)
                # see if we can find a forward declaration in the local scope
                try
                    let T = (getattr sugar-scope symname)
                    let T = (T as type)
                    assert (('opaque? T) and (('superof T) == typename))
                    # reuse type
                    merge got-typedecl `T
                except (err)

            let namestr =
                if declaration? `[(name as Symbol as string)]
                else name
            `[(qq [typename] [namestr])]

    let expr =
        loop (inp outp = body... '())
            sugar-match inp
            case ('< supertype rest...)
                repeat rest...
                    cons (list sc_typename_type_set_super 'this-type supertype) outp
            case (': storagetype rest...)
                repeat rest...
                    cons (list sc_typename_type_set_storage 'this-type storagetype typename-flag-plain) outp
            case (':: storagetype rest...)
                repeat rest...
                    cons (list sc_typename_type_set_storage 'this-type storagetype 0:u32) outp
            case ('do rest...)
                break
                    qq [do]
                        [let] this-type = [typedecl]
                        unquote-splice outp
                        [do]
                            unquote-splice rest...
                        this-type
            default
                break
                    qq [do]
                        [let] this-type = [typedecl]
                        unquote-splice outp
                        [let] scope =
                            [do]
                                unquote-splice inp
                                [locals];
                        [set-symbols-from-scope] this-type ('parent scope)
                        [set-symbols-from-scope] this-type scope
                        this-type
    if declaration?
        qq [let] [name] = [expr]
    else expr

#-------------------------------------------------------------------------------
# standard allocators
#-------------------------------------------------------------------------------

spice __init (target args...)
    let T = ('typeof target)
    let constructor =
        try
            getattr T '__init
        except (err)
            if (('argcount args...) > 0)
                compiler-error! "default constructor takes no arguments"
            return `(target = (nullof T))
    `(constructor target args...)

spice __init-copy (target source)
    let T = ('typeof target)
    let constructor =
        try
            getattr T '__init-copy
        except (err)
            return
                spice-quote
                    __init target
                    target = source
    `(constructor target source)

spice __delete (target)
    let T = ('typeof target)
    let destructor =
        try
            getattr T '__delete
        except (err)
            return `[]
    `(destructor target)

inline gen-allocator-sugar (name f)
    sugar "" (values...)
        spice local-copy-typed (T value)
            spice-quote
                let val = (ptrtoref (f T))
                __init-copy val value
                val
        spice local-copy (value)
            let T = ('typeof value)
            `(local-copy-typed T value)
        spice local-new (T args...)
            spice-quote
                let val = (ptrtoref (f T))
                __init val args...
                val
        sugar-match values...
        case (name '= value)
            qq [let name] = ([local-copy value])
        case (name ': T '= value)
            qq [let name] = ([local-copy-typed T value])
        case (name ': T args...)
            qq [let name] = ([local-new T] (unquote-splice args...))
        case (T args...)
            qq [local-new] [T] (unquote-splice args...)
        default
            compiler-error!
                .. "syntax: " name " <name> [: <type>] [= <value>]"

let local = (gen-allocator-sugar "local" alloca)
let new = (gen-allocator-sugar "new" malloc)
let global = (gen-allocator-sugar "global" private)

fn delete (value)
    free (reftoptr value)

#-------------------------------------------------------------------------------

define struct-dsl
    sugar : (name T)
        qq [=] field-types
            [cons] (list '[name] [T]) field-types

    define-infix> 70 :
    locals;

run-stage;

#-------------------------------------------------------------------------------
# C type support
#-------------------------------------------------------------------------------

# pointers
#-------------------------------------------------------------------------------

'set-symbols pointer
    __@ =
        inline (self index)
            ptrtoref (getelementptr self index)
    __getattr =
        inline (self key)
            getattr (ptrtoref self) key

# unions
#-------------------------------------------------------------------------------

'set-symbols CUnion
    __getattr = extractvalue
    __typecall =
        inline (cls)
            nullof cls

# native structs
#-------------------------------------------------------------------------------

'set-symbols Struct
    __getattr = extractvalue
    __typecall =
        spice "Struct-typecall" (cls args...)
            if ((cls as type) == Struct)
                compiler-error! "Struct type constructor not available"
            let cls = (cls as type)
            let argc = ('argcount args...)
            let st = ('storageof cls)
            loop (i result = 0 `(nullof st))
                if (i == argc)
                    break `(follow result cls)
                let k v = ('dekey ('getarg args... i))
                let k =
                    if (k == unnamed) i
                    else
                        sc_type_field_index cls k
                let ET = (sc_type_element_at cls k)
                let ET = (sc_strip_qualifiers ET)
                let v =
                    if (('pointer? ET) and ('refer? ('qualified-typeof v)))
                        `(imply (reftoptr v) ET)
                    else `(imply v ET)
                _ (i + 1) `(insertvalue result v k)

# C structs
#-------------------------------------------------------------------------------

'set-symbols CStruct
    __getattr = extractvalue
    __typecall =
        spice "CStruct-typecall" (cls args...)
            if ((cls as type) == CStruct)
                compiler-error! "CStruct type constructor is deprecated"
            let cls = (cls as type)
            let argc = ('argcount args...)
            loop (i result = 0 `(nullof cls))
                if (i == argc)
                    break result
                let k v = ('dekey ('getarg args... i))
                let k =
                    if (k == unnamed) i
                    else
                        sc_type_field_index cls k
                let ET = (sc_type_element_at cls k)
                let ET = (sc_strip_qualifiers ET)
                let v =
                    if (('pointer? ET) and ('refer? ('qualified-typeof v)))
                        `(imply (reftoptr v) ET)
                    else `(imply v ET)
                _ (i + 1) `(insertvalue result v k)

sugar struct (name body...)
    fn finalize-struct (T field-types)
        let numfields = (countof field-types)
        let fields = (alloca-array type numfields)
        for i field in (enumerate ('reverse field-types))
            let k T = (decons (field as list) 2)
            fields @ (usize i) = (sc_key_type (k as Symbol) (T as type))
        print T
        if (T < CUnion)
            'set-plain-storage T
                sc_union_type numfields fields
        elseif (T < CStruct)
            'set-plain-storage T
                sc_tuple_type numfields fields
        elseif (T < Struct)
            'set-plain-storage T
                sc_tuple_type numfields fields
        else
            compiler-error!
                .. "type " (repr T) " must have Struct, CStruct or CUnion supertype"
        T

    if (('typeof name) == Symbol)
        if (empty? body...)
            # forward declaration
            return
                qq [typedef] [name]

    let supertype body... =
        sugar-match body...
        case ('union rest...)
            _ `CUnion rest...
        case ('plain rest...)
            _ `CStruct rest...
        case ('< supertype rest...)
            _ supertype rest...
        default
            _ `Struct body...

    qq [typedef] [name] < [supertype] do
        [local] field-types = [`(ptrtoref (alloca list))]
        field-types = [null]
        [using] [struct-dsl]
        [let] scope =
            [do]
                unquote-splice body...
                [locals];
        [set-symbols-from-scope] this-type ('parent scope)
        [set-symbols-from-scope] this-type scope
        [finalize-struct] this-type field-types

# enums
#-------------------------------------------------------------------------------

'set-symbols CEnum
    __== = (box-binary-op (single-binary-op-dispatch icmp==))
    __!= = (box-binary-op (single-binary-op-dispatch icmp!=))
    __imply =
        box-cast
            fn "CEnum-imply" (vT T expr)
                if (T == i32)
                    return `(bitcast expr T)
                compiler-error! "unsupported type"
    __rimply =
        box-cast
            fn "CEnum-imply" (vT T expr)
                if (vT == i32)
                    return `(bitcast expr T)
                compiler-error! "unsupported type"

sugar enum (name values...)
    spice make-enum (name vals...)
        let T = (typename (name as string))
        inline make-enumval (anchor val)
            sc_const_int_new anchor T
                sext (as val i32) u64
        'set-super T CEnum
        'set-plain-storage T i32
        let count = ('argcount vals...)
        loop (i nextval = 0 0)
            if (i >= count)
                break;
            let arg = ('getarg vals... i)
            let anchor = ('anchor arg)
            let key val = ('dekey arg)
            #print arg key val
            if (not ('constant? val))
                sugar-error! anchor "all enum values must be constant"
            _ (i + 1)
                if (key == unnamed)
                    # auto-numerical
                    'set-symbol T (as val Symbol) (make-enumval anchor nextval)
                    nextval + 1
                else
                    'set-symbol T key (make-enumval anchor val)
                    (as val i32) + 1
        T

    fn convert-body (body)
        if false
            # hint return type
            return '()
        let expr body = (decons body)
        cons
            if (('typeof expr) == Symbol)
                Value (list sugar-quote expr)
            else expr
            if (empty? body)
                '()
            else
                convert-body body

    let newbody = (convert-body values...)
    if (('typeof name) == Symbol)
        let namestr = (name as Symbol as string)
        list let name '=
            cons make-enum namestr newbody
    else
        cons make-enum name newbody

run-stage;

#-------------------------------------------------------------------------------

set-globals! (__this-scope)

#-------------------------------------------------------------------------------
# REPL
#-------------------------------------------------------------------------------

# REPL and main loop must stay in core.sc to make sure that they remain
    accessible even when there's no module loading support (for whatever reason)

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
        loop (i s = 0:usize "")
            if (i == n)
                return s
            repeat (i + 1:usize)
                .. s c

    fn leading-spaces (s)
        let len = (countof s)
        loop (i = 0:usize)
            if (i == len)
                return s
            let c = (@ s i)
            if (c != (char " "))
                let s = (sc_string_buffer s)
                return (sc_string_new s i)
            repeat (i + 1:usize)

    fn blank? (s)
        let len = (countof s)
        loop (i = 0:usize)
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
    loop (preload cmdlist counter eval-scope = "" "" 0 eval-scope)
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
            loop (key = unnamed)
                let key value = ('next local-scope key)
                if (key != unnamed)
                    if (key != tmp)
                        'set-symbol eval-scope key value
                    repeat key
                break;
            let count =
                va-lfold 0
                    inline (key value k)
                        let idstr = (make-idstr (counter + k))
                        if (not (none? value))
                            'set-symbol eval-scope (Symbol idstr) (Value value)
                            print idstr "="
                                repr value
                            k + 1
                        else k
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
                            list sugar-set-scope! eval-scope
                            list let tmp '=
                                cons embed
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
                    'format exc
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
    loop (i = 1)
        if (i >= argc)
            break;
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
        elseif ((load sourcepath) == "")
            store arg sourcepath
            # remainder is passed on to script
            break;
        k
    let sourcepath = (load sourcepath)
    if (sourcepath == "")
        read-eval-print-loop;
    else
        let scope =
            Scope (globals)
        'set-docstring! scope unnamed ""
        'set-symbols scope
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
                'format err
        exit 0

raises-compile-error;
run-main;

return;