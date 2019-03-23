#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""glm
    ===

    The `glm` module exports the basic vector and matrix types as well as
    related arithmetic operations which mimic the features available to shaders
    written in the GL shader language.

typedef vec-type
typedef mat-type
typedef vec-type-accessor

#run-stage;

fn element-prefix (element-type)
    match element-type
    case bool "b"
    case i32 "i"
    case u32 "u"
    case f32 ""
    case f64 "d"
    default
        compiler-error! "illegal element type"

@@ type-factory
fn construct-vec-type (element-type size)
    assert ((typeof size) == i32)
    assert (size > 1)
    let prefix = (element-prefix element-type)
    let VT = (vector element-type size)
    typedef (.. prefix "vec" (tostring size)) < vec-type : VT
        'set-symbols this-type
            ElementType = element-type
            Count = size

@@ type-factory
fn construct-mat-type (element-type cols rows)
    if false # recursive function, hint return type
        return type
    assert ((typeof cols) == i32)
    assert ((typeof rows) == i32)
    assert (cols > 1)
    assert (rows > 1)
    let prefix = (element-prefix element-type)
    let vecT =
        construct-vec-type element-type rows
    let MT = (array vecT cols)
    typedef (.. prefix "mat" (tostring cols) "x" (tostring rows))
        \ < mat-type : MT
        'set-symbols this-type
            ElementType = element-type
            ColumnType = vecT
            RowType = (construct-vec-type element-type cols)
            Columns = cols; Rows = rows
        if (cols == rows)
            'set-symbol this-type 'TransposedType this-type
        elseif (cols < rows)
            let TT = ((type-factory construct-mat-type) element-type rows cols)
            'set-symbol this-type 'TransposedType TT
            'set-symbol TT 'TransposedType this-type

fn construct-vec-types (count)
    let count = (i32 count)
    return
        construct-vec-type f32 count
        construct-vec-type f64 count
        construct-vec-type i32 count
        construct-vec-type u32 count
        construct-vec-type bool count

fn construct-mat-types (cols rows)
    let cols = (i32 cols)
    let rows = (i32 rows)
    return
        construct-mat-type f32 cols rows
        construct-mat-type f64 cols rows
        construct-mat-type i32 cols rows
        construct-mat-type u32 cols rows
        construct-mat-type bool cols rows

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
        fn binary-op-expr (op lhsT rhsT lhs rhs)
            raises-compile-error;
            `(op (imply lhs vec-type) rhs)

        fn binary-op-expr-r (op lhsT rhsT lhs rhs)
            raises-compile-error;
            `(op lhs (imply rhs vec-type))

        inline binary-op-dispatch (lop rop op)
            'set-symbol this-type lop
                box-binary-op
                    fn (lhsT rhsT lhs rhs)
                        binary-op-expr op lhsT rhsT lhs rhs
            'set-symbol this-type rop
                box-binary-op
                    fn (lhsT rhsT lhs rhs)
                        binary-op-expr-r op lhsT rhsT lhs rhs

        binary-op-dispatch '__+ '__r+ +
        binary-op-dispatch '__- '__r- -
        binary-op-dispatch '__* '__r* *
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

    @@ box-cast
    fn __imply (vT T self)
        let rhvecT = (vT.RHVectorType as type)
        if ((T == rhvecT) or (T == vec-type))
            let mask = vT.Mask
            return `(bitcast (shufflevector self self mask) rhvecT)
        compiler-error! "unsupported type"

    @@ box-binary-op
    fn __= (lhsT rhsT lhs rhs)
        raises-compile-error;
        let rhvecT = (lhsT.RHVectorType as type)
        let assignmask = lhsT.AssignMask
        let lhsz = ('element-count lhsT)
        let sz = ('element-count rhvecT)
        let vecT = ('superof rhvecT)
        if (lhsz == sz)
            `(assign
                (shufflevector lhs (imply rhs rhvecT) assignmask)
                lhs)
        else
            let expandmask = lhsT.ExpandMask
            spice-quote
                do
                    let rhs = (imply rhs rhvecT)
                    # expand or contract
                    let rhs = (shufflevector rhs rhs expandmask)
                    assign (shufflevector lhs rhs assignmask) lhs


typedef vec-type < immutable
    inline vec-type-constructor (element-type size)
        construct-vec-type (imply element-type type) (imply size i32)

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
        let initval = `(nullof self)
        if (flatargsz == 1)
            # init all components with the same value
            let arg = ('getarg ... 0)
            let argT = ('typeof arg)
            let arg =
                if (argT < vec-type)
                    `(extractelement arg 0)
                else `(arg as ET)
            loop (i value = 0 initval)
                if (i == vecsz)
                    break value
                repeat (i + 1)
                    `(insertelement value arg i)
        elseif (flatargsz == vecsz)
            let total value =
                fold (total value = 0 initval) for arg in ('args ...)
                    let argT = ('typeof arg)
                    if (argT < vec-type)
                        let argET argvecsz =
                            'element@ argT 0; 'element-count argT
                        fold (total value = total value) for k in (range argvecsz)
                            _ (total + 1)
                                `(insertelement value (extractelement arg k) total)
                    else
                        _ (total + 1)
                            `(insertelement value (arg as ET) total)
            value
        else
            compiler-error!
                .. "number of arguments (" (repr flatargsz)
                    \ ") doesn't match number of elements (" (repr vecsz) ")"

    spice __typecall (self ...)
        let self = (self as type)
        if (self == vec-type)
            `(vec-type-constructor ...)
        else
            let args = (sc_argument_list_new (active-anchor))
            for arg in ('args ...)
                let argT = ('typeof arg)
                sc_argument_list_append args
                    if (argT < vec-type) arg
                    elseif (argT < vec-type-accessor)
                        `(imply arg vec-type)
                    else arg
            `(vec-constructor2 self args)

    unlet vec-type-constructor vec-constructor2

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

    do
        fn vec-type-binary-op-expr (symbol lhsT rhsT lhs rhs)
            let Ta = ('element@ lhsT 0)
            let f =
                try ('@ Ta symbol)
                except (err)
                    compiler-error! "unsupported operation"
            let f = (unbox-binary-op-function-type f)

            let rhs =
                if (lhsT == rhsT) rhs
                else
                    `(lhsT [(imply-expr rhsT Ta rhs)])
            return (f lhsT lhsT lhs rhs)

        fn vec-type-binary-op-expr-r (symbol lhsT rhsT lhs rhs)
            let Tb = ('element@ rhsT 0)
            let f =
                try ('@ Tb symbol)
                except (err)
                    compiler-error! "unsupported operation"
            let f = (unbox-binary-op-function-type f)
            let lhs =
                if (lhsT == rhsT) lhs
                else
                    `(rhsT [(imply-expr lhsT Tb lhs)])
            return (f rhsT rhsT lhs rhs)

        inline vec-type-binary-op-dispatch (lop rop symbol)
            'set-symbol this-type lop
                box-binary-op
                    fn (lhsT rhsT lhs rhs) (vec-type-binary-op-expr symbol lhsT rhsT lhs rhs)
            'set-symbol this-type rop
                box-binary-op
                    fn (lhsT rhsT lhs rhs) (vec-type-binary-op-expr-r symbol lhsT rhsT lhs rhs)

        vec-type-binary-op-dispatch '__+ '__r+ '__vector+
        vec-type-binary-op-dispatch '__- '__r- '__vector-
        vec-type-binary-op-dispatch '__* '__r* '__vector*
        vec-type-binary-op-dispatch '__/ '__r/ '__vector/
        vec-type-binary-op-dispatch '__// '__r// '__vector//
        vec-type-binary-op-dispatch '__& '__r& '__vector&
        vec-type-binary-op-dispatch '__| '__r| '__vector|
        vec-type-binary-op-dispatch '__^ '__r^ '__vector^
        vec-type-binary-op-dispatch '__% '__r% '__vector%

        vec-type-binary-op-dispatch '__> '__r> '__vector>
        vec-type-binary-op-dispatch '__< '__r< '__vector<
        vec-type-binary-op-dispatch '__>= '__r>= '__vector>=
        vec-type-binary-op-dispatch '__<= '__r<= '__vector<=

    fn build-access-mask (name)
        let element-set-xyzw = "^[xyzw]{1,4}$"
        let element-set-rgba = "^[rgba]{1,4}$"
        let element-set-stpq = "^[stpq]{1,4}$"
        let element-set-any = "^([xyzw]|[stpq]|[rgba]){1,4}$"

        let s = (name as string)
        let sz = ((countof s) as i32)
        if (sz > 4)
            compiler-error! "too many characters in accessor (try 1 <= x <= 4)"
        let set =
            if ('match? element-set-xyzw s) "xyzw"
            elseif ('match? element-set-rgba s) "rgba"
            elseif ('match? element-set-stpq s) "stpq"
            else
                compiler-error! "try one of xyzw | rgba | stpq"
        fn find-index (set c)
            loop (k = 0)
                let sc = (set @ (k as usize))
                if (c == sc)
                    break k
                k + 1
        if (sz == 1)
            return sz `[(find-index set (s @ 0))]
        # 2 - 4 arguments
        let entries = (alloca-array Value sz)
        for i in (range sz)
            let ui = (i as usize)
            let k = (find-index set (s @ ui))
            entries @ ui = `k
        let VT = (vector i32 sz)
        return sz (sc_const_aggregate_new (active-anchor) VT sz entries)

    @@ memoize
    fn expand-mask (lhsz rhsz)
        let entries = (alloca-array Value lhsz)
        for i in (range lhsz)
            let ui = (i as usize)
            let k = (i % rhsz)
            entries @ ui = `k
        let VT = (vector i32 lhsz)
        return (sc_const_aggregate_new (active-anchor) VT lhsz entries)

    @@ memoize
    fn range-mask (sz)
        let entries = (alloca-array Value sz)
        for i in (range sz)
            let ui = (i as usize)
            entries @ ui = `i
        let VT = (vector i32 sz)
        return (sc_const_aggregate_new (active-anchor) VT sz entries)

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
        let VT = (vector i32 lhsz)
        return (sc_const_aggregate_new (active-anchor) VT lhsz entries)

    @@ type-factory
    fn construct-getter-type (vecrefT mask)
        let storageT = ('storageof vecrefT)
        let ET = ('element@ storageT 0)
        typedef (.. ('string vecrefT) (tostring mask)) < vec-type-accessor : storageT
            let sz = ('element-count ('typeof mask))
            let lhsz = ('element-count vecrefT)
            'set-symbols this-type
                RHVectorType = (construct-vec-type ET sz)
                Mask = mask
                AssignMask = (assign-mask lhsz mask)
                ExpandMask = (expand-mask lhsz sz)

    spice __getattr (self name)
        let name = (name as Symbol as string)
        let sz mask = (build-access-mask name)
        let QT = ('qualified-typeof self)
        if (sz == 1)
            `(extractelement self mask)
        elseif ('refer? QT)
            spice-quote
                bitcast self
                    [(construct-getter-type ('typeof self) mask)]
        else
            spice-quote
                bitcast
                    shufflevector self self mask
                    [(construct-vec-type ('element@ ('typeof self) 0) sz)]

    unlet construct-getter-type assign-mask range-mask expand-mask
        \ build-access-mask

    @@ box-cast
    fn __as (vT T self)
        let ST = ('storageof vT)
        if ((T == vector) or (T == ST))
            `(bitcast self ST)
        elseif (T == Generator)
            let count = ('element-count ST)
            spice-quote
                Generator
                    inline () 0
                    inline (i) (i < count)
                    inline (i) (extractelement self i)
                    inline (i) (i + 1)
        else
            compiler-error! "unsupported type"

    @@ box-binary-op
    fn __== (lhsT rhsT lhs rhs)
        `(all? [(vector.__== lhsT rhsT lhs rhs)])

fn dot (u v)
    let w = (u * v)
    vector-reduce (do +) (w as (storageof (typeof w)))

#-------------------------------------------------------------------------------
# MATRICES
#-------------------------------------------------------------------------------

typedef mat-type < immutable

    inline __unpack (self)
        unpack (self as array)

    inline __@ (self index)
        extractvalue self index

    spice _mat-repr (self)
        let T = ('typeof self)
        let sz = (T.Columns as i32)
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
        let rowT = (T.RowType as type)
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
                compiler-error!
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
                        compiler-error! "too many arguments for column"
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
                compiler-error!
                    .. "number of provided elements for last row (" (repr (i32 row))
                        \ ") doesn't match number of elements required (" (repr (i32 rows)) ")"
            if (col != cols)
                compiler-error!
                    .. "number of provided columns (" (repr (i32 col))
                        \ ") doesn't match number of columns required (" (repr (i32 cols)) ")"
            self

    unlet make-diagonal-vector mat-type-constructor

    @@ box-binary-op
    fn __* (lhsT rhsT lhs rhs)
        let mat-row = row
        if ((lhsT < mat-type)
                and (rhsT < mat-type)
                and (lhsT == (rhsT.TransposedType as type)))
            # mat(i,j) * mat(j,i) -> mat(j,j)
            let VT = ('element@ lhsT 0)
            let sz = ('element-count VT)
            let ET = ('element@ VT 0)
            let destT = (construct-mat-type ET sz sz)
            fold (mat = `(nullof destT)) for i in (range sz)
                let row = `(mat-row lhs i)
                let vec =
                    fold (vec = `(nullof VT)) for j in (range sz)
                        `(insertelement vec (dot row (extractvalue rhs j)) j)
                `(insertvalue mat vec i)
        elseif (rhsT == (lhsT.RowType as type))
            let VT = ('element@ lhsT 0)
            let sz = ('element-count VT)
            # mat(i,j) * vec(i) -> vec(j)
            fold (vec = `(nullof VT)) for i in (range sz)
                `(insertelement vec (dot (mat-row lhs i) rhs) i)
        else
            compiler-error! "unsupported type"

    @@ box-binary-op
    fn __r* (lhsT rhsT lhs rhs)
        if (lhsT == ('element@ rhsT 0))
            # vec(j) * mat(i,j) -> vec(i)
            let sz = ('element-count rhsT)
            fold (vec = `(nullof rhsT.RowType)) for i in (range sz)
                `(insertelement vec (dot lhs (extractvalue rhs i)) i)
        else
            compiler-error! "unsupported type"

    @@ box-cast
    fn __as (vT T self)
        let ST = ('storageof vT)
        if ((T == array) or (T == ST))
            `(bitcast self ST)
        elseif (T == Generator)
            let count = ('element-count ST)
            spice-quote
                Generator
                    inline () 0
                    inline (i) (i < count)
                    inline (i) (extractvalue self i)
                    inline (i) (i + 1)
        else
            compiler-error! "unsupported type"

    @@ box-binary-op
    fn __== (lhsT rhsT lhs rhs)
        if (lhsT == rhsT)
            let cols = ('element-count lhsT)
            let VT = (vector bool cols)
            let vec =
                fold (vec = `(nullof VT)) for i in (range cols)
                    let cmp =
                        `((extractvalue lhs i) == (extractvalue rhs i))
                    `(insertelement vec cmp i)
            return `(all? vec)
        compiler-error! "unsupported type"

spice _transpose (m)
    let T = ('typeof m)
    assert (T < mat-type)
    let TT = (T.TransposedType as type)
    fold (self = `(nullof TT)) for i in (range (T.Rows as i32))
        `(insertvalue self ('row m i) i)

@@ spice-quote
fn transpose (m)
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
