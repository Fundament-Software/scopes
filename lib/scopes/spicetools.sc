#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""spicetools
    ==========

    spicetools provides various utilities which aid with the implementation
    of new spices.

inline parse-argument-matcher (failfunc expr scope params cb)
    #if false
        return `()
    let params = (params as list)
    let params = (uncomma params)
    let paramcount = (countof params)
    loop (i rest varargs scope = 0 params false scope)
        if (empty? rest)
            return
                spice-quote
                    if (not (check-count (sc_argcount expr)
                            [(? varargs (sub paramcount 1) paramcount)]
                            [(? varargs -1 paramcount)]))
                        failfunc;
                scope
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
            repeat (i + 1) rest (| varargs variadic?)
                cb param arg scope
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
                repeat (i + 1) rest varargs
                    cb param arg scope
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
                repeat (i + 1) rest varargs
                    cb param arg scope
        error "unsupported pattern"

fn gen-argument-matcher (failfunc expr scope params)
    let outexpr = (sc_expression_new)
    local outargs = (sc_argument_list_new 0 null)
    let header scope =
        parse-argument-matcher failfunc expr scope params
            inline (param arg scope)
                sc_expression_append outexpr arg
                outargs =
                    sc_argument_list_join_values outargs arg
                'bind scope param arg
    let scope =
        'bind scope '*... (deref outargs)
    _
        spice-quote
            header
            outexpr
        scope

define spice-match
    gen-match-block-parser gen-argument-matcher

locals;
