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
        Value.arglist-sink ('argcount args...)

run-stage;

let x y z w =
    va-map
        inline (x) (x * 2)
        \ 1 2 3 4

assert ((+ x y z w) == 20)

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

# generate 10 numbers
->> (range 10)
    # preserve all existing inputs and outputs
    retain _
        map
            # receives range index
            inline "op1" (x) (/ x)
    retain _
        map
            # receives range index and output of op1, which we discard
            inline "op2" (x) (/ (x * 2))
    retain _
        map
            # receives range index and outputs of op1 & op2, which we discard
            inline "op3" (x) (/ (x * 5))
    map
        # receives inputs and all output values computed in va-mux
        inline (n args...)
            # print sum of op1 + op2 + op3, range index and individual values
            print (+ args...) "<-" n ":" args...
    # /dev/null
    drain

