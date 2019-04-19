Array
=====

Exports a configurable type for a mutable array that stores its elements
on the heap rather than in registers or the stack.

.. type:: Array

   An opaque type of supertype `Struct`.

   .. fn:: (append-slots self n)
   .. inline:: (gen-sort key)
   .. inline:: (emplace-append self args...)
   .. fn:: (clear self)
   .. fn:: (append self value)
   .. inline:: (sort self key)
.. type:: FixedArray

   An opaque type of supertype `Array`.

   .. spice:: (gen-fixed-array-type ...)
   .. fn:: (reserve self count)
   .. inline:: (capacity self)
.. type:: GrowingArray

   An opaque type of supertype `Array`.

   .. fn:: (reserve self count)
   .. fn:: (nearest-capacity capacity count)
   .. inline:: (capacity self)
   .. spice:: (gen-growing-array-type ...)
