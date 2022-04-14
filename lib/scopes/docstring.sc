#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""docstring
    =========

    Implements functions to print and extract documentation from module exports.

fn... docstring (scope : Scope, name : Symbol)
    let val = ('@ scope name)
    let docstr = ('docstring scope name)
    if ('constant? val)
        if (('typeof val) == Closure)
            let val = (val as Closure)
            let fdocstr = ('docstring val)
            let docstr =
                if (empty? fdocstr) docstr
                else fdocstr
            let tmpl = (sc_closure_get_template val)
            ..
                if (sc_template_is_inline tmpl)
                    repr 'inline
                else
                    repr 'fn
                " "
                name as string
                " ("
                do
                    let count = (sc_template_parameter_count tmpl)
                    fold (s = "") for i in (range count)
                        let param = (sc_template_parameter tmpl i)
                        .. s
                            ? (empty? s) "" " "
                            (sc_parameter_name param) as string
                ")\n\n"
                docstr
        elseif (('typeof val) == Scope)
            let mdocstr = ('module-docstring (val as Scope))
            if (empty? mdocstr) docstr
            else mdocstr
        else docstr
    else docstr
case (scope : Scope)
    'module-docstring scope

fn resolve-accessor (scope name)
    raising Error
    returning Scope Symbol
    let handler = ('@ scope symbol-handler-symbol)
    handler := handler as (@ ((_: list Scope) <-: (list Scope) raises Error))
    let expr = (handler (list name) scope)
    let expr = (decons expr)
    if (('typeof expr) == Symbol)
        return scope (expr as Symbol)
    expr as:= list
    let head rest = (decons expr)
    if (head == '.)
        let key value = (decons rest 2)
        let scope key = (this-function scope key)
        let scope = (('@ scope key) as Scope)
        return scope (value as Symbol)
    error "not a valid accessor"

sugar help (value)
    let scope value =
        resolve-accessor sugar-scope value
    sc_write
        try
            let str = (docstring scope value)
            if (empty? str) "no help available\n"
            else str
        except (err)
            "no such symbol in scope\n"
    `()

do
    let docstring help
    locals;