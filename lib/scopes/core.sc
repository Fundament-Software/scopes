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

fn box-integer (value)
    let val = (undef Any)
    let val = (insertvalue val (typeof value) 0)
    let val = (insertvalue val (zext value u64) 1)
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
fn unbox-verify (haveT wantT)
    if (ptrcmp!= haveT wantT)
        sc_anchor_error
            sc_string_join "can't unbox value of type "
                sc_string_join
                    sc_any_repr (box-pointer haveT)
                    sc_string_join " as value of type "
                        sc_any_repr (box-pointer wantT)

inline unbox-symbol (value T)
    unbox-verify (extractvalue value 0) T
    bitcast (extractvalue value 1) T

# turn an Any back into a pointer
inline unbox-pointer (value T)
    unbox-verify (extractvalue value 0) T
    inttoptr (extractvalue value 1) T

fn verify-argument-count (l mincount maxcount)
    let count = (sub (sc_label_argument_count l) 1)
    if (icmp>=s mincount 0)
        if (icmp<s count mincount)
            sc_anchor_error
                sc_string_join "at least "
                    sc_string_join (sc_any_repr (box-integer mincount))
                        sc_string_join " argument(s) expected, got "
                            sc_any_repr (box-integer count)
    if (icmp>=s maxcount 0)
        if (icmp>s count maxcount)
            sc_anchor_error
                sc_string_join "at most "
                    sc_string_join (sc_any_repr (box-integer maxcount))
                        sc_string_join " argument(s) expected, got "
                            sc_any_repr (box-integer count)
    return;

fn Any-indirect-typeof (value)
    let T = (extractvalue value 0)
    if (ptrcmp!= T Parameter) T
    else
        sc_parameter_type (unbox-pointer value Parameter)

fn Any-typeof (value)
    extractvalue value 0

fn Any-constant? (value)
    let T = (extractvalue value 0)
    ptrcmp!= T Parameter

fn Any-none? (value)
    let T = (extractvalue value 0)
    ptrcmp== T Nothing

fn Any-none ()
    let val = (nullof Any)
    let val = (insertvalue val Nothing 0)
    val

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
    let types = (alloca-array type 3)
    do
        store Any (getelementptr types 0)
        store bool (getelementptr types 1)
        let rtype = (sc_tuple_type 2 (bitcast types type-array))
        store type (getelementptr types 0)
        store type (getelementptr types 1)
        let ftype = (sc_function_type rtype 2 (bitcast types type-array))
        sc_scope_set_symbol syntax-scope 'DispatchCastFunctionType (box-pointer ftype)
        sc_scope_set_symbol syntax-scope 'DispatchCastFunctionPointerType
            box-pointer (sc_pointer_type ftype pointer-flag-non-writable unnamed)

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

let function->LabelMacro =
    typify
        fn "function->LabelMacro" (f)
            bitcast f LabelMacro
        LabelMacroFunctionType

# take closure l, typify and compile it and return an Any of LabelMacro type
inline label-macro (l)
    function->LabelMacro (unconst (typify l Label))

inline box-label-macro (l)
    box-pointer (label-macro l)

syntax-extend
    # tuple type constructor
    sc_type_set_symbol tuple '__typecall
        box-label-macro
            fn "tuple" (l)
                verify-argument-count l 1 -1
                let pcount = (sub (sc_label_argument_count l) 2)
                let types = (alloca-array type pcount)
                let loop (i j) = 0 2
                if (icmp<s i pcount)
                    let k arg = (sc_label_argument l j)
                    let T = (unbox-pointer arg type)
                    store T (getelementptr types i)
                    loop (add i 1) (add j 1)
                let ttype = (sc_tuple_type pcount (bitcast types type-array))
                Label-return l
                sc_label_append_argument l unnamed (box-pointer ttype)
                l

    # function pointer type constructor
    sc_type_set_symbol function '__typecall
        box-label-macro
            fn "function" (l)
                verify-argument-count l 2 -1
                let k rtype = (sc_label_argument l 2)
                let rtype = (unbox-pointer rtype type)
                let pcount = (sub (sc_label_argument_count l) 3)
                let types = (alloca-array type pcount)
                let loop (i j) = 0 3
                if (icmp<s i pcount)
                    let k arg = (sc_label_argument l j)
                    let T = (unbox-pointer arg type)
                    store T (getelementptr types i)
                    loop (add i 1) (add j 1)
                let ftype = (sc_function_type rtype pcount (bitcast types type-array))
                Label-return l
                sc_label_append_argument l unnamed (box-pointer ftype)
                l

    # method call syntax
    sc_type_set_symbol Symbol '__call
        box-label-macro
            fn "symbol-call" (l)
                verify-argument-count l 2 -1
                let k sym = (sc_label_argument l 1)
                let k self = (sc_label_argument l 2)
                let T = (Any-indirect-typeof self)
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
                let self-constant? = (Any-constant? self)
                let T =
                    if self-constant?
                        unbox-pointer self selftype
                    else
                        nullof selftype
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
                                sc_string_join (sc_any_repr (box-integer i))
                                    " because it has no key"
                    loop (add i 1)
                        if (band self-constant? (Any-constant? v))
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
                                        sc_label_append_argument nextl unnamed (Any-none)
                                        _ (box-pointer nextl) nextl true
                            sc_label_set_enter active-l (Any-wrap fset)
                            sc_label_clear_arguments active-l
                            sc_label_append_argument active-l unnamed lcont
                            sc_label_append_argument active-l unnamed self
                            sc_label_append_argument active-l unnamed (box-symbol k)
                            if (ptrcmp!= (Any-indirect-typeof v) Any)
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

    sc_type_set_symbol type 'pointer
        box-label-macro
            fn "type-pointer" (l)
                verify-argument-count l 1 1
                let k self = (sc_label_argument l 1)
                let T = (unbox-pointer self type)
                Label-return l
                sc_label_append_argument l unnamed
                    box-pointer
                        sc_pointer_type T pointer-flag-non-writable unnamed
                l

    # typecall

    sc_type_set_symbol type '__call
        box-label-macro
            fn "type-call" (l)
                let k self = (sc_label_argument l 1)
                let T = (unbox-pointer self type)
                let f ok = (sc_type_at T '__typecall)
                if ok
                    sc_label_set_enter l f
                    return l
                sc_anchor_error
                    sc_string_join "no type constructor available for type "
                        sc_any_repr self
                l

    # closure constructor
    sc_type_set_symbol Closure '__typecall
        box-pointer
            inline (cls label frame)
                sc_closure_new label frame

    # symbol constructor
    sc_type_set_symbol Symbol '__typecall
        box-pointer
            inline (cls string)
                sc_symbol_new string

    syntax-scope

'set-symbols Any
    constant? = (typify Any-constant? Any)
    none? = (typify Any-none? Any)
    __repr = sc_any_repr
    indirect-typeof = (typify Any-indirect-typeof Any)
    typeof = (typify Any-typeof Any)

let Syntax-wrap = sc_syntax_wrap

'set-symbols Scope
    set-symbol = sc_scope_set_symbol
    @ = sc_scope_at

'set-symbols string
    join = sc_string_join

let cons = sc_list_cons

'set-symbols list
    join = sc_list_join
    decons =
        typify
            fn (l)
                return
                    load (getelementptr l 0 0)
                    load (getelementptr l 0 1)
            list

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
    dump = sc_label_dump
    function-type = sc_label_function_type
    set-rawcall = sc_label_set_rawcall
    frame = sc_label_frame
    return = (typify Label-return Label)

'set-symbols type
    bitcount = sc_type_bitcountof
    signed? = sc_integer_type_is_signed
    element@ = sc_type_element_at
    element-count = sc_type_countof
    storage = sc_type_storage
    kind = sc_type_kind
    @ = sc_type_at

inline imply

syntax-extend
    # syntax macro type
    let SyntaxMacro = (sc_typename_type "SyntaxMacro")
    let SyntaxMacroFunctionType =
        'pointer (function (tuple list Scope) list list Scope)
    sc_typename_type_set_storage SyntaxMacro SyntaxMacroFunctionType
    sc_scope_set_symbol syntax-scope 'SyntaxMacro (box-pointer SyntaxMacro)
    sc_scope_set_symbol syntax-scope 'SyntaxMacroFunctionType (box-pointer SyntaxMacroFunctionType)

    # any extraction

    inline unbox-integer (value T)
        unbox-verify (extractvalue value 0) T
        itrunc (extractvalue value 1) T

    inline unbox-u32 (value T)
        unbox-verify (extractvalue value 0) T
        bitcast (itrunc (extractvalue value 1) u32) T

    inline unbox-bitcast (value T)
        unbox-verify (extractvalue value 0) T
        bitcast (extractvalue value 1) T

    inline unbox-hidden-pointer (value T)
        unbox-verify (extractvalue value 0) T
        load (inttoptr (extractvalue value 1) ('pointer T))

    fn any-imply (vT T)
        let storageT = ('storage T)
        let kind = ('kind storageT)
        if (icmp== kind type-kind-pointer)
            return (box-pointer unbox-pointer) true
        elseif (icmp== kind type-kind-integer)
            return (box-pointer unbox-integer) true
        elseif (icmp== kind type-kind-real)
            if (ptrcmp== storageT f32)
                return (box-pointer unbox-u32) true
            elseif (ptrcmp== storageT f64)
                return (box-pointer unbox-bitcast) true
        elseif (bor (icmp== kind type-kind-tuple) (icmp== kind type-kind-array))
            return (box-pointer unbox-hidden-pointer) true
        sc_write ('__repr (box-pointer storageT))
        return (Any-none) false

    'set-symbols Any
        __imply =
            box-pointer (unconst (typify any-imply type type))
        __typecall =
            box-label-macro
                fn (l)
                    'verify-argument-count l 2 2
                    let k arg = ('argument l 2)
                    let T = ('indirect-typeof arg)
                    if (ptrcmp== T Any)
                        'return l
                        'append-argument l unnamed arg
                    else
                        'remove-argument l 1
                        if ('constant? arg)
                            'set-enter l (Any-wrap Any-wrap)
                        else
                            let storageT = ('storage T)
                            let kind = ('kind storageT)
                            if (icmp== kind type-kind-pointer)
                                'set-enter l (box-pointer box-pointer)
                            elseif (icmp== kind type-kind-integer)
                                'set-enter l (box-pointer box-integer)
                            elseif (icmp== kind type-kind-extern)
                                'set-enter l (box-pointer box-symbol)
                            #elseif (bor (icmp== kind type-kind-tuple) (icmp== kind type-kind-array))
                            #elseif (icmp== kind type-kind-vector)
                            #elseif (icmp== kind type-kind-real)
                            else
                                sc_anchor_error
                                    'join "can't box value of type "
                                        '__repr (box-pointer T)
                    l

    # list constructor

    'set-symbols list
        __typecall =
            box-label-macro
                fn (l)
                    let k self = ('argument l 1)
                    let count = ('argument-count l)
                    let loop (i tail) = count '(quote ())
                    if (icmp>s i 2)
                        let i = (sub i 1)
                        let k arg = ('argument l i)
                        loop i
                            cons (Any-wrap cons)
                                cons
                                    box-pointer
                                        cons (Any-wrap Any)
                                            cons arg '()
                                    cons (box-pointer tail) '()
                    let tail = (cons (box-pointer tail) '())
                    let genl = (sc_eval_inline tail (nullof Scope))
                    'return l
                    'set-enter l
                        box-pointer
                            Closure genl ('frame l)
                    l

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
                    return (box-symbol bitcast) true
                elseif (icmp>s destw valw)
                    if ('signed? vT)
                        return (box-symbol sext) true
                    else
                        return (box-symbol zext) true
        return (Any-none) false

    fn integer-as (vT T)
        let T =
            if (ptrcmp== T usize) ('storage T)
            else T
        if (icmp== ('kind T) type-kind-integer)
            let valw = ('bitcount vT)
            let destw = ('bitcount T)
            if (icmp== destw valw)
                return (box-symbol bitcast) true
            elseif (icmp>s destw valw)
                if ('signed? vT)
                    return (box-symbol sext) true
                else
                    return (box-symbol zext) true
            else
                return (box-symbol itrunc) true
        elseif (icmp== ('kind T) type-kind-real)
            if ('signed? vT)
                return (box-symbol sitofp) true
            else
                return (box-symbol uitofp) true
        return (Any-none) false

    'set-symbols integer
        __imply =
            box-pointer (unconst (typify integer-imply type type))
        __as =
            box-pointer (unconst (typify integer-as type type))

    inline gen-cast-error (intro-string)
        label-macro
            fn "cast-error" (l)
                'verify-argument-count l 3 3
                let k value = ('argument l 2)
                let k T = ('argument l 3)
                let vT = ('indirect-typeof value)
                let T = (unbox-pointer T type)
                sc_anchor_error
                    sc_string_join intro-string
                        sc_string_join
                            '__repr (box-pointer vT)
                            sc_string_join " to type "
                                '__repr (box-pointer T)
                l

    fn get-cast-dispatcher (symbol vT T)
        let anyf ok = ('@ vT symbol)
        if ok
            let f = (unbox-pointer anyf DispatchCastFunctionPointerType)
            return (f vT T)
        return (Any-none) false

    fn implyfn (vT T)
        get-cast-dispatcher '__imply vT T
    fn asfn (vT T)
        get-cast-dispatcher '__as vT T

    'set-symbols syntax-scope
        implyfn = (typify implyfn type type)
        asfn = (typify asfn type type)

    let try-imply =
        box-label-macro
            fn "imply-dispatch" (l)
                'verify-argument-count l 3 3
                let k fallback = ('argument l 1)
                let k value = ('argument l 2)
                let k T = ('argument l 3)
                let vT = ('indirect-typeof value)
                let T = (unbox-pointer T type)
                let fallback =
                    if ('none? fallback)
                        let error-func =
                            gen-cast-error "can't implicitly cast value of type "
                        let f = (box-pointer error-func)
                        'set-argument l 1 unnamed f
                        f
                    else fallback
                if (ptrcmp!= vT T)
                    let f ok = (implyfn vT T)
                    if ok
                        'remove-argument l 1
                        'set-enter l f
                        return l
                    'set-enter l fallback
                else
                    'return l
                    'append-argument l unnamed value
                l

    'set-symbols syntax-scope
        try-imply = try-imply
        try-as =
            box-label-macro
                fn "as-dispatch" (l)
                    'verify-argument-count l 3 3
                    let k fallback = ('argument l 1)
                    let k value = ('argument l 2)
                    let k T = ('argument l 3)
                    let vT = ('indirect-typeof value)
                    let T = (unbox-pointer T type)
                    let fallback =
                        if ('none? fallback)
                            let error-func =
                                gen-cast-error "can't cast value of type "
                            let f = (box-pointer error-func)
                            'set-argument l 1 unnamed f
                            f
                        else fallback
                    if (ptrcmp!= vT T)
                        let f ok =
                            do
                                # try implicit cast first
                                let f ok = (implyfn vT T)
                                if ok (_ f ok)
                                else
                                    # then try explicit cast
                                    asfn vT T
                        if ok
                            'remove-argument l 1
                            'set-enter l f
                            return l
                        'set-enter l fallback
                    else
                        'return l
                        'append-argument l unnamed value
                    l

    #fn get-op2-dispatcher (symbol lhsT rhsT)
        let anyf ok = ('@ lhsT symbol)
        if ok
            let f = (unbox-pointer anyf DispatchCastFunctionPointerType)
            return (f vT T)
        return (Any-none) false

    #inline make-op2-dispatch (symbol)
        box-label-macro
            fn (l)
                'verify-argument-count l 3 3
                let k fallback = ('argument l 1)
                let k lhs = ('argument l 2)
                let k rhs = ('argument l 3)
                let lhsT = ('indirect-typeof lhs)
                let rhsT = ('indirect-typeof rhs)
                let fallback =
                    if ('none? fallback)
                        let error-func =
                            gen-cast-error "can't implicitly cast value of type "
                        let f = (box-pointer error-func)
                        'set-argument l 1 unnamed f
                        f
                    else fallback
                if (ptrcmp!= vT T)
                    let f ok = (implyfn vT T)
                    if ok
                        'remove-argument l 1
                        'set-enter l f
                        return l
                    'set-enter l fallback
                else
                    'return l
                    'append-argument l unnamed value
                l

    let Syntax-anchor =
        typify
            fn "Syntax-anchor" (sx)
                extractvalue (load sx) 0
            Syntax
    let Syntax-datum =
        typify
            fn "Syntax-datum" (sx)
                extractvalue (load sx) 1
            Syntax

    inline Syntax-unbox (self destT)
        imply (Syntax-datum self) destT

    'set-symbols Syntax
        anchor = Syntax-anchor
        datum = Syntax-datum
        __imply =
            box-pointer
                unconst
                    typify
                        fn "syntax-imply" (vT T)
                            if (ptrcmp== T Any)
                                return (box-pointer Syntax-datum) true
                            else
                                return (box-pointer Syntax-unbox) true
                        \ type type

    # support for calling macro functions directly
    'set-symbols SyntaxMacro
        __call =
            box-pointer
                inline (self at next scope)
                    (bitcast self SyntaxMacroFunctionType) at next scope

    syntax-scope

inline imply (value destT)
    try-imply none value destT

inline as (value destT)
    try-as none value destT

let function->SyntaxMacro =
    typify
        fn "function->SyntaxMacro" (f)
            bitcast f SyntaxMacro
        SyntaxMacroFunctionType

inline syntax-block-scope-macro (l)
    function->SyntaxMacro (unconst (typify l list list Scope))

syntax-extend



    # install basic list hook for this scope
    # is called for every list the expander would otherwise consider a call
    fn list-handler (topexpr env)
        let topexpr-at topexpr-next = ('decons topexpr)
        let sxexpr = (as topexpr-at Syntax)
        let expr expr-anchor = ('datum sxexpr) ('anchor sxexpr)
        if (ptrcmp!= ('typeof expr) list)
            return topexpr env
        let expr = (as expr list)
        let expr-at expr-next = ('decons expr)
        let head-key = ('datum (as expr-at Syntax))
        let head =
            if (ptrcmp== ('typeof head-key) Symbol)
                let head success = ('@ env (as head-key Symbol))
                if success head
                else head-key
            else head-key
        let head =
            if (ptrcmp== ('typeof head) type)
                let attr ok = ('@ (as head type) '__macro)
                if ok attr
                else head
            else head
        if (ptrcmp== ('typeof head) SyntaxMacro)
            let head = (as head SyntaxMacro)
            let expr env = (head expr topexpr-next env)
            let expr = (Syntax-wrap expr-anchor (Any expr) false)
            return (as (as expr Syntax) list) env
        #elseif (has-infix-ops? env expr)
            let at next = (decons expr)
            let expr =
                parse-infix-expr env at next (unconst 0)
            let next = (list-next topexpr)
            let expr = (Syntax-wrap expr-anchor expr false)
            return (list-cons expr next) env
        else
            return topexpr env

    sc_scope_set_symbol syntax-scope (Symbol "#list")
        Any (unconst (typify list-handler list Scope))

    'set-symbol syntax-scope 'test
        Any
            syntax-block-scope-macro
                fn (expr next scope)
                    sc_write "hello\n"
                    return next scope

    syntax-scope

#
    let k =
        as 5 u64
    dump k

    let test =
        typify
            fn (a b)
                add a b
            \ i32 i32

#dump (imply (box-integer 5) i32)

#do
    trycall
        inline ()
            dump "failed!"
        \ test 1 2


sc_exit 0
