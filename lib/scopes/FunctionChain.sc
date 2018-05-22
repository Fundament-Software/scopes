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


let FunctionChain = (typename "FunctionChain")

typefn FunctionChain 'clear (self)
    """"Clear the function chain. When the function chain is applied next,
        no functions will be called.
    assert (not (type== self FunctionChain))
    assert (constant? self)
    typefn self '__typecall (self args...)
    self
typefn FunctionChain 'append (self f)
    """"Append function `f` to function chain. When the function chain is called,
        `f` will be called last. The return value of `f` will be ignored.
    assert (not (type== self FunctionChain))
    assert (constant? f)
    assert (constant? self)
    let oldfn = self.__typecall
    typefn self '__typecall (self args...)
        oldfn self args...
        f args...
    self
typefn FunctionChain 'prepend (self f)
    """"Prepend function `f` to function chain. When the function chain is called,
        `f` will be called first. The return value of `f` will be ignored.
    assert (not (type== self FunctionChain))
    assert (constant? f)
    assert (constant? self)
    let oldfn = self.__typecall
    typefn self '__typecall (self args...)
        f args...
        oldfn self args...
    self

typeinline FunctionChain '__typecall (cls name)
    let T = (typename (.. "<FunctionChain " name ">"))
    set-typename-super! T cls
    typefn T '__typecall (cls args...)
    T

"""".. macro:: (fnchain name)

       Binds a new unique and empty function chain to identifier `name`. The
       function chain's typename is going to incorporate the name of the module
       in which it was declared.
define-macro fnchain
    let name = (decons args)
    list let name '=
        list FunctionChain
            list (do ..)
                'module-name
                "."
                name as Syntax as Symbol as string

do
    let FunctionChain fnchain
    locals;
