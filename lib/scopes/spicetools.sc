
inline parse-argument-matcher (failfunc expr scope params cb)
    #if false
        return `()
    let params = (params as list)
    let params = (uncomma params)
    let paramcount = (countof params)
    loop (i rest varargs = 0 params false)
        if (empty? rest)
            return
                spice-quote
                    if (not (check-count (sc_argcount expr)
                            [(? varargs (sub paramcount 1) paramcount)]
                            [(? varargs -1 paramcount)]))
                        failfunc;
        let paramv rest = (decons rest)
        let T = ('typeof paramv)
        if (T == Symbol)
            let param = (paramv as Symbol)
            let variadic? = ('variadic? param)
            let arg =
                if variadic?
                    if (not (empty? rest))
                        error "vararg parameter is not in last place"
                    `(sc_getarglist expr i)
                else
                    `(sc_getarg expr i)
            cb param arg
            repeat (i + 1) rest (| varargs variadic?)
        elseif (T == list)
            let param = (paramv as list)
            let head head-rest = (decons param)
            let mid mid-rest = (decons head-rest)
            if ((('typeof mid) == Symbol) and ((mid as Symbol) == ':))
                let exprT = (decons mid-rest)
                let exprT = (sc_expand exprT '() scope)
                let param = (head as Symbol)
                if ('variadic? param)
                    error "vararg parameter cannot be typed"
                spice-quote
                    let arg = (sc_getarg expr i)
                    let conv = (imply-converter ('typeof arg) exprT ('constant? arg))
                    let arg =
                        if (operator-valid? conv) `(conv arg)
                        else (failfunc)
                cb param arg
                repeat (i + 1) rest varargs
            elseif ((('typeof mid) == Symbol) and ((mid as Symbol) == 'as))
                let exprT = (decons mid-rest)
                let exprT = (sc_expand exprT '() scope)
                let param = (head as Symbol)
                if ('variadic? param)
                    error "vararg parameter cannot be typed"
                spice-quote
                    let arg = (sc_getarg expr i)
                    let arg =
                        if (('constant? arg) and (('typeof arg) == exprT))
                            arg as exprT
                        else
                            failfunc;
                cb param arg
                repeat (i + 1) rest varargs
        error "unsupported pattern"

fn gen-argument-matcher (failfunc expr scope params)
    let outexpr = (sc_expression_new)
    let outargs = (sc_argument_list_new)
    'set-symbol scope '*... outargs
    let header =
        parse-argument-matcher failfunc expr scope params
            inline (param arg)
                sc_expression_append outexpr arg
                sc_argument_list_append outargs arg
                'set-symbol scope param arg
    spice-quote
        header
        outexpr

define spice-match
    gen-match-block-parser gen-argument-matcher

locals;
