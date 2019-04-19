Array
=====

Exports a configurable type for a mutable array that stores its elements
on the heap rather than in registers or the stack.

.. type:: Array

   ``Array`` < ``Struct`` 
.. typefn:: (Array 'append-slots self n)
.. typefn:: (Array 'gen-sort key)
.. typefn:: (Array 'sort self key)
.. typefn:: (Array 'emplace-append self args...)
.. typefn:: (Array 'clear self)
.. typefn:: (Array 'append self value)
.. type:: FixedArray

   ``FixedArray`` < ``Array`` 
.. spice:: (FixedArray.gen-fixed-array-type ...)
.. typefn:: (FixedArray 'reserve self count)
.. typefn:: (FixedArray 'capacity self)
.. type:: GrowingArray

   ``GrowingArray`` < ``Array`` 
.. typefn:: (GrowingArray 'reserve self count)
.. typefn:: (GrowingArray 'nearest-capacity capacity count)
.. typefn:: (GrowingArray 'capacity self)
.. spice:: (GrowingArray.gen-growing-array-type ...)
