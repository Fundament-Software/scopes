#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""docstring
    =========

    Implements functions to print and extract documentation from module exports.

fn... docstring (scope : Scope, name : Symbol)
    returning string
    raising Error
    let val = ('@ scope name)
    let docstr = ('docstring scope name)
    if ('constant? val)
        let T = ('typeof val)
        if (T == Closure)
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
                    fold (s = str"") for i in (range count)
                        let param = (sc_template_parameter tmpl i)
                        .. s
                            ? (empty? s) str"" str" "
                            (sc_parameter_name param) as string
                ")\n\n"
                docstr
        elseif (T == Scope)
            let scope = (val as Scope)
            let mdocstr = ('module-docstring scope)
            if (empty? mdocstr) docstr
            else mdocstr
        else docstr
    else docstr
case (scope : Scope)
    'module-docstring scope

fn... helpstring (val : Value, name : Symbol, docstr : string)
    returning string
    raising Error
    if ('constant? val)
        let T = ('typeof val)
        if (T == Closure)
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
                    fold (s = str"") for i in (range count)
                        let param = (sc_template_parameter tmpl i)
                        .. s
                            ? (empty? s) str"" str" "
                            (sc_parameter_name param) as string
                ")\n\n"
                docstr
        elseif (T == type)
            let val = (val as type)
            .. (repr 'type) " " (repr val)
                default-styler style-operator " < "
                repr ('superof val)
                if ('opaque? val) str""
                else
                    ..
                        default-styler style-operator
                            if ('plain? val) str" : "
                            else str" :: "
                        repr ('storageof val)
                "\n"
                docstr
                loop (val docstr = val str"")
                    let docstr =
                        fold (docstr) for name value in ('symbols val)
                            repeat
                                .. docstr
                                    "    "
                                    repr name
                                    default-styler style-operator " : "
                                    \ (repr ('typeof value)) "\n"
                    if (val == typename or val == type)
                        break docstr
                    else
                        repeat ('superof val) (.. docstr "\n")

        elseif (T == Scope)
            let scope = (val as Scope)
            let mdocstr = ('module-docstring scope)
            let mdocstr =
                if (empty? mdocstr) docstr
                else mdocstr
            fold (mdocstr = (.. mdocstr "\n")) for scope in ('lineage scope)
                fold (mdocstr) for name value in scope
                    if (('typeof name) == Symbol and ('constant? name))
                        .. mdocstr (repr name)
                            default-styler style-operator " : "
                            \ (repr ('typeof value)) "\n"
                    else mdocstr
        else docstr
    else docstr
case (scope : Scope, name : Symbol)
    let val = ('@ scope name)
    let docstr = ('docstring scope name)
    this-function val name docstr
case (ty : type, name : Symbol)
    let val = ('@ ty name)
    let docstr = ('docstring ty name)
    this-function val name docstr
case (space : Value, name : Symbol)
    let T = ('typeof space)
    if (T == Scope)
        this-function (space as Scope) name
    elseif (T == type)
        this-function (space as type) name
    else
        error "can only access member of scope or type"
case (scope : Scope)
    'module-docstring scope

fn... resolve-accessor
case (scope : Scope, space : Value, name : Value)
    raising Error
    returning Value Symbol
    let handler = ('@ scope symbol-handler-symbol)
    handler := handler as (@ ((_: list Scope) <-: (list Scope) raises Error))
    let expr = (handler (list name) scope)
    let expr = (decons expr)
    if (('typeof expr) == Symbol)
        return space (expr as Symbol)
    expr as:= list
    let head rest = (decons expr)
    if (head == '.)
        let key value = (decons rest 2)
        let space key = (this-function scope space key)
        let T = ('typeof space)
        let space =
            if (T == Scope)
                '@ (space as Scope) key
            elseif (T == type)
                '@ (space as type) key
            else
                error "can only access member of scope or type"
        return space (value as Symbol)
    error "not a valid accessor"

sugar help (value)
    let space value =
        resolve-accessor sugar-scope sugar-scope value
    sc_write
        try
            let str = (helpstring space value)
            if (empty? str) str"no help available\n"
            else str
        except (err)
            str"no such symbol in scope\n"
    `()

do
    let docstring helpstring help
    locals;