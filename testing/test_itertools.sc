
using import itertools

for x y z in (dim 1 2 3)
    print x y z
print;
for x y z in (bitdim 2 1 1)
    print x y z

let binary-range = (range 2)
let gen =
    zip
        imap
            infinite-range
            inline (x) (x * 2)
        span
            arrayof string "yes" "this" "is" "dog?"
            binary-range
            binary-range

for x... in gen
    print x...

let gen =
    imap (range 22)
        inline (x)
            let ph = (radians ((f32 x) * 16.0))
            _ (sin ph) (cos ph)

fold (x y z = 0.0 0.0 0.0) for s t in gen
    print x y z s t
    _ (x + s) y z

print
    ->>
        range 12
        cascade
            gate
                inline (x)
                    (x & 1) == 0
                take 2 '()
                take 2 '()
        flatten
        '()

inline string-buffer-sink (maxsize)
    let buf = (alloca-array i8 maxsize)
    Collector
        inline () 0
        inline (n) (n < maxsize)
        inline (n)
            sc_string_new buf (n as usize)
        inline (src n)
            store (src) (getelementptr buf n)
            n + 1

print
    ->>
        "the quick brown fox jumped over the lazy dog"
        cascade
            limit (inline (x) (x != 32))
            string-buffer-sink 256
        '()

assert
    ==
        ->>
            range 26
            filter
                inline (x) (x != 10)
            cascade
                take 3
                'cons-sink '()
            cascade
                take 3
                'cons-sink '()
            'cons-sink '()
        '(((25) (24 23 22) (21 20 19))
        ((18 17 16) (15 14 13) (12 11 9))
        ((8 7 6) (5 4 3) (2 1 0)))

print
    ->> '(((25) (24 23 22) (21 20 19)) ((18 17 16) (15 14 13) (12 11 9)) ((8 7 6) (5 4 3) (2 1 0)))
        map (inline (x) (x as list)); cat;
        map (inline (x) (x as list)); cat;
        'cons-sink '()

spice va-map (f args...)
    ->>
        'args args...
        map
            inline (arg) `(f arg)
        'append-sink (sc_argument_list_new)

run-stage;

let x y z w =
    va-map
        inline (x) (x * 2)
        \ 1 2 3 4

assert ((+ x y z w) == 20)

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

print
    # generate 10 sample indices
    ->> (range 10)
        # convert index to float
        map f32
        # sum collector output into a temporary
            a bit like reduce, but using a temporary stack variable
        demux 0.0 (do +)
            # feed input signal into each collector, forking the stream
            mux
                # sine curve from sample index
                map sin
                # cosine curve from sample index
                map cos
                # saw wave from sample index
                map
                    inline (x)
                        (x % 1.0) * 2.0 - 1.0
        # record summed output to list
        '()

