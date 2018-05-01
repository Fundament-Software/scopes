FunctionChain
=============

A function chain implements a compile-time observer pattern that allows
a module to call back into dependent modules in a decoupled way.

.. type:: FunctionChain
.. typefn:: (FunctionChain '__apply-type cls name)
.. typefn:: (FunctionChain 'prepend self f)
   
   Prepend function `f` to function chain. When the function chain is called,
   `f` will be called first. The return value of `f` will be ignored.
.. typefn:: (FunctionChain 'append self f)
   
   Append function `f` to function chain. When the function chain is called,
   `f` will be called last. The return value of `f` will be ignored.
.. typefn:: (FunctionChain 'clear self)
   
   Clear the function chain. When the function chain is applied next,
   no functions will be called.
.. macro:: (fnchain name)

   Binds a new unique and empty function chain to identifier `name`. The
   function chain's typename is going to incorporate the name of the module
   in which it was declared.

