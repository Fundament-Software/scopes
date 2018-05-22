#
      \\\
       \\\
     ///\\\
    ///  \\\

    Scopes Compiler
    Copyright (c) 2016, 2017, 2018 Leonard Ritter

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

""""globals
    =======

    These names are bound in every fresh module and main program by default.
    Essential symbols are created by the compiler, and subsequent utility
    functions, macros and types are defined and documented in `core.sc`.

    The core module implements the remaining standard functions and macros,
    parses the command-line and optionally enters the REPL.


""""A pass-through function that allows expressions to evaluate to multiple
    arguments.
inline _ (...)
    return ...

inline unconst-all (args...)
    let loop (i result...) = (va-countof args...)
    if (icmp== i 0)
        result...
    else
        let i = (sub i 1)
        let arg = (va@ i args...)
        loop i (unconst arg) result...

inline tie-const (a b)
    if (constant? a) b
    else (unconst b)

inline cond-const (a b)
    if a b
    else (unconst b)

inline pointer== (a b)
    rawcall icmp== (rawcall ptrtoint a usize) (rawcall ptrtoint b usize)

inline type? (T)
    """".. fn:: (type? T)

           returns `true` if ``T`` is a value of type `type`, otherwise
           `false`.
    rawcall icmp== (rawcall ptrtoint type usize) (rawcall ptrtoint (rawcall typeof T) usize)

inline assert-type (T)
    if (type? T)
    else
        rawcall compiler-error!
            rawcall string-join "type expected, not " (rawcall Any-repr (rawcall Any-wrap T))
inline type== (a b)
    assert-type a
    assert-type b
    rawcall icmp== (rawcall ptrtoint a usize) (rawcall ptrtoint b usize)

inline unknownof (T)
    assert-type T
    bitcast T Unknown

inline todo! (msg)
    compiler-error!
        string-join "TODO: " msg

inline error! (msg)
    __error! msg
    unreachable!;

inline typename-type? (T)
    icmp== (type-kind T) type-kind-typename
inline integer-type? (T)
    icmp== (type-kind T) type-kind-integer
inline real-type? (T)
    icmp== (type-kind T) type-kind-real
inline pointer-type? (T)
    icmp== (type-kind T) type-kind-pointer
inline function-type? (T)
    icmp== (type-kind T) type-kind-function
inline tuple-type? (T)
    icmp== (type-kind T) type-kind-tuple
inline array-type? (T)
    icmp== (type-kind T) type-kind-array
inline vector-type? (T)
    icmp== (type-kind T) type-kind-vector
inline extern-type? (T)
    icmp== (type-kind T) type-kind-extern
inline function-pointer-type? (T)
    if (pointer-type? T)
        function-type? (element-type T 0)
    else (tie-const T false)
inline typename? (val)
    typename-type? (typeof val)
inline integer? (val)
    integer-type? (typeof val)
inline real? (val)
    real-type? (typeof val)
inline pointer? (val)
    pointer-type? (typeof val)
inline array? (val)
    array-type? (typeof val)
inline vector? (T)
    vector-type? (typeof T)
inline tuple? (val)
    tuple-type? (typeof val)
inline extern? (val)
    extern? (typeof val)
inline function-pointer? (val)
    function-pointer-type? (typeof val)
inline Symbol? (val)
    type== (typeof val) Symbol
inline list? (val)
    type== (typeof val) list
inline none? (val)
    type== (typeof val) Nothing

inline gen-get-option (opts...)
    """"Given a variadic list of keyed arguments, generate a function
        ``(get-option name default)`` that either returns an option with the
        given key from ``opts...`` or ``default`` if no such key exists.

        If ``default`` is a function, then the function will be evaluated
        and the result returned.
    fn "get-option" (name default)
        let val = (va@ name opts...)
        if (none? val)
            if (type== (typeof default) Closure)
                default;
            else default
        else val

inline Any-new (val)
    inline construct (outval)
        insertvalue (insertvalue (undef Any) (typeof val) 0) outval 1

    if (type== (typeof val) Any) val
    elseif (constant? val)
        Any-wrap val
    else
        let T = (storageof (typeof val))
        inline new-static-pointer ()
            let ptr = (static-alloc T)
            store val ptr
            construct (ptrtoint ptr u64)
        fn wrap-error ()
            compiler-error!
                string-join "unable to wrap value of storage type "
                    Any-repr (Any-wrap T)
        let val =
            if (tuple-type? T) val
            else
                bitcast val T
        if (pointer-type? T)
            #compiler-message "wrapping pointer"
            construct
                ptrtoint val u64
        elseif (extern-type? T)
            Any-new (unconst val)
        elseif (bor (tuple-type? T) (array-type? T))
            let count = (type-countof T)
            if (icmp== count 0)
                construct 0:u64
            else
                new-static-pointer;
        elseif (integer-type? T)
            construct
                if (signed? (typeof val))
                    sext val u64
                else
                    zext val u64
        elseif (vector-type? T)
            new-static-pointer;
        elseif (real-type? T)
            if (type== T f32)
                construct
                    zext (bitcast val u32) u64
            else
                construct
                    bitcast val u64
        else
            wrap-error;

inline raise! (value)
    __raise! (Any-new value)
    unreachable!;

inline va-empty? (...)
    icmp== (va-countof ...) 0

inline va-types (params...)
    let sz = (va-countof params...)
    let loop (i result...) = sz
    if (icmp== i 0)
        return result...
    let i = (sub i 1)
    let arg = (va@ i params...)
    loop i (typeof arg) result...

inline va-join (a...)
    inline (out...)
        let loop (i out...) = (va-countof a...) out...
        if (icmp!= i 0)
            let i = (sub i 1)
            loop i (va@ i a...) out...
        out...

inline cons (...)
    let i = (va-countof ...)
    if (icmp<s i 2)
        compiler-error! "at least two parameters expected"
    let i = (sub i 2)
    let loop (i at tail) = i (va@ i ...)
    if (icmp== i 0)
        list-cons (Any at) tail
    else
        let i = (sub i 1)
        loop i (va@ i ...)
            list-cons (Any at) tail

inline list-new (...)
    inline loop (i tail)
        if (icmp== i 0) tail
        else
            let val = (va@ (sub i 1) ...)
            loop (sub i 1)
                list-cons (Any-new val) tail
    loop (va-countof ...) eol

# forward decl
inline as
inline forward-as
inline imply
inline forward-imply

inline not (x)
    bxor (imply x bool) true

inline gen-type-op2 (f)
    inline (a b flipped)
        if (type== (typeof a) (typeof b))
            f a b
        elseif flipped
            let result... = (forward-imply a (typeof b))
            if (va-empty? result...)
            else
                f result... b
        else
            let result... = (forward-imply b (typeof a))
            if (va-empty? result...)
            else
                f a result...

syntax-extend
    set-type-symbol! type '__call
        inline (cls ...)
            let val ok = (type@ cls '__typecall)
            if ok
                call val cls ...
            else
                compiler-error!
                    string-join "type "
                        string-join
                            Any-repr (Any-wrap cls)
                            " has no apply-type attribute"

    set-type-symbol! list '__typecall
        inline (cls ...)
            list-new ...
    set-type-symbol! extern '__typecall
        inline (cls ...)
            extern-new ...
    set-type-symbol! Any '__typecall
        inline (cls value)
            Any-new value
    set-type-symbol! Symbol '__typecall
        inline (cls value)
            string->Symbol value
    set-type-symbol! Scope '__typecall
        inline (cls parent clone)
            """"There are four ways to create a new Scope:
                ``Scope``
                    creates an empty scope without parent
                ``Scope parent``
                    creates an empty scope descending from ``parent``
                ``Scope none clone``
                    duplicate ``clone`` without a parent
                ``Scope parent clone``
                    duplicate ``clone``, but descending from ``parent`` instead

            let new? = (type== (typeof clone) Nothing)
            if (type== (typeof parent) Nothing)
                if new?
                    Scope-new;
                else
                    Scope-clone clone
            else
                if new?
                    Scope-new-expand parent
                else
                    Scope-clone-expand parent clone
    set-type-symbol! Scope 'parent Scope-parent

    set-type-symbol! type '__== (gen-type-op2 type==)
    set-type-symbol! Any '__== (gen-type-op2 Any==)
    set-type-symbol! Closure '__== (gen-type-op2 pointer==)
    set-type-symbol! Label '__== (gen-type-op2 pointer==)
    set-type-symbol! Frame '__== (gen-type-op2 pointer==)

    set-type-symbol! Label 'dump
        inline (self)
            dump-label self
    set-type-symbol! Closure 'dump
        inline (self)
            'dump (Closure-label self)

    set-type-symbol! string '__.. (gen-type-op2 string-join)
    set-type-symbol! list '__.. (gen-type-op2 list-join)

    set-type-symbol! type '__getattr
        inline (cls name)
            let val ok = (type@ cls name)
            if ok
                return val
            else
                return;

    set-type-symbol! Symbol '__as
        inline (self destT)
            if (type== destT string)
                Symbol->string self

    set-type-symbol! Symbol '__==
        gen-type-op2
            inline (a b)
                icmp== (bitcast a u64) (bitcast b u64)
    set-type-symbol! Builtin '__==
        gen-type-op2
            inline (a b)
                icmp== (bitcast a u64) (bitcast b u64)

    set-type-symbol! Nothing '__==
        inline (a b flipped)
            type== (typeof a) (typeof b)
    set-type-symbol! Nothing '__!=
        inline (a b flipped)
            bxor (type== (typeof a) (typeof b)) true

    inline setup-int-type (T)
        set-type-symbol! T '__== (gen-type-op2 icmp==)
        set-type-symbol! T '__!= (gen-type-op2 icmp!=)
        set-type-symbol! T '__+ (gen-type-op2 add)
        set-type-symbol! T '__- (gen-type-op2 sub)
        set-type-symbol! T '__neg
            inline (self)
                sub (nullof (typeof self)) self
        set-type-symbol! T '__* (gen-type-op2 mul)
        set-type-symbol! T '__<< (gen-type-op2 shl)
        set-type-symbol! T '__& (gen-type-op2 band)
        set-type-symbol! T '__| (gen-type-op2 bor)
        set-type-symbol! T '__^ (gen-type-op2 bxor)
        set-type-symbol! T '__~
            inline (x)
                bxor x ((typeof x) -1)

        # more aggressive cast that converts from all numerical types
            and usize.
        set-type-symbol! T '__as
            inline hardcast (val destT)
                let vT = (typeof val)
                let destST =
                    if (type== destT usize) (storageof destT)
                    else destT
                if (integer-type? destST)
                    let valw destw = (bitcountof vT) (bitcountof destST)
                    if (icmp== destw valw)
                        bitcast val destT
                    elseif (icmp>s destw valw)
                        if (signed? vT)
                            sext val destT
                        else
                            zext val destT
                    else
                        itrunc val destT
                elseif (real-type? destST)
                    if (signed? vT)
                        sitofp val destT
                    else
                        uitofp val destT

        # only perform safe casts i.e. integer / usize conversions that expand width
        # unless the value is constant
        set-type-symbol! T '__imply
            inline (val destT)
                if (constant? val)
                    hardcast val destT
                else
                    let vT = (typeof val)
                    let destST =
                        if (type== destT usize) (storageof destT)
                        else destT
                    if (integer-type? destST)
                        let valw destw = (bitcountof vT) (bitcountof destST)
                        # must have same signed bit
                        if (icmp== (signed? vT) (signed? destST))
                            if (icmp== destw valw)
                                bitcast val destT
                            elseif (icmp>s destw valw)
                                if (signed? vT)
                                    sext val destT
                                else
                                    zext val destT

        # general constructor
        set-type-symbol! T '__typecall
            inline (destT val)
                if (none? val)
                    nullof destT
                else
                    as val destT

        inline ufdiv (a b)
            fdiv (uitofp a f32) (uitofp b f32)

        inline ufrcp (self)
            fdiv 1.0 (uitofp self f32)

        inline sfdiv (a b)
            fdiv (sitofp a f32) (sitofp b f32)

        inline sfrcp (self)
            fdiv 1.0 (sitofp self f32)

        if (signed? (storageof T))
            set-type-symbol! T '__> (gen-type-op2 icmp>s)
            set-type-symbol! T '__>= (gen-type-op2 icmp>=s)
            set-type-symbol! T '__< (gen-type-op2 icmp<s)
            set-type-symbol! T '__<= (gen-type-op2 icmp<=s)
            set-type-symbol! T '__// (gen-type-op2 sdiv)
            set-type-symbol! T '__/ sfdiv
            set-type-symbol! T '__rcp sfrcp
            set-type-symbol! T '__% (gen-type-op2 srem)
            set-type-symbol! T '__>> (gen-type-op2 ashr)
        else
            set-type-symbol! T '__> (gen-type-op2 icmp>u)
            set-type-symbol! T '__>= (gen-type-op2 icmp>=u)
            set-type-symbol! T '__< (gen-type-op2 icmp<u)
            set-type-symbol! T '__<= (gen-type-op2 icmp<=u)
            set-type-symbol! T '__// (gen-type-op2 udiv)
            set-type-symbol! T '__/ ufdiv
            set-type-symbol! T '__rcp ufrcp
            set-type-symbol! T '__% (gen-type-op2 urem)
            set-type-symbol! T '__>> (gen-type-op2 lshr)

    inline setup-real-type (T)
        inline floordiv (a b)
            sdiv (fptosi a i32) (fptosi b i32)

        # only perform safe casts: i.e. float to double
        set-type-symbol! T '__imply
            inline (val destT)
                let vT = (typeof val)
                if (real-type? destT)
                    let valw destw = (bitcountof vT) (bitcountof destT)
                    if (icmp== destw valw)
                        bitcast val destT
                    elseif (icmp>s destw valw)
                        fpext val destT

        # more aggressive cast that converts from all numerical types
        set-type-symbol! T '__as
            inline hardcast (val destT)
                let vT = (typeof val)
                let destST =
                    if (type== destT usize) (storageof destT)
                    else destT
                if (real-type? destST)
                    let valw destw = (bitcountof vT) (bitcountof destST)
                    if (icmp== destw valw)
                        bitcast val destT
                    elseif (icmp>s destw valw)
                        fpext val destT
                    else
                        fptrunc val destT
                elseif (integer-type? destST)
                    if (signed? destST)
                        fptosi val destT
                    else
                        fptoui val destT

        set-type-symbol! T '__typecall
            inline (destT val)
                if (none? val)
                    nullof destT
                else
                    as val destT

        set-type-symbol! T '__== (gen-type-op2 fcmp==o)
        set-type-symbol! T '__!= (gen-type-op2 fcmp!=u)
        set-type-symbol! T '__> (gen-type-op2 fcmp>o)
        set-type-symbol! T '__>= (gen-type-op2 fcmp>=o)
        set-type-symbol! T '__< (gen-type-op2 fcmp<o)
        set-type-symbol! T '__<= (gen-type-op2 fcmp<=o)
        set-type-symbol! T '__+ (gen-type-op2 fadd)
        set-type-symbol! T '__- (gen-type-op2 fsub)
        set-type-symbol! T '__neg
            inline (self)
                fsub (nullof (typeof self)) self
        set-type-symbol! T '__* (gen-type-op2 fmul)
        set-type-symbol! T '__/ (gen-type-op2 fdiv)
        set-type-symbol! T '__rcp
            inline (self)
                fdiv (imply 1 (typeof self)) self
        set-type-symbol! T '__// (gen-type-op2 floordiv)
        set-type-symbol! T '__% (gen-type-op2 frem)

    setup-int-type bool
    setup-int-type i8
    setup-int-type i16
    setup-int-type i32
    setup-int-type i64
    setup-int-type u8
    setup-int-type u16
    setup-int-type u32
    setup-int-type u64

    setup-int-type usize
    set-typename-super! usize integer

    setup-real-type f32
    setup-real-type f64

    syntax-scope

inline string-repr (val)
    Any-string (Any val)

fn op-prettyname (symbol)
    if (icmp== symbol '__=) "assignment"
    elseif (icmp== symbol '__unpack) "unpacking"
    elseif (icmp== symbol '__countof) "counting"
    elseif (icmp== symbol '__@) "indexing"
    elseif (icmp== symbol '__slice) "slicing"
    elseif (icmp== symbol '__..) "joining"
    elseif (icmp== symbol '__+) "addition"
    elseif (icmp== symbol '__-) "subtraction"
    elseif (icmp== symbol '__*) "multiplication"
    elseif (icmp== symbol '__/) "division"
    elseif (icmp== symbol '__%) "modulo operation"
    elseif (icmp== symbol '__neg) "negation"
    elseif (icmp== symbol '__rcp) "reciprocal"
    elseif (icmp== symbol '__//) "integer division"
    elseif (icmp== symbol '__>>) "right shift"
    elseif (icmp== symbol '__<<) "left shift"
    elseif (icmp== symbol '__&) "bitwise and"
    elseif (icmp== symbol '__|) "bitwise or"
    elseif (icmp== symbol '__^) "bitwise xor"
    elseif (icmp== symbol '__==) "equal comparison"
    elseif (icmp== symbol '__!=) "inequality comparison"
    elseif (icmp== symbol '__>=) "greater-than/equal comparison"
    elseif (icmp== symbol '__<=) "less-than/equal comparison"
    elseif (icmp== symbol '__>) "greater-than comparison"
    elseif (icmp== symbol '__<) "less-than comparison"
    elseif (icmp== symbol '__+=) "in-place addition"
    elseif (icmp== symbol '__-=) "in-place subtraction"
    elseif (icmp== symbol '__*=) "in-place multiplication"
    elseif (icmp== symbol '__/=) "in-place division"
    elseif (icmp== symbol '__//=) "in-place integer division"
    elseif (icmp== symbol '__%=) "in-place modulo operation"
    elseif (icmp== symbol '__>>=) "in-place right shift"
    elseif (icmp== symbol '__<<=) "in-place left shift"
    elseif (icmp== symbol '__&=) "in-place bitwise and"
    elseif (icmp== symbol '__|=) "in-place bitwise or"
    elseif (icmp== symbol '__^=) "in-place bitwise xor"
    else
        string-join
            Any-repr (Any-wrap symbol)
            " operation"

inline opN-dispatch (symbol mincount maxcount)
    let verify-argument-count =
        if (none? mincount)
            inline ()
        else
            inline (c)
                if (icmp<s c mincount)
                    compiler-error!
                        string-join (op-prettyname symbol)
                            string-join " requires at least "
                                string-join (Any-repr (Any-wrap mincount))
                                    string-join " arguments but got "
                                        Any-repr (Any-wrap c)
    let verify-argument-count =
        if (none? maxcount) verify-argument-count
        else
            inline (c)
                if (icmp>s c maxcount)
                    compiler-error!
                        string-join (op-prettyname symbol)
                            string-join " accepts at most "
                                string-join (Any-repr (Any-wrap maxcount))
                                    string-join " arguments but got "
                                        Any-repr (Any-wrap c)
    inline (...)
        verify-argument-count (va-countof ...)
        let self ... = ...
        let T = (typeof self)
        let op success = (type@ T symbol)
        if success
            return (op self ...)
        compiler-error!
            string-join (op-prettyname symbol)
                string-join " does not apply to value of type "
                    Any-repr (Any-wrap T)

inline op1-dispatch (symbol)
    opN-dispatch symbol 1 1

inline op2-dispatch (symbol)
    inline (a b)
        let Ta Tb = (typeof a) (typeof b)
        let op success = (type@ Ta symbol)
        if success
            let result... = (op a b)
            if (icmp== (va-countof result...) 0)
            else
                return result...
        compiler-error!
            string-join (op-prettyname symbol)
                string-join " does not apply to values of type "
                    string-join
                        Any-repr (Any-wrap Ta)
                        string-join " and "
                            Any-repr (Any-wrap Tb)

inline op2-dispatch-bidi (symbol fallback)
    inline (...)
        if (icmp<s (va-countof ...) 2)
            compiler-error!
                string-join (op-prettyname symbol)
                    " requires at least two arguments"
        let a b = ...
        let Ta Tb = (typeof a) (typeof b)
        let op success = (type@ Ta symbol)
        if success
            let result... = (op a b false)
            if (icmp== (va-countof result...) 0)
            else
                return result...
        let op success = (type@ Tb symbol)
        if success
            let result... = (op a b true)
            if (icmp== (va-countof result...) 0)
            else
                return result...
        if (type== (typeof fallback) Nothing)
        else
            return (fallback a b)
        compiler-error!
            string-join (op-prettyname symbol)
                string-join " does not apply to values of type "
                    string-join
                        Any-repr (Any-wrap Ta)
                        string-join " and "
                            Any-repr (Any-wrap Tb)

inline dispatch-unop-binop (f1 f2)
    inline (...)
        if (icmp<s (va-countof ...) 2)
            f1 ...
        else
            f2 ...

inline op2-ltr-multiop (f)
    inline (...)
        if (icmp<=s (va-countof ...) 2)
            return
                f ...
        let a b ... = ...
        let sz = (va-countof ...)
        let loop (i result...) = 0 (f a b)
        if (icmp<s i sz)
            let x = (va@ i ...)
            loop (add i 1) (f result... x)
        else result...

inline op2-rtl-multiop (f)
    inline (...)
        let sz = (va-countof ...)
        if (icmp<=s sz 2)
            return
                f ...
        let i = (sub sz 1)
        let x = (va@ i ...)
        let loop (i result...) = i x
        if (icmp>s i 0)
            let i = (sub i 1)
            let x = (va@ i ...)
            loop i (f x result...)
        else result...

let == = (op2-dispatch-bidi '__==)
let != =
    op2-dispatch-bidi '__!=
        inline (...)
            bxor true (== ...)
let > = (op2-dispatch-bidi '__>)
let >= = (op2-dispatch-bidi '__>=)
let < = (op2-dispatch-bidi '__<)
let <= = (op2-dispatch-bidi '__<=)
let + = (op2-ltr-multiop (op2-dispatch-bidi '__+))
let - =
    dispatch-unop-binop
        op1-dispatch '__neg
        op2-dispatch-bidi '__-
let * = (op2-ltr-multiop (op2-dispatch-bidi '__*))
let / =
    dispatch-unop-binop
        op1-dispatch '__rcp
        op2-dispatch-bidi '__/
let // = (op2-dispatch-bidi '__//)
let % = (op2-dispatch-bidi '__%)
let & = (op2-dispatch-bidi '__&)
let | = (op2-ltr-multiop (op2-dispatch-bidi '__|))
let ^ = (op2-dispatch-bidi '__^)
let ~ = (op1-dispatch '__~)
let << = (op2-dispatch-bidi '__<<)
let >> = (op2-dispatch-bidi '__>>)
let .. = (op2-ltr-multiop (op2-dispatch-bidi '__..))
let countof = (op1-dispatch '__countof)
let unpack = (op1-dispatch '__unpack)
inline at (obj key)
    (op2-dispatch '__@) obj
        if (constant? key)
            if (integer? key)
                if (signed? (typeof key))
                    if (icmp<s key 0)
                        add (i64 (countof obj)) (i64 key)
                    else key
                else key
            else key
        else key
let @ = (op2-ltr-multiop at)

let += = (op2-dispatch '__+=)
let -= = (op2-dispatch '__-=)
let *= = (op2-dispatch '__*=)
let /= = (op2-dispatch '__/=)
let //= = (op2-dispatch '__//=)
let %= = (op2-dispatch '__%=)
let >>= = (op2-dispatch '__>>=)
let <<= = (op2-dispatch '__<<=)
let &= = (op2-dispatch '__&=)
let |= = (op2-dispatch '__|=)
let ^= = (op2-dispatch '__^=)

fn repr

fn type-mismatch-string (want-T have-T)
    .. "type " (repr want-T) " expected, not " (repr have-T)

inline assert-typeof (a T)
    if (type== T (typeof a))
    else
        compiler-error!
            type-mismatch-string T (typeof a)

inline Any-typeof (val)
    assert-typeof val Any
    extractvalue val 0

inline Any-payload (val)
    assert-typeof val Any
    extractvalue val 1

inline forward-repr (value)
    let op success = (type@ (typeof value) '__repr)
    if success
        op value
    else
        Any-repr (Any value)

fn repr (value)
    let T = (typeof value)
    let CT =
        if (type== T Any)
            Any-typeof value
        else T
    inline append-type? ()
        label ret (value)
            return
                tie-const CT value
        if (type== CT i32) (ret false)
        elseif (type== CT bool) (ret false)
        elseif (type== CT Nothing) (ret false)
        elseif (type== CT f32) (ret false)
        elseif (type== CT string) (ret false)
        elseif (type== CT list) (ret false)
        elseif (type== CT Symbol) (ret false)
        elseif (type== CT type) (ret false)
        elseif (vector-type? CT)
            let ET = (element-type CT 0)
            if (type== ET i32) (ret false)
            elseif (type== ET bool) (ret false)
            elseif (type== ET f32) (ret false)
            else (ret true)
        else (ret true)
    let op success = (type@ T '__repr)
    let text =
        if success
            op value
        else
            Any-repr (Any value)
    if (append-type?)
        .. text
            default-styler style-operator ":"
            default-styler style-type (type-name CT)
    else text

inline scalar-type (T)
    let ST = (storageof T)
    if (type== (superof ST) vector)
        element-type ST 0
    else ST
inline select-op (T sop fop)
    let T = (scalar-type T)
    if (type== (superof T) integer) sop
    elseif (type== (superof T) real) fop
    else
        compiler-error!
            string-join "invalid argument type: "
                string-join (Any-repr (Any-wrap T))
                    ". integer or real vector or scalar expected"

inline sabs (x)
    let zero = ((typeof x) 0)
    ? (icmp<s x zero) (sub zero x) x

inline abs (x)
    (select-op (typeof x) sabs fabs) x

inline sign (x)
    (select-op (typeof x) ssign fsign) x

inline powi (base exponent)
    assert-typeof base i32
    assert-typeof exponent i32
    # special case for constant base 2
    if (constant? base)
        if (icmp== base 2)
            return
                shl 1 exponent
    let loop (result cur exponent) =
        tie-const exponent 1
        tie-const exponent base
        exponent
    if (icmp== exponent 0) result
    else
        loop
            if (icmp== (band exponent 1) 0) result
            else
                mul result cur
            mul cur cur
            lshr exponent 1

inline pow (x y)
    (select-op (typeof x) powi powf) x y

inline forward-typeattr (T name)
    let value success = (type@ T name)
    if success
        return value success
    let op success = (type@ T '__typeattr)
    if success
        return (op T name)

inline forward-getattr (self name)
    let T = (typeof self)
    let op success = (type@ T '__getattr)
    if success
        return (op self name)

inline typeattr (T name)
    let result... = (forward-typeattr T name)
    if (va-empty? result...)
        compiler-error!
            string-join "no such attribute "
                string-join (Any-repr (Any-wrap name))
                    string-join " in type "
                        Any-repr (Any-wrap T)
    else result...

inline getattr (self name)
    let result... = (forward-getattr self name)
    if (va-empty? result...)
        compiler-error!
            string-join "no such attribute "
                string-join (Any-repr (Any-wrap name))
                    string-join " in value of type "
                        Any-repr (Any-wrap (typeof self))
    else result...

inline empty? (x)
    == (countof x) 0:usize

inline type< (T superT)
    let loop (T) = T
    let value = (superof T)
    if (type== value superT) (tie-const T true)
    elseif (type== value typename) (tie-const T false)
    else
        loop value

inline type<= (T superT)
    if (type== T superT)
        return true
    type< T superT

inline forward-as (value dest-type)
    let T = (typeof value)
    if (type<= T dest-type)
        return value
    let f ok = (type@ T '__imply)
    if ok
        let result... = (f value dest-type)
        if (icmp!= (va-countof result...) 0)
            return result...
    let f ok = (type@ T '__as)
    if ok
        let result... = (f value dest-type)
        if (icmp!= (va-countof result...) 0)
            return result...

inline as (value dest-type)
    let T = (typeof value)
    if (type<= T dest-type)
        return value
    let f ok = (type@ T '__imply)
    if ok
        let result... = (f value dest-type)
        if (icmp!= (va-countof result...) 0)
            return result...
    let f ok = (type@ T '__as)
    if ok
        let result... = (f value dest-type)
        if (icmp!= (va-countof result...) 0)
            return result...
    compiler-error!
        string-join "cannot convert value of type "
            string-join (Any-repr (Any-wrap T))
                string-join " to "
                    Any-repr (Any-wrap dest-type)

inline forward-imply (value dest-type)
    let T = (typeof value)
    if (type<= T dest-type)
        return value
    let f ok = (type@ T '__imply)
    if ok
        let result... = (f value dest-type)
        if (icmp!= (va-countof result...) 0)
            return result...

inline imply (value dest-type)
    let T = (typeof value)
    if (type<= T dest-type)
        return value
    let f ok = (type@ T '__imply)
    if ok
        let result... = (f value dest-type)
        if (icmp!= (va-countof result...) 0)
            return result...
    compiler-error!
        string-join "cannot implicitly convert value of type "
            string-join (Any-repr (Any-wrap T))
                string-join " to "
                    Any-repr (Any-wrap dest-type)

let hash = (typename-type "hash")
set-typename-storage! hash u64

inline forward-hash (value)
    let T = (typeof value)
    if (type== T hash)
        return value
    let f ok = (type@ T '__hash)
    if ok
        let result = (f value)
        if (type== (typeof result) hash)
            return result
        else
            compiler-error!
                string-join "value of type "
                    string-join (Any-repr (Any-wrap T))
                        string-join "did not hash to type"
                            Any-repr (Any-wrap hash)
    let T =
        if (opaque? T) T
        else (storageof T)
    if (integer-type? T)
        bitcast
            __hash
                zext (bitcast value T) u64
                sizeof T
            hash
    elseif (pointer-type? T)
        bitcast
            __hash
                ptrtoint value u64
                sizeof T
            hash
    elseif (type== T f32)
        bitcast
            __hash
                zext (bitcast value u32) u64
                sizeof T
            hash
    elseif (type== T f64)
        bitcast
            __hash
                bitcast value u64
                sizeof T
            hash

inline hash1 (value)
    let result... = (forward-hash value)
    if (va-empty? result...)
        compiler-error!
            string-join "cannot hash value of type "
                Any-repr (Any-wrap (typeof value))
    result...

let hash2 =
    op2-ltr-multiop
        inline "hash2" (a b)
            bitcast
                __hash2x64
                    bitcast (hash1 a) u64
                    bitcast (hash1 b) u64
                hash

set-type-symbol! hash '__imply
    inline "hash-imply" (self T)
        if (type== T u64)
            bitcast self u64

set-type-symbol! hash '__typecall
    inline "hash" (cls values...)
        if (icmp<s (va-countof values...) 2)
            hash1 values...
        else
            hash2 values...

inline Any-extract (val T)
    assert-typeof val Any
    let valT = (Any-typeof val)
    if (== valT T)
        if (constant? val)
            Any-extract-constant val
        else
            let payload = (Any-payload val)
            let storageT = (storageof T)
            if (pointer-type? storageT)
                inttoptr payload T
            elseif (integer-type? storageT)
                itrunc payload T
            elseif (real-type? storageT)
                bitcast
                    itrunc payload (integer-type (bitcountof storageT) false)
                    T
            elseif (bor (tuple-type? storageT) (array-type? storageT))
                let count = (type-countof storageT)
                load
                    inttoptr payload
                        pointer-type T pointer-flag-non-writable unnamed
            else
                compiler-error!
                    .. "unable to extract value of type " (Any-repr (Any-wrap T))
    elseif (constant? val)
        compiler-error!
            type-mismatch-string T valT
    else
        error!
            .. "while extracting from Any at runtime: "
                type-mismatch-string T valT

inline string->rawstring (s)
    assert-typeof s string
    getelementptr s 0 1 0
inline char (s)
    load (string->rawstring s)

inline Syntax-anchor (sx)
    assert-typeof sx Syntax
    extractvalue (load sx) 0
inline Syntax->datum (sx)
    assert-typeof sx Syntax
    extractvalue (load sx) 1
inline Syntax-quoted? (sx)
    assert-typeof sx Syntax
    extractvalue (load sx) 2

inline Anchor-file (x)
    assert-typeof x Anchor
    extractvalue (load x) 0
inline Anchor-lineno (x)
    assert-typeof x Anchor
    extractvalue (load x) 1
inline Anchor-column (x)
    assert-typeof x Anchor
    extractvalue (load x) 2

inline Exception-anchor (sx)
    assert-typeof sx Exception
    extractvalue (load sx) 0
inline Exception-message (sx)
    assert-typeof sx Exception
    extractvalue (load sx) 1

inline list-empty? (l)
    assert-typeof l list
    icmp== (ptrtoint l usize) 0:usize

inline list-at (l)
    assert-typeof l list
    if (list-empty? l)
        tie-const l (Any-wrap none)
    else
        extractvalue (load l) 0

inline list-next (l)
    assert-typeof l list
    if (list-empty? l)
        tie-const l eol
    else
        bitcast (extractvalue (load l) 1) list

inline list-at-next (l)
    assert-typeof l list
    if (list-empty? l)
        return
            tie-const l (Any-wrap none)
            tie-const l eol
    else
        return
            extractvalue (load l) 0
            bitcast (extractvalue (load l) 1) list

inline decons (val count)
    let at next = (list-at-next val)
    if (type== (typeof count) Nothing)
        return at next
    elseif (icmp<=s count 1)
        return at next
    else
        return at
            decons next (sub count 1)

inline list-countof (l)
    assert-typeof l list
    if (list-empty? l) (tie-const l 0:usize)
    else
        extractvalue (load l) 2

fn string-countof (s)
    assert-typeof s string
    extractvalue (load s) 0

inline min (a b)
    ? (<= a b) a b

inline max (a b)
    ? (>= a b) a b

inline clamp (x mn mx)
    ? (> x mx) mx
        ? (< x mn) mn x

inline slice (obj start-index end-index)
    # todo: this should be isize
    let zero count i0 = (i64 0) (i64 (countof obj)) (i64 start-index)
    let i0 =
        if (>= i0 zero) (min i0 count)
        else (max (+ i0 count) 0:i64)
    let i1 =
        if (type== (typeof end-index) Nothing) count
        else (i64 end-index)
    let i1 =
        max i0
            if (>= i1 zero) i1
            else (+ i1 count)
    (opN-dispatch '__slice) obj (usize i0) (usize i1)

fn string-compare (a b)
    assert-typeof a string
    assert-typeof b string
    let ca = (string-countof a)
    let cb = (string-countof b)
    let cc =
        if (< ca cb) (tie-const cb ca)
        else (tie-const ca cb)
    let pa pb =
        bitcast (getelementptr a 0 1 0) (pointer i8)
        bitcast (getelementptr b 0 1 0) (pointer i8)
    let loop (i) =
        tie-const cc 0:usize
    if (== i cc)
        if (< ca cb)
            return (tie-const cc -1)
        elseif (> ca cb)
            return (tie-const cc 1)
        else
            return (tie-const cc 0)
    let x y =
        load (getelementptr pa i)
        load (getelementptr pb i)
    if (< x y)
        return (tie-const cc -1)
    elseif (> x y)
        return (tie-const cc 1)
    else
        loop (+ i 1:usize)

fn list-reverse (l tail)
    assert-typeof l list
    let tail =
        if (type== (typeof tail) Nothing) eol
        else tail
    assert-typeof tail list
    let loop (l next) = l (tie-const l tail)
    if (list-empty? l) next
    else
        loop (list-next l) (list-cons (list-at l) next)

fn set-scope-symbol! (scope sym value)
    __set-scope-symbol! scope sym (Any value)

fn syntax-error! (anchor msg)
    let T = (typeof anchor)
    if (== T string)
        if (none? msg)
            __error! anchor
            unreachable!;
    set-anchor!
        if (== T Any)
            let T = (Any-typeof anchor)
            if (== T Syntax)
                Syntax-anchor (as anchor Syntax)
            else
                as anchor Anchor
        elseif (== T Syntax)
            Syntax-anchor anchor
        else anchor
    __anchor-error! msg
    unreachable!;

syntax-extend
    # a supertype to be used for conversions
    let immutable = (typename-type "immutable")
    set-scope-symbol! syntax-scope 'immutable immutable
    set-typename-super! integer immutable
    set-typename-super! real immutable
    set-typename-super! vector immutable
    set-typename-super! Symbol immutable
    set-typename-super! CEnum immutable

    let aggregate = (typename-type "aggregate")
    set-scope-symbol! syntax-scope 'aggregate aggregate
    set-typename-super! array aggregate
    set-typename-super! tuple aggregate
    set-typename-super! Any tuple

    let opaquepointer = (typename-type "opaquepointer")
    set-scope-symbol! syntax-scope 'opaquepointer opaquepointer
    set-typename-super! string opaquepointer
    set-typename-super! type opaquepointer

    set-type-symbol! integer '__typecall
        fn (cls ...)
            integer-type ...
    #set-type-symbol! real '__typecall
        fn (cls ...)
            real-type ...
    set-type-symbol! pointer 'set-element-type
        fn (cls ET)
            pointer-type-set-element-type cls ET
    set-type-symbol! pointer 'set-storage
        fn (cls storage)
            pointer-type-set-storage-class cls storage
    set-type-symbol! pointer 'immutable
        fn (cls ET)
            pointer-type-set-flags cls
                bor (pointer-type-flags cls) pointer-flag-non-writable
    set-type-symbol! pointer 'mutable
        fn (cls ET)
            pointer-type-set-flags cls
                band (pointer-type-flags cls)
                    bxor pointer-flag-non-writable -1:u64
    set-type-symbol! pointer 'strip-storage
        fn (cls ET)
            pointer-type-set-storage-class cls unnamed
    set-type-symbol! pointer 'storage
        fn (cls)
            pointer-type-storage-class cls
    set-type-symbol! pointer 'readable?
        fn (cls)
            == (& (pointer-type-flags cls) pointer-flag-non-readable) 0:u64
    set-type-symbol! pointer 'writable?
        fn (cls)
            == (& (pointer-type-flags cls) pointer-flag-non-writable) 0:u64
    set-type-symbol! pointer '__typecall
        fn (cls T opt)
            let flags =
                if (none? opt)
                    pointer-flag-non-writable
                else
                    assert-typeof opt Symbol
                    if (icmp== opt 'mutable) 0:u64
                    else
                        compiler-error! "invalid option passed to pointer type constructor"
            if (none? T)
                if (type== cls pointer)
                    compiler-error! "type expected"
                else
                    nullof cls
            else
                pointer-type T flags unnamed
    fn assert-no-arguments (...)
        if (icmp!= (va-countof ...) 0)
            compiler-error! "default constructor takes no arguments"
    set-type-symbol! array '__typecall
        fn (cls ...)
            if (type== cls array)
                let T size = ...
                array-type T (imply size usize)
            else
                assert-no-arguments ...
                nullof cls
    set-type-symbol! vector '__typecall
        fn (cls ...)
            if (type== cls vector)
                let T size = ...
                vector-type T (imply size usize)
            else
                assert-no-arguments ...
                nullof cls
    set-type-symbol! ReturnLabel '__typecall
        fn (cls ...)
            ReturnLabel-type ...
    set-type-symbol! tuple '__typecall
        fn (cls ...)
            if (type== cls tuple)
                tuple-type ...
            else
                assert-no-arguments ...
                nullof cls
    set-type-symbol! union '__typecall
        fn (cls ...)
            if (type== cls union)
                union-type ...
            else
                assert-no-arguments ...
                nullof cls

    set-type-symbol! typename '__typecall
        inline (cls args...)
            if (type== cls typename)
                let name super storage = args...
                let T = (typename-type name)
                if (not (none? super))
                    set-typename-super! T super
                if (not (none? storage))
                    set-typename-storage! T storage
                T
            else
                compiler-error!
                    string-join "typename "
                        string-join (repr cls)
                            " has no constructor"

    set-type-symbol! function '__typecall
        fn (cls ...)
            function-type ...

    set-type-symbol! Any 'typeof Any-typeof

    set-type-symbol! Any '__imply
        fn (src destT)
            Any-extract src destT

    set-type-symbol! Syntax '__imply
        fn (src destT)
            if (type== destT Any)
                Syntax->datum src
            elseif (type== destT Anchor)
                Syntax-anchor src
            else
                let anyval = (Syntax->datum src)
                let anyT = (Any-typeof anyval)
                if (type== anyT destT)
                    Any-extract anyval destT
                else
                    syntax-error! (Syntax-anchor src)
                        .. (repr destT) " expected, not " (repr anyT)

    set-type-symbol! type '__@
        inline (self key)
            let keyT = (typeof key)
            if (type== keyT Symbol)
                type@ self key
            elseif (type== keyT i32)
                element-type self key
            elseif (type== keyT Nothing)
                element-type self 0
    set-type-symbol! type '__countof type-countof

    let empty-symbol = (Symbol "")

    set-type-symbol! Parameter '__typecall
        fn (cls params...)
            let param1 param2 param3 = params...
            let TT = (tuple (typeof param1) (typeof param2) (typeof param3))
            if (type== TT (tuple Anchor Symbol type))
                Parameter-new param1 param2 param3
            elseif (type== TT (tuple Anchor Symbol Nothing))
                Parameter-new param1 param2 Unknown
            elseif (type== TT (tuple Symbol type Nothing))
                Parameter-new (active-anchor) param1 param2
            elseif (type== TT (tuple Symbol Nothing Nothing))
                Parameter-new (active-anchor) param1 Unknown
            else
                compiler-error! "usage: Parameter [anchor] symbol [type]"

    set-type-symbol! Parameter 'return-label?
        fn (self)
            icmp== (Parameter-index self) 0

    set-type-symbol! Symbol '__call
        inline "methodcall" (name self ...)
            let T = (typeof self)
            let T =
                if (type== T type) self
                else T
            (typeattr T name) self ...

    set-type-symbol! Scope '__getattr
        inline (self key)
            if (constant? self)
                let value success = (Scope@ self key)
                if success
                    Any-extract-constant value
            else
                let value = (Scope@ self key)
                return value

    set-type-symbol! Scope '__@
        fn (self key)
            let value success = (Scope@ self key)
            return
                if (constant? self)
                    Any-extract-constant value
                else value
                success

    set-type-symbol! list '__countof list-countof
    set-type-symbol! list '__getattr
        fn (self name)
            if (== name 'at)
                list-at self
            elseif (== name 'next)
                list-next self
            elseif (== name 'count)
                list-countof self
    set-type-symbol! list '__@
        fn (self i)
            let loop (x i) = (tie-const i self) (i32 i)
            if (< i 0)
                Any none
            elseif (== i 0)
                list-at x
            else
                loop (list-next x) (- i 1)
    set-type-symbol! list '__slice
        fn (self i0 i1)
            # todo: use isize
            let i0 i1 = (i64 i0) (i64 i1)
            let skip-head (l i) =
                tie-const i0 self
                tie-const i0 (i64 0)
            if (< i i0)
                skip-head (list-next l) (+ i (i64 1))
            let count = (i64 (list-countof l))
            if (== (- i1 i0) count)
                return l
            let build-slice (l next i) =
                tie-const i1 l
                tie-const i1 eol
                tie-const i1 i
            if (== i i1)
                list-reverse next
            else
                build-slice (list-next l) (list-cons (list-at l) next) (+ i 1:i64)

    fn list== (a b)
        label xreturn (value)
            return (tie-const (tie-const a b) value)
        if (icmp!= (list-countof a) (list-countof b))
            xreturn false
        let loop (a b) = (tie-const b a) (tie-const a b)
        if (list-empty? a)
            xreturn true
        let u v = (list-at a) (list-at b)
        let uT vT = ('typeof u) ('typeof v)
        if (not (type== uT vT))
            xreturn false
        let un vn = (list-next a) (list-next b)
        if (type== uT list)
            if (list== (imply u list) (imply v list))
                loop un vn
            else
                xreturn false
        elseif (Any== u v)
            loop un vn
        else
            xreturn false

    set-type-symbol! list '__==
        fn (a b flipped)
            if (type== (typeof a) (typeof b))
                list== a b

    inline gen-string-cmp (op)
        fn (a b flipped)
            if (type== (typeof a) (typeof b))
                op (string-compare a b) 0

    set-type-symbol! string '__== (gen-string-cmp ==)
    set-type-symbol! string '__!= (gen-string-cmp !=)
    set-type-symbol! string '__< (gen-string-cmp <)
    set-type-symbol! string '__<= (gen-string-cmp <=)
    set-type-symbol! string '__> (gen-string-cmp >)
    set-type-symbol! string '__>= (gen-string-cmp >=)

    let rawstring = (pointer i8)
    set-scope-symbol! syntax-scope 'rawstring (pointer i8)
    set-type-symbol! string '__imply
        fn (self destT)
            if (type== destT rawstring)
                getelementptr self 0 1 0

    set-type-symbol! string '__hash
        fn (self)
            bitcast
                __hashbytes
                    getelementptr self 0 1 0
                    string-countof self
                hash

    set-type-symbol! string 'from-cstr
        fn (value)
            let loop (i) = (unconst 0:usize)
            let c = (load (getelementptr value i))
            if (icmp== c 0:i8)
                string-new value i
            else
                loop (add i 1:usize)

    set-type-symbol! string '__countof string-countof
    set-type-symbol! string '__@
        fn string-at (s i)
            assert-typeof s string
            let i = (i64 i)
            if (< i 0:i64)
                return (tie-const i 0:i8)
            let len = (i64 (string-countof s))
            if (>= i len)
                return (tie-const (tie-const len i) 0:i8)
            let s = (bitcast (getelementptr s 0 1 0) (pointer i8))
            load (getelementptr s i)
    set-type-symbol! string '__slice
        fn (self i0 i1)
            string-new
                getelementptr (string->rawstring self) i0
                - i1 i0

    set-scope-symbol! syntax-scope 'min (op2-ltr-multiop min)
    set-scope-symbol! syntax-scope 'max (op2-ltr-multiop max)

    syntax-scope

fn Any-list? (val)
    assert-typeof val Any
    type== ('typeof val) list

fn maybe-unsyntax (val)
    if (type== ('typeof val) Syntax)
        extractvalue (load (as val Syntax)) 1
    else val

# print function
fn print (...)
    fn print-element (val)
        let T = (typeof val)
        if (== T string)
            io-write! val
        else
            io-write! (repr val)

    let loop (i) = 0
    if (< i (va-countof ...))
        if (> i 0)
            io-write! " "
        let arg = (va@ i ...)
        print-element arg
        loop (+ i 1)
    else
        io-write! "\n"

fn print-spaces (depth)
    assert-typeof depth i32
    if (icmp== depth 0)
    else
        io-write! "    "
        print-spaces (sub depth 1)

fn walk-list (on-leaf l depth)
    let loop (l) = l
    if (list-empty? l) true
    else
        let at next =
            list-at-next l
        let value =
            maybe-unsyntax at
        if (Any-list? value)
            print-spaces depth
            io-write! ";\n"
            walk-list on-leaf
                as value list
                add depth 1
        else
            on-leaf value depth
            true
        loop next

fn typify (f types...)
    let vacount = (va-countof types...)
    let loop (i types) = 0 (nullof (array-type type (usize vacount)))
    if (== i vacount)
        return
            __typify f vacount (bitcast (allocaof types) (pointer type))
    let T = (va@ i types...)
    let types = (insertvalue types T i)
    loop (+ i 1) types

fn compile-flags (opts...)
    let vacount = (va-countof opts...)
    let loop (i flags) = 0 0:u64
    if (== i vacount)
        return flags
    let flag = (va@ i opts...)
    if (not (constant? flag))
        compiler-error! "symbolic flags must be constant"
    assert-typeof flag Symbol
    loop (+ i 1)
        | flags
            if (== flag 'dump-disassembly) compile-flag-dump-disassembly
            elseif (== flag 'dump-module) compile-flag-dump-module
            elseif (== flag 'dump-function) compile-flag-dump-function
            elseif (== flag 'dump-time) compile-flag-dump-time
            elseif (== flag 'no-debug-info) compile-flag-no-debug-info
            elseif (== flag 'O1) compile-flag-O1
            elseif (== flag 'O2) compile-flag-O2
            elseif (== flag 'O3) compile-flag-O3
            else
                compiler-error!
                    .. "illegal flag: " (repr flag)
                        ". try one of"
                        \ " " (repr 'dump-disassembly)
                        \ " " (repr 'dump-module)
                        \ " " (repr 'dump-function)
                        \ " " (repr 'dump-time)
                        \ " " (repr 'no-debug-info)
                        \ " " (repr 'O1)
                        \ " " (repr 'O2)
                        \ " " (repr 'O3)

fn compile (f opts...)
    __compile f
        compile-flags opts...

fn compile-object (path table opts...)
    __compile-object path table
        compile-flags opts...

fn compile-spirv (f target opts...)
    __compile-spirv f target
        compile-flags opts...

fn compile-glsl (f target opts...)
    __compile-glsl f target
        compile-flags opts...

syntax-extend
    inline gen-type-op2 (op)
        fn (a b flipped)
            if (type== (typeof a) (typeof b))
                op a b
    set-type-symbol! type '__< (gen-type-op2 type<)
    set-type-symbol! type '__<=
        gen-type-op2
            fn (a b)
                if (type== a b) true
                else (type< a b)
    set-type-symbol! type '__> (gen-type-op2 (fn (a b) (type< b a)))
    set-type-symbol! type '__>=
        gen-type-op2
            fn (a b)
                if (type== a b) true
                else (type< b a)

    let Macro = (typename "Macro")
    let BlockScopeFunction =
        pointer
            function
                ReturnLabel (unknownof list) (unknownof Scope)
                \ list list Scope
    set-typename-storage! Macro BlockScopeFunction
    set-type-symbol! Macro '__typecall
        fn (cls f)
            assert-typeof f BlockScopeFunction
            bitcast f Macro
    set-type-symbol! Macro '__as
        fn (self destT)
            if (type== destT function)
                bitcast self BlockScopeFunction
            elseif (type== destT BlockScopeFunction)
                bitcast self BlockScopeFunction
    # support for calling macro functions directly
    set-type-symbol! Macro '__call
        fn (self at next scope)
            (bitcast self BlockScopeFunction) at next scope

    fn block-scope-macro (f)
        Macro
            as (compile (typify f list list Scope))
                BlockScopeFunction
    fn scope-macro (f)
        block-scope-macro
            fn "block-scope-macro" (at next scope)
                let at scope = (f (list-next at) scope)
                return (cons at next) scope
    fn macro (f)
        block-scope-macro
            fn "block-scope-macro" (at next scope)
                return (cons (f (list-next at)) next) scope

    # dotted symbol expander
    # --------------------------------------------------------------------------

    fn dotted-symbol? (env head)
        let s = (Symbol->string head)
        let sz = (countof s)
        let loop (i) = (unconst 0:usize)
        if (== i sz)
            return (unconst false)
        elseif (== (@ s i) (char "."))
            return (unconst true)
        loop (+ i 1:usize)

    fn split-dotted-symbol (head start end tail)
        let tail = (unconst tail)
        let s = (Symbol->string head)
        let loop (i) = (unconst start)
        if (== i end)
            # did not find a dot
            if (== start 0:usize)
                return (cons head tail)
            else
                return (cons (Symbol (slice s start)) tail)
        if (== (@ s i) (char "."))
            let tail =
                # no remainder after dot
                if (== i (- end 1:usize)) tail
                else # remainder after dot, split the rest first
                    split-dotted-symbol head (+ i 1:usize) end tail
            let dot = '.
            if (== i 0:usize)
                # no prefix before dot
                return (cons (unconst dot) tail)
            else
                # prefix before dot
                return
                    cons (Symbol (slice s start i)) dot tail
        loop (+ i 1:usize)

    # infix notation support
    # --------------------------------------------------------------------------

    fn get-ifx-symbol (name)
        Symbol (.. "#ifx:" (Symbol->string name))

    inline make-expand-define-infix (order)
        fn expand-define-infix (args scope)
            let prec token func = (decons args 3)
            let prec =
                as (as prec Syntax) i32
            let token =
                as (as token Syntax) Symbol
            let func =
                if (== ('typeof func) Nothing) token
                else
                    as (as func Syntax) Symbol
            set-scope-symbol! scope (get-ifx-symbol token)
                list prec order func
            return none scope

    fn get-ifx-op (env op)
        let sym = (Syntax->datum (as op Syntax))
        if (== ('typeof sym) Symbol)
            @ env (get-ifx-symbol (as sym Symbol))
        else
            return
                unconst (Any none)
                unconst false

    fn has-infix-ops? (infix-table expr)
        # any expression of which one odd argument matches an infix operator
            has infix operations.
        let loop (expr) = expr
        if (< (countof expr) 3:usize)
            return (unconst false)
        let expr = (list-next expr)
        let at next = (decons expr)
        let result ok = (get-ifx-op infix-table at)
        if ok
            return (unconst true)
        loop expr

    fn unpack-infix-op (op)
        let op-prec op-order op-func = (decons (as op list) 3)
        return
            as op-prec i32
            as op-order Symbol
            as op-func Symbol

    fn infix-op (infix-table token prec pred)
        let op ok =
            get-ifx-op infix-table token
        if ok
            let op-prec = (unpack-infix-op op)
            ? (pred op-prec prec) op (Any none)
        else
            syntax-error! token
                "unexpected token in infix expression"

    fn rtl-infix-op (infix-table token prec pred)
        let op ok =
            get-ifx-op infix-table token
        if ok
            let op-prec op-order = (unpack-infix-op op)
            if (== op-order '<)
                ? (pred op-prec prec) op (Any none)
            else
                unconst (Any none)
        else
            syntax-error! token
                "unexpected token in infix expression"

    fn parse-infix-expr (infix-table lhs state mprec)
        assert-typeof infix-table Scope
        assert-typeof lhs Any
        assert-typeof state list
        assert-typeof mprec i32
        let loop (lhs state) = lhs state
        if (empty? state)
            return lhs state
        let la next-state = (decons state)
        let op = (infix-op infix-table la mprec >=)
        if (== ('typeof op) Nothing)
            return lhs state
        let op-prec op-order op-name = (unpack-infix-op op)
        let rhs-loop (rhs state) = (decons next-state)
        if (empty? state)
            loop (Any (list op-name lhs rhs)) state
        let ra = (list-at state)
        let lop = (infix-op infix-table ra op-prec >)
        let nextop =
            if (== ('typeof lop) Nothing)
                rtl-infix-op infix-table ra op-prec ==
            else lop
        if (== ('typeof nextop) Nothing)
            loop (Any (list op-name lhs rhs)) state
        let nextop-prec = (unpack-infix-op nextop)
        let next-rhs next-state =
            parse-infix-expr infix-table rhs state nextop-prec
        rhs-loop next-rhs next-state

    #---------------------------------------------------------------------------

    # install general list hook for this scope
    # is called for every list the expander would otherwise consider a call
    fn list-handler (topexpr env)
        let sxexpr = (as (list-at topexpr) Syntax)
        let expr expr-anchor = (Syntax->datum sxexpr) (Syntax-anchor sxexpr)
        if (!= ('typeof expr) list)
            return topexpr env
        let expr = (as expr list)
        let head-key = (Syntax->datum (as (list-at expr) Syntax))
        let head =
            if (== ('typeof head-key) Symbol)
                let head success = (@ env (as head-key Symbol))
                if success head
                else head-key
            else head-key
        let head =
            if (== ('typeof head) type)
                let attr ok = (runtime-type@ (as head type) '__macro)
                if ok attr
                else head
            else head
        if (== ('typeof head) Macro)
            let head = (as head Macro)
            let next = (list-next topexpr)
            let expr env = (head expr next env)
            let expr = (Syntax-wrap expr-anchor (Any expr) false)
            return (as (as expr Syntax) list) env
        elseif (has-infix-ops? env expr)
            let at next = (decons expr)
            let expr =
                parse-infix-expr env at next (unconst 0)
            let next = (list-next topexpr)
            let expr = (Syntax-wrap expr-anchor expr false)
            return (list-cons expr next) env
        else
            return topexpr env

    # install general symbol hook for this scope
    # is called for every symbol the expander could not resolve
    fn symbol-handler (topexpr env)
        let at next = (decons topexpr)
        let sxname = (as at Syntax)
        let name name-anchor = (as sxname Symbol) (Syntax-anchor sxname)
        if (dotted-symbol? env name)
            let s = (Symbol->string name)
            let sz = (countof s)
            let expr =
                Any (split-dotted-symbol name (unconst 0:usize) sz eol)
            let expr = (Syntax-wrap name-anchor expr false)
            return (cons expr next) env
        return topexpr env

    set-scope-symbol! syntax-scope 'Macro Macro
    set-scope-symbol! syntax-scope 'block-scope-macro block-scope-macro
    set-scope-symbol! syntax-scope 'scope-macro scope-macro
    set-scope-symbol! syntax-scope 'macro macro
    set-scope-symbol! syntax-scope (Symbol "#list")
        compile (typify list-handler list Scope)
    set-scope-symbol! syntax-scope (Symbol "#symbol")
        compile (typify symbol-handler list Scope)

    # (define name expr ...)
    fn expand-define (expr)
        let defname = (list-at expr)
        let content = (list-next expr)
        list syntax-extend
            list set-scope-symbol! 'syntax-scope
                list quote defname
                cons do content
            'syntax-scope

    inline make-expand-and-or (flip)
        fn (expr)
            if (list-empty? expr)
                syntax-error! "at least one argument expected"
            elseif (== (list-countof expr) 1:usize)
                return (list-at expr)
            let expr = (list-reverse expr)
            let loop (result head) = (decons expr)
            if (list-empty? head)
                return result
            let tmp =
                Parameter-new
                    Syntax-anchor (as (list-at head) Syntax)
                    \ 'tmp Unknown
            loop
                Any
                    list do-in
                        list let tmp '= (list-at head)
                        list if tmp
                            if flip tmp
                            else result
                        list 'else
                            if flip result
                            else tmp
                list-next head

    set-scope-symbol! syntax-scope 'define (macro expand-define)
    set-scope-symbol! syntax-scope 'and (macro (make-expand-and-or false))
    set-scope-symbol! syntax-scope 'or (macro (make-expand-and-or true))
    set-scope-symbol! syntax-scope 'define-infix>
        scope-macro (make-expand-define-infix '>)
    set-scope-symbol! syntax-scope 'define-infix<
        scope-macro (make-expand-define-infix '<)
    syntax-scope

# (define-macro name expr ...)
# implies builtin names:
    args : list
define define-macro
    macro
        fn "expand-define-macro" (expr)
            let name body = (decons expr)
            list define name
                list macro
                    cons fn '(args) body

# (define-scope-macro name expr ...)
# implies builtin names:
    args : list
    scope : Scope
define-macro define-scope-macro
    let name body = (decons args)
    list define name
        list scope-macro
            cons fn '(args syntax-scope) body

# (define-block-scope-macro name expr ...)
# implies builtin names:
    expr : list
    next-expr : list
    scope : Scope
define-macro define-block-scope-macro
    let name body = (decons args)
    list define name
        list block-scope-macro
            cons fn '(expr next-expr syntax-scope) body

# (define-doc symbol string)
define-scope-macro define-doc
    let sxsym str = (decons args 2)
    let sym = (as (as sxsym Syntax) Symbol)
    let str = (as (as str Syntax) string)
    let _ ok = (@ syntax-scope sym)
    if ok
        set-scope-symbol! syntax-scope (Symbol (.. "#doc:" (as sym string))) str
    else
        syntax-error! sxsym "cannot document unbound name"
    return none
        syntax-scope

define-block-scope-macro defer
    let head f rest = (decons expr 2)
    let oldf = (@ syntax-scope 'return)
    let anchor = (Syntax-anchor (as f Syntax))
    let tmp =
        Parameter-new anchor 'tmp Unknown
    let expr =
        list
            list let tmp '= f
            list tmp
                list
                    cons inline "defer-wrapper" (list)
                        list label 'return (list '...)
                            list oldf
                                list tmp '...
                        list _
                        next-expr
    return expr
        syntax-scope

define-macro assert
    fn assertion-error! (constant anchor msg)
        let assert-msg =
            .. "assertion failed: "
                if (== (typeof msg) string) msg
                else (repr msg)
        if constant
            compiler-error! assert-msg
        else
            syntax-error! anchor assert-msg
    let cond body = (decons args)
    let sxcond = (as cond Syntax)
    let anchor = (Syntax-anchor sxcond)
    let tmp =
        Parameter-new anchor 'tmp Unknown
    list do
        list let tmp '= cond
        list if tmp
        list 'else
            cons assertion-error!
                list constant? tmp
                active-anchor;
                if (empty? body)
                    list (repr (Syntax->datum sxcond))
                else body

define-scope-macro del
    let loop (args)
    let sxhead rest = (decons args)
    let head = (as (as sxhead Syntax) Symbol)
    let oldsym ok = (@ syntax-scope head)
    if (not ok)
        syntax-error! sxhead
            .. "no such symbol in scope: " (repr head)
    delete-scope-symbol! syntax-scope head
    if (empty? rest)
        return none syntax-scope
    else
        loop rest

# (. value symbol ...)
define-macro .
    fn op (a b)
        let sym = (as (as b Syntax) Symbol)
        list getattr a (list quote sym)
    let a b rest = (decons args 2)
    let loop (rest result) = rest (op a b)
    if (list-empty? rest) result
    else
        let c rest = (decons rest)
        loop rest (op result c)

inline = (obj value)
    (op2-dispatch '__=) obj value
    return;

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

#define-infix> 70 :
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
#define-infix> 800 .=
#define-infix> 800 @=
#define-infix> 800 =@

#-------------------------------------------------------------------------------
# documentation for builtin forms
#-------------------------------------------------------------------------------

define-doc let
    """".. macro:: (let name ... _:= value ...)

        Binds a list of constants and variables specified on the right-hand
        side to parameter names defined on the left-hand side.

        .. macro:: (let label-name (name ...) _:= value ...)

        Performs the same function as the regular `let`, but associates the
        entry point with a labelname that can be called to effectively produce
        a tail-recursive loop. When some of the arguments on the right hand
        side are not constant, the loop will be unrolled.

        .. macro:: (let name ...)

        Rebinds names already defined in the parent scope to the local scope.
        This becomes useful in conjunction with `locals`, when exporting
        modules.

#-------------------------------------------------------------------------------
# type based function dispatch
#-------------------------------------------------------------------------------

# a lazily constructed type matcher that takes a target function, an error
    function, and finally a set of arguments and calls the target function
    if all arguments could be implicitly converted to the destination type,
    otherwise it calls the error function with a function that returns an
    error message, and a function that returns the original type arguments used.
inline type-matcher (types...)
    inline get-types ()
        types...
    let typesz = (va-countof types...)
    inline "with-target" (f)
        inline "with-error-fn" (f-error)
            inline (args...)
                let sz = (va-countof args...)
                if (icmp!= sz typesz)
                    return
                        f-error
                            inline ()
                                .. "could not resolve overloaded function from number of arguments (expected "
                                    repr typesz
                                    " but got "
                                    repr sz
                                    ")"
                            get-types
                let loop (i outargs...) = sz
                if (icmp== i 0)
                    return
                        f outargs...
                let i = (sub i 1)
                let T = (va@ i types...)
                let arg = (va@ i args...)
                let result... = (forward-imply arg T)
                if (va-empty? result...)
                    return
                        f-error
                            inline ()
                                .. "couldn't convert type of argument "
                                    repr (i + 1)
                                    " from "
                                    repr (typeof arg)
                                    " to parameter type "
                                    repr T
                            get-types
                loop i result... outargs...

# formats a list of types to a string that can be used as readable function
    type signature
fn format-type-signature (types...)
    let typesz = (va-countof types...)
    let keys... = (va-keys types...)
    let loop (i s) = 0 ""
    if (icmp== i typesz)
        return s
    let T = (va@ i types...)
    let k = (va@ i keys...)
    loop (add i 1)
        .. s
            if (k == unnamed)
                repr T
            else
                .. (k as string) ":" (repr T)
            " "

# takes two type matchers that have been specialized up to the function target
    and returns a new type matcher that has also specialized up to the function
    target, which tries to match f1 first, then f2, and otherwise passes
    an error message to the error function, along with all previously attempted
    type signature constructors to the error function.
inline chain-fn-dispatch2 (f1 f2)
    if (none? f2)
        inline "with-error-fn" (f-error)
            fn (args...)
                call
                    f1
                        inline (msgf get-types...)
                            f-error
                                inline ()
                                    .. "could not match arguments of types "
                                        format-type-signature (va-types args...)
                                        "to function"
                                \ get-types...
                    args...
    else
        inline "with-error-fn" (f-error)
            inline (args...)
                call
                    f1
                        inline (msgf get-types1)
                            call
                                f2
                                    inline (msgf get-types...)
                                        f-error
                                            inline ()
                                                .. "could not match arguments of types "
                                                    format-type-signature (va-types args...)
                                                    "to function"
                                            \ get-types1 get-types...
                                args...
                    args...
# same as the previous function, but takes an arbitrary number of arguments
define chain-fn-dispatch (op2-rtl-multiop chain-fn-dispatch2)

# a default error handling function that prints all type signatures and
    produces a compiler error
fn fn-dispatch-error-handler (msgf get-types...)
    let msg =
        .. (msgf) "\n"
            "    Expected one of"
    let sz = (va-countof get-types...)
    let loop (i msg) = 0 msg
    if (icmp== i sz)
        compiler-error! msg
    else
        let get-types = (va@ i get-types...)
        loop (add i 1)
            .. msg "\n        "
                format-type-signature (get-types)

# composes multiple target-bound type matchers into a single function
inline fn-dispatcher (args...)
    (chain-fn-dispatch args...) fn-dispatch-error-handler

# a safe immutable loop construct that never unrolls
define-block-scope-macro loop
    let syntaxmsg = "syntax: (loop (param ...) = arg ...)"
    if ((countof expr) == 1)
        return
            cons
                list 'let 'repeat '() '=
                next-expr
            syntax-scope
    do
        let head params sep args = (decons expr 3)
        if ((countof expr) < 3)
            syntax-error! head syntaxmsg
        params as Syntax as list
        if ((sep as Syntax as Symbol) != '=)
            syntax-error! sep syntaxmsg
        return
            cons
                cons 'let 'repeat params '=
                    if (empty? args)
                        unconst (list)
                    else
                        list
                            cons unconst-all args
                quote
                    let repeat =
                        label (...)
                            repeat
                                unconst-all ...
                next-expr
            syntax-scope

# sugar for fn-dispatcher
define-macro fn...
    let sxname defs = (decons args)
    let name = (sxname as Syntax as Symbol)
    fn handle-argdef (argdef)
        let argdef = (argdef as Syntax as list)
        loop (i indefs argtypes argnames) = 0 argdef '() '()
        if (empty? indefs)
            return (list-reverse argtypes) (list-reverse argnames)
        else
            let indefs =
                do
                    if (i > 0)
                        let comma indefs = (decons indefs)
                        if ((comma as Syntax as Symbol) != ',)
                            syntax-error! comma "',' separator expected"
                        indefs
                    else indefs
            let argname sep argtype indefs = (decons indefs 3)
            argname as Syntax as Symbol # verify argname is a symbol
            if ((sep as Syntax as Symbol) != ':)
                syntax-error! sep "syntax: (name : type, ...)"
            repeat (i + 1) indefs
                cons (list argname '= argtype) argtypes
                cons argname argnames
    fn handle-def (def)
        let argdef body = (decons def)
        let argtypes argnames = (handle-argdef argdef)
        list
            cons type-matcher argtypes
            cons fn argnames body
    loop (indefs outdefs) = defs '()
    if (empty? indefs)
        let args = (Parameter (Syntax-anchor (sxname as Syntax)) 'args...)
        list fn name (list args)
            list
                cons fn-dispatcher
                    list-reverse outdefs
                args
    else
        let def indefs = (decons indefs)
        let def = (def as Syntax as list)
        repeat indefs
            cons
                handle-def def
                outdefs

#-------------------------------------------------------------------------------
# references
#-------------------------------------------------------------------------------

fn construct
fn copy-construct
fn destruct
fn move-construct

let ref = (typename "ref")

fn typeof& (self)
    let T = (typeof self)
    assert (T < ref)
        .. "argument must be of reference type, but is of type " (repr T)
    element-type T 0

let voidstar = (pointer void)

fn pointer-type-imply? (src dest)
    let ET = (element-type src 0)
    let ET =
        if (opaque? ET) ET
        else (storageof ET)
    or
        # casts to voidstar are only permitted if we are not holding
        # a ref to another pointer
        and (not (pointer-type? ET))
            or
                and
                    type== dest ('mutable voidstar)
                    'writable? src
                type== dest voidstar
        type== dest ('strip-storage src)
        type== dest ('immutable src)
        type== dest ('strip-storage ('immutable src))

let ref-attribs-key = '__refattrs

inline type@& (T name)
    let repeat (T) = T
    let attrs ok = (type-local@ T ref-attribs-key)
    if ok
        let val ok = (type@ attrs name)
        if ok
            return val ok
    if (type== T typename)
        return (tie-const T none) (tie-const T false)
    repeat (superof T)

inline set-type-symbol!& (T name value)
    let attrs ok = (type-local@ T ref-attribs-key)
    let attrs =
        if ok attrs
        else
            let attrs = (typename (Symbol->string ref-attribs-key))
            set-type-symbol! T ref-attribs-key attrs
            attrs
    set-type-symbol! attrs name value

inline deref1 (value)
    let T = (typeof value)
    if (T < ref)
        let op = (type@ T '__deref)
        op value
    else value

inline deref (values...)
    let repeat (i result...) = (va-countof values...)
    if (i > 0)
        let i = (i - 1)
        let value = (va@ i values...)
        repeat i
            deref1 value
            result...
    result...

set-type-symbol!& Any 'typeof
    inline (self)
        Any-typeof (deref self)

set-type-symbol!& Any '__imply
    inline (src destT)
        Any-extract (deref src) destT

do
    inline passthru-overload (sym func)
        set-type-symbol! ref sym (fn (a b flipped) (func (deref a) (deref b)))
    passthru-overload '__== ==; passthru-overload '__!= !=
    passthru-overload '__< <; passthru-overload '__<= <=
    passthru-overload '__> >; passthru-overload '__>= >=
    passthru-overload '__& &; passthru-overload '__| |; passthru-overload '__^ ^
    passthru-overload '__+ +; passthru-overload '__- -
    passthru-overload '__* *; passthru-overload '__/ /
    passthru-overload '__** pow
    passthru-overload '__// //
    passthru-overload '__% %
    passthru-overload '__<< <<; passthru-overload '__>> >>
    passthru-overload '__.. ..

    fn passthru-inplace-overload (methodname fallback)
        set-type-symbol! ref methodname
            inline (a b)
                let ET = (typeof& a)
                let op ok = (type@& ET methodname)
                if ok
                    return (op a b)
                let op ok = (type@& ET '__deref)
                if (not ok)
                    compiler-error!
                        .. (op-prettyname methodname)
                            \ " of reference not supported by value of type " (repr ET)
                    return;
                = a (fallback a b)
                true

    passthru-inplace-overload '__+= +
    passthru-inplace-overload '__-= -
    passthru-inplace-overload '__*= *
    passthru-inplace-overload '__/= /
    passthru-inplace-overload '__//= //
    passthru-inplace-overload '__%= %
    passthru-inplace-overload '__>>= >>
    passthru-inplace-overload '__<<= <<
    passthru-inplace-overload '__&= &
    passthru-inplace-overload '__|= |
    passthru-inplace-overload '__^= ^

    fn define-ref-forward (failedf methodname)
        set-type-symbol! ref methodname
            inline (self args...)
                let ET = (typeof& self)
                let op success = (type@& ET methodname)
                if success
                    return (op self args...)
                let op ok = (type@& ET '__deref)
                if (not ok)
                    compiler-error!
                        .. (op-prettyname methodname)
                            \ " of reference not supported by value of type " (repr ET)
                    return;
                failedf (deref self) args...

    fn define-ref-forward-failable (failedf methodname)
        set-type-symbol! ref methodname
            inline (self args...)
                let ET = (typeof& self)
                let op success = (type@& ET methodname)
                if success
                    let result... = (op self args...)
                    if (not (va-empty? result...))
                        return result...
                let op ok = (type@& ET '__deref)
                if (not ok)
                    return;
                failedf (deref self) args...

    define-ref-forward (do -) '__neg
    define-ref-forward (do /) '__rcp
    define-ref-forward countof '__countof
    define-ref-forward unpack '__unpack
    define-ref-forward forward-hash '__hash
    define-ref-forward (do @) '__@
    define-ref-forward-failable forward-as '__as
    define-ref-forward-failable forward-getattr '__getattr

    set-type-symbol! ref '__repr
        fn (self)
            let ET = (typeof& self)
            let op ok = (type@& ET '__repr)
            if ok
                return (op self)
            let op ok = (type@& ET '__deref)
            if ok
                forward-repr (deref self)
            else
                string-repr
                    bitcast self (storageof (typeof self))

    set-type-symbol! ref '__deref
        inline "ref-deref" (self)
            let ET = (typeof& self)
            let op ok = (type@& ET '__deref)
            if ok
                op self
            else
                compiler-error!
                    .. "cannot dereference value of type " (repr ET)

    set-type-symbol! ref '__typeattr
        inline "ref-typeattr" (cls name)
            let T = (storageof cls)
            let ET = (element-type T 0)
            let value success = (type@& ET name)
            if success
                return value
            let op success = (type@& ET '__typeattr)
            if success
                let result... = (op ET name)
                if (va-empty? result...)
                else
                    return result...

    set-type-symbol! ref '__call
        inline "ref-call" (self args...)
            let ET = (typeof& self)
            let op success = (type@& ET '__call)
            if success
                return (op self args...)
            else
                call (deref self) args...

    set-type-symbol! ref '__imply
        inline "ref-imply" (self destT)
            let ptrtype = (storageof (typeof self))
            if (type== destT ptrtype)
                return (bitcast self ptrtype)
            if (pointer-type-imply? ptrtype destT)
                return (bitcast self destT)
            let ET = (element-type ptrtype 0)
            if (array-type? ET)
                let ET = (element-type ET 0)
                let aptrtype = (pointer ET)
                if (type== destT aptrtype)
                    return (bitcast self aptrtype)
            # try to ask target type for implicit refcast
            let op ok = (type@& ET '__imply)
            if ok
                let result... = (op self destT)
                if (not (va-empty? result...))
                    return result...
            # dereference if target type can implicitly cast
            let op ok = (type@& ET '__deref)
            if ok
                return
                    forward-imply (deref self) destT

    set-type-symbol! ref '__=
        inline "ref=" (self value)
            let ET = (typeof& self)
            let op ok = (type@& ET '__=)
            if ok
                let result... = (op self value)
                if (not (va-empty? result...))
                    return result...
            destruct self
            copy-construct self value
            true

    fn make-ref-type (PT)
        let ET = (element-type PT 0)
        let class = ('storage PT)
        let T =
            typename
                ..
                    if ('writable? PT) "&"
                    else "(&)"
                    if (class != unnamed)
                        .. "[" (Symbol->string class) "]"
                    else ""
                    type-name ET
                ref
                PT
        set-type-symbol! T 'ElementType ET
        T

    set-type-symbol! ref '__typecall
        inline "ref-typecall" (cls T)
            assert-typeof T type
            if (T < ref)
                compiler-error!
                    .. "cannot create reference type of reference type "
                        repr T
            make-ref-type T

#var has been removed; use `local`

#global has been removed; use `static`

# (typefn[!][&] type 'symbol (params) body ...)
define-macro typefn
    let ty name params body = (decons args 3)
    list set-type-symbol! ty name
        cons fn params body
define-macro typefn&
    let ty name params body = (decons args 3)
    list set-type-symbol!& ty name
        cons fn params body
define-macro typeinline
    let ty name params body = (decons args 3)
    list set-type-symbol! ty name
        cons inline params body
define-macro typeinline&
    let ty name params body = (decons args 3)
    list set-type-symbol!& ty name
        cons inline params body

inline bitcast& (self destT)
    let T = (typeof self)
    assert (T < ref) "argument must be of reference type"
    let ST = (storageof T)
    # todo: ensure types are compatible
    (bitcast self ('set-element-type ST destT)) as ref

#-------------------------------------------------------------------------------
# default memory allocators
#-------------------------------------------------------------------------------

fn pointer-each (n op value args...)
    let n = (imply n usize)
    if (n == 1:usize)
        op (value as ref) args...
    else
        # unroll only if <= 4 elements
        let loop (i) =
            if ((constant? n) and (n <= 4:usize)) 0:usize
            else (unconst 0:usize)
        if (i < n)
            op
                (getelementptr value i) as ref
                args...
            loop (i + 1:usize)
    return;

fn pointer-each2 (n op value other)
    let n = (imply n usize)
    if (n == 1:usize)
        op (value as ref) (other as ref)
    else
        # unroll only if <= 4 elements
        let loop (i) =
            if ((constant? n) and (n <= 4:usize)) 0:usize
            else (unconst 0:usize)
        if (i < n)
            op
                (getelementptr value i) as ref
                (getelementptr other i) as ref
            loop (i + 1:usize)
    return;

fn construct (value args...)
    """"Invokes the constructor for `value` of reference-like type,
        passing along optional argument set `args...`.
    let ET = (typeof& value)
    let op ok = (type@& ET '__new)
    if (not ok)
        compiler-error!
            .. "type " (repr ET) " has no constructor"
    pointer-each 1 op value args...

fn construct-array (n value args...)
    """"Invokes the constructor for an array `value` of reference-like type,
        assuming that value is a pointer to an array element, passing along
        optional argument set `args...`.
    let ET = (typeof& value)
    let op ok = (type@& ET '__new)
    if (not ok)
        compiler-error!
            .. "type " (repr ET) " has no constructor"
    pointer-each n op value args...

fn destruct (value)
    """"Invokes the destructor for `value` of reference-like type.
    let ET = (typeof& value)
    let op ok = (type@& ET '__delete)
    if (not ok)
        compiler-error!
            .. "type " (repr ET) " has no destructor"
    pointer-each 1 op value

fn destruct-array (n value)
    """"Invokes the destructor for an array `value` of reference-like type,
        assuming that value is a pointer to an array element.
    let ET = (typeof& value)
    let op ok = (type@& ET '__delete)
    if (not ok)
        compiler-error!
            .. "type " (repr ET) " has no destructor"
    pointer-each n op value

fn copy-construct (value source)
    """"Invokes the copy constructor for `value` of reference-like type if
        present, passing `source` as a value from which to copy.

        `source` does not have to be of reference type, but can also be of
        immutable element type.
    let ET = (typeof& value)
    let op ok = (type@& ET '__copy)
    if (not ok)
        compiler-error!
            .. "type " (repr ET) " has no copy constructor"
    pointer-each 1 op value source

fn copy-construct-array (n value source)
    """"Invokes the copy constructor for an array `value` of reference-like type,
        passing `source` as a value from which to copy.

        `source` has to be the first (referenced) element of an array too.
    let ET = (typeof& value)
    let op ok = (type@& ET '__copy)
    if (not ok)
        compiler-error!
            .. "type " (repr ET) " has no copy constructor"
    assert ((typeof source) < ref)
    pointer-each2 n op value source

fn move-construct (value source)
    """"Invokes the move constructor for `value` of reference-like type,
        passing `source` as the reference from which to move.
    let ET = (typeof& value)
    let op ok = (type@& ET '__move)
    if ok
        pointer-each2 1 op value source
    else
        # try copy, then destruct source
        copy-construct value source
        destruct source

fn move-construct-array (n value source)
    """"Invokes the move constructor for an array of pointers `value`
        passing `source` as an array of pointers from which to move.
    let ET = (typeof& value)
    let op ok = (type@& ET '__move)
    if ok
        pointer-each2 n op value source
    else
        copy-construct-array n value source
        destruct-array n source

let Memory = (typename "Memory")
typeinline Memory '__typecall (cls T args...)
    if (((typeof T) == Symbol) and (T == 'copy))
        if ((va-countof args...) > 1)
            compiler-error! "copy constructor only takes one argument"
        (type@ cls 'copy) cls args...
    else cls
        (type@ cls 'new) cls T args...

typeinline Memory 'delete (cls value)
    destruct value
    (type@ cls 'free) cls value

typeinline Memory 'copy (cls value)
    let T = (typeof value)
    let ET =
        if (T < ref)
            element-type (storageof T) 0
        else T
    let self =
        ((type@ cls 'allocate) cls ET) as ref
    copy-construct self value
    self

typeinline Memory 'new (cls T args...)
    let self =
        ((type@ cls 'allocate) cls T) as ref
    construct self args...
    self

let HeapMemory = (typename "HeapMemory" (super = Memory))
typeinline HeapMemory 'allocate (cls T)
    malloc T
typeinline HeapMemory 'free (cls value)
    free value
    return;
typeinline HeapMemory 'allocate-array (cls T count)
    malloc-array T count
typeinline HeapMemory 'free-array (cls value count)
    free value
    return;

let FunctionMemory = (typename "FunctionMemory" (super = Memory))
typeinline FunctionMemory 'allocate (cls T)
    alloca T
typeinline FunctionMemory 'free (cls value)
    return;
typeinline FunctionMemory 'allocate-array (cls T count)
    alloca-array T count
typeinline FunctionMemory 'free-array (cls value count)
    return;

let GlobalMemory = (typename "GlobalMemory" (super = Memory))
typeinline GlobalMemory 'allocate (cls T)
    static-alloc T
typeinline GlobalMemory 'free (cls value)
    return;
typeinline GlobalMemory 'allocate-array (cls T count)
    assert (constant? count) "count must be constant"
    bitcast
        static-alloc
            array T count
        'set-storage (pointer T 'mutable) 'Private
typeinline GlobalMemory 'free-array (cls value count)
    return;

typefn ref '__delete (self)
    let T = (typeof self)
    let ptrT = (storageof T)
    let class = ('storage ptrT)
    if (class == unnamed)
        'delete HeapMemory self
    elseif (class == 'Function)
        'delete FunctionMemory self
    elseif (class == 'Private)
        'delete GlobalMemory self
    else
        .. "cannot delete reference of type " (repr T)

#-------------------------------------------------------------------------------
# default constructors and destructors for basic types
#-------------------------------------------------------------------------------

do
    fn simple-new (self args...)
        let ET = (typeof& self)
        if (va-empty? args...)
            store (nullof ET) self
        else
            store (ET args...) self

    fn simple-delete (self)
        # todo: init value to deadbeef-style noise in debug mode?

    fn simple-copy (self other)
        let ET = (typeof& self)
        store (imply other ET) self

    fn simple-deref (self)
        load self

    fn setup-simple-type (T)
        set-type-symbol!& T '__new simple-new
        set-type-symbol!& T '__delete simple-delete
        set-type-symbol!& T '__copy simple-copy
        set-type-symbol!& T '__move simple-copy
        set-type-symbol!& T '__deref simple-deref

    setup-simple-type immutable
    setup-simple-type pointer

    set-type-symbol!& opaquepointer '__delete simple-delete
    set-type-symbol!& opaquepointer '__copy simple-copy
    set-type-symbol!& opaquepointer '__move simple-copy
    set-type-symbol!& opaquepointer '__deref simple-deref

#-------------------------------------------------------------------------------

inline supercall (cls methodname self args...)
    let cls = (imply cls type)
    let methodname = (imply methodname Symbol)
    let T = (typeof self)
    if (T < ref)
        assert ((typeof& self) <= cls)
        let superT = (superof cls)
        let f ok = (type@& superT methodname)
        if (not ok)
            compiler-error!
                .. "supertype " (repr superT) " of type " (repr cls)
                    \ " does not have a reference method " (repr methodname)
        f self args...
    else
        assert (T <= cls)
        let superT = (superof cls)
        (typeattr superT methodname) self args...

#-------------------------------------------------------------------------------
# default value constructors
#-------------------------------------------------------------------------------

inline local (T args...)
    FunctionMemory T args...

inline new (T args...)
    HeapMemory T args...

inline static (T args...)
    GlobalMemory T args...

inline delete (self)
    """"destructs and frees `value` of types that have the `__delete` method
        implemented. The free method must also invoke the destructor.
    let T = (typeof self)
    let op ok = (type@ T '__delete)
    if ok
        op self
        return;
    else
        .. "cannot delete value of type " (repr T)

#-------------------------------------------------------------------------------
# docstrings
#-------------------------------------------------------------------------------

fn docstring (f)
    let T = (typeof f)
    if (T == Closure)
        Label-docstring (Closure-label f)
    elseif (T == Scope)
        Scope-docstring f
    else
        compiler-error! "this type does have a docstring"

#-------------------------------------------------------------------------------
# null type
#-------------------------------------------------------------------------------

""""The type of the `null` constant. This type is uninstantiable.
let NullType = (typename "NullType")
set-typename-storage! NullType (pointer void)
set-type-symbol! NullType '__imply
    fn (self destT)
        if (pointer-type? destT)
            nullof destT
set-type-symbol! NullType '__==
    fn (a b flipped)
        if flipped
            if (pointer-type? (storageof (typeof a)))
                icmp== (ptrtoint a usize) 0:usize
        else
            if (pointer-type? (storageof (typeof b)))
                icmp== (ptrtoint b usize) 0:usize

""""A pointer constant of type `NullType` that is always zero and casts to
    any pointer type.
let null = (nullof NullType)

#-------------------------------------------------------------------------------
# module loading
#-------------------------------------------------------------------------------

define package
    let package = (Scope)
    set-scope-symbol! package 'path
        list
            .. compiler-dir "/lib/scopes/?.sc"
            .. compiler-dir "/lib/scopes/?/init.sc"
    set-scope-symbol! package 'modules (Scope)
    package

set-type-symbol! Scope '__..
    inline "Scope-join" (a b)
        """"Join two scopes ``a`` and ``b`` into a new scope so that the
            root of ``a`` descends from ``b``.
        fn clone-contents (a b)
            # search first upwards for the root scope of a, then clone a
                piecewise with the cloned scopes as parents
            let parent = (Scope-parent a)
            let b =
                if (parent == null) b
                else
                    clone-contents parent b
            Scope b a
        clone-contents (unconst a) (unconst b)

syntax-extend
    fn make-module-path (base-dir pattern name)
        let sz = (countof pattern)
        loop (i start result) = 0:usize 0:usize ""
        if (i == sz)
            return (.. result (slice pattern start))
        if ((@ pattern i) != (char "?"))
            repeat (i + 1:usize) start result
        else
            repeat (i + 1:usize) (i + 1:usize)
                .. result (slice pattern start i) name

    fn exec-module (expr eval-scope)
        let expr-anchor = (Syntax-anchor expr)
        let f = (compile (eval expr eval-scope))
        let rettype =
            element-type (element-type ('typeof f) 0) 0
        let ModuleFunctionType = (pointer (function (ReturnLabel (unknownof Any))))
        let fptr =
            if (rettype == Any)
                f as ModuleFunctionType
            else
                # build a wrapper
                let expr =
                    list
                        list let 'tmp '= (list f)
                        list unconst (list Any-new 'tmp)
                let expr = ((Syntax-wrap expr-anchor (Any expr) false) as Syntax)
                let f = (compile (eval expr (globals)))
                f as ModuleFunctionType
        fptr;

    fn dots-to-slashes (pattern)
        let sz = (countof pattern)
        loop (i start result) = 0:usize 0:usize ""
        if (i == sz)
            return (.. result (slice pattern start))
        let c = (@ pattern i)
        if (c == (char "/"))
            error!
                .. "no slashes permitted in module name: " pattern
        elseif (c == (char "\\"))
            error!
                .. "no slashes permitted in module name: " pattern
        elseif (c != (char "."))
            repeat (i + 1:usize) start result
        elseif (icmp== (i + 1:usize) sz)
            error!
                .. "invalid dot at ending of module '" pattern "'"
        else
            if (icmp== i start)
                if (icmp>u start 0:usize)
                    repeat (i + 1:usize) (i + 1:usize)
                        .. result (slice pattern start i) "../"
            repeat (i + 1:usize) (i + 1:usize)
                .. result (slice pattern start i) "/"

    fn load-module (module-name module-path opts...)
        if (not (file? module-path))
            error!
                .. "no such module: " module-path
        let module-path = (realpath module-path)
        let module-dir = (dirname module-path)
        let expr = (list-load module-path)
        let eval-scope = (va@ 'scope opts...)
        let eval-scope =
            if (none? eval-scope)
                Scope (globals)
            else eval-scope
        let main-module? = (va@ 'main-module? opts...)
        set-scope-symbol! eval-scope 'main-module?
            if (none? main-module?) false
            else main-module?
        set-scope-symbol! eval-scope 'module-path module-path
        set-scope-symbol! eval-scope 'module-dir module-dir
        set-scope-symbol! eval-scope 'module-name module-name
        let content = (exec-module expr (Scope eval-scope))
        return content (unconst true)

    fn patterns-from-namestr (base-dir namestr)
        # if namestr starts with a slash (because it started with a dot),
            we only search base-dir
        if ((@ namestr 0) == (char "/"))
            unconst
                list
                    .. base-dir "?.sc"
                    .. base-dir "?/init.sc"
        else
            let package = (unconst package)
            package.path as list

    let incomplete = (typename "incomplete")
    fn require-from (base-dir name)
        let name = (unconst name)
        let package = (unconst package)
        assert-typeof name Symbol
        let namestr = (Symbol->string name)
        let namestr = (dots-to-slashes namestr)
        inline load-module-from-symbol (name)
            let modules = (package.modules as Scope)
            let loop (patterns) = (patterns-from-namestr base-dir namestr)
            if (empty? patterns)
                return (unconst (Any none)) (unconst false)
            let pattern patterns = (decons patterns)
            let pattern = (pattern as string)
            let module-path = (realpath (make-module-path base-dir pattern namestr))
            if (empty? module-path)
                loop patterns
            let module-path-sym = (Symbol module-path)
            let content ok = (@ modules module-path-sym)
            if ok
                if (('typeof content) == type)
                    if (content == incomplete)
                        error!
                            .. "trying to import module " (repr name)
                                " while it is being imported"
                return content (unconst true)
            if (not (file? module-path))
                loop patterns
            set-scope-symbol! modules module-path-sym incomplete
            let content ok = (load-module (name as string) module-path)
            set-scope-symbol! modules module-path-sym content
            return content ok
        let content ok = (load-module-from-symbol name)
        if ok
            return content
        io-write! "no such module '"
        io-write! (Symbol->string name)
        io-write! "' in paths:\n"
        let loop (patterns) = (patterns-from-namestr base-dir namestr)
        if (empty? patterns)
            error! "failed to import module"
        let pattern patterns = (decons patterns)
        let pattern = (pattern as string)
        let module-path = (make-module-path base-dir pattern namestr)
        io-write! "    "
        io-write! module-path
        io-write! "\n"
        loop patterns

    set-scope-symbol! syntax-scope 'require-from require-from
    set-scope-symbol! syntax-scope 'load-module load-module
    syntax-scope

""""export locals as a chain of two new scopes: a scope that contains
    all the constant values in the immediate scope, and a scope that contains
    the runtime values.
define-scope-macro locals
    let docstr = (Scope-docstring syntax-scope unnamed)
    let constant-scope = (Scope)
    if (not (empty? docstr))
        set-scope-docstring! constant-scope unnamed docstr
    let tmp = (Parameter 'tmp)
    loop (last-key result) = unnamed (list tmp)
    let key value =
        Scope-next syntax-scope last-key
    if (key == unnamed)
        return
            cons do
                list let tmp '= (list Scope constant-scope)
                result
            syntax-scope
    else
        let keydocstr =
            Scope-docstring syntax-scope key
        repeat key
            if (key == unnamed)
                # skip
                result
            else
                let keyT = ('typeof value)
                if ((keyT == Parameter) or (keyT == Label))
                    cons
                        list set-scope-symbol! tmp (list quote key) value
                        list set-scope-docstring! tmp (list quote key) keydocstr
                        result
                else
                    set-scope-symbol! constant-scope key value
                    set-scope-docstring! constant-scope key keydocstr
                    result

define-macro import
    fn resolve-scope (scope namestr start)
        let sz = (countof namestr)
        loop (i start scope) = start start scope
        if (i == sz)
            return scope (Symbol (slice namestr start i))
        if ((@ namestr i) == (char "."))
            if (i == start)
                repeat (add i 1:usize) (add i 1:usize) scope
        repeat (add i 1:usize) start scope

    let sxname rest = (decons args)
    let name = (sxname as Syntax as Symbol)
    let namestr = (name as string)
    list syntax-extend
        list let 'scope 'key '=
            list resolve-scope 'syntax-scope namestr 0:usize
        list set-scope-symbol! 'scope 'key
            list 'require-from 'module-dir
                list quote name
        'syntax-scope

let i8* = (pointer i8)
let llvm.eh.sjlj.setjmp =
    extern 'llvm.eh.sjlj.setjmp (function i32 i8*)
let llvm.eh.sjlj.longjmp =
    extern 'llvm.eh.sjlj.longjmp (function void i8*)
let llvm.frameaddress =
    extern 'llvm.frameaddress (function i8* i32)
let llvm.stacksave =
    extern 'llvm.stacksave (function i8*)

inline xpcall (f errorf)
    let pad = (alloca-exception-pad)
    let old-pad =
        set-exception-pad pad
    if (== operating-system 'windows)
        if ((catch-exception pad (nullof i8*)) != 0)
            set-exception-pad old-pad
            errorf (exception-value pad)
        else
            let result... = (f)
            set-exception-pad old-pad
            result...
    else
        if ((catch-exception pad) != 0)
            set-exception-pad old-pad
            errorf (exception-value pad)
        else
            let result... = (f)
            set-exception-pad old-pad
            result...

#fn xpcall (f errorf)
    let pad = (alloca exception-pad-type)
    let old-pad =
        set-exception-pad pad
    let pad-target = (bitcast pad (pointer i8* 'mutable))
    store (llvm.frameaddress 0) pad-target
    store (llvm.stacksave) (getelementptr pad-target 2)
    if ((llvm.eh.sjlj.setjmp (bitcast pad-target i8*)) != 0)
        set-exception-pad old-pad
        errorf (exception-value pad)
    else
        let result... = (f)
        set-exception-pad old-pad
        result...

del i8*

fn format-exception (exc)
    if (('typeof exc) == Exception)
        let exc = (exc as Exception)
        format-message (Exception-anchor exc)
            .. (default-styler style-error "error:")
                \ " " (Exception-message exc)
    else
        .. "exception raised: " (repr exc) "\n"

fn prompt (prefix preload)
    __prompt prefix
        if (none? preload) ""
        else preload

#-------------------------------------------------------------------------------
# match
#-------------------------------------------------------------------------------

# earliest form of match macro - doesn't do elaborate patterns yet, just
    simple switch-case style comparisons
define-macro match
    let value rest = (decons args)
    let tmp = (Parameter 'tmp)

    fn vardef? (val)
        (('typeof val) == Symbol) and ((@ (val as Symbol as string) 0) == (char "$"))

    fn match-pattern (src sxkey)
        assert-typeof src Parameter
        assert-typeof sxkey Syntax
        let anchor = (sxkey as Anchor)
        let key = (sxkey as Any)
        let result =
            if (('typeof key) == list)
                let key = (key as list)
                if (empty? key)
                    list 'empty? src
                else
                    let head rest = (decons key)
                    let head = (head as Syntax as Symbol)
                    if (head == 'quote)
                        list '== src sxkey
                    # better support generic iterator
                    #elseif (head == 'list)
                        fn process-list-args (anchor src rest)
                            if (empty? rest)
                                list (list 'empty? src)
                            else
                                let tmp tmprest = (Parameter 'tmp) (Parameter 'rest)
                                let x rest = (decons rest)
                                cons
                                    list 'not (list 'empty? src)
                                    list do-in
                                        list let tmp tmprest '= (list 'decons src)
                                        match-pattern tmp (x as Syntax)
                                    process-list-args anchor tmprest rest
                        cons 'and
                            list '== (list typeof src) list
                            process-list-args anchor src rest
                    elseif (head == 'or)
                        if ((countof rest) < 2)
                            error! "'or' needs two arguments"
                        fn process-or-args (src rest)
                            let a rest = (decons rest)
                            if ((countof rest) <= 1)
                                let b = (decons rest)
                                list 'or
                                    match-pattern src (a as Syntax)
                                    match-pattern src (b as Syntax)
                            else
                                list 'or
                                    match-pattern src (a as Syntax)
                                    process-or-args src rest
                        process-or-args src rest
                    else
                        error!
                            .. "invalid pattern: " (repr key)
            elseif (vardef? key)
                let sym = (Symbol (slice (key as Symbol as string) 1))
                list do-in
                    list let sym '= src
                    true
            else
                # simple comparison
                list '== src key
        #print result
        Syntax-wrap anchor (Any result) false

    fn process (i src expr)
        if (empty? expr)
            error! "else expected"
        let pair rest = (decons expr)
        let key dst = (decons (pair as Syntax as list))
        let kkey = (key as Syntax as Any)
        let keytype = ('typeof kkey)
        if ((keytype == Symbol) and (kkey == 'else))
            cons (cons 'else dst) '()
        else
            cons
                cons
                    ? (i == 0) 'if 'elseif
                    match-pattern src (key as Syntax)
                    dst
                process (i + 1) src rest
    cons do
        list let tmp '= value
        process (unconst 0) tmp rest

#-------------------------------------------------------------------------------
# various C related sugar
#-------------------------------------------------------------------------------

# labels safecast to function pointers
typefn Closure '__imply (self destT)
    if (function-pointer-type? destT)
        let ET = (rawcall element-type destT 0)
        let sz = (itrunc (rawcall type-countof ET) i32)
        if (rawcall function-type-variadic? ET)
            compiler-error! "cannot typify to variadic function"
        let loop (i args...) = sz
        if (icmp== i 1)
            let result =
                unconst (typify self args...)
            if (destT != (typeof result))
                syntax-error! (Label-anchor (Closure-label self))
                    .. "function does not compile to type " (repr destT)
                        \ " but has type " (repr (typeof result))
            return result
        else
            let i-1 = (sub i 1)
            loop i-1 (rawcall element-type ET i-1) args...

# pointer comparisons
typeinline pointer '__== (a b flipped)
    if flipped
        icmp== (ptrtoint (a as (typeof b)) usize) (ptrtoint b usize)
    else
        icmp== (ptrtoint a usize) (ptrtoint (b as (typeof a)) usize)

# pointer cast to element type executes load
typeinline pointer '__as (self destT)
    if (type== destT (element-type (typeof self) 0))
        load self

# also supports mutable pointer safecast to immutable pointer
typeinline pointer '__imply (self destT)
    if (type== destT ref)
        bitcast self (ref (typeof self))
    elseif (pointer-type-imply? (typeof self) destT)
        bitcast self destT

# support getattr syntax
typefn pointer '__getattr (self name)
    let ET = (element-type (typeof self) 0)
    let op success = (type@& ET '__getattr)
    if success
        let result... = (op (self as ref) name)
        if (va-empty? result...)
        else
            return result...
    forward-getattr (load self) name

typefn& pointer '__getattr (self name)
    '__getattr (deref self) name

# support @
typeinline pointer '__@ (self index)
    let index =
        if (none? index) 0:usize # simple dereference
        else index
    (getelementptr self (usize index)) as ref

# extern cast to element type/pointer executes load/unconst
typeinline extern '__imply (self destT)
    let ET = (element-type (typeof self) 0)
    if (type== destT ET)
        unconst self
    else
        forward-imply (load self) destT

typeinline extern '__getattr (self name)
    let T = (typeof self)
    let pET = (element-type T 0)
    let ET = (element-type pET 0)
    let op success = (type@& ET '__getattr)
    if success
        let result... = (op (unconst (bitcast self (storageof T))) name)
        if (va-empty? result...)
        else
            return result...
    forward-getattr (load self) name

typeinline extern '__as (self destT)
    forward-as (load self) destT

# support assignment syntax for extern
typeinline extern '__= (self value)
    let ET = (element-type (element-type (typeof self) 0) 0)
    store (imply value ET) self
    true

# support @ for extern
typeinline extern '__@ (self value)
    @ (unconst self) value

do
    fn get-op (self op)
        let self = (load self)
        let op ok = (type@ (typeof self) op)
        return self op ok
    fn forward-op (op)
        typefn extern op (a b flipped)
            let a b op ok =
                if flipped
                    _ a (get-op b op)
                else
                    let a op ok = (get-op a op)
                    _ a b op ok
            if ok
                op a b flipped
    let ops... = '__* '__/ '__// '__+ '__- '__**
    let loop (i) = (va-countof ops...)
    if (i > 0)
        let i = (i - 1)
        forward-op
            va@ i ops...
        loop i

do
    fn unenum (val)
        let T = (typeof val)
        if (T < CEnum)
            bitcast val (storageof T)
        else val

    # support for downcast
    typefn CEnum '__imply (self destT)
        let ST = (storageof (typeof self))
        if (type== destT ST)
            bitcast self ST
        elseif (type== destT i32)
            bitcast self i32

    typefn CEnum '__as (self destT)
        let ST = (storageof (typeof self))
        if (type== destT integer)
            bitcast self ST

    fn passthru-overload (sym func)
        set-type-symbol! CEnum sym (fn (a b flipped) (func (unenum a) (unenum b)))
    fn passthru-overload1 (sym func)
        set-type-symbol! CEnum sym (fn (self) (func (unenum self)))

    passthru-overload '__!= !=; passthru-overload '__== ==
    passthru-overload '__< <; passthru-overload '__<= <=
    passthru-overload '__> >; passthru-overload '__>= >=
    passthru-overload '__+ +; passthru-overload '__- -
    passthru-overload '__* *; passthru-overload '__/ /
    passthru-overload '__// //; passthru-overload '__% %
    passthru-overload '__<< <<; passthru-overload '__>> >>
    passthru-overload '__| |
    passthru-overload '__^ ^
    passthru-overload '__& &
    passthru-overload1 '__~ ~
    passthru-overload1 '__rcp /
    passthru-overload1 '__neg -

typefn CStruct 'structof (cls args...)
    let sz = (va-countof args...)
    if (icmp== sz 0)
        nullof cls
    else
        let T = (storageof cls)
        let keys... = (va-keys args...)
        let loop (i instance) = 0 (nullof cls)
        if (icmp<s i sz)
            let key = (va@ i keys...)
            let arg = (va@ i args...)
            let k =
                if (key == unnamed) i
                else
                    element-index T key
            let ET = (element-type T k)
            loop (add i 1)
                insertvalue instance (imply arg ET) k
        else
            instance

inline CStruct->tuple (self)
    bitcast& self (storageof (@ (typeof self)))

typefn& CStruct '__new (self args...)
    let sz = (va-countof args...)
    let T = (typeof& self)
    # construct as tuple
    construct
        CStruct->tuple self
    if (icmp>s sz 0)
        # todo: do a bit more efficient initialization, as right now we're first
            default-initing, and then rewriting some of the values anyways
        let keys... = (va-keys args...)
        let loop (i) = 0
        if (icmp<s i sz)
            let key = (va@ i keys...)
            let arg = (va@ i args...)
            let k =
                if (key == unnamed) i
                else
                    element-index T key
            =
                (getelementptr self 0 k) as ref
                arg
            loop (i + 1)

typefn& CStruct '__copy (self other)
    let ET = (typeof& self)
    let self = (CStruct->tuple self)
    let otherT = (typeof other)
    let other =
        if (otherT == ET)
            bitcast other (storageof ET)
        elseif (otherT < ref and (typeof& other) == ET)
            CStruct->tuple other
        else
            compiler-error!
                .. "can't copy-construct " (repr ET)
                    \ " from type " (repr (typeof other))
    copy-construct self other

typeinline& CStruct '__delete (self)
    destruct
        CStruct->tuple self

# support for C struct initializers
typeinline CStruct '__typecall (cls args...)
    if (cls == CStruct)
        compiler-error! "CStruct type constructor is deprecated"
    else
        'structof cls args...

# access reference to struct element from pointer/reference
typeinline& CStruct '__getattr (self name)
    let ET = (element-type (typeof self) 0)
    let idx = (element-index ET name)
    if (icmp>=s idx 0)
        # cast result to reference
        (getelementptr self 0 idx) as ref

typeinline CStruct '__getattr (self name)
    let idx = (element-index (typeof self) name)
    if (icmp>=s idx 0)
        extractvalue self idx

#-------------------------------------------------------------------------------

# support for basic C union initializer
typefn CUnion '__typecall (cls)
    nullof cls

typefn& CUnion '__new (self)
    let cls = (@ (typeof self))
    store (nullof cls) self

typefn& CUnion '__copy (self other)
    let other =
        if ((typeof other) < ref) (load other)
        else other
    store other self

typefn& CUnion '__delete (self)

# access reference to union element from pointer/reference
typeinline& CUnion '__getattr (self name)
    let ET = (element-type (typeof self) 0)
    let idx = (element-index ET name)
    if (icmp>=s idx 0)
        let FT = (element-type ET idx)
        let newPT =
            'set-element-type (storageof (typeof self)) FT
        # cast pointer to reference to alternative type
        (bitcast self newPT) as ref

typeinline CUnion '__getattr (self name)
    let idx = (element-index (typeof self) name)
    if (icmp>=s idx 0)
        extractvalue self idx

# extern call attempts to cast arguments to correct type
typefn extern '__call (self ...)
    label docall (dest ET)
        let sz = (va-countof ...)
        let count = (itrunc (rawcall type-countof ET) i32)
        let variadic = (rawcall function-type-variadic? ET)
        let loop (i args...) = sz
        if (icmp== i 0)
            return
                rawcall dest args...
        else
            let i-1 = (sub i 1)
            let arg = (va@ i-1 ...)
            if ((not variadic) or (icmp<s i count))
                let argtype = (rawcall element-type ET i)
                loop i-1 (imply arg argtype) args...
            else
                loop i-1 arg args...

    let pET = (rawcall element-type (typeof self) 0)
    let ET = (rawcall element-type pET 0)
    let ST = (rawcall superof ET)
    if (type== ST function)
        docall self ET
    elseif (function-pointer-type? ET) # can also call pointer to pointer to function
        docall (load self) (rawcall element-type ET 0)
    else
        (load self) ...

#-------------------------------------------------------------------------------
# using
#-------------------------------------------------------------------------------

fn merge-scope-symbols (source target filter)
    fn filter-contents (source target filter)
        let parent = (Scope-parent source)
        let target =
            if (parent == null) target
            else
                filter-contents parent target filter
        let loop (last-key) = (unconst unnamed)
        let key value =
            Scope-next source last-key
        if (key != unnamed)
            if
                or
                    none? filter
                    do
                        let keystr = (key as string)
                        string-match? filter keystr
                set-scope-symbol! target key value
            loop key
        else
            target
    filter-contents (unconst source) (unconst target) filter

define-scope-macro using
    let name rest = (decons args)
    let nameval = (name as Syntax as Any)
    if ((('typeof nameval) == Symbol) and ((nameval as Symbol) == 'import))
        let module-dir ok = (@ syntax-scope 'module-dir)
        if (not ok)
            error! "using import requires module-dir symbol in scope"
        let module-dir = (module-dir as string)
        let name rest = (decons rest)
        let name = (name as Syntax as Symbol)
        let module = ((require-from module-dir name) as Scope)
        return (unconst (list do))
            .. module syntax-scope
    let pattern =
        if (empty? rest)
            unconst '()
        else
            let token pattern rest = (decons rest 2)
            let token = (token as Syntax as Symbol)
            if (token != 'filter)
                syntax-error! (active-anchor)
                    "syntax: using <scope> [filter <filter-string>]"
            let pattern = (pattern as Syntax as string)
            list pattern
    # attempt to import directly if possible
    label process (src)
        return (unconst (list do))
            if (empty? pattern)
                merge-scope-symbols src syntax-scope
            else
                merge-scope-symbols src syntax-scope ((@ pattern 0) as string)
    if (('typeof nameval) == Symbol)
        let sym = (nameval as Symbol)
        let src ok = (@ syntax-scope sym)
        if (ok and (('typeof src) == Scope))
            process (src as Scope)
    elseif (('typeof nameval) == Scope)
        process (nameval as Scope)
    return
        list syntax-extend
            cons merge-scope-symbols name 'syntax-scope pattern
        syntax-scope

define-macro from
    inline load-from (src keys...)
        let loop (i result...) = (va-countof keys...)
        if (i == 0)
            result...
        else
            let i = (i - 1)
            let key = (va@ i keys...)
            loop i
                src @ key
                result...
    let src kw params = (decons args 2)
    if ((kw as Syntax as Symbol) != 'let)
        syntax-error! kw "`let` keyword expected"
    fn quotify (params)
        if (empty? params)
            unconst '()
        else
            let entry rest = (decons params)
            entry as Syntax as Symbol
            cons
                list quote entry
                quotify rest
    cons let
        .. params
            list '=
                cons load-from src
                    quotify params

#-------------------------------------------------------------------------------
# struct declaration
#-------------------------------------------------------------------------------

define-scope-macro struct
    inline begin-arg ()
    inline end-args (f) (f)
    inline append-arg (prevf x...)
        inline (f)
            prevf
                inline ()
                    return x... (f)

    define struct-dsl
        define-block-scope-macro :
            let args = (list-next expr)
            let lhs rhs = (decons args 2)
            lhs as Syntax as Symbol
            return
                cons
                    list let 'field-types '=
                        list append-arg 'field-types (list lhs '= rhs)
                    next-expr
                syntax-scope
        define-macro method
            let name params body = (decons args 2)
            list set-type-symbol! 'this-struct name
                cons fn params body
        define-macro method&
            let name params body = (decons args 2)
            list set-type-symbol!& 'this-struct name
                cons fn params body
        define-macro inlinemethod
            let name params body = (decons args 2)
            list set-type-symbol! 'this-struct name
                cons inline params body
        define-macro inlinemethod&
            let name params body = (decons args 2)
            list set-type-symbol!& 'this-struct name
                cons inline params body

        define-infix> 70 :
        locals;

    fn finalize-struct (T field-types)
        set-typename-storage! T
            call
                if (T < CStruct)
                    tuple
                else
                    union
                field-types begin-arg
        T

    let head body = (decons args)
    let head = (head as Syntax)
    let any-head = (head as Any)
    fn complete-declaration ()
    inline generate-expression (superT name body)
        if (('typeof (name as Any)) == Symbol)
            # constant
            let symname = (name as Any as Symbol)
            # see if we can find a forward declaration in the local scope
            let T ok = (Scope-local@ syntax-scope symname)
            fn completable-type? (T)
                and
                    (typeof T) == type
                    typename-type? T
                    opaque? T
                    (superof T) == typename

            return
                if (empty? body)
                    # forward declaration
                    list let name '=
                        list do
                            list using struct-dsl
                            list let 'this-struct '=
                                list typename-type
                                    symname as string
                            'this-struct
                else
                    # body declaration
                    list let name '=
                        cons do
                            list using struct-dsl
                            list let 'this-struct '=
                                list if (list completable-type? T) T
                                list 'else
                                    list typename-type
                                        symname as string
                            list let name '= 'this-struct
                            list set-typename-super! 'this-struct superT
                            list let 'field-types '= end-args
                            ..
                                cons do body
                                list
                                    list finalize-struct 'this-struct 'field-types
                syntax-scope
        else
            # expression
            return
                cons do
                    list using struct-dsl
                    list let 'this-struct '=
                        list typename-type
                            list (do as) name string
                    list set-typename-super! 'this-struct superT
                    list let 'field-types '= end-args
                    ..
                        cons do body
                        list
                            list finalize-struct 'this-struct 'field-types
                syntax-scope

    if (('typeof any-head) == Symbol and any-head as Symbol == 'union)
        let head body = (decons body)
        generate-expression CUnion (head as Syntax) body
    else
        generate-expression CStruct head body

#-------------------------------------------------------------------------------
# enum declaration
#-------------------------------------------------------------------------------

define-scope-macro enum
    fn make-enum (T vals...)
        set-typename-super! T CEnum
        set-typename-storage! T i32
        let count = (va-countof vals...)
        let keys... = (va-keys vals...)
        let loop (i nextval) = 0 0
        if (i < count)
            let key = (va@ i keys...)
            let val = (va@ i vals...)
            if (not (constant? val))
                compiler-error! "all enum values must be constant"
            loop (i + 1)
                if (key == unnamed)
                    # auto-numerical
                    set-type-symbol! T val (bitcast (imply nextval i32) T)
                    nextval + 1
                else
                    set-type-symbol! T key (bitcast (imply val i32) T)
                    val + 1
        T

    fn convert-body (body)
        let expr body = (decons body)
        let anyexpr = (expr as Syntax as Any)
        cons
            if (('typeof anyexpr) == Symbol)
                Any (list quote expr)
            else expr
            if (empty? body)
                unconst '()
            else
                convert-body body

    let name body = (decons args)
    let anyname = (name as Syntax as Any)
    let newbody = (convert-body body)
    return
        if (('typeof anyname) == Symbol)
            let namestr = (anyname as Symbol as string)
            list let name '=
                cons make-enum (list typename-type namestr) newbody
        else
            cons make-enum (list typename-type (list (do as) name string)) newbody
        syntax-scope

#-------------------------------------------------------------------------------
# none
#-------------------------------------------------------------------------------

typefn Nothing '__hash (self)
    hash Nothing 0

#-------------------------------------------------------------------------------
# aggregate
#-------------------------------------------------------------------------------

typefn& aggregate '__deref (self)
    load self

#-------------------------------------------------------------------------------
# tuples
#-------------------------------------------------------------------------------

do
    inline tuple-each (f)
        fn (self)
            let ET = (typeof& self)
            let count = (i32 (type-countof ET))
            let loop (i) = 0
            if (icmp<s i count)
                f ((getelementptr self 0 i) as ref)
                loop (i + 1)

    inline tuple-each2 (f)
        fn (self other)
            let ET = (typeof& self)
            let count = (i32 (type-countof ET))
            if ((typeof other) < ref)
                assert ((typeof& other) == ET)
                let loop (i) = 0
                if (icmp<s i count)
                    f
                        (getelementptr self 0 i) as ref
                        (getelementptr other 0 i) as ref
                    loop (i + 1)
            else
                # copy right-hand side by element
                let other = (imply other ET)
                let loop (i) = 0
                if (icmp<s i count)
                    f
                        (getelementptr self 0 i) as ref
                        extractvalue other i
                    loop (i + 1)

    set-type-symbol!& tuple '__new (tuple-each construct)
    set-type-symbol!& tuple '__delete (tuple-each destruct)
    set-type-symbol!& tuple '__copy (tuple-each2 copy-construct)
    set-type-symbol!& tuple '__move (tuple-each2 move-construct)

typefn tuple '__hash (self)
    # hash all tuple values
    let T = (typeof self)
    let count = (type-countof T)
    let loop (i result) = 0:usize (hash tuple)
    if (icmp<s i count)
        loop (i + 1:usize)
            hash result (extractvalue self i)
    else
        result

typefn tuple '__countof (self)
    countof (typeof self)

typefn tuple '__@ (self at)
    let val = (at as integer)
    extractvalue self val

typeinline& tuple '__@ (self at)
    let val = (at as integer)
    (getelementptr self 0 val) as ref

typefn tuple '__unpack (self)
    let T = (typeof self)
    let count = (type-countof T)
    let loop (i result...) = count
    if (i == 0) result...
    else
        let i = (sub i 1)
        loop i
            va-key
                element-name T i
                extractvalue self i
            result...

typeinline& tuple '__unpack (self)
    let T = (typeof& self)
    let count = (type-countof T)
    let loop (i result...) = count
    if (i == 0) result...
    else
        let i = (sub i 1)
        loop i
            va-key
                element-name T i
                self @ i
            result...

# access reference to struct element from pointer/reference
typefn& tuple '__getattr (self name)
    let ET = (element-type (typeof self) 0)
    let idx = (element-index ET name)
    if (icmp>=s idx 0)
        # cast result to reference
        let val = (getelementptr self 0 idx)
        ref val

typefn tuple '__getattr (self name)
    let idx = (element-index (typeof self) name)
    if (icmp>=s idx 0)
        extractvalue self idx

fn tupleof (...)
    let sz = (va-countof ...)
    let keys... = (va-keys ...)
    # build tuple type
    let loop (i result...) = sz
    if (icmp>s i 0)
        let i = (sub i 1)
        let T = (va@ i ...)
        let key = (va@ i keys...)
        loop i (va-key key (typeof T)) result...
    else
        # build tuple
        let loop (i result) = 0 (nullof (tuple result...))
        if (icmp<s i sz)
            let T = (va@ i ...)
            loop (add i 1)
                insertvalue result T i
        else
            result

#-------------------------------------------------------------------------------
# compile time closures
#-------------------------------------------------------------------------------

let Capture = (typename "Capture")
let MutableCapture = (typename "Capture" Capture)

typefn& MutableCapture '__copy (self other)
    (type@& tuple '__copy) self other

define-macro capture
    fn make-typename (TT)
        let T =
            typename
                .. "Capture"
                    string-repr TT
                super = Capture
                storage = TT
        T
    fn convert (self TT)
        unpack (bitcast self TT)
    fn convert& (self TT)
        unpack (bitcast& self TT)

    let args params body = (decons args 2)
    let arglist = (args as Syntax as Any as list)
    let head arglist = (decons arglist)
    if
        or (('typeof (head as Syntax as Any)) != Symbol)
            ((head as Syntax as Symbol) != 'square-list)
        syntax-error! head "square brackets expected"
    let params = (params as Syntax as list)
    let T = (Parameter 'T)
    let EV = (Parameter 'EV)
    let TT = (Parameter 'TT)
    let self = (Parameter 'self)
    list do
        list let EV '=
            cons tupleof arglist
        list let TT '= (list typeof EV)
        list let T '=
            list make-typename TT
                list fn '()
        list set-type-symbol! T (list quote '__call)
            cons fn (cons self params)
                cons let
                    .. arglist
                        list '= (list convert self TT)
                body
        list bitcast EV T

define-macro capture&
    fn make-typename (TT)
        let T =
            typename
                .. "MutableCapture"
                    string-repr TT
                super = MutableCapture
                storage = TT
        T
    inline convert& (self TT)
        unpack (bitcast& self TT)

    let args params body = (decons args 2)
    let arglist = (args as Syntax as Any as list)
    let head arglist = (decons arglist)
    if
        or (('typeof (head as Syntax as Any)) != Symbol)
            ((head as Syntax as Symbol) != 'square-list)
        syntax-error! head "square brackets expected"
    let params = (params as Syntax as list)
    let T = (Parameter 'T)
    let EV = (Parameter 'EV)
    let TT = (Parameter 'TT)
    let self = (Parameter 'self)
    list do
        list let EV '=
            cons tupleof arglist
        list let TT '= (list typeof EV)
        list let T '=
            list make-typename TT
                list fn '()
        list set-type-symbol!& T (list quote '__call)
            cons fn (cons self params)
                cons let
                    .. arglist
                        list '= (list convert& self TT)
                body
        list local (list quote 'copy)
            list bitcast EV T

#-------------------------------------------------------------------------------
# arrays
#-------------------------------------------------------------------------------

do
    inline array-each (f)
        fn (self)
            let ET = (typeof& self)
            let count = (type-countof ET)
            f count ((getelementptr self 0 0) as ref)

    inline array-each2 (fsingle fmany)
        fn (self other)
            let ET = (typeof& self)
            let count = (type-countof ET)
            if ((typeof other) < ref)
                assert ((typeof& other) == ET)
                fmany count
                    (getelementptr self 0 0) as ref
                    (getelementptr other 0 0) as ref
            else
                # copy right-hand side by element
                let other = (imply other ET)
                let loop (i) = 0
                if (icmp<s i count)
                    fsingle
                        (getelementptr self 0 i) as ref
                        extractvalue other i
                    loop (i + 1)

    set-type-symbol!& array '__new (array-each construct-array)
    set-type-symbol!& array '__delete (array-each destruct-array)
    set-type-symbol!& array '__copy (array-each2 copy-construct copy-construct-array)
    set-type-symbol!& array '__move (array-each2 move-construct move-construct-array)

typefn array '__countof (self)
    countof (typeof self)

typefn array '__unpack (self)
    let count = (type-countof (typeof self))
    let loop (i result...) = count
    if (i == 0) result...
    else
        let i = (sub i 1)
        loop i
            extractvalue self i
            result...

typefn& array '__unpack (self)
    let count = (type-countof (typeof& self))
    let loop (i result...) = count
    if (i == 0) result...
    else
        let i = (sub i 1)
        loop i
            self @ i
            result...

typefn& array '__imply (self destT)
    let ET = (@ (typeof& self))
    if ((destT < pointer) and ((@ destT) == ET))
        bitcast self destT

typefn array '__@ (self at)
    let val = (at as integer)
    if (constant? val)
        extractvalue self val
    else
        compiler-error! "index into immutable array must be constant"

typeinline& array '__@ (self at)
    let val = (at as integer)
    (getelementptr self 0 val) as ref

fn arrayof (T ...)
    let count = (va-countof ...)
    let loop (i result) = 0 (nullof (array T (usize count)))
    if (i < count)
        let element = (va@ i ...)
        loop (i + 1)
            insertvalue result (imply element T) i
    else result

#-------------------------------------------------------------------------------
# iterators
#-------------------------------------------------------------------------------

let Generator = (typename "Generator")
set-typename-storage! Generator (storageof Closure)
typeinline Generator '__typecall (cls iter init)
    inline get-iter-init ()
        return iter init
    bitcast get-iter-init Generator
typeinline Generator '__call (self)
    if (not (constant? self))
        compiler-error! "Generator must be constant"
    let f = (bitcast self Closure)
    call f

typeinline typename 'symbols (self)
    Generator
        label (fret fdone key)
            let key value =
                type-next self key
            if (key == unnamed)
                fdone;
            else
                fret key key value
        unconst unnamed

typeinline typename 'elements (self)
    let count =
        type-countof self
    Generator
        label (fret fdone i)
            if (i == count)
                fdone;
            else
                fret (i + 1) (element-type self i)
        tie-const self 0

typeinline Scope '__as (self destT)
    if (destT == Generator)
        Generator
            label (fret fdone key)
                let key value =
                    Scope-next self key
                if (key == unnamed)
                    fdone;
                else
                    fret key key value
            unconst unnamed

typeinline list '__as (self destT)
    if (destT == Generator)
        Generator
            label (fret fdone cell)
                if (empty? cell)
                    fdone;
                else
                    let at next = (decons cell)
                    fret next at
            self

inline va-each (values...)
    let count = (va-countof values...)
    Generator
        label (fret fdone i)
            if (i == count)
                fdone;
            else
                fret (add i 1) (va@ i values...)
        0

inline va-each-reversed (values...)
    let count = (va-countof values...)
    Generator
        label (fret fdone i)
            if (i == 0)
                fdone;
            else
                let i = (sub i 1)
                fret i (va@ i values...)
        count

inline range (a b c)
    let num-type = (typeof a)
    let step =
        if (c == none)
            num-type 1
        else c
    let from =
        if (b == none)
            num-type 0
        else a
    let to =
        if (b == none) a
        else b
    Generator
        label (f fdone x)
            if (x < to)
                f (x + step) x
            else
                fdone;
        unconst from

inline multirange (size...)
    let dims = (va-countof size...)
    let size = (* size...)
    let ET = (typeof size)
    let one = (ET 1)
    Generator
        label (fret fdone x)
            if (x == size)
                fdone;
            else
                let repeat (i k result...) = 0 x
                if (i < dims)
                    let D = (va@ i size...)
                    let u = (k % D)
                    repeat (i + 1) ((k - u) // D) ((va-join result...) u)
                fret (x + one) result...
        unconst
            ET 0

inline unroll-range (a b c)
    let num-type = (typeof a)
    let step =
        if (c == none)
            num-type 1
        else c
    let from =
        if (b == none)
            num-type 0
        else a
    let to =
        if (b == none) a
        else b
    assert (constant? to)
    assert (constant? from)
    assert (constant? step)
    Generator
        label (f fdone x)
            if (x < to)
                f (x + step) x
            else
                fdone;
        from

inline zip (a b)
    let iter-a init-a = ((a as Generator))
    let iter-b init-b = ((b as Generator))
    Generator
        label (fret fdone t)
            let a = (@ t 0)
            let b = (@ t 1)
            iter-a
                label (next-a at-a...)
                    iter-b
                        label (next-b at-b...)
                            fret
                                tupleof next-a next-b
                                \ at-a... at-b...
                        \ fdone b
                \ fdone a
        tupleof init-a init-b

inline map (x f)
    """"Maps function `f (skip values...)` to elements of iterable `x`.

        `skip` is a function that can be called to purge the active element
        from the output (allowing map to also act as a filter).
    let iter init = ((x as Generator))
    Generator
        label (fret fdone value)
            let skip (value) = value
            iter
                label (next values...)
                    fret next
                        f
                            label ()
                                skip next
                            values...
                fdone
                value
        init

inline enumerate (x)
    zip
        unroll-range 0x7fffffff
        x as Generator

inline fold (init gen f)
    let iter start = ((gen as Generator))
    let loop (result next) = init start
    label break ()
        return result
    iter
        label (next args...)
            loop
                f break result args...
                next
        \ break next

define-scope-macro breakable-block
    let old-recur recur-ok = (@ syntax-scope 'recur)
    let old-return ok = (@ syntax-scope 'return)
    return
        list
            cons inline "breakable-block" '()
                list let 'break '= 'return
                if ok
                    list let 'recur '= old-recur
                else
                    unconst (list del 'recur)
                if ok
                    list let 'return '= old-return
                else
                    unconst (list del 'return)
                args
        syntax-scope

define-macro for
    loop (it params) = args '()
    if (empty? it)
        error! "'in' expected"
    let sxat it = (decons it)
    let at = (sxat as Syntax as Symbol)
    if (at != 'in)
        repeat it (cons sxat params)
    let generator-expr body = (decons it)
    let params = (list-reverse params)
    let iter = (Parameter 'iter)
    let start = (Parameter 'start)
    let next = (Parameter 'next)
    let loopsym = (Symbol "#loop")
    list breakable-block
        list let iter start '= (list (list (do as) generator-expr Generator))
        list iter
            list label loopsym (cons next params)
                list label 'continue '()
                    list iter loopsym 'break next
                cons do body
                list 'continue
            \ 'break start

define-macro while
    let cond-expr body = (decons args)
    list breakable-block
        list let 'continue '()
        list if (list unconst cond-expr)
            cons do body
            list 'continue

#-------------------------------------------------------------------------------
# search utilities
#-------------------------------------------------------------------------------

fn bsearch (arr value)
    """"binary-search array-like value `arr` for `value`
    let count = (countof arr)
    let T = (typeof count)
    let one = (T 1)
    loop (lo hi) = (T 0) (count - one)
    let med = ((lo + hi) >> one)
    if (lo > hi)
        return med (unconst false)
    let at = (arr @ med)
    if (at == value)
        return med (unconst true)
    elseif (value > at)
        repeat (med + one) hi
    else
        repeat lo (med - one)

#-------------------------------------------------------------------------------
# label utilities
#-------------------------------------------------------------------------------

typefn Label 'parameters (self)
    let count = (Label-parameter-count self)
    Generator
        label (fret fdone x)
            if (x == count)
                fdone;
            else
                fret (x + 1) (Label-parameter self x)
        tie-const self 0:usize

#-------------------------------------------------------------------------------
# vectors
#-------------------------------------------------------------------------------

fn vectorof (T ...)
    let count = (va-countof ...)
    let loop (i result) = 0 (nullof (vector T (usize count)))
    if (i < count)
        let element = (va@ i ...)
        loop (i + 1)
            insertelement result (imply element T) i
    else result

inline vector-signed-dispatch (fsigned funsigned)
    fn (a b)
        if (signed? (element-type (typeof a) 0))
            fsigned a b
        else
            funsigned a b

set-type-symbol! integer '__vector+ add
set-type-symbol! integer '__vector- sub
set-type-symbol! integer '__vector* mul
set-type-symbol! integer '__vector// (vector-signed-dispatch sdiv udiv)
set-type-symbol! integer '__vector% (vector-signed-dispatch srem urem)
set-type-symbol! integer '__vector& band
set-type-symbol! integer '__vector| bor
set-type-symbol! integer '__vector^ bxor
set-type-symbol! integer '__vector<< shl
set-type-symbol! integer '__vector>> (vector-signed-dispatch ashr lshr)
set-type-symbol! integer '__vector== icmp==
set-type-symbol! integer '__vector!= icmp!=
set-type-symbol! integer '__vector> (vector-signed-dispatch icmp>s icmp>u)
set-type-symbol! integer '__vector>= (vector-signed-dispatch icmp>s icmp>=u)
set-type-symbol! integer '__vector< (vector-signed-dispatch icmp<s icmp<u)
set-type-symbol! integer '__vector<= (vector-signed-dispatch icmp<=s icmp<=u)

set-type-symbol! real '__vector+ fadd
set-type-symbol! real '__vector- fsub
set-type-symbol! real '__vector* fmul
set-type-symbol! real '__vector/ fdiv
set-type-symbol! real '__vector% frem
set-type-symbol! real '__vector== fcmp==o
set-type-symbol! real '__vector!= fcmp!=u
set-type-symbol! real '__vector> fcmp>o
set-type-symbol! real '__vector>= fcmp>=o
set-type-symbol! real '__vector< fcmp<o
set-type-symbol! real '__vector<= fcmp<=o

inline vector-op2-dispatch (symbol)
    fn (a b flipped)
        if (type== (typeof a) (typeof b))
            let Ta = (element-type (typeof a) 0)
            let op success = (type@ Ta symbol)
            if success
                let result... = (op a b)
                if (icmp== (va-countof result...) 0)
                else
                    return result...

set-type-symbol! vector '__+ (vector-op2-dispatch '__vector+)
set-type-symbol! vector '__- (vector-op2-dispatch '__vector-)
set-type-symbol! vector '__* (vector-op2-dispatch '__vector*)
set-type-symbol! vector '__/ (vector-op2-dispatch '__vector/)
set-type-symbol! vector '__// (vector-op2-dispatch '__vector//)
set-type-symbol! vector '__% (vector-op2-dispatch '__vector%)
set-type-symbol! vector '__& (vector-op2-dispatch '__vector&)
set-type-symbol! vector '__| (vector-op2-dispatch '__vector|)
set-type-symbol! vector '__^ (vector-op2-dispatch '__vector^)
set-type-symbol! vector '__== (vector-op2-dispatch '__vector==)
set-type-symbol! vector '__!= (vector-op2-dispatch '__vector!=)
set-type-symbol! vector '__> (vector-op2-dispatch '__vector>)
set-type-symbol! vector '__>= (vector-op2-dispatch '__vector>=)
set-type-symbol! vector '__< (vector-op2-dispatch '__vector<)
set-type-symbol! vector '__<= (vector-op2-dispatch '__vector<=)

typefn vector '__countof (self)
    type-countof (typeof self)

typefn vector '__repr (self)
    let count = (type-countof (typeof self))
    let loop (i result) = 0:usize ""
    if (i < count)
        loop (i + 1:usize)
            .. result
                if (i == 0:usize) "<"
                else " "
                repr (extractelement self i)
    else
        .. result ">"

typefn vector '__unpack (v)
    let count = (type-countof (typeof v))
    let loop (i result...) = count
    if (i == 0) result...
    else
        let i = (sub i 1)
        loop i
            extractelement v i
            result...

typefn vector '__@ (self x)
    if ((typeof x) < integer)
        extractelement self x

typefn vector '__slice (self i0 i1)
    if ((constant? i0) and (constant? i1))
        let usz = (sub i1 i0)
        let loop (i mask) = i0 (nullof (vector i32 usz))
        if (icmp<u i i1)
            loop (add i 1:usize) (insertelement mask (i32 i) (sub i i0))
        else
            shufflevector self self mask
    else
        compiler-error! "slice indices must be constant"

fn vector-reduce (f v)
    let loop (v) = v
    let sz = (countof v)
    # special cases for low vector sizes
    if (sz == 1:usize)
        extractelement v 0
    elseif (sz == 2:usize)
        f
            extractelement v 0
            extractelement v 1
    elseif (sz == 3:usize)
        f
            f
                extractelement v 0
                extractelement v 1
            extractelement v 2
    elseif (sz == 4:usize)
        f
            f
                extractelement v 0
                extractelement v 1
            f
                extractelement v 2
                extractelement v 3
    else
        let hsz = (sz >> 1:usize)
        let fsz = (hsz << 1:usize)
        if (fsz != sz)
            compiler-error! "vector size must be a power of two"
        let a = (slice v 0 hsz)
        let b = (slice v hsz fsz)
        loop (f a b)

fn any? (v)
    vector-reduce bor v
fn all? (v)
    vector-reduce band v

typefn vector '__.. (a b flipped)
    let Ta Tb = (typeof a) (typeof b)
    if (not (vector-type? Ta))
        return;
    if (not (vector-type? Tb))
        return;
    let ET = (element-type Ta 0)
    if (not (type== ET (element-type Tb 0)))
        return;
    if (type== Ta Tb)
        let usz = (mul (type-countof (typeof a)) 2:usize)
        let sz = (itrunc usz i32)
        let loop (i mask) = 0 (nullof (vector i32 usz))
        if (icmp<u i sz)
            loop (add i 1) (insertelement mask i i)
        else
            shufflevector a b mask
    else
        let asz = (type-countof (typeof a))
        let bsz = (type-countof (typeof b))
        let count = (add asz bsz)
        let loop (i result) = 0:usize (nullof (vector ET count))
        if (icmp<u i asz)
            loop (add i 1:usize)
                insertelement result (extractelement a i) i
        elseif (icmp<u i count)
            loop (add i 1:usize)
                insertelement result (extractelement b (sub i asz)) i
        else result

#-------------------------------------------------------------------------------
# constants
#-------------------------------------------------------------------------------

let pi:f32 = 3.141592653589793:f32
let pi:f64 = 3.141592653589793:f64
""""The number π, the ratio of a circle's circumference C to its diameter d.
    Explicitly type-annotated versions of the constant are available as `pi:f32`
    and `pi:f64`.
let pi = pi:f32

let e:f32 = 2.718281828459045:f32
let e:f64 = 2.718281828459045:f64
""""Euler's number, also known as Napier's constant. Explicitly type-annotated
    versions of the constant are available as `e:f32` and `e:f64`
let e = e:f32

#-------------------------------------------------------------------------------
# apply locals as globals
#-------------------------------------------------------------------------------

# cleanup
del hash1
del hash2
del at

set-globals!
    .. (locals) (globals)

#-------------------------------------------------------------------------------
# REPL
#-------------------------------------------------------------------------------

fn compiler-version-string ()
    let vmin vmaj vpatch = (compiler-version)
    .. "Scopes " (string-repr vmin) "." (string-repr vmaj)
        if (vpatch == 0) ""
        else
            .. "." (string-repr vpatch)
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

fn read-eval-print-loop ()
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
                        pointer (function (ReturnLabel (unknownof Scope) (unknownof i32)))
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

fn run-main (args...)
    let argcount = (va-countof args...)
    let loop (i sourcepath parse-options) = 1 none true
    if (i < argcount)
        let k = (i + 1)
        let arg = (va@ i args...)
        if (parse-options and ((@ arg 0) == (char "-")))
            if ((arg == "--help") or (arg == "-h"))
                print-help args...
            elseif ((== arg "--version") or (== arg "-v"))
                print-version;
            elseif ((== arg "--signal-abort") or (== arg "-s"))
                set-signal-abort! true
                loop k sourcepath parse-options
            elseif (== arg "--")
                loop k sourcepath false
            else
                print
                    .. "unrecognized option: " arg
                        \ ". Try --help for help."
                exit 1
                unreachable!;
        elseif (sourcepath == none)
            loop k arg parse-options
        # remainder is passed on to script
        #else
            print
                .. "unrecognized argument: " arg
                    \ ". Try --help for help."
            exit 1
            unreachable!;
    if (sourcepath == none)
        read-eval-print-loop;
    else
        let scope =
            Scope (globals)
        set-scope-symbol! scope 'args
            fn ()
                return sourcepath (va@ i args...)
        load-module "" sourcepath
            scope = scope
            main-module? = true
        exit 0
        unreachable!;

run-main (args)
true

