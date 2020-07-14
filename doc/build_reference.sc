#!/usr/bin/env scopes
using import Array
using import Map
using import Set
import UTF-8
let char = UTF-8.char

let argc argv = (launch-args)
assert (argc >= 3)
let scriptpath =
    string (argv @ 1)
let modulepath module =
    if ((string (argv @ 2)) == "core.sc")
        _ "core.sc" (globals)
    else
        let modulepath =
            string (argv @ 2)
        _ modulepath
            (require-from module-dir modulepath) as Scope
let chapter = (string (argv @ 3))

let module = (module as Scope)

fn starts-with-letter (s)
    let c = (s @ 0)
    or
        (c >= (char "a")) and (c <= (char "z"))
        (c >= (char "A")) and (c <= (char "Z"))

let EntryT = (tuple Symbol Value)
local objs : (GrowingArray EntryT)

local used-keys : (Set Symbol)
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
        if (k == unnamed)
            continue;
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

let asterisk-char = 42:i8 # "*"

fn docstring-is-complete (str)
    str @ 0 == asterisk-char

fn repeat-string (n c)
    loop (i s = 0:usize "")
        if (i == n)
            return s
        _ (i + 1:usize)
            .. s c

fn write-docstring (docstring-indent tab str)
    let c = (countof str)
    if (c > 0)
        io-write! tab
        for i in (range c)
            let s = (slice str i (i + 1))
            io-write! s
            if ((s == "\n") and ((i + 1) != c))
                io-write! docstring-indent

fn get-defstring (def-parent-name def-type def-name)
    # e.g. *type*{.property} `Array`{.descname} [](#scopes.type.Array "Permalink to this definition"){.headerlink} {#scopes.type.Array}
    let parent-name =
        if (empty? def-parent-name)
            "scopes"
        else
            "scopes." .. def-parent-name
    let def-id = (parent-name .. "." .. def-type .. "." .. def-name)
    return ("*" .. def-type .. "*{.property} " ..
            "`" .. def-name .. "`{.descname} " ..
            "[](#" .. def-id .. " \"Permalink to this definition\"){.headerlink} " ..
            "{#" .. def-id .. "}")

fn get-fnstring (fn-parent-name fn-type fn-name fn-args)
    # e.g. *fn*{.property} `__@`{.descname} (self index) [](#scopes.type.Array "Permalink to this definition"){.headerlink} {#scopes.fn.__@}
    let parent-name =
        if (empty? fn-parent-name)
            "scopes"
        else
            "scopes." .. fn-parent-name

    let fn-id = (parent-name .. "." .. fn-type .. "." .. fn-name)
    return ("*" .. fn-type .. "*{.property} " ..
            "`" .. fn-name .. "`{.descname} " ..
            (? (empty? fn-args) "()" ("(*&ensp;" .. fn-args .. "&ensp;*)")) ..
            "[](#" .. fn-id .. " \"Permalink to this definition\"){.headerlink} " ..
            "{#" .. fn-id .. "}")

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
        if typemember? "    "
        else ""
    let docstring-indent =
        if typemember? "        "
        else "    "
    let tab =
        if typemember? "   "
        else "   "
    if (docstring-is-complete docstr)
        if typemember?
            io-write! indent
            write-docstring indent "" docstr
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

            local args = ""
            let count = (sc_template_parameter_count label)
            for i in (range count)
                let param = (sc_template_parameter label i)
                args ..= ((sc_parameter_name param) as string)
                if (i < (count - 1))
                    args ..= " "

            io-write! (get-fnstring parent-name (? (sc_template_is_inline label) "inline" "fn") key args)
            io-write! "\n\n"
            io-write! indent
            io-write! ":"

            if (empty? docstr)
                if (empty? docstr2)
                    write-docstring docstring-indent tab "\n"
                else
                    write-docstring docstring-indent tab docstr2
            else
                write-docstring docstring-indent tab docstr

            io-write! "\n"
        return;
    elseif (T == Builtin)
        io-write! indent
        io-write! (get-fnstring parent-name "builtin" key "...")
        io-write! "\n\n"
        io-write! indent
        io-write! ":"
    elseif (T == SugarMacro)
        io-write! indent

        let parent-name =
            if (empty? parent-name)
                "scopes"
            else
                "scopes." .. parent-name
        let sugar-id = (parent-name .. ".sugar." .. key)
        io-write! (
            "*sugar*{.property} (`" .. key .. "`{.descname} *&ensp;...&ensp;*)" ..
            " [](#" .. sugar-id .. " \"Permalink to this definition\"){.headerlink} " ..
            "{#" .. sugar-id .. "}")

        io-write! "\n\n"
        io-write! indent
        io-write! ":"
    elseif (T == SpiceMacro)
        io-write! indent
        io-write! (get-fnstring parent-name "spice" key "...")
        io-write! "\n\n"
        io-write! indent
        io-write! ":"
    elseif (T == type)
        if (typemember? and (not has-docstr))
            return;
        let ty = (entry as type)
        io-write! indent
        io-write! (get-defstring parent-name "type" key)
        let superT = ('superof ty)
        let ST =
            if ('opaque? ty) Unknown
            else ('storageof ty)
        io-write! "\n\n"
        io-write! indent
        io-write! ":"
        if has-docstr
            write-docstring docstring-indent tab docstr
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
                io-write! " labeled `"
                io-write! (tostring ty)
                io-write! "`"
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
        io-write! (get-fnstring parent-name "compiledfn" key "...")
        io-write! "\n\n"
        io-write! indent
        io-write! ":"
        io-write! tab
        io-write! "A"
        if (('kind entry) == value-kind-global)
            io-write! "n external"
        else
            io-write! " compiled"
        io-write! " function of type `"
        io-write! (tostring fntype)
        io-write! "`.\n"

        if (not has-docstr)
            io-write! "\n"
            return;

    elseif (T == Unknown)
        # a dreaded curse!
        return;
    else
        if (typemember? and (not has-docstr))
            return;
        io-write! indent
        io-write! (get-defstring parent-name "define" key)
        io-write! "\n\n"
        io-write! indent
        io-write! ":"
        if (not has-docstr)
            io-write! tab
            io-write! "A constant of type `"
            io-write! (tostring T)
            io-write! "`.\n\n"
            return;

    if (not has-docstr)
        write-docstring docstring-indent tab "\n"
    else
        write-docstring docstring-indent tab docstr

    io-write! "\n"

let moduledoc = ('module-docstring module)
if (not (empty? moduledoc))
    io-write! "<style type=\"text/css\" rel=\"stylesheet\">"
    io-write! ("body { counter-reset: chapter " .. chapter .. "; }")
    io-write! "</style>\n\n"
    io-write! (moduledoc as string)
    io-write! "\n"

for entry in objs
    let key entry = (unpack entry)
    print-entry module module (deref key) (deref entry) ""

