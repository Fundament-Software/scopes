#!/usr/bin/env scopes
using import Array
using import Map
import UTF-8
let char = UTF-8.char

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

local used-keys : (Set Symbol)
#let used-keys = (typename "used-keys")
loop (scope = module)
    if (scope == null)
        break;
    let parent = ('parent scope)
    for k in ('deleted scope)
        if (('typeof k) != Symbol)
            continue;
        k := k as Symbol
        if (not ('in? used-keys k))
            'insert used-keys k
    for k v in scope
        if (('typeof k) != Symbol)
            continue;
        k := k as Symbol
        if (not ('in? used-keys k))
            'insert used-keys k
            let T = ('typeof v)
            'append objs
                tupleof k v
            ;
    repeat parent

fn entry-key (x)
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

'sort objs entry-key

fn member-key (x)
    let key entry = (unpack x)
    let T = ('typeof entry)
    let s = (key as string)
    s

fn docstring-is-complete (str)
    (slice str 0 2) == ".."

fn repeat-string (n c)
    loop (i s = 0:usize "")
        if (i == n)
            return s
        _ (i + 1:usize)
            .. s c

fn write-docstring (tab str)
    let c = (countof str)
    if (c > 0)
        io-write! tab
        io-write! "\n"
        io-write! tab
        for i in (range c)
            let s = (slice str i (i + 1))
            io-write! s
            if ((s == "\n") and ((i + 1) != c))
                io-write! tab

fn print-entry (module parent key entry parent-name opts...)
    let print-entry = this-function
    let typemember? = ((typeof parent) == type)
    let sym = key
    let key = (key as string)
    let prefix1 = (slice key 0 1)
    let prefix2 = (slice key 0 2)
    if (prefix1 == "#")
        return;
    elseif
        and (prefix2 == "__")
            not typemember?
                #and typemember?
                    or
                        sym == '__typecall
                        sym == '__call
        return;
    let T = ('typeof entry)
    let docstr =
        static-if typemember?
            'docstring parent sym
        else
            'docstring module sym
            #parent @ (Symbol (.. "#doc:" key))
    let has-docstr = (not (empty? docstr))
    let docstr =
        if has-docstr (docstr as string)
        else ""
    let indent =
        if typemember? "   "
        else ""
    let tab =
        if typemember? "      "
        else "   "
    if (docstring-is-complete docstr)
        if typemember?
            write-docstring indent docstr
        else
            io-write! docstr
            io-write! "\n"
        return;
    if (T == Closure)
        let func = (entry as Closure)
        let docstr2 = ('docstring func)
        let label =
            sc_closure_get_template func
        if (docstring-is-complete docstr2)
            io-write! docstr2
            io-write! "\n"
        else
            io-write! indent
            if (sc_template_is_inline label)
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
            if (empty? docstr)
                write-docstring tab docstr2
            else
                write-docstring tab docstr
        return;
    elseif (T == Builtin)
        io-write! indent
        io-write! ".. builtin:: ("
        io-write! key
        io-write! " ...)\n"
    elseif (T == SugarMacro)
        io-write! indent
        io-write! ".. sugar:: ("
        io-write! key
        io-write! " ...)\n"
    elseif (T == SpiceMacro)
        io-write! indent
        io-write! ".. spice:: ("
        io-write! key
        io-write! " ...)\n"
    elseif (T == type)
        if (typemember? and (not has-docstr))
            return;
        let ty = (entry as type)
        io-write! indent
        io-write! ".. type:: "
        io-write! key
        let superT = ('superof ty)
        let ST =
            if ('opaque? ty) Unknown
            else ('storageof ty)
        io-write! "\n\n"
        if has-docstr
            write-docstring tab docstr
            io-write! "\n"
        else
            io-write! tab
            let has-storage? = (ST != Unknown)
            let plain? = ('plain? ty)
            io-write! "A"
            if has-storage?
                if plain?
                    io-write! " plain"
                else
                    io-write! "n unique"
            else
                io-write! "n opaque"
            io-write! " type"
            let tystr = (tostring ty)
            if ((let tystr = (tostring ty)) != key)
                io-write! " labeled ``"
                io-write! (tostring ty)
                io-write! "``"
            if (superT != typename)
                io-write! " of supertype `"
                io-write! (tostring superT)
                io-write! "`"
                if has-storage?
                    io-write! " and"
            if has-storage?
                io-write! " of storage type `"
                io-write! (tostring ST)
                io-write! "`"
            io-write! ".\n\n"
        if (not typemember?)
            local members : (GrowingArray EntryT)
            for k v in ('symbols ty)
                'append members
                    tupleof k v
            'sort members member-key
            for entry in members
                let k v = (unpack entry)
                let k = (dupe (deref k))
                let v = (dupe (deref v))
                print-entry module ty k v key
        return;
    elseif ('function-pointer? T)
        let fntype = ('element@ T 0)
        let params = ('element-count fntype)
        io-write! indent
        io-write! ".. compiledfn:: ("
        io-write! key
        io-write! " ...)\n\n"
        io-write! tab
        io-write! "A"
        if (('kind entry) == value-kind-global)
            io-write! "n external"
        else
            io-write! " compiled"
        io-write! " function of type ``"
        io-write! (tostring fntype)
        io-write! "``.\n"
    elseif (T == Unknown)
        # a dreaded curse!
        return;
    else
        if (typemember? and (not has-docstr))
            return;
        io-write! indent
        io-write! ".. define:: "
        io-write! key
        io-write! "\n"
        if (not has-docstr)
            io-write! "\n"
            io-write! tab
            io-write! "A constant of type `"
            io-write! (tostring T)
            io-write! "`.\n"
    write-docstring tab docstr

let moduledoc = ('docstring module unnamed)
if (not (empty? moduledoc))
    io-write! (moduledoc as string)
    io-write! "\n"

for entry in objs
    let key entry = (unpack entry)
    print-entry module module (deref key) (deref entry) ""

