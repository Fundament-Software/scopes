
# for each element in generator a, repeat generator b
inline span (a b)
    let iter-a init-a = ((a as Generator))
    let iter-b init-b = ((b as Generator))
    Generator
        inline (fdone t)
            let a = (@ t 0)
            let b = (@ t 1)
            let next-a at-a... = (iter-a fdone a)
            label ret
                inline eog ()
                    let next-next-a at-a... = (iter-a fdone next-a)
                    let next-b at-b... = (iter-b fdone init-b)
                    merge ret (tupleof next-a next-b) at-a... at-b...
                let next-b at-b... = (iter-b eog b)
                _ (tupleof a next-b) at-a... at-b...
        tupleof init-a init-b

inline map (gen f)
    let iter init = ((gen as Generator))
    Generator
        inline (fdone it)
            let next at... = (iter fdone it)
            _ next (f at...)
        init

inline filter (gen f)
    let iter init = ((gen as Generator))
    Generator
        inline (fdone it)
            label ret
                loop (it = it)
                    inline eog ()
                        merge ret
                    let next at... = (iter eog it)
                    if (f at...)
                        return next at...
                    repeat next
            fdone;
        init

inline folding (init gen f)
    let iter gen-init = ((gen as Generator))
    Generator
        inline (fdone it)
            let val it = (unpack it)
            let next at... = (iter fdone it)
            let val = (f val at...)
            _ (tupleof val next) val
        tupleof init gen-init

define zip (spice-macro (fn (args) (rtl-multiop args (Value zip))))
define span (spice-macro (fn (args) (rtl-multiop args (Value span))))

locals;
