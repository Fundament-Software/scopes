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

""""A string containing the folder path to the compiler environment. Typically
    the compiler environment is the folder that contains the ``bin`` folder
    containing the compiler executable.
let compiler-dir
""""A string constant containing the file path to the compiler executable.
let compiler-path
""""A string constant indicating the time and date the compiler was built.
let compiler-timestamp
""""A boolean constant indicating if the compiler was built in debug mode.
let debug-build?
""""A string constant indicating the operating system the compiler was built
    for. It equals to ``"linux"`` for Linux builds, ``"windows"`` for Windows
    builds, ``"macos"`` for macOS builds and ``"unknown"`` otherwise.
let operating-system
""""A constant of type `i32` indicating the maximum number of recursions
    permitted for an inline. When this number is exceeded, an error is raised
    during typechecking. Currently, the limit is set at 64 recursions. This
    restriction has been put in place to prevent the compiler from overflowing
    its stack memory.
let unroll-limit

let default-target-triple = (sc_default_target_triple)

# square list expressions are ast unquotes by default
let square-list = spice-unquote-arguments

# first we alias u64 to the integer type that can hold a pointer
let intptr = u64

inline swap (a b)
    """"safely exchanges the contents of two references
    let tmp = (deref (dupe b))
    assign (dupe a) b
    assign tmp a
    return;

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

fn error (msg)
    hide-traceback;
    raise (sc_error_new msg)

fn error@ (anchor traceback-msg error-msg)
    """"usage example::
            error@ ('anchor value) "while checking parameter" "error in value"
    hide-traceback;
    let err = (sc_error_new error-msg)
    sc_error_append_calltrace err (sc_valueref_tag anchor `traceback-msg)
    raise err

fn error@+ (error anchor traceback-msg)
    """"usage example::
            except (err)
                error@+ err ('anchor value) "while processing stream"
    hide-traceback;
    sc_error_append_calltrace error (sc_valueref_tag anchor `traceback-msg)
    raise error

# print an unboxing error given two types
fn unbox-verify (value wantT)
    let haveT = (sc_value_type value)
    if (ptrcmp!= haveT wantT)
        hide-traceback;
        error@
            sc_value_anchor value
            "while trying to unbox value"
            sc_string_join "can't unbox value of type "
                sc_string_join
                    sc_value_repr (box-pointer haveT)
                    sc_string_join " as value of type "
                        sc_value_repr (box-pointer wantT)
    if (sc_value_is_constant value)
    else
        hide-traceback;
        error@
            sc_value_anchor value
            "while trying to unbox value"
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
            error
                sc_string_join "at least "
                    sc_string_join (sc_value_repr (box-integer mincount))
                        sc_string_join " argument(s) expected, got "
                            sc_value_repr (box-integer count)
    if (icmp>=s maxcount 0)
        if (icmp>s count maxcount)
            error
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
let
    ellipsis-symbol = (sc_symbol_new "...")
    list-handler-symbol = (sc_symbol_new "#list")
    symbol-handler-symbol = (sc_symbol_new "#symbol")
    typed-symbol-handler-symbol = (sc_symbol_new "#typed-symbol")

# execute until here and treat the remainder as a new translation unit
run-stage; # 1

# we can now access TypeArrayPointer as a compile time value
let void =
    sc_arguments_type 0 (nullof TypeArrayPointer)

fn build-typify-function (f)
    let types = (alloca-array type 1:usize)
    store Value (getelementptr types 0)
    let types = (bitcast types TypeArrayPointer)
    let result = (sc_compile (sc_typify f 1 types) compile-flag-cache)
    let result-type = (sc_value_type result)
    if (ptrcmp!= result-type SpiceMacroFunction)
        error
            sc_string_join "spice macro must have type "
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
            let typecount = (sub argcount 1)
            spice-quote
                let types = (alloca-array type typecount)
                spice-unquote
                    let body = (sc_expression_new)
                    loop (i j = 1 0)
                        if (icmp== i argcount)
                            break;
                        let ty = (sc_getarg args i)
                        if (ptrcmp!= (sc_value_type ty) type)
                            error "type expected"
                        sc_expression_append body
                            `(store ty (getelementptr types j))
                        _ (add i 1) (add j 1)
                    body
                sc_typify src_fn typecount (bitcast types TypeArrayPointer)

        build-typify-function typify

let static-typify =
    do
        fn static-typify (args)
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

        build-typify-function static-typify

run-stage; # 2

let spice-macro-verify-signature =
    static-typify (fn "spice-macro-verify-signature" (f)) SpiceMacroFunction

inline function->SpiceMacro (f)
    spice-macro-verify-signature f
    bitcast f SpiceMacro

# take closure l, typify and compile it and return a function of SpiceMacro type
inline spice-macro (l)
    function->SpiceMacro (static-typify l Value)

inline box-spice-macro (l)
    sc_valueref_tag
        sc_value_anchor `l
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
            let anchor = (sc_value_anchor args)
            loop (i ret = 2 init)
                if (icmp== i argcount)
                    break ret
                let arg =
                    sc_getarg args i
                let k = (sc_type_key (sc_value_qualified_type arg))
                let v = (sc_keyed_new unnamed arg)
                _ (add i 1)
                    if use-indices
                        `(f [(sub i 2)] k v ret)
                    else
                        sc_valueref_tag anchor `(f k v ret)
        _
            spice-macro (fn "va-lfold" (args) (va-lfold args false))
            spice-macro (fn "va-lifold" (args) (va-lfold args true))

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
                let v = (sc_valueref_tag
                    (sc_value_anchor arg) (sc_keyed_new unnamed arg))
                _ i
                    sc_valueref_tag
                        sc_value_anchor arg
                        if use-indices
                            `(f [(sub i 2)] k v ret)
                        else
                            `(f k v ret)
        _
            spice-macro (fn "va-rfold" (args) (va-rfold args false))
            spice-macro (fn "va-rifold" (args) (va-rfold args true))

fn type< (T superT)
    sc_type_is_superof superT T

fn type> (superT T)
    sc_type_is_superof superT T

fn type<= (T superT)
    bor (type< T superT) (ptrcmp== T superT)

fn type>= (superT T)
    bor (type< T superT) (ptrcmp== T superT)

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
    fn (args) (compare-type args (static-typify f type type))

let static-error =
    box-spice-macro
        fn "static-error" (args)
            returning Value
            let argcount = (sc_argcount args)
            verify-count argcount 1 1
            let msg = (sc_getarg args 0)
            let msg = (unbox-pointer msg string)
            hide-traceback;
            raise (sc_error_new msg)

let elementof =
    box-spice-macro
        fn "elementof" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 2
            let self = (unbox-pointer (sc_getarg args 0) type)
            let index =
                if (icmp== argcount 2)
                    let arg = (sc_getarg args 1)
                    if (ptrcmp== (sc_value_type arg) Symbol)
                        let sym = (unbox-integer arg Symbol)
                        sc_type_field_index self sym
                    else
                        unbox-integer arg i32
                else 0
            hide-traceback;
            `[(sc_type_element_at self index)]

inline sc_argument_list_map_new (N mapf)
    let values = (alloca-array Value N)
    loop (i = 0)
        if (icmp== i N)
            break;
        store (sc_identity (mapf i)) (getelementptr values i)
        add i 1
    sc_argument_list_new N values

inline sc_argument_list_map_filter_new (maxN mapf)
    let values = (alloca-array Value maxN)
    loop (i k = 0 0)
        if (icmp== i maxN)
            break
                sc_argument_list_new k values
        let ok value = (mapf i)
        if ok
            store (sc_identity value) (getelementptr values k)
            repeat (add i 1) (add k 1)
        else
            repeat (add i 1) k

fn sc_argument_list_join (a b)
    let A = (sc_argcount a)
    let B = (sc_argcount b)
    let N = (add A B)
    let values = (alloca-array Value N)
    loop (i = 0)
        if (icmp== i A)
            break;
        store (sc_getarg a i) (getelementptr values i)
        add i 1
    loop (i = A)
        if (icmp== i N)
            break;
        store (sc_getarg b (sub i A)) (getelementptr values i)
        add i 1
    sc_argument_list_new N values

let argumentsof =
    box-spice-macro
        fn "argumentsof" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 1
            let self = (unbox-pointer (sc_getarg args 0) type)
            hide-traceback;
            let count = (sc_arguments_type_argcount self)
            sc_argument_list_map_new count
                inline (i)
                    `[(sc_arguments_type_getarg self i)]

let elementsof =
    box-spice-macro
        fn "elementsof" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 1
            let self = (unbox-pointer (sc_getarg args 0) type)
            hide-traceback;
            let count = (sc_type_countof self)
            sc_argument_list_map_new count
                inline (i)
                    `[(sc_type_element_at self i)]

let locationof =
    box-spice-macro
        fn "locationof" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 1
            let self = (sc_getarg args 0)
            `[(sc_global_location self)]

let bindingof =
    box-spice-macro
        fn "locationof" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 1
            let self = (sc_getarg args 0)
            `[(sc_global_binding self)]

let storagecast =
    box-spice-macro
        fn "storagecast" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 1
            let self = (sc_getarg args 0)
            let T = (sc_type_storage (sc_value_type self))
            return (sc_valueref_tag (sc_value_anchor args) `(bitcast self T))

"""".. spice:: (&? value)

       Returns `true` if `value` is a reference, otherwise `false`.
let &? =
    box-spice-macro
        fn "&?" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 1
            let self = (sc_getarg args 0)
            let selfT =
                if (band
                        (ptrcmp== (sc_value_type self) type)
                        (sc_value_is_constant self))
                    unbox-pointer self type
                else (sc_value_qualified_type self)
            let isref = (sc_type_is_refer selfT)
            `isref

# typecall
sc_type_set_symbol type '__call
    box-spice-macro
        fn "type-call" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 -1
            let self = (sc_getarg args 0)
            let T = (unbox-pointer self type)
            let f = (sc_type_at T '__typecall)
            return (sc_valueref_tag (sc_value_anchor args) `(f args))

# methodcall
sc_type_set_symbol Symbol '__call
    box-spice-macro
        fn "symbol-call" (args)
            hide-traceback;
            let argcount = (sc_argcount args)
            verify-count argcount 2 -1
            let self = (sc_getarg args 1)
            let T = (sc_value_type self)
            let f = (sc_type_at T '__methodcall)
            sc_valueref_tag (sc_value_anchor args) `(f args)

# methodcall for any type
sc_type_set_symbol typename '__methodcall
    box-spice-macro
        fn "typename-methodcall" (args)
            hide-traceback;

            fn resolve-method (self symval)
                hide-traceback;
                let sym = (unbox-symbol symval Symbol)
                let T = (sc_value_type self)
                try (return (sc_type_at T sym))
                except (err)
                    # if calling method of type, try typemethod
                    if (ptrcmp== T type)
                        if (sc_value_is_constant self)
                            let self = (unbox-pointer self type)
                            return (sc_type_at self sym)
                    raise err

            let argcount = (sc_argcount args)
            verify-count argcount 2 -1
            let symval = (sc_getarg args 0)
            let self = (sc_getarg args 1)
            let method = (resolve-method self symval)
            let arglist = (sc_extract_argument_list_new args 1)
            sc_valueref_tag (sc_value_anchor args) `(method arglist)

do
    fn get-key-value-args (args)
        let argcount = (sc_argcount args)
        verify-count argcount 2 3
        let self = (sc_getarg args 0)
        if (icmp== argcount 3)
            let key = (sc_getarg args 1)
            if (sc_value_is_constant key)
                if (icmp== (unbox-symbol key Symbol) unnamed)
                    hide-traceback;
                    error "value is missing key"
            let value = (sc_getarg args 2)
            return self key value
        else
            let arg = (sc_getarg args 1)
            let key = (sc_type_key (sc_value_qualified_type arg))
            if (icmp== key unnamed)
                hide-traceback;
                error "value is missing key"
            let arg = (sc_keyed_new unnamed arg)
            return self (box-symbol key) arg

    inline gen-key-type-set (selftype fset)
        box-spice-macro
            fn "set-symbol" (args)
                hide-traceback;
                let self key value = (get-key-value-args args)
                `(fset self key value)

    inline gen-key-type-define (selftype fset)
        box-spice-macro
            fn "define-symbol" (args)
                let self key value = (get-key-value-args args)
                if (sc_value_is_constant self)
                    if (sc_value_is_constant key)
                        if (sc_value_is_pure value)
                            let self = (unbox-pointer self selftype)
                            let key = (unbox-symbol key Symbol)
                            fset self key value
                            return `()
                error "all arguments must be constant"

    fn get-key-value-scope-args (args)
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
            if (icmp== key unnamed)
                hide-traceback;
                error "value is missing key"
            let arg = (sc_keyed_new unnamed arg)
            return self (box-symbol key) arg

    inline gen-key-scope-set (selftype fset)
        box-spice-macro
            fn "set-symbol" (args)
                hide-traceback;
                let self key value = (get-key-value-scope-args args)
                `(fset self `key value)

    inline gen-key-scope-define (selftype fset)
        box-spice-macro
            fn "define-symbol" (args)
                let self key value = (get-key-value-scope-args args)
                if (sc_value_is_constant self)
                    if (sc_value_is_constant key)
                        if (sc_value_is_pure value)
                            let self = (unbox-pointer self selftype)
                            let result = (fset self key value)
                            return `result
                        else
                            error "value argument must be constant"
                    else
                        error "key argument must be constant"
                else
                    error "scope must be constant"

    # quick assignment of type attributes
    sc_type_set_symbol type 'set-symbol (gen-key-type-set type sc_type_set_symbol)
    sc_type_set_symbol type 'define-symbol (gen-key-type-define type sc_type_set_symbol)
    sc_type_set_symbol Scope 'bind (gen-key-scope-set Scope sc_scope_bind)
    sc_type_set_symbol Scope 'define (gen-key-scope-define Scope sc_scope_bind)

# static pointer type constructor
sc_type_set_symbol pointer '__typecall
    box-spice-macro
        fn "pointer.__typecall" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 2
            let cls = (sc_getarg args 0)
            if (ptrcmp== (unbox-pointer cls type) pointer)
                verify-count argcount 2 2
                let self = (sc_getarg args 1)
                let T = (unbox-pointer self type)
                `[(sc_pointer_type T pointer-flag-non-writable unnamed)]
            else
                # default constructor
                `(nullof cls)

# dynamic pointer type constructor
sc_type_set_symbol pointer 'type
    box-pointer
        inline "pointer.type" (T)
            sc_pointer_type T pointer-flag-non-writable unnamed

inline aggregate-type-constructor (start f)
    box-spice-macro
        fn "aggregate-type-constructor" (args)
            let argcount = (sc_argcount args)
            verify-count argcount start -1
            let pcount = (sub argcount start)
            let types = (alloca-array type pcount)
            loop (i = start)
                if (icmp== i argcount)
                    break;
                let arg = (sc_getarg args i)
                let k = (sc_type_key (sc_value_qualified_type arg))
                let arg = (unbox-pointer arg type)
                store (sc_key_type k arg)
                    getelementptr types (sub i start)
                add i 1
            sc_valueref_tag (sc_value_anchor args)
                `[(f pcount types)]

inline runtime-aggregate-type-constructor (f)
    box-spice-macro
        fn "runtime-aggregate-type-constructor" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 0 -1
            spice-quote
                let types = (alloca-array type argcount)
                spice-unquote
                    let body = (sc_expression_new)
                    loop (i = 0)
                        if (icmp== i argcount)
                            break;
                        let arg = (sc_getarg args i)
                        let k = (sc_type_key (sc_value_qualified_type arg))
                        let arg = (sc_keyed_new unnamed arg)
                        if (ptrcmp!= (sc_value_type arg) type)
                            error "type expected"
                        sc_expression_append body
                            `(store
                                (sc_key_type k arg)
                                (getelementptr types i))
                        add i 1
                    body
                f argcount types

# static tuple type constructor
sc_type_set_symbol tuple '__typecall (aggregate-type-constructor 1 sc_tuple_type)
sc_type_set_symbol tuple 'packed (aggregate-type-constructor 1 sc_packed_tuple_type)
let union-storageof = (aggregate-type-constructor 0 sc_union_storage_type)

# dynamic tuple type constructor
sc_type_set_symbol tuple 'type (runtime-aggregate-type-constructor sc_tuple_type)
sc_type_set_symbol tuple 'packed-type (runtime-aggregate-type-constructor sc_packed_tuple_type)
let union-storage-type = (runtime-aggregate-type-constructor sc_union_storage_type)

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

# static function type constructor
sc_type_set_symbol function '__typecall
    box-spice-macro
        fn "function.__typecall" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 2 -1
            let pcount = (sub argcount 2)
            let rtype = (sc_getarg args 1)
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

# dynamic function type constructor
sc_type_set_symbol function 'type
    box-spice-macro
        fn "function.type" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 -1
            let pcount = (sub argcount 1)
            let rtype = (sc_getarg args 0)
            spice-quote
                let types = (alloca-array type pcount)
                spice-unquote
                    let expr = (sc_expression_new)
                    loop (i = 1)
                        if (icmp== i argcount)
                            break;
                        let arg = (sc_getarg args i)
                        sc_expression_append expr
                            `(store arg (getelementptr types [(sub i 1)]))
                        add i 1
                    expr
                sc_function_type rtype pcount types

let raises =
    box-spice-macro
        fn "raises" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 2 2
            let self = (sc_getarg args 0)
            let except_type = (sc_getarg args 1)
            let T = (unbox-pointer self type)
            let exceptT = (unbox-pointer except_type type)
            `[(sc_function_type_raising T exceptT)]

sc_type_set_symbol type 'raises
    box-spice-macro
        fn "'raises function" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 2 2
            let self = (sc_getarg args 0)
            let except_type = (sc_getarg args 1)
            `(sc_function_type_raising self except_type)

# closure constructor
#sc_type_set_symbol Closure '__typecall
    box-pointer
        inline (cls func frame)
            sc_closure_new func frame

# symbol constructor
sc_type_set_symbol Symbol '__typecall
    box-pointer
        spice-macro
            fn (args)
                let argcount = (sc_argcount args)
                verify-count argcount 2 2
                let value = (sc_getarg args 1)
                if (sc_value_is_constant value)
                    `[(sc_symbol_new (unbox-pointer value string))]
                else
                    `(sc_symbol_new value)

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
            error "arguments must be constant"

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
            error "arguments must be constant"

let static-branch =
    spice-macro
        fn (args)
            let argcount = (sc_argcount args)
            verify-count argcount 3 3
            let cond = (sc_getarg args 0)
            let thenf = (sc_getarg args 1)
            let elsef = (sc_getarg args 2)
            if (sc_value_is_constant cond)
            else
                hide-traceback;
                error@ (sc_value_anchor cond) "while checking condition"
                    "condition must be constant"
            let value = (unbox-integer cond bool)
            sc_valueref_tag (sc_value_anchor args) `([(? value thenf elsef)])

sc_type_set_symbol Value '__typecall
    box-spice-macro
        fn (args)
            raising Error
            let args = (sc_getarglist args 1)
            spice-quote
                spice-quote
                    spice-unquote args

let __unbox =
    spice-macro
        fn (args)
            let argc = (sc_argcount args)
            verify-count argc 2 2
            let value = (sc_getarg args 0)
            let T = (sc_getarg args 1)
            let T = (unbox-pointer T type)
            let VT = (sc_value_type value)
            sc_value_unwrap T value
let
    type== = (spice-macro (type-comparison-func ptrcmp==))
    type!= = (spice-macro (type-comparison-func ptrcmp!=))
    type<= = (spice-macro (type-comparison-func type<=))
    type>= = (spice-macro (type-comparison-func type>=))
    type< = (spice-macro (type-comparison-func type<))
    type> = (spice-macro (type-comparison-func type>))

let NullType = (sc_typename_type "NullType" typename)
let Accessor = (sc_typename_type "Accessor" typename)
sc_typename_type_set_storage Accessor (sc_type_storage Closure) typename-flag-plain

let cons =
    spice-macro
        fn (args)
            let argc = (sc_argcount args)
            verify-count argc 1 -1
            let block = (sc_expression_new)
            let last = (sc_getarg args (sub argc 1))
            sc_expression_append block last
            loop (i last = (sub argc 1) last)
                if (icmp== i 0)
                    break block
                let i = (sub i 1)
                let arg = (sc_getarg args i)
                let anchor = (sc_value_anchor arg)
                # if both arguments are constant, produce a constant list
                if (sc_value_is_constant last)
                    if (sc_value_is_constant arg)
                        let last = (sc_list_cons arg (unbox-pointer last list))
                        let last = (sc_valueref_tag anchor `last)
                        sc_expression_append block last
                        repeat i last
                let T = (sc_value_type arg)
                let arg =
                    if (ptrcmp== T Value) arg
                    elseif (sc_value_is_pure arg) ``arg
                    else (sc_value_wrap T arg)
                let last =
                    sc_valueref_tag anchor `(sc_list_cons arg last)
                sc_expression_append block last
                _ i last

let list-constructor =
    spice-macro
        fn (args)
            raising Error
            let argc = (sc_argcount args)
            let anchor = (sc_value_anchor args)
            let newargs =
                sc_argument_list_map_new argc
                    inline (i)
                        let i = (add i 1)
                        if (icmp== i argc)
                            sc_valueref_tag anchor `'()
                        else
                            sc_getarg args i
            sc_valueref_tag anchor `(cons newargs)

let decons =
    spice-macro
        fn (args)
            raising Error
            let argc = (sc_argcount args)
            verify-count argc 1 2
            let self = (sc_getarg args 0)
            let count =
                if (icmp== argc 2) (unbox-integer (sc_getarg args 1) i32)
                else 1
            let block = (sc_expression_new)
            let argcount = (add count 1)
            let retargs = (alloca-array Value argcount)
            loop (i next = 0 self)
                let expr = `(sc_list_decons next)
                sc_expression_append block expr
                spice-quote
                    let at next = expr
                store at (getelementptr retargs i)
                let i = (add i 1)
                if (icmp== i count)
                    store next (getelementptr retargs i)
                    sc_expression_append block
                        sc_argument_list_new argcount retargs
                    break block
                _ i next

run-stage; # 3

'define-symbol type 'set-symbols
    inline "set-symbols" (self values...)
        va-lfold none
            inline (key value)
                'set-symbol self key value
            values...
'define-symbol type 'define-symbols
    inline "define-symbols" (self values...)
        va-lfold none
            inline (key value)
                'define-symbol self key value
            values...

'define-symbol Scope 'bind-symbols
    inline "bind-symbols" (self values...)
        va-lfold self
            inline (key value self)
                'bind self key value
            values...
'define-symbol Scope 'define-symbols
    inline "define-symbols" (self values...)
        va-lfold self
            inline (key value self)
                'define self key value
            values...

'define-symbols Value
    constant? = sc_value_is_constant
    pure? = sc_value_is_pure
    kind = sc_value_kind
    tag =
        inline (self anchor)
            sc_valueref_tag anchor self
    none? = (static-typify Value-none? Value)
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
            _ k (sc_keyed_new unnamed self)

'define-symbols Scope
    @ = sc_scope_at
    local@ = sc_scope_local_at
    next = sc_scope_next
    next-deleted = sc_scope_next_deleted
    docstring = sc_scope_docstring
    module-docstring = sc_scope_module_docstring
    parent = sc_scope_get_parent
    unparent = sc_scope_unparent
    reparent = sc_scope_reparent
    bind-with-docstring = sc_scope_bind_with_docstring
    unbind = sc_scope_unbind

'define-symbols string
    join = sc_string_join
    match? = sc_string_match

'define-symbols Error
    format = sc_format_error
    dump = sc_dump_error
    append =
        inline "append" (self anchor traceback-msg)
            sc_error_append_calltrace self
                sc_valueref_tag anchor `traceback-msg
            self

'define-symbols list
    __countof = sc_list_count
    join = sc_list_join
    @ = sc_list_at
    next = sc_list_next
    decons = decons
    reverse = sc_list_reverse
    dump = sc_list_dump

'define-symbols type
    bitcount = sc_type_bitcountof
    signed? = sc_integer_type_is_signed
    strip-qualifiers = sc_strip_qualifiers
    plain? = sc_type_is_plain
    element@ = sc_type_element_at
    element-count = sc_type_countof
    unsized? = sc_type_is_unsized
    storageof = sc_type_storage
    kind = sc_type_kind
    sizeof = sc_type_sizeof
    alignof = sc_type_alignof
    offsetof = sc_type_offsetof
    @ = sc_type_at
    local@ = sc_type_local_at
    opaque? = sc_type_is_opaque
    string = sc_type_string
    superof = sc_typename_type_get_super
    docstring = sc_type_get_docstring
    set-docstring = sc_type_set_docstring
    set-opaque =
        inline (type)
            sc_typename_type_set_opaque type
    set-storage =
        inline (type storage-type)
            sc_typename_type_set_storage type storage-type 0:u32
    set-plain-storage =
        inline (type storage-type)
            sc_typename_type_set_storage type storage-type typename-flag-plain
    return-type = sc_function_type_return_type
    keyof = sc_type_key
    key-type =
        inline (self key)
            sc_key_type key self
    unique-type = sc_unique_type
    view-type =
        inline (self id)
            static-branch (none? id)
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
    mutable& =
        fn (cls)
            sc_refer_type cls
                band (sc_refer_flags cls)
                    bxor pointer-flag-non-writable -1:u64
                sc_refer_storage_class cls
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
    refer->pointer-type =
        fn "refer->pointer-type" (cls)
            sc_pointer_type
                sc_strip_qualifiers cls
                sc_refer_flags cls
                sc_refer_storage_class cls
    pointer->refer-type =
        fn "pointer->refer-type" (cls)
            sc_refer_type
                sc_type_element_at cls 0
                sc_pointer_type_get_flags cls
                sc_pointer_type_get_storage_class cls

let mutable =
    spice-macro
        fn (args)
            let argc = (sc_argcount args)
            verify-count argc 1 2
            if (icmp== argc 1)
                let self = (sc_getarg args 0)
                let T = (unbox-pointer self type)
                if ('refer? T)
                    return `[('mutable& T)]
                else
                    return `[('mutable T)]
            elseif (ptrcmp== (unbox-pointer (sc_getarg args 0) type) pointer)
                let self = (sc_getarg args 1)
                let T = (unbox-pointer self type)
                let T = (sc_pointer_type T pointer-flag-non-writable unnamed)
                return `[('mutable T)]
            error "syntax: (mutable pointer-type) or (mutable pointer type)"

let protect =
    spice-macro
        fn (args)
            let argc = (sc_argcount args)
            verify-count argc 1 1
            let self = (sc_getarg args 0)
            let T = (sc_value_type self)
            if (type< T pointer)
                return `(bitcast self [('immutable T)])
            error "syntax: (protect pointer-value)"

let rawstring = (pointer i8)

# cheap version of `not` - to be replaced further down
inline not (value)
    bxor value true

# supertype for unique structs
let Struct = (sc_typename_type "Struct" typename)
sc_typename_type_set_opaque Struct

""""Generators provide a protocol for iterating the contents of containers and
    enumerating sequences. They are primarily used by `for` and `fold`, but can
    also be used separately.

    Each generator instance is equivalent to a closure that when called returns
    four functions:

    * A function ``state... <- fn start ()`` which returns the initial state of
      the generator as an arbitrary number of arbitrarily typed values. The
      initially returned state defines the format of the generators internal
      state.
    * A function ``bool <- fn valid? (state...)`` which takes the current
      generator state and returns `true` when the generator can resolve the
      state to a collection item, otherwise `false`, indicating that the
      generator has been depleted.
    * A function ``value... <- fn at (state...)`` which takes the current
      generator state and returns the collection item this state maps to. The
      function may not be called for a state for which ``valid?`` has reported
      to be depleted.
    * A function ``state... <- fn next (state...)`` which takes the current
      generator state and returns the state mapping to the next item in the
      collection. The new state must have the same type signature as the
      previous state. The function may not be called for a state for which
      ``valid?`` has reported to be depleted.

    It is allowed to call any of these functions multiple times with any valid
    state, effectively restarting the Generator at an arbitrary point, as
    Generators are not expected to have side effects. In controlled
    circumstances a Generator may choose to be impure, but should be documented
    accordingly.

    Here is a typical pattern for constructing a generator::

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

    The generator can then be subsequently used like this::

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

let Generator = (sc_typename_type "Generator" typename)
'set-plain-storage Generator ('storageof Closure)

# collector type
let Collector = (sc_typename_type "Collector" typename)
'set-plain-storage Collector ('storageof Closure)

# syntax macro type
let SugarMacro = (sc_typename_type "SugarMacro" typename)
let SugarMacroFunction =
    pointer
        raises
            function (Arguments list Scope) list Scope
            Error
'set-plain-storage SugarMacro SugarMacroFunction

# any extraction

inline unbox (value T)
    hide-traceback;
    unbox-verify value T
    __unbox value T

fn value-as (vT T expr)
    if true
        return `(unbox expr T)
    error "unsupported type"

'define-symbols Value
    __as =
        inline (vT T)
            inline "Value-as" (self) (unbox self T)
    __rimply =
        inline (vT T)
            inline "Value-rimply" (self) `self

inline spice-cast-macro (f)
    """"to be used for __as, __ras, __imply and __rimply
        returns a callable converter (f value) that performs the cast or
        no arguments if the cast can not be performed.
    spice-macro
        fn (args)
            raising Error
            let argc = (sc_argcount args)
            verify-count argc 2 2
            let source-type = (unbox-pointer (sc_getarg args 0) type)
            let target-type = (unbox-pointer (sc_getarg args 1) type)
            f source-type target-type

inline spice-converter-macro (f)
    """"to be used for converter that need to do additional
        dispatch, e.g. do something else when the value is a constant
        returns a quote that performs the cast (f value T)
    spice-macro
        fn (args)
            raising Error
            let argc = (sc_argcount args)
            verify-count argc 2 2
            let self = (sc_getarg args 0)
            let _T = (sc_getarg args 1)
            let T = (unbox-pointer _T type)
            try
                f self T
            except (err)
                hide-traceback;
                error@+ err (sc_value_anchor self)
                    sc_string_join "while attempting to convert argument to " (sc_value_repr _T)

'set-symbols SpiceMacro
    __rimply =
        box-pointer
            spice-cast-macro
                fn (vT T)
                    if (ptrcmp== vT SpiceMacroFunction)
                        return `(inline (self) (bitcast self T))
                    elseif (ptrcmp== vT Closure)
                        return `spice-macro
                    `()

# integer casting

fn integer-tobool (args)
    raising Error
    let argc = (sc_argcount args)
    verify-count argc 1 1
    let self = (sc_getarg args 0)
    if ('constant? self)
        let result = (icmp!= (sc_const_int_extract self) 0:u64)
        return `result
    let T = ('typeof self)
    return `(icmp!= self (nullof T))

# as of yet unused
inline safe-integer-cast (self T)
    let selfT = ('typeof self)
    if ('constant? self)
    error
        sc_string_join
            "can not implicitly convert non-constant integer of type "
            sc_value_repr `selfT

let static-integer->integer =
    spice-converter-macro
        inline (self T)
            let selfT = ('typeof self)
            # allow non-destructive conversions
            let selfST = ('storageof selfT)
            let u64val = (sc_const_int_extract self)
            let ST = ('storageof T)
            let destw = ('bitcount ST)
            if (icmp!= (band u64val 0x8000000000000000:u64) 0:u64)
                # if the sign bit is set, signedness matters
                if (icmp!= ('signed? selfST) ('signed? ST))
                    error "implicit conversion of integer value requires same signedness"
            if (icmp>s 64 destw)
                let diff = (zext (sub 64 destw) u64)
                # check if truncation destroys bits
                let cmpval = (shl u64val diff)
                let cmpval =
                    ? ('signed? ST) (ashr cmpval diff) (lshr cmpval diff)
                if (icmp!= u64val cmpval)
                    error "integer value does not fit target type"
            return (sc_const_int_new T u64val)

let static-integer->real =
    spice-converter-macro
        inline (self T)
            let selfT = ('typeof self)
            # allow non-destructive conversions
            let selfST = ('storageof selfT)
            let u64val = (sc_const_int_extract self)
            return
                if ('signed? selfT)
                    sc_const_real_new T (sitofp u64val f64)
                else
                    sc_const_real_new T (uitofp u64val f64)

let integer->integer =
    spice-converter-macro
        inline (self T)
            let selfT = ('typeof self)
            let ST = ('storageof T)
            let destw = ('bitcount ST)
            if (band ('constant? self) (icmp<=u destw 64))
                # allow destructive conversions
                #let selfST = ('storageof selfT)
                let u64val = (sc_const_int_extract self)
                return (sc_const_int_new T u64val)
            let vT = selfT
            let valw = ('bitcount vT)
            if (icmp== destw valw)
                return `(bitcast self T)
            elseif (icmp>s destw valw)
                if ('signed? vT)
                    return `(sext self T)
                else
                    return `(zext self T)
            else
                return `(itrunc self T)

let integer->real =
    spice-converter-macro
        inline (self T)
            let selfT = ('typeof self)
            let ST = ('storageof T)
            let vT = selfT
            let signed? = ('signed? vT)
            if ('constant? self)
                # allow destructive conversions
                #let selfST = ('storageof selfT)
                let u64val = (sc_const_int_extract self)
                let f64val =
                    if signed?
                        sitofp u64val f64
                    else
                        uitofp u64val f64
                return (sc_const_real_new T f64val)
            if signed?
                return `(sitofp self T)
            else
                return `(uitofp self T)

fn integer-static-imply (vT T)
    if (type< T integer)
        let ST = ('storageof T)
        if (icmp== ('kind ST) type-kind-integer)
            if (icmp<=u ('bitcount ST) 64)
                return `(inline (self) (static-integer->integer self T))
    elseif (type< T real)
        let ST = ('storageof T)
        if (icmp== ('kind ST) type-kind-real)
            let valw = ('bitcount vT)
            let destw = ('bitcount ST)
            return `(inline (self) (static-integer->real self T))
    elseif (type== T real)
        return `(inline (self) (static-integer->real self f32))
    `()

fn integer-imply (vT T)
    if (type< T integer)
        let ST = ('storageof T)
        if (icmp== ('kind ST) type-kind-integer)
            let valw = ('bitcount vT)
            let destw = ('bitcount ST)
            # must have larger size or equal size and same bitwidth
            if (bor (icmp>s destw valw)
                    (band (icmp== destw valw)
                        (icmp== ('signed? vT) ('signed? ST))))
                return `(inline (self) (integer->integer self T))
    `()

fn integer-as (vT T)
    if (type< T integer)
        let ST = ('storageof T)
        if (icmp== ('kind ST) type-kind-integer)
            return `(inline (self) (integer->integer self T))
    elseif (type< T real)
        let ST = ('storageof T)
        if (icmp== ('kind ST) type-kind-real)
            return `(inline (self) (integer->real self T))
    `()

let real->real =
    spice-converter-macro
        inline (self T)
            let selfT = ('typeof self)
            let ST = ('storageof T)
            let destw = ('bitcount ST)
            if ('constant? self)
                # allow destructive conversions
                let f64val = (sc_const_real_extract self)
                return (sc_const_real_new T f64val)
            let vT = selfT
            let valw = ('bitcount vT)
            let kind = ('kind ST)
            if (icmp== destw valw)
                return `(bitcast self T)
            elseif (icmp>s destw valw)
                return `(fpext self T)
            else
                return `(fptrunc self T)

let real->integer =
    spice-converter-macro
        inline (self T)
            let selfT = ('typeof self)
            let ST = ('storageof T)
            let signed? = ('signed? ST)
            if ('constant? self)
                # allow destructive conversions
                let f64val = (sc_const_real_extract self)
                let u64val =
                    if signed?
                        fptosi f64val u64
                    else
                        fptoui f64val u64
                return (sc_const_int_new T u64val)
            if signed?
                return `(fptosi self T)
            else
                return `(fptoui self T)

# only perform safe casts: i.e. float to double
fn real-imply (vT T)
    if (type< T real)
        let ST = ('storageof T)
        if (icmp== ('kind ST) type-kind-real)
            let valw = ('bitcount vT)
            let destw = ('bitcount ST)
            if (icmp>=s destw valw)
                return `(inline (self) (real->real self T))
    `()

# more aggressive cast that converts from all numerical types
fn real-as (vT T)
    if (type< T real)
        let ST = ('storageof T)
        let kind = ('kind ST)
        if (icmp== kind type-kind-real)
            return `(inline (self) (real->real self T))
    elseif (type< T integer)
        let ST = ('storageof T)
        let kind = ('kind ST)
        if (icmp== kind type-kind-integer)
            return `(inline (self) (real->integer self T))
    `()

# cast protocol
#------------------------------------------------------------------------------

inline cast-error (intro-string vT T)
    hide-traceback;
    error
        sc_string_join intro-string
            sc_string_join ('__repr (box-pointer vT))
                sc_string_join " to type " ('__repr (box-pointer T))

fn operator-valid? (value)
    ptrcmp!= ('typeof value) void

fn cast-converter (symbol rsymbol vQT T)
    """"for two given types, find a matching conversion function
        this function only works inside a spice macro
    let vT = ('strip-qualifiers vQT)
    label next
        let f =
            try ('@ vT symbol)
            except (err) (merge next)
        let conv = (sc_prove `(f vT T))
        if (operator-valid? conv) (return conv)
        if (ptrcmp!= vT vQT)
            let conv = (sc_prove `(f vQT T))
            if (operator-valid? conv) (return conv)
    label next
        let f =
            try ('@ T rsymbol)
            except (err) (merge next)
        let conv = (sc_prove `(f vT T))
        if (operator-valid? conv) (return conv)
        if (ptrcmp!= vT vQT)
            let conv = (sc_prove `(f vQT T))
            if (operator-valid? conv) (return conv)
    return (sc_empty_argument_list)

fn imply-converter (vQT T static?)
    let vT = ('strip-qualifiers vQT)
    if (ptrcmp== vT T)
        return `_
    if (sc_type_is_superof T vT)
        return `_
    if (ptrcmp== T bool)
        try
            return ('@ vT '__tobool)
        except (err)
    if static?
        let conv =
            cast-converter '__static-imply '__static-rimply vQT T
        if (operator-valid? conv) (return conv)
    cast-converter '__imply '__rimply vQT T

fn as-converter (vQT T static?)
    let vT = ('strip-qualifiers vQT)
    if (ptrcmp== vT T)
        return `_
    if (sc_type_is_superof T vT)
        return `_
    if (ptrcmp== T bool)
        try
            return ('@ vT '__tobool)
        except (err)
    let conv = (cast-converter '__as '__ras vQT T)
    if (operator-valid? conv) (return conv)
    # try implicit cast last
    if static?
        let conv =
            cast-converter '__static-imply '__static-rimply vQT T
        if (operator-valid? conv) (return conv)
    cast-converter '__imply '__rimply vQT T

inline gen-cast-op (f str)
    spice-macro
        fn "cast-op" (args)
            let argc = ('argcount args)
            verify-count argc 2 2
            let value = ('getarg args 0)
            let anyT = ('getarg args 1)
            let vT = ('qualified-typeof value)
            let T = (unbox-pointer anyT type)
            let conv = (f vT T ('constant? value))
            if (operator-valid? conv)
                return `(conv value)
            cast-error str vT T

let imply = (gen-cast-op imply-converter "can't coerce value of type ")
let as = (gen-cast-op as-converter "can't cast value of type ")

# operator protocol
#------------------------------------------------------------------------------

fn unary-operator (symbol T)
    """"for an operation performed on one variable argument type, find a
        matching operator function. This function only works inside a spice
        macro.
    label next
        let f =
            try ('@ T symbol)
            except (err) (merge next)
        let op = (sc_prove `(f T))
        if (operator-valid? op) (return op)
    return (sc_empty_argument_list)

fn binary-operator (symbol lhsT rhsT)
    """"for an operation performed on two argument types, of which only
        the left type can provide a suitable candidate, find a matching
        operator function. This function only works inside a spice macro.
    label next
        let f =
            try ('@ lhsT symbol)
            except (err) (merge next)
        let op = (sc_prove `(f lhsT rhsT))
        if (operator-valid? op) (return op)
    return (sc_empty_argument_list)

fn binary-operator-r (rsymbol lhsT rhsT)
    """"for an operation performed on two argument types, of which only
        the right type can provide a suitable candidate, find a matching
        operator function. This function only works inside a spice macro.
    label next
        let f =
            try ('@ rhsT rsymbol)
            except (err) (merge next)
        let op = (sc_prove `(f lhsT rhsT))
        if (operator-valid? op) (return op)
    return (sc_empty_argument_list)

fn balanced-binary-operator (symbol rsymbol lhsT rhsT lhs-static? rhs-static?)
    """"for an operation performed on two argument types, of which either
        type can provide a suitable candidate, return a matching operator.
        This function only works inside a spice macro.
    # try the left type
    let op = (binary-operator symbol lhsT rhsT)
    if (operator-valid? op) (return op)
    if (ptrcmp!= lhsT rhsT)
        # asymmetrical types

        # try the right type
        let op = (binary-operator-r rsymbol lhsT rhsT)
        if (operator-valid? op) (return op)

        # can we cast rhsT to lhsT?
        let conv = (imply-converter rhsT lhsT rhs-static?)
        if (operator-valid? conv)
            # is symmetrical op supported for the left type?
            let op = (binary-operator symbol lhsT lhsT)
            if (operator-valid? op)
                return `(inline (lhs rhs) (op lhs (conv rhs)))

        # can we cast lhsT to rhsT?
        let conv = (imply-converter lhsT rhsT lhs-static?)
        if (operator-valid? conv)
            # is symmetrical op supported for the right type?
            let op = (binary-operator symbol rhsT rhsT)
            if (operator-valid? op)
                return `(inline (lhs rhs) (op (conv lhs) rhs))
    return (sc_empty_argument_list)

# right hand has same type as left hand
fn balanced-lvalue-binary-operator (symbol lhsT rhsT rhs-static?)
    """"for an operation performed on two argument types, of which only the
        left type type can provide a suitable candidate, return a matching operator.
        This function only works inside a spice macro.
    # try the left type
    let op = (binary-operator symbol lhsT rhsT)
    if (operator-valid? op) (return op)
    if (ptrcmp!= lhsT rhsT)
        # asymmetrical types

        # can we cast rhsT to lhsT?
        let conv = (imply-converter rhsT lhsT rhs-static?)
        if (operator-valid? conv)
            # is symmetrical op supported for the left type?
            let op = (binary-operator symbol lhsT lhsT)
            if (operator-valid? op)
                return `(inline (lhs rhs) (op lhs (conv rhs)))
    return (sc_empty_argument_list)

fn unary-op-error (friendly-op-name T)
    hide-traceback;
    error
        'join "can't "
            'join friendly-op-name
                'join " value of type " ('__repr (box-pointer T))

fn binary-op-error (friendly-op-name lhsT rhsT)
    hide-traceback;
    error
        'join "can't "
            'join friendly-op-name
                'join " values of types "
                    'join ('__repr (box-pointer lhsT))
                        'join " and "
                            '__repr (box-pointer rhsT)

fn balanced-binary-operation (args symbol rsymbol friendly-op-name)
    let argc = ('argcount args)
    verify-count argc 2 2
    let lhs rhs =
        'getarg args 0
        'getarg args 1
    let lhsT = ('typeof lhs)
    let rhsT = ('typeof rhs)
    let op = (balanced-binary-operator symbol rsymbol lhsT rhsT
        ('constant? lhs) ('constant? rhs))
    if (operator-valid? op)
        return ('tag `(op lhs rhs) ('anchor args))
    hide-traceback;
    binary-op-error friendly-op-name lhsT rhsT

fn balanced-lvalue-binary-operation (args symbol friendly-op-name)
    let argc = ('argcount args)
    verify-count argc 2 2
    let lhs rhs =
        'getarg args 0
        'getarg args 1
    let lhsT = ('typeof lhs)
    let rhsT = ('typeof rhs)
    let op = (balanced-lvalue-binary-operator symbol lhsT rhsT ('constant? rhs))
    if (operator-valid? op)
        return ('tag `(op lhs rhs) ('anchor args))
    hide-traceback;
    binary-op-error friendly-op-name lhsT rhsT

# right hand has fixed type - this one doesn't need a dispatch step
fn unbalanced-binary-operation (args symbol rtype friendly-op-name)
    let argc = ('argcount args)
    verify-count argc 2 2
    let lhs rhs =
        'getarg args 0
        'getarg args 1
    let lhsT = ('typeof lhs)
    let rhsT = ('typeof rhs)
    let rhs =
        if (ptrcmp== rhsT rtype) rhs
        else
            # can we cast rhsT to rtype?
            let conv = (imply-converter rhsT rtype ('constant? rhs))
            if (operator-valid? conv)
                `(conv rhs)
            else
                cast-error "can't coerce secondary argument of type " rhsT rtype
    let f =
        try ('@ lhsT symbol)
        except (err)
            unary-op-error friendly-op-name lhsT
    'tag `(f lhs rhs) ('anchor args)

# unary operations don't need a dispatch step either
fn unary-operation (args symbol friendly-op-name)
    let argc = ('argcount args)
    verify-count argc 1 1
    let u = ('getarg args 0)
    let T = ('typeof u)
    let f =
        try ('@ T symbol)
        except (err)
            unary-op-error friendly-op-name T
    'tag `(f u) ('anchor args)

fn unary-or-balanced-binary-operation (args usymbol ufriendly-op-name symbol rsymbol friendly-op-name)
    let argc = ('argcount args)
    if (icmp== argc 1)
        unary-operation args usymbol ufriendly-op-name
    else
        balanced-binary-operation args symbol rsymbol friendly-op-name

fn unary-or-unbalanced-binary-operation (args usymbol ufriendly-op-name symbol rtype friendly-op-name)
    let argc = ('argcount args)
    if (icmp== argc 1)
        unary-operation args usymbol ufriendly-op-name
    else
        unbalanced-binary-operation args symbol rtype friendly-op-name

inline unary-op-dispatch (symbol friendly-op-name)
    spice-macro (fn (args) (unary-operation args symbol friendly-op-name))

inline unary-or-balanced-binary-op-dispatch (usymbol ufriendly-op-name symbol rsymbol friendly-op-name)
    spice-macro (fn (args) (unary-or-balanced-binary-operation
        args usymbol ufriendly-op-name symbol rsymbol friendly-op-name))

inline unary-or-unbalanced-binary-op-dispatch (usymbol ufriendly-op-name symbol rtype friendly-op-name)
    spice-macro (fn (args) (unary-or-unbalanced-binary-operation
        args usymbol ufriendly-op-name symbol rtype friendly-op-name))

inline balanced-binary-op-dispatch (symbol rsymbol friendly-op-name)
    spice-macro
        fn (args)
            hide-traceback;
            balanced-binary-operation args symbol rsymbol friendly-op-name

inline balanced-lvalue-binary-op-dispatch (symbol friendly-op-name)
    spice-macro
        fn (args)
            hide-traceback;
            balanced-lvalue-binary-operation args symbol friendly-op-name

inline unbalanced-binary-op-dispatch (symbol rtype friendly-op-name)
    spice-macro
        fn (args)
            hide-traceback;
            unbalanced-binary-operation args symbol rtype friendly-op-name

inline spice-binary-op-macro (f)
    """"to be used for binary operators of which either type can
        provide an operation. returns a callable operator (f lhs rhs) that
        performs the operation or no arguments if the operation can not be
        performed.
    spice-macro
        fn (args)
            raising Error
            let argc = (sc_argcount args)
            verify-count argc 2 2
            let lhs-type = (unbox-pointer (sc_getarg args 0) type)
            let rhs-type = (unbox-pointer (sc_getarg args 1) type)
            f lhs-type rhs-type

inline simple-binary-op (f)
    """"for cases where the type only interacts with itself
    spice-binary-op-macro
        inline (lhsT rhsT)
            if (ptrcmp== lhsT rhsT)
                return `f
            `()

inline simple-folding-binary-op (f unboxer boxer)
    spice-binary-op-macro
        inline (lhsT rhsT)
            let f =
                spice-macro
                    fn (args)
                        let argc = ('argcount args)
                        verify-count argc 2 2
                        let lhs = ('getarg args 0)
                        let rhs = ('getarg args 1)
                        'tag
                            if (band ('constant? lhs) ('constant? rhs))
                                let T = ('typeof lhs)
                                let lhs = (unboxer lhs)
                                let rhs = (unboxer rhs)
                                boxer T (f lhs rhs)
                            else
                                `(f lhs rhs)
                            'anchor args
            if (ptrcmp== lhsT rhsT)
                return `f
            `()

fn autoboxer (T x) `x
inline simple-folding-autotype-binary-op (f unboxer)
    simple-folding-binary-op f unboxer autoboxer

inline simple-signed-binary-op (sf uf)
    spice-binary-op-macro
        inline (lhsT rhsT)
            if (ptrcmp== lhsT rhsT)
                if ('signed? lhsT)
                    return `sf
                else
                    return `uf
            `()

inline simple-folding-signed-binary-op (sf uf unboxer boxer)
    spice-binary-op-macro
        inline (lhsT rhsT)
            let f =
                spice-macro
                    fn (args)
                        let argc = ('argcount args)
                        verify-count argc 2 2
                        let lhs = ('getarg args 0)
                        let rhs = ('getarg args 1)
                        let lhsT = ('typeof lhs)
                        let signed? = ('signed? lhsT)
                        'tag
                            if (band ('constant? lhs) ('constant? rhs))
                                let lhs = (unboxer lhs)
                                let rhs = (unboxer rhs)
                                if signed?
                                    boxer lhsT (sf lhs rhs)
                                else
                                    boxer lhsT (uf lhs rhs)
                            elseif signed?
                                `(sf lhs rhs)
                            else
                                `(uf lhs rhs)
                            'anchor args
            if (ptrcmp== lhsT rhsT)
                return `f
            `()

inline simple-folding-autotype-signed-binary-op (sf uf unboxer)
    simple-folding-signed-binary-op sf uf unboxer autoboxer

# support for calling macro functions directly
'set-symbols SugarMacro
    __call =
        box-pointer
            spice-macro
                fn (args)
                    raising Error
                    let argc = (sc_argcount args)
                    verify-count argc 3 3
                    let self = (sc_getarg args 0)
                    let topexpr = (sc_getarg args 1)
                    let scope = (sc_getarg args 2)
                    `((bitcast self SugarMacroFunction) topexpr scope)

'define-symbols Symbol
    unique =
        inline (cls name)
            sc_symbol_new_unique name
    variadic? = sc_symbol_is_variadic

'set-symbols Symbol
    __== = (box-pointer (simple-folding-autotype-binary-op icmp== sc_const_int_extract))
    __!= = (box-pointer (simple-folding-autotype-binary-op icmp!= sc_const_int_extract))
    __as =
        box-pointer
            spice-cast-macro
                inline (vT T)
                    if (ptrcmp== T string)
                        return `sc_symbol_to_string
                    `()

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
    __== = (box-pointer (simple-binary-op ptrcmp==))
    __!= = (box-pointer (simple-binary-op ptrcmp!=))
    __.. = (box-pointer (simple-folding-autotype-binary-op sc_string_join
        (inline (x) (unbox-pointer x string))))
    __< = (box-pointer (simple-binary-op (inline (a b) (icmp<s (sc_string_compare a b) 0))))
    __<= = (box-pointer (simple-binary-op (inline (a b) (icmp<=s (sc_string_compare a b) 0))))
    __> = (box-pointer (simple-binary-op (inline (a b) (icmp>s (sc_string_compare a b) 0))))
    __>= = (box-pointer (simple-binary-op (inline (a b) (icmp>=s (sc_string_compare a b) 0))))

'define-symbols list
    __typecall = list-constructor
    __repr =
        inline "list-repr" (self)
            sc_list_repr self

'set-symbols list
    __.. = (box-pointer (simple-binary-op sc_list_join))
    __== = (box-pointer (simple-binary-op sc_list_compare))

fn dispatch-and-or (args flip)
    let argc = ('argcount args)
    verify-count argc 2 -1
    let elsef cond =
        'getarg args 0
        'getarg args 1
    let thenargs =
        'getarglist args 1
    let call-elsef = `(elsef)
    let condT = ('typeof cond)
    let conv = (imply-converter condT bool ('constant? cond))
    let condbool =
        if (operator-valid? conv)
            hide-traceback;
            sc_prove ('tag `(conv cond) ('anchor cond))
        else cond
    if (ptrcmp== ('typeof condbool) bool)
        if ('constant? condbool)
            let value = (unbox-integer condbool bool)
            return
                if (bxor value flip) thenargs
                else call-elsef
    elseif flip
        return call-elsef
    else
        return thenargs
    let ifval = (sc_if_new)
    if flip
        sc_if_append_then_clause ifval condbool call-elsef
        sc_if_append_else_clause ifval thenargs
    else
        sc_if_append_then_clause ifval condbool thenargs
        sc_if_append_else_clause ifval call-elsef
    ifval

let safe-shl =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 2 2
            let lhs rhs = ('getarg args 0) ('getarg args 1)
            let lhsT = ('typeof lhs)
            let rhsT = ('typeof rhs)
            let bits = ('bitcount lhsT)
            let maskval = (zext (sub bits 1) u64)
            let mask = (sc_const_int_new rhsT maskval)
            if ('constant? lhs)
                if ('constant? rhs)
                    let lhs = (sc_const_int_extract lhs)
                    let rhs = (sc_const_int_extract rhs)
                    let lhs = (shl lhs (band rhs maskval))
                    return (sc_const_int_new lhsT lhs)
            # mask right hand side by bit width
            `(shl lhs (band rhs mask))

fn powi (base exponent)
    let T = (typeof base)
    let
        0T = (nullof T)
        1T = (zext 1 T)
        2T = (zext 2 T)
    # special case for constant base 2
    if (icmp== base 2T)
        return
            shl 1T exponent
    loop (result cur exponent = 1T base exponent)
        if (icmp== exponent 0T)
            return result
        else
            repeat
                do
                    if (icmp== (band exponent 1T) 0T) result
                    else
                        mul result cur
                mul cur cur
                lshr exponent 1T

'set-symbols integer
    __tobool = (box-pointer (spice-macro integer-tobool))
    __imply = (box-pointer (spice-cast-macro integer-imply))
    __static-imply = (box-pointer (spice-cast-macro integer-static-imply))
    __as = (box-pointer (spice-cast-macro integer-as))
    __+ = (box-pointer (simple-folding-binary-op add sc_const_int_extract sc_const_int_new))
    __- = (box-pointer (simple-folding-binary-op sub sc_const_int_extract sc_const_int_new))
    __* = (box-pointer (simple-folding-binary-op mul sc_const_int_extract sc_const_int_new))
    __** = (box-pointer (simple-folding-binary-op powi sc_const_int_extract sc_const_int_new))
    __// = (box-pointer (simple-folding-signed-binary-op sdiv udiv sc_const_int_extract sc_const_int_new))
    __/ =
        box-pointer
            simple-folding-autotype-signed-binary-op
                inline (a b) (fdiv (sitofp a f32) (sitofp b f32))
                inline (a b) (fdiv (uitofp a f32) (uitofp b f32))
                \ sc_const_int_extract
    __% = (box-pointer (simple-folding-signed-binary-op srem urem sc_const_int_extract sc_const_int_new))
    __& = (box-pointer (simple-folding-binary-op band sc_const_int_extract sc_const_int_new))
    __| = (box-pointer (simple-folding-binary-op bor sc_const_int_extract sc_const_int_new))
    __^ = (box-pointer (simple-folding-binary-op bxor sc_const_int_extract sc_const_int_new))
    __<< = (box-pointer (simple-binary-op safe-shl))
    __>> = (box-pointer (simple-folding-signed-binary-op ashr lshr sc_const_int_extract sc_const_int_new))
    __== = (box-pointer (simple-folding-autotype-binary-op icmp== sc_const_int_extract))
    __!= = (box-pointer (simple-folding-autotype-binary-op icmp!= sc_const_int_extract))
    __< = (box-pointer (simple-folding-autotype-signed-binary-op icmp<s icmp<u sc_const_int_extract))
    __<= = (box-pointer (simple-folding-autotype-signed-binary-op icmp<=s icmp<=u sc_const_int_extract))
    __> = (box-pointer (simple-folding-autotype-signed-binary-op icmp>s icmp>u sc_const_int_extract))
    __>= = (box-pointer (simple-folding-autotype-signed-binary-op icmp>=s icmp>=u sc_const_int_extract))

inline floordiv (a b)
    sdiv (fptosi a i32) (fptosi b i32)

'set-symbols real
    __imply = (box-pointer (spice-cast-macro real-imply))
    __as = (box-pointer (spice-cast-macro real-as))
    __== = (box-pointer (simple-folding-autotype-binary-op fcmp==o sc_const_real_extract))
    __!= = (box-pointer (simple-folding-autotype-binary-op fcmp!=u sc_const_real_extract))
    __> = (box-pointer (simple-folding-autotype-binary-op fcmp>o sc_const_real_extract))
    __>= = (box-pointer (simple-folding-autotype-binary-op fcmp>=o sc_const_real_extract))
    __< = (box-pointer (simple-folding-autotype-binary-op fcmp<o sc_const_real_extract))
    __<= = (box-pointer (simple-folding-autotype-binary-op fcmp<=o sc_const_real_extract))
    __+ = (box-pointer (simple-folding-binary-op fadd sc_const_real_extract sc_const_real_new))
    __- = (box-pointer (simple-folding-binary-op fsub sc_const_real_extract sc_const_real_new))
    __* = (box-pointer (simple-folding-binary-op fmul sc_const_real_extract sc_const_real_new))
    __/ = (box-pointer (simple-folding-binary-op fdiv sc_const_real_extract sc_const_real_new))
    __// = (box-pointer (simple-folding-autotype-binary-op floordiv sc_const_real_extract))
    __% = (box-pointer (simple-folding-binary-op frem sc_const_real_extract sc_const_real_new))
    __** = (box-pointer (simple-folding-binary-op powf sc_const_real_extract sc_const_real_new))

'set-symbols Value
    __== = (box-pointer (simple-binary-op sc_value_compare))

'set-symbols Closure
    __== = (box-pointer (simple-folding-autotype-binary-op ptrcmp== sc_const_pointer_extract))
    __!= = (box-pointer (simple-folding-autotype-binary-op ptrcmp!= sc_const_pointer_extract))

'define-symbols type
    __@ = sc_type_element_at

'set-symbols type
    __== = (box-pointer (simple-binary-op type==))
    __!= = (box-pointer (simple-binary-op type!=))
    __< = (box-pointer (simple-binary-op type<))
    __<= = (box-pointer (simple-binary-op type<=))
    __> = (box-pointer (simple-binary-op type>))
    __>= = (box-pointer (simple-binary-op type>=))
    __countof =
        box-spice-macro
            fn "type-countof" (args)
                let argc = ('argcount args)
                verify-count argc 1 1
                let self = ('getarg args 0)
                hide-traceback;
                `[(sc_type_countof (unbox-pointer self type))]
    __toptr =
        box-pointer
            spice-macro
                fn (args)
                    let argc = ('argcount args)
                    verify-count argc 1 1
                    let self = ('getarg args 0)
                    'tag `[(sc_refer_type (unbox-pointer self type) pointer-flag-non-writable unnamed)]
                        'anchor args
    __toref =
        box-pointer
            inline (self)
                pointer self
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
                let self = (unbox-pointer self type)
                let key = (unbox-symbol key Symbol)
                hide-traceback;
                return (sc_type_at self key)

'set-symbols Scope
    __== = (box-pointer (simple-binary-op ptrcmp==))
    __getattr =
        box-spice-macro
            fn "scope-getattr" (args)
                let argc = ('argcount args)
                verify-count argc 2 2
                let self key =
                    'getarg args 0
                    'getarg args 1
                let self = (unbox-pointer self Scope)
                hide-traceback;
                return (sc_scope_at self key)
    __typecall =
        box-spice-macro
            fn "scope-typecall" (args)
                """"There are two ways to create a new Scope:
                    ``Scope``
                        creates an empty scope without parent
                    ``Scope parent``
                        creates an empty scope descending from ``parent``
                let argc = ('argcount args)
                verify-count argc 1 2
                switch argc
                case 1 `(sc_scope_new)
                default
                    `(sc_scope_new_subscope [ ('getarg args 1) ])

#---------------------------------------------------------------------------
# nothing type
#---------------------------------------------------------------------------

'set-symbols Nothing
    __tobool = (box-pointer (inline () false))
    __== =
        box-pointer
            spice-binary-op-macro
                inline (lhsT rhsT)
                    inline always-true (a b) true
                    if (ptrcmp== rhsT Nothing)
                        return `always-true
                    `()

#---------------------------------------------------------------------------
# null type
#---------------------------------------------------------------------------

""""The type of the `null` constant. This type is uninstantiable.
'set-plain-storage NullType (pointer void)
do
    inline null== (lhs rhs) (icmp== (ptrtoint rhs usize) 0:usize)
    inline nullr== (lhs rhs) (icmp== (ptrtoint lhs usize) 0:usize)

    'set-symbols NullType
        __repr =
            box-pointer
                inline (self)
                    sc_default_styler style-number "null"
        __imply =
            box-pointer
                spice-cast-macro
                    fn "null-imply" (cls T)
                        if (icmp== ('kind ('storageof T)) type-kind-pointer)
                            return `(inline (self) (bitcast self T))
                        `()
        __== =
            box-pointer
                spice-binary-op-macro
                    inline (lhsT rhsT)
                        if (icmp== ('kind ('storageof rhsT)) type-kind-pointer)
                            return `null==
                        `()
        __r== =
            box-pointer
                spice-binary-op-macro
                    inline (lhsT rhsT)
                        if (icmp== ('kind ('storageof lhsT)) type-kind-pointer)
                            return `nullr==
                        `()

let
    and-branch = (spice-macro (fn (args) (dispatch-and-or args true)))
    or-branch = (spice-macro (fn (args) (dispatch-and-or args false)))
    #implyfn = (static-typify implyfn type type)
    #asfn = (static-typify asfn type type)
    countof = (unary-op-dispatch '__countof "count")
    unpack = (unary-op-dispatch '__unpack "unpack")
    hash1 = (unary-op-dispatch '__hash "hash")
    ~ = (unary-op-dispatch '__~ "bitwise-negate")
    == = (balanced-binary-op-dispatch '__== '__r== "compare")
    != = (balanced-binary-op-dispatch '__!= '__r!= "compare")
    < = (balanced-binary-op-dispatch '__< '__r< "compare")
    <= = (balanced-binary-op-dispatch '__<= '__r<= "compare")
    > = (balanced-binary-op-dispatch '__> '__r> "compare")
    >= = (balanced-binary-op-dispatch '__>= '__r>= "compare")
    + = (balanced-binary-op-dispatch '__+ '__r+ "add")
    - = (unary-or-balanced-binary-op-dispatch '__neg "negate" '__- '__r- "subtract")
    * = (balanced-binary-op-dispatch '__* '__r* "multiply")
    ** = (balanced-binary-op-dispatch '__** '__r** "exponentiate")
    / = (unary-or-balanced-binary-op-dispatch '__rcp "invert" '__/ '__r/ "real-divide")
    // = (balanced-binary-op-dispatch '__// '__r// "integer-divide")
    % = (balanced-binary-op-dispatch '__% '__r% "modulate")
    & = (unary-or-balanced-binary-op-dispatch '__toptr "reference" '__& '__r& "apply bitwise-and to")
    | = (balanced-binary-op-dispatch '__| '__r| "apply bitwise-or to")
    ^ = (balanced-binary-op-dispatch '__^ '__r^ "apply bitwise-xor to")
    << = (balanced-binary-op-dispatch '__<< '__r<< "apply left shift with")
    >> = (balanced-binary-op-dispatch '__>> '__r>> "apply right shift with")
    .. = (balanced-binary-op-dispatch '__.. '__r.. "join")
    = = (balanced-lvalue-binary-op-dispatch '__= "apply assignment with")
    @ = (unary-or-unbalanced-binary-op-dispatch '__toref "dereference" '__@ integer "apply subscript operator with")
    #getattr = (unbalanced-binary-op-dispatch '__getattr Symbol "get attribute from")
    lslice = (unbalanced-binary-op-dispatch '__lslice usize "apply left-slice operator with")
    rslice = (unbalanced-binary-op-dispatch '__rslice usize "apply right-slice operator with")

let getattr =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 2 2
            let lhs rhs =
                'getarg args 0
                'getarg args 1
            let lhsT = ('typeof lhs)
            let rhsT = ('typeof rhs)
            let rhs =
                if (ptrcmp== rhsT Symbol) rhs
                else
                    # can we cast rhsT to rtype?
                    let conv = (imply-converter rhsT Symbol ('constant? rhs))
                    if (operator-valid? conv)
                        sc_prove `(conv rhs)
                    else
                        cast-error "can't coerce secondary argument of type " rhsT Symbol
            label skip-accessor-lookup
                let sym = (unbox-symbol rhs Symbol)
                let prop =
                    try ('@ lhsT sym)
                    else (merge skip-accessor-lookup)
                if (ptrcmp!= ('typeof prop) Accessor)
                    merge skip-accessor-lookup;
                if (not ('constant? prop))
                    merge skip-accessor-lookup
                let prop = (unbox-pointer prop Accessor)
                let prop = (bitcast prop Closure)
                return
                    'tag `(prop lhs rhs) ('anchor args)
            let f =
                try ('@ lhsT '__getattr)
                else
                    unary-op-error "get attribute from" lhsT
            'tag `(f lhs rhs) ('anchor args)

let drop =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let value = ('getarg args 0)
            let anchor = ('anchor args)
            let dropped = (sc_prove ('tag `(dropped? value) anchor))
            if (unbox-integer dropped bool) `()
            else
                let block = (sc_expression_new)
                sc_expression_append block ('tag `(__drop value) anchor)
                sc_expression_append block ('tag `(lose value) anchor)
                block

let forward-repr =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let value = ('getarg args 0)
            let T = ('typeof value)
            try
                let f = (sc_type_at T '__repr)
                `(f value)
            except (err)
                `(sc_value_content_repr value)

let repr =
    spice-macro
        fn (args)
            fn type-is-default-suffix? (CT)
                if (sc_type_is_default_suffix CT) true
                elseif (ptrcmp== CT NullType) true
                else false
            let argc = ('argcount args)
            verify-count argc 1 1
            let value = ('getarg args 0)
            let T = ('typeof value)
            let s = `(forward-repr value)
            if (type-is-default-suffix? T) s
            else

                let suffix =
                    sc_string_join
                        sc_default_styler style-operator ":"
                        sc_default_styler style-type ('string T)
                `(sc_string_join s suffix)

let tostring =
    box-spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 -1
            let value = ('getarg args 0)
            try
                `([('@ ('typeof value) '__tostring)] value)
            except (err)
                if ('constant? value)
                    `[(sc_value_tostring value)]
                else
                    `(sc_value_tostring value)

run-stage; # 4

inline gen-cast? (converterf)
    spice-macro
        fn "cast?" (args)
            let argc = ('argcount args)
            verify-count argc 2 2
            let value = ('getarg args 0)
            let T = (as ('getarg args 1) type)
            let valueT constant =
                if (band (== ('typeof value) type) ('constant? value))
                    _ (as value type) false
                else (_ ('qualified-typeof value) ('constant? value))
            let conv = (converterf valueT T constant)
            let result = (operator-valid? conv)
            `result

let imply? = (gen-cast? imply-converter)
let as? = (gen-cast? as-converter)

'set-symbols integer
    __~ = (box-pointer (inline (self) (^ self (as -1 (typeof self)))))
    __neg = (box-pointer (inline (self) (- (as 0 (typeof self)) self)))
    __rcp = (box-pointer (inline (self) (/ (as 1 (typeof self)) self)))

'set-symbols real
    __neg = (box-pointer (inline (self) (- (as 0 (typeof self)) self)))
    __rcp = (box-pointer (inline (self) (/ (as 1 (typeof self)) self)))
    __tobool = (box-pointer (inline (self) (!= self (as 0 (typeof self)))))

let opaque =
    spice-macro
        fn "opaque" (args)
            let argc = ('argcount args)
            verify-count argc 1 2
            let name = (as ('getarg args 0) string)
            let supertype =
                if (> argc 1) (as ('getarg args 1) type)
                else typename
            let TT = (sc_typename_type name supertype)
            sc_typename_type_set_opaque TT
            'tag `TT ('anchor args)

'set-symbols typename
    # inverted compare attempts regular compare
    __!= = (box-pointer (simple-binary-op (inline (lhs rhs) (not (== lhs rhs)))))
    # default assignment operator
    __= =
        box-pointer
            simple-binary-op
                spice-macro
                    fn (args)
                        let argc = ('argcount args)
                        verify-count argc 2 2
                        let lhs = ('getarg args 0)
                        let rhs = ('getarg args 1)
                        let anchor = ('anchor args)
                        spice-quote
                            [('tag `(__drop lhs) anchor)]
                            [('tag `(assign rhs lhs) anchor)]
    # default dereference
    __toptr =
        box-pointer
            spice-macro
                fn (args)
                    let argc = ('argcount args)
                    verify-count argc 1 1
                    let self = ('getarg args 0)
                    if ('refer? ('qualified-typeof self))
                        return ('tag `(reftoptr self) ('anchor args))
                    error "can not convert immutable value to pointer"
    # dynamic typename constructor
    type = `sc_typename_type
    # static typename constructor
    __typecall =
        box-pointer
            spice-macro
                fn "typename.__typecall" (args)
                    let argc = ('argcount args)
                    verify-count argc 1 -1
                    let cls = (as ('getarg args 0) type)
                    if (!= cls typename)
                        hide-traceback;
                        error
                            sc_string_join "typename "
                                sc_string_join ('__repr `cls)
                                    " has no constructor"
                    verify-count argc 2 3
                    let name = (as ('getarg args 1) string)
                    let supertype =
                        if (> argc 2) (as ('getarg args 2) type)
                        else typename
                    'tag `[(sc_typename_type name supertype)] ('anchor args)

let null = (nullof NullType)

# slightly better not - to be replaced further down once more
inline not (value)
    bxor (imply value bool) true

let function->SugarMacro =
    static-typify
        fn "function->SugarMacro" (f)
            bitcast f SugarMacro
        SugarMacroFunction

inline sugar-block-scope-macro (f)
    function->SugarMacro (static-typify f list Scope)

inline sugar-scope-macro (f)
    sugar-block-scope-macro
        fn (topexpr scope)
            let at next = (decons topexpr)
            let at = (as at list)
            let head = ('@ at)
            let anchor = ('anchor head)
            hide-traceback;
            let at scope = (f ('next at) scope)
            return (cons ('tag `at anchor) next) scope

inline sugar-macro (f)
    sugar-block-scope-macro
        fn (topexpr scope)
            let at next = (decons topexpr)
            let at = (as at list)
            let head = ('@ at)
            let anchor = ('anchor head)
            hide-traceback;
            let at = (f ('next at))
            return (cons ('tag `at anchor) next) scope

inline empty? (value)
    == (countof value) 0

let print =
    do
        inline print-element (i key value)
            let value = (view value)
            static-branch (const.icmp<=.i32.i32 i 0)
                inline ()
                inline ()
                    sc_write " "
            static-branch (== (typeof value) string)
                inline ()
                    sc_write value
                inline ()
                    sc_write (repr value)

        inline print (values...)
            va-lifold none print-element values...
            sc_write "\n"

let report =
    spice-macro
        fn (args)
            raising Error
            let anchor = ('__repr `[('anchor args)])
            spice-quote
                print anchor args
                view args

fn extract-integer (value)
    if (== ('kind value) value-kind-const-int)
        return (sc_const_int_extract value)
    error@ ('anchor value) "while extracting integer" "integer constant expected"

'set-symbol integer '__typecall
    box-pointer
        spice-macro
            fn (args)
                raising Error
                let cls = (as ('getarg args 0) type)
                let argc = ('argcount args)
                if (ptrcmp== cls integer)
                    verify-count argc 2 3
                    let size = ('getarg args 1)
                    let size = (extract-integer size)
                    let signed =
                        if (== argc 2) false
                        else (as ('getarg args 2) bool)
                    `[(sc_integer_type (as size i32) signed)]
                else
                    verify-count argc 1 2
                    if (== argc 1)
                        `(nullof cls)
                    else
                        let value = ('getarg args 1)
                        `(as value cls)

'define-symbol real '__typecall
    inline (cls value)
        static-branch (none? value)
            inline () (nullof cls)
            inline ()
                as value cls

'set-symbols string
    __imply =
        box-pointer
            spice-cast-macro
                fn (vT T)
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
                                        let s c = (sc_string_buffer str)
                                        s
                    if (ptrcmp== T rawstring)
                        return `string->rawstring
                    `()

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
            if ('function? fT)
                let variadic? = ('variadic? fT)
                let pcount = ('element-count fT)
                if (| (& (not variadic?) (== pcount argc))
                      (& variadic? (<= pcount argc)))
                    let outargs = ('tag (sc_call_new self) ('anchor args))
                    sc_call_set_rawcall outargs true
                    loop (i = 0)
                        if (== i argc) (break)
                        let arg = ('getarg args (add i 1))
                        let argT = ('qualified-typeof arg)
                        if (>= i pcount)
                            sc_call_append_argument outargs arg
                        else
                            let paramT = ('element@ fT i)
                            let outarg =
                                if (sc_type_compatible argT paramT) arg
                                else
                                    ('tag `(imply arg paramT) ('anchor arg))
                            sc_call_append_argument outargs outarg
                        + i 1
                    return outargs
            # let prover handle type error
            'tag `(rawcall self [('getarglist args 1)]) ('anchor args)

fn pointer-type-imply? (src dest)
    let ET = ('element@ src 0)
    let ET =
        if ('opaque? ET) ET
        else ('storageof ET)
    let ETkind = ('kind ET)
    # [T x n](*) is interpreted as T(*)
    let ET src =
        if (icmp== ETkind type-kind-array)
            let ET = ('element@ ET 0)
            let src = ('change-element-type src ET)
            _ ET src
        else (_ ET src)
    if (not (icmp== ETkind type-kind-pointer))
        # casts to voidstar are only permitted if we are not holding
        # a ref to another pointer
        if (type== dest voidstar)
            return true
        elseif (type== dest (mutable voidstar))
            if ('writable? src)
                return true
    if (type== dest src)
        return true
    elseif (type== dest ('strip-pointer-storage-class src))
        return true
    elseif (type== dest ('immutable src))
        return true
    elseif (type== dest ('strip-pointer-storage-class ('immutable src)))
        return true
    return false

fn pointer-imply (vT T)
    if (icmp== ('kind T) type-kind-pointer)
        if (pointer-type-imply? vT T)
            return `(inline (self) (bitcast self T))
    `()

fn pointer-as (vT T)
    if (type< T pointer)
        let ST = ('storageof T)
        if (icmp== ('kind ST) type-kind-pointer)
            if (icmp== ('pointer-storage-class vT) ('pointer-storage-class T))
                return `(inline (self) (bitcast self T))
    `()

'set-symbols pointer
    __call = coerce-call-arguments
    __imply = (box-pointer (spice-cast-macro pointer-imply))
    __as = (box-pointer (spice-cast-macro pointer-as))

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

fn split-dotted-symbol (name)
    let anchor = ('anchor name)
    let s = (as (as name Symbol) string)
    let sz = (countof s)
    loop (i = sz)
        if (== i 0:usize)
            # did not find a dot - return as-is
            return name
        let _i = i
        let i = (- i 1:usize)
        if (== (@ s i) dot-char)
            # skip trailing dot
            if (== _i sz)
                repeat i
            # if a dot followed this dot, skip it
            if (== (@ s _i) dot-char)
                repeat i
            # ignore prefix dot - return as-is
            if (== i 0:usize)
                return name
            let k = (- i 1:usize)
            # if a dot precedes this dot, skip it
            if (== (@ s k) dot-char)
                repeat i
            # we found a good solo dot inbetween two characters
            let ltoken = (Symbol (lslice s i))
            let rtoken = (Symbol (rslice s _i))
            let manchor = (sc_anchor_offset anchor (as i i32))
            let ranchor = (sc_anchor_offset anchor (as _i i32))
            # build expression
            let expr =
                list
                    'tag `dot-sym manchor
                    'tag `ltoken anchor
                    'tag `rtoken ranchor
            return ('tag `expr manchor)
        i

# infix notation support
# --------------------------------------------------------------------------

fn get-ifx-symbol (name)
    Symbol (.. "#ifx:" (as name string))

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
    let scope = ('bind scope `[(get-ifx-symbol token)]
        `[(cons prec (cons order (cons func '())))])
    return none scope

inline make-expand-define-infix (order)
    fn (args scope)
        expand-define-infix args scope order

fn get-ifx-op (env op)
    '@ env `[(get-ifx-symbol (as op Symbol))]

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
            try  (get-ifx-op infix-table token)
            except (err)
                hide-traceback;
                error@ ('anchor token) "while attempting to parse infix token"
                    .. "unexpected token '"
                        .. (tostring token) "' in infix expression"
        let op-prec = (unpack-infix-op op)
        ? (pred op-prec prec) op `none

let infix-op-gt = (infix-op >)
let infix-op-ge = (infix-op >=)

fn rtl-infix-op-eq (infix-table token prec)
    let op =
        try (get-ifx-op infix-table token)
        except (err)
            error@ ('anchor token) "while attempting to parse infix token"
                .. "unexpected token '"
                    .. (tostring token) "' in infix expression"
    let op-prec op-order = (unpack-infix-op op)
    if (== op-order '<)
        ? (== op-prec prec) op `none
    else
        `none

fn parse-infix-expr (infix-table lhs state mprec)
    hide-traceback;
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
                let anchor = ('anchor la)
                break
                    'tag `[(list ('tag `op-name anchor) lhs rhs)] anchor
                    state
            let ra __ = ('decons state)
            let lop = (infix-op-gt infix-table ra op-prec)
            let nextop =
                if (== ('typeof lop) Nothing)
                    rtl-infix-op-eq infix-table ra op-prec
                else lop
            if (== ('typeof nextop) Nothing)
                let anchor = ('anchor la)
                break
                    'tag `[(list ('tag `op-name anchor) lhs rhs)] anchor
                    state
            let nextop-prec = (unpack-infix-op nextop)
            let next-rhs next-state =
                this-function infix-table rhs state nextop-prec
            _ next-rhs next-state

let parse-infix-expr =
    static-typify parse-infix-expr Scope Value list i32

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
            '@ env `[(as head-key Symbol)]
        except (err) head-key
    let head =
        try
            '@ (as head type) '__macro
        except (err) head
    if (== ('typeof head) SugarMacro)
        let head = (as head SugarMacro)
        let expr env =
            try
                hide-traceback;
                head topexpr env
            except (err)
                hide-traceback;
                let msg = `"while expanding sugar macro"
                sc_error_append_calltrace err ('tag msg expr-anchor)
                raise err
        return expr env
    elseif (has-infix-ops? env expr)
        let at next = ('decons expr)
        let expr =
            try
                hide-traceback;
                parse-infix-expr env at next 0
            except (err)
                hide-traceback;
                error@+ err ('anchor topexpr-at) "while expanding infix expression"
        return (cons expr topexpr-next) env
    else
        return topexpr env

# install general symbol hook for this scope
# is called for every symbol the expander could not resolve
fn symbol-handler (topexpr env)
    let at next = ('decons topexpr)
    let sxname = at
    let name = (as sxname Symbol)
    let s = (as name string)
    if (>= (countof s) 2:usize)
        let ch = (@ s 0)
        switch ch
        #pass 126:i8 # ~
        pass 64:i8 # @
        #pass 47:i8 # /
        pass 38:i8 # &
        pass 45:i8 # -
        do
            # split
            let anchor = ('anchor sxname)
            let lop = ('tag `[(Symbol (lslice s 1))] anchor)
            let rop = ('tag `[(Symbol (rslice s 1))] (sc_anchor_offset anchor 1))
            return
                cons
                    'tag `[(list lop rop)] anchor
                    next
                env
        default
            ;
    if (dotted-symbol? env name)
        let expr =
            split-dotted-symbol sxname
        return (cons expr next) env
    label skip
        let handler =
            try ('@ env `typed-symbol-handler-symbol)
            except (err)
                merge skip
        return
            cons
                'tag `(handler sxname env) ('anchor sxname)
                next
            env
    return topexpr env

fn quasiquote-list
inline quasiquote-any (x)
    let T = ('typeof x)
    let anchor = ('anchor x)
    if (== T list)
        quasiquote-list (as x list)
    else
        list ('tag `sc_valueref_tag anchor)
            ('tag `anchor anchor)
            list ('tag `sugar-quote anchor) x
fn quasiquote-list (x)
    if (empty? x)
        return (list sugar-quote x)
    let aat next = ('decons x)
    let at = aat
    let anchor = ('anchor at)
    let T = ('typeof at)
    if (== T list)
        let at = (as at list)
        if (not (empty? at))
            let at-at at-next = ('decons at)
            if (== ('typeof at-at) Symbol)
                let at-at = (as at-at Symbol)
                if (== at-at 'unquote-splice)
                    return
                        list `sc_list_join
                            cons ('tag `do anchor) at-next
                            quasiquote-list next
                elseif (== at-at 'square-list)
                    if (> (countof at-next) 1)
                        return
                            list `sc_list_join
                                list ('tag `list anchor) (cons ('tag `_ anchor) at-next)
                                quasiquote-list next
    elseif (== T Symbol)
        let at = (as at Symbol)
        if (== at 'unquote)
            return (cons ('tag `do anchor) next)
        elseif (== at 'square-list)
            return (cons ('tag `do anchor) next)
        elseif (== at 'quasiquote)
            return (quasiquote-list (quasiquote-list next))
    let val = (quasiquote-any aat)
    list cons ('tag `val anchor) (quasiquote-list next)

fn expand-and-or (expr f)
    if (empty? expr)
        error "at least one argument expected"
    elseif (== (countof expr) 1)
        return ('@ expr)
    let expr = ('reverse expr)
    loop (result head = ('decons expr))
        if (empty? head)
            return result
        let at next = ('decons head)
        _ `[(list f (list inline '() result) at)] next

inline make-expand-and-or (f)
    fn (expr)
        expand-and-or expr f

fn ltr-multiop (args target mincount)
    let argc = ('argcount args)
    verify-count argc mincount -1
    'tag
        if (<= argc mincount)
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
        'anchor args

fn rtl-multiop (args target mincount)
    let argc = ('argcount args)
    verify-count argc mincount -1
    'tag
        if (<= argc mincount)
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
        'anchor args

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
        let argkey = ('keyof ('qualified-typeof arg))
        if (== key argkey)
            return
                sc_keyed_new unnamed arg
        + i 1
    `(elsef)

# modules
####

""""A symbol table of type `Scope` which holds configuration options and module
    contents. It is managed by the module import system.

    ``package.path`` holds a list of all search paths in the form of simple
    string patterns. Changing it alters the way modules are searched for in
    the next run stage.

    ``package.modules`` is another scope symbol table mapping full module
    paths to their contents. When a module is first imported, its contents
    are cached in this table. Subsequent imports of the same module will be
    resolved to these cached contents.
let package = (sc_typename_type "scopes.package" typename)
'set-symbols package
    path =
        Value
            list
                .. compiler-dir "/lib/scopes/?.sc"
                .. compiler-dir "/lib/scopes/?/init.sc"
    modules = `[(Scope)]

fn clone-scope-contents (a b)
    """"Join two scopes ``a`` and ``b`` into a new scope so that the
        root of ``a`` descends from ``b``.
    # search first upwards for the root scope of a, then clone a
        piecewise with the cloned scopes as parents
    let parent = ('parent a)
    if (== parent (nullof Scope))
        return ('reparent a b)
    'reparent a
        this-function parent b

'set-symbols Scope
    __.. = (box-pointer (simple-binary-op clone-scope-contents))

fn extract-single-arg (args)
    let argc = ('argcount args)
    verify-count argc 1 1
    'getarg args 0

fn extract-single-type-arg (args)
    let value = (extract-single-arg args)
    if (== ('typeof value) type)
        as value type
    else
        'qualified-typeof value

inline make-const-value-property-function (func)
    spice-macro
        fn (args)
            let value = (extract-single-arg args)
            let val = (func value)
            `val

inline make-const-type-property-function (func)
    spice-macro
        fn (args)
            let val = (extract-single-type-arg args)
            let val = (func val)
            `val

let
    constant? =
        spice-macro
            fn "constant?" (args)
                let value = (extract-single-arg args)
                `[('constant? value)]
    signed? =
        make-const-type-property-function
            fn (T)
                if (== ('kind T) type-kind-integer)
                    'signed? T
                else
                    hide-traceback;
                    error "integer type expected"
    unsized? =
        make-const-type-property-function sc_type_is_unsized
    storageof = (make-const-type-property-function sc_type_storage)
    superof = (make-const-type-property-function sc_typename_type_get_super)
    sizeof = (make-const-type-property-function sc_type_sizeof)
    bitcountof = (make-const-type-property-function sc_type_bitcountof)
    alignof = (make-const-type-property-function sc_type_alignof)
    unqualified = (make-const-type-property-function sc_strip_qualifiers)
    qualifiersof = (make-const-value-property-function sc_value_qualified_type)
    keyof = (make-const-type-property-function sc_type_key)
    uniqueof =
        spice-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 2 2
                let T = ('getarg args 0)
                let index = (as ('getarg args 1) i32)
                let T =
                    if (== ('typeof T) type)
                        as T type
                    else
                        'qualified-typeof T
                `[(sc_unique_type T index)]
    viewof =
        spice-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 1 -1
                let T = ('getarg args 0)
                let T =
                    if (== ('typeof T) type)
                        as T type
                    else
                        'qualified-typeof T
                let T = (sc_view_type T -1)
                loop (T i = T 1)
                    if (== i argc)
                        break `T
                    let index = (as ('getarg args i) i32)
                    repeat (sc_view_type T index) (add i 1)
    returnof =
        make-const-type-property-function
            fn (T)
                let T =
                    if ('function-pointer? T) ('element@ T 0)
                    elseif ('function? T) T
                    else
                        hide-traceback;
                        error "function type expected"
                'return-type T
    offsetof =
        spice-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 2 2
                let value = ('getarg args 0)
                let T =
                    if (== ('typeof value) type)
                        as value type
                    else
                        'qualified-typeof value
                let index = ('getarg args 1)
                let index =
                    if (== ('typeof index) Symbol)
                        sc_type_field_index T (as index Symbol)
                    else
                        as index i32
                let offset = (sc_type_offsetof T index)
                `offset

#del extract-single-arg
#del make-const-type-property-function

let Closure->Accessor =
    spice-macro
        fn "Closure->Accessor" (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let self = ('getarg args 0)
            if (not ('constant? self))
                error "Closure must be constant"
            let self = (as self Closure)
            let self = (bitcast self Accessor)
            `self

let Closure->Generator =
    spice-macro
        fn "Closure->Generator" (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let self = ('getarg args 0)
            if (not ('constant? self))
                error "Closure must be constant"
            let self = (as self Closure)
            let self = (bitcast self Generator)
            `self

let Closure->Collector =
    spice-macro
        fn "Closure->Collector" (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let self = ('getarg args 0)
            if (not ('constant? self))
                error "Closure must be constant"
            let self = (as self Closure)
            let self = (bitcast self Collector)
            `self

# (define name expr ...)
fn expand-define (expr)
    raising Error
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
    .. = (spice-macro (fn (args) (rtl-multiop args `.. 2)))
    + = (spice-macro (fn (args) (ltr-multiop args `+ 2)))
    * = (spice-macro (fn (args) (ltr-multiop args `* 2)))
    @ = (spice-macro (fn (args) (ltr-multiop args `@ 1)))
    | = (spice-macro (fn (args) (ltr-multiop args `| 2)))
    & = (spice-macro (fn (args) (ltr-multiop args `& 1)))
    va-option-branch = (spice-macro va-option-branch)
    sugar-set-scope! =
        sugar-scope-macro
            fn (args sugar-scope)
                raising Error
                let scope rest = (decons args)
                return none
                    as scope Scope

inline select-op-macro (sop uop fop numargs)
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
                if (type== ('superof T) integer)
                    if ('signed? T) `sop
                    else `uop
                elseif (type== ('superof T) real) `fop
                else
                    error
                        sc_string_join "invalid argument type: "
                            sc_string_join (sc_value_repr (box-pointer T))
                                ". integer or real vector or scalar expected"
            `(fun a b)

let sabs =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let arg = ('getarg args 0)
            let T = ('storageof ('typeof arg))
            if (not (< T integer))
                error "integer expected"
            let bits = ('bitcount T)
            let shift = (sc_const_int_new T (as (- bits 1) u64))
            spice-quote
                let mask = (ashr arg shift)
                sub (bxor arg mask) mask

let pow = **
let abs = (select-op-macro sabs _ fabs 1)
let sign = (select-op-macro ssign ssign fsign 1)

let hash = (sc_typename_type "hash" typename)
'set-plain-storage hash u64

# final `not` - this one folds the constant
let not =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let value = ('getarg args 0)
            let conv = (imply-converter ('typeof value) bool ('constant? value))
            let value =
                if (operator-valid? conv)
                    hide-traceback;
                    sc_prove ('tag `(conv value) ('anchor value))
                else value
            if ('constant? value)
                `[(not (unbox-integer value bool))]
            else
                `(not value)

let list-handler =
    box-pointer (static-typify list-handler list Scope)
let symbol-handler =
    box-pointer (static-typify symbol-handler list Scope)
indirect-let list-handler-symbol = list-handler
indirect-let symbol-handler-symbol = symbol-handler

run-stage; # 5

inline make-inplace-let-op (op)
    sugar-macro
        fn expand-infix-let (expr)
            raising Error
            let name value = (decons expr 2)
            qq [let] [name] = ([op] [name] [value])

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

    := =
        sugar-macro
            fn expand-infix-let (expr)
                raising Error
                let name value = (decons expr 2)
                qq [let] [name] = [value]
    as:= = (make-inplace-let-op as)
    <- =
        sugar-macro
            fn expand-apply (expr)
                raising Error
                let f args = (decons expr 2)
                qq ([f] [args])

#define-infix< 40 , _

define-infix< 50 -> inline
define-infix< 50 =
define-infix< 50 +=; define-infix< 50 -=; define-infix< 50 *=; define-infix< 50 /=
define-infix< 50 //=; define-infix< 50 %=
define-infix< 50 >>=; define-infix< 50 <<=
define-infix< 50 &=; define-infix< 50 |=; define-infix< 50 ^=
define-infix< 50 ..=

define-infix< 50 :=
define-infix< 50 as:=

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
define-infix> 800 <-

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
# collector
#---------------------------------------------------------------------------

'set-symbols Collector
    __typecall =
        inline "Collector-new" (cls init valid? at collect)
            Closure->Collector
                inline "get-init" ()
                    _ init valid? at collect
    __call =
        spice-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 1 1
                let self = ('getarg args 0)
                if (not ('constant? self))
                    error "Generator must be constant"
                let self = (self as Collector)
                let self = (bitcast self Closure)
                `(self)

#---------------------------------------------------------------------------
# for iterator
#---------------------------------------------------------------------------

'set-symbols Generator
    __typecall =
        inline "Generator-new" (cls start valid? at next)
            """"Takes four functions ``start``, ``valid?``, ``at`` and ``next``
                and returns a new generator ready for use.
            Closure->Generator
                inline "get-iter-init" ()
                    _ start valid? at next
    __call =
        spice-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 1 1
                let self = ('getarg args 0)
                if (not ('constant? self))
                    error "Generator must be constant"
                let self = (self as Generator)
                let self = (bitcast self Closure)
                `(self)

'set-docstring Generator '__call
    """".. spice:: (__call self)

           Returns, in this order, the four functions ``start``, ``valid?``,
           ``init`` and ``next`` which are required to enumerate generator
           `self`.

# typical pattern for a generator:
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

fn next-head? (next)
    if (not (empty? next))
        let expr next = (decons next)
        if (('typeof expr) == list)
            let at = ('@ (expr as list))
            if (('typeof at) == Symbol)
                return (at as Symbol) ('anchor at)
    _ unnamed unknown-anchor

"""".. sugar:: (for name ... _:in gen body...)

    Defines a loop that enumerates all elements in collection or sequence
    `gen`, unpacking each element and binding its arguments to the names
    defined by `name ...`.

    `gen` must either be of type `Generator` or provide a cast to
    `Generator`.

    Within the loop body, special forms ``break`` and ``continue`` can be used
    to abort the loop early or skip ahead to the next element. The loop
    will always evaluate to no arguments.

    For a loop form that permits you to maintain additional state and break
    with a value, see `fold`.

    Usage example::

        # print numbers from 0 to 9, skipping number 5
        for i in (range 100)
            if (i == 10)
                # abort the loop
                break;
            if (i == 5)
                # skip this index
                continue;
            print i
define for
    sugar-block-scope-macro
        fn "expand-for" (topexpr scope)
            let expr next-expr = (decons topexpr)
            let else-head else-head-anchor = (next-head? next-expr)
            let has-else? = (else-head == 'else)
            let else-body next-expr =
                if has-else?
                    let at next-expr = (decons next-expr)
                    let block = (at as list)
                    let at block = (decons block)
                    _ block next-expr
                else
                    _ '() next-expr
            let expr = (expr as list)
            let head args = (decons expr)
            let it params =
                loop (it params = args '())
                    if (empty? it)
                        error "'in' expected"
                    let sxat it = (decons it)
                    let at = (sxat as Symbol)
                    if (at == 'in)
                        break it params
                    _ it (cons sxat params)
            let generator-expr body = (decons it)
            let subscope = (Scope scope)
            let generator-expr-anchor = ('anchor generator-expr)
            let generator-expr _next subscope = (sc_expand generator-expr '() subscope)
            spice-quote
                let start valid? at next =
                    (as generator-expr Generator);
            let result =
                cons
                    spice-quote start valid? at next # order expressions
                        loop (it... = (start))
                            if (valid? it...)
                                let args... = (at it...)
                                inline continue ()
                                    spice-unquote
                                        'tag
                                            spice-quote
                                                repeat (next it...)
                                            generator-expr-anchor
                                spice-unquote
                                    let expr =
                                        loop (params expr = params (list '= args...))
                                            if (empty? params)
                                                break expr
                                            let param next = (decons params)
                                            _ next (cons param expr)
                                    let expr = (cons let expr)
                                    let subscope =
                                        'bind subscope 'continue continue
                                    let value = (sc_expand (cons do expr body) '() subscope)
                                    'tag value ('anchor head)
                                continue;
                            else
                                spice-unquote
                                    if has-else?
                                        let value = (sc_expand (cons do else-body) '() subscope)
                                        'tag `(break [value]) else-head-anchor
                                    else
                                        `(break)
                    next-expr
            return result scope

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
                        error
                            .. "can't hash storage of type " (repr OT)
            `(bitcast (sc_hash conv_u64 [('sizeof T)]) hash)

'set-symbols hash
    __hash = (inline (self) self)
    __== = integer.__==
    __!= = integer.__!=
    __as =
        spice-cast-macro
            fn "hash-as" (vT T)
                let ST = ('storageof vT)
                if (T == ST)
                    return `(inline (self) (bitcast self T))
                elseif (T == integer)
                    return `(inline (self) (bitcast self ST))
                `()
    __ras =
        spice-cast-macro
            fn "hash-as" (vT T)
                if (vT == ('storageof vT))
                    return `(inline (self) (bitcast self T))
                `()
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
                        ltr-multiop ('getarglist args 1) hash2 2
    from-bytes =
        inline "hash.from-bytes" (data size)
            bitcast (sc_hashbytes data size) hash

'set-symbols Nothing
    __hash =
        inline "Nothing.__hash" (self) (nullof hash)

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
            raising Error
            let argc = ('argcount args)
            if (argc == 1)
                let arg = ('getarg args 0)
                if (('typeof arg) == CompileStage)
                    return arg
            ``args

let incomplete = (typename "incomplete")

run-stage; # 6

let question-mark-char = 63:i8 # "?"
fn make-module-path (pattern name)
    let sz = (countof pattern)
    loop (i start result = 0:usize 0:usize "")
        if (i == sz)
            return (.. result (rslice pattern start))
        if ((@ pattern i) != question-mark-char)
            repeat (i + 1:usize) start result
        else
            repeat (i + 1:usize) (i + 1:usize)
                .. result (rslice (lslice pattern i) start) name

fn exec-module (expr eval-scope)
    let ModuleFunctionType = (pointer (raises (function Value) Error))
    let StageFunctionType = (pointer (raises (function CompileStage) Error))
    let expr-anchor = ('anchor expr)
    let f =
        do
            hide-traceback;
            sc_eval expr-anchor (expr as list) eval-scope
    loop (f expr-anchor = f expr-anchor)
        # build a wrapper
        let wrapf =
            spice-quote
                fn "exec-module-stage" ()
                    raising Error
                    hide-traceback;
                    wrap-if-not-run-stage (f)
        let path =
            .. ((sc_anchor_path expr-anchor) as string) ":"
                tostring `[(sc_anchor_lineno expr-anchor)]
        sc_template_set_name wrapf (Symbol path)
        let wrapf = (sc_typify_template wrapf 0 (undef TypeArrayPointer))
        let f =
            do
                hide-traceback;
                sc_compile wrapf
                    compile-flag-cache
                    # can't use this flag yet because it breaks code
                    #| compile-flag-cache compile-flag-O2
        if (('typeof f) == StageFunctionType)
            let fptr = (f as StageFunctionType)
            let result =
                do
                    hide-traceback;
                    fptr;
            let result = (bitcast result Value)
            repeat result ('anchor result)
        else
            let fptr = (f as ModuleFunctionType)
            let result =
                do
                    hide-traceback;
                    fptr;
            break result

let slash-char = 47:i8 # "/"
let backslash-char = 92:i8 # "\"
fn dots-to-slashes (pattern)
    let sz = (countof pattern)
    loop (i start result = 0:usize 0:usize "")
        if (i == sz)
            return (.. result (rslice pattern start))
        let c = (@ pattern i)
        if (c == slash-char)
            error
                .. "no slashes permitted in module name: " pattern
        elseif (c == backslash-char)
            error
                .. "no slashes permitted in module name: " pattern
        elseif (c != dot-char)
            repeat (i + 1:usize) start result
        elseif (icmp== (i + 1:usize) sz)
            error
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
        hide-traceback;
        error
            .. "no such module: " module-path
    let module-path = (sc_realpath module-path)
    let module-dir = (sc_dirname module-path)
    let expr =
        do
            hide-traceback;
            sc_parse_from_path module-path
    let eval-scope =
        'bind-symbols
            va-option scope opts...
                do
                    sc_scope_new_subscope_with_docstring (sc_get_globals) ""
            main-module? =
                va-option main-module? opts... false
            module-path = module-path
            module-dir = module-dir
            module-name = module-name
    try
        hide-traceback;
        exec-module expr (Scope eval-scope)
    except (err)
        hide-traceback;
        error@+ err unknown-anchor
            "while loading module " .. module-path

fn patterns-from-namestr (base-dir namestr)
    # if namestr starts with a slash (because it started with a dot),
        we only search base-dir
    if ((@ namestr 0:usize) == slash-char)
        list
            .. base-dir "?.sc"
            .. base-dir "?/init.sc"
    else
        ('@ package 'path) as list

inline slice (value start end)
    rslice (lslice value end) start

fn require-from (base-dir name)
    #assert-typeof name Symbol
    let namestr = (dots-to-slashes (name as string))
    let all-patterns = (patterns-from-namestr base-dir namestr)
    loop (patterns = all-patterns)
        if (empty? patterns)
            hide-traceback;
            error
                .. "failed to import module '" (repr name) "'\n"
                    \ "no such module '" (as name string) "' in paths:"
                    loop (patterns str = all-patterns "")
                        if (empty? patterns)
                            break str
                        let pattern patterns = (decons patterns)
                        let pattern = (pattern as string)
                        let module-path = (make-module-path pattern namestr)
                        repeat patterns
                            .. str "\n"  "    " module-path
        let pattern patterns = (decons patterns)
        let pattern = (pattern as string)
        let module-path = (sc_realpath (make-module-path pattern namestr))
        if (empty? module-path)
            repeat patterns
        let module-path-sym = (Symbol module-path)
        fn get-modules () (('@ package 'modules) as Scope)
        fn get-modules-path (symbol) ('@ (get-modules) symbol)
        fn set-modules-path (symbol value)
            'set-symbol package 'modules
                'bind (get-modules) symbol value
        let content =
            try (get-modules-path module-path-sym)
            except (err)
                if (not (sc_is_file module-path))
                    repeat patterns
                set-modules-path module-path-sym incomplete
                let content =
                    do
                        hide-traceback;
                        load-module (name as string) module-path
                set-modules-path module-path-sym content
                return content
        if (('typeof content) == type)
            if (content == incomplete)
                error
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
                    if ((@ namestr i) == dot-char)
                        if (i == start)
                            repeat (add i 1:usize) (add i 1:usize)
                    repeat (add i 1:usize) start
            let sxname rest = (decons args)
            let name = (sxname as Symbol)
            let namestr = (name as string)
            let module-dir = (('@ scope 'module-dir) as string)
            let key = (resolve-scope scope namestr 0:usize)
            let module =
                do
                    hide-traceback;
                    require-from module-dir name
            _ module
                'bind scope key module

#---------------------------------------------------------------------------
# using
#---------------------------------------------------------------------------

fn merge-scope-symbols (source target filter)
    fn process-keys (source target filter)
        loop (last-index target = -1 target)
            let key value index = ('next source last-index)
            if (index < 0)
                break target
            else
                if
                    or
                        none? filter
                        and (('typeof key) == Symbol)
                            do
                                let keystr = (key as Symbol as string)
                                let ok = ('match? filter keystr)
                                ok
                    repeat index
                        'bind target key value
                else
                    repeat index target
    fn filter-contents (source target filter)
        let parent = ('parent source)
        if (parent == null)
            return
                process-keys source target filter
        process-keys source
            this-function parent target filter
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
                hide-traceback;
                let module = ((require-from module-dir name) as Scope)
                return (list)
                    .. module sugar-scope

            let pattern =
                if (empty? rest)
                    '()
                else
                    let token pattern rest = (decons rest 2)
                    let token = (token as Symbol)
                    if (token != 'filter)
                        error
                            "syntax: using <scope> [filter <filter-string>]"
                    let value = ((sc_expand pattern rest sugar-scope) as string)
                    list value
            # attempt to import directly if possible
            inline process (src)
                _ (list do)
                    if (empty? pattern)
                        merge-scope-symbols src sugar-scope none
                    else
                        merge-scope-symbols src sugar-scope (('@ pattern) as string)
            let nameval = (sc_expand nameval '() sugar-scope)
            if (('typeof nameval) == Scope)
                return (process (nameval as Scope))
            let nameval = (sc_prove nameval)
            if (not ('constant? nameval))
                hide-traceback;
                error "argument passed to `using` must be constant at sugar time"
            let nameval =
                if (('typeof nameval) == type)
                    hide-traceback;
                    '@ (nameval as type) '__using
                else nameval
            if (('typeof nameval) == Scope)
                return (process (nameval as Scope))
            hide-traceback;
            error "using: scope expected"
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
            raising Error
            let name body = (decons expr)
            list define name
                list sugar-macro
                    cons fn '(args)
                        list raising Error
                        body

let __static-assert =
    spice-macro
        fn "__static-assert" (args)
            let argc = ('argcount args)
            verify-count argc 2 2
            let expr msg =
                'getarg args 0
                'getarg args 1
            let msg = (msg as string)
            let val = (expr as bool)
            if (not val)
                hide-traceback;
                error
                    .. "assertion failed: " msg
            `()

let __assert =
    spice-macro
        fn "__assert" (args)
            inline check-assertion (result anchor msg)
                if (not result)
                    print anchor
                        .. "assertion failed: " msg
                    sc_set_signal_abort true
                    sc_abort;

            let argc = ('argcount args)
            verify-count argc 2 2
            let expr msg =
                'getarg args 0
                'getarg args 1
            if (('typeof msg) != string)
                error "string expected as second argument"
            let anchor = ('anchor args)
            'tag `(check-assertion expr anchor msg) anchor

fn gen-vector-reduction (f v sz)
    returning Value
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
                    f (extractelement v 0) (extractelement v 1)
        case 3
            break
                spice-quote
                    f (f (extractelement v 0) (extractelement v 1))
                        extractelement v 2
        case 4
            break
                spice-quote
                    f
                        f (extractelement v 0) (extractelement v 1)
                        f (extractelement v 2) (extractelement v 3)
        default
            if ((sz & 1) == 0)
                # clean pow2 slice
                let hsz = (sz >> 1)
                let hsz-value = (hsz as usize)
                repeat
                    spice-quote
                        f
                            lslice v hsz-value
                            rslice v hsz-value
                    hsz
            else
                # split into even sum and 1
                let rhs = `(rslice v 1)
                let rhs = (this-function f rhs (sz - 1))
                break `(f (extractelement v 0) rhs)

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
            gen-vector-reduction f v sz

let __countof-aggregate =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let self = ('getarg args 0)
            let T = ('typeof self)
            let sz = ('element-count T)
            `[(sz as usize)]

let key =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 2 2
            let sym = (('getarg args 0) as Symbol)
            let value = ('getarg args 1)
            sc_keyed_new sym value

run-stage; # 7

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
            cons fn '(topexpr sugar-scope)
                list let 'expr 'next-expr '= (list decons 'topexpr)
                list let 'expr '= (list (do as) 'expr list)
                body

'set-symbols type
    symbols =
        inline "symbols" (self)
            Generator
                inline () (sc_type_next self unnamed)
                inline (key value) (key != unnamed)
                inline (key value) (_ key value)
                inline (key value) (sc_type_next self key)
    elements =
        inline "elements" (self)
            let count = ('element-count self)
            Generator
                inline () 0
                inline (i) (i < count)
                inline (i) ('element@ self i)
                inline (i) (i + 1)

do
    inline lineage-generator (self)
        Generator
            inline () self
            inline (self) (self != null)
            inline (self) self
            inline (self)
                sc_scope_get_parent self

    inline scope-generator (self)
        Generator
            inline () (sc_scope_next self -1)
            inline (key value index) (index >= 0)
            inline (key value index) (_ key value)
            inline (key value index)
                sc_scope_next self index

    'set-symbols Scope
        lineage = lineage-generator
        deleted =
            inline (self)
                Generator
                    inline () (sc_scope_next_deleted self -1)
                    inline (key index) (index >= 0)
                    inline (key index) key
                    inline (key index)
                        sc_scope_next_deleted self index
        __as =
            spice-cast-macro
                fn "scope-as" (vT T)
                    if (T == Generator)
                        return `scope-generator
                    `()

do
    inline string-generator (self)
        let buf sz = ('buffer self)
        Generator
            inline () 0:usize
            inline (i) (i < sz)
            inline (i) (load (getelementptr buf i))
            inline (i) (i + 1:usize)

    inline string-generator-range (self start end)
        start := start as usize
        let buf sz = ('buffer self)
        let end =
            static-branch (none? end)
                inline () sz
                inline ()
                    end := end as usize
                    ? (sz < end) sz end
        Generator
            inline () start
            inline (i) (i < end)
            inline (i) (load (getelementptr buf i))
            inline (i) (i + 1:usize)

    inline string-collector (maxsize)
        let buf = (alloca-array i8 maxsize)
        Collector
            inline () 0
            inline (n) (n < maxsize)
            inline (n)
                sc_string_new buf (n as usize)
            inline (src n)
                store (src) (getelementptr buf n)
                n + 1

    fn i8->string(c)
        let ptr = (alloca i8)
        store c ptr
        sc_string_new ptr 1

    'set-symbols string
        collector = string-collector
        range = string-generator-range
        __hash =
            inline (self)
                hash.from-bytes ('buffer self)
        __ras =
            spice-cast-macro
                fn "string-as" (vT T)
                    if (vT == i8)
                        return `i8->string
                    `()
        __as =
            spice-cast-macro
                fn "string-as" (vT T)
                    if (T == Generator)
                        return `string-generator
                    `()

do
    inline list-generator (self)
        Generator
            inline () self
            inline (cell) (not (empty? cell))
            inline (cell) (sc_list_at cell)
            inline (cell) (sc_list_next cell)

    inline list-collector (self)
        Collector
            inline () self
            inline (it) true
            inline (it) ('reverse it)
            inline (src it)
                cons (src) it

    'set-symbols list
        cons-sink =
            inline "list-collector" (self)
                Collector
                    inline () self
                    inline (it) true
                    inline (it) it
                    inline (src it)
                        cons (src) it
        __as =
            spice-cast-macro
                fn "list-as" (vT T)
                    if (T == Generator)
                        return `list-generator
                    elseif (T == Collector)
                        return `list-collector
                    `()

'set-symbols Value
    args =
        inline "Value-args" (self)
            let argc = ('argcount self)
            Generator
                inline () 0
                inline (x) (x < argc)
                inline (x) ('getarg self x)
                inline (x) (x + 1)
    reverse-args =
        inline "Value-reverse-args" (self)
            let argc = ('argcount self)
            Generator
                inline () argc
                inline (x) (x > 0)
                inline (x) ('getarg self (x - 1))
                inline (x) (x - 1)
    arglist-sink =
        inline "arglist-sink" (N)
            let values = (alloca-array Value N)
            Collector
                inline () 0
                inline (i) (i < N)
                inline (i)
                    sc_argument_list_new i values
                inline (src i)
                    store (sc_identity (src)) (getelementptr values i)
                    add i 1

inline range (a b c)
    let num-type = (typeof a)
    let step =
        static-branch (none? c)
            inline () (1 as num-type)
            inline () c
    let from =
        static-branch (none? b)
            inline () (0 as num-type)
            inline () a
    let to =
        static-branch (none? b)
            inline () a
            inline () b
    Generator
        inline () (deref from)
        inline (x) (x < to)
        inline (x) x
        inline (x) (x + step)

inline rrange (a b c)
    """"same as range, but iterates range in reverse; arguments are passed
        in the same format, so rrange can act as a drop-in replacement for range.
    let num-type = (typeof a)
    let step =
        static-branch (none? c)
            inline () (1 as num-type)
            inline () c
    let from =
        static-branch (none? b)
            inline () (0 as num-type)
            inline () a
    let to =
        static-branch (none? b)
            inline () a
            inline () b
    let to = (((to - from + (step - 1)) // step) * step + from)
    Generator
        inline () (_ to (to - step))
        inline (x0 x-1) (x0 > from)
        inline (x0 x-1) x-1
        inline (x0 x-1) (_ x-1 (x-1 - step))

let parse-compile-flags =
    spice-macro
        fn (args)
            inline flag-error (flag)
                error
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

    inline compile-glsl (version target func flags...)
        sc_compile_glsl version target func (parse-compile-flags flags...)

    inline compile-spirv (target func flags...)
        sc_compile_spirv target func (parse-compile-flags flags...)

    inline compile-object (target file-kind path table flags...)
        sc_compile_object target file-kind path table (parse-compile-flags flags...)

inline convert-assert-args (args cond msg)
    if ((countof args) == 2) msg
    else
        if (('typeof cond) == list)
            `[(sc_list_repr (cond as list))]
        else
            `[('__repr cond)]

define-sugar-macro static-assert
    let cond msg body = (decons args 2)
    let anchor = ('anchor cond)
    let msg = (convert-assert-args args cond msg)
    list ('tag `__static-assert anchor) cond msg

define-sugar-macro assert
    let cond msg body = (decons args 2)
    let msg = (convert-assert-args args cond msg)
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
            sc_argument_list_map_new count
                inline (i)
                    let argT = ('element@ T i)
                    `(extractf self i)

let __unpack-aggregate = (make-unpack-function extractvalue)

let __unpack-keyed-aggregate =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 1
            let self = ('getarg args 0)
            let T = ('typeof self)
            let count = ('element-count T)
            sc_argument_list_map_new count
                inline (i)
                    let argT = ('element@ T i)
                    let key = ('keyof argT)
                    sc_keyed_new key `(extractvalue self i)

'set-symbols tuple
    __unpack = __unpack-keyed-aggregate
    __countof = __countof-aggregate
    __getattr = extractvalue
    __@ = extractvalue

inline gen-tupleof (type-func)
    spice-macro
        fn (args)
            let argc = ('argcount args)
            #verify-count argc 0 -1
            raising Error

            # build tuple type and values
            # also check if all arguments are constant
            let values = (alloca-array Value argc)
            let field-types = (alloca-array type argc)
            let const? =
                loop (i const? = 0 true)
                    if (i == argc)
                        break const?
                    let k arg = ('dekey ('getarg args i))
                    store arg (getelementptr values i)
                    let T = ('key-type ('typeof arg) k)
                    store T (getelementptr field-types i)
                    _ (i + 1) (const? & ('constant? arg))

            let TT = (type-func argc field-types)
            if const?
                sc_const_aggregate_new TT argc values
            else
                # generate insert instructions
                loop (i result = 0 `(nullof TT))
                    if (i == argc)
                        break result
                    let arg = (load (getelementptr values i))
                    _ (i + 1)
                        `(insertvalue result arg i)

let tupleof = (gen-tupleof sc_tuple_type)
let packedtupleof = (gen-tupleof sc_packed_tuple_type)

#-------------------------------------------------------------------------------
# arrays
#-------------------------------------------------------------------------------

'set-symbols array
    __== =
        simple-binary-op
            spice-macro
                fn (args)
                    let self = ('getarg args 0)
                    let other = ('getarg args 1)
                    let block = (sc_if_new)
                    for i in (range ('element-count ('typeof self)))
                        sc_if_append_then_clause block
                            `((self @ i) != (other @ i))
                            `false
                    sc_if_append_else_clause block `true
                    block
    __unpack = __unpack-aggregate
    __countof = __countof-aggregate
    __@ =
        inline (self index)
            extractvalue self index
    # dynamic array constructor
    type =
        inline "array.type" (element-type size)
            sc_array_type element-type (size as usize)
    # static array constructor
    __typecall =
        spice-macro
            fn "array.__typecall" (args)
                let argc = ('argcount args)
                verify-count argc 1 3
                raising Error
                let cls = (('getarg args 0) as type)
                if (cls == array)
                    verify-count argc 2 3
                    let element-type = (('getarg args 1) as type)
                    let size =
                        if (argc == 2) -1:u64
                        else (extract-integer ('getarg args 2))
                    `[(sc_array_type element-type (size as usize))]
                else
                    verify-count argc 1 1
                    `(nullof cls)
    __imply =
        do
            inline arrayref->pointer (T)
                inline (self)
                    bitcast (reftoptr self) T
            spice-cast-macro
                fn "array.__imply" (cls T)
                    # &(array T n) -> @T
                    if ('refer? cls)
                        if ('pointer? T)
                            let clsET = ('element@ cls 0)
                            let TET = ('element@ T 0)
                            if (== clsET TET)
                                return `(arrayref->pointer T)
                    `()
    __as =
        do
            inline array-generator (arr)
                # arr must be a reference
                static-branch (&? arr)
                    inline ()
                    inline ()
                        static-error "only array references can be cast to generator"
                let count = (countof arr)
                Generator
                    inline () 0:usize
                    inline (x) (< x count)
                    inline (x) (@ arr x)
                    inline (x) (+ x 1:usize)
            spice-cast-macro
                fn "array.__as" (vT T)
                    if (T == Generator)
                        return `array-generator
                    `()
    __rimply =
        do
            inline passthru (self) self
            spice-cast-macro
                fn "array.__rimply" (cls T)
                    if ('unsized? T)
                        if (('kind cls) == type-kind-array)
                            if (('element@ cls 0) == ('element@ T 0))
                                return `passthru
                    `()

    __typematch =
        spice-cast-macro
            fn "array.__typematch" (cls T)
                if ('unsized? cls) # unsized array
                    if (('kind T) == type-kind-array)
                        if (('element@ T 0) == ('element@ cls 0))
                            return `true
                `false

inline gen-arrayof (gentypef insertop)
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 1 -1
            raising Error

            let ET = (('getarg args 0) as type)
            let numvals = (sub argc 1)

            let values = (alloca-array Value numvals)
            # check if all arguments are constant and convert accordingly
            let const? =
                loop (i const? = 0 true)
                    if (i == numvals)
                        break const?
                    let arg = ('getarg args (add i 1))
                    let arg =
                        if ((sc_value_type arg) == ET) arg
                        else
                            hide-traceback;
                            sc_prove ('tag `(arg as ET) ('anchor arg))
                    store arg (getelementptr values i)
                    _ (i + 1) (const? & ('constant? arg))

            let TT = (gentypef ET (usize numvals))
            if const?
                sc_const_aggregate_new TT numvals values
            else
                # generate insert instructions
                loop (i result = 0 `(nullof TT))
                    if (i == numvals)
                        break result
                    let arg = (load (getelementptr values i))
                    _ (i + 1) `(insertop result arg i)

let arrayof = (gen-arrayof sc_array_type insertvalue)

# destructors for aggregates

'set-symbols aggregate
    __drop =
        spice-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 1 1
                let self = ('getarg args 0)
                let T = ('typeof self)
                if (not ('plain? T))
                    let block = (sc_expression_new)
                    let count = ('element-count T)
                    # extract all values and drop em
                    loop (i = 0)
                        if (i == count) (break)
                        sc_expression_append block `(__drop (extractvalue self i))
                        i + 1
                    sc_expression_append block `()
                    return block
                `()

#-------------------------------------------------------------------------------
# vectors
#-------------------------------------------------------------------------------

fn any? (v)
    vector-reduce bor v
fn all? (v)
    vector-reduce band v

inline signed-vector-binary-op (sf uf)
    spice-macro
        fn (args)
            raising Error
            let argc = (sc_argcount args)
            verify-count argc 2 2
            let lhs = ('getarg args 0)
            let rhs = ('getarg args 1)
            let lhsT = ('typeof lhs)
            let T = ('element@ lhsT 0)
            if ('signed? T)
                `(sf lhs rhs)
            else
                `(uf lhs rhs)

'set-symbols integer
    __vector+  = add
    __vector-  = sub
    __vector*  = mul
    __vector// = (signed-vector-binary-op sdiv udiv)
    __vector%  = (signed-vector-binary-op srem urem)
    __vector&  = band
    __vector|  = bor
    __vector^  = bxor
    __vector<< = shl
    __vector>> = (signed-vector-binary-op ashr lshr)
    __vector== = icmp==
    __vector!= = icmp!=
    __vector>  = (signed-vector-binary-op icmp>s icmp>u)
    __vector>= = (signed-vector-binary-op icmp>=s icmp>=u)
    __vector<  = (signed-vector-binary-op icmp<s icmp<u)
    __vector<= = (signed-vector-binary-op icmp<=s icmp<=u)

'set-symbols real
    __vector+  = fadd
    __vector-  = fsub
    __vector*  = fmul
    __vector** = powf
    __vector/  = fdiv
    __vector%  = frem
    __vector== = fcmp==o
    __vector!= = fcmp!=u
    __vector>  = fcmp>o
    __vector>= = fcmp>=o
    __vector<  = fcmp<o
    __vector<= = fcmp<=o

fn vector-binary-operator (symbol lhsT rhsT)
    label next
        if (ptrcmp== lhsT rhsT)
            let ET = ('element@ lhsT 0)
            let f =
                try ('@ ET symbol)
                except (err)
                    merge next
            return f
    `()

inline vector-binary-op-dispatch (symbol)
    spice-binary-op-macro
        inline (lhsT rhsT) (vector-binary-operator symbol lhsT rhsT)

'set-symbols vector
    __+ = (vector-binary-op-dispatch '__vector+)
    __- = (vector-binary-op-dispatch '__vector-)
    __* = (vector-binary-op-dispatch '__vector*)
    __** = (vector-binary-op-dispatch '__vector**)
    __/ = (vector-binary-op-dispatch '__vector/)
    __// = (vector-binary-op-dispatch '__vector//)
    __% = (vector-binary-op-dispatch '__vector%)
    __& = (vector-binary-op-dispatch '__vector&)
    __| = (vector-binary-op-dispatch '__vector|)
    __^ = (vector-binary-op-dispatch '__vector^)
    __<< = (vector-binary-op-dispatch '__vector<<)
    __>> = (vector-binary-op-dispatch '__vector>>)
    __== = (vector-binary-op-dispatch '__vector==)
    __!= = (vector-binary-op-dispatch '__vector!=)
    __> = (vector-binary-op-dispatch '__vector>)
    __>= = (vector-binary-op-dispatch '__vector>=)
    __< = (vector-binary-op-dispatch '__vector<)
    __<= = (vector-binary-op-dispatch '__vector<=)
    smear =
        spice-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 2 2
                let value size =
                    'getarg args 0
                    'getarg args 1
                let ET = ('typeof value)
                let n = (size as i32)
                let N = (n as usize)
                let T = (sc_vector_type ET N)
                if ('constant? value)
                    let values = (alloca-array Value N)
                    for i in (range N)
                        store value (getelementptr values i)
                    `[(sc_const_aggregate_new T n values)]
                else
                    # vector size must be at least 2 to be SPIR-V compliant
                    let T1 = (sc_vector_type ET 2)
                    let maskT = (sc_vector_type i32 N)
                    let values = (alloca-array Value N)
                    let zero = `0
                    for i in (range N)
                        store zero (getelementptr values i)
                    let mask = (sc_const_aggregate_new maskT n values)
                    spice-quote
                        let vec = (insertelement (nullof T1) value 0)
                        shufflevector vec vec mask
    __lslice =
        spice-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 2 2
                let self offset =
                    'getarg args 0
                    'getarg args 1
                if (not ('constant? offset))
                    error "slice offset must be constant"
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
                    store `i (getelementptr maskvals i)
                    i + 1
                let maskT =
                    sc_vector_type i32 offset:usize
                let mask = (sc_const_aggregate_new maskT offset maskvals)
                `(shufflevector self self mask)
    __rslice =
        spice-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 2 2
                let self offset =
                    'getarg args 0
                    'getarg args 1
                if (not ('constant? offset))
                    error "slice offset must be constant"
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
                    store `[(i + offset)] (getelementptr maskvals i)
                    i + 1
                let maskT =
                    sc_vector_type i32 total:usize
                let mask = (sc_const_aggregate_new maskT total maskvals)
                `(shufflevector self self mask)
    __unpack = `[(make-unpack-function extractelement)]
    __countof = __countof-aggregate
    __@ =
        inline (self index)
            extractelement self index
    # dynamic vector type constructor
    type =
        inline "vector.type" (element-type size)
            sc_vector_type element-type (size as usize)
    # static vector type constructor
    __typecall =
        spice-macro
            fn "vector.__typecall" (args)
                let argc = ('argcount args)
                verify-count argc 1 3
                raising Error
                let cls = (('getarg args 0) as type)
                if (cls == vector)
                    verify-count argc 3 3
                    let element-type = (('getarg args 1) as type)
                    let size = (extract-integer ('getarg args 2))
                    `[(sc_vector_type element-type (size as usize))]
                else
                    verify-count argc 1 1
                    `(nullof cls)

let vectorof = (gen-arrayof sc_vector_type insertelement)

#-------------------------------------------------------------------------------

let
    min =
        spice-macro
            fn (args)
                ltr-multiop args
                    Value
                        inline "min" (a b)
                            ? (<= a b) a b
                    2
    max =
        spice-macro
            fn (args)
                ltr-multiop args
                    Value
                        inline "max" (a b)
                            ? (>= a b) a b
                    2

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
        spice-cast-macro
            fn (srcT destT)
                let func->closure =
                    spice-macro
                        fn (args)
                            let argc = ('argcount args)
                            verify-count argc 2 2
                            let expr = ('getarg args 0)
                            let destT = (('getarg args 1) as type)
                            let funcT = ('element@ destT 0)
                            let sz = ('element-count funcT)
                            let func = (expr as Closure)
                            if ('variadic? funcT)
                                error "cannot typify to variadic function"
                            let args = (alloca-array type sz)
                            for i in (range sz)
                                store ('element@ funcT i) (getelementptr args i)
                            let result =
                                sc_typify func sz args
                            let resultT = ('typeof result)
                            if (resultT != destT)
                                error
                                    .. "function does not compile to type " (repr destT)
                                        \ " but has type " (repr resultT)
                            return result
                if ('function-pointer? destT)
                    return `(inline (self) (func->closure self destT))
                `()

inline extern-new (name T attrs...)
    let flags = (va-option flags attrs... 0:u32)
    let storage-class = (va-option storage-class attrs... unnamed)
    let location = (va-option location attrs... -1)
    let binding = (va-option binding attrs... -1)
    let set = (va-option set attrs... -1)
    let glob = (sc_global_new name T flags storage-class)
    if (location > -1) (sc_global_set_location glob location)
    if (binding > -1) (sc_global_set_binding glob binding)
    if (set > -1) (sc_global_set_descriptor_set glob set)
    glob

let extern =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 2 -1
            raising Error
            let name = (('getarg args 0) as Symbol)
            let T = (('getarg args 1) as type)
            loop (i flags storage-class location binding set = 2 0:u32 unnamed -1 -1 -1)
                if (i == argc)
                    let expr = `[(sc_global_new name T flags storage-class)]
                    let expr =
                        if (location > -1)
                            spice-quote
                                sc_global_set_location expr location
                                expr
                        else expr
                    let expr =
                        if (binding > -1)
                            spice-quote
                                sc_global_set_binding expr binding
                                expr
                        else expr
                    let expr =
                        if (set > -1)
                            spice-quote
                                sc_global_set_descriptor_set expr set
                                expr
                        else expr
                    break expr
                let arg = ('getarg args i)
                let k v = ('dekey arg)
                let flags storage-class location binding set =
                    if (k == unnamed)
                        let k = (arg as Symbol)
                        let newflag =
                            if (k == 'buffer-block) global-flag-buffer-block
                            elseif (k == 'non-writable) global-flag-non-writable
                            elseif (k == 'non-readable) global-flag-non-readable
                            elseif (k == 'volatile) global-flag-volatile
                            elseif (k == 'coherent) global-flag-coherent
                            elseif (k == 'restrict) global-flag-restrict
                            elseif (k == 'block) global-flag-block
                            else
                                error ("unrecognized flag: " .. (repr k))
                        _ (bor flags newflag) storage-class location binding set
                    elseif (k == 'storage-class)
                        _ flags (arg as Symbol) location binding set
                    elseif (k == 'location)
                        _ flags storage-class (arg as i32) binding set
                    elseif (k == 'binding)
                        _ flags storage-class location (arg as i32) set
                    elseif (k == 'set)
                        _ flags storage-class location binding (arg as i32)
                    else
                        error ("unrecognized key: " .. (repr k))
                _ (i + 1) flags storage-class location binding set

let
    private =
        spice-macro
            fn (args)
                let argc = ('argcount args)
                verify-count argc 1 1
                let T = ('getarg args 0)
                let T = (T as type)
                extern-new unnamed T (storage-class = 'Private)

#-------------------------------------------------------------------------------

fn extract-name-params-body (expr)
    let arg body = (decons expr)
    if (('typeof arg) == list)
        return `"" (arg as list) body
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

inline gen-match-block-parser (handle-case)
    sugar-block-scope-macro
        fn (topexpr scope)
            let expr next = (decons topexpr)
            let expr = (expr as list)
            let head arg argrest = (decons expr 2)
            let arg argrest scope = (sc_expand arg argrest scope)
            let outnext = (alloca-array list 1)
            let outexpr next =
                spice-quote
                    label ok-label
                        spice-unquote
                            let outexpr = (sc_expression_new)
                            loop (next = next)
                                let head head-anchor = (next-head? next)
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
                                                    let unpack-expr newscope =
                                                        handle-case fail-case token newscope cond
                                                    let body =
                                                        try
                                                            hide-traceback;
                                                            sc_expand (cons do body) '() newscope
                                                        except (err)
                                                            hide-traceback;
                                                            error@+ err head-anchor "while expanding case"
                                                    spice-quote
                                                        unpack-expr
                                                        spice-unquote
                                                            'tag `(merge ok-label body) head-anchor
                                    repeat next
                                case 'default
                                    let expr next = (decons next)
                                    let expr = (expr as list)
                                    let head body = (decons expr)
                                    let body =
                                        try
                                            hide-traceback;
                                            sc_expand (cons do body) '() scope
                                        except (err)
                                            hide-traceback;
                                            error@+ err head-anchor "while expanding default case"
                                    sc_expression_append outexpr
                                        'tag `(merge ok-label body) head-anchor
                                    store next outnext
                                    break outexpr
                                default
                                    hide-traceback;
                                    error "default branch missing"
            return (cons outexpr (load outnext)) scope

fn gen-sugar-matcher (failfunc expr scope params)
    returning Value Scope
    let params = (params as list)
    let paramcount = (countof params)
    let outexpr = (sc_expression_new)
    loop (i rest next varargs scope = 0 params expr false scope)
        if (empty? rest)
            return
                spice-quote
                    if (not (check-count (sc_list_count expr)
                            [(? varargs (sub paramcount 1) paramcount)]
                            [(? varargs -1 paramcount)]))
                        failfunc;
                    outexpr
                scope
        else
            let paramv rest = (decons rest)
            let T = ('typeof paramv)
            if (T == Symbol)
                let param = (paramv as Symbol)
                let variadic? = ('variadic? param)
                let arg next =
                    if variadic?
                        if (not (empty? rest))
                            error
                                "variadic match pattern is not in last place"
                        _ next `[]
                    else
                        spice-quote
                            let arg next =
                                sc_list_decons next
                        _ arg next
                sc_expression_append outexpr arg
                let scope = ('bind scope param arg)
                repeat (i + 1) rest next (| varargs variadic?) scope
            elseif (T == string)
                let str = (paramv as string)
                sc_expression_append outexpr
                    spice-quote
                        let arg next = (sc_list_decons next)
                        if (ptrcmp!= ('typeof arg) string)
                            failfunc;
                        if ((arg as string) != str)
                            failfunc;
                repeat (i + 1) rest next varargs scope
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
                    repeat (i + 1) rest next varargs scope
                elseif ((('typeof mid) == Symbol) and ((mid as Symbol) == 'as))
                    let exprT = (decons mid-rest)
                    let exprT = (sc_expand exprT '() scope)
                    let param = (head as Symbol)
                    if ('variadic? param)
                        error
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
                    let scope = ('bind scope param arg)
                    repeat (i + 1) rest next varargs scope
                elseif ((('typeof mid) == Symbol) and ((mid as Symbol) == 'is))
                    # check that argument is of constant type, but don't unbox
                    let exprT = (decons mid-rest)
                    let exprT = (sc_expand exprT '() scope)
                    let param = (head as Symbol)
                    if ('variadic? param)
                        error
                            "vararg parameter cannot be typed"
                    sc_expression_append outexpr
                        spice-quote
                            let arg next =
                                sc_list_decons next
                            if (not (('constant? arg) and (('typeof arg) == exprT)))
                                failfunc;
                    let scope = ('bind scope param arg)
                    repeat (i + 1) rest next varargs scope
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
                                let expr scope =
                                    this-function failfunc arg scope param
                                expr
                    repeat (i + 1) rest next varargs scope
            else
                hide-traceback;
                error@ ('anchor paramv) "while parsing pattern" "unsupported pattern"

define sugar-match
    gen-match-block-parser gen-sugar-matcher

#-------------------------------------------------------------------------------

define sugar
    inline wrap-sugar-macro (f)
        sugar-block-scope-macro
            fn (topexpr scope)
                let new-expr new-next new-scope = (f topexpr scope)
                let x next = (decons topexpr)
                let anchor = ('anchor x)
                let new-expr = ('tag `new-expr anchor)
                _
                    static-branch (none? new-next)
                        inline ()
                            cons new-expr next
                        inline ()
                            cons new-expr new-next
                    static-branch (none? new-scope)
                        inline () scope
                        inline () new-scope

    sugar-block-scope-macro
        fn "expand-sugar" (topexpr scope)
            raising Error
            let expr next = (decons topexpr)
            let expr = (expr as list)
            let head expr = (decons expr)
            let name params body =
                extract-name-params-body expr
            let func =
                spice-quote
                    inline (topexpr sugar-scope)
                        let _expr next-expr = (decons topexpr)
                        let head expr = (sc_list_decons (_expr as list))
                        label ok-label
                            inline return-ok (args...)
                                merge ok-label args...
                            label fail-label
                                inline fail-case ()
                                    merge fail-label
                                spice-unquote
                                    let subscope = (Scope scope)
                                    let subscope =
                                        'bind-symbols subscope
                                            next-expr = next-expr
                                            sugar-scope = sugar-scope
                                            expr-head = head
                                            expression = _expr
                                    let unpack-expr subscope =
                                        gen-sugar-matcher fail-case expr subscope params
                                    let body =
                                        sc_expand (cons do body) '() subscope
                                    spice-quote
                                        unpack-expr
                                        spice-unquote
                                            'tag `(return-ok body) unknown-anchor
                            hide-traceback;
                            error
                                .. "syntax error: pattern does not match format " (repr params)
            let wrapped =
                qq
                    [wrap-sugar-macro func];
            let wrapped =
                'tag `wrapped ('anchor head)
            let outexpr =
                if (('typeof name) == Symbol)
                    Value
                        qq
                            [let name] = [wrapped]
                else wrapped
            return (cons outexpr next) scope

#-------------------------------------------------------------------------------

fn uncomma (l)
    """"uncomma list l, wrapping all comma separated symbols as new lists
        example::

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
            error "unexpected comma"
        cons
            if ((countof current) == 1) ('@ current)
            else ('tag `current anchor)
            total
    if (comma-separated? l)
        fn process (l)
            raising Error
            if (empty? l)
                return unknown-anchor '() '()
            let at next = (decons l)
            let anchor current total = (this-function next)
            let anchor = ('anchor at)
            if ((('typeof at) == Symbol) and ((at as Symbol) == ',))
                if (empty? next)
                    return anchor '() total
                return anchor '() (merge-lists anchor current total)
            else
                return anchor (cons at current) total
        return (merge-lists (process l))
    else l

#inline spice-macro (f)
    spice-macro-verify-signature f
    bitcast (static-typify f Value) SpiceMacro

define spice
    sugar-macro
        fn "expand-spice" (expr)
            raising Error
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
                                error "vararg parameter is not in last place"
                            cons
                                qq
                                    [let paramv] =
                                        [`sc_getarglist ('tag `args ('anchor paramv)) i];
                                body
                        else
                            cons
                                qq
                                    [let paramv] =
                                        [`sc_getarg ('tag `args ('anchor paramv)) i];
                                body
                    repeat (i + 1) rest body (| varargs variadic?)
                let anchor = ('anchor name)
                let content =
                    cons (list args)
                        'tag
                            Value
                                qq
                                    [verify-count] [(list sc_argcount args)]
                                        [(? varargs (sub paramcount 1) paramcount)]
                                        [(? varargs -1 paramcount)]
                            anchor
                        body
                let _fn = ('tag `fn anchor)
                break
                    if (('typeof name) == Symbol)
                        qq
                            [let name] =
                                [spice-macro]
                                    [_fn] [(name as Symbol as string)] (args)
                                        [Value]
                                            [(cons inline content)] args
                    else
                        qq
                            [spice-macro]
                                [_fn name] (args)
                                    [Value]
                                        [(cons inline content)] args

#-------------------------------------------------------------------------------

fn gen-match-matcher

fn gen-or-matcher (failfunc expr scope params)
    return
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
        scope

fn gen-match-matcher (failfunc expr scope cond)
    """"features:
        <constant> -> (input == <constant>)
        (or <expr_a> <expr_b>) -> (or <expr_a> <expr_b>)

        TODO:
        (: x T) -> ((typeof input) == T), let x = input
        <unknown symbol> -> unpack as symbol
    returning Value Scope
    let condT = ('typeof cond)
    if (condT == list)
        let cond-anchor = ('anchor cond)
        let cond = (uncomma (cond as list))
        let cond =
            if (has-infix-ops? scope cond)
                let at next = ('decons cond)
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
        error (.. "unsupported pattern: " (repr cond))
    let cond =
        sc_expand cond '() scope
    return
        spice-quote
            if (expr != cond)
                failfunc;
        scope

#-------------------------------------------------------------------------------

define match
    gen-match-block-parser gen-match-matcher

let OverloadedFunction = (typename "OverloadedFunction")
'set-opaque OverloadedFunction

"""" (va-append-va (inline () (_ b ...)) a...) -> a... b...
define va-append-va
    spice-macro
        fn "va-va-append" (args)
            raising Error
            let argc = ('argcount args)
            verify-count argc 1 -1
            let end = ('getarg args 0)
            sc_argument_list_map_new argc
                inline (i)
                    let i = (add i 1)
                    if (i == argc)
                        'tag `(end) ('anchor end)
                    else
                        'getarg args i

define va-empty?
    spice-macro
        fn "va-empty?" (args)
            raising Error
            let argc = ('argcount args)
            'tag `[(argc == 0)] ('anchor args)

define va@
    spice-macro
        fn "va@" (args)
            let argc = ('argcount args)
            verify-count argc 1 -1
            let at = (('getarg args 0) as i32)
            'getarg args (at + 1)

"""".. spice:: (va-map f ...)

       Filter each argument in `...` through `f` and return the resulting list
       of arguments. Arguments where `f` returns void are filtered from the
       result.
define va-map
    spice-macro
        fn "va-map" (args)
            #raising Error
            let argc = ('argcount args)
            verify-count argc 1 -1
            let f = ('getarg args 0)
            sc_argument_list_map_filter_new (argc - 1)
                inline (i)
                    let i = (i + 1)
                    let arg = ('getarg args i)
                    let outarg =
                        do
                            hide-traceback;
                            sc_prove ('tag `(f arg) ('anchor arg))
                    _ (('typeof outarg) != void) outarg

"""".. spice:: (va-range a (? b))

       If `b` is not specified, returns a sequence of integers from zero to `b`,
       otherwise a sequence of integers from `a` to `b`.
define va-range
    spice-macro
        fn "va-range" (args)
            #raising Error
            let argc = ('argcount args)
            verify-count argc 1 2
            let a = (('getarg args 0) as i32)
            let a b =
                if (argc == 2)
                    _ a (('getarg args 1) as i32)
                else
                    _ 0 a
            if ((b - a) > unroll-limit)
                hide-traceback;
                error "too many elements specified for range"
            let count = (b - a)
            sc_argument_list_map_new count
                inline (i)
                    let i = (i + a)
                    'tag `i ('anchor args)

"""" (va-split n a...) -> (inline () a...[n .. (va-countof a...)-1]) a...[0 .. n-1]
define va-split
    spice-macro
        fn "va-split" (args)
            raising Error
            let argc = ('argcount args)
            verify-count argc 1 -1
            let pos = (('getarg args 0) as i32)
            let largs =
                sc_argument_list_map_new pos
                    inline (i)
                        let i = (i + 1)
                        'getarg args i
            let rcount = (argc - pos - 1)
            let rcount = (? (rcount < 0) 0 rcount)
            let rargs =
                sc_argument_list_map_new rcount
                    inline (i)
                        let i = (i + pos + 1)
                        'getarg args i
            `(_ (inline () largs) (inline () rargs))

"""" filter all keyed values
define va-unnamed
    spice-macro
        fn "va-unnamed" (args)
            raising Error
            let argc = ('argcount args)
            verify-count argc 0 -1
            sc_argument_list_map_filter_new argc
                inline (i)
                    let arg = ('getarg args i)
                    let k = (sc_type_key ('qualified-typeof arg))
                    _ (k == unnamed) arg

run-stage; # 8

inline va-join (a...)
    inline (b...)
        va-append-va (inline () b...) a...

""""A `Generator` that iterates through all 32-bit signed integer values starting
    at 0. This generator does never terminate; when it exceeds the maximum
    positive integer value of 2147483647, it overflows and continues with the
    minimum negative integer value of -2147483648.
let infinite-range =
    Generator
        inline () 0
        inline (x) true
        inline (x) x
        inline (x) (x + 1)

inline zip (a b)
    let start-a valid-a at-a next-a = ((a as Generator))
    let start-b valid-b at-b next-b = ((b as Generator))
    let start-a... = (start-a)
    let lsize = (va-countof start-a...)
    let start... = (va-append-va start-b start-a...)
    Generator
        inline () start...
        inline (it...)
            let it-a it-b = (va-split lsize it...)
            (valid-a (it-a)) & (valid-b (it-b))
        inline (it...)
            let it-a it-b = (va-split lsize it...)
            va-append-va (inline () (at-b (it-b))) (at-a (it-a))
        inline (it...)
            let it-a it-b = (va-split lsize it...)
            va-append-va (inline () (next-b (it-b))) (next-a (it-a))

inline enumerate (x)
    zip infinite-range x

#-------------------------------------------------------------------------------
# function memoization
#-------------------------------------------------------------------------------

inline memoize (f)
    fn (args...)
        let key = `[f args...]
        try
            sc_map_get key
        except (err)
            let value =
                `[(f args...)]
            sc_map_set key value
            value

inline type-factory (f)
    let f = (memoize f)
    fn (...)
        ((f ...) as type)

spice memocall (f args...)
    if (not ('pure? f))
        hide-traceback; error@ ('anchor f) "while checking callable"
            "callable must be pure"
    for arg in ('args args...)
        if (not ('pure? arg))
            hide-traceback; error@ ('anchor arg) "while checking argument"
                "arguments must be pure"
    let key = `[f args...]
    let key = (sc_prove key)
    try
        sc_map_get key
    except (err)
        let value = (sc_prove `(f args...))
        for arg in ('args value)
            if (not ('pure? arg))
                hide-traceback; error "all returned arguments must be pure"
        sc_map_set key value
        value

#-------------------------------------------------------------------------------
# static-compile*
#-------------------------------------------------------------------------------

spice _static-compile (func flags)
    flags as:= u64
    hide-traceback;
    'tag `[(sc_compile func flags)] ('anchor args)

spice _static-compile-glsl (version target func flags)
    version as:= i32
    target as:= Symbol
    flags as:= u64
    hide-traceback;
    'tag `[(sc_compile_glsl version target func flags)] ('anchor args)

spice _static-compile-spirv (target func flags)
    target as:= Symbol
    flags as:= u64
    hide-traceback;
    'tag `[(sc_compile_spirv target func flags)] ('anchor args)

spice-quote
    inline static-compile (func flags...)
        _static-compile func (parse-compile-flags flags...)
    inline static-compile-glsl (version target func flags...)
        _static-compile-glsl version target func (parse-compile-flags flags...)
    inline static-compile-spirv (target func flags...)
        _static-compile-spirv target func (parse-compile-flags flags...)

#-------------------------------------------------------------------------------
# function overloading
#-------------------------------------------------------------------------------

fn sc_argument_list_join_values (a b...)
    let A = (sc_argcount a)
    let B = (va-countof b...)
    let N = (add A B)
    let values = (alloca-array Value N)
    loop (i = 0)
        if (icmp== i A)
            break;
        store (sc_getarg a i) (getelementptr values i)
        add i 1
    va-map
        inline (i)
            store (sc_identity (va@ i b...)) (getelementptr values (A + i))
        va-range B
    sc_argument_list_new N values

let nodefault = (opaque "nodefault")
fn nodefault? (x)
    (('typeof x) == type) and (x as type == nodefault)

spice overloaded-fn-append (T args...)
    let outtype = (T as type)
    let acount = ('argcount args...)
    let functions = ('@ outtype 'templates)
    let functypes = ('@ outtype 'parameter-types)
    let defaults = ('@ outtype 'parameter-defaults)
    let functions functypes defaults =
        loop (i functions functypes defaults = 0 functions functypes defaults)
            if (i >= acount)
                break functions functypes defaults
            let f = ('getarg args... i)
            let ftype = ('getarg args... (i + 1))
            let fdefs = ('getarg args... (i + 2))
            let i = (i + 3)
            if (('typeof ftype) == Nothing)
                # separator for (using ...)
                let fT = ('typeof f)
                if ('function-pointer? fT)
                    if ((('kind f) != value-kind-function)
                        and (not ('constant? f)))
                        error "argument must be constant or function"
                    let fT = ('element@ fT 0)
                    let argcount = ('element-count fT)
                    let types =
                        loop (k types = 0 void)
                            if (k == argcount)
                                break types
                            let argT = ('element@ fT k)
                            repeat (k + 1)
                                sc_arguments_type_join types argT
                    repeat i
                        sc_argument_list_join_values functions f
                        sc_argument_list_join_values functypes types
                        sc_argument_list_join_values defaults `none
                elseif (fT == type)
                    if (fT == outtype)
                        error "cannot inherit from own type"
                    let fT = (f as type)
                    if (fT < OverloadedFunction)
                        let fns = ('@ fT 'templates)
                        let ftypes = ('@ fT 'parameter-types)
                        let fdefs = ('@ fT 'parameter-defaults)
                        if (not (('argcount fns) == ('argcount ftypes) == ('argcount fdefs)))
                            error "argument list size mismatch"
                        # copy over existing options
                        repeat i
                            sc_argument_list_join functions fns
                            sc_argument_list_join functypes ftypes
                            sc_argument_list_join defaults fdefs
                elseif (fT == Closure)
                    # ensure argument is constant
                    f as Closure
                    # append as templated option
                    repeat i
                        sc_argument_list_join_values functions f
                        sc_argument_list_join_values functypes Variadic
                        sc_argument_list_join_values defaults `none
                error
                    .. "cannot embed argument of type "
                        repr ('typeof f)
                        " in overloaded function"
            else
                let T = (ftype as type)
                repeat i
                    sc_argument_list_join_values functions f
                    sc_argument_list_join_values functypes ftype
                    sc_argument_list_join_values defaults fdefs
    'set-symbol outtype 'templates functions
    'set-symbol outtype 'parameter-types functypes
    'set-symbol outtype 'parameter-defaults defaults
    T

'set-symbols OverloadedFunction
    append = overloaded-fn-append
    __typecall =
        spice "dispatch-overloaded-function" (cls args...)
            let T = (cls as type)
            let fns = ('@ T 'templates)
            let ftypes = ('@ T 'parameter-types)
            let fdefaults = ('@ T 'parameter-defaults)
            let count = ('argcount args...)
            for f FT defs in (zip ('args fns) (zip ('args ftypes) ('args fdefaults)))
                let FT = (FT as type)
                let has-defs? = (('typeof defs) != Nothing)
                let defs =
                    if has-defs? (sc_prove `(defs))
                    else `()
                let argcount = (sc_arguments_type_argcount FT)
                let variadic? =
                    (argcount > 0) and
                        ((sc_arguments_type_getarg FT (argcount - 1)) == Variadic)
                let defcount = ('argcount defs)
                if has-defs?
                    assert (argcount <= defcount)
                        "number of defaults must match number of arguments"
                    if ((not variadic?) & (count > argcount))
                        continue;
                elseif variadic?
                    if ((count + 1) < argcount)
                        continue;
                elseif (count != argcount)
                    continue;
                let failed = (ptrtoref (alloca bool))
                failed = false
                let explicit-argcount = (? variadic? (argcount - 1) argcount)
                for i in (range explicit-argcount)
                    inline no-match ()
                        failed = true
                        break;
                    let arg =
                        if (i < count) ('getarg args... i)
                        elseif has-defs?
                            let arg = ('getarg defs i)
                            if (nodefault? arg) # argument missing
                                no-match;
                            continue;
                        else
                            no-match;
                    let qargT = ('qualified-typeof arg)
                    let argT = ('strip-qualifiers qargT)
                    let qparamT = (sc_arguments_type_getarg FT i)
                    let paramT = ('strip-qualifiers qparamT)
                    if (paramT == Unknown)
                        continue;
                    elseif (argT <= paramT)
                        if (('refer? qparamT) & (not ('refer? qargT)))
                            no-match;
                        continue;
                    try
                        let matchfunc = ('@ paramT '__typematch)
                        let result = (sc_prove `(matchfunc paramT argT))
                        if (result as bool)
                            if (('refer? qparamT) & (not ('refer? qargT)))
                                no-match;
                            continue;
                        else
                            no-match;
                    else;
                    assert (paramT != Variadic)
                    let conv = (imply-converter argT paramT ('constant? arg))
                    if (not (operator-valid? conv))
                        no-match;
                if failed
                    continue;
                # success, generate call
                let lasti = (argcount - 1)
                let outargs = ('tag (sc_call_new f) ('anchor args))
                sc_call_set_rawcall outargs true
                for i arg in (enumerate ('args args...))
                    let qargT = ('qualified-typeof arg)
                    let argT = ('strip-qualifiers qargT)
                    let qparamT = (sc_arguments_type_getarg FT (min i lasti))
                    let paramT = ('strip-qualifiers qparamT)
                    let outarg =
                        if (paramT == Unknown) arg
                        elseif (paramT == Variadic) arg
                        elseif (argT <= paramT) arg
                        else `(imply arg paramT)
                    sc_call_append_argument outargs outarg
                # complete default values
                for i in (range count explicit-argcount)
                    let arg = ('getarg defs i)
                    let argT = ('typeof arg)
                    sc_call_append_argument outargs arg
                if true
                    return outargs
            # if we got here, there was no match
            error
                .. "could not match argument types ("
                    do
                        loop (i str = 0 "")
                            if (i < count)
                                repeat (i + 1)
                                    .. str
                                        ? (i == 0) "" " "
                                        repr ('qualified-typeof ('getarg args... i))
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
    spice make-defaults (atypes defaults...)
        let atypes = (atypes as type)
        let outargs =
            sc_argument_list_map_new ('argcount defaults...)
                inline (i)
                    let def = ('getarg defaults... i)
                    let paramT = (sc_arguments_type_getarg atypes i)
                    if (not ('constant? def))
                        hide-traceback;
                        error@ ('anchor def) "while checking default argument"
                            "default argument must be constant"
                    let argT = ('typeof def)
                    if (nodefault? def) def
                    elseif (paramT == Unknown) def
                    elseif (paramT == Variadic) def
                    elseif (argT <= paramT) def
                    else
                        let conv = (as-converter argT paramT true)
                        if (not (operator-valid? conv))
                            hide-traceback;
                            error@ ('anchor def) "while checking default argument"
                                "default argument does not match argument type"
                        let def = (sc_prove `(conv def))
                        if (not ('constant? def))
                            hide-traceback;
                            error@ ('anchor def) "while checking default argument"
                                "default argument must be constant after conversion"
                        def
        `(inline () outargs)

    spice init-overloaded-function (T)
        let T = (T as type)
        'set-symbols T
            templates = (sc_argument_list_new 0 null)
            parameter-types = (sc_argument_list_new 0 null)
            parameter-defaults = (sc_argument_list_new 0 null)
        T

    let inline? =
        (expr-head as Symbol) == 'inline...
    let finalize-overloaded-fn = overloaded-fn-append
    let fn-name bind-name? inlined-case =
        sugar-match name...
        case (name as Symbol; rest...) (_ name true rest...)
        case (name as string; rest...) (_ (Symbol name) false rest...)
        default (_ unnamed false name...)
        #default
            error
                """"syntax: (fn... name|"name") (case pattern body...) ...
    let outtype =
        spice-quote
            init-overloaded-function
                typename [(fn-name as string)] OverloadedFunction
    let bodyscope = (Scope sugar-scope)
    let bodyscope =
        'bind bodyscope 'this-function outtype
    let next-expr =
        if (inlined-case == '()) next-expr
        else
            let at = (decons inlined-case)
            cons
                cons ('tag `'case ('anchor at)) inlined-case
                next-expr
    loop (next outargs = next-expr (sc_argument_list_new 0 null))
        let next-anchor =
            if (empty? next) unknown-anchor
            else
                let at = (decons next)
                'anchor at
        sugar-match next
        case (('case 'using body...) rest...)
            let obj = (sc_expand (cons do body...) '() sugar-scope)
            repeat rest...
                sc_argument_list_join_values outargs obj `none `none
        case (('case condv body...) rest...)
            let tmpl = ('tag (sc_template_new fn-name) ('anchor condv))
            if inline?
                sc_template_set_inline tmpl
            let scope = (Scope bodyscope)
            repeat rest...
                loop
                    expr types defaults scope =
                        uncomma (condv as list)
                        sc_argument_list_new 0 null
                        sc_argument_list_new 0 null
                        scope
                    sugar-match expr
                    case ()
                        #hide-traceback;
                        let body =
                            try
                                hide-traceback;
                                sc_expand (cons do body...) '() scope
                            except (err)
                                hide-traceback;
                                error@+ err next-anchor "while expanding case"
                        sc_template_set_body tmpl body
                        let atypes = `(Arguments types)
                        break
                            sc_argument_list_join_values outargs
                                \ tmpl atypes `(make-defaults atypes defaults)
                    case ((arg as Symbol) ': rest...)
                        hide-traceback;
                        error@ ('anchor condv) "while parsing pattern"
                            "single typed parameter definition is missing trailing comma or semicolon"
                    case ((arg as Symbol) '= rest...)
                        hide-traceback;
                        error@ ('anchor condv) "while parsing pattern"
                            "single default parameter definition is missing trailing comma or semicolon"
                    case ((arg as Symbol) rest...)
                        if ('variadic? arg)
                            if (not (empty? rest...))
                                error "variadic parameter must be in last place"
                        let param = (sc_parameter_new arg)
                        sc_template_append_parameter tmpl param
                        repeat rest...
                            sc_argument_list_join_values types
                                ? ('variadic? arg) Variadic Unknown
                            sc_argument_list_join_values defaults nodefault
                            'bind scope arg param
                    case (((arg as Symbol) '= def) rest...)
                        if ('variadic? arg)
                            error "variadic parameter can not have default value"
                        let param = (sc_parameter_new arg)
                        sc_template_append_parameter tmpl param
                        let def = (sc_expand def '() sugar-scope)
                        repeat rest...
                            sc_argument_list_join_values types Unknown
                            sc_argument_list_join_values defaults def
                            'bind scope arg param
                    case (((arg as Symbol) ': T '= def) rest...)
                        if ('variadic? arg)
                            error "a typed parameter can't be variadic"
                        let T = (sc_expand T '() sugar-scope)
                        let param = (sc_parameter_new arg)
                        sc_template_append_parameter tmpl param
                        let def = (sc_expand def '() sugar-scope)
                        repeat rest...
                            sc_argument_list_join_values types T
                            sc_argument_list_join_values defaults def
                            'bind scope arg param
                    case (((arg as Symbol) ': T) rest...)
                        if ('variadic? arg)
                            error "a typed parameter can't be variadic"
                        let T = (sc_expand T '() sugar-scope)
                        let param = (sc_parameter_new arg)
                        sc_template_append_parameter tmpl param
                        repeat rest...
                            sc_argument_list_join_values types T
                            sc_argument_list_join_values defaults nodefault
                            'bind scope arg param
                    default
                        let at = (decons expr)
                        hide-traceback;
                        error@ ('anchor at) "while parsing pattern"
                            "syntax: (parameter-name[: type], ...)"
        default
            let sugar-scope =
                if bind-name?
                    'bind sugar-scope fn-name outtype
                else sugar-scope
            return
                `(finalize-overloaded-fn outtype outargs)
                next
                sugar-scope

let inline... = fn...

sugar from (src 'let params...)
    spice load-from (src keys...)
        let count = ('argcount keys...)
        sc_argument_list_map_new count
            inline (i)
                let key = ('getarg keys... i)
                `(getattr src key)

    fn quotify (params)
        if (empty? params)
            return '()
        let entry rest = (decons params)
        entry as Symbol
        cons
            list sugar-quote entry
            this-function rest

    cons let
        .. params...
            list '=
                cons load-from src
                    quotify params...

define zip (spice-macro (fn (args) (ltr-multiop args `zip 2)))

sugar chain-typed-symbol-handler (handler)
    spice default-handler (symbol env)
        hide-traceback;
        '@ (env as Scope) symbol
    let handler =
        sc_expand handler '() sugar-scope
    let next-handler =
        try ('@ sugar-scope typed-symbol-handler-symbol)
        except (err) `default-handler
    let handler =
        spice-quote
            inline "#hidden" (symbol env)
                handler next-handler symbol env
    return `() next-expr
        'bind sugar-scope typed-symbol-handler-symbol handler

run-stage; # 9

inline _memo (f)
    inline (...)
        memocall f ...

inline memo (f) (memocall _memo f)

'set-symbols list
    rjoin =
        fn "rjoin" (lside rside)
            loop (params expr = lside rside)
                if (empty? params) (break expr)
                let param next = (decons params)
                _ next (cons param expr)
    token-split =
        fn "token-split" (expr token errmsg)
            loop (it params = expr '())
                if (empty? it)
                    error errmsg
                let sxat it = (decons it)
                let at = (sxat as Symbol)
                if (at == token)
                    break params it
                _ it (cons sxat params)

define-sugar-block-scope-macro static-if
    fn process (anchor body next-expr)
        returning list list
        let cond body = (decons body)
        let elseexpr next-next-expr =
            if (empty? next-expr)
                _ '() next-expr
            else
                let else-expr next-next-expr = (decons next-expr)
                if (('typeof else-expr) == list)
                    let kw body = (decons (else-expr as list))
                    let anchor = ('anchor kw)
                    if (('typeof kw) == Symbol)
                        let kw = (kw as Symbol)
                        switch kw
                        case 'elseif
                            this-function anchor body next-next-expr
                        case 'else
                            _ body next-next-expr
                        default
                            _ '() next-expr
                    else
                        _ '() next-expr
                else
                    _ '() next-expr
        return
            list
                'tag
                    Value
                        list static-branch
                            'tag `[(list imply cond bool)] ('anchor cond)
                            cons inline "#hidden" '() body
                            cons inline "#hidden" '() elseexpr
                    anchor
            next-next-expr
    let kw body = (decons expr)
    let body next-expr = (process ('anchor kw) body next-expr)
    return
        cons
            cons ('tag `do ('anchor kw)) body
            next-expr
        sugar-scope

define-sugar-block-scope-macro sugar-if
    fn process (sugar-scope body next-expr)
        returning list list
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
                        this-function sugar-scope body next-next-expr
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
        error "condition must be constant"
    let kw body = (decons expr)
    let body next-expr = (process sugar-scope body next-expr)
    return
        cons
            cons do body
            next-expr
        sugar-scope

define-sugar-block-scope-macro @@
    raising Error
    let kw body = (decons expr)
    let anchor = ('anchor kw)
    let head = (kw as Symbol)
    let result next-expr =
        loop (body next-expr result = body next-expr '())
            if (empty? next-expr)
                hide-traceback;
                error "decorator is not applied to anything"
            let result =
                cons
                    if ((countof body) == 1)
                        '@ body
                    else
                        'tag `body anchor
                    result
            let follow-expr next-next-expr = (decons next-expr)
            if (('typeof follow-expr) == list)
                let kw body = (decons (follow-expr as list))
                let kw = (kw as Symbol)
                if (kw == head)
                    # more decorators
                    repeat body next-next-expr result
                else
                    # terminating actual expression
                    let newkw = (Symbol (.. "decorate-" (kw as string)))
                    break
                        cons
                            'tag `newkw anchor
                            \ follow-expr result
                        next-next-expr
            else
                # default decorator for arbitrary values
                break
                    cons 'decorate-vvv follow-expr result
                    next-next-expr
    return (cons result next-expr) sugar-scope

define-sugar-block-scope-macro vvv
    raising Error
    let kw body = (decons expr)
    let head = (kw as Symbol)
    let result next-expr =
        do  #label ok (body next-expr result = body next-expr '())
            if (empty? next-expr)
                error "expression decorator is not applied to anything"
            let result =
                list
                    if ((countof body) == 1)
                        '@ body
                    else
                        `body
            let follow-expr next-next-expr = (decons next-expr)
            let follow-expr next-next-expr =
                sc_expand follow-expr next-next-expr sugar-scope
            _
                cons ('tag `'decorate-vvv ('anchor kw)) follow-expr result
                next-next-expr
    return
        cons result next-expr
        sugar-scope

define-sugar-macro decorate-vvv
    raising Error
    let expr decorators = (decons args)
    loop (in out = decorators expr)
        if (empty? in)
            break out
        let decorator in = (decons in)
        repeat in
            `[(cons decorator (list out))]

define-sugar-macro decorate-fn
    raising Error
    let fnexpr decorators = (decons args)
    let kw name body = (decons (fnexpr as list) 2)
    let name-is-symbol? = (('typeof name) == Symbol)
    let fnexpr =
        'tag
            Value
                cons kw
                    if name-is-symbol?
                        'tag `[(name as Symbol as string)] ('anchor name)
                    else name
                    body
            'anchor fnexpr
    let result =
        loop (in out = decorators fnexpr)
            if (empty? in)
                break out
            let decorator in = (decons in)
            repeat in
                `[(cons decorator (list out))]
    if name-is-symbol?
        `[(list let name '= result)]
    else
        result

let
    decorate-inline = decorate-fn
    decorate-typedef = decorate-fn
    decorate-struct = decorate-fn

define-sugar-macro decorate-let
    raising Error
    let letexpr decorators = (decons args)
    let anchor = ('anchor letexpr)
    let letexpr = (letexpr as list)
    let kw entry = (decons letexpr 2)
    if (('typeof entry) == list)
        # map form: wrap each arg
        let result =
            loop (in out = ('next letexpr) '())
                if (empty? in)
                    break out
                let entry in = (decons in)
                let entry-anchor = ('anchor entry)
                let k eq val = (decons (entry as list) 2)
                let result =
                    loop (in out = decorators val)
                        if (empty? in)
                            break out
                        let decorator in = (decons in)
                        repeat in
                            list
                                'tag `[(cons decorator out)] entry-anchor
                repeat in
                    cons
                        'tag `[(cons k eq result)] anchor
                        out
        cons let ('reverse result)
    else
        # unpack form: wrap all args
        let params values =
            loop (expr params = letexpr '())
                if (empty? expr)
                    error "reimport form not supported for decorate-let"
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
        loop (in out = params (list '= ('tag `result anchor)))
            if (empty? in)
                break out
            let param params = (decons in)
            repeat params
                cons param out

define-sugar-scope-macro sugar-eval
    let subscope =
        'bind (Scope sugar-scope) 'sugar-scope sugar-scope
    let at next = (decons args)
    return
        exec-module ('tag `args ('anchor at)) subscope
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
    #eval = sc_eval
    load-library = sc_load_library
    load-object = sc_load_object

sugar fold-locals (args...)
    fn stage-constant? (value)
        ('pure? value) and (('typeof value) != SpiceMacro)
    let scope = sugar-scope
    let _1 _2 = (decons args...)
    let init _2 = (sc_expand _1 _2 sugar-scope)
    let _1 _2 = (decons _2)
    let f = (sc_expand _1 _2 sugar-scope)
    let block = (sc_expression_new)
    sc_expression_append block init
    let anchor = ('anchor expression)
    # first go through the constants
    let outval =
        loop (index outval = -1 init)
            let key value index = ('next scope index)
            if (index < 0)
                break outval
            if (stage-constant? value)
                let docstr = ('docstring scope key)
                let expr = ('tag `(f outval key docstr value) anchor)
                sc_expression_append block expr
                repeat index expr
            else
                repeat index outval
    # then go through the rest
    loop (index outval = -1 outval)
        let key value index = ('next scope index)
        if (index < 0)
            return block
        if (not (stage-constant? value))
            let docstr = ('docstring scope key)
            let expr = ('tag `(f outval key docstr value) anchor)
            sc_expression_append block expr
            repeat index expr
        else
            repeat index outval

sugar :: ((name as Symbol))
    let start-expr = next-expr
    # find end of scope
    loop (body next-expr = '() next-expr)
        if (empty? next-expr)
            error
                .. "missing `" (name as string) " ::` in block"
        let at next = (decons next-expr)
        let args =
            label cont
                if (('typeof at) == list)
                    let atlist = (at as list)
                    let head args = (decons atlist)
                    if (('typeof head) == Symbol)
                        if ((head as Symbol) == name)
                            sugar-match args
                            case ('::)
                                merge cont '()
                            case ((...) '::)
                                merge cont ...
                            default
                                ;
                repeat (cons at body) next
        # found tail
        let anchor = ('anchor at)
        let body = ('reverse body)
        let expr =
            if (not (empty? args))
                qq [embed]
                    unquote
                        'tag (Value (qq [let] (unquote-splice args) =
                                ([label] [name] (unquote-splice body)))) anchor
            else
                qq [label] [name]
                    unquote-splice body
        return expr next

run-stage; # 10

#-------------------------------------------------------------------------------
# static-match
#-------------------------------------------------------------------------------

sugar static-match (cond)
    spice handle-static-match (cond args...)
        let argc = ('argcount args...)
        loop (i = 0)
            i1 := i + 1
            if (i1 < argc)
                # at least two args
                let test = ('getarg args... i)
                let then = ('getarg args... i1)
                let result = ('tag (sc_prove `(cond == test)) ('anchor test))
                result as:= bool
                if result
                    return `(then)
            else
                # else-arg
                let then = ('getarg args... i)
                return `(then)
            i1 + 1

    let cond = (sc_expand cond '() sugar-scope)
    let outargs =
        sc_argument_list_map_new 1
            inline (i) cond
    loop (next-expr outargs = next-expr outargs)
        sugar-match next-expr
        case (('case it body...) rest...)
            let it = (sc_expand it '() sugar-scope)
            let body = (sc_expand (cons embed body...) '() sugar-scope)
            repeat rest...
                sc_argument_list_join_values outargs it `(inline () [body])
        case (('default body...) rest...)
            let body = (sc_expand (cons embed body...) '() sugar-scope)
            let outargs =
                sc_argument_list_join_values outargs `(inline () [body])
            return `(handle-static-match outargs) rest...
        default
            hide-traceback;
            error "default branch missing"

#-------------------------------------------------------------------------------
# unlet
#-------------------------------------------------------------------------------

sugar unlet ((name as Symbol) names...)
    do
        hide-traceback;
        '@ sugar-scope name
    let scope =
        'unbind sugar-scope name
    let start valid? at next = ((names... as Generator))
    loop (scope it... = scope (start))
        if (valid? it...)
            let name = (at it...)
            let name = (name as Symbol)
            hide-traceback;
            '@ scope name
            let scope =
                'unbind scope name
            repeat scope (next it...)
        else
            break `() next-expr scope

#-------------------------------------------------------------------------------
# fold iteration
#-------------------------------------------------------------------------------

"""".. sugar:: (fold (state ... _:= init...) _:for name ... _:in gen body...)

       This is a combination of the `loop` and `for` forms. It enumerates all
       elements in collection or sequence `gen`, unpacking each element and
       binding its arguments to the names defined by `name ...`, while
       the loop state `state ...` is initialized from `init...`.

       Similar to `loop`, the body expression must return the next state of
       the loop. The state of `gen` is transparently maintained and does not
       have to be managed.

       Unlike `for`, `fold` requires calls to ``break`` to pass a state
       compatible with `state ...`. Otherwise they serve the same function.

       Usage example::

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
sugar fold ((binding...) 'for expr...)
    let itparams it = ('token-split expr... 'in "'in' expected")
    let foldparams init = ('token-split binding... '= "'=' expected")
    let generator-expr body = (decons it)
    let subscope = (Scope sugar-scope)
    let anchor = ('anchor expression)
    return
        spice-quote
            let init... =
                spice-unquote
                    let expr = (sc_expand (cons _ init) '() subscope)
                    expr
            let gen =
                spice-unquote
                    let expr = (sc_expand generator-expr '() subscope)
                    expr
            let start valid? at next = ((as gen Generator))
            let start... = (start)
            let lsize = (va-countof start...)
            loop (args... = (va-append-va (inline () init...) start...))
                let it state = (va-split lsize args...)
                let it... = (it)
                if (valid? it...)
                    inline continue ()
                        repeat (va-append-va state (next it...))
                    inline _repeat (newstate...)
                        repeat (va-append-va (inline () newstate...) (next it...))
                    let at... = (at it...)
                    let state... = (state)
                    _repeat
                        spice-unquote
                            let expr1 expr2 =
                                cons let ('rjoin itparams (list '= at...))
                                cons let ('rjoin foldparams (list '= state...))
                            let expr1 = ('tag `expr1 anchor)
                            let expr2 = ('tag `expr2 anchor)
                            let subscope =
                                'bind subscope 'continue continue
                            let subscope =
                                'bind subscope 'repeat _repeat
                            let result = (sc_expand (cons ('tag `do anchor) expr1 expr2 body) '() subscope)
                            result
                else
                    break (state)

#-------------------------------------------------------------------------------
# bind decorator
#-------------------------------------------------------------------------------

sugar bind (...)
    inline nop ()
    return
        qq [embed]
            [let] (unquote-splice ...) = (unquote-splice next-expr)
            [nop]
        '()

#-------------------------------------------------------------------------------
# scope export
#-------------------------------------------------------------------------------

define append-to-scope
    fn stage-constant? (value)
        ('pure? value) and (('typeof value) != SpiceMacro)

    spice-macro
        fn (args)
            let packedscope = ('getarg args 0)
            let key = ('getarg args 1)
            let docstr = ('getarg args 2)
            let values = ('getarglist args 3)
            let constant-scope? = ('constant? packedscope)
            if (constant-scope? & (stage-constant? values))
                let scope = (packedscope as Scope)
                let scope =
                    'bind-with-docstring scope key values (docstr as string)
                return `scope
            # for non-constant values, we need to create a new scope
            let packedscope =
                if constant-scope? `(Scope [(packedscope as Scope)])
                else packedscope
            if (('argcount values) != 1)
                let block = (sc_expression_new)
                let acount = ('argcount values)
                let outargs = `(alloca-array Value acount)
                sc_expression_append block outargs
                for i arg in (enumerate ('args values))
                    sc_expression_append block
                        if (('typeof arg) == Value)
                            `(store ``arg (getelementptr outargs i))
                        else
                            `(store `arg (getelementptr outargs i))
                sc_expression_append block
                    `('bind-with-docstring packedscope key (sc_argument_list_new acount outargs) docstr)
                block
            else
                spice-quote
                    'bind-with-docstring packedscope key values docstr

""""Export locals as a chain of up to two new scopes: a scope that contains
    all the constant values in the immediate scope, and a scope that contains
    the runtime values. If all values in the scope are constant, then the
    resulting scope will also be constant.
sugar locals ()
    spice make-scope (docstring)
        let docstring = (docstring as string)
        # create a scope constant at compile time
        `[(sc_scope_new_with_docstring docstring)]
    let docstring = ('module-docstring sugar-scope)
    list fold-locals (list make-scope docstring) append-to-scope

#-------------------------------------------------------------------------------
# typedef
#-------------------------------------------------------------------------------

define append-to-type
    fn stage-constant? (value)
        ('pure? value) and (('typeof value) != SpiceMacro)

    spice-macro
        fn (args)
            let T = ('getarg args 0)
            let key = ('getarg args 1)
            let docstr = ('getarg args 2)
            if (('typeof key) != Symbol)
                # ignore non-keys
                return T
            if (key as Symbol == unnamed)
                # ignore docstring
                return T
            let value = ('getarg args 3)
            if ('constant? T)
                let T = (T as type)
                if (stage-constant? value)
                    let key = (key as Symbol)
                    'set-symbol T key value
                    'set-docstring T key (docstr as string)
                    `T
                else
                    let wrapvalue =
                        if (('typeof value) == Value) value
                        else `value
                    spice-quote
                        embed
                            sc_type_set_symbol T key wrapvalue
                            sc_type_set_docstring T key docstr
                            T
            else
                spice-quote
                    embed
                        sc_type_set_symbol T key value
                        sc_type_set_docstring T key docstr
                        T

sugar typedef+ (T body...)
    let anchor = ('anchor expr-head)
    let body =
        qq [do]
            unquote-splice body...
            [fold-locals] this-type [append-to-type]
    qq [do]
        [let] this-type = [T]
        [let] super-type = ([superof T])
        [('tag `body anchor)]
        this-type

""""a type declaration syntax; when the name is a string, the type is declared
    at runtime.
sugar typedef (name body...)
    let declaration? = (('typeof name) == Symbol)

    if declaration?
        let name-exists =
            try
                '@ sugar-scope (name as Symbol)
                true
            except (err)
                false
        if name-exists
            hide-traceback;
            error@ ('anchor name) "while defining type"
                .. "symbol '" (name as Symbol as string) "' already defined in scope"

    let expr supertype has-supertype-def? =
        sugar-match body...
        case ('< supertype rest...)
            _ rest... supertype true
        default
            _ body... `typename false

    let typedecl =
        qq [typename]
            unquote
                if declaration?
                    `[(name as Symbol as string)]
                else
                    name
            [supertype]

    spice set-opaque (T)
        sc_typename_type_set_opaque (T as type)
        `()

    spice set-storage (T ST flags)
        let T = (T as type)
        let ST = (ST as type)
        let flags = (flags as u32)
        sc_typename_type_set_storage T ST flags
        `()

    fn check-no-storage (storage? storagetype)
        if storage?
            error@ ('anchor storagetype) "while matching pattern" "storage type already defined"

    fn complete-outp (outp storage?)
        if storage? outp
        else
            cons (list set-opaque 'this-type) outp

    let expr =
        loop (inp outp storage? = expr '() false)
            sugar-match inp
            case ('< supertype rest...)
                error@ ('anchor supertype) "while matching pattern" "supertype must be in first place"
            case (': storagetype rest...)
                check-no-storage storage? storagetype
                repeat rest...
                    cons (list set-storage 'this-type
                        storagetype typename-flag-plain) outp
                    true
            case (':: storagetype rest...)
                check-no-storage storage? storagetype
                repeat rest...
                    cons (list set-storage 'this-type
                        storagetype 0:u32) outp
                    true
            case ('do rest...)
                break
                    qq [do]
                        [let] this-type = [typedecl]
                        [let] super-type = [supertype]
                        unquote-splice outp
                        [do]
                            unquote-splice rest...
                        this-type
            default
                let outp = (complete-outp outp storage?)
                break
                    qq [do]
                        [let] this-type = [typedecl]
                        [let] super-type = [supertype]
                        unquote-splice outp
                        [do]
                            unquote-splice inp
                            [fold-locals] this-type [append-to-type]
                        this-type
    if declaration?
        qq [let] [name] = [expr]
    else expr

#-------------------------------------------------------------------------------
# standard allocators
#-------------------------------------------------------------------------------

inline gen-allocator-sugar (name copyf newf)
    sugar "" (values...)
        spice copy-by-value (value)
            let T = ('typeof value)
            `(copyf T value)
        let anchor = ('anchor expression)
        let _let = ('tag `let anchor)
        let result =
            sugar-match values...
            case (name '= value)
                let callexpr =
                    'tag `[(qq ([copy-by-value value]))] anchor
                qq [_let name] = [callexpr]
            case (name ': T '= value)
                let callexpr =
                    'tag `[(qq ([copyf T value]))] anchor
                qq [_let name] = [callexpr]
            case (name ': T args...)
                let callexpr =
                    'tag `[(qq [newf T] (unquote-splice args...))] anchor
                qq [_let name] = [callexpr]
            case (T args...)
                qq [newf T] (unquote-splice args...)
            default
                error
                    .. "syntax: " name " <name> [: <type>] [= <value>]"
        'tag `result anchor

let local =
    gen-allocator-sugar "local"
        spice "local-copy" (T value)
            spice-quote
                let val = (alloca T)
                store (imply value T) val
                ptrtoref val
        spice "local-new" (T args...)
            spice-quote
                let val = (alloca T)
                store (T args...) val
                ptrtoref val

let global =
    gen-allocator-sugar "global"
        spice "global-copy" (T value)
            let val = (extern-new unnamed (T as type) (storage-class = 'Private))
            let qval = `(ptrtoref val)
            if (('constant? value) and (('typeof value) == Closure))
                # constructor, build function
                let f = (value as Closure)
                spice-quote
                    fn constructor ()
                        store (f) val
                        ;
                let constructor = (sc_typify_template constructor 0 null)
                sc_global_set_constructor val constructor
                qval
            else
                let init = (sc_prove `(imply value T))
                if ('pure? init)
                    hide-traceback;
                    sc_global_set_initializer val init
                    qval
                else
                    spice-quote
                        store init val
                        qval

        spice "global-new" (T args...)
            let T = (T as type)
            let val = (extern-new unnamed (T as type) (storage-class = 'Private))
            let qval = `(ptrtoref val)
            let pure-args? =
                for arg in ('args args...)
                    if (not ('pure? arg))
                        break false
                else true
            if pure-args?
                # static constructor
                spice-quote
                    fn constructor ()
                        store (T args...) val
                        ;
                let constructor = (sc_typify_template constructor 0 null)
                sc_global_set_constructor val constructor
                qval
            else
                let init = (sc_prove `(T args...))
                if ('pure? init)
                    hide-traceback;
                    sc_global_set_initializer val init
                    qval
                else
                    spice-quote
                        store init val
                        qval

run-stage; # 11

#-------------------------------------------------------------------------------
# list constant expression folding
#-------------------------------------------------------------------------------

typedef+ list
    spice __countof (self)
        if ('constant? self)
            `[(sc_list_count (self as list))]
        else
            `(sc_list_count self)

    spice __unpack (self)
        if (not ('constant? self))
            error "can only unpack constant lists"
        let self = (self as list)
        let count = (countof self)
        if (count == 0)
            return `()
        let values = (alloca-array Value count)
        loop (i self = 0 self)
            if (empty? self)
                break;
            let at next = (decons self)
            store
                # can unpack constants right away
                if ('constant? at) at
                else
                    # keep impure values as values
                    ``at
                getelementptr values i
            repeat (i + 1) next
        sc_argument_list_new count values

#-------------------------------------------------------------------------------
# images
#-------------------------------------------------------------------------------

typedef+ SampledImage
    spice __typecall (cls T)
        let T = (T as type)
        `[(sc_sampled_image_type T)]
    let type = sc_sampled_image_type

typedef+ Image
    spice __typecall (cls T dim depth arrayed multisampled sampled format access)
        let
            dim = (dim as Symbol)
            depth = (depth as i32)
            arrayed = (arrayed as i32)
            multisampled = (multisampled as i32)
            sampled = (sampled as i32)
            format = (format as Symbol)
            access = (access as Symbol)
        let T = (T as type)
        `[(sc_image_type T dim depth arrayed multisampled sampled format access)]
    let type = sc_image_type

#-------------------------------------------------------------------------------
# C type support
#-------------------------------------------------------------------------------

# importing
#-------------------------------------------------------------------------------

sugar include (args...)
    fn gen-code (cfilename code opts scope)
        let scope =
            do
                hide-traceback;
                (sc_import_c cfilename code opts scope)
        `scope

    let modulename = (('@ sugar-scope 'module-path) as string)
    loop (args modulename ext opts includestr scope = args... modulename ".c" '() "" (nullof Scope))
        sugar-match args
        case (('using name) rest...)
            let value = ((sc_expand name '() sugar-scope) as Scope)
            repeat rest... modulename ext opts includestr value
        case (('extern "C++") rest...)
            if (modulename == ".cpp")
                hide-traceback;
                error "duplicate 'extern \"C++\"'"
            repeat rest... modulename ".cpp" opts includestr scope
        case (('options opts...) rest...)
            let opts =
                loop (outopts inopts = '() opts...)
                    if (empty? inopts)
                        break ('reverse outopts)
                    let at next = (decons inopts)
                    let val =
                        do
                            let expr = (sc_expand at '() sugar-scope)
                            sc_prove expr
                    if (('typeof val) != string)
                        error "option arguments must evaluate to constant strings"
                    val as:= string
                    outopts := (cons val outopts)
                    repeat outopts next
            repeat rest... modulename ext opts includestr scope
        case ((s as string) rest...)
            if (not (empty? includestr))
                hide-traceback;
                error "duplicate include string"
            repeat rest... modulename ext opts s scope
        case ()
            let sz = (countof includestr)
            if (sz == 0)
                hide-traceback;
                error "include string is empty"
            let includestr =
                if (includestr @ (sz - 1) == 10:i8)
                    # code block
                    includestr
                else (.. "#include \"" includestr "\"")
            return
                gen-code (.. modulename ext) includestr opts scope
                next-expr
        default
            hide-traceback;
            error (.. "invalid syntax: " (repr args))

# pointers
#-------------------------------------------------------------------------------

'set-symbols pointer
    __@ =
        inline (self index)
            ptrtoref (getelementptr self index)
    __toref =
        inline (self)
            ptrtoref self
    __getattr =
        inline (self key)
            getattr (ptrtoref self) key
    __== =
        simple-binary-op
            inline (a b)
                icmp== (ptrtoint a intptr) (ptrtoint b intptr)

# native structs
#-------------------------------------------------------------------------------

spice destructor (self)
    let T = ('typeof self)
    let numfields = ('element-count T)
    # drop fields in order
    let block = (sc_expression_new)
    loop (i = 0)
        if (i == numfields)
            break;
        sc_expression_append block
            spice-quote
                __drop (extractvalue self i)
        i + 1
    sc_expression_append block `()
    block

fn constructor (cls args...)
    let cls = (cls as type)
    let struct-fields =
        try ('@ cls '__fields__)
        except (err) `none
    let argc = ('argcount args...)
    let numfields = ('element-count cls)
    let fields = (malloc-array Value numfields)
    loop (i = 0)
        if (i == numfields)
            break;
        store (null as Value) (getelementptr fields i)
        i + 1
    # collect initializers from arguments
    loop (i = 0)
        if (i == argc)
            break;
        let arg = ('getarg args... i)
        let key v = ('dekey arg)
        let k =
            if (key == unnamed) i
            else
                sc_type_field_index cls key
        if ((load (getelementptr fields k)) != null)
            error@ ('anchor arg) "while initializing struct fields"
                "field is already initialized"
        let ET = (sc_type_element_at cls k)
        let ET = (sc_strip_qualifiers ET)
        let v =
            do
                hide-traceback;
                sc_prove ('tag `(imply v ET) ('anchor arg))
        store v (getelementptr fields k)
        i + 1
    # complete default initializers
    let const? = (cls < CStruct)
    let const? =
        loop (i const? = 0 const?)
            if (i == numfields)
                break const?
            let elem = (load (getelementptr fields i))
            let elem =
                if (elem == null)
                    define elem
                        :: success
                        :: skip
                        if (('typeof struct-fields) == Nothing)
                            merge skip
                        let field = ('getarg struct-fields i)
                        let field = (field as type)
                        let elem =
                            try ('@ field 'Default)
                            except (err)
                                merge skip
                        merge success elem
                        skip ::
                        # default initializer
                        let ET = (sc_type_element_at cls i)
                        try
                            sc_prove `(ET)
                        except (err)
                            error
                                .. "field " (repr ET) " has no default initializer"
                        success ::
                    store elem (getelementptr fields i)
                    elem
                else elem
            _ (i + 1) (const? & ('constant? elem))
    if const?
        sc_const_aggregate_new cls numfields fields
    else
        let block = (sc_expression_new)
        loop (i result = 0 `(nullof cls))
            sc_expression_append block result
            if (i == numfields)
                break block
            let elem = (load (getelementptr fields i))
            let field = ('getarg struct-fields i)
            let result = ('tag `(insertvalue result elem i) ('anchor field))
            _ (i + 1) result

'set-symbols Struct
    __getattr = extractvalue
    __typecall =
        spice "Struct-typecall" (cls args...)
            if ((cls as type) == Struct)
                error "Struct type constructor not available"
            constructor cls args...
    __drop = destructor

# tuple construction
#-------------------------------------------------------------------------------

# comparison
spice tuple== (self other)
    let cls = ('typeof self)
    let numfields = ('element-count cls)
    let block = (sc_if_new)
    let quoted-false = `false
    loop (i = 0)
        if (i == numfields)
            break;
        sc_if_append_then_clause block `((@ self i) != (@ other i)) quoted-false
        i + 1
    sc_if_append_else_clause block `true
    block

typedef+ tuple
    spice-quote
        inline __== (cls T)
            static-if (cls == T)
                tuple==

    # extend type constructor with value constructor
    spice __typecall (cls args...)
        let cls = (cls as type)
        if (cls == this-type)
            return `([tuple.__typecall] cls args...)
        let argc = ('argcount args...)
        let numfields = ('element-count cls)
        let fields = (malloc-array Value numfields)
        loop (i = 0)
            if (i == numfields)
                break;
            store (null as Value) (getelementptr fields i)
            i + 1
        # collect initializers from arguments
        loop (i = 0)
            if (i == argc)
                break;
            let arg = ('getarg args... i)
            let k v = ('dekey arg)
            let k =
                if (k == unnamed) i
                else
                    sc_type_field_index cls k
            if ((load (getelementptr fields k)) != null)
                error@ ('anchor arg) "while initializing tuple fields"
                    "field is already initialized"
            let ET = (sc_type_element_at cls k)
            let ET = (sc_strip_qualifiers ET)
            let v =
                if (('pointer? ET) and ('refer? ('qualified-typeof v)))
                    `(imply (reftoptr v) ET)
                else `(imply v ET)
            store v (getelementptr fields k)
            i + 1
        let block = (sc_expression_new)
        loop (i result = 0 `(nullof cls))
            sc_expression_append block result
            if (i == numfields)
                break block
            let elem =
                if ((load (getelementptr fields i)) == null)
                    # default initializer
                    let ET = (sc_type_element_at cls i)
                    try
                        sc_prove `(ET)
                    except (err)
                        error
                            .. "field " (repr ET) " has no default initializer"
                else
                    load (getelementptr fields i)
            let result = `(insertvalue result elem i)
            _ (i + 1) result

# C structs
#-------------------------------------------------------------------------------

fn nested-union-field-accessor

fn nested-struct-field-accessor (cls name)
    returning Value
    let index =
        try
            sc_type_field_index cls name
        except (err)
            # check for unnamed attributes
            for i in (range ('element-count cls))
                let ET = ('element@ cls i)
                let k = ('keyof ET)
                if (k == unnamed)
                    let op =
                        if (ET < CStruct)
                            this-function ET name
                        elseif (ET < CUnion)
                            nested-union-field-accessor ET name
                        else
                            sc_empty_argument_list;
                    if (operator-valid? op)
                        return
                            spice-quote
                                inline (self)
                                    op (extractvalue self i)
            return (sc_empty_argument_list)
    spice-quote
        inline (self)
            extractvalue self index

spice gen-union-extractvalue (self name)
    let qcls = ('qualified-typeof self)
    let cls = ('strip-qualifiers qcls)
    let fields = (('@ cls '__fields) as type)
    let i =
        if (('typeof name) == Symbol)
            hide-traceback;
            sc_type_field_index fields (name as Symbol)
        else
            name as i32
    let ET = ('key-type ('element@ fields i) unnamed)
    if ('refer? qcls)
        let ptrT = ('refer->pointer-type qcls)
        let ptrET = ('change-element-type ptrT ET)
        spice-quote
            let ptr = (reftoptr self)
            let ptr = (bitcast ptr ptrET)
            ptrtoref ptr
    else
        let ptrET = ('change-storage-class ('mutable (pointer.type ET)) 'Function)
        spice-quote
            let ptr = (alloca cls)
            store self ptr
            let ptr = (bitcast ptr ptrET)
            load ptr

fn nested-union-field-accessor (qcls name)
    returning Value
    let cls = ('strip-qualifiers qcls)
    let fields = (('@ cls '__fields) as type)
    let index =
        try
            sc_type_field_index fields name
        except (err)
            # check for unnamed attributes
            for i in (range ('element-count fields))
                let ET = ('element@ fields i)
                let k = ('keyof ET)
                if (k == unnamed)
                    let op =
                        if (ET < CStruct)
                            nested-struct-field-accessor ET name
                        elseif (ET < CUnion)
                            this-function ET name
                        else
                            sc_empty_argument_list;
                    if (operator-valid? op)
                        return
                            spice-quote
                                inline (self)
                                    op (gen-union-extractvalue self i)
            return (sc_empty_argument_list)
    spice-quote
        inline (self)
            gen-union-extractvalue self index

'set-symbols CStruct
    __getattr =
        spice "CStruct-getattr" (self name)
            let op = (nested-struct-field-accessor ('typeof self) (name as Symbol))
            if (operator-valid? op)
                `(op self)
            else
                # produces a useful error message
                'tag `(extractvalue self name) ('anchor name)
    __typecall =
        spice "CStruct-typecall" (cls args...)
            if ((cls as type) == CStruct)
                error "CStruct type constructor is deprecated"
            constructor cls args...
    __drop = destructor

# unions
#-------------------------------------------------------------------------------

typedef+ CUnion
    inline __typecall (cls value...)
        local self = (nullof cls)
        static-if (va-empty? value...)
            # keep as-is
        else
            # initialize member
            let key = (keyof value...)
            let content = value...
            (getattr self key) = content
        self

    spice __getattr (self name)
        let op = (nested-union-field-accessor ('qualified-typeof self) (name as Symbol))
        if (operator-valid? op)
            `(op self)
        else
            # produces a useful error message
            `(gen-union-extractvalue self name)

# enums (classical C enums or tagged unions)
#-------------------------------------------------------------------------------

do
    inline simple-unary-storage-op (f)
        inline (self) (f (storagecast self))
    inline simple-binary-storage-op (f)
        simple-binary-op (inline (a b) (f (storagecast a) (storagecast b)))

    'set-symbols CEnum
        __== = (simple-binary-op icmp==)
        __!= = (simple-binary-op icmp!=)
        __> = (simple-binary-storage-op (_ >))
        __< = (simple-binary-storage-op (_ <))
        __>= = (simple-binary-storage-op (_ >=))
        __<= = (simple-binary-storage-op (_ <=))
        __~ = (simple-unary-storage-op (_ ~))
        __neg = (simple-unary-storage-op (_ -))
        __+ = (simple-binary-storage-op (_ +))
        __- = (simple-binary-storage-op (_ -))
        __* = (simple-binary-storage-op (_ *))
        __/ = (simple-binary-storage-op (_ /))
        __// = (simple-binary-storage-op (_ //))
        __| = (simple-binary-storage-op (_ |))
        __& = (simple-binary-storage-op (_ &))
        __^ = (simple-binary-storage-op (_ ^))
        __imply =
            spice-cast-macro
                fn "CEnum-imply" (vT T)
                    let ST = ('storageof vT)
                    if (T == ST)
                        return `(inline (self) (bitcast self T))
                    elseif (T == integer)
                        return `storagecast
                    `()
        __rimply =
            spice-cast-macro
                fn "CEnum-imply" (vT T)
                    let ST = ('storageof T)
                    if (vT == ST)
                        return `(inline (self) (bitcast self T))
                    `()

#-------------------------------------------------------------------------------
# string construction
#-------------------------------------------------------------------------------

typedef+ string
    inline... __typecall
    case (cls : type, buf : rawstring,)
        sc_string_new_from_cstr buf
    case (cls : type, buf : rawstring, size : usize)
        sc_string_new buf size

#-------------------------------------------------------------------------------
# defer
#-------------------------------------------------------------------------------

@@ memo
inline defer-type (f)
    typedef (.. "<defer " (tostring f) ">") :: (storageof none)
        inline __drop (self)
            f;
            ;

spice defer (f args...)
    spice-quote
        bitcast none
            defer-type
                inline ()
                    f args...
        ;

#-------------------------------------------------------------------------------
# type initializers
#-------------------------------------------------------------------------------

typedef TypeInitializer
    inline __static-imply (cls T)
        inline (value)
            (bitcast value Closure) T

inline typeinit (...)
    bitcast
        inline (T)
            T ...
        typedef "typeinit" < TypeInitializer : (storageof Closure)

#-------------------------------------------------------------------------------
# Accessors
#-------------------------------------------------------------------------------

typedef+ Accessor
    inline __typecall (cls closure)
        Closure->Accessor closure

#-------------------------------------------------------------------------------
# hex/oct/bin conversion
#-------------------------------------------------------------------------------

fn integer->string (value base)
    let N = 65
    let T = (typeof value)
    let digits = (alloca-array i8 N)
    let absvalue = (abs value)
    let neg? = (value != absvalue)
    loop (i value = N absvalue)
        if (i == 0)
            break (string digits N)
        let i = (i - 1)
        let digit = ((value % base) as i8)
        digits @ i =
            + digit
                ? (digit >= 10:i8) (97:i8 - 10:i8) 48:i8
        let value = (value // base)
        if (value == (0 as T))
            let i =
                if ((i > 0) & neg?)
                    let i = (i - 1)
                    digits @ i = 45:i8
                    i
                else i
            break (string (& (digits @ i)) ((N - i) as usize))
        repeat i value

fn bin (value)
    let value = (value as integer)
    integer->string value (2 as (typeof value))

fn oct (value)
    let value = (value as integer)
    integer->string value (8 as (typeof value))

fn dec (value)
    let value = (value as integer)
    integer->string value (10 as (typeof value))

fn hex (value)
    let value = (value as integer)
    integer->string value (16 as (typeof value))

#-------------------------------------------------------------------------------
# constants
#-------------------------------------------------------------------------------

""""See `pi`.
let pi:f32 = 3.141592653589793:f32
""""See `pi`.
let pi:f64 = 3.141592653589793:f64
""""The number π, the ratio of a circle's circumference C to its diameter d.
    Explicitly type-annotated versions of the constant are available as `pi:f32`
    and `pi:f64`.
let pi = pi:f32

""""See `e`.
let e:f32 = 2.718281828459045:f32
""""See `e`.
let e:f64 = 2.718281828459045:f64
""""Euler's number, also known as Napier's constant. Explicitly type-annotated
    versions of the constant are available as `e:f32` and `e:f64`
let e = e:f32

#-------------------------------------------------------------------------------

unlet _memo dot-char dot-sym ellipsis-symbol _Value constructor destructor
    \ gen-tupleof nested-struct-field-accessor nested-union-field-accessor
    \ tuple== gen-arrayof

run-stage; # 12

#-------------------------------------------------------------------------------

# the main loop must stay in core.sc to make sure that it remains
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
    io-write! "   "; io-write! (default-styler style-number "\\\\\\   ")
    io-write! (compiler-version-string); io-write! "\n";
    io-write! " "; io-write! (default-styler style-comment "///")
    io-write! (default-styler style-sfxfunction "\\\\\\")
    io-write! "  http://scopes.rocks"; io-write! "\n";
    io-write! (default-styler style-comment "///"); io-write! "  "
    io-write! (default-styler style-function "\\\\\\"); io-write! "\n"

#-------------------------------------------------------------------------------

set-globals! (__this-scope)

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

let minus-char = 45:i8 # "-"
fn run-main ()
    let argc argv = (launch-args)
    let exename = (load (getelementptr argv 0))
    let exename = (sc_string_new_from_cstr exename)
    let sourcepath = (alloca string)
    let parse-options = (alloca bool)
    store "" sourcepath
    store true parse-options
    let start-offset =
        loop (i = 1)
            if (i >= argc)
                break i
            let k = (i + 1)
            let arg = (load (getelementptr argv i))
            let arg = (sc_string_new_from_cstr arg)
            if ((load parse-options) and ((@ arg 0:usize) == minus-char))
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
                break k
            k
    let sourcepath = (load sourcepath)
    let sourcepath =
        if (sourcepath == "")
            .. compiler-dir "/lib/scopes/console.sc"
        else sourcepath
    let argc = (argc - start-offset)
    let argv = (& (@ argv start-offset))
    @@ spice-quote
    fn script-launch-args ()
        return sourcepath argc argv
    let scope =
        'bind-symbols
            sc_scope_new_subscope_with_docstring (globals) ""
            script-launch-args = script-launch-args
    do
        hide-traceback;
        load-module "" sourcepath
            scope = scope
            main-module? = true
        ;
    exit 0

raising Error
hide-traceback;
if true
    run-main;

return;