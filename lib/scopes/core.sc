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

let set-anchor! = sc_set_active_anchor
let __anchor-error! = sc_anchor_error

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
        __anchor-error!
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
            __anchor-error!
                sc_string_join "at least "
                    sc_string_join (sc_any_repr (box-integer mincount))
                        sc_string_join " argument(s) expected, got "
                            sc_any_repr (box-integer count)
    if (icmp>=s maxcount 0)
        if (icmp>s count maxcount)
            __anchor-error!
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

fn none? (value)
    ptrcmp== (typeof value) Nothing

syntax-extend
    let val = (box-pointer (sc_pointer_type type pointer-flag-non-writable unnamed))
    sc_scope_set_symbol syntax-scope 'type-array val
    sc_scope_set_symbol syntax-scope 'LabelMacroFunctionType
        box-pointer (sc_type_storage LabelMacro)
    syntax-scope

fn Label-return (l)
    let k cont = (sc_label_argument l 0)
    sc_label_set_enter l (Any-wrap _)
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
            __anchor-error! "label macro must return label"
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
                    __anchor-error!
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
                        __anchor-error!
                            sc_string_join "cannot set symbol from argument "
                                sc_string_join (sc_any_repr (box-integer i))
                                    " because it has no key"
                    loop (add i 1)
                        if (band self-constant? (Any-constant? v))
                            fset T k v
                            active-l
                        else
                            let nextl = (sc_label_new_cont)
                            sc_label_set_enter nextl (Any-wrap _)
                            sc_label_append_argument nextl unnamed cont
                            sc_label_set_enter active-l (Any-wrap fset)
                            sc_label_clear_arguments active-l
                            sc_label_append_argument active-l unnamed (box-pointer nextl)
                            sc_label_append_argument active-l unnamed self
                            sc_label_append_argument active-l unnamed (box-symbol k)
                            if (ptrcmp!= (Any-indirect-typeof v) Any)
                                __anchor-error!
                                    sc_string_join "cannot set symbol "
                                        sc_string_join (sc_any_repr (box-symbol k))
                                            sc_string_join " because variable is not of type "
                                                sc_any_repr (box-pointer Any)
                            sc_label_append_argument active-l unnamed v
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
                __anchor-error!
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
    __countof =
        fn (self)
            if (icmp== (ptrtoint self intptr) 0:u64) 0:usize
            else
                load (getelementptr self 0 2)
    join = sc_list_join
    @ =
        typify
            fn (self)
                let at next = (sc_list_decons self)
                return at
            list
    next =
        typify
            fn (self)
                let at next = (sc_list_decons self)
                return next
            list
    decons = sc_list_decons

# label accessors
'set-symbols Label
    verify-argument-count = verify-argument-count
    argument = sc_label_argument
    argument-count = sc_label_argument_count
    remove-argument = sc_label_remove_argument
    append-argument = sc_label_append_argument
    insert-argument = sc_label_insert_argument
    set-argument = sc_label_set_argument
    clear-arguments = sc_label_clear_arguments
    enter = sc_label_get_enter
    set-enter = sc_label_set_enter
    dump = sc_label_dump
    function-type = sc_label_function_type
    set-rawcall = sc_label_set_rawcall
    frame = sc_label_frame
    append-parameter = sc_label_append_parameter
    return = (typify Label-return Label)

'set-symbols type
    bitcount = sc_type_bitcountof
    signed? = sc_integer_type_is_signed
    element@ = sc_type_element_at
    element-count = sc_type_countof
    storage = sc_type_storage
    kind = sc_type_kind
    @ = sc_type_at
    opaque? = sc_type_is_opaque

inline imply

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
    sc_typename_type_set_super Any tuple

    let opaquepointer = (sc_typename_type "opaquepointer")
    sc_typename_type_set_super string opaquepointer
    sc_typename_type_set_super type opaquepointer

    sc_typename_type_set_super usize integer

    # syntax macro type
    let SyntaxMacro = (sc_typename_type "SyntaxMacro")
    let SyntaxMacroFunctionType =
        'pointer (function (tuple list Scope) list list Scope)
    sc_typename_type_set_storage SyntaxMacro SyntaxMacroFunctionType

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
        return (Any-wrap none) false

    'set-symbols Any
        __imply =
            box-pointer (unconst (typify any-imply type type))
        __typecall =
            box-label-macro
                fn (l)
                    'verify-argument-count l 2 2
                    inline make-none ()
                        Any-wrap none
                    let k arg = ('argument l 2)
                    let T = ('indirect-typeof arg)
                    if (ptrcmp== T Any)
                        'return l
                        'append-argument l unnamed arg
                    else
                        'remove-argument l 1
                        if ('constant? arg)
                            'set-enter l (Any-wrap Any-wrap)
                        elseif (ptrcmp== T Nothing)
                            'set-enter l (Any-wrap make-none)
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
                                __anchor-error!
                                    'join "can't box value of type "
                                        '__repr (box-pointer T)
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
        return (Any-wrap none) false

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
        return (Any-wrap none) false

    inline box-binary-op-dispatch (f)
        box-pointer (unconst (typify f type type))
    inline single-binary-op-dispatch (destf)
        fn (lhsT rhsT)
            if (ptrcmp== lhsT rhsT)
                return (Any-wrap destf) true
            return (Any-wrap none) false

    inline box-cast-dispatch (f)
        box-pointer (unconst (typify f type type))

    inline gen-cast-error (intro-string)
        label-macro
            fn "cast-error" (l)
                'verify-argument-count l 3 3
                let k value = ('argument l 2)
                let k T = ('argument l 3)
                let vT = ('indirect-typeof value)
                let T = (unbox-pointer T type)
                __anchor-error!
                    sc_string_join intro-string
                        sc_string_join
                            '__repr (box-pointer vT)
                            sc_string_join " to type "
                                '__repr (box-pointer T)
                l

    let DispatchCastFunctionType =
        'pointer (function (tuple Any bool) type type)

    fn get-cast-dispatcher (symbol vT T)
        let anyf ok = ('@ vT symbol)
        if ok
            let f = (unbox-pointer anyf DispatchCastFunctionType)
            return (f vT T)
        return (Any-wrap none) false

    fn implyfn (vT T)
        get-cast-dispatcher '__imply vT T
    fn asfn (vT T)
        get-cast-dispatcher '__as vT T

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

    let try-as =
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

    let UnaryOpFunctionType =
        'pointer (function (tuple Any bool) type)

    let BinaryOpFunctionType =
        'pointer (function (tuple Any bool) type type)

    fn get-binary-op-dispatcher (symbol lhsT rhsT)
        let anyf ok = ('@ lhsT symbol)
        if ok
            let f = (unbox-pointer anyf BinaryOpFunctionType)
            return (f lhsT rhsT)
        return (Any-wrap none) false

    fn binary-op-label-cast-then-macro (l f castf lhsT rhs)
        let k cont = ('argument l 0)
        # next label
        let lcont = (sc_label_new_cont)
        let param = (sc_parameter_new (sc_get_active_anchor) unnamed lhsT)
        'append-parameter lcont param
        # need to generate cast
        'clear-arguments l
        'set-enter l castf
        'append-argument l unnamed (box-pointer lcont)
        'append-argument l unnamed rhs
        'append-argument l unnamed (box-pointer lhsT)
        'set-enter lcont f
        'append-argument lcont unnamed cont
        return lcont param

    fn binary-op-label-macro (l symbol rsymbol friendly-op-name)
        'verify-argument-count l 2 2
        let lhsk lhs = ('argument l 1)
        let rhsk rhs = ('argument l 2)
        let lhsT = ('indirect-typeof lhs)
        let rhsT = ('indirect-typeof rhs)
        # try direct version first
        let f ok = (get-binary-op-dispatcher symbol lhsT rhsT)
        if ok
            'set-enter l f
            return l
        # if types are unequal, we can try other options
        if (ptrcmp!= lhsT rhsT)
            # try reverse version next
            let f ok = (get-binary-op-dispatcher rsymbol rhsT lhsT)
            if ok
                'set-argument l 1 rhsk rhs
                'set-argument l 2 lhsk lhs
                'set-enter l f
                return l
        # we give up
        __anchor-error!
            'join "can't "
                'join friendly-op-name
                    'join " values of types "
                        'join
                            '__repr (box-pointer lhsT)
                            'join " and "
                                '__repr (box-pointer rhsT)
        l

    # both types are typically the same
    fn sym-binary-op-label-macro (l symbol rsymbol friendly-op-name)
        'verify-argument-count l 2 2
        let lhsk lhs = ('argument l 1)
        let rhsk rhs = ('argument l 2)
        let lhsT = ('indirect-typeof lhs)
        let rhsT = ('indirect-typeof rhs)
        # try direct version first
        let f ok = (get-binary-op-dispatcher symbol lhsT rhsT)
        if ok
            'set-enter l f
            return l
        # if types are unequal, we can try other options
        if (ptrcmp!= lhsT rhsT)
            # try reverse version next
            let f ok = (get-binary-op-dispatcher rsymbol rhsT lhsT)
            if ok
                'set-argument l 1 rhsk rhs
                'set-argument l 2 lhsk lhs
                'set-enter l f
                return l
            # can the operation be performed on the lhs type?
            let f ok = (get-binary-op-dispatcher symbol lhsT lhsT)
            if ok
                # can we cast rhsT to lhsT?
                let castf ok = (implyfn rhsT lhsT)
                if ok
                    let lcont param =
                        binary-op-label-cast-then-macro l f castf lhsT rhs
                    'append-argument lcont lhsk lhs
                    'append-argument lcont rhsk (box-pointer param)
                    return l
            # can the operation be performed on the rhs type?
            let f ok = (get-binary-op-dispatcher symbol rhsT rhsT)
            if ok
                # can we cast lhsT to rhsT?
                let castf ok = (implyfn lhsT rhsT)
                if ok
                    let lcont param =
                        binary-op-label-cast-then-macro l f castf rhsT lhs
                    'append-argument lcont lhsk (box-pointer param)
                    'append-argument lcont rhsk rhs
                    return l
        # we give up
        __anchor-error!
            'join "can't "
                'join friendly-op-name
                    'join " values of types "
                        'join
                            '__repr (box-pointer lhsT)
                            'join " and "
                                '__repr (box-pointer rhsT)
        l

    # right hand has fixed type
    fn asym-binary-op-label-macro (l symbol rtype friendly-op-name)
        'verify-argument-count l 2 2
        let lhsk lhs = ('argument l 1)
        let rhsk rhs = ('argument l 2)
        let lhsT = ('indirect-typeof lhs)
        let rhsT = ('indirect-typeof rhs)
        let f ok = ('@ lhsT symbol)
        if ok
            if (ptrcmp== rhsT rtype)
                'set-enter l f
                return l
            # can we cast rhsT to rtype?
            let castf ok = (implyfn rhsT rtype)
            if ok
                let lcont param =
                    binary-op-label-cast-then-macro l f castf rtype rhs
                'append-argument lcont lhsk lhs
                'append-argument lcont rhsk (box-pointer param)
                return l
        # we give up
        __anchor-error!
            'join "can't "
                'join friendly-op-name
                    'join " values of types "
                        'join
                            '__repr (box-pointer lhsT)
                            'join " and "
                                '__repr (box-pointer rhsT)
        l

    fn unary-op-label-macro (l symbol friendly-op-name)
        'verify-argument-count l 1 1
        let lhsk lhs = ('argument l 1)
        let lhsT = ('indirect-typeof lhs)
        let f ok = ('@ lhsT symbol)
        if ok
            'set-enter l f
            return l
        __anchor-error!
            'join "can't "
                'join friendly-op-name
                    'join " value of type "
                        '__repr (box-pointer lhsT)
        l

    inline make-unary-op-dispatch (symbol friendly-op-name)
        box-label-macro (fn (l) (unary-op-label-macro l symbol friendly-op-name))

    inline make-binary-op-dispatch (symbol rsymbol friendly-op-name)
        box-label-macro (fn (l) (binary-op-label-macro l symbol rsymbol friendly-op-name))

    inline make-sym-binary-op-dispatch (symbol rsymbol friendly-op-name)
        box-label-macro (fn (l) (sym-binary-op-label-macro l symbol rsymbol friendly-op-name))

    inline make-asym-binary-op-dispatch (symbol rtype friendly-op-name)
        box-label-macro (fn (l) (asym-binary-op-label-macro l symbol rtype friendly-op-name))

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
            box-cast-dispatch
                fn "syntax-imply" (vT T)
                    if (ptrcmp== T Any)
                        return (box-pointer Syntax-datum) true
                    else
                        return (box-pointer Syntax-unbox) true

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
                        return (Any-wrap symbol-imply) true
                    return (Any-wrap none) true

    fn string@ (self i)
        let s = (bitcast (getelementptr self 0 1 0) ('pointer i8))
        load (getelementptr s i)

    'set-symbols string
        __.. = (box-binary-op-dispatch (single-binary-op-dispatch sc_string_join))
        __countof = sc_string_count
        __@ = string@
        __lslice = sc_string_lslice
        __rslice = sc_string_rslice

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
        __.. = (box-binary-op-dispatch (single-binary-op-dispatch sc_list_join))
        __repr =
            inline "list-repr" (self)
                '__repr (Any self)

    inline single-signed-binary-op-dispatch (sf uf)
        fn (lhsT rhsT)
            if (ptrcmp== lhsT rhsT)
                if ('signed? lhsT)
                    return (Any-wrap sf) true
                else
                    return (Any-wrap uf) true
            return (Any-wrap none) false

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
        __== = (box-binary-op-dispatch (single-binary-op-dispatch (typify ptrcmp== type type)))
        __!= = (box-binary-op-dispatch (single-binary-op-dispatch (typify ptrcmp!= type type)))
        __@ = sc_type_element_at
        __getattr = sc_type_at

    'set-symbols Scope
        __getattr = sc_scope_at

    'set-symbols syntax-scope
        immutable = (box-pointer immutable)
        aggregate = (box-pointer aggregate)
        opaquepointer = (box-pointer opaquepointer)
        SyntaxMacro = (box-pointer SyntaxMacro)
        SyntaxMacroFunctionType = (box-pointer SyntaxMacroFunctionType)
        DispatchCastFunctionType = (box-pointer DispatchCastFunctionType)
        BinaryOpFunctionType = (box-pointer BinaryOpFunctionType)
        implyfn = (typify implyfn type type)
        asfn = (typify asfn type type)
        try-imply = try-imply
        try-as = try-as
        countof = (make-unary-op-dispatch '__countof "count")
        not = (make-unary-op-dispatch '__not "negate")
        ~ = (make-unary-op-dispatch '__~ "bitwise-negate")
        repr = (make-unary-op-dispatch '__repr "get representational string of")
        tostring = (make-unary-op-dispatch '__tostring "get string of")
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

inline syntax-block-scope-macro (f)
    function->SyntaxMacro (unconst (typify f list list Scope))

inline syntax-scope-macro (f)
    syntax-block-scope-macro
        fn (at next scope)
            let at scope = (f ('next at) scope)
            return (cons (Any at) next) scope

inline syntax-macro (f)
    syntax-block-scope-macro
        fn (at next scope)
            return (cons (Any (f ('next at))) next) scope

fn empty? (value)
    == (countof value) 0:usize

syntax-extend
    # dotted symbol expander
    # --------------------------------------------------------------------------

    let dot-char = 46:i8 # "."
    let dot-sym = '.

    fn dotted-symbol? (env head)
        if (== head dot-sym)
            return false
        let s = (as head string)
        let sz = (countof s)
        let loop (i) = 0:usize
        if (== i sz)
            return false
        elseif (== (@ s i) dot-char)
            return true
        loop (+ i 1:usize)

    fn split-dotted-symbol (head start end tail)
        let s = (as head string)
        let loop (i) = start
        if (== i end)
            # did not find a dot
            if (== start 0:usize)
                return (cons (Any head) tail)
            else
                return (cons (Any (Symbol (lslice s start))) tail)
        if (== (@ s i) dot-char)
            let tail =
                # no remainder after dot
                if (== i (- end 1:usize)) tail
                else # remainder after dot, split the rest first
                    split-dotted-symbol head (+ i 1:usize) end tail
            let result = (cons (Any dot-sym) tail)
            if (== i 0:usize)
                # no prefix before dot
                return result
            else
                # prefix before dot
                let size = (- i start)
                return
                    cons (Any (Symbol (rslice (lslice s start) size))) result
        loop (+ i 1:usize)

    # infix notation support
    # --------------------------------------------------------------------------

    fn get-ifx-symbol (name)
        Symbol (.. "#ifx:" name)

    fn expand-define-infix (args scope order)
        let prec rest = ('decons args)
        let token rest = ('decons rest)
        let func rest = ('decons rest)
        let prec =
            as (as prec Syntax) i32
        let token =
            as (as token Syntax) Symbol
        let func =
            if (== ('typeof func) Nothing) token
            else
                as (as func Syntax) Symbol
        'set-symbol scope (get-ifx-symbol token)
            Any
                cons (Any prec)
                    cons (Any order)
                        cons (Any func) '()
        return none scope

    inline make-expand-define-infix (order)
        fn (args scope)
            expand-define-infix args scope order

    fn get-ifx-op (env op)
        let sym = ('datum (as op Syntax))
        if (== ('typeof sym) Symbol)
            getattr env (get-ifx-symbol (as sym Symbol))
        else
            return (Any none) false

    fn has-infix-ops? (infix-table expr)
        # any expression of which one odd argument matches an infix operator
            has infix operations.
        let loop (expr) = expr
        if (< (countof expr) 3:usize)
            return false
        let __ expr = ('decons expr)
        let at next = ('decons expr)
        let result ok = (get-ifx-op infix-table at)
        if ok
            return true
        loop expr

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
            let op ok =
                get-ifx-op infix-table token
            if ok
                let op-prec = (unpack-infix-op op)
                ? (pred op-prec prec) op (Any none)
            else
                set-anchor! ('anchor (as token Syntax))
                __anchor-error!
                    "unexpected token in infix expression"
                unreachable!;
    let infix-op-gt = (infix-op >)
    let infix-op-ge = (infix-op >=)

    fn rtl-infix-op-eq (infix-table token prec)
        let op ok =
            get-ifx-op infix-table token
        if ok
            let op-prec op-order = (unpack-infix-op op)
            if (== op-order '<)
                ? (== op-prec prec) op (Any none)
            else
                Any none
        else
            set-anchor! ('anchor (as token Syntax))
            __anchor-error!
                "unexpected token in infix expression"
            unreachable!;

    fn parse-infix-expr (infix-table lhs state mprec)
        let loop (lhs state) = lhs state
        if (empty? state)
            return lhs state
        let la next-state = ('decons state)
        let op = (infix-op-ge infix-table la mprec)
        if (== ('typeof op) Nothing)
            return lhs state
        let op-prec op-order op-name = (unpack-infix-op op)
        let rhs-loop (rhs state) = ('decons next-state)
        if (empty? state)
            loop (Any (list op-name lhs rhs)) state
        let ra __ = ('decons state)
        let lop = (infix-op-gt infix-table ra op-prec)
        let nextop =
            if (== ('typeof lop) Nothing)
                rtl-infix-op-eq infix-table ra op-prec
            else lop
        if (== ('typeof nextop) Nothing)
            loop (Any (list op-name lhs rhs)) state
        let nextop-prec = (unpack-infix-op nextop)
        let next-rhs next-state =
            parse-infix-expr infix-table rhs state nextop-prec
        rhs-loop next-rhs next-state

    let parse-infix-expr =
        typify parse-infix-expr Scope Any list i32

    #---------------------------------------------------------------------------

    # install general list hook for this scope
    # is called for every list the expander would otherwise consider a call
    fn list-handler (topexpr env)
        let topexpr-at topexpr-next = ('decons topexpr)
        let sxexpr = (as topexpr-at Syntax)
        let expr expr-anchor = ('datum sxexpr) ('anchor sxexpr)
        if (!= ('typeof expr) list)
            return topexpr env
        let expr = (as expr list)
        let expr-at expr-next = ('decons expr)
        let head-key = ('datum (as expr-at Syntax))
        let head =
            if (== ('typeof head-key) Symbol)
                let head success = (getattr env (as head-key Symbol))
                if success head
                else head-key
            else head-key
        let head =
            if (== ('typeof head) type)
                let attr ok = (getattr (as head type) '__macro)
                if ok attr
                else head
            else head
        if (== ('typeof head) SyntaxMacro)
            let head = (as head SyntaxMacro)
            let expr env = (head expr topexpr-next env)
            let expr = (Syntax-wrap expr-anchor (Any expr) false)
            return (as (as expr Syntax) list) env
        elseif (has-infix-ops? env expr)
            let at next = ('decons expr)
            let expr =
                parse-infix-expr env at next 0
            let expr = (Syntax-wrap expr-anchor expr false)
            return (cons expr topexpr-next) env
        else
            return topexpr env

    # install general symbol hook for this scope
    # is called for every symbol the expander could not resolve
    fn symbol-handler (topexpr env)
        let at next = ('decons topexpr)
        let sxname = (as at Syntax)
        let name name-anchor = (as sxname Symbol) ('anchor sxname)
        if (dotted-symbol? env name)
            let s = (as name string)
            let sz = (countof s)
            let expr =
                Any (split-dotted-symbol name (unconst 0:usize) sz eol)
            let expr = (Syntax-wrap name-anchor expr false)
            return (cons expr next) env
        return topexpr env

    'set-symbol syntax-scope (Symbol "#list")
        Any (unconst (typify list-handler list Scope))

    'set-symbol syntax-scope (Symbol "#symbol")
        Any (unconst (typify symbol-handler list Scope))

    # dot macro
    # (. value symbol ...)
    'set-symbols syntax-scope
        . =
            Any
                syntax-macro
                    fn (args)
                        fn op (a b)
                            let sym = (as (as b Syntax) Symbol)
                            list getattr a (list quote sym)
                        let a rest = ('decons args)
                        let b rest = ('decons rest)
                        let loop (rest result) = rest (op a b)
                        if (empty? rest)
                            result
                        else
                            let c rest = ('decons rest)
                            loop rest (op result c)
        define-infix> = (Any (syntax-scope-macro (make-expand-define-infix '>)))
        define-infix< = (Any (syntax-scope-macro (make-expand-define-infix '<)))

    syntax-scope

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


sc_exit 0
