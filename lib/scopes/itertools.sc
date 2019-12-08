#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""itertools
    =========

    itertools provides various utilities which simplify the composition of
    generators and collectors.

using import spicetools

#---------------------------------------------------------------------------
# generators
#---------------------------------------------------------------------------

""""for each element in generator a, repeat generator b and return both states
inline span (a b)
    let start-a valid-a at-a next-a = ((a as Generator))
    let start-b valid-b at-b next-b = ((b as Generator))
    let start-a... = (start-a)
    let start-b... = (start-b)
    let lsize = (va-countof start-a...)
    let start... = (va-append-va (inline () start-b...) start-a...)
    # if b is empty, nothing will ever be produced
    let b-items? = (valid-b start-b...)
    Generator
        inline () start...
        inline (it...)
            let it-a it-b = (va-split lsize it...)
            b-items? & (valid-a (it-a))
        inline (it...)
            let it-a it-b = (va-split lsize it...)
            va-append-va (inline () (at-b (it-b))) (at-a (it-a))
        inline (it...)
            let it-a it-b = (va-split lsize it...)
            let next-b... = (next-b (it-b))
            if (valid-b next-b...)
                va-append-va (inline () next-b...) (it-a)
            else
                va-append-va (inline () start-b...) (next-a (it-a))

spice unpack-dim (n size...)
    let acount = ('argcount size...)
    let values = (alloca-array Value acount)
    fold (expr = n) for i S in (enumerate ('args size...))
        let arg = `(expr % S)
        values @ i = arg
        _ `(expr // S)
    sc_argument_list_new acount values

spice unpack-bitdim (n size...)
    let acount = ('argcount size...)
    let values = (alloca-array Value acount)
    let _1 = `(1 as (typeof n))
    fold (expr = n) for i S in (enumerate ('args size...))
        let arg = `(expr & ((_1 << S) - _1))
        values @ i = arg
        _ `(expr >> S)
    sc_argument_list_new acount values

""""a branchless generator that iterates multidimensional coordinates
@@ spice-quote
inline dim (x n...)
    let T = (typeof x)
    let size = (* x n...)
    let start = (0 as T)
    let _1 = (1 as T)
    Generator
        inline () start
        inline (n) (n < size)
        inline (n) (unpack-dim n x n...)
        inline (n) (n + _1)

""""a variant of dim optimized for power of two sizes; the dimensions are
    specified as exponents of 2
@@ spice-quote
inline bitdim (x n...)
    let T = (typeof x)
    let start = (0 as T)
    let _1 = (1 as T)
    let size = (_1 << (+ x n...))
    Generator
        inline () start
        inline (n) (n < size)
        inline (n) (unpack-bitdim n x n...)
        inline (n) (n + _1)

inline imap (gen f)
    let start valid at next = ((gen as Generator))
    Generator start valid
        inline (it...)
            f (at it...)
        next

inline ipair (gen N)
    """"generate one variadic argument from N generated arguments
    let N =
        static-if (none? N) 2
        else N
    static-assert (constant? N)
    let start valid at next = ((gen as Generator))
    let start... = (start)
    let lsize = (va-countof start...)
    let range... = (va-range N)
    Generator
        inline () start...
        inline (it...)
            va-lfold
                inline () it...
                inline "#hidden" (key value it)
                    let it... = (it)
                    if (not (valid it...))
                        return false
                    inline () (next it...)
                range...
            true
        inline (it...)
            let result =
                va-lfold
                    inline ()
                        _
                            inline () it...
                            inline () ()
                    inline "#hidden" (key value it)
                        let it vals = (it)
                        let it... = (it)
                        let vals... = (vals)
                        inline ()
                            _
                                inline () (next it...)
                                inline ()
                                    va-append-va (inline () (at it...)) vals...
                    range...
            let it vals = (result)
            vals;
        inline (it...)
            call
                va-lfold
                    inline () it...
                    inline "#hidden" (key value it)
                        let it... = (it)
                        inline () (next it...)
                    range...

""""when generator a is exhausted, continue with generator b
    both generators must yield the same value type
inline join (a b)
    let start-a valid-a at-a next-a = ((a as Generator))
    let start-b valid-b at-b next-b = ((b as Generator))
    let start-a... = (start-a)
    let lsize = (va-countof start-a...)
    let start... = (va-append-va start-b start-a...)
    Generator
        inline () start...
        inline (it...)
            let it-a it-b = (va-split lsize it...)
            (valid-a (it-a)) | (valid-b (it-b))
        inline (it...)
            let it-a it-b = (va-split lsize it...)
            let it-a... = (it-a)
            if (valid-a it-a...)
                at-a it-a...
            else
                at-b (it-b)
        inline (it...)
            let it-a it-b = (va-split lsize it...)
            let it-a... = (it-a)
            let valid? = (valid-a it-a...)
            va-append-va
                inline ()
                    let it-b... = (it-b)
                    if valid? it-b...
                    else
                        next-b it-b...
                if valid? (next-a it-a...)
                else it-a...

define zip (spice-macro (fn (args) (ltr-multiop args `zip 2)))
define span (spice-macro (fn (args) (rtl-multiop args `span 2)))
define join (spice-macro (fn (args) (rtl-multiop args `join 2)))

#---------------------------------------------------------------------------
# collectors
#---------------------------------------------------------------------------

inline collect (coll)
    """"run collector until full and return the result
    let start valid? at collect = ((coll as Collector))
    let start... = (start)
    loop (state... = start...)
        if (valid? state...)
            repeat (collect (inline () ()) state...)
        else
            break (at state...)

inline each (generator collector)
    """"fold output from generator into collector
    inline _each (collector)
        let c-init c-valid? c-at c-collect = ((collector as Collector))
        let g-start g-valid? g-at g-next = ((generator as Generator))
        let init... = (c-init)
        let start... = (g-start)
        let lsize = (va-countof start...)
        Collector
            inline () (va-append-va (inline () init...) start...)
            inline (args...)
                let it state = (va-split lsize args...)
                let it... = (it)
                let state... = (state)
                (g-valid? it...) & (c-valid? state...)
            inline (args...)
                let it state = (va-split lsize args...)
                let it... = (it)
                let state... = (state)
                c-at state...
            inline (src args...)
                let it state = (va-split lsize args...)
                let it... = (it)
                let state... = (state)
                let src... = (va-append-va src (g-at it...))
                let newstate... = (c-collect (inline () src...) state...)
                va-append-va (inline () newstate...) (g-next it...)
    static-if (none? collector) _each
    else (_each collector)

# this version of compose offers better traceback info
spice compose (collector...)
    spice compose-inner (coll collector...)
        let expr = (sc_expression_new)
        sc_expression_append expr coll
        fold (coll = coll) for prevcoll in ('reverse-args collector...)
            let coll =
                try
                    hide-traceback;
                    sc_prove `(prevcoll coll)
                except (err)
                    hide-traceback;
                    error@+ err ('anchor prevcoll) "while composing collector"
            sc_expression_append expr coll
            coll
        expr
    spice-quote
        inline "compose" (coll)
            compose-inner coll collector...

#inline compose (collector...)
    inline "compose" (coll)
        va-rfold coll
            inline (key value coll)
                value coll
            collector...

inline cat (coll)
    """"treat input as a generator and forward its arguments individually
    inline _cat (coll)
        let init valid? at collect = ((coll as Collector))
        Collector init valid? at
            inline "cat-collect" (src it...)
                fold (state... = it...) for val... in (src)
                    if (valid? state...)
                        collect (inline () val...) state...
                    else
                        break state...
    static-if (none? coll) _cat
    else (_cat coll)

""""Expands a processing chain into nested expressions so that each expression
    is passed as tailing argument to the following expression.

    `__` can be used as a placeholder token to position the previous expression.

    example::

        --> x
            f
            g
            h 2 __
            k

    expands to::

        k
            h 2
                g
                    f x
sugar --> (expr ...)
    fn placeholder? (elem)
        (('typeof elem) == Symbol) and (elem as Symbol == '__)

    fold (outp = expr) for expr in ...
        let anchor = ('anchor expr)
        'tag
            match ('typeof expr)
            case list
                let prev-outp = outp
                let expr = (expr as list)
                let outp found =
                    fold (outp found = '() false) for elem in expr
                        if (placeholder? elem)
                            if found
                                hide-traceback;
                                error@ ('anchor elem) "while expanding expression" "duplicate placeholder token"
                            _ (cons prev-outp outp) true
                        else
                            _ (cons elem outp) found
                if found
                    `[('reverse outp)]
                else
                    `[(.. expr (list prev-outp))]
            default
                `[(list expr outp)]
            anchor

inline ->> (generator collector...)
    collect
        each generator
            va-rfold none
                inline (key value coll)
                    static-if (none? coll) value
                    else
                        value coll
                collector...

inline flatten (coll)
    """"collect variadic input as individual single items
    inline _flatten (coll)
        let init valid? at collect = ((coll as Collector))
        Collector init valid? at
            inline (src args...)
                call
                    va-lfold (inline () args...)
                        inline (key value it)
                            inline ()
                                collect (inline () value) (it)
                        src;
    static-if (none? coll) _flatten
    else (_flatten coll)

inline map (f coll)
    inline _map (coll)
        let init valid? at collect = ((coll as Collector))
        Collector init valid? at
            inline "map-push" (src args...)
                let src... = (f (src))
                collect (inline () src...) args...
    static-if (none? coll) _map
    else (_map coll)

inline reduce (init f)
    Collector
        inline () init
        inline (it) true
        inline (it) it
        inline (src it)
            f it (src)

let drain =
    Collector
        inline () ()
        inline () true
        inline () ()
        inline (src) (src) ()

inline limit (f coll)
    inline _limit (coll)
        let init valid? at collect = ((coll as Collector))
        Collector
            inline ()
                _ true (init)
            inline (ok? it...)
                ok? & (valid? it...)
            inline (ok? it...)
                at it...
            inline (src ok? it...)
                let src... = (src)
                let outit... = (collect (inline () src...) it...)
                _ (f src...) outit...
    static-if (none? coll) _limit
    else (_limit coll)

inline gate (f a b)
    """"if f is true, collect input in a, otherwise collect in b
        when both are full, output both
        until then, new input for full containers is discarded
    inline _gate (b)
        let a-init a-valid? a-at a-collect = ((a as Collector))
        let b-init b-valid? b-at b-collect = ((b as Collector))
        let a-start... = (a-init)
        let b-start... = (b-init)
        let lsize = (va-countof a-start...)
        let start... =
            va-append-va (inline () b-start...) a-start...
        Collector
            inline () start...
            inline (it...)
                let a-it b-it = (va-split lsize it...)
                (a-valid? (a-it)) | (b-valid? (b-it))
            inline (it...)
                let a-it b-it = (va-split lsize it...)
                let a... = (a-at (a-it))
                let b... = (b-at (b-it))
                va-append-va (inline () b...) a...
            inline (src it...)
                let a-it b-it = (va-split lsize it...)
                let src... = (src)
                if (f src...)
                    let a-it... = (a-it)
                    if (a-valid? a-it...)
                        let a-it... =  (a-collect (inline () src...) a-it...)
                        return
                            va-append-va b-it a-it...
                else
                    let b-it... = (b-it)
                    if (b-valid? b-it...)
                        let b-it... =  (b-collect (inline () src...) (b-it))
                        return
                            va-append-va (inline () b-it...) (a-it)
                it...
    static-if (none? b) _gate
    else (_gate b)

inline filter (f coll)
    inline _filter (coll)
        let init valid? at collect = ((coll as Collector))
        Collector init valid? at
            inline (src rest...)
                let src... = (src)
                if (f src...)
                    collect (inline () src...) rest...
                else
                    rest...
    static-if (none? coll) _filter
    else (_filter coll)

inline take (n coll)
    """"limit collector to output n items
    inline _take (coll)
        let init valid? at collect = ((coll as Collector))
        Collector
            inline () (_ 0 (init))
            inline (it rest...)
                (valid? rest...) & (it < n)
            inline (it rest...) (at rest...)
            inline (src it rest...)
                _ (it + 1) (collect src rest...)
    static-if (none? coll) _take
    else (_take coll)

inline cascade1 (a b)
    inline _cascade (b)
        let start-a valid-a at-a collect-a = ((a as Collector))
        let start-b valid-b at-b collect-b = ((b as Collector))
        let start-a... = (start-a)
        let start-b... = (start-b)
        let lsize = (va-countof start-a...)
        let start... = (va-append-va (inline () start-b...) true start-a...)
        # if a is empty, nothing will ever be produced
        let a-items? = (valid-a start-a...)
        Collector
            inline () start...
            inline (first? it...)
                let it-a it-b = (va-split lsize it...)
                a-items? & (valid-b (it-b))
            inline (first? it...)
                let it-a it-b = (va-split lsize it...)
                let it-b... = (it-b)
                at-b
                    if first?
                        it-b...
                    else
                        collect-b (inline () (at-a (it-a))) it-b...
            inline (src first? it...)
                let it-a it-b = (va-split lsize it...)
                let collect-a... = (collect-a src (it-a))
                if (valid-a collect-a...)
                    va-append-va it-b false collect-a...
                else
                    va-append-va
                        inline ()
                            collect-b (inline () (at-a collect-a...)) (it-b)
                        \ true start-a...
    static-if (none? b) _cascade
    else (_cascade b)

inline cascade (collector...)
    """"two collectors:
        every time a is full, b collects a and a is reset
        when b ends, the remainder of a is collected
    inline (coll)
        cascade1
            va-rfold none
                inline (key value coll)
                    static-if (none? coll) value
                    else
                        value coll
                collector...
            coll

inline mux1 (c1 c2 coll)
    """"send input into two collectors which fork the target collector
    inline _mux (coll)
        let c1 = (c1 coll)
        let c2 = (c2 coll)
        let init1 valid1? at1 collect1 = ((c1 as Collector))
        let init2 valid2? at2 collect2 = ((c2 as Collector))
        let init1... = (init1)
        let init2... = (init2)
        let lsize = (va-countof init1...)
        let start... =
            va-append-va (inline () init2...) init1...
        Collector
            inline "mux-init" () start...
            inline "mux-valid?" (it...)
                let it1 it2 = (va-split lsize it...)
                (valid1? (it1)) & (valid2? (it2))
            inline "mux-at" (it...)
                let it1 it2 = (va-split lsize it...)
                let at1... = (at1 (it1))
                let at2... = (at2 (it2))
                va-append-va (inline () at2...) at1...
            inline "mux-collect" (src it...)
                let it1 it2 = (va-split lsize it...)
                let src... = (src)
                let src = (inline () src...)
                let it1... = (collect1 src (it1))
                let it2... = (collect2 src (it2))
                va-append-va (inline () it2...) it1...
    static-if (none? coll) _mux
    else (_mux coll)

inline mux (collector...)
    """"send input into multiple collectors which each fork the target collector
    let c1 c2 c... = collector...
    static-if (none? c2) c1
    else
        va-lfold (mux1 c1 c2)
            inline (key c2 c1)
                mux1 c1 c2
            c...

inline demux (init-value f collector...)
    let muxed = (mux collector...)
    """"a reducing sink for mux streams
    inline _demux (coll)
        local reduced = init-value
        let sink =
            Collector
                inline ()
                inline () true
                inline ()
                inline (src)
                    reduced = (f reduced (src))
                    ;

        let muxed = (muxed sink)
        let init1 valid1? at1 collect1 = ((muxed as Collector))
        let init2 valid2? at2 collect2 = ((coll as Collector))
        let init1... = (init1)
        let init2... = (init2)
        let lsize = (va-countof init1...)
        let start... =
            va-append-va (inline () init2...) init1...
        Collector
            inline "demux-init" () start...
            inline "demux-valid?" (it...)
                let it1 it2 = (va-split lsize it...)
                (valid1? (it1)) & (valid2? (it2))
            inline "demux-at" (it...)
                let it1 it2 = (va-split lsize it...)
                at2 (it2)
            inline "demux-collect" (src it...)
                let it1 it2 = (va-split lsize it...)
                let it1... = (collect1 src (it1))
                let it2... = (collect2 (inline () (deref reduced)) (it2))
                reduced = init-value
                va-append-va (inline () it2...) it1...
    #static-if (none? coll) _demux
    #else (_demux coll)

inline retain1 (mapl child coll)
    let mapl =
        static-if (none? mapl) (inline (...) ...)
        else mapl
    """"output both child input and child output
    inline _retain1 (coll)
        let ch = (child coll)
        let init1 valid1? at1 collect1 = ((ch as Collector))
        let init2 valid2? at2 collect2 = ((coll as Collector))
        Collector init1 valid1? at1
            inline "append-collect" (src it...)
                let src... = (src)
                let sink =
                    Collector init2 valid2? at2
                        inline (src2 it2...)
                            collect2
                                inline ()
                                    va-append-va src2 (mapl src...)
                                it2...
                let __ __ __ collect = (((child sink) as Collector))
                collect (inline () src...) it...
    static-if (none? coll) _retain1
    else (_retain1 coll)

@@ spice-quote
inline retain (mapl ...)
    """"feeds the input through a composition of collectors and feeds the
        input along with the composition output to the next collector.
        if mapl is not none, it allows to specify the portion of the input that
        will be passed to the end point.
    retain1 mapl (compose ...)

unlet cascade1 retain1

do
    let span dim bitdim imap ipair join zip span join collect each compose cat
        \ --> ->> flatten map reduce drain limit gate filter take cascade mux
        \ demux

    locals;
