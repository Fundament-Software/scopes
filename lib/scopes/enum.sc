#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""Enum
    ====

    support for defining tagged unions and classical enums via enum sugar

# tagged union / sum type
typedef Enum
    spice __dispatch (self handlers...)
        let qcls = ('qualified-typeof self)
        let cls = ('strip-qualifiers qcls)
        let payload-cls = ('element@ cls 1)
        let fields = (('@ cls '__fields) as type)
        let field-types = ('@ cls '__fields__)
        let field-type-args = ('args field-types)
        let expression =
        let tag = `(extractvalue self 0)
        let sw = (sc_switch_new tag)
        let refer = ('refer? qcls)
        let ptrT = ('refer->pointer-type qcls)
        for arg in ('args handlers...)
            let key arg = ('dekey arg)
            if (key == unnamed)
                sc_switch_append_default sw `(arg)
            else
                let i = (sc_type_field_index fields key)
                let ET = ('key-type ('element@ fields i) unnamed)
                let field = (('getarg field-types i) as type)
                let lit = ('@ field 'Literal)
                let payload = `(extractvalue self 1)
                let extracted =
                    if refer
                        let ptrET = ('change-element-type ptrT ET)
                        spice-quote
                            let ptr = (reftoptr payload)
                            let ptr = (bitcast ptr ptrET)
                            ptrtoref ptr
                    else
                        let ptrET = ('change-storage-class
                            ('mutable (pointer.type ET)) 'Function)
                        spice-quote
                            let ptr = (alloca payload-cls)
                            store payload ptr
                            let ptr = (bitcast ptr ptrET)
                            load ptr
                let ET = ('strip-qualifiers ET)
                let extracted =
                    if (ET < tuple)
                        `(unpack extracted)
                    else extracted
                sc_switch_append_case sw lit `(arg extracted)
        sw

fn define-field-runtime (T name field-type index-value)
    let fields = ('@ T '__fields__)
    let index = (('@ T '__index__) as u64)
    let index-anchor = ('anchor index-value)
    let index-value =
        if (('typeof index-value) == Nothing) index
        else (extract-integer index-value)
    let next-index = (index-value + 1)
    let FT = (typename.type
        (.. "enum-field<" (name as Symbol as string) ":"
            (tostring (field-type as type)) "=" (tostring index-value) ">")
        typename)
    'set-opaque FT
    'set-symbols FT
        Name = name
        Type = field-type
        Index = index-value
    let FT = `FT
    let fields =
        sc_argument_list_join_values fields FT
    sc_type_set_symbol T '__fields__ fields
    sc_type_set_symbol T '__index__ next-index
    index-value

spice define-field (enum-type name opts...)
    if (not ('constant? enum-type))
        error "enum-type must be constant"
    let T = (enum-type as type)
    let argc = ('argcount opts...)
    let field-type index-value =
        if (argc == 0) (_ `Nothing `none)
        elseif (argc == 1) (_ ('getarg opts... 0) `none)
        else (_ ('getarg opts... 0) ('getarglist opts... 1))
    define-field-runtime T name field-type index-value

fn shabbysort (buf sz)
    for i in (range sz)
        for j in (range (i + 1) sz)
            let a = (load (getelementptr buf i))
            let ak = (extractvalue a 0)
            let b = (load (getelementptr buf j))
            let bk = (extractvalue b 0)
            if (ak > bk)
                # swap
                store b (getelementptr buf i)
                store a (getelementptr buf j)

fn build-repr-switch-case (litT self field-types allow-dupes?)
    let sw = (sc_switch_new self)
    let numfields = ('argcount field-types)
    let sorted = (alloca-array (tuple u64 type) numfields)
    for i in (range numfields)
        let field = (('getarg field-types i) as type)
        let field = (field as type)
        let index = (('@ field 'Index) as u64)
        store (tupleof index field) (getelementptr sorted i)
    # sort array so duplicates are next to each other and merge names
    shabbysort sorted numfields
    loop (i = 0)
        if (i == numfields)
            break;
        let index field = (unpack (load (getelementptr sorted i)))
        let lit = (sc_const_int_new litT index)
        let name = (('@ field 'Name) as Symbol as string)
        # accumulate all fields with the same index
        let i name =
            loop (i name = (i + 1) name)
                if (i == numfields)
                    break i name
                let index2 field2 = (unpack (load (getelementptr sorted i)))
                if (index2 == index)
                    if (not allow-dupes?)
                        error "duplicate tags not permitted for tagged unions"
                    let name2 = (('@ field2 'Name) as Symbol as string)
                    repeat (i + 1) (.. name "|" name2)
                else
                    break i name
        let name =
            sc_default_styler style-number name
        sc_switch_append_case sw lit name
        i
    sc_switch_append_default sw "?invalid?"
    sw

inline tag-constructor (
    enum-type index-value payload-type field-type field-pointer-type ...)
    let payload = (alloca payload-type)
    let PT = (typeof payload)
    let destptr = (bitcast payload field-pointer-type)
    let payload = (field-type ...)
    store payload destptr
    let payload = (load (bitcast destptr PT))
    let value = (undef enum-type)
    let value = (insertvalue value index-value 0)
    insertvalue value payload 1

fn finalize-enum-runtime (T storage)
    let field-types = ('@ T '__fields__)
    let field-type-args = ('args field-types)
    # figure out integer bit width and signedness for the enumerator
    let width signed =
        fold (width signed = 0 false) for field in field-type-args
            let field = (field as type)
            let index = (('@ field 'Index) as u64 as i64)
            let signed = (signed | (index < 0))
            let iwidth =
                if signed
                    if ((index >= -0x80) and (index < 0x80)) 8
                    elseif ((index >= -0x8000) and (index < 0x8000)) 16
                    elseif ((index >= -0x80000000) and (index < 0x80000000)) 32
                    else 64
                else
                    if (index <= 1) 1
                    if (index <= 0xff) 8
                    elseif (index <= 0xffff) 16
                    elseif (index <= 0xffffffff) 32
                    else 64
            _ (max width iwidth) signed
    let classic? =
        if (T < CEnum) true
        elseif (T < Enum) false
        else
            error
                .. "type " (repr T) " must have Enum or CEnum supertype"
                    \ " but has supertype " (repr ('superof T))
    let using-scope = (Scope)
    'set-symbol T '__using using-scope
    if classic?
        # no storage definition means `plain` was used.
        let storage =
            if (('typeof storage) == type)
                storage as type
            else
                i32
        if (not (storage < integer))
            error "enum storage must be of integer type."
        'set-plain-storage T storage
        # enum is integer
        for field in field-type-args
            let field = (field as type)
            let name = (('@ field 'Name) as Symbol)
            let index = (('@ field 'Index) as u64)
            let field-type = (('@ field 'Type) as type)
            if (field-type != Nothing)
                error "plain enums can't have tagged fields"
            let value = (sc_const_int_new T index)
            'set-symbol T name value
            'bind using-scope name value
        # build repr function
        spice-quote
            inline __repr (self)
                spice-unquote
                    build-repr-switch-case T self field-types true
        'set-symbol T '__repr __repr
    else
        let index-type = (sc_integer_type width signed)
        # build repr function
        spice-quote
            inline __repr (self)
                let val = (extractvalue self 0)
                spice-unquote
                    build-repr-switch-case index-type val field-types false
        # build match function
        spice-quote
        'set-symbols T
            __repr = __repr
        let numfields = ('argcount field-types)
        let fields = (alloca-array type numfields)
        for i in (range numfields)
            let field = (('getarg field-types i) as type)
            let name = (('@ field 'Name) as Symbol)
            let field-type = (('@ field 'Type) as type)
            let field-type = ('key-type field-type name)
            store field-type (getelementptr fields i)
        'set-symbols T
            __fields = (sc_tuple_type numfields fields)
        let payload-type = (sc_union_storage_type numfields fields)
        'set-storage T
            tuple.type index-type payload-type
        let consts = (alloca-array Value 2)
        # build value constructors
        for i _field in (enumerate field-type-args)
            let field = (_field as type)
            let name = (('@ field 'Name) as Symbol)
            let index = (('@ field 'Index) as u64)
            let field-type = (('@ field 'Type) as type)
            let index-value = (sc_const_int_new index-type index)
            'set-symbol field 'Literal index-value
            let value =
                if (field-type == Nothing)
                    # can provide a constant
                    store index-value (getelementptr consts 0)
                    store (sc_const_null_new payload-type) (getelementptr consts 1)
                    sc_const_aggregate_new T 2 consts
                else
                    # must provide a constructor
                    let TT = ('change-storage-class
                            ('mutable (pointer.type field-type)) 'Function)
                    spice-quote
                        inline constructor (...)
                            tag-constructor T index-value payload-type field-type TT ...
                    sc_template_set_name constructor name
                    constructor
            'set-symbol T name value
            'bind using-scope name value


spice finalize-enum (T storage)

    if ('constant? T)
        finalize-enum-runtime (T as type) storage
        `()
    else
        `(finalize-enum-runtime T storage)

sugar enum (name body...)

    let supertype body has-supertype? storage =
        sugar-match body...
        case ('plain rest...)
            _ `CEnum rest... true `none
        case (': storage rest...)
            _ `CEnum rest... true `storage
        case ('< supertype rest...)
            _ supertype rest... true `none
        default
            _ `Enum body... false `none

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

    # detect and rewrite top level field forms
    let body =
        loop (result body = '() body)
            if (empty? body)
                break ('reverse result)
            let expr next = (decons body)
            let anchor = ('anchor expr)
            let this-type =
                'tag `'this-type anchor
            let exprT = ('typeof expr)
            let expr =
                if (exprT == Symbol)
                    let newexpr =
                        qq [let] [expr] =
                            [define-field] [this-type] '[expr] [Nothing]
                    `newexpr
                elseif (exprT == list)
                    sugar-match (expr as list)
                    case ((name is Symbol) ': T...)
                        let newexpr =
                            qq [let] [name] =
                                [define-field] [this-type] '[name] ([tuple] (unquote-splice T...))
                        `newexpr
                    case ((name is Symbol) '= index...)
                        let newexpr =
                            qq [let] [name] =
                                [define-field] [this-type] '[name] [Nothing]
                                    unquote-splice index...
                        `newexpr
                    default expr
                else expr
            repeat (cons expr result) next

    spice init-fields (enum-type)
        fn init-fields-runtime (T)
            sc_type_set_symbol T '__fields__ (sc_argument_list_new 0 null)
            sc_type_set_symbol T '__index__ (sc_const_int_new u64 0:u64)

        if ('constant? enum-type)
            init-fields-runtime (enum-type as type)
            `()
        else
            `(init-fields-runtime enum-type)
    qq
        unquote-splice
            if has-fwd-decl
                if has-supertype?
                    hide-traceback;
                    error "completing enum declaration must not define a supertype"
                qq [typedef+] [name]
            else
                qq [typedef] [name] < [supertype] do
        [init-fields] this-type
        inline tag (...)
            [define-field] this-type ...
        [do]
            unquote-splice body
            [fold-locals] this-type [append-to-type]
        [finalize-enum] this-type [storage]
        this-type

sugar dispatch (value)
    loop (next outp = next-expr '())
        sugar-match next
        case (('case (name is Symbol) (args...) body...) rest...)
            repeat rest...
                cons
                    qq [name] =
                        [inline] "#hidden" [args...] (unquote-splice body...)
                    outp
        case (('default body...) rest...)
            return
                qq '__dispatch [value] (unquote-splice ('reverse outp))
                    [inline] "#hidden" () (unquote-splice body...)
                rest...
        default
            error "missing default case"

do
    let enum dispatch Enum
    locals;
