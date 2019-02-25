#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""FunctionChain
    =============

    A function chain implements a compile-time observer pattern that allows
    a module to call back into dependent modules in a decoupled way.

    See following example::

        using import FunctionChain

        # declare new function chain
        fnchain activate

        fn handler (x)
            print "handler activated with argument" x

        'append activate handler

        'append activate
            fn (x)
                print "last handler activated with argument" x

        'prepend activate
            fn (x)
                print "first handler activated with argument" x

        activate 1
        activate 2
        'clear activate
        'append activate handler
        activate 3

    Running this program will output:

    ..  code-block:: none

        first handler activated with argument 1
        handler activated with argument 1
        last handler activated with argument 1
        first handler activated with argument 2
        handler activated with argument 2
        last handler activated with argument 2
        handler activated with argument 3


spice clear (cls)
    """"Clear the function chain. When the function chain is applied next,
        no functions will be called.
    let cls = (cls as type)
    method '__typecall cls (cls args...)
    cls

spice append (cls f)
    """"Append function `f` to function chain. When the function chain is called,
        `f` will be called last. The return value of `f` will be ignored.
    let cls = (cls as type)
    let oldfn = cls.__typecall

    @@ ast-quote
    method '__typecall cls (cls args...)
        oldfn cls args...
        f args...

    cls

spice prepend (cls f)
    """"Prepend function `f` to function chain. When the function chain is called,
        `f` will be called first. The return value of `f` will be ignored.
    let cls = (cls as type)
    let oldfn = cls.__typecall

    @@ ast-quote
    method '__typecall cls (cls args...)
        f args...
        oldfn cls args...

    cls

inline on (cls)
    """"Returns a decorator that appends the provided function to the
        function chain.
    inline (f)
        'append cls f
        f

let FunctionChain = (define-typename "FunctionChain")
'define-symbol FunctionChain '__typecall
    fn (cls name)
        let T = (typename (.. "<FunctionChain " name ">"))
        'set-super T cls
        'set-symbols T
            clear = clear
            append = append
            prepend = prepend
            on = on
        method '__typecall T (cls args...)
        T

"""".. macro:: (fnchain name)

       Binds a new unique and empty function chain to identifier `name`. The
       function chain's typename is going to incorporate the name of the module
       in which it was declared.
sugar fnchain ((name as Symbol))
    list let name '=
        FunctionChain
            ..
                syntax-scope.module-name as string
                \ "." (name as string)

let decorate-fnchain = decorate-fn

do
    let FunctionChain fnchain decorate-fnchain
    locals;
