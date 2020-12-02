#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""glm
    ===

    The `glm` module exports the basic vector and matrix types as well as
    related arithmetic operations which mimic the features available to shaders
    written in the GL shader language.

spice element-prefix (element-type)
    match (element-type as type)
    case bool "b"
    case i8 "i8"
    case i16 "i16"
    case i32 "i"
    case u8 "u8"
    case u16 "u16"
    case u32 "u"
    case f32 ""
    case f64 "d"
    default
        error "illegal element type"

spice element-one (element-type)
    match (element-type as type)
    case bool `true
    case i8 `1:i8
    case i16 `1:i16
    case i32 `1:i32
    case u8 `1:u8
    case u16 `1:u16
    case u32 `1:u32
    case f32 `1.0
    case f64 `1.0:f64
    default
        error "illegal element type"

run-stage;

typedef vec-type < immutable
typedef mat-type < immutable

@@ memo
inline construct-gvec-type (size)
    static-assert ((typeof size) == i32)
    static-assert (size > 1)
    typedef (.. "gvec" (tostring size)) < vec-type
        let Count = size

@@ memo
inline construct-vec-type (element-type size)
    let gvec-type = (construct-gvec-type size)
    let prefix = (element-prefix element-type)
    let size = gvec-type.Count
    let VT = (vector element-type size)
    typedef (.. prefix "vec" (tostring size)) < gvec-type : VT
        let ElementType = element-type
        let Zero = (nullof element-type)
        let One = (element-one element-type)
        let LiteralShuffleMask =
            bitcast
                vectorof element-type Zero One
                    va-map
                        inline () Zero
                        va-range (size - 2)
                this-type

inline construct-mat-type (element-type cols rows)
    static-assert ((typeof cols) == i32)
    static-assert ((typeof rows) == i32)
    assert (cols > 1)
    assert (rows > 1)
    let prefix = (element-prefix element-type)
    let vecT =
        construct-vec-type element-type rows
    let MT = (matrix vecT cols)
    typedef (.. prefix "mat" (tostring cols) "x" (tostring rows))
        \ < mat-type : MT
        let
            ElementType = element-type
            ColumnType = vecT
            RowType = (construct-vec-type element-type cols)
            Columns = cols; Rows = rows
        do
            static-if (cols == rows)
                'define-symbol this-type 'TransposedType this-type
            elseif (cols < rows)
                let TT = ((memo this-function) element-type rows cols)
                'define-symbol this-type 'TransposedType TT
                'define-symbol TT 'TransposedType this-type
let construct-mat-type = (memo construct-mat-type)

inline construct-vec-types (count)
    let count = (i32 count)
    _
        construct-vec-type f32 count
        construct-vec-type f64 count
        construct-vec-type i32 count
        construct-vec-type u32 count
        construct-vec-type bool count

inline construct-mat-types (cols rows)
    let cols = (i32 cols)
    let rows = (i32 rows)
    _
        construct-mat-type f32 cols rows
        construct-mat-type f64 cols rows
        construct-mat-type i32 cols rows
        construct-mat-type u32 cols rows
        construct-mat-type bool cols rows

let gvec2 = (construct-gvec-type 2)
let gvec3 = (construct-gvec-type 3)
let gvec4 = (construct-gvec-type 4)

let vec2 dvec2 ivec2 uvec2 bvec2 = (construct-vec-types 2)
let vec3 dvec3 ivec3 uvec3 bvec3 = (construct-vec-types 3)
let vec4 dvec4 ivec4 uvec4 bvec4 = (construct-vec-types 4)

let mat2x2 dmat2x2 imat2x2 umat2x2 bmat2x2 = (construct-mat-types 2 2)
let mat2x3 dmat2x3 imat2x3 umat2x3 bmat2x3 = (construct-mat-types 2 3)
let mat2x4 dmat2x4 imat2x4 umat2x4 bmat2x4 = (construct-mat-types 2 4)
let mat2 dmat2 imat2 umat2 bmat2 = mat2x2 dmat2x2 imat2x2 umat2x2 bmat2x2

let mat3x2 dmat3x2 imat3x2 umat3x2 bmat3x2 = (construct-mat-types 3 2)
let mat3x3 dmat3x3 imat3x3 umat3x3 bmat3x3 = (construct-mat-types 3 3)
let mat3x4 dmat3x4 imat3x4 umat3x4 bmat3x4 = (construct-mat-types 3 4)
let mat3 dmat3 imat3 umat3 bmat3 = mat3x3 dmat3x3 imat3x3 umat3x3 bmat3x3

let mat4x2 dmat4x2 imat4x2 umat4x2 bmat4x2 = (construct-mat-types 4 2)
let mat4x3 dmat4x3 imat4x3 umat4x3 bmat4x3 = (construct-mat-types 4 3)
let mat4x4 dmat4x4 imat4x4 umat4x4 bmat4x4 = (construct-mat-types 4 4)
let mat4 dmat4 imat4 umat4 bmat4 = mat4x4 dmat4x4 imat4x4 umat4x4 bmat4x4

#-------------------------------------------------------------------------------
# VECTORS
#-------------------------------------------------------------------------------

typedef vec-type-accessor
    do
        inline binary-op-dispatch (lop rop op)
            'set-symbol this-type lop
                inline (lhsT rhsT)
                    inline (lhs rhs) (op (imply lhs vec-type) rhs)
            'set-symbol this-type rop
                inline (lhsT rhsT)
                    inline (lhs rhs) (op lhs (imply rhs vec-type))

        binary-op-dispatch '__+ '__r+ +
        binary-op-dispatch '__- '__r- -
        binary-op-dispatch '__* '__r* *
        binary-op-dispatch '__** '__r** **
        binary-op-dispatch '__/ '__r/ /
        binary-op-dispatch '__// '__r// //
        binary-op-dispatch '__& '__r& &
        binary-op-dispatch '__| '__r| |
        binary-op-dispatch '__^ '__r^ ^
        binary-op-dispatch '__% '__r% %
        binary-op-dispatch '__> '__r> >
        binary-op-dispatch '__< '__r< <
        binary-op-dispatch '__>= '__r>= >=
        binary-op-dispatch '__<= '__r<= <=

    @@ spice-cast-macro
    fn __imply (vT T)
        let rhvecT = (('@ vT 'RHVectorType) as type)
        if ((T == rhvecT) or (T == vec-type))
            let mask = ('@ vT 'Mask)
            return `(inline (self) (bitcast (shufflevector self self mask) rhvecT))
        `()

    @@ spice-binary-op-macro
    fn __= (lhsT rhsT)
        raising Error
        inline sym-assign (lhs rhs rhvecT assignmask)
            assign (shufflevector lhs (imply rhs rhvecT) assignmask) lhs
        inline asym-assign (lhs rhs rhvecT assignmask expandmask)
            let rhs = (imply rhs rhvecT)
            # expand or contract
            let rhs = (shufflevector rhs rhs expandmask)
            assign (shufflevector lhs rhs assignmask) lhs

        let rhvecT = (('@ lhsT 'RHVectorType) as type)
        let assignmask = ('@ lhsT 'AssignMask)
        let lhsz = ('element-count lhsT)
        let sz = ('element-count rhvecT)
        let vecT = ('superof rhvecT)
        if (lhsz == sz)
            `(inline (lhs rhs) (sym-assign lhs rhs rhvecT assignmask))
        else
            let expandmask = ('@ lhsT 'ExpandMask)
            `(inline (lhs rhs) (asym-assign lhs rhs rhvecT assignmask expandmask))

fn vector-init-size-error (vecsz flatargsz)
    error
        .. "number of arguments (" (repr flatargsz)
            \ ") doesn't match number of elements (" (repr vecsz) ")"

typedef+ vec-type
    spice vec-constructor2 (self ...)
        let self = (self as type)
        let ET argsz =
            'element@ self 0; 'argcount ...
        # count sum of elements
        let flatargsz =
            fold (total = 0) for arg in ('args ...)
                let argT = ('typeof arg)
                + total
                    if (argT < vec-type)
                        'element-count argT
                    else 1
        let vecsz = ('element-count self)
        let initval = (sc_const_null_new self)
        if (flatargsz == 0)
            initval
        elseif (flatargsz == 1)
            # init all components with the same value
            let arg = ('getarg ... 0)
            let argT = ('typeof arg)
            let arg =
                if (argT < vec-type)
                    `(extractelement arg 0)
                else `(arg as ET)
            let smear = vector.smear
            `(bitcast (smear arg vecsz) self)
        elseif (flatargsz == vecsz)
            let values = (alloca-array Value vecsz)
            fold (total = 0) for arg in ('args ...)
                let argT = ('typeof arg)
                if (argT < vec-type)
                    let argET argvecsz =
                        'element@ argT 0; 'element-count argT
                    fold (total = total) for k in (range argvecsz)
                        values @ total = `(extractelement arg k)
                        total + 1
                else
                    values @ total = arg
                    total + 1
            let args = (sc_argument_list_new vecsz values)
            `(bitcast (vectorof ET args) self)
        else
            vector-init-size-error vecsz flatargsz

    spice __typecall (self ...)
        let self = (self as type)
        if (self == vec-type)
            `(construct-vec-type ...)
        else
            let args =
                sc_argument_list_map_new ('argcount ...)
                    inline (i)
                        let arg = ('getarg ... i)
                        let argT = ('typeof arg)
                        if (argT < vec-type) arg
                        elseif (argT < vec-type-accessor)
                            `(imply arg vec-type)
                        else arg
            `(vec-constructor2 self args)

    unlet vec-constructor2

    spice _vec-repr (self)
        let T = ('typeof self)
        let sz = ('element-count T)
        let s =
            fold (s = (spice-quote "<")) for i in (range sz)
                let txt = `(repr (extractelement self i))
                if (i == 0)
                    `(.. s txt)
                else
                    `(.. s " " txt)
        `(.. s ">")

    @@ spice-quote
    fn __repr (self) (_vec-repr self)

    unlet _vec-repr

    inline __@ (self i)
        extractelement self i

    inline __unpack (self)
        unpack (self as vector)

    inline __neg (self)
        - ((typeof self) 0) self

    inline __rcp (self)
        / ((typeof self) 1) self

    fn do-rimply (vT T static?)
        if (not ('opaque? T))
            let ET = ('element@ T 0)
            # can we cast vT to ET?
            let conv = (imply-converter vT ET static?)
            if (operator-valid? conv)
                let sz = ('element-count T)
                return
                    spice-quote
                        inline (value)
                            bitcast (vector.smear (conv value) sz) T
        `()

    @@ spice-cast-macro
    fn __static-rimply (vT T)
        do-rimply vT T true

    @@ spice-cast-macro
    fn __rimply (vT T)
        do-rimply vT T false

    unlet do-rimply

    do
        fn vec-type-binary-op (symbol lhsT rhsT)
            label next
                let Ta = ('element@ lhsT 0)
                let f =
                    try ('@ Ta symbol)
                    except (err) (merge next)
                if (lhsT == rhsT)
                    return f
            #
                let conv = (imply-converter rhsT Ta false)
                if (operator-valid? conv)
                    return `(inline (lhs rhs) (f lhs (lhsT (conv rhs))))
                let conv = (imply-converter rhsT Ta true)
                if (operator-valid? conv)
                    return `(inline (lhs rhs) (f lhs (lhsT (conv (verify-constant rhs)))))
            `()

        fn vec-type-binary-op-r (symbol lhsT rhsT)
            label next
                let Tb = ('element@ rhsT 0)
                let f =
                    try ('@ Tb symbol)
                    except (err) (merge next)
                if (lhsT == rhsT)
                    return f
            #
                let conv = (imply-converter lhsT Tb false)
                if (operator-valid? conv)
                    return `(inline (lhs rhs) (f (rhsT (conv lhs)) rhs))
                let conv = (imply-converter lhsT Tb true)
                if (operator-valid? conv)
                    return `(inline (lhs rhs) (f (rhsT (conv (verify-constant lhs))) rhs))
            `()

        inline vec-type-binary-op-dispatch (lop rop symbol)
            'set-symbol this-type lop
                spice-binary-op-macro
                    inline (lhsT rhsT) (vec-type-binary-op symbol lhsT rhsT)
            'set-symbol this-type rop
                spice-binary-op-macro
                    inline (lhsT rhsT) (vec-type-binary-op-r symbol lhsT rhsT)

        vec-type-binary-op-dispatch '__+ '__r+ '__vector+
        vec-type-binary-op-dispatch '__- '__r- '__vector-
        vec-type-binary-op-dispatch '__* '__r* '__vector*
        vec-type-binary-op-dispatch '__** '__r** '__vector**
        vec-type-binary-op-dispatch '__/ '__r/ '__vector/
        vec-type-binary-op-dispatch '__// '__r// '__vector//
        vec-type-binary-op-dispatch '__& '__r& '__vector&
        vec-type-binary-op-dispatch '__| '__r| '__vector|
        vec-type-binary-op-dispatch '__^ '__r^ '__vector^
        vec-type-binary-op-dispatch '__% '__r% '__vector%
        vec-type-binary-op-dispatch '__>> '__r>> '__vector>>
        vec-type-binary-op-dispatch '__<< '__r<< '__vector<<

        vec-type-binary-op-dispatch '__> '__r> '__vector>
        vec-type-binary-op-dispatch '__< '__r< '__vector<
        vec-type-binary-op-dispatch '__>= '__r>= '__vector>=
        vec-type-binary-op-dispatch '__<= '__r<= '__vector<=

    fn build-access-mask (name width)
        let element-set-xyzw = "^[xyzw01]{1,4}$"
        let element-set-rgba = "^[rgba01]{1,4}$"
        let element-set-stpq = "^[stpq01]{1,4}$"
        let element-set-any = "^([xyzw01]|[stpq01]|[rgba01]){1,4}$"

        let s = (name as string)
        let sz = ((countof s) as i32)
        if (sz > 4)
            error "too many characters in accessor (try 1 <= x <= 4)"
        let set =
            if ('match? element-set-xyzw s) "xyzw"
            elseif ('match? element-set-rgba s) "rgba"
            elseif ('match? element-set-stpq s) "stpq"
            else
                error "try one of xyzw | rgba | stpq"
        fn find-index (set width c)
            switch c
            pass 48:i8 # 0
            pass 49:i8 # 1
            do
                _ (width + c as i32 - 48) true
            default
                loop (k = 0)
                    let sc = (set @ (k as usize))
                    if (c == sc)
                        break k false
                    k + 1
        if (sz == 1)
            let k literal? = (find-index set width (s @ 0))
            return sz `k literal?
        # 2 - 4 arguments
        let entries = (alloca-array Value sz)
        local literals? = false
        for i in (range sz)
            let ui = (i as usize)
            let k literal? = (find-index set width (s @ ui))
            literals? |= literal?
            entries @ ui = `k
        let VT = (vector.type i32 sz)
        return sz (sc_const_aggregate_new VT sz entries) literals?

    @@ memoize
    fn expand-mask (lhsz rhsz)
        let entries = (alloca-array Value lhsz)
        for i in (range lhsz)
            let ui = (i as usize)
            let k = (i % rhsz)
            entries @ ui = `k
        let VT = (vector.type i32 lhsz)
        return (sc_const_aggregate_new VT lhsz entries)

    @@ memoize
    fn range-mask (sz)
        let entries = (alloca-array Value sz)
        for i in (range sz)
            let ui = (i as usize)
            entries @ ui = `i
        let VT = (vector.type i32 sz)
        return (sc_const_aggregate_new VT sz entries)

    @@ memoize
    fn assign-mask (lhsz mask)
        let sz = ('element-count ('typeof mask))
        let entries = (alloca-array Value lhsz)
        for i in (range lhsz)
            let ui = (i as usize)
            entries @ ui = `i
        for i in (range sz)
            let ui = ((sc_const_extract_at mask i) as i32 as usize)
            let k = (lhsz + i)
            entries @ ui = `k
        let VT = (vector.type i32 lhsz)
        return (sc_const_aggregate_new VT lhsz entries)

    @@ memoize
    fn construct-getter-type (vecrefT mask)
        let storageT = ('storageof vecrefT)
        let ET = ('element@ storageT 0)
        let sz = ('element-count ('typeof mask))
        let lhsz = ('element-count vecrefT)
        @@ spice-quote
        typedef [(.. ('string vecrefT) "." (tostring mask))]
            \ < vec-type-accessor : storageT
            let
                RHVectorType = (construct-vec-type ET sz)
                Mask = mask
                AssignMask = [(assign-mask lhsz mask)]
                ExpandMask = [(expand-mask lhsz sz)]

    spice __getattr (self name)
        let name = (name as Symbol as string)
        let QT = ('qualified-typeof self)
        let T = ('typeof self)
        let ETsz = ('element-count T)
        let sz mask literals? = (build-access-mask name ETsz)
        if (sz == 1)
            if literals?
                let mask = (mask as i32)
                let mask_zero = ETsz
                let mask_one = (ETsz + 1)
                if (mask == mask_zero)
                    '@ ('typeof self) 'Zero
                elseif (mask == mask_one)
                    '@ ('typeof self) 'One
                else
                    error "illegal literal mask"
            else
                `(extractelement self mask)
        elseif (('refer? QT) & (not literals?))
            spice-quote
                bitcast self
                    [(construct-getter-type T mask)]
        else
            let shmask = ('@ T 'LiteralShuffleMask)
            spice-quote
                bitcast
                    shufflevector self shmask mask
                    construct-vec-type [('element@ T 0)] sz

    unlet construct-getter-type assign-mask range-mask expand-mask
        \ build-access-mask

    @@ spice-cast-macro
    fn __as (vT T)
        inline vector-generator (self count)
            Generator
                inline () 0
                inline (i) (i < count)
                inline (i) (extractelement self i)
                inline (i) (i + 1)

        let ST = ('storageof vT)
        if ((T == vector) or (T == ST))
            return `(inline (self) (bitcast self ST))
        elseif (T == Generator)
            let count = ('element-count ST)
            return `(inline (self) (vector-generator self count))
        `()

    let __== =
        simple-binary-op
            inline (lhs rhs)
                all? (== (storagecast lhs) (storagecast rhs))

inline dot (u v)
    let w = (u * v)
    vector-reduce (do +) (w as (storageof (typeof w)))

#-------------------------------------------------------------------------------
# MATRICES
#-------------------------------------------------------------------------------

typedef+ mat-type

    inline __unpack (self)
        unpack (self as array)

    inline __@ (self index)
        extractvalue self index

    spice _mat-repr (self)
        let T = ('typeof self)
        let sz = (('@ T 'Columns) as i32)
        let s =
            fold (s = (spice-quote "[")) for i in (range sz)
                let txt = `('__repr (extractvalue self i))
                if (i == 0)
                    `(.. s txt)
                else
                    `(.. s " " txt)
        `(.. s "]")

    @@ spice-quote
    fn __repr (self) (_mat-repr self)

    unlet _mat-repr

    #inline empty-value (T)
        nullof T

    spice row (self i)
        let T = ('typeof self)
        let rowT = (('@ T 'RowType) as type)
        let cols = ('element-count T)
        fold (vec = `(nullof rowT)) for j in (range cols)
            `(insertelement vec (extractelement (extractvalue self j) i) j)

    fn make-diagonal-vector (VT i)
        let vec = `(nullof VT)
        if (i < ('element-count VT))
            let ET = ('element@ VT 0)
            `(insertelement vec (ET 1) i)
        else vec

    inline mat-type-constructor (element-type cols rows)
        construct-mat-type
            imply element-type type; imply cols i32; imply rows i32

    spice __typecall (cls ...)
        let cls = (cls as type)
        if (cls == mat-type)
            return `(mat-type-constructor ...)
        let cols = ('element-count cls)
        let VT = ('element@ cls 0)
        let ET = ('element@ VT 0)
        let rows = ('element-count VT)
        let argsz = ('argcount ...)
        if (argsz == 0)
            fold (self = `(nullof cls)) for i in (range cols)
                `(insertvalue self [(make-diagonal-vector VT i)] i)
        elseif (argsz == 1)
            # construct from scalar or matrix
            let arg = ('getarg ... 0)
            let argT = ('typeof arg)
            if (argT < mat-type)
                # construct from matrix
                if (argT == cls)
                    # same matrix type, just return the argument
                    arg
                else
                    # build a matrix that is bigger or smaller
                    let can-copy-vectors? = (('element@ argT 0) == VT)
                    let argcols = ('element-count argT)
                    let argrows = ('element-count ('element@ argT 0))
                    let minrows = (min rows argrows)
                    fold (self = `(nullof cls)) for i in (range cols)
                        if (i < argcols)
                            if can-copy-vectors?
                                `(insertvalue self (extractvalue arg i) i)
                            else
                                let argvec = `(extractvalue arg i)
                                let vec = (make-diagonal-vector VT i)
                                let vec =
                                    # element-wise construction
                                    # start off with default diagonal vector
                                    fold (vec = vec) for j in (range minrows)
                                        `(insertelement vec
                                            ((extractelement argvec j) as ET) j)
                                `(insertvalue self vec i)
                        else
                            # build default diagonal vector
                            `(insertvalue self [(make-diagonal-vector VT i)] i)
            elseif (argT < vec-type)
                error
                    .. (repr (i32 cols)) " column vectors required"
            else
                # build a matrix with diagonal elements set to arg
                let arg = `(arg as ET)
                let emptyVT = `(nullof VT)
                fold (self = `(nullof cls)) for i in (range cols)
                    `(insertvalue self (insertelement emptyVT arg i) i)
        else
            # construct from arbitrary composition of vectors and elements,
                which must nevertheless align to vector boundary size
            # unpack all elements and count offsets as we go
            let emptyVT = `(nullof VT)
            let self vec col row =
                fold (self vec col row = `(nullof cls) emptyVT 0 0)
                    \ for arg in ('args ...)

                    let argT = ('typeof arg)
                    let is-vector? = (argT < vec-type)
                    let nextrow =
                        + row
                            if is-vector? ('element-count argT)
                            else 1
                    if (nextrow > rows)
                        error "too many arguments for column"
                    let vec =
                        if is-vector?
                            if (argT == VT) arg # same vector type
                            else # convert and insert values by element
                                let vsz = ('element-count argT)
                                fold (vec = vec) for j in (range vsz)
                                    let value = `(extractelement arg j)
                                    `(insertelement vec (value as ET) [(row + j)])
                        else # assume element type
                            `(insertelement vec (arg as ET) row)
                    if (nextrow == rows) # end of column
                        _ `(insertvalue self vec col) emptyVT (col + 1) 0
                    else # not arrived yet
                        _ self vec col nextrow
            if (row != 0)
                error
                    .. "number of provided elements for last row (" (repr (i32 row))
                        \ ") doesn't match number of elements required (" (repr (i32 rows)) ")"
            if (col != cols)
                error
                    .. "number of provided columns (" (repr (i32 col))
                        \ ") doesn't match number of columns required (" (repr (i32 cols)) ")"
            self

    unlet make-diagonal-vector mat-type-constructor

    @@ spice-binary-op-macro
    fn __* (lhsT rhsT)
        let mat-row = row
        if (and
                (lhsT < mat-type)
                (rhsT < mat-type)
                ((('@ rhsT 'Rows) as i32) == (('@ lhsT 'Columns) as i32)))
            # column type of lhsT
            let VT = ('element@ lhsT 0)
            let ET = ('element@ VT 0)
            let
                dest-columns = (('@ rhsT 'Columns) as i32)
                dest-rows = (('@ lhsT 'Rows) as i32)
            let destT = `(construct-mat-type ET dest-columns dest-rows)
            spice-quote
                inline (lhs rhs)
                    spice-unquote
                        # because we know the destination has the same number of rows as lhs
                        let rows = (alloca-array Value dest-rows)
                        for i in (range dest-rows)
                            rows @ i = `(mat-row lhs i)
                        # a row has dest-columns elements, so we iterate on that
                        fold (mat = `(nullof destT)) for i in (range dest-columns)
                            let vec =
                                fold (vec = `(nullof VT)) for j in (range dest-rows)
                                    let row = (deref (rows @ j))
                                    `(insertelement vec (dot row (extractvalue rhs i)) j)
                            `(insertvalue mat vec i)
        elseif (rhsT == (('@ lhsT 'RowType) as type))
            let VT = ('element@ lhsT 0)
            let sz = ('element-count VT)
            # mat(i,j) * vec(i) -> vec(j)
            spice-quote
                inline (lhs rhs)
                    spice-unquote
                        fold (vec = `(nullof VT)) for i in (range sz)
                            `(insertelement vec (dot (mat-row lhs i) rhs) i)
        else
            `()

    @@ spice-binary-op-macro
    fn __r* (lhsT rhsT)
        if (lhsT == ('element@ rhsT 0))
            # vec(j) * mat(i,j) -> vec(i)
            let sz = ('element-count rhsT)
            spice-quote
                inline (lhs rhs)
                    spice-unquote
                        fold (vec = `(nullof rhsT.RowType)) for i in (range sz)
                            `(insertelement vec (dot lhs (extractvalue rhs i)) i)
        else
            `()

    @@ spice-cast-macro
    fn __as (vT T)
        inline matrix-generator (self count)
            Generator
                inline () 0
                inline (i) (i < count)
                inline (i) (extractvalue self i)
                inline (i) (i + 1)

        let ST = ('storageof vT)
        if ((T == array) or (T == ST))
            return `(inline (self) (bitcast self ST))
        elseif (T == Generator)
            let count = ('element-count ST)
            return `(inline (self) (matrix-generator self count))
        `()

    @@ spice-binary-op-macro
    fn __== (lhsT rhsT)
        if (lhsT == rhsT)
            let cols = ('element-count lhsT)
            let VT = (vector.type bool cols)
            spice-quote
                inline (lhs rhs)
                    all?
                        spice-unquote
                            fold (vec = `(nullof VT)) for i in (range cols)
                                let cmp =
                                    `((extractvalue lhs i) == (extractvalue rhs i))
                                `(insertelement vec cmp i)
        else
            return `()

spice _transpose (m)
    let T = ('typeof m)
    assert (T < mat-type)
    let TT = (('@ T 'TransposedType) as type)
    fold (self = `(nullof TT)) for i in (range (('@ T 'Rows) as i32))
        `(insertvalue self ('row m i) i)

@@ spice-quote
inline transpose (m)
    _transpose m

spice mix (a b x)
    let Ta = ('typeof a)
    let Tx = ('typeof x)
    if ((Tx == bool) or ((Tx < vec-type) and (('element@ Tx 0) == bool)))
        `(? x b a)
    else
        let x =
            if (Tx < vec-type) x
            else `(Ta x)
        `(fmix a b x)

#-------------------------------------------------------------------------------

#if main-module?
    assert
        60.0 ==
            dot
                vec4 1 2 3 4
                vec4 4 5 6 7

do
    let vec-type mat-type
    let gvec2 gvec3 gvec4

    let vec2 dvec2 ivec2 uvec2 bvec2
    let vec3 dvec3 ivec3 uvec3 bvec3
    let vec4 dvec4 ivec4 uvec4 bvec4

    let mat2x2 dmat2x2 imat2x2 umat2x2 bmat2x2
    let mat2x3 dmat2x3 imat2x3 umat2x3 bmat2x3
    let mat2x4 dmat2x4 imat2x4 umat2x4 bmat2x4
    let mat2 dmat2 imat2 umat2 bmat2

    let mat3x2 dmat3x2 imat3x2 umat3x2 bmat3x2
    let mat3x3 dmat3x3 imat3x3 umat3x3 bmat3x3
    let mat3x4 dmat3x4 imat3x4 umat3x4 bmat3x4
    let mat3 dmat3 imat3 umat3 bmat3

    let mat4x2 dmat4x2 imat4x2 umat4x2 bmat4x2
    let mat4x3 dmat4x3 imat4x3 umat4x3 bmat4x3
    let mat4x4 dmat4x4 imat4x4 umat4x4 bmat4x4
    let mat4 dmat4 imat4 umat4 bmat4

    let dot transpose mix
    locals;
