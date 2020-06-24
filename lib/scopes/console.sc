#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""console
    =======

    Implements the read-eval-print loop for Scopes' console.

#-------------------------------------------------------------------------------
# Scopes Console
#-------------------------------------------------------------------------------

fn... read-eval-print-loop
case (global-scope, show-logo : bool = false, history-path : string = "")
    fn repeat-string (n c)
        loop (i s = 0:usize "")
            if (i == n)
                return s
            repeat (i + 1:usize)
                .. s c

    fn leading-spaces (s)
        let len = (countof s)
        loop (i = 0:usize)
            if (i == len)
                return s
            let c = (@ s i)
            if (c != 32:i8)
                let s = (sc_string_buffer s)
                return (sc_string_new s i)
            repeat (i + 1:usize)

    fn blank? (s)
        let len = (countof s)
        loop (i = 0:usize)
            if (i == len)
                return true
            if ((@ s i) != 32:i8)
                return false
            repeat (i + 1:usize)

    let cwd =
        realpath "."

    if show-logo
        print-logo;

    let eval-scope = (Scope global-scope)

    if (not (empty? history-path))
        sc_prompt_load_history history-path

    sugar help ((value as Symbol))
        let val =
            try
                '@ sugar-scope value
            except (err)
                print "no such symbol in scope"
                return `()
        if (('constant? val) and ('typeof val) == Closure)
            let val = (val as Closure)
            let docstr = ('docstring val)
            let tmpl = (sc_closure_get_template val)
            if (sc_template_is_inline tmpl)
                sc_write (repr 'inline)
            else
                sc_write (repr 'fn)
            sc_write " "
            sc_write (value as string)
            sc_write " ("
            let count = (sc_template_parameter_count tmpl)
            for i in (range count)
                if (i != 0)
                    sc_write " "
                let param = (sc_template_parameter tmpl i)
                sc_write ((sc_parameter_name param) as string)
            sc_write ")\n\n"
            if (not (empty? docstr))
                sc_write docstr
            return `()
        let docstr = ('docstring sugar-scope value)
        if (empty? docstr)
            print "no help available"
        else
            sc_write docstr
        `()

    sugar sh (values...)
        let system = (extern 'system (function i32 rawstring))
        let block = (sc_expression_new)
        let str = `""
        sc_expression_append block str
        fold (str = str) for elem in values...
            let str =
                if (('typeof elem) == string)
                    `(.. str elem " ")
                elseif (('typeof elem) == Symbol)
                    `(.. str [(elem as Symbol as string)] " ")
                else
                    `(.. str (tostring elem) " ")
            sc_expression_append block str
            str
        qq [embed]
            [let] $? = ([system] [block])
            ;

    let eval-scope =
        'bind-symbols eval-scope
            module-dir = cwd
            module-path = (cwd .. "/<console>.sc")
            module-name = "<console>"
            main-module? = true
            sh = sh
            help = help
            exit =
                typedef (do "Enter 'exit;' or Ctrl+D to exit")
                    inline __typecall () (if true (exit 0))

    fn autocomplete-symbol (text ctx scope)
        loop (i0 i scope = 0 0 scope)
            let c = (text @ i)
            if (c == 0:i8)
            elseif ((c == 46:i8) & (i != i0))
                let key =
                    Symbol (string (& (text @ i0)) (i - i0))
                let subkey = ('@ scope key)
                let i = (i + 1)
                if ('constant? subkey)
                    let T = ('typeof subkey)
                    if (T == Scope)
                        repeat i i (subkey as Scope)
                    elseif (T == type)
                        repeat i i
                            fold (scope = (Scope)) for k v in ('symbols (subkey as type))
                                'bind scope k v

            else
                repeat i0 (i + 1) scope
            sc_prompt_add_completion_from_scope ctx text i0 scope
            return;

    global autocomplete-scope : Scope
    fn autocomplete (text ctx)
        # Tab on an empty string gives an indentation
        if ((@ text) == 0)
            sc_prompt_add_completion ctx "    "
            return;
        let scope = (deref autocomplete-scope)
        if (scope != null)
            try
                autocomplete-symbol text ctx scope
            else;
        ;

    loop (preload cmdlist counter eval-scope = "" "" 0 eval-scope)
        autocomplete-scope = eval-scope

        sc_prompt_set_autocomplete_handler autocomplete

        fn make-idstr (counter)
            .. "$" (tostring counter)

        let idstr = (make-idstr counter)
        let promptstr =
            .. idstr " "
                default-styler style-comment "►"
        let promptlen = ((countof idstr) + 2:usize)
        let success cmd =
            sc_prompt
                ..
                    if (empty? cmdlist) promptstr
                    else
                        repeat-string promptlen "."
                    " "
                preload
        if (not (empty? history-path))
            sc_prompt_save_history history-path
        if (not success)
            return;
        fn endswith-blank (s)
            let slen = (countof s)
            if (slen == 0:usize) false
            else
                (@ s (slen - 1:usize)) == 32:i8
        let enter-multiline = (endswith-blank cmd)
        let terminated? =
            (blank? cmd) or
                (empty? cmdlist) and (not enter-multiline)
        let cmdlist =
            .. cmdlist
                if enter-multiline
                    lslice cmd ((countof cmd) - 1:usize)
                else cmd
                "\n"
        let preload =
            if terminated? ""
            else (leading-spaces cmd)
        if (not terminated?)
            repeat preload cmdlist counter eval-scope

        spice count-folds (x key values...)
            let x = (x as i32)
            let tmp = (Symbol "#result...")
            let key = (key as Symbol)
            if (key == unnamed)
                return x
            if (key == tmp)
                return x
            x + 1

        fn unlocal (x)
            let T = ('qualifiersof x)
            if ('refer? T)
                if ((sc_refer_storage_class T) == 'Function)
                    # convert local to global
                    let globalx =
                        extern-new unnamed ('typeof x)
                            storage-class = 'Private
                    let block = (sc_expression_new)
                    sc_expression_append block `(store (deref x) globalx)
                    sc_expression_append block `(ptrtoref globalx)
                    return (sc_prove block)
            x

        spice append-to-scope (scope key docstr vals...)
            let tmp = (Symbol "#result...")
            if ((key != tmp) and (key != unnamed))
                if (('argcount vals...) != 1)
                    let block = (sc_expression_new)
                    let acount = ('args vals...)
                    let outargs = `(alloca-array Value acount)
                    sc_expression_append block outargs
                    for i arg in (enumerate ('args vals...))
                        sc_expression_append block
                            if (('typeof arg) == Value)
                                `(store ``arg (getelementptr outargs i))
                            else
                                let arg = (unlocal arg)
                                `(store `arg (getelementptr outargs i))
                    sc_expression_append block
                        `('bind-with-docstring scope key (sc_argument_list_new acount outargs) docstr)
                    return block
                else
                    return
                        spice-quote
                            'bind-with-docstring scope key [(unlocal vals...)] docstr
            scope

        sugar fold-imports ((eval-scope as Scope))
            loop (scope outscope = sugar-scope `eval-scope)
                if (scope == eval-scope)
                    break outscope
                if (scope == null)
                    break outscope
                let stmt =
                    qq [fold-locals] [outscope] [append-to-scope]
                let expr = (sc_expand stmt '() scope)
                repeat ('parent scope) expr

        spice print-bound-names (key vals...)
            let outargs =
                sc_argument_list_map_new ('argcount vals...)
                    inline (i)
                        let arg = ('getarg vals... i)
                        `(repr arg)
            spice-quote
                print key [(default-styler style-operator "=")] outargs

        spice print-retargs (_inserts counter vals...)
            let inserts = (_inserts as i32)
            let counter = (counter as i32)
            let count = ('argcount vals...)
            if inserts
                if (count != 0)
                    let outargs =
                        sc_argument_list_map_new ('argcount vals...)
                            inline (i)
                                let arg = ('getarg vals... i)
                                `(repr arg)
                    return
                        spice-quote
                            print outargs
                            _inserts
            elseif (count != 0)
                let outargs =
                    sc_argument_list_map_new (count * 2 + 1)
                        inline (i)
                            if (i == count)
                                return `[(default-styler style-operator "=")]
                            elseif (i > count)
                                let i = (i - count - 1)
                                let arg = ('getarg vals... i)
                                return `(repr arg)
                            let arg = ('getarg vals... i)
                            let idstr = (make-idstr (counter + i))
                            let idsym = (Symbol idstr)
                            `idstr
                return
                    spice-quote
                        print outargs
                        _inserts
            _inserts

        spice handle-retargs (inserts counter eval-scope vals...)
            let inserts = (inserts as i32)
            let counter = (counter as i32)
            let count = ('argcount vals...)
            if inserts
            elseif (count != 0)
                local eval-scope = eval-scope
                let block = (sc_expression_new)
                for i in (range count)
                    let arg = ('getarg vals... i)
                    let idstr = (make-idstr (counter + i))
                    let idsym = (Symbol idstr)
                    eval-scope =
                        do
                            let eval-scope = (deref eval-scope)
                            if (('typeof arg) == Value)
                                `('bind eval-scope idsym ``arg)
                            else
                                let arg = (unlocal arg)
                                `('bind eval-scope idsym `arg)
                    sc_expression_append block eval-scope
                let counter = (counter + count)
                let eval-scope = (deref eval-scope)
                return
                    spice-quote
                        block
                        _ counter eval-scope
            spice-quote
                _ counter eval-scope

        fn get-bound-name (eval-scope expr)
            if ((countof expr) == 1)
                let at = ('@ expr)
                if (('typeof at) == Symbol)
                    let at = (at as Symbol)
                    try
                        return at ('@ eval-scope at)
                    except (err)
            _ unnamed `()

        let counter eval-scope =
            try
                let user-expr = (list-parse cmdlist)
                let expression-anchor = ('anchor user-expr)
                let user-expr = (user-expr as list)
                let bound-name bound-val = (get-bound-name eval-scope user-expr)
                if (bound-name != unnamed)
                    # just print the value
                    @@ spice-quote
                    fn expr ()
                        raising Error
                        print-bound-names bound-name bound-val
                    let f = (sc_compile (sc_typify_template expr 0 null) 0:u64)
                    let fptr = (f as (pointer (raises (function void) Error)))
                    fptr;
                    _ counter eval-scope
                else
                    let tmp = (Symbol "#result...")
                    let list-expression =
                        qq
                            [raising] [Error]
                            [let] [tmp] =
                                [embed]
                                    unquote-splice user-expr
                            [let]
                                inserted =
                                    [print-retargs]
                                        [fold-locals] 0 [count-folds]
                                        \ [counter] [tmp]
                                eval-scope = ([fold-imports] [eval-scope])
                            [handle-retargs] inserted
                                \ [counter] eval-scope [tmp]
                    hide-traceback;
                    let expression = (sc_eval
                        expression-anchor list-expression (Scope eval-scope))
                    let f = (sc_compile expression 0:u64)
                    let fptr =
                        f as (pointer (raises (function (Arguments i32 Scope)) Error))
                    fptr;
            except (exc)
                'dump exc
                _ counter eval-scope
        repeat "" "" counter eval-scope

if main-module?
    read-eval-print-loop (globals) true
        .. cache-dir "/console.history"

do
    let read-eval-print-loop
    locals;

