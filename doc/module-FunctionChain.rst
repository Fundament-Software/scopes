FunctionChain
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

.. type:: FunctionChain

   ``FunctionChain`` : ``_type(*)`` 
.. spice:: (FunctionChain.__call ...)
.. typefn:: (FunctionChain 'append self f)
   
   Append function `f` to function chain. When the function chain is called,
   `f` will be called last. The return value of `f` will be ignored.
.. typefn:: (FunctionChain 'on self)
   
   Returns a decorator that appends the provided function to the
   function chain.
.. typefn:: (FunctionChain 'prepend self f)
   
   Prepend function `f` to function chain. When the function chain is called,
   `f` will be called first. The return value of `f` will be ignored.
.. typefn:: (FunctionChain 'clear self)
   
   Clear the function chain. When the function chain is applied next,
   no functions will be called.
.. sugar:: (decorate-fnchain ...)
.. sugar:: (fnchain name)

   Binds a new unique and empty function chain to identifier `name`. The
   function chain's typename is going to incorporate the name of the module
   in which it was declared.

