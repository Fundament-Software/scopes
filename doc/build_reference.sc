#!/usr/bin/env scopes
using import Array

let argc argv = (launch-args)
assert (argc >= 2)
let scriptpath =
    string (argv @ 1)
let modulepath module =
    if (argc < 3)
        _ "core.sc" (globals)
    else
        let modulepath =
            string (argv @ 2)
        _ modulepath
            (require-from module-dir modulepath) as Scope

let module = (module as Scope)

fn starts-with-letter (s)
    let c = (s @ 0)
    or
        (c >= (char "a")) and (c <= (char "z"))
        (c >= (char "A")) and (c <= (char "Z"))

let EntryT = (tuple Symbol Value)
local objs : (GrowingArray EntryT)

loop (scope = module)
    if (scope == null)
        break;
    let parent = ('parent scope)
    let a b = parent
    for k v in scope
        let T = ('typeof v)
        'append objs
            tupleof k v
    repeat parent

'sort objs
    fn (x)
        let key entry = (unpack x)
        let T = ('typeof entry)
        let s = (key as string)
        ..
            do
                if (T == Closure) "C"
                elseif (T == Builtin) "E"
                elseif (T == SugarMacro) "D"
                elseif (T == SpiceMacro) "F"
                elseif ('function-pointer? T) "G"
                elseif (T == type) "B"
                else "A"
            if (starts-with-letter s) s
            else
                if ((countof s) < 2:usize)
                    .. " 1" s
                else
                    .. " 2" s

fn docstring-is-complete (str)
    (slice str 0 2) == ".."

fn write-docstring (str)
    let c = (countof str)
    if (c > 0)
        io-write! "   \n   "
        for i in (range c)
            let s = (slice str i (i + 1))
            io-write! s
            if ((s == "\n") and ((i + 1) != c))
                io-write! "   "

fn repeat-string (n c)
    loop (i s = 0:usize "")
        if (i == n)
            return s
        _ (i + 1:usize)
            .. s c

fn print-entry (module parent key entry parent-name opts...)
    let typemember? = ((typeof parent) == type)
    let sym = key
    let key = (key as string)
    let prefix1 = (slice key 0 1)
    let prefix2 = (slice key 0 2)
    if (prefix1 == "#")
        return;
    elseif
        and (prefix2 == "__")
            not
                and typemember?
                    or
                        sym == '__new
                        sym == '__typecall
                        sym == '__call
        return;
    let T = ('typeof entry)
    let docstr =
        if typemember? ""
        else
            'docstring module sym
            #parent @ (Symbol (.. "#doc:" key))
    let has-docstr = (not (empty? docstr))
    let docstr =
        if has-docstr (docstr as string)
        else ""
    if (docstring-is-complete docstr)
        io-write! docstr
        io-write! "\n"
        return;
    if (T == Closure)
        let func = (entry as Closure)
        let docstr = ('docstring func)
        let label =
            sc_closure_get_template func
        if (docstring-is-complete docstr)
            io-write! docstr
            io-write! "\n"
        else
            if typemember?
                io-write! ".. typefn:: ("
                io-write! parent-name
                io-write! " '"
            elseif (sc_template_is_inline label)
                io-write! ".. inline:: ("
            else
                io-write! ".. fn:: ("
            io-write! key
            let count = (sc_template_parameter_count label)
            for i in (range count)
                let param = (sc_template_parameter label i)
                io-write! " "
                io-write! ((sc_parameter_name param) as string)
            io-write! ")\n"
            write-docstring docstr
    elseif (T == Builtin)
        io-write! ".. builtin:: ("
        if typemember?
            io-write! parent-name; io-write! "."
        io-write! key
        io-write! " ...)\n"
    elseif (T == SugarMacro)
        io-write! ".. sugar:: ("
        if typemember?
            io-write! parent-name; io-write! "."
        io-write! key
        io-write! " ...)\n"
    elseif (T == SpiceMacro)
        io-write! ".. spice:: ("
        if typemember?
            io-write! parent-name; io-write! "."
        io-write! key
        io-write! " ...)\n"
    elseif (T == type)
        if (typemember? and (not has-docstr))
            return;
        let ty = (entry as type)
        io-write! ".. type:: "
        if typemember?
            io-write! parent-name; io-write! "."
        io-write! key
        let superT = ('superof ty)
        let ST =
            if ('opaque? ty) Unknown
            else ('storageof ty)
        io-write! "\n\n   "
        io-write! "``"
        io-write! (tostring ty)
        io-write! "`` "
        if (superT != typename)
            io-write! "< ``"
            io-write! (tostring superT)
            io-write! "`` "
        if (ST != Unknown)
            if (sc_type_is_plain ty)
                io-write! ": "
            else
                io-write! ":: "
            io-write! "``"
            io-write! (tostring ST)
            io-write! "`` "
        io-write! "\n"
        if (not typemember?)
            for k v in ('symbols ty)
                print-entry module ty k v key
    elseif ('function-pointer? T)
        let fntype = ('element@ T 0)
        let params = ('element-count fntype)
        io-write! ".. compiledfn:: ("
        if typemember?
            io-write! parent-name; io-write! "."
        io-write! key
        io-write! " ...)\n\n"
        io-write! "   ``"
        io-write! (tostring fntype)
        io-write! "``\n"
    elseif (T == Unknown)
        # a dreaded curse!
        return;
    else
        if (typemember? and (not has-docstr))
            return;
        io-write! ".. define:: "
        if typemember?
            io-write! parent-name; io-write! "."
        io-write! key
        if false
            io-write! "\n"
        else
            io-write! "\n\n"
            io-write! "   ``"
            io-write! (tostring T)
            io-write! "``\n"
    write-docstring docstr

let moduledoc = ('docstring module unnamed)
if (not (empty? moduledoc))
    io-write! (moduledoc as string)
    io-write! "\n"

for entry in objs
    let key entry = (unpack entry)
    print-entry module module (deref key) (deref entry) ""

