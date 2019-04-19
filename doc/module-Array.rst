Array
=====

Exports a configurable type for a mutable array that stores its elements
on the heap rather than in registers or the stack.

.. type:: Array

   An opaque type of supertype `Struct`.
.. typefn:: (Array 'sort self key)
.. typefn:: (Array 'append-slots self n)
.. typefn:: (Array 'append self value)
.. typefn:: (Array 'emplace-append self args...)
.. typefn:: (Array 'clear self)
.. typefn:: (Array 'gen-sort key)
.. type:: FixedArray

   An opaque type of supertype `Array`.
.. spice:: (FixedArray.gen-fixed-array-type ...)
.. typefn:: (FixedArray 'reserve self count)
.. typefn:: (FixedArray 'capacity self)
.. type:: GrowingArray

   An opaque type of supertype `Array`.
.. typefn:: (GrowingArray 'nearest-capacity capacity count)
.. spice:: (GrowingArray.gen-growing-array-type ...)
.. typefn:: (GrowingArray 'reserve self count)
.. typefn:: (GrowingArray 'capacity self)
