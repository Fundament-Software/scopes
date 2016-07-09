IR

# the bootstrap language is feature equivalent to LLVM IR, and translates
# directly to LLVM API commands. It sounds like a Turing tarpit, but the fact
# that it gives us simple means to get out of it and extend those means
# turns it more into Turing lube.

# from here, outside the confines of the horribly slow and tedious C++ compiler
# context, we're going to implement our own language.

################################################################################
# declare the bang API as we don't have the means to comfortably import clang
# declarations yet.

# opaque declarations for the bang compiler Environment and the Values of
# its S-Expression tree, which can be Table, String, Symbol, Integer, Real.
struct Environment
struct Value

struct PValue

deftype rawstring (* i8)

declare printf (function i32 (* i8) ...)
declare strcmp (function i32 rawstring rawstring)

declare bang_print (function void rawstring)

deftype EnvironmentRef (* Environment)
deftype ValueRef (* Value)

defvalue dump-value
    declare "bang_dump_value" (function void ValueRef)

deftype preprocessor-func
    function ValueRef EnvironmentRef ValueRef

defvalue set-preprocessor
    declare "bang_set_preprocessor" (function void (* preprocessor-func))

defvalue kind?
    declare "bang_get_kind" (function i32 ValueRef)

defvalue table-size
    declare "bang_size" (function i32 ValueRef)

defvalue table-at
    declare "bang_at" (function ValueRef ValueRef i32)

defvalue set-at
    declare "bang_set_at" (function ValueRef ValueRef i32 ValueRef)

defvalue set-key
    declare "bang_set_key" (function void ValueRef rawstring ValueRef)

defvalue get-key
    declare "bang_get_key" (function ValueRef ValueRef rawstring)

defvalue slice
    declare "bang_slice" (function ValueRef ValueRef i32 i32)

defvalue merge
    declare "bang_merge" (function ValueRef ValueRef ValueRef)

defvalue value==
    declare "bang_eq" (function i1 ValueRef ValueRef)

defvalue string-value
    declare "bang_string_value" (function rawstring ValueRef)

defvalue error-message
    declare "bang_error_message" (function void ValueRef rawstring ...)

defvalue value-type-none (int i32 0)
defvalue value-type-table (int i32 1)
defvalue value-type-string (int i32 2)
defvalue value-type-symbol (int i32 3)
defvalue value-type-integer (int i32 4)
defvalue value-type-real (int i32 5)

################################################################################
# fundamental helper functions

defvalue table?
    define "" (value)
        function i1 ValueRef
        label ""
            ret
                icmp ==
                    call kind? value
                    value-type-table

defvalue symbol?
    define "" (value)
        function i1 ValueRef
        label ""
            ret
                icmp ==
                    call kind? value
                    value-type-symbol

defvalue empty?
    define "" (value)
        function i1 ValueRef
        label ""
            ret
                icmp ==
                    call table-size value
                    int i32 0

################################################################################
# build and install the preprocessor hook function.

# all top level expressions go through the preprocessor, which then descends
# the expression tree and translates it to bang IR.
define global-preprocessor (env value)
    preprocessor-func
    label ""
        # here we can implement our own language
        cond-br
            call table? value
            label if-table
                cond-br
                    call empty? value
                    label fail
                    label if-not-empty
                        defvalue head
                            call table-at value (int i32 0)
                        cond-br
                            call value== head
                                quote Value IR
                            label if-IR
                                call printf
                                    bitcast
                                        global "" "is table!\n"
                                        rawstring
                                    call kind? value
                                defvalue ir-value
                                    call set-at value
                                        int i32 0
                                        quote Value do
                                br
                                    label done
                            label wrong-head
                                call error-message value
                                    bitcast (global "" "unknown special form\n") rawstring
                                ret
                                    null ValueRef
            label fail
    label fail
        call error-message value
            bitcast (global "" "unhandled expression\n") rawstring
        br
            label done
    label done
        ret
            phi
                ValueRef
                ir-value if-IR
                (null ValueRef) fail

# install preprocessor and continue evaluating the module
run
    define "" ()
        function void
        label ""
            call set-preprocessor global-preprocessor
            ret;

# all top level expressions from here go through the preprocessor
# which means from here, we're in a new context
################################################################################

IR
    defvalue hello-world
        bitcast
            global ""
                "Hello World!\n"
            * i8

    define main ()
        function void
        label ""
            call dump-value
                quote Value
                    run
                        print "'\"" '"\''
                        print "yo
                        yo" "hey hey" "ho"
                        print (
                        ) a b c (
                         ) d e f
                            g h i
                        # compare
                        do
                            if cond1:
                                do-this;
                            else if cond2:
                                do-that;
                            else:
                                do-something;

                        # to
                        do
                            if (cond1) {
                                do-this;
                            } else if (cond2) {
                                do-that;
                            } else {
                                do-that;
                            }
                        0x1A.25A 0x.e 0xaF0.3 0 1 1e+5 .134123123 123 012.3 12.512e+12 0 0 0 0 0 +1 -0.1 +0.1 2.5 +1 -1 +100 -100 +100
                        0666 ..5 ... :: .: a. a: a:a:a .a .a ...a 35.3 0x7fffffffffffffff 0x10 +inf -inf +nan -nan
                        -.0 1.0 1.594 1 .1e+12 .0 0. +5 +.5 .5 -1 -.5 - +a +0 -0 -2 2.5 inf nan n na i in
                        ... a. a: aa a,b,c 0.
                        a = 3, b = 4, c = 5;
                        {a b c} [(d)f g]
                        {a,(),;b, c;d e;}[]
                        [abc:a,b,c d,d;a,b,c,d;]
                        [][a,b,d,e f;]
                        [a = b,c = d,e = f]
                        [a b: c d,q,d e,e,]
                        [a b: c d;q;d e;e;]
                        ab.bc..cd...de.a.b.c.d
                        [ptr, * ptr, const * ptr]
                        int x, int y; x = 5, y = z
                        do                                # (do
                            a; b; c d
                        . .. ... ....
                        {
                            if a: b q, c d, d e;
                            if b: c;
                            if c {
                                b q;
                                c d;
                                d e;
                                };
                            }
                        a b;c d;
                            f g
                        do
                            print x; print
                                a + b
                        if q: a b, c d;
                        a b c,
                            d e f
                        e f, g h, i j k,m;
                        g h, i j k;
                        n o;
                        f g,q,w,q e
                        (if a == b && c == d: print a; print b;)
                        if a == b && c == d:
                            print "yes"; print "no"
                            print c
                        {
                            if (true)

                            {
                            }
                            else if (false)
                            {
                            }
                            else
                            {
                            };
                            print("hi",1,2,3,auto(),5,2 + 1);
                        }

                        if a == b && c == d: print a; print b;
                        if a; q e
                            a b c d;
                            e f;
                            g h; [i];
                            g h; j; k; l; m
                            j k; l m
                            teamo beamo
                        else
                            e f g
                        define "" ()
                            # comment
                            function void
                            label ""
                                call printf
                                    bitcast
                                        global ""
                                            "running in compiler!\n"
                                        * i8
                                ret;

            defvalue Q
                quote Value word
            call printf
                bitcast
                    global ""
                        "quote = %x %p %p\n"
                    * i8
                dump
                    load
                        getelementptr
                            bitcast
                                Q
                                * i32
                            int i32 0
                Q
                Q

            call printf hello-world
            cond-br
                int i1 1
                label then
                label else
        label then
            defvalue c0
                bitcast
                    global "" "Choice 1\n"
                    * i8
            br
                label done
        label else
            defvalue c1
                bitcast
                    global "" "Choice 2\n"
                    * i8
            br
                label done
        label done
            call printf
                phi
                    * i8
                    c0 then
                    c1 else
            ret;

    # dump-module
    run main

