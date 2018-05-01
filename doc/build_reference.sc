#!/usr/bin/env scopes
using import Array

let scriptpath modulepath = (args)
let module =
    if (none? modulepath)
        Any (globals)
    else
        load-module "" modulepath

let module = (module as Scope)

fn starts-with-letter (s)
    let c = (s @ 0)
    or
        (c >= (char "a")) and (c <= (char "z"))
        (c >= (char "A")) and (c <= (char "Z"))

let EntryT = (tuple (unknownof Symbol) (unknownof Any))
let objs = (local (Array EntryT))

loop (scope) = module
if (scope != null)
    let a b = (Scope-parent scope)
    for k v in scope
        let T = ('typeof v)
        'append objs
            tupleof k v
    repeat (Scope-parent scope)

'sort objs
    fn (x)
        let key entry = (unpack x)
        let T = ('typeof entry)
        let s = (key as string)
        ..
            do
                if (T == Closure) "C"
                elseif (T == Builtin) "E"
                elseif (T == Macro) "D"
                elseif (function-pointer-type? T) "F"
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
    let loop (i s) =
        tie-const n (usize 0)
        tie-const n ""
    if (i == n)
        return s
    loop (i + (usize 1))
        .. s c

fn print-entry (parent key entry parent-name)
    let key = (key as string)
    let prefix1 = (slice key 0 1)
    let prefix2 = (slice key 0 2)
    if (prefix1 == "#")
        return;
    elseif (prefix2 == "__")
        return;
    let T = ('typeof entry)
    let typemember? = ((typeof parent) == type)
    let docstr has-docstr =
        if typemember?
            unconst-all "" false
        else
            parent @ (Symbol (.. "#doc:" key))
    let docstr =
        if has-docstr (docstr as string)
        else (unconst "")
    if (docstring-is-complete docstr)
        io-write! docstr
        io-write! "\n"
        return;
    if (T == Closure)
        let func = (entry as Closure)
        let docstr = (docstring func)
        let label =
            Closure-label func
        if (docstring-is-complete docstr)
            io-write! docstr
            io-write! "\n"
        else
            if typemember?
                io-write! ".. typefn:: ("
                io-write! parent-name
                io-write! " '"
            else
                io-write! ".. fn:: ("
            io-write! key
            for i param in (enumerate ('parameters label))
                if (i > 0)
                    io-write! " "
                    io-write!
                        (Parameter-name param) as string
            io-write! ")\n"
            write-docstring docstr
    elseif (T == Builtin)
        io-write! ".. builtin:: ("
        if typemember?
            io-write! parent-name; io-write! "."
        io-write! key
        io-write! " ...)\n"
    elseif (T == Macro)
        io-write! ".. macro:: ("
        if typemember?
            io-write! parent-name; io-write! "."
        io-write! key
        io-write! " ...)\n"
    elseif (T == type)
        if (typemember? and (not has-docstr))
            return;
        io-write! ".. type:: "
        if typemember?
            io-write! parent-name; io-write! "."
        io-write! key
        io-write! "\n"
        if (not typemember?)
            let ty = (entry as type)
            for k v in (typename.symbols ty)
                print-entry ty k v key
                #print k v
    elseif (function-pointer-type? T)
        let fntype = (@ T)
        let params = (countof fntype)
        io-write! ".. compiledfn:: ("
        if typemember?
            io-write! parent-name; io-write! "."
        io-write! key
        io-write! " ...)\n\n"
        io-write! "   ``"
        io-write! (string-repr fntype)
        io-write! "``\n"
    elseif (T < extern)
        return;
    else
        if (typemember? and (not has-docstr))
            return;
        io-write! ".. define:: "
        if typemember?
            io-write! parent-name; io-write! "."
        io-write! key
        io-write! "\n"
    write-docstring docstr

let moduledoc = (Scope-docstring module)
if (not (empty? moduledoc))
    io-write! (moduledoc as string)
    io-write! "\n"

for entry in objs
    let key entry = (unpack entry)
    print-entry module key entry