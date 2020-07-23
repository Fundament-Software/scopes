<style type="text/css" rel="stylesheet">body { counter-reset: chapter 12; }</style>

FunctionChain
=============

A function chain implements a compile-time observer pattern that allows
a module to call back into dependent modules in a decoupled way.

See following example:

    :::scopes
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

    :::text
    first handler activated with argument 1
    handler activated with argument 1
    last handler activated with argument 1
    first handler activated with argument 2
    handler activated with argument 2
    last handler activated with argument 2
    handler activated with argument 3

*type*{.property} `FunctionChain`{.descname} [](#scopes.type.FunctionChain "Permalink to this definition"){.headerlink} {#scopes.type.FunctionChain}

:   A plain type of storage type `(opaque@ _type)`.

    *spice*{.property} `__call`{.descname} (*&ensp;...&ensp;*)[](#scopes.FunctionChain.spice.__call "Permalink to this definition"){.headerlink} {#scopes.FunctionChain.spice.__call}

    :   

    *inline*{.property} `__repr`{.descname} (*&ensp;self&ensp;*)[](#scopes.FunctionChain.inline.__repr "Permalink to this definition"){.headerlink} {#scopes.FunctionChain.inline.__repr}

    :   

    *spice*{.property} `__typecall`{.descname} (*&ensp;...&ensp;*)[](#scopes.FunctionChain.spice.__typecall "Permalink to this definition"){.headerlink} {#scopes.FunctionChain.spice.__typecall}

    :   

    *inline*{.property} `append`{.descname} (*&ensp;self f&ensp;*)[](#scopes.FunctionChain.inline.append "Permalink to this definition"){.headerlink} {#scopes.FunctionChain.inline.append}

    :   Append function `f` to function chain. When the function chain is called,
        `f` will be called last. The return value of `f` will be ignored.

    *inline*{.property} `clear`{.descname} (*&ensp;self&ensp;*)[](#scopes.FunctionChain.inline.clear "Permalink to this definition"){.headerlink} {#scopes.FunctionChain.inline.clear}

    :   Clear the function chain. When the function chain is applied next,
        no functions will be called.

    *inline*{.property} `on`{.descname} (*&ensp;self&ensp;*)[](#scopes.FunctionChain.inline.on "Permalink to this definition"){.headerlink} {#scopes.FunctionChain.inline.on}

    :   Returns a decorator that appends the provided function to the
        function chain.

    *inline*{.property} `prepend`{.descname} (*&ensp;self f&ensp;*)[](#scopes.FunctionChain.inline.prepend "Permalink to this definition"){.headerlink} {#scopes.FunctionChain.inline.prepend}

    :   Prepend function `f` to function chain. When the function chain is called,
        `f` will be called first. The return value of `f` will be ignored.

*sugar*{.property} (`decorate-fnchain`{.descname} *&ensp;...&ensp;*) [](#scopes.sugar.decorate-fnchain "Permalink to this definition"){.headerlink} {#scopes.sugar.decorate-fnchain}

:   

*sugar*{.property} (`fnchain`{.descname} *&ensp;name&ensp;*) [](#scopes.sugar.fnchain "Permalink to this definition"){.headerlink} {#scopes.sugar.fnchain}

:   Binds a new unique and empty function chain to identifier `name`. The
    function chain's typename is going to incorporate the name of the module
    in which it was declared.

