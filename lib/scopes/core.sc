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

# first we alias u64 to the integer type that can hold a pointer
let intptr = u64

# pointer comparison as a template function, because we'll compare pointers of many types
fn ptrcmp!= (t1 t2)
    icmp!= (ptrtoint t1 intptr) (ptrtoint t2 intptr)

fn ptrcmp== (t1 t2)
    icmp== (ptrtoint t1 intptr) (ptrtoint t2 intptr)

fn box-i32 (value)
    let val = (undef Any)
    let val = (insertvalue val (typeof value) 0)
    let val = (insertvalue val (sext value u64) 1)
    val

# turn a symbol-like value (storage type u64) to an Any
fn box-symbol (value)
    let val = (undef Any)
    let val = (insertvalue val (typeof value) 0)
    let val = (insertvalue val value 1)
    val

# turn a pointer value into an Any
fn box-pointer (value)
    let val = (undef Any)
    let val = (insertvalue val (typeof value) 0)
    let val = (insertvalue val (ptrtoint value u64) 1)
    val

# print an unboxing error given two types
fn unbox-error (haveT wantT)
    sc_anchor_error
        sc_string_join "can't unbox value of type "
            sc_string_join
                sc_any_repr (box-pointer haveT)
                sc_string_join " as value of type "
                    sc_any_repr (box-pointer wantT)

inline unbox-symbol (value T)
    let valueT = (extractvalue value 0)
    if (ptrcmp!= valueT T)
        unbox-error valueT T
    bitcast (extractvalue value 1) T

# turn an Any back into a pointer
inline unbox-pointer (value T)
    let valueT = (extractvalue value 0)
    if (ptrcmp!= valueT T)
        unbox-error valueT T
    inttoptr (extractvalue value 1) T

fn verify-argument-count (l mincount maxcount)
    let count = (sub (sc_label_argument_count l) 1)
    if (icmp>=s mincount 0)
        if (icmp<s count mincount)
            sc_anchor_error
                sc_string_join "at least "
                    sc_string_join (sc_any_repr (box-i32 mincount))
                        sc_string_join " argument(s) expected, got "
                            sc_any_repr (box-i32 count)
    if (icmp>=s maxcount 0)
        if (icmp>s count maxcount)
            sc_anchor_error
                sc_string_join "at most "
                    sc_string_join (sc_any_repr (box-i32 maxcount))
                        sc_string_join " argument(s) expected, got "
                            sc_any_repr (box-i32 count)
    return;

fn indirect-typeof (value)
    let T = (extractvalue value 0)
    if (ptrcmp!= T Parameter) T
    else
        sc_parameter_type (unbox-pointer value Parameter)

fn any-constant? (value)
    let T = (extractvalue value 0)
    ptrcmp!= T Parameter

fn any-none? (value)
    let T = (extractvalue value 0)
    ptrcmp== T Nothing

fn any-none ()
    let val = (nullof Any)
    let val = (insertvalue val Nothing 0)
    val

# we don't have typedef or anything, so we need to open a new compiler context
# to alias the pointer type that we need (pointer to types) so it's constant
syntax-extend
    let val = (box-pointer (sc_pointer_type type pointer-flag-non-writable unnamed))
    sc_scope_set_symbol syntax-scope 'type-array val
    sc_scope_set_symbol syntax-scope 'LabelMacroFunctionType
        box-pointer (sc_type_storage LabelMacro)
    syntax-scope

fn Label-return (l)
    let k cont = (sc_label_argument l 0)
    sc_label_set_enter l (box-symbol _)
    sc_label_clear_arguments l
    sc_label_append_argument l unnamed cont

syntax-extend
    fn typify (l)
        verify-argument-count l 1 -1
        let count = (sc_label_argument_count l)
        let k src_label = (sc_label_argument l 1)
        let src_label = (unbox-pointer src_label Closure)
        let pcount = (sub count 2)
        let types = (alloca-array type pcount)
        let loop (i j) = 2 0
        if (icmp<s i count)
            let k ty = (sc_label_argument l i)
            store (unbox-pointer ty type) (getelementptr types j)
            loop (add i 1) (add j 1)
        let func =
            sc_typify src_label pcount (bitcast types type-array)
        Label-return l
        sc_label_append_argument l unnamed (box-pointer func)
        l

    do
        let types = (alloca-array type 1:usize)
        store Label (getelementptr types 0)
        let types = (bitcast types type-array)
        let result = (sc_compile (sc_typify typify 1 types) 0:u64)
        let result-type = (extractvalue result 0)
        if (ptrcmp!= result-type LabelMacroFunctionType)
            sc_anchor_error "label macro must return label"
        let result =
            insertvalue result LabelMacro 0
        sc_scope_set_symbol syntax-scope 'typify result

    syntax-scope

# take closure l, typify and compile it and return an Any of LabelMacro type
inline label-macro (l)
    let l = (typify l Label)
    # todo: verify return function signature is correct
    bitcast (unconst l) LabelMacro

inline box-label-macro (l)
    box-pointer (label-macro l)

syntax-extend
    # method call syntax
    sc_type_set_symbol Symbol '__call
        box-label-macro
            fn "symbol-call" (l)
                verify-argument-count l 2 -1
                let k sym = (sc_label_argument l 1)
                let k self = (sc_label_argument l 2)
                let T = (indirect-typeof self)
                let f ok = (sc_type_at T (unbox-symbol sym Symbol))
                if ok
                    sc_label_remove_argument l 1
                    sc_label_set_enter l f
                else
                    sc_anchor_error
                        sc_string_join "no method named "
                            sc_string_join (sc_any_repr sym)
                                sc_string_join " in value of type "
                                    sc_any_repr (box-pointer T)
                l

    inline gen-key-any-set (selftype fset)
        box-label-macro
            fn "set-symbols" (l)
                verify-argument-count l 1 -1
                let k cont = (sc_label_argument l 0)
                let k self = (sc_label_argument l 1)
                let T = (unbox-pointer self selftype)
                let count = (sc_label_argument_count l)
                let numentries = (sub count 2)
                let keys = (alloca-array Symbol numentries)
                let values = (alloca-array Any numentries)
                let loop (i j) = 2 0
                if (icmp<s i count)
                    let k v = (sc_label_argument l i)
                    store k (getelementptr keys j)
                    store v (getelementptr values j)
                    loop (add i 1) (add j 1)
                Label-return l
                let loop (i active-l) = 0 l
                if (icmp<s i numentries)
                    let k = (load (getelementptr keys i))
                    let v = (load (getelementptr values i))
                    if (icmp== k unnamed)
                        sc_anchor_error
                            sc_string_join "cannot set symbol from argument "
                                sc_string_join (sc_any_repr (box-i32 i))
                                    " because it has no key"
                    loop (add i 1)
                        if (any-constant? v)
                            fset T k v
                            active-l
                        else
                            let lcont nextl complete =
                                do
                                    # last entry
                                    if (icmp== (add i 1) numentries)
                                        _ cont (nullof Label) false
                                    else
                                        let nextl = (sc_label_new_cont)
                                        sc_label_append_argument nextl unnamed (any-none)
                                        _ (box-pointer nextl) nextl true
                            sc_label_set_enter active-l (Any-wrap fset)
                            sc_label_clear_arguments active-l
                            sc_label_append_argument active-l unnamed lcont
                            sc_label_append_argument active-l unnamed self
                            sc_label_append_argument active-l unnamed (box-symbol k)
                            if (ptrcmp!= (indirect-typeof v) Any)
                                sc_anchor_error
                                    sc_string_join "cannot set symbol "
                                        sc_string_join (sc_any_repr (box-symbol k))
                                            sc_string_join " because variable is not of type "
                                                sc_any_repr (box-pointer Any)
                            sc_label_append_argument active-l unnamed v
                            if complete
                                sc_label_set_complete active-l
                            nextl
                l

    # quick assignment of type attributes
    sc_type_set_symbol type 'set-symbols (gen-key-any-set type sc_type_set_symbol)
    sc_type_set_symbol Scope 'set-symbols (gen-key-any-set Scope sc_scope_set_symbol)

    syntax-scope

'set-symbols Scope
    set-symbol = sc_scope_set_symbol

# label accessors
'set-symbols Label
    verify-argument-count = verify-argument-count
    argument = sc_label_argument
    argument-count = sc_label_argument_count
    remove-argument = sc_label_remove_argument
    append-argument = sc_label_append_argument
    insert-argument = sc_label_insert_argument
    set-argument = sc_label_set_argument
    enter = sc_label_get_enter
    set-enter = sc_label_set_enter
    return = Label-return

'set-symbols type
    bitcount = sc_type_bitcountof
    signed? = sc_integer_type_is_signed

syntax-extend
    # integer casting

    'set-symbols integer
        __imply =
            box-label-macro
                fn "integer-imply" (l)
                    'verify-argument-count l 3 3
                    let k fallback = ('argument l 1)
                    let k value = ('argument l 2)
                    let k T = ('argument l 3)
                    let vT = (indirect-typeof value)
                    let T = (unbox-pointer T type)
                    let T =
                        if (ptrcmp== T usize) (sc_type_storage T)
                        else T
                    label select-op (op)
                        'remove-argument l 1
                        'set-enter l (box-symbol op)
                        return l
                    if (icmp== (sc_type_kind T) type-kind-integer)
                        let valw = ('bitcount vT)
                        let destw = ('bitcount T)
                        # must have same signed bit
                        if (icmp== ('signed? vT) ('signed? T))
                            if (icmp== destw valw)
                                select-op bitcast
                            elseif (icmp>s destw valw)
                                if ('signed? vT)
                                    select-op sext
                                else
                                    select-op zext
                    'set-enter l fallback
                    return l
        __as =
            box-label-macro
                fn "integer-as" (l)
                    'verify-argument-count l 3 3
                    let k fallback = ('argument l 1)
                    let k value = ('argument l 2)
                    let k T = ('argument l 3)
                    let vT = (indirect-typeof value)
                    let T = (unbox-pointer T type)
                    let T =
                        if (ptrcmp== T usize) (sc_type_storage T)
                        else T
                    label select-op (op)
                        'remove-argument l 1
                        'set-enter l (box-symbol op)
                        return l
                    if (icmp== (sc_type_kind T) type-kind-integer)
                        let valw = ('bitcount vT)
                        let destw = ('bitcount T)
                        if (icmp== destw valw)
                            select-op bitcast
                        elseif (icmp>s destw valw)
                            if ('signed? vT)
                                select-op sext
                            else
                                select-op zext
                        else
                            select-op itrunc
                    elseif (icmp== (sc_type_kind T) type-kind-real)
                        if ('signed? vT)
                            select-op sitofp
                        else
                            select-op uitofp
                    'set-enter l fallback
                    return l

    inline gen-cast-error (intro-string)
        label-macro
            fn "cast-error" (l)
                'verify-argument-count l 3 3
                let k value = ('argument l 2)
                let k T = ('argument l 3)
                let vT = (indirect-typeof value)
                let T = (unbox-pointer T type)
                sc_anchor_error
                    sc_string_join intro-string
                        sc_string_join
                            sc_any_repr (box-pointer vT)
                            sc_string_join " to type "
                                sc_any_repr (box-pointer T)
                l

    inline gen-cast-dispatch (symbol error-msg)
        box-label-macro
            fn "cast-dispatch" (l)
                'verify-argument-count l 3 3
                let k fallback = ('argument l 1)
                let k value = ('argument l 2)
                let k T = ('argument l 3)
                let vT = (indirect-typeof value)
                let T = (unbox-pointer T type)
                let fallback =
                    if (any-none? fallback)
                        let error-func =
                            gen-cast-error error-msg
                        let f = (box-pointer error-func)
                        'set-argument l 1 unnamed f
                        f
                    else fallback
                if (ptrcmp!= vT T)
                    let f ok = (sc_type_at vT symbol)
                    if ok
                        'set-enter l f
                    else
                        'set-enter l fallback
                else
                    'return l
                    'append-argument l unnamed value
                l

    'set-symbols syntax-scope
        forward-imply =
            gen-cast-dispatch '__imply "can't implicitly cast value of type "
        __forward-as =
            gen-cast-dispatch '__as "can't cast value of type "

    syntax-scope

inline forward-as (fallback value destT)
    forward-imply
        inline ()
            __forward-as fallback value destT
        \ value destT

inline imply (value destT)
    forward-imply none value destT

inline as (value destT)
    forward-as none value destT

let k =
    as 5 f32
dump k

sc_exit 0
