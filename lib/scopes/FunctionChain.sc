#
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.

""""FunctionChain
    =============

    A function chain implements a compile-time observer pattern that allows
    a module to call back into dependent modules in a decoupled way.

    See following example:

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

        first handler activated with argument 1
        handler activated with argument 1
        last handler activated with argument 1
        first handler activated with argument 2
        handler activated with argument 2
        last handler activated with argument 2
        handler activated with argument 3

typedef FunctionChain : (storageof type)
    inline __repr (self)
        repr (bitcast self type)

    inline clear (self)
        """"Clear the function chain. When the function chain is applied next,
            no functions will be called.
        let cls = (bitcast self type)
        'define-symbol cls 'chain
            inline (args...)
        self

    inline append (self f)
        """"Append function `f` to function chain. When the function chain is called,
            `f` will be called last. The return value of `f` will be ignored.
        let cls = (bitcast self type)
        static-assert (constant? cls)
        let oldfn = cls.chain
        'define-symbol cls 'chain
            inline (args...)
                oldfn args...
                f args...
        self

    inline prepend (self f)
        """"Prepend function `f` to function chain. When the function chain is called,
            `f` will be called first. The return value of `f` will be ignored.
        let cls = (bitcast self type)
        static-assert (constant? cls)
        let oldfn = cls.chain
        'define-symbol cls 'chain
            inline (cls args...)
                f args...
                oldfn args...
        self

    inline on (self)
        """"Returns a decorator that appends the provided function to the
            function chain.
        inline (f)
            'append self f
            f

    spice __typecall (cls name)
        let name = (name as string)
        let T = (typename.type (.. "<FunctionChain " name ">") typename)
        'set-opaque T
        'set-symbol T 'chain
            inline (args...)
        bitcast T this-type

    spice __call (self args...)
        let self = (bitcast (self as this-type) type)
        let func = ('@ self 'chain)
        'tag `(func args...) ('anchor args)

""""*sugar*{.property} (`fnchain`{.descname} *&ensp;name&ensp;*) [](#scopes.sugar.fnchain "Permalink to this definition"){.headerlink} {#scopes.sugar.fnchain}

    :   Binds a new unique and empty function chain to identifier `name`. The
        function chain's typename is going to incorporate the name of the module
        in which it was declared.
sugar fnchain (name)
    let symbol? = (('typeof name) == Symbol)
    let namestr =
        if symbol? (name as Symbol as string)
        else (name as string)
    let namestr =
        .. (('@ sugar-scope 'module-name) as string) "." namestr
    let expr = (list FunctionChain namestr)
    if symbol?
        list let name '= expr
    else
        expr

let decorate-fnchain = decorate-fn

do
    let FunctionChain fnchain decorate-fnchain
    locals;
