Array
=====

Exports a configurable type for a mutable array that stores its elements
on the heap rather than in registers or the stack.

.. type:: Array

   An opaque type of supertype `Struct`.
.. inline:: (Array.sort self key)
.. fn:: (Array.append-slots self n)
.. fn:: (Array.append self value)
.. inline:: (Array.emplace-append self args...)
.. fn:: (Array.clear self)
.. inline:: (Array.gen-sort key)
.. type:: FixedArray

   An opaque type of supertype `Array`.
.. spice:: (FixedArray.gen-fixed-array-type ...)
.. fn:: (FixedArray.reserve self count)
.. inline:: (FixedArray.capacity self)
.. type:: GrowingArray

   An opaque type of supertype `Array`.
.. fn:: (GrowingArray.nearest-capacity capacity count)
.. spice:: (GrowingArray.gen-growing-array-type ...)
.. fn:: (GrowingArray.reserve self count)
.. inline:: (GrowingArray.capacity self)
