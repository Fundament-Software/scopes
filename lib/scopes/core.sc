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

let __typify = sc_typify
let __compile = sc_compile
let __compile-object = sc_compile_object
let __compile-spirv = sc_compile_spirv
let __compile-glsl = sc_compile_glsl
let eval = sc_eval
let compiler-version = sc_compiler_version
let verify-stack! = sc_verify_stack
let enter-solver-cli! = sc_enter_solver_cli
let launch-args = sc_launch_args

let default-styler = sc_default_styler
let io-write! = sc_write
let format-message = sc_format_message
let __prompt = sc_prompt
let set-autocomplete-scope! = sc_set_autocomplete_scope

let file? = sc_is_file
let directory? = sc_is_directory
let realpath = sc_realpath
let dirname = sc_dirname
let basename = sc_basename

let globals = sc_get_globals
let set-globals! = sc_set_globals

let set-error! = sc_set_last_error
let set-location-error! = sc_set_last_location_error
let set-runtime-error! = sc_set_last_runtime_error
let get-error = sc_get_last_error
let format-error = sc_format_error
let CompileError = sc_location_error_new
let RuntimeError = sc_runtime_error_new
let exit = sc_exit
let set-signal-abort! = sc_set_signal_abort

let __hash = sc_hash
let __hash2x64 = sc_hash2x64
let __hashbytes = sc_hashbytes

let set-anchor! = sc_set_active_anchor
let active-anchor = sc_get_active_anchor

let import-c = sc_import_c
let load-library = sc_load_library

let Scope@ = sc_scope_at
let Scope-local@ = sc_scope_local_at
let Scope-docstring = sc_scope_get_docstring
let set-scope-docstring! = sc_scope_set_docstring
let Scope-new = sc_scope_new
let Scope-clone = sc_scope_clone
let Scope-new-expand = sc_scope_new_subscope
let Scope-clone-expand = sc_scope_clone_subscope
let Scope-parent = sc_scope_get_parent
let delete-scope-symbol! = sc_scope_del_symbol
let Scope-next = sc_scope_next

let string->Symbol = sc_symbol_new
let Symbol->string = sc_symbol_to_string

let string-join = sc_string_join
let string-new = sc_string_new
let string-match? = sc_string_match

let Any-repr = sc_any_repr
let Any-string = sc_any_string
let Any== = sc_any_eq

let list-cons = sc_list_cons
let list-join = sc_list_join
let list-dump = sc_list_dump

let Syntax-new = sc_syntax_new
let Syntax-wrap = sc_syntax_wrap
let Syntax-strip = sc_syntax_strip
let list-load = sc_syntax_from_path
let list-parse = sc_syntax_from_string

let element-type = sc_type_element_at
let type-countof = sc_type_countof
let sizeof = sc_type_sizeof
let runtime-type@ = sc_type_at
let element-index = sc_type_field_index
let element-name = sc_type_field_name
let type-kind = sc_type_kind
let storageof = sc_type_storage
let opaque? = sc_type_is_opaque
let type-name = sc_type_string
let type-next = sc_type_next
let set-type-symbol! = sc_type_set_symbol

let pointer-type = sc_pointer_type
let pointer-type-set-element-type = sc_pointer_type_set_element_type
let pointer-type-set-storage-class = sc_pointer_type_set_storage_class
let pointer-type-set-flags = sc_pointer_type_set_flags
let pointer-type-flags = sc_pointer_type_get_flags
let pointer-type-set-storage-class = sc_pointer_type_set_storage_class
let pointer-type-storage-class = sc_pointer_type_get_storage_class

let extern-type-location = sc_extern_type_location
let extern-type-binding = sc_extern_type_binding

let bitcountof = sc_type_bitcountof

let integer-type = sc_integer_type
let signed? = sc_integer_type_is_signed

let typename-type = sc_typename_type
let set-typename-super! = sc_typename_type_set_super
let superof = sc_typename_type_get_super
let set-typename-storage! = sc_typename_type_set_storage

let array-type = sc_array_type
let vector-type = sc_vector_type

let function-type-variadic? = sc_function_type_is_variadic

let Image-type = sc_image_type
let SampledImage-type = sc_sampled_image_type

let Parameter-new = sc_parameter_new
let Parameter-index = sc_parameter_index
let Parameter-name = sc_parameter_name

let Label-dump = sc_label_dump
let Label-docstring = sc_label_docstring
let Label-anchor = sc_label_anchor
let Label-name = sc_label_name
let Label-countof-reachable = sc_label_countof_reachable
let Label-set-inline! = sc_label_set_inline
let Label-enter = sc_label_get_enter
let Label-set-enter! = sc_label_set_enter

let Frame-dump = sc_frame_dump

let Closure-label = sc_closure_label
let Closure-frame = sc_closure_frame

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

fn raise-compile-error! (value)
    set-error! (CompileError value)
    __raise!;

# print an unboxing error given two types
fn unbox-verify (haveT wantT)
    if (ptrcmp!= haveT wantT)
        raise-compile-error!
            sc_string_join "can't unbox value of type "
                sc_string_join
                    sc_any_repr (box-pointer haveT)
                    sc_string_join " as value of type "
                        sc_any_repr (box-pointer wantT)

inline unbox-integer (value T)
    unbox-verify (extractvalue value 0) T
    itrunc (extractvalue value 1) T

inline unbox-symbol (value T)
    unbox-verify (extractvalue value 0) T
    bitcast (extractvalue value 1) T

# turn an Any back into a pointer
inline unbox-pointer (value T)
    unbox-verify (extractvalue value 0) T
    inttoptr (extractvalue value 1) T

fn verify-list-count (l mincount maxcount)
    let count = (itrunc (sc_list_count l) i32)
    if (icmp>=s mincount 0)
        if (icmp<s count mincount)
            raise-compile-error!
                sc_string_join "at least "
                    sc_string_join (sc_any_repr (box-integer mincount))
                        sc_string_join " argument(s) expected, got "
                            sc_any_repr (box-integer count)
    if (icmp>=s maxcount 0)
        if (icmp>s count maxcount)
            raise-compile-error!
                sc_string_join "at most "
                    sc_string_join (sc_any_repr (box-integer maxcount))
                        sc_string_join " argument(s) expected, got "
                            sc_any_repr (box-integer count)

fn verify-argument-count (l mincount maxcount)
    let original-args = (sc_label_get_arguments l)
    let __ args = (sc_list_decons original-args)
    verify-list-count args mincount maxcount
    return original-args

fn verify-keyed-count (l mincount maxcount)
    let original-args = (sc_label_get_keyed l)
    let __ args = (sc_list_decons original-args)
    verify-list-count args mincount maxcount
    return original-args

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

syntax-extend
    let val = (box-pointer (sc_pointer_type type pointer-flag-non-writable unnamed))
    sc_scope_set_symbol syntax-scope 'type-array val
    let T = (sc_type_storage LabelMacro)
    sc_scope_set_symbol syntax-scope 'LabelMacroFunctionType (box-pointer T)
    sc_scope_set_symbol syntax-scope 'ellipsis-symbol (box-symbol (sc_symbol_new "..."))
    syntax-scope

fn Label-return-args (l arg)
    let args = (sc_label_get_keyed l)
    let cont args = (sc_list_decons args)
    sc_label_set_enter l (Any-wrap _)
    sc_label_set_keyed l (sc_list_cons cont arg)

fn Label-return0 (l)
    Label-return-args l '()

fn Label-return1 (l arg1)
    Label-return-args l (sc_list_cons arg1 '())

fn Label-return2 (l arg1 arg2)
    Label-return-args l (sc_list_cons arg1 (sc_list_cons arg2 '()))

fn Label-return3 (l arg1 arg2 arg3)
    Label-return-args l (sc_list_cons arg1 (sc_list_cons arg2 (sc_list_cons arg3 '())))

fn pack-symbol-value (symbol value)
    box-pointer
        sc_list_cons (box-symbol symbol)
            sc_list_cons value '()

fn unpack-symbol-value (pair)
    let pair = (unbox-pointer pair list)
    let k pair = (sc_list_decons pair)
    let v = (sc_list_decons pair)
    return (unbox-symbol k Symbol) v

syntax-extend
    fn typify (l)
        let args = (verify-keyed-count l 1 -1)
        let args = (sc_list_next args)
        let src_label args = (sc_list_decons args)
        let k src_label = (unpack-symbol-value src_label)
        let src_label = (unbox-pointer src_label Closure)
        let count = (itrunc (sc_list_count args) i32)
        let types = (alloca-array type count)
        let loop (i args) = 0 args
        if (icmp<s i count)
            let at args = (sc_list_decons args)
            let k ty = (unpack-symbol-value at)
            store (unbox-pointer ty type) (getelementptr types i)
            loop (add i 1) args
        let func =
            sc_typify src_label count (bitcast types type-array)
        Label-return1 l
            pack-symbol-value unnamed (box-pointer func)
    do
        let types = (alloca-array type 1:usize)
        store Label (getelementptr types 0)
        let types = (bitcast types type-array)
        let result = (sc_compile (sc_typify typify 1 types) 0:u64)
        let result-type = (extractvalue result 0)
        if (ptrcmp!= result-type LabelMacroFunctionType)
            raise-compile-error!
                sc_string_join "label macro must have type "
                    sc_any_repr (box-pointer LabelMacroFunctionType)
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
    fn va-fold (l use-indices reverse)
        let args = (verify-keyed-count l 1 -1)
        let cont args = (sc_list_decons args)
        let f args = (sc_list_decons args)
        let k f = (unpack-symbol-value f)
        let numentries = (itrunc (sc_list_count args) i32)
        let frame = (sc_label_frame l)
        let anchor = (sc_label_anchor l)
        let args =
            if reverse
                sc_list_reverse args
            else args
        if (icmp== numentries 0)
            Label-return0 l
            return;
        let loop (i arg active-l last-param) = 0 args l (Any-wrap none)
        if (icmp!= (sc_list_count arg) 0:usize)
            let at arg = (sc_list_decons arg)
            let k v = (unpack-symbol-value at)
            loop (add i 1) arg
                do
                    let nextl = (sc_label_new_cont_template)
                    let param = (sc_parameter_new anchor ellipsis-symbol Unknown)
                    sc_label_append_parameter nextl param
                    let boxed-param = (box-pointer param)
                    sc_label_set_enter nextl (Any-wrap _)
                    sc_label_set_keyed nextl
                        sc_list_cons cont
                            sc_list_cons (pack-symbol-value unnamed boxed-param) '()
                    sc_label_set_enter active-l f
                    let args =
                        sc_list_cons (pack-symbol-value unnamed (box-symbol k))
                            sc_list_cons (pack-symbol-value unnamed v)
                                if (icmp>s i 0)
                                    sc_list_cons (pack-symbol-value unnamed last-param) '()
                                else '()
                    sc_label_set_keyed active-l
                        sc_list_cons
                            pack-symbol-value unnamed (box-pointer (sc_closure_new nextl frame))
                            if use-indices
                                let i =
                                    if reverse (sub (sub numentries i) 1)
                                    else i
                                sc_list_cons (pack-symbol-value unnamed (box-integer i)) args
                            else args
                    _ nextl boxed-param

    sc_scope_set_symbol syntax-scope 'va-lfold (box-label-macro (fn "va-lfold" (l) (va-fold l false false)))
    sc_scope_set_symbol syntax-scope 'va-lifold (box-label-macro (fn "va-ilfold" (l) (va-fold l true false)))
    sc_scope_set_symbol syntax-scope 'va-rfold (box-label-macro (fn "va-rfold" (l) (va-fold l false true)))
    sc_scope_set_symbol syntax-scope 'va-rifold (box-label-macro (fn "va-rifold" (l) (va-fold l true true)))

    fn type< (T superT)
        let loop (T) = T
        let value = (sc_typename_type_get_super T)
        if (ptrcmp== value superT) true
        elseif (ptrcmp== value typename) false
        else (loop value)

    fn type<= (T superT)
        if (ptrcmp== T superT) true
        else (type< T superT)

    fn type> (superT T)
        bxor (type<= T superT) true

    fn type>= (superT T)
        bxor (type< T superT) true

    fn compare-type (l f)
        let args = (verify-keyed-count l 2 2)
        let cont args = (sc_list_decons args)
        let a args = (sc_list_decons args)
        let b args = (sc_list_decons args)
        let k a = (unpack-symbol-value a)
        let k b = (unpack-symbol-value b)
        if (Any-constant? a)
            if (Any-constant? b)
                Label-return1 l
                    pack-symbol-value unnamed
                        box-integer
                            f (unbox-pointer a type) (unbox-pointer b type)
                return;
        sc_label_set_enter l (box-pointer f)

    inline type-comparison-func (f)
        fn (l) (compare-type l (unconst (typify f type type)))

    sc_scope_set_symbol syntax-scope 'type== (box-label-macro (type-comparison-func ptrcmp==))
    sc_scope_set_symbol syntax-scope 'type!= (box-label-macro (type-comparison-func ptrcmp!=))
    sc_scope_set_symbol syntax-scope 'type< (box-label-macro (type-comparison-func type<))
    sc_scope_set_symbol syntax-scope 'type<= (box-label-macro (type-comparison-func type<=))
    sc_scope_set_symbol syntax-scope 'type> (box-label-macro (type-comparison-func type>))
    sc_scope_set_symbol syntax-scope 'type>= (box-label-macro (type-comparison-func type>=))

    # tuple type constructor
    sc_type_set_symbol tuple '__typecall
        box-label-macro
            fn "tuple" (l)
                let args = (verify-keyed-count l 1 -1)
                let args = (sc_list_next (sc_list_next args))
                let pcount = (itrunc (sc_list_count args) i32)
                let types = (alloca-array type pcount)
                let loop (i args) = 0 args
                if (icmp<s i pcount)
                    let arg args = (sc_list_decons args)
                    let k arg = (unpack-symbol-value arg)
                    let T = (unbox-pointer arg type)
                    store T (getelementptr types i)
                    loop (add i 1) args
                let ttype = (sc_tuple_type pcount (bitcast types type-array))
                Label-return1 l
                    pack-symbol-value unnamed (box-pointer ttype)

    # function pointer type constructor
    sc_type_set_symbol function '__typecall
        box-label-macro
            fn "function" (l)
                let args = (verify-keyed-count l 2 -1)
                let args = (sc_list_next (sc_list_next args))
                let rtype args = (sc_list_decons args)
                let k rtype = (unpack-symbol-value rtype)
                let rtype = (unbox-pointer rtype type)
                let pcount = (itrunc (sc_list_count args) i32)
                let types = (alloca-array type pcount)
                let loop (i args) = 0 args
                if (icmp<s i pcount)
                    let arg args = (sc_list_decons args)
                    let k arg = (unpack-symbol-value arg)
                    let T = (unbox-pointer arg type)
                    store T (getelementptr types i)
                    loop (add i 1) args
                let ftype = (sc_function_type rtype pcount (bitcast types type-array))
                Label-return1 l
                    pack-symbol-value unnamed (box-pointer ftype)

    # method call syntax
    sc_type_set_symbol Symbol '__call
        box-label-macro
            fn "symbol-call" (l)
                let args = (verify-keyed-count l 2 -1)
                let cont args = (sc_list_decons args)
                let sym retargs = (sc_list_decons args)
                let self args = (sc_list_decons retargs)
                let k sym = (unpack-symbol-value sym)
                let k self = (unpack-symbol-value self)
                let T = (Any-indirect-typeof self)
                let ok f = (sc_type_at T (unbox-symbol sym Symbol))
                if ok
                    sc_label_set_keyed l (sc_list_cons cont retargs)
                    sc_label_set_enter l f
                else
                    raise-compile-error!
                        sc_string_join "no method named "
                            sc_string_join (sc_any_repr sym)
                                sc_string_join " in value of type "
                                    sc_any_repr (box-pointer T)

    inline gen-key-any-set (selftype fset)
        box-label-macro
            fn "set-symbol" (l)
                let args = (verify-keyed-count l 2 3)
                let args = (sc_list_next args)
                let lself args = (sc_list_decons args)
                let key value =
                    if (icmp== (sc_list_count args) 2:usize)
                        let key args = (sc_list_decons args)
                        let value args = (sc_list_decons args)
                        let k key = (unpack-symbol-value key)
                        let k value = (unpack-symbol-value value)
                        _ key value
                    else
                        let key value = (unpack-symbol-value (sc_list_at args))
                        _ (box-symbol key) value
                let k self = (unpack-symbol-value lself)
                if (Any-constant? self)
                    if (Any-constant? key)
                        if (Any-constant? value)
                            let self = (unbox-pointer self selftype)
                            let key = (unbox-symbol key Symbol)
                            fset self key value
                            Label-return0 l
                            return;
                Label-return3 l lself
                    pack-symbol-value unnamed key
                    pack-symbol-value unnamed value
                sc_label_set_enter l (Any-wrap fset)

    # quick assignment of type attributes
    sc_type_set_symbol type 'set-symbol (gen-key-any-set type sc_type_set_symbol)
    sc_type_set_symbol Scope 'set-symbol (gen-key-any-set Scope sc_scope_set_symbol)

    sc_type_set_symbol type 'pointer
        box-label-macro
            fn "type-pointer" (l)
                let args = (verify-keyed-count l 1 1)
                let args = (sc_list_next args)
                let self args = (sc_list_decons args)
                let k self = (unpack-symbol-value self)
                let T = (unbox-pointer self type)
                Label-return1 l
                    pack-symbol-value unnamed
                        box-pointer
                            sc_pointer_type T pointer-flag-non-writable unnamed

    sc_type_set_symbol type 'raising
        box-label-macro
            fn "type-raising" (l)
                let args = (verify-keyed-count l 1 1)
                let args = (sc_list_next args)
                let self args = (sc_list_decons args)
                let k self = (unpack-symbol-value self)
                let T = (unbox-pointer self type)
                Label-return1 l
                    pack-symbol-value unnamed
                        box-pointer
                            sc_function_type_raising T

    # typecall

    sc_type_set_symbol type '__call
        box-label-macro
            fn "type-call" (l)
                let args = (sc_label_get_keyed l)
                let args = (sc_list_next args)
                let self args = (sc_list_decons args)
                let k self = (unpack-symbol-value self)
                let T = (unbox-pointer self type)
                let ok f = (sc_type_at T '__typecall)
                if ok
                    sc_label_set_enter l f
                    return;
                raise-compile-error!
                    sc_string_join "no type constructor available for type "
                        sc_any_repr self

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

    sc_scope_set_symbol syntax-scope 'none?
        box-label-macro
            fn (l)
                let args = (verify-keyed-count l 1 1)
                let args = (sc_list_next args)
                let value args = (sc_list_decons args)
                let k value = (unpack-symbol-value value)
                Label-return1 l
                    pack-symbol-value unnamed
                        box-integer
                            ptrcmp== (Any-typeof value) Nothing

    fn unpack2 (l)
        let args = (verify-keyed-count l 2 2)
        let args = (sc_list_next args)
        let a args = (sc_list_decons args)
        let b args = (sc_list_decons args)
        let k a = (unpack-symbol-value a)
        let k b = (unpack-symbol-value b)
        return a b

    sc_scope_set_symbol syntax-scope 'const.icmp<=.i32.i32
        box-label-macro
            fn (l)
                let a b = (unpack2 l)
                if (Any-constant? a)
                    if (Any-constant? b)
                        let a = (unbox-integer a i32)
                        let b = (unbox-integer b i32)
                        Label-return1 l
                            pack-symbol-value unnamed
                                box-integer (icmp<=s a b)
                        return;
                raise-compile-error! "arguments must be constant"

    sc_scope_set_symbol syntax-scope 'const.add.i32.i32
        box-label-macro
            fn (l)
                let a b = (unpack2 l)
                if (Any-constant? a)
                    if (Any-constant? b)
                        let a = (unbox-integer a i32)
                        let b = (unbox-integer b i32)
                        Label-return1 l
                            pack-symbol-value unnamed
                                box-integer (add a b)
                        return;
                raise-compile-error! "arguments must be constant"

    sc_scope_set_symbol syntax-scope 'constbranch
        box-label-macro
            fn (l)
                let args = (verify-keyed-count l 3 3)
                let args = (sc_list_next args)
                let cond args = (sc_list_decons args)
                let thenf args = (sc_list_decons args)
                let elsef args = (sc_list_decons args)
                let k cond = (unpack-symbol-value cond)
                let k thenf = (unpack-symbol-value thenf)
                let k elsef = (unpack-symbol-value elsef)
                if (Any-constant? cond)
                else
                    raise-compile-error! "condition must be constant"
                let value = (unbox-integer cond bool)
                Label-return0 l
                sc_label_set_enter l
                    if value thenf
                    else elsef

    sc_type_set_symbol Any '__typecall
        box-label-macro
            fn (l)
                let args = (verify-argument-count l 2 2)
                inline make-none ()
                    Any-wrap none
                let args = (sc_list_next (sc_list_next args))
                let self = (sc_list_at args)
                let T = (Any-indirect-typeof self)
                Label-return1 l (pack-symbol-value unnamed self)
                if (ptrcmp!= T Any)
                    if (Any-constant? self)
                        sc_label_set_enter l (Any-wrap Any-wrap)
                    elseif (ptrcmp== T Nothing)
                        sc_label_set_enter l (Any-wrap make-none)
                    else
                        let storageT = (sc_type_storage T)
                        let kind = (sc_type_kind storageT)
                        if (icmp== kind type-kind-pointer)
                            sc_label_set_enter l (box-pointer box-pointer)
                        elseif (icmp== kind type-kind-integer)
                            sc_label_set_enter l (box-pointer box-integer)
                        elseif (icmp== kind type-kind-extern)
                            sc_label_set_enter l (box-pointer box-symbol)
                        #elseif (bor (icmp== kind type-kind-tuple) (icmp== kind type-kind-array))
                        #elseif (icmp== kind type-kind-vector)
                        #elseif (icmp== kind type-kind-real)
                        else
                            raise-compile-error!
                                sc_string_join "can't box value of type "
                                    sc_any_repr (box-pointer T)


    syntax-scope

fn cons (values...)
    va-rifold
        inline (i key value next)
            constbranch (none? next)
                inline () value
                inline ()
                    sc_list_cons (Any value) next
        values...

fn make-list (values...)
    constbranch (const.icmp<=.i32.i32 (va-countof values...) 0)
        inline () '()
        inline ()
            va-rifold
                inline (i key value next)
                    sc_list_cons (Any value)
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
    return at
        constbranch (const.icmp<=.i32.i32 count 1)
            inline () next
            inline () (decons next (const.add.i32.i32 count -1))

inline set-symbols (self values...)
    va-lfold
        inline (key value)
            'set-symbol self key value
        values...

fn Label-return-keyed (l values...)
    let values... =
        va-rfold
            inline (key value next...)
                return (pack-symbol-value key value) next...
            values...
    let args = (sc_label_get_keyed l)
    let cont args = (sc_list_decons args)
    sc_label_set_enter l (Any-wrap _)
    sc_label_set_keyed l (sc_list_cons cont (make-list values...))

fn Label-return (l values...)
    let args = (sc_label_get_arguments l)
    let cont args = (sc_list_decons args)
    sc_label_set_enter l (Any-wrap _)
    sc_label_set_arguments l (sc_list_cons cont (make-list values...))

'set-symbol type 'set-symbols set-symbols
'set-symbol Scope 'set-symbols set-symbols

'set-symbols Any
    constant? = (typify Any-constant? Any)
    none? = (typify Any-none? Any)
    __repr = sc_any_repr
    indirect-typeof = (typify Any-indirect-typeof Any)
    typeof = (typify Any-typeof Any)

let Syntax-wrap = sc_syntax_wrap

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
'set-symbols Label
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

'set-symbols Closure
    frame = sc_closure_frame
    label = sc_closure_label

let rawstring = ('pointer i8)

inline box-cast-dispatch (f)
    box-pointer (unconst (typify f type type))

inline not (value)
    bxor value true

inline Syntax-unbox

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
        'pointer ('raising (function (tuple list Scope) list list Scope))
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

    fn any-imply (vT T)
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

    inline Any-rimply (cls value)
        Any value

    'set-symbols Any
        __imply =
            box-pointer (unconst (typify any-imply type type))
        __rimply =
            box-cast-dispatch
                fn "syntax-imply" (T vT)
                    return (Any-wrap Any-rimply) true

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
        box-pointer (unconst (typify f type type))
    inline single-binary-op-dispatch (destf)
        fn (lhsT rhsT)
            if (ptrcmp== lhsT rhsT)
                return (Any-wrap destf)
            raise-compile-error! "unsupported type"

    inline gen-cast-error (intro-string)
        label-macro
            fn "cast-error" (l)
                let args = ('verify-argument-count l 2 2)
                let cont value T = ('decons args 3)
                let vT = ('indirect-typeof value)
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

    let DispatchCastFunctionType =
        'pointer ('raising (function Any type type))

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
            let ok f = (trycall unbox-dispatch-cast-function-type anyf)
            if (not ok)
                attribute-format-error! vT symbol (get-error)
            let ok f = (trycall f vT T)
            if ok
                return true f false
        let ok anyf = ('@ T rsymbol)
        if ok
            let ok f = (trycall unbox-dispatch-cast-function-type anyf)
            if (not ok)
                attribute-format-error! vT symbol (get-error)
            let ok f = (trycall f T vT)
            if ok
                return true f true
        return false (Any-wrap none) false

    fn implyfn (vT T)
        get-cast-dispatcher '__imply '__rimply vT T
    fn asfn (vT T)
        get-cast-dispatcher '__as '__ras vT T

    let imply =
        box-label-macro
            fn "imply-dispatch" (l)
                let args = ('verify-argument-count l 2 2)
                let cont value anyT = ('decons args 3)
                let vT = ('indirect-typeof value)
                let T = (unbox-pointer anyT type)
                if (ptrcmp!= vT T)
                    let ok f reverse = (implyfn vT T)
                    if ok
                        'set-enter l f
                        if reverse ('set-arguments l (make-list cont anyT value))
                        return;
                    'set-enter l
                        box-pointer
                            gen-cast-error "can't implicitly cast value of type "
                else
                    'return l value

    let as =
        box-label-macro
            fn "as-dispatch" (l)
                let args = ('verify-argument-count l 2 2)
                let cont value anyT = ('decons args 3)
                let vT = ('indirect-typeof value)
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
                        'set-enter l f
                        if reverse ('set-arguments l (make-list cont anyT value))
                        return;
                    'set-enter l
                        box-pointer
                            gen-cast-error "can't cast value of type "
                else
                    'return l value

    let UnaryOpFunctionType =
        'pointer ('raising (function Any type))

    let BinaryOpFunctionType =
        'pointer ('raising (function Any type type))

    fn unbox-binary-op-function-type (anyf)
        unbox-pointer anyf BinaryOpFunctionType

    fn get-binary-op-dispatcher (symbol lhsT rhsT)
        let ok anyf = ('@ lhsT symbol)
        if ok
            let ok f = (trycall unbox-binary-op-function-type anyf)
            if (not ok)
                attribute-format-error! lhsT symbol (get-error)
            return (trycall f lhsT rhsT)
        return false (undef Any)

    fn binary-op-label-cast-then-macro (l f castf lhsT rhs)
        # next label
        let lcont = (sc_label_new_cont_template)
        let param = (sc_parameter_new (sc_get_active_anchor) unnamed Unknown)
        'append-parameter lcont param
        # need to generate cast
        'set-enter l castf
        'set-arguments l
            list
                box-pointer (Closure lcont ('frame l))
                rhs
                box-pointer lhsT
        'set-enter lcont f
        return lcont param


    # both types are typically the same
    fn sym-binary-op-label-macro (l symbol rsymbol friendly-op-name)
        let args = ('verify-argument-count l 2 2)
        let cont lhs rhs = ('decons args 3)
        let lhsT = ('indirect-typeof lhs)
        let rhsT = ('indirect-typeof rhs)
        # try direct version first
        let ok f = (get-binary-op-dispatcher symbol lhsT rhsT)
        if ok
            'set-enter l f
            return;
        # if types are unequal, we can try other options
        if (ptrcmp!= lhsT rhsT)
            # try reverse version next
            let ok f = (get-binary-op-dispatcher rsymbol rhsT lhsT)
            if ok
                'return l rhs lhs
                'set-enter l f
                return;
            # can the operation be performed on the lhs type?
            let ok f = (get-binary-op-dispatcher symbol lhsT lhsT)
            if ok
                # can we cast rhsT to lhsT?
                let ok castf reverse = (implyfn rhsT lhsT)
                if ok
                    if (not reverse)
                        let lcont param =
                            binary-op-label-cast-then-macro l f castf lhsT rhs
                        'set-arguments lcont
                            list cont lhs (box-pointer param)
                        return;
            # can the operation be performed on the rhs type?
            let ok f = (get-binary-op-dispatcher symbol rhsT rhsT)
            if ok
                # can we cast lhsT to rhsT?
                let ok castf reverse = (implyfn lhsT rhsT)
                if ok
                    if (not reverse)
                        let lcont param =
                            binary-op-label-cast-then-macro l f castf rhsT lhs
                        'set-arguments lcont
                            list cont (box-pointer param) rhs
                        return;
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
    fn asym-binary-op-label-macro (l symbol rtype friendly-op-name)
        let args = ('verify-argument-count l 2 2)
        let cont lhs rhs = ('decons args 3)
        let lhsT = ('indirect-typeof lhs)
        let rhsT = ('indirect-typeof rhs)
        let ok f = ('@ lhsT symbol)
        if ok
            if (ptrcmp== rhsT rtype)
                'set-enter l f
                return;
            # can we cast rhsT to rtype?
            let ok castf reverse = (implyfn rhsT rtype)
            if ok
                if (not reverse)
                    let lcont param =
                        binary-op-label-cast-then-macro l f castf rtype rhs
                    'set-arguments lcont
                        list cont lhs (box-pointer param)
                    return;
        # we give up
        raise-compile-error!
            'join "can't "
                'join friendly-op-name
                    'join " values of types "
                        'join
                            '__repr (box-pointer lhsT)
                            'join " and "
                                '__repr (box-pointer rhsT)

    fn unary-op-label-macro (l symbol friendly-op-name)
        let args = ('verify-argument-count l 1 1)
        let cont lhs = ('decons args 2)
        let lhsT = ('indirect-typeof lhs)
        let ok f = ('@ lhsT symbol)
        if ok
            'set-enter l f
            return;
        raise-compile-error!
            'join "can't "
                'join friendly-op-name
                    'join " value of type "
                        '__repr (box-pointer lhsT)

    inline make-unary-op-dispatch (symbol friendly-op-name)
        box-label-macro (fn (l) (unary-op-label-macro l symbol friendly-op-name))

    inline make-sym-binary-op-dispatch (symbol rsymbol friendly-op-name)
        box-label-macro (fn (l) (sym-binary-op-label-macro l symbol rsymbol friendly-op-name))

    inline make-asym-binary-op-dispatch (symbol rtype friendly-op-name)
        box-label-macro (fn (l) (asym-binary-op-label-macro l symbol rtype friendly-op-name))

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
                        return (Any-wrap symbol-imply)
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
                '__repr (Any self)

    inline single-signed-binary-op-dispatch (sf uf)
        fn (lhsT rhsT)
            if (ptrcmp== lhsT rhsT)
                if ('signed? lhsT)
                    return (Any-wrap sf)
                else
                    return (Any-wrap uf)
            raise-compile-error! "unsupported type"

    fn dispatch-and-or (l flip)
        let args = ('verify-argument-count l 2 2)
        let cont cond elsef = ('decons args 3)
        if (Any-constant? cond)
            let value = (unbox-integer cond bool)
            if (bxor value flip)
                'return l cond
            else
                'return l
                'set-enter l elsef
            return;
        let nextl = (sc_label_new_inline_template)
        'set-enter nextl (Any-wrap _)
        let params = ('parameters nextl)
        'set-arguments nextl
            list (box-pointer (unbox-pointer ('@ params) Parameter)) cond
        if flip
            'return l cond
                elsef
                box-pointer (Closure nextl ('frame l))
        else
            'return l cond
                box-pointer (Closure nextl ('frame l))
                elsef
        'set-enter l (Any-wrap branch)

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
            box-label-macro
                fn "type-getattr" (l)
                    let args = ('verify-argument-count l 2 2)
                    let cont self key = ('decons args 3)
                    if (Any-constant? self)
                        if (Any-constant? key)
                            let self = (unbox-pointer self type)
                            let key = (unbox-symbol key Symbol)
                            let ok result = (sc_type_at self key)
                            'return l result (box-integer ok)
                            return;
                    'set-enter l (Any-wrap sc_type_at)

    'set-symbols Scope
        __getattr = sc_scope_at
        __typecall =
            box-label-macro
                fn "scope-typecall" (l)
                    """"There are four ways to create a new Scope:
                        ``Scope``
                            creates an empty scope without parent
                        ``Scope parent``
                            creates an empty scope descending from ``parent``
                        ``Scope none clone``
                            duplicate ``clone`` without a parent
                        ``Scope parent clone``
                            duplicate ``clone``, but descending from ``parent`` instead
                    let args = ('verify-argument-count l 1 3)
                    let cont cls parent clone = ('decons args 4)
                    let new? = (type== ('indirect-typeof clone) Nothing)
                    if (type== ('indirect-typeof parent) Nothing)
                        if new?
                            'return l
                            'set-enter l (Any-wrap Scope-new)
                        else
                            'return l clone
                            'set-enter l (Any-wrap Scope-clone)
                    else
                        if new?
                            'return l parent
                            'set-enter l (Any-wrap Scope-new-expand)
                        else
                            'return l parent clone
                            'set-enter l (Any-wrap Scope-clone-expand)

    let Syntax-datum =
        typify
            fn "Syntax-datum" (sx)
                extractvalue (load sx) 1
            Syntax

    'set-symbols Syntax
        anchor =
            typify
                fn "Syntax-anchor" (sx)
                    extractvalue (load sx) 0
                Syntax
        datum = Syntax-datum
        __imply =
            box-cast-dispatch
                fn "syntax-imply" (vT T)
                    if (ptrcmp== T Any)
                        return (box-pointer Syntax-datum)
                    elseif false
                        # force function to become raising
                        raise-compile-error! "unsupported type"
                    else
                        return (box-pointer Syntax-unbox)

    'set-symbols syntax-scope
        and-branch = (box-label-macro (fn (l) (dispatch-and-or l true)))
        or-branch = (box-label-macro (fn (l) (dispatch-and-or l false)))
        immutable = (box-pointer immutable)
        aggregate = (box-pointer aggregate)
        opaquepointer = (box-pointer opaquepointer)
        SyntaxMacro = (box-pointer SyntaxMacro)
        SyntaxMacroFunctionType = (box-pointer SyntaxMacroFunctionType)
        DispatchCastFunctionType = (box-pointer DispatchCastFunctionType)
        BinaryOpFunctionType = (box-pointer BinaryOpFunctionType)
        implyfn = (typify implyfn type type)
        asfn = (typify asfn type type)
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
        constbranch = constbranch
    syntax-scope

inline Syntax-unbox (self destT)
    imply ('datum self) destT

inline not (value)
    bxor (imply value bool) true

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

fn cons (at next)
    sc_list_cons (Any at) next

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
    let f ok = (getattr T '__tostring)
    constbranch ok
        inline ()
            f value
        inline ()
            sc_any_string (Any value)

fn repr (value)
    let T = (typeof value)
    let f ok = (getattr T '__repr)
    let s =
        constbranch ok
            inline ()
                f value
            inline ()
                sc_any_repr (Any value)
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

syntax-extend
    # implicit argument type coercion for functions, externs and typed labels
    # --------------------------------------------------------------------------

    let coerce-call-arguments =
        box-label-macro
            fn "coerce-call-arguments" (l)
                let args = ('arguments l)
                let cont self args = ('decons args 2)
                let T = ('indirect-typeof self)
                let fptrT =
                    if (== ('kind T) type-kind-extern) ('element@ T 0)
                    elseif (== T Label) ('function-type (as self Label))
                    else T
                let fT = ('element@ fptrT 0)
                #print fT args
                let frame = ('frame l)
                let anchor = ('body-anchor l)
                let pcount = (- ('element-count fT) 1)
                let acount = (as (countof args) i32)
                if (== pcount acount)
                    let loop (i inargs outargs l) = 1 args '() l
                    if (<= i pcount)
                        let arg inargs = ('decons inargs)
                        let argT = ('indirect-typeof arg)
                        let paramT = ('element@ fT i)
                        #print argT paramT
                        loop (+ i 1) inargs
                            if (== argT paramT) arg
                                _ (cons arg outargs) l
                            else
                                # need to generate an implicit cast
                                let nextl = (sc_label_new_cont_template)
                                let param = (sc_parameter_new anchor unnamed Unknown)
                                'append-parameter nextl param
                                'set-enter l (Any imply)
                                'set-arguments l (list (Closure nextl frame) arg paramT)
                                _ (cons (Any param) outargs) nextl
                    # do the call with (possibly) modified arguments
                    'set-enter l self
                    'set-arguments l (cons cont ('reverse outargs))
                    'set-rawcall l
                else
                    # is going to fail anyway, forward to compiler
                    'set-enter l self
                    'set-arguments l (cons cont args)
                    'set-rawcall l

    'set-symbols extern
        __call = coerce-call-arguments

    'set-symbols pointer
        __call = coerce-call-arguments

    'set-symbols Label
        __call = coerce-call-arguments

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
                return (cons head tail)
            else
                return (cons (Symbol (lslice s start)) tail)
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
                    cons (Symbol (rslice (lslice s start) size)) result
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
            Any (cons prec (cons order (cons func '())))
        return none scope

    inline make-expand-define-infix (order)
        fn (args scope)
            expand-define-infix args scope order

    fn get-ifx-op (env op)
        let sym = ('datum (as op Syntax))
        if (== ('typeof sym) Symbol)
            getattr env (get-ifx-symbol (as sym Symbol))
        else
            return false (Any none)

    fn has-infix-ops? (infix-table expr)
        # any expression of which one odd argument matches an infix operator
            has infix operations.
        let loop (expr) = expr
        if (< (countof expr) 3:usize)
            return false
        let __ expr = ('decons expr)
        let at next = ('decons expr)
        let ok result = (get-ifx-op infix-table at)
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
            let ok op =
                get-ifx-op infix-table token
            if ok
                let op-prec = (unpack-infix-op op)
                ? (pred op-prec prec) op (Any none)
            else
                set-anchor! ('anchor (as token Syntax))
                raise-compile-error!
                    "unexpected token in infix expression"
    let infix-op-gt = (infix-op >)
    let infix-op-ge = (infix-op >=)

    fn rtl-infix-op-eq (infix-table token prec)
        let ok op =
            get-ifx-op infix-table token
        if ok
            let op-prec op-order = (unpack-infix-op op)
            if (== op-order '<)
                ? (== op-prec prec) op (Any none)
            else
                Any none
        else
            set-anchor! ('anchor (as token Syntax))
            raise-compile-error!
                "unexpected token in infix expression"

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
                let ok head = (getattr env (as head-key Symbol))
                if ok head
                else head-key
            else head-key
        let head =
            if (== ('typeof head) type)
                let ok attr = (getattr (as head type) '__macro)
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

    fn unwrap-syntax (x)
        if (== ('typeof x) Syntax)
            'datum (as x Syntax)
        else x

    fn backquote-list (x)
        inline backquote-any (ox)
            let x = (unwrap-syntax ox)
            let T = ('typeof x)
            if (== T list)
                backquote-list (as x list)
            else
                cons quote (cons ox '())
        if (empty? x)
            return (cons quote (cons x '()))
        let aat next = ('decons x)
        let at = (unwrap-syntax aat)
        let T = ('typeof at)
        if (== T list)
            let at = (as at list)
            if (not (empty? at))
                let at-at at-next = ('decons at)
                let at-at = (unwrap-syntax at-at)
                if (== ('typeof at-at) Symbol)
                    let at-at = (as at-at Symbol)
                    if (== at-at 'unquote-splice)
                        return
                            cons (Any-wrap sc_list_join)
                                cons (cons do at-next)
                                    cons (backquote-list next) '()
        elseif (== T Symbol)
            let at = (as at Symbol)
            if (== at 'unquote)
                return (cons do next)
            elseif (== at 'backquote)
                return (backquote-list (backquote-list next))
        return
            cons cons
                cons (backquote-any aat)
                    cons (backquote-list next) '()

    fn quote-label (expr scope)
        let arg = (sc_syntax_wrap (sc_get_active_anchor) (Any expr) false)
        let arg = (as ('datum (as arg Syntax)) list)
        sc_eval_inline arg scope

    fn expand-and-or (expr f)
        if (empty? expr)
            raise-compile-error! "at least one argument expected"
        elseif (== (countof expr) 1:usize)
            return ('@ expr)
        let expr = ('reverse expr)
        let loop (result head) = ('decons expr)
        if (empty? head)
            return result
        let at next = ('decons head)
        loop (Any (list f at (list inline '() result))) next

    inline make-expand-and-or (f)
        fn (expr)
            expand-and-or expr f

    fn ltr-multiop (l target)
        let args = ('verify-argument-count l 2 -1)
        let cont args = ('decons args)
        if (== (countof args) 2:usize)
            'set-enter l target
        else
            let frame = ('frame l)
            let anchor = (active-anchor)
            # call for multiple args
            let lhs args = ('decons args)
            let loop (lhs args l) = lhs args l
            if (not (empty? args))
                let rhs args = ('decons args)
                'set-enter l (Any target)
                if (empty? args)
                    'set-arguments l (list cont lhs rhs)
                else
                    let nextl = (sc_label_new_cont_template)
                    let param = (sc_parameter_new anchor unnamed Unknown)
                    'append-parameter nextl param
                    'set-arguments l (list (Closure nextl frame) lhs rhs)
                    loop (Any param) args nextl
        return;

    fn rtl-multiop (l target)
        let args = ('verify-argument-count l 2 -1)
        let cont args = ('decons args)
        if (== (countof args) 2:usize)
            'set-enter l target
        else
            let frame = ('frame l)
            let anchor = (active-anchor)
            # call for multiple args
            let args = ('reverse args)
            let rhs args = ('decons args)
            let loop (rhs args l) = rhs args l
            if (not (empty? args))
                let lhs args = ('decons args)
                'set-enter l (Any target)
                if (empty? args)
                    'set-arguments l (list cont lhs rhs)
                else
                    let nextl = (sc_label_new_cont_template)
                    let param = (sc_parameter_new anchor unnamed Unknown)
                    'append-parameter nextl param
                    'set-arguments l (list (Closure nextl frame) lhs rhs)
                    loop (Any param) args nextl
        return;

    # dot macro
    # (. value symbol ...)
    'set-symbols syntax-scope
        backquote-list = backquote-list
        backquote =
            Any
                syntax-macro
                    fn (args)
                        backquote-list args
        #quote-label = quote-label
        quote-inline =
            Any
                syntax-scope-macro
                    fn (args scope)
                        return
                            cons quote-label
                                cons (backquote-list args)
                                    cons scope '()
                            scope
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
        and = (Any (syntax-macro (make-expand-and-or and-branch)))
        or = (Any (syntax-macro (make-expand-and-or or-branch)))
        define-infix> = (Any (syntax-scope-macro (make-expand-define-infix '>)))
        define-infix< = (Any (syntax-scope-macro (make-expand-define-infix '<)))
        .. = (box-label-macro (fn (l) (rtl-multiop l (Any ..))))
        + = (box-label-macro (fn (l) (ltr-multiop l (Any +))))
        * = (box-label-macro (fn (l) (ltr-multiop l (Any *))))

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

inline char (s)
    let s sz = (sc_string_buffer s)
    load s

#syntax-extend
    'set-symbols syntax-scope
        block-macro =
            Any
                syntax-macro
                    fn (args)
                        let arg =
                            backquote
                                label-macro
                                    fn (source-label)
                                        let enter =
                                            Closure
                                                unquote
                                                    cons do args
                                                'frame source-label
                                        'return source-label
                                        'set-enter source-label (Any enter)
                        let arg = ('decons arg)
                        arg as list
    syntax-scope

#syntax-extend
    'set-symbols syntax-scope
        hello =
            Any
                block-macro
                    let k arg = ('argument source-label 1)
                    quote-inline
                        io-write! "hello "
                        io-write! (unquote arg)
                        io-write! "\n"
    syntax-scope

#-------------------------------------------------------------------------------
# REPL
#-------------------------------------------------------------------------------

fn compiler-version-string ()
    let vmin vmaj vpatch = (compiler-version)
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

#fn read-eval-print-loop ()
    fn repeat-string (n c)
        let loop (i s) =
            tie-const n (usize 0)
            tie-const n ""
        if (i == n)
            return s
        loop (i + (usize 1))
            .. s c

    fn leading-spaces (s)
        let len = (i32 (countof s))
        let loop (i) = (tie-const len 0)
        if (i == len)
            return s
        let c = (@ s i)
        if (c != (char " "))
            return (string-new (string->rawstring s) (usize i))
        loop (i + 1)

    fn blank? (s)
        let len = (i32 (countof s))
        let loop (i) =
            tie-const len 0
        if (i == len)
            return (unconst true)
        if ((@ s i) != (char " "))
            return (unconst false)
        loop (i + 1)

    let cwd =
        realpath "."

    print-logo;
    print " "
        compiler-version-string;

    let global-scope = (globals)
    let eval-scope = (Scope global-scope)
    set-autocomplete-scope! eval-scope

    set-scope-symbol! eval-scope 'module-dir cwd
    loop (preload cmdlist counter eval-scope) = "" "" 0 eval-scope
    #dump "loop"
    fn make-idstr (counter)
        .. "$" (string-repr counter)

    let idstr = (make-idstr counter)
    let promptstr =
        .. idstr " "
            default-styler style-comment "►"
    let promptlen = ((countof idstr) + 2:usize)
    let cmd success =
        prompt
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
        if (slen == 0:usize) (unconst false)
        else
            (@ s (slen - 1:usize)) == (char " ")
    let enter-multiline = (endswith-blank cmd)
    #dump "loop 1"
    let terminated? =
        (blank? cmd) or
            (empty? cmdlist) and (not enter-multiline)
    let cmdlist =
        .. cmdlist
            if enter-multiline
                slice cmd 0 -1
            else cmd
            "\n"
    let preload =
        if terminated? (unconst "")
        else (leading-spaces cmd)
    if (not terminated?)
        repeat preload cmdlist counter eval-scope

    define-scope-macro set-scope!
        let scope rest = (decons args)
        return
            none
            scope as Syntax as Scope

    define-scope-macro get-scope
        return
            syntax-scope
            syntax-scope

    fn handle-retargs (counter eval-scope local-scope vals...)
        # copy over values from local-scope
        for k v in local-scope
            set-scope-symbol! eval-scope k v
        let count = (va-countof vals...)
        let loop (i) = 0
        if (i < count)
            let x = (va@ i vals...)
            let k = (counter + i)
            let idstr = (make-idstr k)
            set-scope-symbol! eval-scope (Symbol idstr) x
            print idstr "="
                repr x
            loop (add i 1)
        return
            unconst eval-scope
            unconst count

    let eval-scope count =
        xpcall
            inline ()
                let expr = (list-parse cmdlist)
                let expr-anchor = (Syntax-anchor expr)
                let tmp = (Parameter 'vals...)
                let expr =
                    Syntax-wrap expr-anchor
                        Any
                            list
                                list handle-retargs counter
                                    cons do
                                        list set-scope! eval-scope
                                        list __defer (list tmp)
                                            list _ (list get-scope) (list locals) tmp
                                        expr as list
                        false
                let f = (compile (eval (expr as Syntax) eval-scope))
                let fptr =
                    f as
                        pointer (function (ReturnLabel Scope i32))
                set-anchor! expr-anchor
                return (fptr)
            inline (exc)
                io-write!
                    format-exception exc
                return eval-scope (unconst 0)
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
    unreachable!;

fn print-version ()
    print
        compiler-version-string;
    print "Executable path:" compiler-path
    exit 0
    unreachable!;

fn test-compiler-version ()
    let vmin vmaj vpatch = (compiler-version)
    return vmin vmaj vpatch

fn run-main ()
    dump org
    let args = (launch-args)
    let exename args = ('decons args)
    let exename = (exename as string)
    let sourcepath = (alloca string)
    let parse-options = (alloca bool)
    store "" sourcepath
    store true parse-options
    let loop (args i) = args 1
    if (not (empty? args))
        let k = (i + 1)
        let arg args = ('decons args)
        let arg = (arg as string)
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
                unreachable!;
        elseif ((load sourcepath) == "")
            store arg sourcepath
            loop args k arg parse-options
        # remainder is passed on to script
    let sourcepath = (load sourcepath)
    if (sourcepath == "")
        #read-eval-print-loop;
    else
        let scope =
            Scope (globals)
        'set-symbol scope
            script-launch-args =
                Any
                    fn ()
                        return sourcepath args
        #load-module "" sourcepath
            scope = scope
            main-module? = true
        exit 0
        unreachable!;

run-main;
true

