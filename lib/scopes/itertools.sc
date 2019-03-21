
# for each element in generator a, repeat generator b
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

inline map (gen f)
    let start valid at next = ((gen as Generator))
    Generator start valid
        inline (it...)
            f (at it...)
        next

inline filter (gen f)
    let start valid at next = ((gen as Generator))
    # skip to first valid iterator
    let start... =
        loop (it... = (start))
            if (valid it...)
                if (f (at it...))
                    break it...
                else
                    repeat (next it...)
            else
                break it...
    Generator
        inline () start...
        valid
        at
        inline (it...)
            loop (it... = (next it...))
                if (valid it...)
                    if (f (at it...))
                        break it...
                    else
                        repeat (next it...)
                else
                    break it...
        next

define zip (spice-macro (fn (args) (rtl-multiop args (Value zip))))
define span (spice-macro (fn (args) (rtl-multiop args (Value span))))

locals;
