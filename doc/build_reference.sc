#!/usr/bin/env scopes
using import Array

fn starts-with-letter (s)
    let c = (s @ 0)
    or
        (c >= (char "a")) and (c <= (char "z"))
        (c >= (char "A")) and (c <= (char "Z"))

let EntryT = (tuple (unknownof Symbol) (unknownof Any))
let objs = (local (Array EntryT))

loop (scope) = (globals)
if (scope != null)
    let a b = (Scope-parent scope)
    for k v in scope
        let T = ('typeof v)
        #if
            or
                T == Closure
                T == Builtin
                T == Macro
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

print
    """"Scopes Language Reference
        =========================
for entry in objs
    let key entry = (unpack entry)
    let T = ('typeof entry)
    let key = (key as string)
    let prefix = (slice key 0 1)
    if (prefix == "#")
        continue;
    elseif (prefix == "_")
        continue;
    elseif (T == Closure)
        let func = (entry as Closure)
        let docstr = (docstring func)
        let label =
            Closure-label func
        if (docstring-is-complete docstr)
            io-write! docstr
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
        io-write! key
        io-write! " ...)\n"
    elseif (T == Macro)
        io-write! ".. macro:: ("
        io-write! key
        io-write! " ...)\n"
    elseif (T == type)
        io-write! ".. type:: "
        io-write! key
        io-write! "\n"
    elseif (function-pointer-type? T)
        let fntype = (@ T)
        let params = (countof fntype)
        io-write! ".. compiledfn:: ("
        io-write! key
        io-write! " ...)\n\n"
        io-write! "   ``"
        io-write! (string-repr fntype)
        io-write! "``\n"
    elseif (T < extern)
        continue;
    else
        io-write! ".. define:: "
        io-write! key
        io-write! "\n"
        #for i param in (enumerate ('parameters label))
            print i param

