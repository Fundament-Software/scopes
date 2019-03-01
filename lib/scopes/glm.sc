#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""glm
    ===

    The `glm` module exports the basic vector and matrix types as well as
    related arithmetic operations which mimic the features available to shaders
    written in the GL shader language.

typedef vec-type < immutable
typedef mat-type < immutable

typedef vec-type-accessor
    'set-symbols this-type
        __imply =
            box-cast
                fn "vec-type-accessor-imply" (vT T self)
                    let rhvecT = (vT.RHVectorType as type)
                    if ((T == rhvecT) or (T == ('superof rhvecT)))
                        let mask = vT.Mask
                        return `(bitcast (shufflevector self self mask) rhvecT)
                    compiler-error! "unsupported type"
        __= =
            box-binary-op
                fn "vec-type-accessor-assign" (lhsT rhsT lhs rhs)
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
                        ast-quote
                            do
                                let rhs = (imply rhs rhvecT)
                                # expand or contract
                                let rhs = (shufflevector rhs rhs expandmask)
                                assign (shufflevector lhs rhs assignmask) lhs

run-stage;

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
            let TT = (construct-mat-type element-type rows cols)
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

let element-set-xyzw = "^[xyzw]{1,4}$"
let element-set-rgba = "^[rgba]{1,4}$"
let element-set-stpq = "^[stpq]{1,4}$"
let element-set-any = "^([xyzw]|[stpq]|[rgba]){1,4}$"

do
    let this-type = vec-type

    inline vec-type-constructor (element-type size)
        construct-vec-type (imply element-type type) (imply size i32)

    spice vec-constructor (self ...)
        let self = (self as type)
        if (self == vec-type)
            `(vec-type-constructor ...)
        else
            let ET argsz =
                'element@ self 0; 'argcount ...
            # count sum of elements
            let flatargsz =
                loop (i total = 0 0)
                    if (i == argsz)
                        break total
                    let arg = ('getarg ... i)
                    let argT = ('typeof arg)
                    repeat (i + 1)
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
                    else arg
                loop (i value = 0 initval)
                    if (i == vecsz)
                        break value
                    repeat (i + 1)
                        `(insertelement value arg i)
            elseif (flatargsz == vecsz)
                loop (i total value = 0 0 initval)
                    if (i == argsz)
                        break value
                    let arg = ('getarg ... i)
                    let argT = ('typeof arg)
                    repeat (i + 1)
                        if (argT < vec-type)
                            let argET argvecsz =
                                'element@ argT 0; 'element-count argT
                            loop (k total value = 0 total value)
                                if (k == argvecsz)
                                    break total value
                                repeat (k + 1)
                                    total + 1;
                                    `(insertelement value (extractelement arg k) total)
                        else
                            _ (total + 1)
                                `(insertelement value (arg as ET) total)
            else
                compiler-error!
                    .. "number of arguments (" (repr flatargsz)
                        \ ") doesn't match number of elements (" (repr vecsz) ")"

    method '__repr (self)
        tostring (self as vector)

    method '__@ (self i)
        extractelement self i

    method '__unpack (self)
        unpack (self as vector)

    method inline '__neg (self)
        - ((typeof self) 0) self

    method inline '__rcp (self)
        / ((typeof self) 1) self

    fn vec-type-binary-op-expr (symbol lhsT rhsT lhs rhs)
        let Ta = ('element@ lhsT 0)
        let f =
            try ('@ Ta symbol)
            except (err)
                compiler-error! "unsupported operation"
        let f = (unbox-binary-op-function-type f)
        let rhs =
            if (rhsT == lhsT) rhs
            else `(lhsT rhs)
        return (f lhsT lhsT lhs rhs)

    fn vec-type-binary-op-expr-r (symbol lhsT rhsT lhs rhs)
        let Tb = ('element@ rhsT 0)
        let f =
            try ('@ Tb symbol)
            except (err)
                compiler-error! "unsupported operation"
        let f = (unbox-binary-op-function-type f)
        let lhs =
            if (rhsT == lhsT) lhs
            else `(rhsT lhs)
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

    #set-type-symbol! vec-type '__neg
        inline (self)
            - ((typeof self) 0) self

    #set-type-symbol! vec-type '__rcp
        inline (self)
            / ((typeof self) 1) self

    fn build-access-mask (name)
        let s = (name as string)
        let sz = ((countof s) as i32)
        if (sz > 4)
            compiler-error! "too many characters in accessor (try 1 <= x <= 4)"
        let set =
            if (sc_string_match element-set-xyzw s) "xyzw"
            elseif (sc_string_match element-set-rgba s) "rgba"
            elseif (sc_string_match element-set-stpq s) "stpq"
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
            #if ((typeof mask) == i32)
                let index = mask
                typeinline T '__deref (self)
                    extractelement (load self) index
                typeinline T '__imply (self destT)
                    if (destT == ET)
                        deref self
                typeinline T '__= (self other)
                    let other = (imply other ET)
                    store
                        insertelement (load self) other index
                        self
                    true
            let sz = ('element-count ('typeof mask))
            let lhsz = ('element-count vecrefT)
            'set-symbols this-type
                RHVectorType = (construct-vec-type ET sz)
                Mask = mask
                AssignMask = (assign-mask lhsz mask)
                ExpandMask = (expand-mask lhsz sz)

    spice vec-getattr (self name)
        let name = (name as Symbol as string)
        let sz mask = (build-access-mask name)
        let QT = ('qualified-typeof self)
        if (sz == 1)
            `(extractelement self mask)
        elseif ('refer? QT)
            ast-quote
                bitcast self
                    [(construct-getter-type ('typeof self) mask)]
        else
            ast-quote
                bitcast
                    shufflevector self self mask
                    [(construct-vec-type ('element@ ('typeof self) 0) sz)]

    'set-symbols this-type
        __typecall = vec-constructor
        __getattr = vec-getattr
        __as =
            box-cast
                fn "vec-type-as" (vT T self)
                    let ST = ('storageof vT)
                    if ((T == vector) or (T == ST))
                        `(bitcast self ST)
                    elseif (T == Generator)
                        let count = ('element-count ST)
                        ast-quote
                            Generator
                                inline (fdone index)
                                    if (index == count)
                                        fdone;
                                    else
                                        _ (index + 1) (extractelement self index)
                                0
                    else
                        compiler-error! "unsupported type"
        __== =
            box-binary-op
                fn "vec-type==" (lhsT rhsT lhs rhs)
                    `(all? [(vector.__== lhsT rhsT lhs rhs)])

run-stage;

#-------------------------------------------------------------------------------

set-type-symbol! mat-type '__unpack
    inline "mat-type-unpack" (self)
        unpack
            bitcast self (storageof (typeof self))

set-type-symbol! mat-type '__@
    inline "mat-type@" (self i)
        extractvalue self i

set-type-symbol! mat-type '__as
    inline "mat-type-as" (self destT)
        let ST = (storageof (typeof self))
        if ((destT == array) or (destT == ST))
            bitcast self ST
        elseif (destT == Generator)
            let count = (countof ST)
            Generator
                label (fret fdone index)
                    if (index == count)
                        fdone;
                    else
                        fret (index + 1:usize) (extractvalue self index)
                0:usize

fn empty-value (T)
    nullof T
fn make-diagonal-vector (VT i)
    let vec = (empty-value VT)
    if (i < VT.Count)
        insertelement vec (VT.ElementType 1) i
    else vec

set-type-symbol! mat-type '__typecall
    inline "mat-type-new" (cls ...)
        if (cls == mat-type)
            let ET cols rows = ...
            return
                construct-mat-type
                    imply ET type
                    imply cols i32
                    imply rows i32
        let VT = cls.ColumnType
        let argsz = (va-countof ...)
        if (argsz == 0)
            fold
                empty-value cls
                unroll-range cls.Columns
                fn (break self i)
                    insertvalue self
                        make-diagonal-vector VT i
                        i
        elseif (argsz == 1)
            # construct from scalar or matrix
            let arg = (va@ 0 ...)
            let argT = (typeof arg)
            if (argT < mat-type)
                # construct from matrix
                if (argT == cls)
                    # same matrix type, just return the argument
                    arg
                else
                    # build a matrix that is bigger or smaller
                    let can-copy-vectors? = (argT.ColumnType == VT)
                    fold
                        empty-value cls
                        unroll-range cls.Columns
                        fn (break self i)
                            if (i < argT.Columns)
                                if can-copy-vectors?
                                    insertvalue self
                                        extractvalue arg i
                                        i
                                else
                                    let ET = cls.ElementType
                                    let argvec = (extractvalue arg i)
                                    insertvalue self
                                        # element-wise construction
                                        fold
                                            # start off with default diagonal vector
                                            make-diagonal-vector VT i
                                            unroll-range (min cls.Rows argT.Rows)
                                            fn (break vec j)
                                                insertelement vec ((extractelement argvec j) as ET) j
                                        i
                            else
                                # build default diagonal vector
                                insertvalue self (make-diagonal-vector VT i) i
            elseif (argT < vec-type)
                compiler-error!
                    .. (repr (i32 cls.Columns)) " column vectors required"
            else
                # build a matrix with diagonal elements set to arg
                let arg = (arg as cls.ElementType)
                fold
                    empty-value cls
                    unroll-range cls.Columns
                    inline (break self i)
                        insertvalue self (insertelement (empty-value VT) arg i) i
        else
            # construct from arbitrary composition of vectors and elements,
                which must nevertheless align to vector boundary size
            # unpack all elements and count offsets as we go
            let f =
                fold
                    fn ()
                        return (empty-value cls) (empty-value VT) 0:usize 0:usize
                    va-each ...
                    inline (break f arg)
                        let self vec col row = (f)
                        let argT = (typeof arg)
                        let rows = cls.Rows
                        let is-vector? = (argT < vec-type)
                        let nextrow =
                            + row
                                if is-vector? argT.Count
                                else 1:usize
                        if (nextrow > rows)
                            compiler-error! "too many arguments for column"
                        let vec =
                            if is-vector?
                                if (argT == VT)
                                    # same vector type
                                    arg
                                else
                                    # insert values bit by bit
                                    fold vec
                                        enumerate arg
                                        fn (break vec j value)
                                            insertelement vec (value as cls.ElementType) (row + j)
                            else
                                # assume element type
                                insertelement vec (arg as cls.ElementType) row
                        if (nextrow == rows) # end of column
                            fn ()
                                return
                                    insertvalue self vec col
                                    empty-value VT
                                    col + 1:usize
                                    0:usize
                        else # not arrived yet
                            fn ()
                                return self vec col nextrow
            let self vec col row = (f)
            if (row != 0:usize)
                compiler-error!
                    .. "number of provided elements for last row (" (repr (i32 row))
                        \ ") doesn't match number of elements required (" (repr (i32 cls.Rows)) ")"
            if (col != cls.Columns)
                compiler-error!
                    .. "number of provided columns (" (repr (i32 col))
                        \ ") doesn't match number of columns required (" (repr (i32 cls.Columns)) ")"
            self

set-type-symbol! mat-type '__==
    inline "mat-type==" (self other flipped)
        let T = (typeof self)
        if (type== T (typeof other))
            all?
                fold
                    empty-value (vector bool T.Columns)
                    unroll-range T.Columns
                    inline (break vec i)
                        insertelement vec
                            (extractvalue self i) == (extractvalue other i)
                            i

inline mat-row (mat i)
    let T = (typeof mat)
    fold
        nullof T.RowType
        unroll-range T.Columns
        inline (break vec j)
            insertelement vec (extractelement (extractvalue mat j) i) j

set-type-symbol! mat-type 'row mat-row

fn dot (u v)
    let w = (u * v)
    vector-reduce (do +) (w as (storageof (typeof w)))

fn transpose (m)
    let T = (typeof m)
    let TT = T.TransposedType
    fold
        nullof TT
        unroll-range T.Rows
        inline (break self i)
            insertvalue self
                mat-row m i
                i

inline mix (a b x)
    let Ta = (typeof a)
    let Tx = (typeof x)
    if ((Tx == bool) or ((Tx < vec-type) and ((@ Tx) == bool)))
        ? x b a
    else
        fmix a b
            if (Tx < vec-type) x
            else
                Ta x

set-type-symbol! mat-type '__*
    inline "mat-type*" (a b flipped)
        let Ta = (typeof a)
        let Tb = (typeof b)
        if ((Ta < mat-type) and (Tb < mat-type) and (Ta == Tb.TransposedType))
            # mat(i,j) * mat(j,i) -> mat(j,j)
            let sz = Ta.Rows
            let destT = (construct-mat-type Ta.ElementType sz sz)
            let VT = Ta.ColumnType
            fold
                nullof destT
                unroll-range sz
                inline (break mat i)
                    let row = (mat-row a i)
                    insertvalue mat
                        fold
                            nullof VT
                            unroll-range sz
                            inline (break vec j)
                                insertelement vec
                                    dot row (extractvalue b j)
                                    j
                        i
        elseif (flipped and (Ta == Tb.ColumnType))
            # vec(j) * mat(i,j) -> vec(i)
            fold
                nullof Tb.RowType
                unroll-range Tb.Columns
                inline (break vec i)
                    insertelement vec
                        dot a (extractvalue b i)
                        i
        elseif (Tb == Ta.RowType)
            # mat(i,j) * vec(i) -> vec(j)
            fold
                nullof Ta.ColumnType
                unroll-range Ta.Rows
                inline (break vec i)
                    insertelement vec
                        dot (mat-row a i) b
                        i

set-type-symbol! mat-type '__repr
    inline "mat-type-repr" (self)
        let sz = (i32 ((typeof self) . Columns))
        let loop (i result...) = sz "]"
        if (i != 0)
            let i = (i - 1)
            loop i
                ? (i == 0) "" " "
                repr ((extractvalue self i) as vector)
                result...
        .. "[" result...

#-------------------------------------------------------------------------------

if main-module?
    assert
        60 ==
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
