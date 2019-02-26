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

typedef FunctionChain : ('storageof type)
    method '__repr (self)
        repr (bitcast self type)

    method 'clear (self)
        """"Clear the function chain. When the function chain is applied next,
            no functions will be called.
        let cls = (bitcast self type)
        method inline 'chain cls (cls args...)
        self

    method 'append (self f)
        """"Append function `f` to function chain. When the function chain is called,
            `f` will be called last. The return value of `f` will be ignored.
        let cls = (bitcast self type)
        let oldfn = cls.chain
        @@ ast-quote
        method inline 'chain cls (cls args...)
            oldfn cls args...
            f args...
        self

    method 'prepend (self f)
        """"Prepend function `f` to function chain. When the function chain is called,
            `f` will be called first. The return value of `f` will be ignored.
        let cls = (bitcast self type)
        let oldfn = cls.chain
        @@ ast-quote
        method inline 'chain cls (cls args...)
            f args...
            oldfn cls args...
        self

    method inline 'on (self)
        """"Returns a decorator that appends the provided function to the
            function chain.
        inline (f)
            'append self f
            f

    @@ ast-quote
    method '__typecall (cls name)
        let T = (typename (.. "<FunctionChain " name ">"))
        method inline 'chain T (cls args...)
        bitcast T this-type

run-stage;

'set-symbol FunctionChain '__call
    spice "call-fnchain" (self)
        let self = (bitcast (self as FunctionChain) type)
        let func = self.chain
        `(func)

"""".. macro:: (fnchain name)

       Binds a new unique and empty function chain to identifier `name`. The
       function chain's typename is going to incorporate the name of the module
       in which it was declared.
sugar fnchain ((name as Symbol))
    let namestr =
        .. (syntax-scope.module-name as string) "." (name as string)
    list let name '= (list FunctionChain namestr)

let decorate-fnchain = decorate-fn

do
    let FunctionChain fnchain decorate-fnchain
    locals;
