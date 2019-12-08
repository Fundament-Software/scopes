#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""struct
    ======

    Support for defining structs through the `struct` sugar.

sugar struct (name body...)
    fn define-field-runtime (T anchor name field-type default-value)
        let fields = ('@ T '__fields__)
        let default-anchor = ('anchor default-value)
        let field-type = (field-type as type)
        let field-type default-value =
            if (field-type == Unknown)
                let value = (sc_prove default-value)
                _ ('typeof value) value
            elseif (('typeof default-value) != Nothing)
                let value = (sc_prove ('tag
                    `(imply default-value field-type) default-anchor))
                _ field-type value
            else
                _ field-type default-value
        if (not ('constant? default-value))
            error@ default-anchor "while checking default initializer"
                "initializer must be constant"
        let FT = (typename.type
            (.. "struct-field<" (name as Symbol as string) ":"
                (tostring (field-type as type)) ">")
            typename)
        'set-opaque FT
        'set-symbol FT 'Type
            sc_key_type (name as Symbol) (field-type as type)
        if (('typeof default-value) != Nothing)
            'set-symbol FT 'Default default-value
        let FT = ('tag `FT anchor)
        let fields =
            sc_argument_list_join_values fields FT
        sc_type_set_symbol T '__fields__ fields
        FT

    spice define-field (struct-type name field-type default-value...)
        if (not ('constant? struct-type))
            error "struct-type must be constant"
        let anchor = ('anchor struct-type)
        let T = (struct-type as type)
        let default-value =
            if (('argcount default-value...) == 0) `none
            else default-value...
        define-field-runtime T anchor name field-type default-value

    spice finalize-struct (T packed?)
        fn finalize-struct-runtime (T packed?)
            let field-types = ('@ T '__fields__)
            let numfields = ('argcount field-types)
            let fields = (alloca-array type numfields)
            for i field in (enumerate ('args field-types))
                let field = (field as type)
                let field = (('@ field 'Type) as type)
                (ptrtoref (getelementptr fields i)) = field
            if (T < CUnion)
                if packed?
                    error "unions can't be packed"
                'set-symbol T '__fields
                    sc_tuple_type numfields fields
                'set-plain-storage T
                    sc_union_storage_type numfields fields
            elseif (T < CStruct)
                'set-plain-storage T
                    if packed?
                        sc_packed_tuple_type numfields fields
                    else
                        sc_tuple_type numfields fields
            elseif (T < Struct)
                'set-storage T
                    if packed?
                        sc_packed_tuple_type numfields fields
                    else
                        sc_tuple_type numfields fields
            else
                error
                    .. "type " (repr T) " must have Struct, CStruct or CUnion supertype"
                        \ " but has supertype " (repr ('superof T))
        if ('constant? T)
            finalize-struct-runtime (T as type) (packed? as bool)
            `()
        else
            `(finalize-struct-runtime T packed?)

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
            try ('@ sugar-scope symname) true
            except (err) false
        else false

    let packed? body =
        sugar-match body
        case ('packed rest...)
            _ true rest...
        default
            _ false body

    # detect and rewrite top level field forms
    let body =
        loop (result body = '() body)
            if (empty? body)
                break ('reverse result)
            let expr next = (decons body)
            let expr =
                if (('typeof expr) == list)
                    let anchor = ('anchor expr)
                    sugar-match (expr as list)
                    case ((name as Symbol) ': T)
                        let newexpr =
                            qq [define-field] [('tag `'this-type anchor)] '[name] [T]
                        `newexpr
                    case ((name as Symbol) ': T '= default...)
                        let newexpr =
                            qq [define-field] [('tag `'this-type anchor)] '[name] [T]
                                unquote-splice default...
                        `newexpr
                    case ((name as Symbol) '= default...)
                        let newexpr =
                            qq [define-field] [('tag `'this-type anchor)] '[name] [Unknown]
                                unquote-splice default...
                        `newexpr
                    default expr
                else expr
            repeat (cons expr result) next

    spice init-fields (struct-type)
        fn init-fields-runtime (T)
            sc_type_set_symbol T '__fields__ (sc_argument_list_new 0 null)

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
        inline field (...)
            [define-field] this-type ...
        [do]
            unquote-splice body
            [fold-locals] this-type [append-to-type]
        [finalize-struct] this-type [packed?]
        this-type

do
    let struct
    locals;
