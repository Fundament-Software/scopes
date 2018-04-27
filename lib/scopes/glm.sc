
let vec-type = (typename "vec-type" (fn ()))
let mat-type = (typename "mat-type" (fn ()))
set-typename-super! vec-type immutable
set-typename-super! mat-type immutable

fn element-prefix (element-type)
    match element-type
        bool "b"
        i32 "i"
        u32 "u"
        f32 ""
        f64 "d"
        else
            compiler-error! "illegal element type"

fn construct-vec-type (element-type size)
    assert (size > 1:usize)
    let prefix = (element-prefix element-type)
    let T =
        typename
            .. prefix "vec" (Any-string (Any (i32 size)))
            fn ()
    set-typename-super! T vec-type
    set-typename-storage! T (vector element-type size)
    set-type-symbol! T 'ElementType element-type
    set-type-symbol! T 'Count size
    T

fn construct-mat-type (element-type cols rows)
    assert (cols > 1:usize)
    assert (rows > 1:usize)
    let prefix = (element-prefix element-type)
    let T =
        typename
            .. prefix "mat"
                Any-string (Any (i32 cols))
                "x"
                Any-string (Any (i32 rows))
            fn ()
    let vecT =
        construct-vec-type element-type rows
    set-typename-super! T mat-type
    set-typename-storage! T (array vecT cols)
    set-type-symbol! T 'ElementType element-type
    set-type-symbol! T 'ColumnType vecT
    set-type-symbol! T 'RowType
        construct-vec-type element-type cols
    set-type-symbol! T 'Columns cols
    set-type-symbol! T 'Rows rows
    if (cols == rows)
        set-type-symbol! T 'TransposedType T
    elseif (cols < rows)
        let TT = (construct-mat-type element-type rows cols)
        set-type-symbol! T 'TransposedType TT
        set-type-symbol! TT 'TransposedType T
    T

fn construct-vec-types (count)
    let count = (usize count)
    return
        construct-vec-type f32 count
        construct-vec-type f64 count
        construct-vec-type i32 count
        construct-vec-type u32 count
        construct-vec-type bool count

fn construct-mat-types (cols rows)
    let cols = (usize cols)
    let rows = (usize rows)
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

set-type-symbol! vec-type 'repr
    fn vec-type-repr (self)
        repr (self as vector)

set-type-symbol! vec-type '@
    fn "vec-type@" (self i)
        extractelement self i

set-type-symbol! vec-type 'as
    fn vec-type-as (self destT)
        let ST = (storageof (typeof self))
        if ((destT == vector) or (destT == ST))
            bitcast self ST
        elseif (destT == Generator)
            let count = (countof ST)
            Generator
                label (fret fdone index)
                    if (index == count)
                        fdone;
                    else
                        fret (index + 1:usize) (extractelement self index)
                0:usize

set-type-symbol! vec-type 'unpack
    fn "vec-type-unpack" (self)
        unpack
            bitcast self (storageof (typeof self))

set-type-symbol! vec-type 'apply-type
    fn vec-type-new (self ...)
        let ET = (@ self)
        let argsz = (va-countof ...)
        let loop (i args...) = argsz
        if (i != 0)
            let i = (i - 1)
            let arg = (va@ i ...)
            let arg = (imply arg immutable)
            let argT = (typeof arg)
            if (argT <: vec-type)
                let argET argvecsz = (@ argT) ((countof argT) as i32)
                let flatten-loop (k args...) = argvecsz args...
                if (k != 0)
                    let k = (k - 1)
                    flatten-loop k ((extractelement arg k) as ET) args...
                loop i args...
            loop i (arg as ET) args...
        let argsz = (va-countof args...)
        let vecsz = ((countof self) as i32)
        if (argsz == 1)
            # init all components with the same value
            let arg = (va@ 0 args...)
            let loop (i value) = 0 (nullof self)
            if (i < vecsz)
                loop (i + 1) (insertelement value arg i)
            value
        elseif (argsz == vecsz)
            let loop (i value) = 0 (nullof self)
            if (i < vecsz)
                let arg = (va@ i args...)
                loop (i + 1) (insertelement value arg i)
            value
        else
            compiler-error!
                .. "number of arguments (" (repr argsz)
                    \ ") doesn't match number of elements (" (repr vecsz) ")"

set-type-symbol! vec-type '==
    fn vec-type== (self other flipped)
        if (type== (typeof self) (typeof other))
            all? ((self as vector) == (other as vector))

fn valid-element-type? (T)
    (T < integer) or (T < real)

fn vec-type-binop (f)
    fn (a b flipped)
        let T1 T2 = (typeof a) (typeof b)
        label compute (a b)
            return
                if (type== (typeof a) (typeof b))
                    let ET = (@ (typeof a))
                    f ET a b
        if flipped
            if (T1 < vec-type)
                compute a b
            elseif (valid-element-type? T1)
                compute ((typeof b) a) b
        else
            if (T2 < vec-type)
                compute a b
            elseif (valid-element-type? T2)
                compute a ((typeof a) b)

set-type-symbol! vec-type '+
    vec-type-binop
        fn (ET a b)
            if (ET <: real)
                fadd a b
            elseif (ET <: integer)
                add a b
set-type-symbol! vec-type '-
    vec-type-binop
        fn (ET a b)
            if (ET <: real)
                fsub a b
            elseif (ET <: integer)
                sub a b
set-type-symbol! vec-type '*
    vec-type-binop
        fn (ET a b)
            if (ET <: real)
                fmul a b
            elseif (ET <: integer)
                mul a b
set-type-symbol! vec-type '/
    vec-type-binop
        fn (ET a b)
            if (ET <: real)
                fdiv a b
set-type-symbol! vec-type '//
    vec-type-binop
        fn (ET a b)
            if (ET <: integer)
                if (signed? ET)
                    sdiv a b
                else
                    udiv a b
set-type-symbol! vec-type '%
    vec-type-binop
        fn (ET a b)
            if (ET <: real)
                frem a b
            elseif (ET <: integer)
                if (signed? ET)
                    srem a b
                else
                    urem a b

fn set-vector-cmp-binop! (op ff fs fu)
    set-type-symbol! vec-type op
        vec-type-binop
            fn (ET a b)
                if (ET <: real)
                    ff a b
                elseif (ET <: integer)
                    if (signed? ET)
                        fs a b
                    else
                        fu a b
set-vector-cmp-binop! '< fcmp<o icmp<s icmp<u
set-vector-cmp-binop! '> fcmp>o icmp>s icmp>u
set-vector-cmp-binop! '<= fcmp<=o icmp<=s icmp<=u
set-vector-cmp-binop! '>= fcmp>=o icmp>=s icmp>=u

set-type-symbol! vec-type 'getattr
    fn "vec-type-getattr" (self name)
        let s = (name as string)
        let set =
            if (string-match? element-set-xyzw s) "xyzw"
            elseif (string-match? element-set-rgba s) "rgba"
            elseif (string-match? element-set-stpq s) "stpq"
            else
                return;
        fn find-index (set c)
            let setloop (k) = 0
            let sc = (set @ k)
            if (c != sc)
                setloop (k + 1)
            k
        let sz = (countof s)
        if (sz == 1)
            let k = (find-index set (s @ 0))
            extractelement self k
        elseif (sz <= 4:usize)
            let loop (i mask) = 0:usize (nullof (vector i32 sz))
            if (i < sz)
                let k = (find-index set (s @ i))
                loop (i + 1)
                    insertelement mask k i
            bitcast
                shufflevector self self mask
                construct-vec-type (@ (typeof self)) sz

#-------------------------------------------------------------------------------

set-type-symbol! mat-type 'unpack
    fn "mat-type-unpack" (self)
        unpack
            bitcast self (storageof (typeof self))

set-type-symbol! mat-type '@
    fn "mat-type@" (self i)
        extractvalue self i

set-type-symbol! mat-type 'as
    fn "mat-type-as" (self destT)
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

set-type-symbol! mat-type 'apply-type
    fn "mat-type-new" (cls ...)
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
                    fn (break self i)
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
                    fn (break f arg)
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

set-type-symbol! mat-type '==
    fn "mat-type==" (self other flipped)
        let T = (typeof self)
        if (type== T (typeof other))
            all?
                fold
                    empty-value (vector bool T.Columns)
                    unroll-range T.Columns
                    fn (break vec i)
                        insertelement vec
                            (extractvalue self i) == (extractvalue other i)
                            i

fn mat-row (mat i)
    let T = (typeof mat)
    fold
        nullof T.RowType
        unroll-range T.Columns
        fn (break vec j)
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
        fn (break self i)
            insertvalue self
                mat-row m i
                i

set-type-symbol! mat-type '*
    fn "mat-type*" (a b flipped)
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
                fn (break mat i)
                    let row = (mat-row a i)
                    insertvalue mat
                        fold
                            nullof VT
                            unroll-range sz
                            fn (break vec j)
                                insertelement vec
                                    dot row (extractvalue b j)
                                    j
                        i
        elseif (flipped and (Ta == Tb.ColumnType))
            # vec(j) * mat(i,j) -> vec(i)
            fold
                nullof Tb.RowType
                unroll-range Tb.Columns
                fn (break vec i)
                    insertelement vec
                        dot a (extractvalue b i)
                        i
        elseif (Tb == Ta.RowType)
            # mat(i,j) * vec(i) -> vec(j)
            fold
                nullof Ta.ColumnType
                unroll-range Ta.Rows
                fn (break vec i)
                    insertelement vec
                        dot (mat-row a i) b
                        i

set-type-symbol! mat-type 'repr
    fn "mat-type-repr" (self)
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

    let dot transpose
    let construct-vec-type
    locals;
