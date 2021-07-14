#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""switcher
    ========

    A lazy, partially extendable switch case generator.

sugar switcher-case (lit body...)
    qq 'case this-switcher [lit]
        [inline] "#hidden" (this-condition switcher-context...)
            unquote-splice body...

sugar switcher-default (body...)
    qq 'default this-switcher
        [inline] "#hidden" (this-condition switcher-context...)
            unquote-splice body...

run-stage;

type Switcher
    fn stage-case (cls lit value)
        if (not ('constant? lit))
            error "constant literal expected"
        if (not ('pure? value))
            error "pure callable expected"
        'set-symbols cls
            literals = (sc_argument_list_join_values ('@ cls 'literals) lit)
            handlers = (sc_argument_list_join_values ('@ cls 'handlers) value)
        ;

    fn stage-default (cls value)
        if (not ('pure? value))
            error "pure callable expected"
        'set-symbol cls 'default value
        ;

    spice case (cls lit value)
        stage-case (cls as type) lit value

    spice default (cls value)
        stage-default (cls as type) value

    spice __typecall (cls name)
        spice build-switch-expr (cls value ctx...)
            let cls = (cls as type)
            let literals = ('@ cls 'literals)
            let handlers = ('@ cls 'handlers)
            let default = ('@ cls 'default)
            if ('constant? value)
                let words = (sc_const_int_word_count value)
                # find constant
                for i in (range ('argcount literals))
                    let lit handler = ('getarg literals i) ('getarg handlers i)
                    let same? =
                        if (lit == value) true # fast path
                        elseif ((sc_const_int_word_count lit) == words) # exact path
                            for w in (range words)
                                if ((sc_const_int_extract_word value w)
                                    != (sc_const_int_extract_word lit w))
                                    break false
                            else true
                        else false
                    if same?
                        return `(handler value ctx...)
                return `(default value ctx...)
            let sw = (sc_switch_new value)
            for i in (range ('argcount literals))
                let lit handler = ('getarg literals i) ('getarg handlers i)
                sc_switch_append_case sw lit `(handler value ctx...)
            sc_switch_append_default sw `(default value ctx...)
            sw

        let T = (typename.type (name as string) (cls as type))
        'set-symbols T
            literals = (sc_argument_list_new 0 null)
            handlers = (sc_argument_list_new 0 null)
            __typecall = build-switch-expr
        T

sugar switcher (name body...)
    let namestr = (name as Symbol as string)
    qq [let] [name] =
        [do]
            [let] case = [switcher-case]
            [let] default = [switcher-default]
            [let] this-switcher = ([Switcher] [namestr])
            unquote-splice body...
            this-switcher

sugar switcher+ (name body...)
    let boundname = ('@ sugar-scope name)
    qq [do]
        [let] case = [switcher-case]
        [let] default = [switcher-default]
        [let] this-switcher = [boundname]
        unquote-splice body...
        this-switcher

do
    let switcher switcher+ Switcher
    locals;
