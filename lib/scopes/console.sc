#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

#-------------------------------------------------------------------------------
# Scopes Console
#-------------------------------------------------------------------------------

fn read-eval-print-loop ()
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
            if (c != (char " "))
                let s = (sc_string_buffer s)
                return (sc_string_new s i)
            repeat (i + 1:usize)

    fn blank? (s)
        let len = (countof s)
        loop (i = 0:usize)
            if (i == len)
                return true
            if ((@ s i) != (char " "))
                return false
            repeat (i + 1:usize)

    let cwd =
        realpath "."

    print-logo;

    let global-scope = (globals)
    let eval-scope = (Scope global-scope)
    let history-path =
        .. cache-dir "/console.history"

    set-autocomplete-scope! eval-scope
    sc_load_history history-path

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
                sc_write (sc_parameter_name param)
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

    'set-symbols eval-scope
        module-dir = cwd
        module-path = (cwd .. "/<console>.sc")
        module-name = "<console>"
        main-module? = true
        sh = sh
        help = help
        exit =
            typedef (do "Enter 'exit;' or Ctrl+D to exit")
                inline __typecall () (if true (exit 0))

    loop (preload cmdlist counter = "" "" 0)
        fn make-idstr (counter)
            .. "$" (tostring counter)

        let idstr = (make-idstr counter)
        let promptstr =
            .. idstr " "
                default-styler style-comment "►"
        let promptlen = ((countof idstr) + 2:usize)
        let success cmd =
            __prompt
                ..
                    if (empty? cmdlist) promptstr
                    else
                        repeat-string promptlen "."
                    " "
                preload
        sc_save_history history-path
        if (not success)
            return;
        fn endswith-blank (s)
            let slen = (countof s)
            if (slen == 0:usize) false
            else
                (@ s (slen - 1:usize)) == (char " ")
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
            repeat preload cmdlist counter

        spice count-folds (x key values...)
            let x = (x as i32)
            let tmp = (Symbol "#result...")
            let key = (key as Symbol)
            if (key == unnamed) # module docstring
                return x
            if (key == tmp)
                return x
            x + 1

        spice append-to-scope (scope key docstr vals...)
            let tmp = (Symbol "#result...")
            if ((key != tmp) and (key != unnamed))
                if (('argcount vals...) != 1)
                    let block = (sc_expression_new)
                    let outargs = `(sc_argument_list_new)
                    sc_expression_append block outargs
                    for arg in ('args vals...)
                        sc_expression_append block
                            if (('typeof arg) == Value)
                                `(sc_argument_list_append outargs ``arg)
                            else
                                `(sc_argument_list_append outargs `arg)
                    sc_expression_append block
                        `('set-symbol scope key outargs)
                    sc_expression_append block
                        `('set-docstring scope key docstr)
                    sc_expression_append block scope
                    return block
                else
                    return
                        spice-quote
                            'set-symbol scope key vals...
                            'set-docstring scope key docstr
                            scope
            scope

        sugar fold-imports ((eval-scope as Scope))
            let stmt =
                qq [fold-locals] [eval-scope] [append-to-scope]
            loop (scope output = sugar-scope '())
                if (scope == eval-scope)
                    break (cons embed output)
                if (scope == null)
                    break (cons embed output)
                let expr = (sc_expand stmt '() scope)
                repeat ('parent scope) (cons expr output)

        spice print-bound-names (key vals...)
            let outargs = (sc_argument_list_new)
            for arg in ('args vals...)
                sc_argument_list_append outargs `(repr arg)
            spice-quote
                print key [(default-styler style-operator "=")] outargs

        spice handle-retargs (inserts counter eval-scope vals...)
            let inserts = (inserts as i32)
            let counter = (counter as i32)
            let count = ('argcount vals...)
            if inserts
                let outargs = (sc_argument_list_new)
                if (count != 0)
                    for arg in ('args vals...)
                        sc_argument_list_append outargs `(repr arg)
                    return
                        spice-quote
                            print outargs
                            counter
            elseif (count != 0)
                let outargs = (sc_argument_list_new)
                let block = (sc_expression_new)
                let eval-scope = (eval-scope as Scope)
                fold (count = 0) for arg in ('args vals...)
                    let idstr = (make-idstr (counter + count))
                    sc_argument_list_append outargs `idstr
                    let idstr = (Symbol idstr)
                    sc_expression_append block
                        if (('typeof arg) == Value)
                            `('set-symbol eval-scope idstr ``arg)
                        else
                            `('set-symbol eval-scope idstr `arg)
                    count + 1
                sc_argument_list_append outargs (default-styler style-operator "=")
                for arg in ('args vals...)
                    sc_argument_list_append outargs `(repr arg)
                let counter = (counter + count)
                return
                    spice-quote
                        block
                        print outargs
                        counter
            `counter

        fn get-bound-name (eval-scope expr)
            if ((countof expr) == 1)
                let at = ('@ expr)
                if (('typeof at) == Symbol)
                    let at = (at as Symbol)
                    try
                        return at ('@ eval-scope at)
                    except (err)
            _ unnamed `()

        let counter =
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
                    counter
                else
                    let tmp = (Symbol "#result...")
                    let list-expression =
                        qq
                            [raising] [Error]
                            [let] [tmp] =
                                [embed]
                                    unquote-splice user-expr
                            [fold-imports] [eval-scope]
                            [handle-retargs]
                                [fold-locals] 0 [count-folds]
                                \ [counter] [eval-scope] [tmp]
                    hide-traceback;
                    let expression = (sc_eval
                        expression-anchor list-expression (Scope eval-scope))
                    let f = (sc_compile expression 0:u64)
                    let fptr =
                        f as (pointer (raises (function i32) Error))
                    fptr;
            except (exc)
                'dump exc
                counter
        repeat "" "" counter

if main-module?
    read-eval-print-loop;

do
    let read-eval-print-loop
    locals;

