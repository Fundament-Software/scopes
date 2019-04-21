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

# execute until here and treat the remainder as a new translation unit
run-stage; # 1

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

fn box-empty ()
    sc_argument_list_new;

fn box-none ()
    sc_const_aggregate_new Nothing 0 (undef ValueArrayPointer)

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

inline raises-compile-error ()
    if false
        error "hidden"

fn type< (T superT)
    sc_type_is_superof superT T

fn type> (superT T)
    sc_type_is_superof superT T

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
    fn (args) (compare-type args (static-typify f type type))

let static-error =
    box-spice-macro
        fn "static-error" (args)
            if false
                return `()
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
                if (icmp== argcount 2) (unbox-integer (sc_getarg args 1) i32)
                else 0
            hide-traceback;
            `[(sc_type_element_at self index)]

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
            return `(bitcast self T)

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

# method call syntax
sc_type_set_symbol Symbol '__call
    box-spice-macro
        fn "symbol-call" (args)
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
            let value = (sc_getarg args 2)
            return self key value
        else
            let arg = (sc_getarg args 1)
            let key = (sc_type_key (sc_value_qualified_type arg))
            let arg = (sc_keyed_new unnamed arg)
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
                            return `()
                error "all arguments must be constant"

    inline gen-key-any-define-internal (selftype fset)
        box-spice-macro
            fn "define-internal-symbol" (args)
                let self key value = (get-key-value-args args)
                if (sc_value_is_constant self)
                    if (sc_value_is_constant key)
                        let self = (unbox-pointer self selftype)
                        let key = (unbox-symbol key Symbol)
                        fset self key value
                        return `()
                error "scope and key must be constant"

    # quick assignment of type attributes
    sc_type_set_symbol type 'set-symbol (gen-key-any-set type sc_type_set_symbol)
    sc_type_set_symbol Scope 'set-symbol (gen-key-any-set Scope sc_scope_set_symbol)
    sc_type_set_symbol type 'define-symbol (gen-key-any-define type sc_type_set_symbol)
    sc_type_set_symbol Scope 'define-symbol (gen-key-any-define Scope sc_scope_set_symbol)
    sc_type_set_symbol Scope 'define-internal-symbol (gen-key-any-define-internal Scope sc_scope_set_symbol)

# static pointer type constructor
sc_type_set_symbol pointer '__typecall
    box-spice-macro
        fn "pointer.__typecall" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 2 2
            let self = (sc_getarg args 1)
            let T = (unbox-pointer self type)
            `[(sc_pointer_type T pointer-flag-non-writable unnamed)]

# dynamic pointer type constructor
sc_type_set_symbol pointer 'type
    box-pointer
        inline "pointer.type" (T)
            sc_pointer_type T pointer-flag-non-writable unnamed

# static tuple type constructor
sc_type_set_symbol tuple '__typecall
    box-spice-macro
        fn "tuple.__typecall" (args)
            let argcount = (sc_argcount args)
            verify-count argcount 1 -1
            let pcount = (sub argcount 1)
            let types = (alloca-array type pcount)
            loop (i = 1)
                if (icmp== i argcount)
                    break;
                let arg = (sc_getarg args i)
                let k = (sc_type_key (sc_value_qualified_type arg))
                let arg = (unbox-pointer arg type)
                store (sc_key_type k arg)
                    getelementptr types (sub i 1)
                add i 1
            sc_valueref_tag (sc_value_anchor args)
                `[(sc_tuple_type pcount types)]

# dynamic tuple type constructor
sc_type_set_symbol tuple 'type
    box-spice-macro
        fn "tuple.type" (args)
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
                sc_tuple_type argcount types

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
            `([(? value thenf elsef)])

sc_type_set_symbol Value '__typecall
    box-spice-macro
        fn (args)
            raises-compile-error;
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
            raises-compile-error;
            let argc = (sc_argcount args)
            let newargs = (sc_argument_list_new)
            let anchor = (sc_value_anchor args)
            loop (i = 1)
                if (icmp== i argc)
                    sc_argument_list_append newargs
                        sc_valueref_tag anchor `'()
                    break
                        sc_valueref_tag anchor `(cons newargs)
                sc_argument_list_append newargs (sc_getarg args i)
                add i 1

run-stage; # 3

inline decons (self count)
    let count =
        static-branch (none? count)
            inline () 1
            inline () count
    let at next = (sc_list_decons self)
    _ at
        static-branch (const.icmp<=.i32.i32 count 1)
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
    next = sc_scope_next
    next-deleted = sc_scope_next_deleted
    docstring = sc_scope_get_docstring
    set-docstring = sc_scope_set_docstring
    parent = sc_scope_get_parent

'define-symbols string
    join = sc_string_join
    match? = sc_string_match

'define-symbols Error
    format = sc_format_error
    dump = sc_dump_error
    inline append (self anchor traceback-msg)
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
    plain? = sc_type_is_plain
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
    key = sc_type_key
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

let mutable =
    spice-macro
        fn (args)
            let argc = (sc_argcount args)
            verify-count argc 1 2
            if (icmp== argc 1)
                let self = (sc_getarg args 0)
                let T = (unbox-pointer self type)
                return `[('mutable T)]
            elseif (ptrcmp== (unbox-pointer (sc_getarg args 0) type) pointer)
                let self = (sc_getarg args 1)
                let T = (unbox-pointer self type)
                let T = (sc_pointer_type T pointer-flag-non-writable unnamed)
                return `[('mutable T)]
            error "syntax: (mutable pointer-type) or (mutable pointer type)"

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
            raises-compile-error;
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
            raises-compile-error;
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
    raises-compile-error;
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
            if ('constant? self)
                # allow destructive conversions
                let selfST = ('storageof selfT)
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

fn integer-static-imply (vT T)
    if (type< T integer)
        let ST = ('storageof T)
        if (icmp== ('kind ST) type-kind-integer)
            return `(inline (self) (static-integer->integer self T))
    elseif (type< T real)
        let ST = ('storageof T)
        if (icmp== ('kind ST) type-kind-real)
            let valw = ('bitcount vT)
            let destw = ('bitcount ST)
            return `(inline (self) (static-integer->real self T))
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
            if ('signed? vT)
                return `(inline (self) (sitofp self T))
            else
                return `(inline (self) (uitofp self T))
    `()

# only perform safe casts: i.e. float to double
fn real-imply (vT T)
    if (type< T real)
        let ST = ('storageof T)
        if (icmp== ('kind ST) type-kind-real)
            let valw = ('bitcount vT)
            let destw = ('bitcount ST)
            if (icmp== destw valw)
                return `(inline (self) (bitcast self T))
            elseif (icmp>s destw valw)
                return `(inline (self) (fpext self T))
    `()

# more aggressive cast that converts from all numerical types
fn real-as (vT T)
    if (type< T real)
        let ST = ('storageof T)
        let kind = ('kind ST)
        if (icmp== kind type-kind-real)
            let valw destw = ('bitcount vT) ('bitcount ST)
            if (icmp== destw valw)
                return `(inline (self) (bitcast self T))
            elseif (icmp>s destw valw)
                return `(inline (self) (fpext self T))
            else
                return `(inline (self) (fptrunc self T))
    elseif (type< T integer)
        let ST = ('storageof T)
        let kind = ('kind ST)
        if (icmp== kind type-kind-integer)
            if ('signed? ST)
                return `(inline (self) (fptosi self T))
            else
                return `(inline (self) (fptoui self T))
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

fn cast-converter (symbol rsymbol vT T)
    """"for two given types, find a matching conversion function
        this function only works inside a spice macro
    label next
        let f =
            try ('@ vT symbol)
            except (err) (merge next)
        let conv = (sc_prove `(f vT T))
        if (operator-valid? conv) (return conv)
    label next
        let f =
            try ('@ T rsymbol)
            except (err) (merge next)
        let conv = (sc_prove `(f vT T))
        if (operator-valid? conv) (return conv)
    return (sc_empty_argument_list)

fn imply-converter (vT T static?)
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
            cast-converter '__static-imply '__static-rimply vT T
        if (operator-valid? conv) (return conv)
    cast-converter '__imply '__rimply vT T

fn as-converter (vT T static?)
    if (ptrcmp== vT T)
        return `_
    if (sc_type_is_superof T vT)
        return `_
    if (ptrcmp== T bool)
        try
            return ('@ vT '__tobool)
        except (err)
    let conv = (cast-converter '__as '__ras vT T)
    if (operator-valid? conv) (return conv)
    # try implicit cast last
    if static?
        let conv =
            cast-converter '__static-imply '__static-rimply vT T
        if (operator-valid? conv) (return conv)
    cast-converter '__imply '__rimply vT T

inline gen-cast-op (f str)
    spice-macro
        fn "cast-op" (args)
            let argc = ('argcount args)
            verify-count argc 2 2
            let value = ('getarg args 0)
            let anyT = ('getarg args 1)
            let vT = ('typeof value)
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

fn unary-op-error (friendly-op-name T)
    error
        'join "can't "
            'join friendly-op-name
                'join " value of type " ('__repr (box-pointer T))

fn binary-op-error (friendly-op-name lhsT rhsT)
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
    `(f u)

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
    spice-macro (fn (args) (balanced-binary-operation args symbol rsymbol friendly-op-name))

inline unbalanced-binary-op-dispatch (symbol rtype friendly-op-name)
    spice-macro (fn (args) (unbalanced-binary-operation args symbol rtype friendly-op-name))

inline spice-binary-op-macro (f)
    """"to be used for binary operators of which either type can
        provide an operation. returns a callable operator (f lhs rhs) that
        performs the operation or no arguments if the operation can not be
        performed.
    spice-macro
        fn (args)
            raises-compile-error;
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

inline simple-folding-autotype-signed-binary-op (sf uf unboxer)
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
                                    `[(sf lhs rhs)]
                                else
                                    `[(uf lhs rhs)]
                            elseif signed?
                                `(sf lhs rhs)
                            else
                                `(uf lhs rhs)
                            'anchor args
            if (ptrcmp== lhsT rhsT)
                return `f
            `()

# support for calling macro functions directly
'set-symbols SugarMacro
    __call =
        box-pointer
            spice-macro
                fn (args)
                    raises-compile-error;
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
    __== = (box-pointer (simple-binary-op icmp==))
    __!= = (box-pointer (simple-binary-op icmp!=))
    __imply =
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
    verify-count argc 2 2
    let cond elsef =
        'getarg args 0
        'getarg args 1
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
                if (bxor value flip) cond
                else call-elsef
    elseif flip
        return call-elsef
    else
        return cond
    let ifval = (sc_if_new)
    if flip
        sc_if_append_then_clause ifval condbool call-elsef
        sc_if_append_else_clause ifval cond
    else
        sc_if_append_then_clause ifval condbool cond
        sc_if_append_else_clause ifval call-elsef
    ifval

let safe-shl =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 2 2
            let lhs rhs = ('getarg args 0) ('getarg args 1)
            let rhsT = ('typeof rhs)
            let bits = ('bitcount ('typeof lhs))
            let mask = (zext (sub bits 1) u64)
            let mask = (sc_const_int_new rhsT mask)
            # mask right hand side by bit width
            `(shl lhs (band rhs mask))

'set-symbols integer
    __tobool = (box-pointer (spice-macro integer-tobool))
    __imply = (box-pointer (spice-cast-macro integer-imply))
    __static-imply = (box-pointer (spice-cast-macro integer-static-imply))
    __as = (box-pointer (spice-cast-macro integer-as))
    __+ = (box-pointer (simple-folding-binary-op add sc_const_int_extract sc_const_int_new))
    __- = (box-pointer (simple-folding-binary-op sub sc_const_int_extract sc_const_int_new))
    __neg = (box-pointer (inline (self) (sub (nullof (typeof self)) self)))
    __* = (box-pointer (simple-folding-binary-op mul sc_const_int_extract sc_const_int_new))
    __// = (box-pointer (simple-signed-binary-op sdiv udiv))
    __/ =
        box-pointer
            simple-signed-binary-op
                inline (a b) (fdiv (sitofp a f32) (sitofp b f32))
                inline (a b) (fdiv (uitofp a f32) (uitofp b f32))
    __rcp = (spice-quote (inline (self) (fdiv 1.0 (as self f32))))
    __% = (box-pointer (simple-signed-binary-op srem urem))
    __& = (box-pointer (simple-folding-binary-op band sc_const_int_extract sc_const_int_new))
    __| = (box-pointer (simple-folding-binary-op bor sc_const_int_extract sc_const_int_new))
    __^ = (box-pointer (simple-folding-binary-op bxor sc_const_int_extract sc_const_int_new))
    __<< = (box-pointer (simple-binary-op safe-shl))
    __>> = (box-pointer (simple-signed-binary-op ashr lshr))
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
    __== = (box-pointer (simple-binary-op fcmp==o))
    __!= = (box-pointer (simple-binary-op fcmp!=u))
    __> = (box-pointer (simple-binary-op fcmp>o))
    __>= = (box-pointer (simple-binary-op fcmp>=o))
    __< = (box-pointer (simple-binary-op fcmp<o))
    __<= = (box-pointer (simple-binary-op fcmp<=o))
    __+ = (box-pointer (simple-binary-op fadd))
    __- = (box-pointer (simple-binary-op fsub))
    __neg = (box-pointer (inline (self) (fsub (nullof (typeof self)) self)))
    __* = (box-pointer (simple-binary-op fmul))
    __/ = (box-pointer (simple-binary-op fdiv))
    __rcp = (box-pointer (inline (self) (fdiv (uitofp 1 (typeof self)) self)))
    __// = (box-pointer (simple-binary-op floordiv))
    __% = (box-pointer (simple-binary-op frem))


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
    __== = (box-pointer (simple-binary-op ptrcmp==))
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
                'tag `(sc_scope_at args) ('anchor args)
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
                case 1 `(sc_scope_new)
                case 2 `(sc_scope_new_subscope [ ('getarg args 1) ])
                default
                    # argc == 3
                    let parent = ('getarg args 1)
                    if (type== ('typeof parent) Nothing)
                        `(sc_scope_clone [ ('getarg args 2) ])
                    else
                        `(sc_scope_clone_subscope
                            [(sc_extract_argument_list_new args 1)])

#---------------------------------------------------------------------------
# nothing type
#---------------------------------------------------------------------------

'set-symbols Nothing
    __tobool = (box-pointer (inline () false))

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
    / = (unary-or-balanced-binary-op-dispatch '__rcp "invert" '__/ '__r/ "real-divide")
    // = (balanced-binary-op-dispatch '__// '__r// "integer-divide")
    % = (balanced-binary-op-dispatch '__% '__r% "modulate")
    & = (unary-or-balanced-binary-op-dispatch '__toptr "reference" '__& '__r& "apply bitwise-and to")
    | = (balanced-binary-op-dispatch '__| '__r| "apply bitwise-or to")
    ^ = (balanced-binary-op-dispatch '__^ '__r^ "apply bitwise-xor to")
    << = (balanced-binary-op-dispatch '__<< '__r<< "apply left shift with")
    >> = (balanced-binary-op-dispatch '__>> '__r>> "apply right shift with")
    .. = (balanced-binary-op-dispatch '__.. '__r.. "join")
    = = (balanced-binary-op-dispatch '__= '__r= "apply assignment with")
    @ = (unary-or-unbalanced-binary-op-dispatch '__toref "dereference" '__@ integer "apply subscript operator with")
    getattr = (unbalanced-binary-op-dispatch '__getattr Symbol "get attribute from")
    lslice = (unbalanced-binary-op-dispatch '__lslice usize "apply left-slice operator with")
    rslice = (unbalanced-binary-op-dispatch '__rslice usize "apply right-slice operator with")

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
            let s =
                try
                    let f = (sc_type_at T '__repr)
                    `(f value)
                except (err)
                    `(sc_value_content_repr value)
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
                else (_ ('typeof value) ('constant? value))
            let conv = (imply-converter valueT T constant)
            let result = (operator-valid? conv)
            `result

let imply? = (gen-cast? imply-converter)
let as? = (gen-cast? as-converter)

'set-symbols integer
    __~ = (box-pointer (inline (self) (^ self (as -1 (typeof self)))))

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
    __= = (box-pointer (simple-binary-op (inline (lhs rhs) (__drop lhs) (assign rhs lhs))))
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

fn empty? (value)
    == (countof value) 0

#fn cons (at next)
    sc_list_cons (Value at) next

let print =
    do
        inline print-element (i key value)
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
            raises-compile-error;
            let anchor = ('__repr `[('anchor args)])
            spice-quote
                print anchor args
                view args

'define-symbol integer '__typecall
    inline (cls value)
        static-branch (none? value)
            inline () (nullof cls)
            inline ()
                as value cls

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
                                if (== argT paramT) arg
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

'set-symbols pointer
    __call = coerce-call-arguments
    __imply = (box-pointer (spice-cast-macro pointer-imply))

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
        `[(cons prec (cons order (cons func '())))]
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
                parse-infix-expr infix-table rhs state nextop-prec
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
            '@ env (as head-key Symbol)
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
        return (as expr list) env
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
        case 45:i8 # -
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
        _ `[(list f at (list inline '() result))] next

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
        let argkey = ('key ('qualified-typeof arg))
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
let package = (Scope)
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
        return (Scope b a)
    Scope
        clone-scope-contents parent b
        a

'set-symbols Scope
    __.. = (box-pointer (simple-binary-op clone-scope-contents))

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
                `[('constant? value)]
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
                raises-compile-error;
                let scope rest = (decons args)
                return none
                    as scope Scope

'define-internal-symbol (__this-scope) list-handler-symbol
    box-pointer (static-typify list-handler list Scope)
'define-internal-symbol (__this-scope) symbol-handler-symbol
    box-pointer (static-typify symbol-handler list Scope)

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
                    error
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

run-stage; # 5

inline make-inplace-let-op (op)
    sugar-macro
        fn expand-infix-let (expr)
            raises-compile-error;
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
                raises-compile-error;
                let name value = (decons expr 2)
                qq [let] [name] = [value]
    as:= = (make-inplace-let-op as)
    <- =
        sugar-macro
            fn expand-apply (expr)
                raises-compile-error;
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
            spice-quote
                let start valid? at next =
                    (as [(sc_expand generator-expr '() subscope)] Generator);
            return
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
                                            'anchor generator-expr
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
                                continue;
                            else
                                break;
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
            ``args

let incomplete = (typename "incomplete")

run-stage; # 6

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
    let ModuleFunctionType = (pointer (raises (function Value) Error))
    let StageFunctionType = (pointer (raises (function CompileStage) Error))
    let expr-anchor = ('anchor expr)
    let f =
        do
            hide-traceback;
            sc_eval expr-anchor (expr as list) eval-scope
    loop (f = f)
        # build a wrapper
        let wrapf =
            spice-quote
                fn "exec-module-stage" ()
                    raises-compile-error;
                    hide-traceback;
                    wrap-if-not-run-stage (f)
        let wrapf = (sc_typify_template wrapf 0 (undef TypeArrayPointer))
        let f = (sc_compile wrapf 0:u64)
        if (('typeof f) == StageFunctionType)
            let fptr = (f as StageFunctionType)
            let result =
                do
                    hide-traceback;
                    fptr;
            repeat (bitcast result Value)
        else
            let fptr = (f as ModuleFunctionType)
            let result =
                do
                    hide-traceback;
                    fptr;
            break result

fn dots-to-slashes (pattern)
    let sz = (countof pattern)
    loop (i start result = 0:usize 0:usize "")
        if (i == sz)
            return (.. result (rslice pattern start))
        let c = (@ pattern i)
        if (c == (char "/"))
            error
                .. "no slashes permitted in module name: " pattern
        elseif (c == (char "\\"))
            error
                .. "no slashes permitted in module name: " pattern
        elseif (c != (char "."))
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
    let expr = (sc_parse_from_path module-path)
    let eval-scope =
        va-option scope opts...
            do
                let newscope = (Scope (sc_get_globals))
                'set-docstring newscope unnamed ""
                newscope
    'set-symbols eval-scope
        main-module? =
            va-option main-module? opts... false
        module-path = module-path
        module-dir = module-dir
        module-name = module-name
    hide-traceback;
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
                    error "failed to import module"
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
                let content =
                    do
                        hide-traceback;
                        load-module (name as string) module-path
                'set-symbol modules module-path-sym content
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
                    if ((@ namestr i) == (char "."))
                        if (i == start)
                            repeat (add i 1:usize) (add i 1:usize)
                    repeat (add i 1:usize) start
            let sxname rest = (decons args)
            let name = (sxname as Symbol)
            let namestr = (name as string)
            let module-dir = (scope.module-dir as string)
            let key = (resolve-scope scope namestr 0:usize)
            let module =
                do
                    hide-traceback;
                    require-from module-dir name
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
                            'set-docstring constant-scope key keydocstr
                            `none
                        else
                            spice-quote
                                sc_scope_set_symbol tmp key `value
                                sc_scope_set_docstring tmp key keydocstr

            let build-locals =
                spice-macro
                    fn (args)
                        let scope = (('getarg args 0) as Scope)
                        let docstr = ('docstring scope unnamed)
                        let constant-scope = (Scope)
                        if (not (empty? docstr))
                            'set-docstring constant-scope unnamed docstr
                        let tmp = `(Scope constant-scope)
                        let block = (sc_expression_new)
                        sc_expression_append block tmp
                        loop (last-key = unnamed)
                            let key value = ('next scope last-key)
                            if (key == unnamed)
                                sc_expression_append block tmp
                                return block
                            #if (not (stage-constant? value))
                            let keydocstr = ('docstring scope key)
                            let value = ('tag (sc_extract_argument_new value 0) ('anchor value))
                            sc_expression_append block
                                `(build-local constant-scope tmp key value keydocstr)
                            repeat key

            return `(build-locals scope) scope

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
                hide-traceback;
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
                        error
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
            let nameval =
                if (('typeof nameval) == list)
                    let val = (sc_expand nameval '() sugar-scope)
                    val
                else nameval
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
            raises-compile-error;
            let name body = (decons expr)
            list define name
                list sugar-macro
                    cons fn '(args)
                        list raises-compile-error;
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
            fn check-assertion (result msg)
                if (not result)
                    hide-traceback;
                    error
                        .. "assertion failed: " msg
                return;

            let argc = ('argcount args)
            verify-count argc 2 2
            let expr msg =
                'getarg args 0
                'getarg args 1
            if (('typeof msg) != string)
                error "string expected as second argument"
            'tag `(check-assertion expr msg) ('anchor args)

fn gen-vector-reduction (f v sz)
    if false
        return `[]
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
                let rhs = (gen-vector-reduction f rhs (sz - 1))
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
    inline scope-generator (self)
        Generator
            inline () (sc_scope_next self unnamed)
            inline (key value) (key != unnamed)
            inline (key value) (_ key value)
            inline (key value)
                sc_scope_next self key

    'set-symbols Scope
        deleted =
            inline (self)
                Generator
                    inline () (sc_scope_next_deleted self unnamed)
                    inline (key) (key != unnamed)
                    inline (key) key
                    inline (key)
                        sc_scope_next_deleted self key
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

    fn i8->string(c)
        let ptr = (alloca i8)
        store c ptr
        sc_string_new ptr 1

    'set-symbols string
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
    append-sink =
        inline "Value-args" (self)
            let argc = ('argcount self)
            Collector
                inline () self
                inline (self) true
                inline (self) self
                inline (src self)
                    sc_argument_list_append self (src)
                    self

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
        inline () from
        inline (x) (x < to)
        inline (x) x
        inline (x) (x + step)

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

    inline compile-glsl (target func flags...)
        sc_compile_glsl target func (parse-compile-flags flags...)

    inline compile-spirv (target func flags...)
        sc_compile_spirv target func (parse-compile-flags flags...)

    inline compile-object (func table flags...)
        sc_compile_object func table (parse-compile-flags flags...)

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
            let outargs = (sc_argument_list_new)
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
                raises-compile-error;
                let cls = (('getarg args 0) as type)
                if (cls == array)
                    verify-count argc 3 3
                    let element-type = (('getarg args 1) as type)
                    let size = (('getarg args 2) as i32)
                    `[(sc_array_type element-type (size as usize))]
                else
                    verify-count argc 1 1
                    `(nullof cls)
    __as =
        do
            inline array-generator (arr)
                let count = (countof arr)
                let stackarr = (ptrtoref (alloca (typeof arr)))
                stackarr = arr
                Generator
                    inline () 0:usize
                    inline (x) (< x count)
                    inline (x) (@ stackarr x)
                    inline (x) (+ x 1:usize)
            spice-cast-macro
                fn "array.__as" (vT T)
                    if (T == Generator)
                        return `array-generator
                    `()

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

inline signed-vector-binary-op (sf uf)
    spice-macro
        fn (args)
            raises-compile-error;
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
    __vector>> = (simple-signed-binary-op ashr lshr)
    __vector== = icmp==
    __vector!= = icmp!=
    __vector>  = (signed-vector-binary-op icmp>s icmp>u)
    __vector>= = (signed-vector-binary-op icmp>s icmp>=u)
    __vector<  = (signed-vector-binary-op icmp<s icmp<u)
    __vector<= = (signed-vector-binary-op icmp<=s icmp<=u)

'set-symbols real
    __vector+  = fadd
    __vector-  = fsub
    __vector*  = fmul
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
                raises-compile-error;
                let cls = (('getarg args 0) as type)
                if (cls == vector)
                    verify-count argc 3 3
                    let element-type = (('getarg args 1) as type)
                    let size = (('getarg args 2) as i32)
                    `[(sc_vector_type element-type (size as usize))]
                else
                    verify-count argc 1 1
                    `(nullof cls)

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
    let binding = (va-option location attrs... -1)
    sc_global_new name T flags storage-class location binding

let extern =
    spice-macro
        fn (args)
            let argc = ('argcount args)
            verify-count argc 2 -1
            raises-compile-error;
            let name = (('getarg args 0) as Symbol)
            let T = (('getarg args 1) as type)
            loop (i flags storage-class location binding = 2 0:u32 unnamed -1 -1)
                if (i == argc)
                    break
                        `[(sc_global_new name T
                            flags storage-class location binding)]
                let arg = ('getarg args i)
                let k v = ('dekey arg)
                let flags storage-class location binding =
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
                        _ (bor flags newflag) storage-class location binding
                    elseif (k == 'storage-class)
                        _ flags (arg as Symbol) location binding
                    elseif (k == 'location)
                        _ flags storage-class (arg as i32) binding
                    elseif (k == 'binding)
                        _ flags storage-class location (arg as i32)
                    else
                        error ("unrecognized key: " .. (repr k))
                _ (i + 1) flags storage-class location binding

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
        fn (topexpr scope)
            let expr next = (decons topexpr)
            let expr = (expr as list)
            let head arg argrest = (decons expr 2)
            let arg argrest = (sc_expand arg argrest scope)
            let outnext = (alloca-array list 1)
            let outexpr next =
                spice-quote
                    label ok-label
                        inline return-ok (args...)
                            merge ok-label args...
                        spice-unquote
                            let outexpr = (sc_expression_new)
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
                                    hide-traceback;
                                    error "default branch missing"
            return (cons outexpr (load outnext)) scope

fn gen-sugar-matcher (failfunc expr scope params)
    if false
        return `()
    let params = (params as list)
    let paramcount = (countof params)
    let outexpr = (sc_expression_new)
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
                            error
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
            elseif (T == string)
                let str = (paramv as string)
                sc_expression_append outexpr
                    spice-quote
                        let arg next = (sc_list_decons next)
                        if (ptrcmp!= ('typeof arg) string)
                            failfunc;
                        if ((arg as string) != str)
                            failfunc;
                repeat (i + 1) rest next varargs
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
                hide-traceback;
                error@ ('anchor paramv) "while parsing pattern" "unsupported pattern"
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
            fn (topexpr scope)
                let new-expr new-next = (f topexpr scope)
                let x next = (decons topexpr)
                let anchor = ('anchor x)
                let new-expr =('tag `new-expr anchor)
                return
                    static-branch (none? new-next)
                        inline ()
                            cons new-expr next
                        inline ()
                            cons new-expr new-next
                    scope

    sugar-block-scope-macro
        fn "expand-sugar" (topexpr scope)
            raises-compile-error;
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
                                    'set-symbols subscope
                                        next-expr = next-expr
                                        sugar-scope = sugar-scope
                                        expr-head = head
                                        expression = _expr
                                    let unpack-expr =
                                        gen-sugar-matcher fail-case expr subscope params
                                    let body =
                                        sc_expand (cons do body) '() subscope
                                    spice-quote
                                        unpack-expr
                                        return-ok body
                            error "syntax error"
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
            else `current
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

#inline spice-macro (f)
    spice-macro-verify-signature f
    bitcast (static-typify f Value) SpiceMacro

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
                let content =
                    cons (list args)
                        'tag
                            Value
                                qq
                                    [verify-count] ([`sc_argcount args])
                                        [(? varargs (sub paramcount 1) paramcount)]
                                        [(? varargs -1 paramcount)]
                            'anchor name
                        body
                break
                    if (('typeof name) == Symbol)
                        qq
                            [let name] =
                                [spice-macro]
                                    [fn] [(name as Symbol as string)] (args)
                                        [Value]
                                            [(cons inline content)] args
                    else
                        qq
                            [spice-macro]
                                [fn name] (args)
                                    [Value]
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
        return `()
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
    spice-quote
        if (expr != cond)
            failfunc;

#-------------------------------------------------------------------------------

define match
    gen-match-block-parser gen-match-matcher

let OverloadedFunction = (typename "OverloadedFunction")
'set-opaque OverloadedFunction

"""" (va-append-va (inline () (_ b ...)) a...) -> a... b...
define va-append-va
    spice-macro
        fn "va-va-append" (args)
            raises-compile-error;
            let argc = ('argcount args)
            verify-count argc 1 -1
            let end = ('getarg args 0)
            let newargs = (sc_argument_list_new)
            loop (i = 1)
                if (i == argc)
                    sc_argument_list_append newargs ('tag `(end) ('anchor end))
                    break newargs
                sc_argument_list_append newargs ('getarg args i)
                repeat (i + 1)

define va-empty?
    spice-macro
        fn "va-empty?" (args)
            raises-compile-error;
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
            #raises-compile-error;
            let argc = ('argcount args)
            verify-count argc 1 -1
            let f = ('getarg args 0)
            let outargs = (sc_argument_list_new)
            loop (i = 1)
                if (i == argc)
                    break outargs
                let arg = ('getarg args i)
                let outarg = (sc_prove `(f arg))
                if (('typeof outarg) != void)
                    sc_argument_list_append outargs outarg
                i + 1

"""".. spice:: (va-range a (? b))

       If `b` is not specified, returns a sequence of integers from zero to `b`,
       otherwise a sequence of integers from `a` to `b`.
define va-range
    spice-macro
        fn "va-range" (args)
            #raises-compile-error;
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
            let outargs = (sc_argument_list_new)
            loop (i = a)
                if (i == b)
                    break outargs
                sc_argument_list_append outargs `i
                i + 1

"""" (va-split n a...) -> (inline () a...[n .. (va-countof a...)-1]) a...[0 .. n-1]
define va-split
    spice-macro
        fn "va-split" (args)
            raises-compile-error;
            let argc = ('argcount args)
            verify-count argc 1 -1
            let pos = (('getarg args 0) as i32)
            let largs = (sc_argument_list_new)
            let rargs = (sc_argument_list_new)
            loop (i = 1)
                if (i > pos)
                    break;
                sc_argument_list_append largs ('getarg args i)
                repeat (i + 1)
            loop (i = (pos + 1))
                if (i >= argc)
                    break;
                sc_argument_list_append rargs ('getarg args i)
                repeat (i + 1)
            `(_ (inline () largs) (inline () rargs))

"""" filter all keyed values
define va-unnamed
    spice-macro
        fn "va-unnamed" (args)
            raises-compile-error;
            let argc = ('argcount args)
            verify-count argc 0 -1
            let outargs = (sc_argument_list_new)
            loop (i = 0)
                if (i == argc)
                    break;
                let arg = ('getarg args i)
                let k = (sc_type_key ('qualified-typeof arg))
                if (k == unnamed)
                    sc_argument_list_append outargs arg
                repeat (i + 1)
            outargs

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
# function overloading
#-------------------------------------------------------------------------------

spice overloaded-fn-append (T args...)
    let outtype = (T as type)
    let functions = ('@ outtype 'templates)
    let functypes = ('@ outtype 'parameter-types)
    for i in (range 0 ('argcount args...) 2)
        let f = ('getarg args... i)
        let ftype = ('getarg args... (i + 1))
        if (('typeof ftype) == Nothing)
            # separator for (using ...)
            let fT = ('typeof f)
            if ('function-pointer? fT)
                if ((('kind f) != value-kind-function)
                    and (not ('constant? f)))
                    error "argument must be constant or function"
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
                    error "cannot inherit from own type"
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
                error
                    .. "cannot embed argument of type "
                        repr ('typeof f)
                        " in overloaded function"
        else
            let T = (ftype as type)
            sc_argument_list_append functions f
            sc_argument_list_append functypes ftype
    T

'set-symbols OverloadedFunction
    append = overloaded-fn-append
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
                    let outargs = ('tag (sc_call_new f) ('anchor args))
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
                                let conv = (imply-converter argT paramT ('constant? arg))
                                if (operator-valid? conv) `(conv arg)
                                else (merge break-next)
                        sc_call_append_argument outargs outarg
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
    spice init-overloaded-function (T)
        let T = (T as type)
        'set-symbols T
            templates = (sc_argument_list_new)
            parameter-types = (sc_argument_list_new)
        T

    let inline? =
        (expr-head as Symbol) == 'inline...
    let finalize-overloaded-fn = overloaded-fn-append
    let fn-name =
        sugar-match name...
        case (name as Symbol;) name
        case (name as string;) (Symbol name)
        case () unnamed
        default
            error
                """"syntax: (fn... name|"name") (case pattern body...) ...
    let outargs = (sc_argument_list_new)
    let outtype =
        spice-quote
            init-overloaded-function
                typename [(fn-name as string)] OverloadedFunction
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
                let tmpl = ('tag (sc_template_new fn-name) ('anchor condv))
                if inline?
                    sc_template_set_inline tmpl
                sc_argument_list_append outargs tmpl
                let scope = (Scope bodyscope)
                let types = (sc_argument_list_new)
                loop (expr = (uncomma (condv as list)))
                    sugar-match expr
                    case ()
                        let body = (sc_expand (cons do body...) '() scope)
                        sc_template_set_body tmpl body
                        sc_argument_list_append outargs `(Arguments types)
                        break;
                    case ((arg as Symbol) ': T)
                        hide-traceback;
                        error@ ('anchor condv) "while parsing pattern"
                            "single typed parameter definition is missing trailing comma or semicolon"
                    case ((arg as Symbol) rest...)
                        if ('variadic? arg)
                            if (not (empty? rest...))
                                error "variadic parameter must be in last place"
                        let param = (sc_parameter_new arg)
                        sc_template_append_parameter tmpl param
                        'set-symbol scope arg param
                        sc_argument_list_append types
                            ? ('variadic? arg) Variadic Unknown
                        repeat rest...
                    case (((arg as Symbol) ': T) rest...)
                        if ('variadic? arg)
                            error "a typed parameter can't be variadic"
                        let T = (sc_expand T '() sugar-scope)
                        let param = (sc_parameter_new arg)
                        sc_template_append_parameter tmpl param
                        'set-symbol scope arg param
                        sc_argument_list_append types T
                        repeat rest...
                    default
                        hide-traceback;
                        error@ ('anchor condv) "while parsing pattern"
                            "syntax: (parameter-name[: type], ...)"
            repeat rest...
        default
            sugar-match name...
            case (name as Symbol;)
                'set-symbol sugar-scope fn-name outtype
            default;
            return
                `(finalize-overloaded-fn outtype outargs)
                next

let inline... = fn...

sugar from (src 'let params...)
    spice load-from (src keys...)
        let args = (sc_argument_list_new)
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

define zip (spice-macro (fn (args) (ltr-multiop args `zip 2)))

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
                list static-branch
                    'tag `[(list imply cond bool)] ('anchor cond)
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
        error "condition must be constant"
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
    raises-compile-error;
    let kw body = (decons expr)
    let head = (kw as Symbol)
    let result next-expr =
        loop (body next-expr result = body next-expr '())
            if (empty? next-expr)
                error "expression decorator is not applied to anything"
            let result =
                cons
                    if ((countof body) == 1)
                        '@ body
                    else
                        `body
                    result
            let follow-expr next-next-expr = (decons next-expr)
            break
                cons ('tag `'decorate-vvv ('anchor kw)) follow-expr result
                next-next-expr
    return
        cons result next-expr
        sugar-scope

define-sugar-macro decorate-vvv
    raises-compile-error;
    let expr decorators = (decons args)
    loop (in out = decorators expr)
        if (empty? in)
            break out
        let decorator in = (decons in)
        repeat in
            `[(cons decorator (list out))]

define-sugar-macro decorate-fn
    raises-compile-error;
    let fnexpr decorators = (decons args)
    let kw name body = (decons (fnexpr as list) 2)
    let name-is-symbol? = (('typeof name) == Symbol)
    let fnexpr = `[(list do fnexpr)]
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
    raises-compile-error;
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
    let subscope = (Scope sugar-scope)
    'set-symbol subscope 'sugar-scope sugar-scope
    return
        exec-module `args subscope
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

sugar fold-locals (args...)
    let scope = sugar-scope
    let _1 _2 = (decons args...)
    let init _2 = (sc_expand _1 _2 sugar-scope)
    let _1 _2 = (decons _2)
    let f = (sc_expand _1 _2 sugar-scope)
    let block = (sc_expression_new)
    sc_expression_append block init
    loop (last-key outval = unnamed init)
        let key value = ('next scope last-key)
        if (key == unnamed)
            return block
        let docstr = ('docstring scope key)
        let expr = ('tag `(f outval key docstr value) ('anchor expression))
        sc_expression_append block expr
        repeat key expr

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
    let outargs = (sc_argument_list_new)
    sc_argument_list_append outargs cond
    loop (next-expr = next-expr)
        sugar-match next-expr
        case (('case it body...) rest...)
            let it = (sc_expand it '() sugar-scope)
            let body = (sc_expand (cons embed body...) '() sugar-scope)
            sc_argument_list_append outargs it
            sc_argument_list_append outargs `(inline () [body])
            repeat rest...
        case (('default body...) rest...)
            let body = (sc_expand (cons embed body...) '() sugar-scope)
            sc_argument_list_append outargs `(inline () [body])
            return `(handle-static-match outargs) rest...
        default
            hide-traceback;
            error "default branch missing"

#-------------------------------------------------------------------------------
# unlet
#-------------------------------------------------------------------------------

sugar unlet ((name as Symbol) names...)
    sc_scope_del_symbol sugar-scope name
    for name in names...
        let name = (name as Symbol)
        hide-traceback;
        getattr sugar-scope name
        sc_scope_del_symbol sugar-scope name
    `()

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

       Unlike `for`, `fold` requires both calls to ``break`` and ``continue``
       to pass a state compatible with `state ...`. Otherwise they serve
       the same function.

       Usage example::

            # add numbers from 0 to 9, skipping number 5, and print the result
            print
                fold (sum = 0) for i in (range 100)
                    if (i == 10)
                        # abort the loop
                        break sum
                    if (i == 5)
                        # skip this index
                        continue sum
                    # continue with the next state for sum
                    sum + i
sugar fold ((binding...) 'for expr...)
    let itparams it = ('token-split expr... 'in "'in' expected")
    let foldparams init = ('token-split binding... '= "'=' expected")
    let generator-expr body = (decons it)
    let subscope = (Scope sugar-scope)
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
                    let at... = (at it...)
                    let state... = (state)
                    let newstate... =
                        spice-unquote
                            let expr1 expr2 =
                                cons let ('rjoin itparams (list '= at...))
                                cons let ('rjoin foldparams (list '= state...))
                            'set-symbol subscope 'continue continue
                            let result = (sc_expand (cons do expr1 expr2 body) '() subscope)
                            result
                    repeat (va-append-va (inline () newstate...) (next it...))
                else
                    break (state)

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
    qq [do]
        [let] this-type = [T]
        [let] super-type = ([superof T])
        [do]
            unquote-splice body...
            [fold-locals] this-type [append-to-type]
        this-type

""""a type declaration syntax; when the name is a string, the type is declared
    at runtime.
sugar typedef (name body...)
    let declaration? = (('typeof name) == Symbol)

    if declaration?
        let name-exists =
            try
                getattr sugar-scope (name as Symbol)
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

inline gen-allocator-sugar (name f)
    sugar "" (values...)
        spice local-copy-typed (T value)
            spice-quote
                let val = (ptrtoref (f T))
                assign (imply value T) val
                val
        spice local-copy (value)
            let T = ('typeof value)
            `(local-copy-typed T value)
        spice local-new (T args...)
            spice-quote
                let val = (ptrtoref (f T))
                assign (T args...) val
                val
        let anchor = ('anchor expression)
        let result =
            sugar-match values...
            case (name '= value)
                let callexpr =
                    'tag `[(qq ([local-copy value]))] anchor
                qq [let name] = [callexpr]
            case (name ': T '= value)
                let callexpr =
                    'tag `[(qq ([local-copy-typed T value]))] anchor
                qq [let name] = [callexpr]
            case (name ': T args...)
                let callexpr =
                    'tag `[(qq [local-new T] (unquote-splice args...))] anchor
                qq [let name] = [callexpr]
            case (T args...)
                qq [local-new T] (unquote-splice args...)
            default
                error
                    .. "syntax: " name " <name> [: <type>] [= <value>]"
        'tag `result anchor

let local = (gen-allocator-sugar "local" alloca)
let new = (gen-allocator-sugar "new" malloc)
let global = (gen-allocator-sugar "global" private)

fn delete (value)
    free (reftoptr value)

#-------------------------------------------------------------------------------

define struct-dsl
    sugar : (name T)
        fn define-field-runtime (T name field-type)
            let fields = ('@ T '__fields__)
            sc_argument_list_append fields
                sc_key_type (name as Symbol) (field-type as type)
            sc_type_set_symbol T '__fields__ fields

        spice define-field (struct-type name field-type)
            if ('constant? struct-type)
                let T = (struct-type as type)
                define-field-runtime T name field-type
                `()
            else
                'tag `(define-field-runtime struct-type `name `field-type)
                    'anchor args

        let anchor = ('anchor expression)
        qq [define-field] [('tag `'this-type anchor)] '[name] [T]

    define-infix> 70 :
    locals;

run-stage; # 11

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
    fn gen-code (cfilename targetsym filter code opts)
        let scope =
            do
                hide-traceback;
                (sc_import_c cfilename code opts)
        if (targetsym == unnamed)
            if (empty? filter)
                qq [using] [scope]
            else
                qq [using] [scope] filter [filter]
        else
            qq [let] [targetsym] = [scope]

    let modulename = (('@ sugar-scope 'module-path) as string)
    loop (args targetsym filter modulename ext opts includestr = args... unnamed "" modulename ".c" '() "")
        sugar-match args
        case (('import (name as Symbol)) rest...)
            if (targetsym != unnamed)
                error "duplicate 'import'"
            if (not (empty? filter))
                error "can't use filter with 'import'"
            repeat rest... name filter (.. modulename "." (name as string)) ext opts includestr
        case (('filter (pattern as string)) rest...)
            if (not (empty? filter))
                error "duplicate 'filter'"
            if (targetsym != unnamed)
                error "can't use filter with 'import'"
            repeat rest... targetsym pattern modulename ext opts includestr
        case (('extern "C++") rest...)
            if (modulename == ".cpp")
                error "duplicate 'extern \"C++\"'"
            repeat rest... targetsym filter modulename ".cpp" opts includestr
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
            repeat rest... targetsym filter modulename ext opts includestr
        case ((s as string) rest...)
            if (not (empty? includestr))
                error "duplicate include string"
            repeat rest... targetsym filter modulename ext opts s
        case ()
            if (not (empty? includestr))
                # simple include string
                let includestr =
                    "#include \"" .. includestr .. "\""
                hide-traceback;
                return
                    gen-code (.. modulename ext) targetsym filter includestr opts
                    next-expr
            elseif (not (empty? next-expr))
                # full source code
                let code rest = (decons next-expr)
                if (('typeof code) == string)
                    hide-traceback;
                    return
                        gen-code (.. modulename ext) targetsym filter (code as string) opts
                        rest
            error "string block expected as next expression"
        default
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
                error "Struct type constructor not available"
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
                error "CStruct type constructor is deprecated"
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
    spice finalize-struct (T)
        fn finalize-struct-runtime (T)
            let field-types = ('@ T '__fields__)
            let numfields = ('argcount field-types)
            let fields = (alloca-array type numfields)
            for i field in (enumerate ('args field-types))
                (ptrtoref (getelementptr fields i)) = (field as type)
            if (T < CUnion)
                'set-plain-storage T
                    sc_union_type numfields fields
            elseif (T < CStruct)
                'set-plain-storage T
                    sc_tuple_type numfields fields
            elseif (T < Struct)
                'set-storage T
                    sc_tuple_type numfields fields
            else
                error
                    .. "type " (repr T) " must have Struct, CStruct or CUnion supertype"
                        \ " but has supertype " (repr ('superof T))
        if ('constant? T)
            finalize-struct-runtime (T as type)
            `()
        else
            `(finalize-struct-runtime T)

    let supertype body has-supertype? =
        sugar-match body...
        case ('union rest...)
            _ `CUnion rest... true
        case ('plain rest...)
            _ `CStruct rest... true
        case ('< supertype rest...)
            _ supertype rest... true
        default
            _ `Struct body... false

    let has-fwd-decl =
        if (('typeof name) == Symbol)
            if (empty? body)
                # forward declaration
                return
                    qq [typedef] [name] < [supertype] do

            let symname = (name as Symbol)
            # see if we can find a forward declaration in the local scope
            try (getattr sugar-scope symname) true
            except (err) false
        else false

    spice init-fields (struct-type)
        fn init-fields-runtime (T)
            sc_type_set_symbol T '__fields__ (sc_argument_list_new)

        if ('constant? struct-type)
            init-fields-runtime (struct-type as type)
            `()
        else
            `(init-fields-runtime struct-type)
    qq
        unquote-splice
            if has-fwd-decl
                if has-supertype?
                    hide-traceback;
                    error "completing struct declaration must not define a supertype"
                qq [typedef+] [name]
            else
                qq [typedef] [name] < [supertype] do
        [init-fields] this-type
        [using] [struct-dsl]
        [do]
            unquote-splice body
            [fold-locals] this-type [append-to-type]
        [finalize-struct] this-type
        this-type

# enums
#-------------------------------------------------------------------------------

do
    inline simple-binary-storage-op (f)
        simple-binary-op (inline (a b) (f (storagecast a) (storagecast b)))

    'set-symbols CEnum
        __== = (simple-binary-op icmp==)
        __!= = (simple-binary-op icmp!=)
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

sugar enum (name values...)
    spice make-enum (name vals...)
        let T = (typename.type (name as string) CEnum)

        inline build-type (self)
            let repr-expr = (sc_expression_new)
            inline make-enumval (key val)
                let const = (sc_const_int_new T (sext (as val i32) u64))
                'set-symbol T key const
                let str = (sc_default_styler style-number (key as string))
                sc_expression_append repr-expr
                    spice-quote
                        if (self == const) (return str)
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
                    error "all enum values must be constant"
                _ (i + 1)
                    if (key == unnamed)
                        # auto-numerical
                        make-enumval (as val Symbol) nextval
                        nextval + 1
                    else
                        make-enumval key val
                        (as val i32) + 1

            sc_expression_append repr-expr
                spice-quote
                    return (repr (storagecast self))
            repr-expr

        spice-quote
            fn enum-repr (self)
                spice-unquote
                    build-type self
        'set-symbol T '__repr enum-repr
        T

    fn convert-body (body)
        if false
            # hint return type
            return '()
        let expr body = (decons body)
        cons
            if (('typeof expr) == Symbol)
                `[(list sugar-quote expr)]
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

unlet _memo dot-char dot-sym ellipsis-symbol symbol-handler-symbol
    \ list-handler-symbol struct-dsl _Value

run-stage; # 12

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
    io-write! "   "; io-write! (default-styler style-number "\\\\\\   ")
    io-write! (compiler-version-string); io-write! "\n";
    io-write! " "; io-write! (default-styler style-comment "///")
    io-write! (default-styler style-sfxfunction "\\\\\\")
    io-write! "  http://scopes.rocks"; io-write! "\n";
    io-write! (default-styler style-comment "///"); io-write! "  "
    io-write! (default-styler style-function "\\\\\\"); io-write! "\n"

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

    let global-scope = (globals)
    let eval-scope = (Scope global-scope)
    set-autocomplete-scope! eval-scope

    'set-symbols eval-scope
        module-dir = cwd
        module-path = (cwd .. "/<console>.sc")
        module-name = "<console>"
        main-module? = true
        exit =
            typedef (do "Enter 'exit;' or Ctrl+D to exit")
                inline __typecall () (if true (exit 0))

    loop (preload cmdlist counter = "" "" 0)
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
            repeat preload cmdlist counter

        spice count-folds (x key values...)
            let x = (x as i32)
            let tmp = (Symbol "#result...")
            if (key != tmp) (x + 1)
            else x

        spice append-to-scope (scope key docstr vals...)
            let tmp = (Symbol "#result...")
            if (key != tmp)
                if (('argcount vals...) != 1)
                    let block = (sc_expression_new)
                    let outargs = `(sc_argument_list_new)
                    sc_expression_append block outargs
                    for arg in ('args vals...)
                        sc_expression_append block
                            if (('typeof arg) == Value)
                                `(sc_argument_list_append outargs ``arg)
                            else
                                `(sc_argument_list_append outargs `arg)
                    sc_expression_append block
                        `('set-symbol scope key outargs)
                    sc_expression_append block
                        `('set-docstring scope key docstr)
                    sc_expression_append block scope
                    return block
                else
                    return
                        spice-quote
                            'set-symbol scope key vals...
                            'set-docstring scope key docstr
                            scope
            scope

        spice print-bound-names (key vals...)
            let outargs = (sc_argument_list_new)
            for arg in ('args vals...)
                sc_argument_list_append outargs `(repr arg)
            spice-quote
                print key [(default-styler style-operator "=")] outargs

        spice handle-retargs (inserts counter eval-scope vals...)
            let inserts = (inserts as i32)
            let counter = (counter as i32)
            let count = ('argcount vals...)
            if inserts
                let outargs = (sc_argument_list_new)
                if (count != 0)
                    for arg in ('args vals...)
                        sc_argument_list_append outargs `(repr arg)
                    return
                        spice-quote
                            print outargs
                            counter
            elseif (count != 0)
                let outargs = (sc_argument_list_new)
                let block = (sc_expression_new)
                let eval-scope = (eval-scope as Scope)
                fold (count = 0) for arg in ('args vals...)
                    let idstr = (make-idstr (counter + count))
                    sc_argument_list_append outargs `idstr
                    let idstr = (Symbol idstr)
                    sc_expression_append block
                        if (('typeof arg) == Value)
                            `('set-symbol eval-scope idstr ``arg)
                        else
                            `('set-symbol eval-scope idstr `arg)
                    count + 1
                sc_argument_list_append outargs (default-styler style-operator "=")
                for arg in ('args vals...)
                    sc_argument_list_append outargs `(repr arg)
                let counter = (counter + count)
                return
                    spice-quote
                        block
                        print outargs
                        counter
            `counter

        fn get-bound-name (eval-scope expr)
            if ((countof expr) == 1)
                let at = ('@ expr)
                if (('typeof at) == Symbol)
                    let at = (at as Symbol)
                    try
                        return at ('@ eval-scope at)
                    except (err)
            _ unnamed `()



        let counter =
            try
                let user-expr = (list-parse cmdlist)
                let expression-anchor = ('anchor user-expr)
                let user-expr = (user-expr as list)
                let bound-name bound-val = (get-bound-name eval-scope user-expr)
                if (bound-name != unnamed)
                    # just print the value
                    @@ spice-quote
                    fn expr ()
                        raises-compile-error;
                        print-bound-names bound-name bound-val
                    let f = (sc_compile (sc_typify_template expr 0 null) 0:u64)
                    let fptr = (f as (pointer (raises (function void) Error)))
                    fptr;
                    counter
                else
                    let tmp = (Symbol "#result...")
                    let list-expression =
                        qq
                            raises-compile-error;
                            [let] [tmp] =
                                [embed]
                                    unquote-splice user-expr
                            [fold-locals] [eval-scope] [append-to-scope]
                            [handle-retargs]
                                [fold-locals] 0 [count-folds]
                                \ [counter] [eval-scope] [tmp]
                    hide-traceback;
                    let expression = (sc_eval
                        expression-anchor list-expression (Scope eval-scope))
                    let f = (sc_compile expression 0:u64)
                    let fptr =
                        f as (pointer (raises (function i32) Error))
                    fptr;
            except (exc)
                'dump exc
                counter
        repeat "" "" counter

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
        'set-docstring scope unnamed ""
        'set-symbols scope
            script-launch-args =
                fn ()
                    return sourcepath argc argv
        do  #try
            hide-traceback;
            load-module "" sourcepath
                scope = scope
                main-module? = true
            ;
        #except (err)
            #print
                default-styler style-error "error:"
                'format err
            ;
        exit 0

raises-compile-error;
hide-traceback;
run-main;

return;