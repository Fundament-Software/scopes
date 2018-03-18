
let vec-type = (typename-type "vec-type")
let mat-type = (typename-type "mat-type")
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
        typename-type
            .. prefix "vec" (Any-string (Any (i32 size)))
    set-typename-super! T vec-type
    set-typename-storage! T (vector element-type size)
    T

fn construct-mat-type (element-type cols rows)
    assert (cols > 1:usize)
    assert (rows > 1:usize)
    let prefix = (element-prefix element-type)
    let T =
        typename-type
            .. prefix "mat"
                Any-string (Any (i32 cols))
                "x"
                Any-string (Any (i32 rows))
    let vecT =
        construct-vec-type element-type rows
    set-typename-super! T mat-type
    set-typename-storage! T (array vecT cols)
    T

let vec2 = (construct-vec-type f32 2:usize)
let vec3 = (construct-vec-type f32 3:usize)
let vec4 = (construct-vec-type f32 4:usize)
let dvec2 = (construct-vec-type f64 2:usize)
let dvec3 = (construct-vec-type f64 3:usize)
let dvec4 = (construct-vec-type f64 4:usize)
let ivec2 = (construct-vec-type i32 2:usize)
let ivec3 = (construct-vec-type i32 3:usize)
let ivec4 = (construct-vec-type i32 4:usize)
let uvec2 = (construct-vec-type u32 2:usize)
let uvec3 = (construct-vec-type u32 3:usize)
let uvec4 = (construct-vec-type u32 4:usize)
let bvec2 = (construct-vec-type bool 2:usize)
let bvec3 = (construct-vec-type bool 3:usize)
let bvec4 = (construct-vec-type bool 4:usize)

let mat2 = (construct-mat-type f32 2:usize 2:usize)
let mat3 = (construct-mat-type f32 3:usize 3:usize)
let mat4 = (construct-mat-type f32 4:usize 4:usize)

let element-set-xyzw = "^[xyzw]{1,4}$"
let element-set-rgba = "^[rgba]{1,4}$"
let element-set-stpq = "^[stpq]{1,4}$"
let element-set-any = "^([xyzw]|[stpq]|[rgba]){1,4}$"

set-type-symbol! vec-type 'repr
    fn vec-type-repr (self)
        repr (self as vector)

set-type-symbol! vec-type 'as
    fn vec-type-as (self destT)
        let ST = (storageof (typeof self))
        if ((destT == vector) or (destT == ST))
            bitcast self ST

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

fn vec-type-binop (f)
    fn (a b flipped)
        let T1 T2 = (typeof a) (typeof b)
        label compute (a b)
            if (type== (typeof a) (typeof b))
                let ET = (@ (typeof a))
                f ET a b
        if flipped
            let ET = (@ T2)
            if (T1 <: vec-type)
                compute a b
            else
                compute ((typeof b) a) b
        else
            let ET = (@ T1)
            if (T2 <: vec-type)
                compute a b
            else
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
    fn vec-type-getattr (self name)
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

fn sum (v)

fn dot (u v)
    let w = (u * v)
    vector-reduce (do +) (w as (storageof (typeof w)))

if main-module?
    assert
        60 ==
            dot
                vec4 1 2 3 4
                vec4 4 5 6 7

do
    let vec2 vec3 vec4 \
        dvec2 dvec3 dvec4 \
        ivec2 ivec3 ivec4 \
        uvec2 uvec3 uvec4 \
        bvec2 bvec3 bvec4
    let mat2 mat3 mat4
    let dot
    let construct-vec-type
    locals;
